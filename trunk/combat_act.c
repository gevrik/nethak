/**************************************************************************
 * File:    combat_act.c                                                  *
 * Author:  Midboss                                                       *
 * Purpose: Command functions for turn-based combat on ROM MUDs.          *
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

/*
 * Begins a fight!
 */
void do_fight (CHAR_DATA * ch, char * argument)
{
	CHAR_DATA * vch;

	if (is_fighting (ch))
	{
		send_to_char ("You're already fighting!\n\r", ch);
		return;
	}

	if (argument[0] == '\0')
	{
		send_to_char ("Fight whom?\n\r", ch);
		return;
	}
	if ((vch = get_char_room (ch, argument)) == NULL)
	{
		send_to_char ("They aren't here!\n\r", ch);
		return;
	}
	if (vch == ch)
	{
		send_to_char ("You won!\n\r", ch);
		return;
	}

	//Might want to add kill steal/illegal player kill protection to this.
	if (is_fighting (vch))
	{
		char_to_combat (ch, vch->in_battle);
		act ("You rush headlong into the fray!", ch, NULL, vch, TO_CHAR);
		act ("$n rushes headlong into the fray!", ch, NULL, vch, TO_ROOM);
		return;
	}

	initiate_combat (ch, vch);
	act ("You initiate combat against $N.", ch, NULL, vch, TO_CHAR);
	act ("$n initiates combat against you.", ch, NULL, vch, TO_VICT);
	act ("$n initiates combat against $N.", ch, NULL, vch, TO_NOTVICT);
	return;
}

/*
 * Basic attack.  This includes damroll and crude level based attack bonus and
 * defense application.
 */
void do_attack (CHAR_DATA * ch, char * argument)
{
	CHAR_DATA * vch;

	if (!is_fighting (ch))
	{
		send_to_char ("You're not fighting!\n\r", ch);
		return;
	}

	if (argument[0] == '\0')
	{
		send_to_char ("Attack whom?\n\r", ch);
		return;
	}
	if ((vch = get_char_room (ch, argument)) == NULL)
	{
		send_to_char ("They aren't here!\n\r", ch);
		return;
	}
	if (!is_fighting (vch) || is_same_group (ch, vch))
	{
		send_to_char ("They aren't fighting you!\n\r", ch);
		return;
	}

	if (!is_active (ch))
	{
		send_to_char ("Just wait your turn.\n\r", ch);
		return;
	}

	act ("You execute a physical attack against $N.", ch, NULL, vch, TO_CHAR);
	act ("$n executes a physical attack against you.", ch, NULL, vch, TO_VICT);
	act ("$n executes a physical attack against $N.", ch, NULL, vch, TO_NOTVICT);
	damage_new (ch, vch, (GET_DAMROLL(ch) + ch->level) - vch->level, 0, "attack",
				FALSE, DAM_OTHER);
	
	if (is_fighting (ch))
		turn_end (ch->turn, 200 - GET_SPEED (ch));

	return;
}

/*
 * Displays the active turn list.
 * Goes 20 turns deep, assuming everyone uses "Attack".
 */
void do_turns (CHAR_DATA * ch, char * argument)
{
	char buf[100];
	TURN_DATA * turn, * list;
	int round = 1, count = 0;
	int atime = 200, btime = 0;

	if (!is_fighting (ch))
	{
		send_to_char ("You aren't part of a battle.\n\r", ch);
		return;
	}

	for (list = ch->in_battle->turn_list; list != NULL; list = list->next)
		count++;

	if (count < 1)
	{
		send_to_char ("You aren't part of a battle, yet.\n\r", ch);
		return;
	}

	list = ch->in_battle->turn_list;
	turn = ch->in_battle->turn_list;
	while (round < 21)
	{
		if (turn->roundtime + (atime - GET_SPEED (turn->unit)) >
			list->roundtime + (btime - GET_SPEED (turn->unit)))
		{
			if (list->next == NULL)
			{
				turn = list;
				list = ch->in_battle->turn_list;
				btime += 200;
			}
			else
			{
				turn = list;
				list = list->next;
			}
		}
		atime += 200;

		if (turn->unit != ch)
		{
			sprintf (buf, "{D[{C%2d{D]{R $N", round);
			act (buf, ch, NULL, turn->unit, TO_CHAR);
		}
		else
		{
			sprintf (buf, "{D[{C%2d{D]{w You{x\n\r", round);
			send_to_char (buf, ch);
		}
		round++;
	}
	return;
}

/*
 * Displays the unit list for a battle.
 */
void do_units (CHAR_DATA * ch, char * argument)
{
	char buf[100];
	CHAR_DATA * unit;
	int count = 0;

	if (!is_fighting (ch))
	{
		send_to_char ("You aren't part of a battle.\n\r", ch);
		return;
	}

	for (unit = ch->in_battle->unit_list; unit != NULL; unit = unit->next_in_battle)
	{
		count++;

		if (unit != ch)
		{
			if (is_same_group (unit, ch))
				sprintf (buf, "{D[{M%2d{D]{C $N", count);
			else
				sprintf (buf, "{D[{M%2d{D]{R $N", count);
			act (buf, ch, NULL, unit, TO_CHAR);
		}
		else
		{
			sprintf (buf, "{D[{M%2d{D]{w You{x\n\r", count);
			send_to_char (buf, ch);
		}
	}
	return;
}

/*
 * Skips this turn, letting the next character go first.
 */
void do_wait (CHAR_DATA * ch, char * argument)
{
	if (!is_fighting (ch) || !is_active (ch))
	{
		act ("You wait patiently.", ch, NULL, NULL, TO_CHAR);
		act ("$n waits patiently.", ch, NULL, NULL, TO_ROOM);
		return;
	}

	act ("You bide your time to see what $N will do.", ch, NULL,
			ch->in_battle->turn_list->next->unit, TO_CHAR);
	act ("$n bides $s time to see what you will do.", ch, NULL,
			ch->in_battle->turn_list->next->unit, TO_VICT);
	act ("$n bides $s time to see what $N will do.", ch, NULL,
			ch->in_battle->turn_list->next->unit, TO_NOTVICT);
	skip_turn (ch->turn, TRUE);
	return;
}

/*
 * Attempts to flee from combat.
 */
void do_escape (CHAR_DATA * ch, char * argument)
{
	COMBAT_DATA * battle;

	if (!is_fighting (ch))
	{
		send_to_char ("You try frantically to escape from your own stupidity.\n\r", ch);
		return;
	}
	if (!is_active (ch))
	{
		send_to_char ("Just wait your turn to run away screaming!\n\r", ch);
		return;
	}

	if (number_percent () > 40)
	{
		act ("You failed to escape!", ch, NULL, NULL, TO_CHAR);
		act ("$n tries desperately to escape, but fails!", ch, NULL, NULL, TO_ROOM);
		turn_end (ch->turn, 160 - GET_SPEED (ch));
		return;
	}

	battle = ch->in_battle;

	act ("You execute a daring escape to stage left!", ch, NULL, NULL, TO_CHAR);
	act ("$n executes a daring escape to stage left!", ch, NULL, NULL, TO_ROOM);
	char_from_combat (ch);
	check_victory (battle);
	return;
}
