/**************************************************************************
 * File:    combat.c                                                      *
 * Author:  Midboss                                                       *
 * Purpose: Core functionality for turn-based combat on ROM MUDs.         *
 * License:                                                               *
 *       Give credit where it is due; be it in the main combat helpfile   *
 *       or in your login sequence.  Just don't claim this as your own    *
 *       original work, lest you become the next Vryce.                   *
 *                                                                        *
 *    This code is provided as-is, and was created on a stock QuickMUD    *
 *    server.  The only guarantee I'll offer is that it works, when       *
 *    properly installed, on a ROM MUD (in particular, QuickMUD).         *
 **************************************************************************/

#if defined(macintosh)
#include <types.h>
#else
#include <sys/types.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include "merc.h"
#include "interp.h"

/*
 * Updater for what few autonomous functions used in this combat system.
 * Mostly just triggers mobile's turns and updates battlefield conditions.
 * Also controls the automatic turn skip when someone tries to stall.
 */
void combat_update (void)
{
	COMBAT_DATA * battle, * battle_next;
	CHAR_DATA * ch;

	for (battle = battle_list; battle != NULL; battle = battle_next)
	{
		battle_next = battle->next;

		if (battle->turn_list->roundtime >= 2000000000)
			nuke_roundtime (battle);

		ch = battle->turn_list->unit;

		if (ch == NULL)
		{
			bug ("combat_update (): Turn with NULL character!", 0);
			clean_combat (battle);
		}
		else if (IS_NPC (ch))
			mobile_turn (ch);
		else if (ch->desc == NULL)
			linkdead_turn (ch);
		else
		{
			battle->turn_list->timer++;

			if (battle->turn_list->timer == 70)
				turn_end (battle->turn_list, 300);
		}
	}
}

/*
 * Forces a mobile to take its turn.
 */
void mobile_turn (CHAR_DATA * ch)
{
	CHAR_DATA * vch, * list;

	vch = ch;

	for (list = ch->in_battle->unit_list; list != NULL; list = list->next_in_battle)
	{
		if (!is_same_group (ch, list))
		{
			if (vch == ch)
				vch = list;
			else if (vch != ch && number_percent () < 45)
				vch = list;
			else continue;
		}
	}

	/*
	 * I'm leaving this at just attack for obvious reasons.
	 * Be creative, will ya?
	 */
	if (vch == ch)
		clean_combat (ch->in_battle);
	else
		do_attack (ch, vch->name);
}

/*
 * PC version of mobile turn, for linkdead characters.
 */
void linkdead_turn (CHAR_DATA * ch)
{
	CHAR_DATA * vch, * list;

	if (number_percent () < 40)
	{
		do_escape (ch, "");
		return;
	}

	vch = ch;

	for (list = ch->in_battle->unit_list; list != NULL; list = list->next_in_battle)
	{
		if (!is_same_group (ch, list))
		{
			if (vch == ch)
				vch = list;
			else if (vch != ch && number_percent () < 45)
				vch = list;
			else continue;
		}
	}

	if (vch == ch)
		clean_combat (ch->in_battle);
	else
		do_attack (ch, vch->name);
}

/*
 * This is a simple replacement to the original damage, here solely to make
 * the snippet run clean in stockier MUDs.
 */
bool damage_new (CHAR_DATA * ch, CHAR_DATA * vch, long dam, int sn, char * damverb,
				 bool hide, bool fKill)
{
	/* Make sure the damage is valid. */
	if (ch == NULL || vch == NULL)
	{
		bug ("damage_new(): missing character", 0);
		return FALSE;
	}

	/* Apply damage caps. */
	if (!fKill)
		dam = UMIN (dam, vch->hit - 1);

	dam = UMIN (dam, 1200);

	/* If you add damtype(s), this'd be where to check them. */

	/* Dish out the damage. */
	vch->hit = UMAX (0, vch->hit - dam);
	
	/* Grab an sn-based damverb if one isn't present. */
	if (damverb[0] == '\0')
	{
        if (sn >= 0 && sn < MAX_SKILL && skill_table[sn].noun_damage != NULL)
            damverb = skill_table[sn].noun_damage;
		else
			damverb = "attack";
	}

	/* Dispatch messaging, if hide is false. */
	if (!hide)
	{
		char buf[200];

		if (dam > 0)
		{
			sprintf (buf, "{CYour $t hits $N for %ld damage!{x\n\r", dam);
			act (buf, ch, damverb, vch, TO_CHAR);

			sprintf (buf, "{R$n's $t hits you for %ld damage!{x\n\r", dam);
			act (buf, ch, damverb, vch, TO_VICT);
			
			sprintf (buf, "{D$n's $t hits $N for %ld damage!{x\n\r", dam);
			act (buf, ch, damverb, vch, TO_NOTVICT);
		}
		else
		{
			act ("{CYour $t misses $N!{x\n\r", ch, damverb, vch, TO_CHAR);
			act ("{R$n's $t misses you!{x\n\r", ch, damverb, vch, TO_VICT);
			act ("{D$n's $t misses $N!{x\n\r", ch, damverb, vch, TO_NOTVICT);
		}
	}
	/* Trigger death. */
	if (vch->hit <= 0)
		kill_unit (vch);

	return TRUE;
}

/*
 * Dispatches the KO affect to a unit and removes its turn from the list.
 * Will not remove the unit from combat.
 */
void kill_unit (CHAR_DATA * ch)
{
	if (!is_fighting (ch))
		return;


	SET_BIT(ch->affected_by, AFF_KNOCKOUT);
	remove_turn (ch->turn);

	act ("{DYou feel the sweet embrace of sleep eternal as your body "
	     "fades out of existence.{x\n\r",
			ch, NULL, NULL, TO_CHAR);
	act ("$n's body slowly fades into nothingness.\n\r", ch, NULL, NULL, TO_ROOM);

	check_victory (ch->in_battle);
}

/*
 * Removes the KO affect from a unit and adds its turn back to the list.
 * Will not remove the unit from combat.
 */
void revive_unit (CHAR_DATA * ch)
{
	if (!is_fighting (ch))
		return;

	REMOVE_BIT(ch->affected_by, AFF_KNOCKOUT);

	ch->turn->roundtime = (ch->in_battle->turn_list->roundtime + 500);
	insert_turn (ch->in_battle, ch->turn);
}

/*
 * This will end combat.  You're on your own when tallying up the experience
 * gains and whatnot, I'm just giving you the core, after all.
 */
void check_victory (COMBAT_DATA * battle)
{
	CHAR_DATA * leader;
	CHAR_DATA * unit;
	bool victory = TRUE;
	bool found = FALSE;


	//First look for PCs.  Since mobs don't gain, if only mobs are left, we abort.
	for (unit = battle->unit_list; unit != NULL; unit = unit->next_in_battle)
		if (!IS_NPC (unit))
			found = TRUE;

	if (!found)
	{
		clean_combat (battle);
		return;
	}

	//PCs exist!  Grab a guy.
	for (leader = battle->unit_list; leader != NULL; leader = leader->next_in_battle)
	{
		if (IS_AFFECTED (leader, AFF_KNOCKOUT))
			continue;

		//Once we find the first guy that isn't dead, we look for another guy that
		//isn't dead, and isn't on his team.  If one is found, battle's over.
		for (unit = battle->unit_list; unit != NULL; unit = unit->next_in_battle)
		{
			if ((IS_AFFECTED (unit, AFF_KNOCKOUT) && !is_same_group (leader, unit))
				|| is_same_group (leader, unit) || leader == unit)
				continue;
			else
			{
				victory = FALSE;
				break;
			}
		}

		if (!victory)
			continue;
		else
		{
			for (unit = battle->unit_list; unit != NULL; unit = unit->next_in_battle)
			{
				if (is_same_group (leader, unit))
					act ("Your party has won the battle!\n\r", unit, NULL, NULL, TO_CHAR);
				else
					act ("Your party has lost the battle...\n\r", unit, NULL, NULL, TO_CHAR);
			}
			clean_combat (battle);
			return;
		}

	}
}

/*
 * True if ch is part of any combat.
 */
bool is_fighting (CHAR_DATA * ch)
{
	if (ch->in_battle != NULL)
		return TRUE;

	return FALSE;
}

/*
 * True if ch may take a turn right now.
 */
bool is_active (CHAR_DATA * ch)
{
	if (!is_fighting (ch))
		return FALSE;	

	if (ch->turn == ch->in_battle->turn_list)
		return TRUE;

	return FALSE;
}

/*
 * Inserts a character into combat and adds their turn to the list.
 */
void char_to_combat (CHAR_DATA * ch, COMBAT_DATA * combat)
{
	TURN_DATA * turn;

	if (combat->unit_list == NULL)
		combat->unit_list = ch;
	else
	{
		ch->next_in_battle = combat->unit_list;
		combat->unit_list  = ch;
	}
	ch->in_battle = combat;

	turn = new_turn ();
	turn->unit = ch;
	turn->in_battle = combat;

	//So people don't end up getting a dozen turns by joining in.
	if (combat->turn_count > 0) 
		turn->roundtime = (combat->turn_list->roundtime + 200) - GET_SPEED (ch);
	else
		turn->roundtime = 500 - GET_SPEED (ch);

	ch->turn = turn;
	insert_turn (combat, turn);
}

/*
 * Removes a ch and their turn entry from combat.
 */
void char_from_combat (CHAR_DATA * ch)
{
	CHAR_DATA * prev;

	if (ch == NULL)
	{
		bug ("cfc () -- null ch!?", 0);
		return;
	}

	if (!is_fighting (ch))
		return;

	if (ch->in_battle->unit_list == ch)
	{
		ch->in_battle->unit_list = ch->next_in_battle;
		ch->next_in_battle = NULL;
	}
	else
	{
		for (prev = ch->in_battle->unit_list; prev != NULL; prev = prev->next_in_battle)
		{
			if (prev->next_in_battle == ch)
			{
				prev->next_in_battle = ch->next_in_battle;
				ch->next_in_battle = NULL;
				break;
			}
		}

		if (prev == NULL)
		{
			bug ("cfc () -- ch not found.", 0);
			return;
		}
	}
	remove_turn (ch->turn);
	ch->turn = NULL;
	ch->in_battle = NULL;

	/*
	 * Here I've chosen to revive characters as they leave combat, but
	 * you'll want to enter PCs into your death handler at this point,
	 * and extract mobiles.
	 */

	if (IS_AFFECTED (ch, AFF_KNOCKOUT))
	{
		REMOVE_BIT(ch->affected_by, AFF_KNOCKOUT);
		ch->hit = UMAX(1, ch->hit);
	}
}

/*
 * Returns the data for ch's next/current turn.
 */
TURN_DATA * get_turn_char (CHAR_DATA * ch)
{
	TURN_DATA * turn;

	if (!is_fighting (ch))
		return NULL;

	for (turn = ch->in_battle->turn_list; turn != NULL; turn = turn->next)
		if (turn->unit == ch)
			return turn;

	return NULL;
}

/*
 * Inserts a turn into its proper position in the specified list.
 */
void insert_turn (COMBAT_DATA * combat, TURN_DATA * nTurn)
{

	if (combat->turn_list == NULL)
	{
		combat->turn_list = nTurn;
		nTurn->in_battle = combat;
	}
	else
	{
		TURN_DATA * prev;

		if (combat->turn_list->roundtime > nTurn->roundtime)
		{
			nTurn->next = combat->turn_list;
			combat->turn_list = nTurn;
			nTurn->in_battle = combat;
		}
		else
		{
			for (prev = combat->turn_list; prev != NULL; prev = prev->next)
			{
				if (prev->next != NULL && prev->next->roundtime > nTurn->roundtime)
				{
					nTurn->next = prev->next;
					prev->next  = nTurn;
					nTurn->in_battle = combat;
					break;
				}
				else if (prev->next == NULL)
				{
					prev->next  = nTurn;
					nTurn->in_battle = combat;
					break;
				}
			}
		}
	}
}

/*
 * Removes a turn from the list without freeing it.
 * It isn't freed here because you have to use this every time you move a turn
 * to a new position in the list.
 */
void remove_turn (TURN_DATA * turn)
{
	TURN_DATA * prev;

	if (turn == NULL)
	{
		bug ("remove_turn () -- null turn!?", 0);
		return;
	}

	if (turn == turn->in_battle->turn_list)
	{
		turn->in_battle->turn_list = turn->next;
		turn->next = NULL;
	}
	else
	{
		for (prev = turn->in_battle->turn_list; prev != NULL; prev = prev->next)
		{
			if (prev->next == turn)
			{
				prev->next = turn->next;
				turn->next = NULL;
				break;
			}
		}
	}
}


/*
 * Similar to roundtime (), but used specifically at the end of each action.
 */
void turn_end (TURN_DATA * turn, int roundtime)
{
	turn->timer = 0;
	turn->roundtime += roundtime;
	/*
	 * On my MUD, I have code here to update affects each turn.
	 */
	remove_turn (turn);
	insert_turn (turn->in_battle, turn);
	turn->in_battle->turn_count++;
}

/*
 * Moves the turn down the list a bit.
 */
void skip_turn (TURN_DATA * turn, bool fWilling)
{
	turn->roundtime = turn->next->roundtime + number_range (45, 75);
	remove_turn (turn);
	insert_turn (turn->in_battle, turn);
	if (fWilling)
		turn->in_battle->turn_count++;
}

/*
 * Adds to the roundtime of a turn, then replaces it in the order.
 */
void roundtime (TURN_DATA * turn, int roundtime)
{
	turn->roundtime += roundtime;
	remove_turn (turn);
	insert_turn (turn->in_battle, turn);
}

/*
 * Swaps the placement of two turns, by switching roundtimes and reordering.
 */
void swap_turn (TURN_DATA * aTurn, TURN_DATA * bTurn)
{
	COMBAT_DATA * battle = aTurn->in_battle;
	int rt = aTurn->roundtime;

	aTurn->roundtime = bTurn->roundtime;
	bTurn->roundtime = rt;

	remove_turn (aTurn);
	remove_turn (bTurn);
	insert_turn (battle, aTurn);
	insert_turn (battle, bTurn);
}

/*
 * Just in case some newb with a trigger leaves a battle against a training
 * dummy running for weeks on end with trigs, or something.  You can never
 * have too much protection, y'know.                             -- Midboss
 */
void nuke_roundtime (COMBAT_DATA * battle)
{
	TURN_DATA * list;

	for (list = battle->turn_list; list != NULL; list = list->next)
		list->roundtime -= 2000000000;
}

/*
 * Creates a new battle between two parties.
 * Recycles the data if there somehow aren't enough people.
 */
void initiate_combat (CHAR_DATA * ch, CHAR_DATA * vch)
{
	CHAR_DATA * och;
	TURN_DATA * turn;
	COMBAT_DATA * battle;
	int acount = 0, bcount = 0;

	battle = new_combat ();

	if (battle_list == NULL)
		battle_list = battle;
	else
	{
		battle->next = battle_list;
		battle_list = battle;
	}


	for (och = ch->in_room->people; och != NULL; och = och->next_in_room)
	{
		if (is_same_group (ch, och))
			char_to_combat (och, battle);
		if (is_same_group (vch, och))
			char_to_combat (och, battle);
	}

	for (turn = battle->turn_list; turn != NULL; turn = turn->next)
	{
		if (is_same_group (ch, turn->unit))
			acount++;
		else 
			bcount++;
	}
	if (acount < 1 || bcount < 1)
		clean_combat (battle);
}

/*
 * Frees combat data at the end of battle.
 */
void clean_combat (COMBAT_DATA * battle)
{
	COMBAT_DATA * list;
	CHAR_DATA * unit, *unit_next;

	for (unit = battle->unit_list; unit != NULL; unit = unit_next)
	{
		unit_next = unit->next_in_battle;
		char_from_combat (unit);
		//Perhaps call raw_kill() here on KOd mobiles?
	}

	if (battle == battle_list)
		battle_list = battle->next;
	else
	for (list = battle_list; list != NULL; list = list->next)
	{
		if (list->next == battle)
			list->next = battle->next;
	}

	battle->next = NULL;

	free_combat (battle);
}

/*
 * Standard Merc recycling functions for turns and battles.
 */
TURN_DATA *turn_free;

TURN_DATA *new_turn (void)
{
    static TURN_DATA turn_zero;
    TURN_DATA *turn;

    if (turn_free == NULL)
        turn = alloc_perm (sizeof (*turn));
    else
    {
        turn = turn_free;
        turn_free = turn_free->next;
    }

    *turn = turn_zero;
    VALIDATE (turn);
    return turn;
}

void free_turn (TURN_DATA * turn)
{
    if (!IS_VALID (turn))
        return;

    INVALIDATE (turn);

    turn->next = turn_free;
    turn_free = turn;
}

COMBAT_DATA *combat_free;

COMBAT_DATA *new_combat (void)
{
    static COMBAT_DATA combat_zero;
    COMBAT_DATA *combat;

    if (combat_free == NULL)
        combat = alloc_perm (sizeof (*combat));
    else
    {
        combat = combat_free;
        combat_free = combat_free->next;
    }

    *combat = combat_zero;
    VALIDATE (combat);
    return combat;
}

void free_combat (COMBAT_DATA * combat)
{
    if (!IS_VALID (combat))
        return;

    INVALIDATE (combat);

    combat->next = combat_free;
    combat_free = combat;
}
