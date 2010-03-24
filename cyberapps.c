#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include "mud.h"

void do_sn_jackhammer(CHAR_DATA *ch, char *argument) {

	DESCRIPTOR_DATA *d;
	OBJ_DATA *obj;
	EXIT_DATA *xit, *texit;
	int edir;
	ROOM_INDEX_DATA *location;
	char buf[MAX_STRING_LENGTH];
	char bufa[MAX_STRING_LENGTH];
	PLANET_DATA *planet;
	bool ch_snippet;

	planet = ch->in_room->area->planet;

	if (IS_NPC(ch) || !ch->pcdata || !ch->in_room)
		return;

	if (IS_SET( ch->in_room->room_flags , ROOM_NOPEDIT )) {
		send_to_char("> you cannot use this app here\n\r", ch);
		return;
	}

	if (argument[0] == '\0') {
		send_to_char("> syntax: jackhammer <direction>\n\r", ch);
		return;
	}

	location = ch->in_room;
	edir = get_dir(argument);
	xit = get_exit(ch->in_room, edir);

	if (!xit) {
		send_to_char("> &Rno connection in that direction&w\n\r", ch);
		return;
	}

	if (!IS_SET( xit->exit_info , EX_ISDOOR )) {
		send_to_char("> &Rno codegate in that direction&w\n\r", ch);
		return;
	} else {

		ch_snippet = FALSE;

		for (obj = ch->last_carrying; obj; obj = obj->prev_content) {
			if ( obj->item_type == ITEM_SNIPPET && !strcmp(obj->name,
					"jackhammer") ) {
				ch_snippet = TRUE;
				separate_obj(obj);
				obj_from_char(obj);
				extract_obj( obj );
			}
		}

		if (!ch_snippet) {
			send_to_char("> &Rjackhammer application needed&w\n\r", ch);
			return;
		}

		sprintf(buf, "> %s's jackhammer breaks down the gate to the: %s",
				ch->name, dir_name[edir]);
		echo_to_room(AT_RED, ch->in_room, buf);

		sprintf(bufa, "> %s used JACKHAMMER in %s ",
				ch->name, ch->in_room->area->planet->name);
		echo_to_clan(AT_RED, bufa, ECHOTAR_ALL, ch->in_room->area->planet->governed_by);

		REMOVE_BIT( xit->exit_info , EX_ISDOOR );
		REMOVE_BIT( xit->exit_info , EX_LOCKED );
		REMOVE_BIT( xit->exit_info , EX_CLOSED );
		xit->key = -1;
		texit = get_exit_to(xit->to_room, rev_dir[edir], ch->in_room->vnum);
		if (texit) {
			REMOVE_BIT( texit->exit_info , EX_ISDOOR );
			REMOVE_BIT( texit->exit_info , EX_LOCKED );
			REMOVE_BIT( texit->exit_info , EX_CLOSED );
			texit->key = -1;
		}

	}

	SET_BIT( ch->in_room->area->flags , AFLAG_MODIFIED );

}

void do_sn_krash(CHAR_DATA *ch, char *argument) {

	CLAN_DATA *clan;
	DESCRIPTOR_DATA *d;
	OBJ_DATA *obj;
	ROOM_INDEX_DATA *location;
	char buf[MAX_STRING_LENGTH];
	char bufa[MAX_STRING_LENGTH];
	PLANET_DATA *planet;
	bool ch_snippet;

	planet = ch->in_room->area->planet;

	if (IS_NPC(ch) || !ch->pcdata || !ch->in_room)
		return;

	if ( IS_NPC(ch) || !ch->pcdata )
	   {
	       send_to_char ( "huh?\n\r" , ch );
	       return;
	   }

	   clan = ch->pcdata->clan;

	   if ( ( planet = ch->in_room->area->planet ) == NULL )
	   {
	       send_to_char ( "> &Ryou cannot do that here&w\n\r" , ch );
	       return;
	   }

	   if ( IS_SET( planet->flags, PLANET_NOCAP ) )
	   {
	       send_to_char( "> &Ryou cannot do that here&w\n\r", ch );
	       return;
	   }

		if ( IS_SET( ch->in_room->room_flags, ROOM_SAFE ) )
		{
			set_char_color( AT_MAGIC, ch );
			send_to_char( "> this is not a good place to do that\n\r", ch );
			return;
		}

		if ( IS_SET( ch->in_room->room_flags, ROOM_HOTEL ) )
		{
			set_char_color( AT_MAGIC, ch );
			send_to_char( "> this is not a good place to do that\n\r", ch );
			return;
		}

		if ( IS_SET( ch->in_room->room_flags, ROOM_EMPLOYMENT ) )
		{
			set_char_color( AT_MAGIC, ch );
			send_to_char( "> this is not a good place to do that\n\r", ch );
			return;
		}

		if ( ch->position == POS_FIGHTING )
		{
			send_to_char( "> cannot krash in combat\n\r" , ch );
			return;
		}

		if ( ch->position <= POS_SLEEPING )
		{
			send_to_char( "> you are sleeping\n\r" , ch );
			return;
		}

	   if ( clan == planet->governed_by )
	   {
	       send_to_char ( "> &Ryour organization already controls this system&w\n\r" , ch );
	       return;
	   }


		ch_snippet = FALSE;

		for (obj = ch->last_carrying; obj; obj = obj->prev_content) {
			if (obj->item_type == ITEM_SNIPPET && !strcmp(obj->name,
					"krash")) {
				ch_snippet = TRUE;
				separate_obj(obj);
				obj_from_char(obj);
				extract_obj( obj );
			}
		}

		if (!ch_snippet) {
			send_to_char("> &Rkrash application needed&w\n\r", ch);
			return;
		}

		WAIT_STATE( ch, skill_table[gsn_propaganda]->beats );

		sprintf(buf, "> %s's krash slows down the cpu of the system.",
				ch->name);
		echo_to_room(AT_YELLOW, ch->in_room, buf);

		sprintf(bufa, "> %s used KRASH in %s ",
				ch->name, ch->in_room->area->planet->name);
		echo_to_clan(AT_RED, bufa, ECHOTAR_ALL, ch->in_room->area->planet->governed_by);

		//found = FALSE;

//		   for( d = first_descriptor; d; d = d->next )
//		   {
//		      if( !d->character )
//		         continue;
//		      if( d->connected != CON_PLAYING )
//		         continue;
//		      if( IS_IMMORTAL( d->character ) )
//		         continue;
//
//		      if( d->character->pcdata->clan == location->area->planet->governed_by )
//		      {
//
//			      send_to_char( "> &R[&YALERT&R]&W enemy activity! krash used!\n\r", d->character );
//			      ch_printf( d->character, "> &R[&YALERT&R]&W in system: %s&w\n\r", ch->in_room->area->planet->name );
//
//		      }
//		   }

		   planet->pop_support -= 1;

			if ( planet->pop_support > 100 )
				planet->pop_support = 100;
			if ( planet->pop_support < -100 )
				planet->pop_support = -100;

}

void do_sn_spun(CHAR_DATA *ch, char *argument) {

	OBJ_DATA *obj;
	int energyplus;
	char buf[MAX_STRING_LENGTH];
	bool ch_snippet;

	if ( IS_NPC(ch) || !ch->pcdata )
	   {
	       send_to_char ( "huh?\n\r" , ch );
	       return;
	   }


		if ( ch->position <= POS_SLEEPING )
		{
			send_to_char( "> you are hibernating\n\r" , ch );
			return;
		}

		if ( ch->move >= ch->max_move )
		{
			send_to_char( "> you are already at maximum\n\r" , ch );
			return;
		}

		ch_snippet = FALSE;

		for (obj = ch->last_carrying; obj; obj = obj->prev_content) {
			if (obj->item_type == ITEM_SNIPPET && !strcmp(obj->name,
					"spun")) {
				ch_snippet = TRUE;
				energyplus = obj->value[0];
				separate_obj(obj);
				obj_from_char(obj);
				extract_obj( obj );
			}
		}

		if (!ch_snippet) {
			send_to_char("> &Rspun application needed&w\n\r", ch);
			return;
		}

		WAIT_STATE( ch, skill_table[gsn_propaganda]->beats );

		sprintf(buf, "> %s uses spun to regain some energy",
				ch->name);
		echo_to_room(AT_RED, ch->in_room, buf);


		if ( ch->move < ch->max_move )
		{
			if ( ch->move + energyplus > ch->max_move )
				ch->move = ch->max_move;
			else
				ch->move = ch->move + energyplus;
		}

}

void do_sn_reconstruct(CHAR_DATA *ch, char *argument) {

	OBJ_DATA *obj;
	int energyplus;
	char buf[MAX_STRING_LENGTH];
	bool ch_snippet;

	if ( IS_NPC(ch) || !ch->pcdata )
	   {
	       send_to_char ( "huh?\n\r" , ch );
	       return;
	   }


		if ( ch->position <= POS_SLEEPING )
		{
			send_to_char( "> you are hibernating\n\r" , ch );
			return;
		}

		if ( ch->hit >= ch->max_hit )
		{
			send_to_char( "> you are already at maximum\n\r" , ch );
			return;
		}

		ch_snippet = FALSE;

		for (obj = ch->last_carrying; obj; obj = obj->prev_content) {
			if (obj->item_type == ITEM_SNIPPET && !strcmp(obj->name,
					"reconstruct")) {
				ch_snippet = TRUE;
				energyplus = obj->value[0] / 4;
				separate_obj(obj);
				obj_from_char(obj);
				extract_obj( obj );
			}
		}

		if (!ch_snippet) {
			send_to_char("> &Rreconstruct application needed&w\n\r", ch);
			return;
		}

		WAIT_STATE( ch, skill_table[gsn_propaganda]->beats );

		sprintf(buf, "> %s uses reconstruct to regain some health",
				ch->name);
		echo_to_room(AT_RED, ch->in_room, buf);


		if ( ch->hit < ch->max_hit )
		{
			if ( ch->hit + energyplus > ch->max_hit )
				ch->hit = ch->max_hit;
			else
				ch->hit = ch->hit + energyplus;
		}

}

void do_sn_dropline(CHAR_DATA *ch, char *argument) {

	OBJ_DATA *obj;
	char buf[MAX_STRING_LENGTH];
	bool ch_snippet;

	if ( IS_NPC(ch) || !ch->pcdata )
	   {
	       send_to_char ( "huh?\n\r" , ch );
	       return;
	   }


		if ( ch->position <= POS_SLEEPING )
		{
			send_to_char( "> you are hibernating\n\r" , ch );
			return;
		}

		if ( ch->fighting )
		{
			send_to_char( "> dropline cannot be used in combat\n\r", ch );
			return;
		}

		if ( IS_SET( ch->in_room->room_flags, ROOM_ARENA ) )
		{
			send_to_char( "> &Rfinish the current match first&w\n\r", ch );
			return;
		}

		if ( ch->in_room->vnum <= 20 )
		{
			send_to_char( "> &Ryou cannot use this command in the tutorial&w\n\r", ch );
			return;
		}

		ch_snippet = FALSE;

		for (obj = ch->last_carrying; obj; obj = obj->prev_content) {
			if (obj->item_type == ITEM_SNIPPET && !strcmp(obj->name,
					"dropline")) {
				ch_snippet = TRUE;
				separate_obj(obj);
				obj_from_char(obj);
				extract_obj( obj );
			}
		}

		if (!ch_snippet) {
			send_to_char("> &Rdropline application needed&w\n\r", ch);
			return;
		}

		WAIT_STATE( ch, skill_table[gsn_propaganda]->beats );

		sprintf(buf, "> %s drops their line",
				ch->name);
		echo_to_room(AT_RED, ch->in_room, buf);

		if( !ch->plr_home )
		{
			char_from_room( ch );
			char_to_room( ch, get_room_index( ROOM_VNUM_STRAY ) );
			do_look( ch, "auto" );
			return;
		}

		char_from_room( ch );
		char_to_room( ch, ch->plr_home );
		do_look( ch, "auto" );

		return;


}

void do_sn_uninstall(CHAR_DATA *ch, char *argument) {

	OBJ_DATA *obj;
	char buf[MAX_STRING_LENGTH];
	bool ch_snippet;

	if (IS_NPC(ch) || !ch->pcdata)
		return;

	if (argument[0] == '\0') {
		send_to_char("> syntax: uninstall <skillsoft>\n\r", ch);
		return;
	}

		ch_snippet = FALSE;

		for (obj = ch->last_carrying; obj; obj = obj->prev_content) {
			if ( obj->item_type == ITEM_SNIPPET && !strcmp(obj->name,
					"uninstall") ) {
				ch_snippet = TRUE;
				separate_obj(obj);
				obj_from_char(obj);
				extract_obj( obj );
			}
		}

		if (!ch_snippet) {
			send_to_char("> &Runinstall application needed&w\n\r", ch);
			return;
		}

	    int sn;

	    sn = skill_lookup( argument );

	    if ( sn == -1 )
	    {
	        send_to_char( "> no such skillsoft\n\r", ch );
		return;
	    }

	    if ( ch->pcdata->learned[sn] <= 0 )
	    {
	    send_to_char( "> you dont know that skillsoft\n\r", ch );
		return;
	    }

		sprintf(buf, "> %s uninstalls a skillsoft",
				ch->name);
		echo_to_room(AT_YELLOW, ch->in_room, buf);

	    send_to_char( "> you uninstalled the skillsoft\n\r", ch );

	    if ( ch->pcdata->learned[sn] == 100 )
	    {
		    ch->pcdata->adept_skills--;
	    }

	    ch->pcdata->learned[sn] = 0;
	    ch->pcdata->num_skills--;
	    save_char_obj(ch);

}

void do_sn_anchor( CHAR_DATA *ch, char *argument )
{
	OBJ_DATA *obj;
	ROOM_INDEX_DATA *destination;
	char buf[MAX_STRING_LENGTH];
	bool ch_snippet;
	int targetnode;

	if ( IS_NPC(ch) || !ch->pcdata )
	   {
	       send_to_char ( "huh?\n\r" , ch );
	       return;
	   }


		if ( ch->position <= POS_SLEEPING )
		{
			send_to_char( "> you are hibernating\n\r" , ch );
			return;
		}

		if ( ch->fighting )
		{
			send_to_char( "> anchor cannot be used in combat\n\r", ch );
			return;
		}

		if ( IS_SET( ch->in_room->room_flags, ROOM_ARENA ) )
		{
			send_to_char( "> &Rfinish the current match first&w\n\r", ch );
			return;
		}

		if ( ch->in_room->vnum <= 20 )
		{
			send_to_char( "> &Ryou cannot use this in the tutorial&w\n\r", ch );
			return;
		}

		ch_snippet = FALSE;

		for (obj = ch->last_carrying; obj; obj = obj->prev_content) {
			if ( obj->item_type == ITEM_SNIPPET && nifty_is_name("anchor", obj->name) ) {
				ch_snippet = TRUE;
				targetnode = obj->value[1];

				destination = get_room_index( targetnode );

				if ( IS_SET( destination->room_flags , ROOM_PLR_HOME ) )
				{
					send_to_char( "> &Rtarget location is not valid&w\n\r", ch );
					separate_obj(obj);
					obj_from_char(obj);
					extract_obj( obj );
					return;
				}

				separate_obj(obj);
				obj->value[0] -= 1;

				if (obj->value[0] < 1)
				{
				obj_from_char(obj);
				extract_obj( obj );
				send_to_char("> &Ranchor application has expired&w\n\r", ch);
				}
			}
		}

		if (!ch_snippet) {
			send_to_char("> &Ranchor application needed&w\n\r", ch);
			return;
		}



		WAIT_STATE( ch, skill_table[gsn_propaganda]->beats );

		sprintf(buf, "> %s uses an anchor app",
				ch->name);
		echo_to_room(AT_RED, ch->in_room, buf);

			char_from_room( ch );
			char_to_room( ch, get_room_index( targetnode ) );
			do_look( ch, "auto" );
			return;

	}

void do_sn_audit( CHAR_DATA *ch, char *argument )
{
	char buf [MAX_STRING_LENGTH];
	ROOM_INDEX_DATA	*location;
	AREA_DATA *area;
	PLANET_DATA *planet;
	OBJ_DATA *obj;
	bool ch_snippet;
	int chance, roll, margin, count = 0;

	location = ch->in_room;
	planet = ch->in_room->area->planet;
	area = ch->in_room->area;

	if ( IS_NPC(ch) || !ch->pcdata )
	   {
	       send_to_char ( "huh?\n\r" , ch );
	       return;
	   }


		if ( ch->position <= POS_SLEEPING )
		{
			send_to_char( "> you are hibernating\n\r" , ch );
			return;
		}

		if ( ch->fighting )
		{
			send_to_char( "> audit cannot be used in combat\n\r", ch );
			return;
		}

		if ( IS_SET( ch->in_room->room_flags, ROOM_SAFE ) )
		{
			send_to_char( "> &Raudit cannot be used in safe nodes&w\n\r", ch );
			return;
		}

		if ( ch->in_room->vnum <= 20 )
		{
			send_to_char( "> &Ryou cannot use this in the tutorial&w\n\r", ch );
			return;
		}


		if ( argument[0] == '\0' )
		{
			send_to_char("> &Rsyntax: audit [option]&w\n\r", ch);
			send_to_char("> &Wgain information about specified node&w\n\r", ch);
			send_to_char("> &Woptions: fw (firewall)&w\n\r", ch);
			return;
		}

		ch_snippet = FALSE;

		for (obj = ch->last_carrying; obj; obj = obj->prev_content) {
			if (obj->item_type == ITEM_SNIPPET && !strcmp(obj->name,
					"audit")) {
				ch_snippet = TRUE;

				obj->value[0] -= 1;

				if (obj->value[0] < 1)
				{
				separate_obj(obj);
				obj_from_char(obj);
				extract_obj( obj );
				send_to_char("> &Raudit application has expired&w\n\r", ch);
				}

			}

		}

	if (!ch_snippet) {
		send_to_char("> &Raudit application needed&w\n\r", ch);
		return;
	}

	WAIT_STATE( ch, skill_table[gsn_propaganda]->beats );

    sprintf(buf,"> &y%s uses an audit application", ch->name);
    act(AT_WHITE,buf, ch, NULL, NULL, TO_ROOM);

	chance = IS_NPC(ch) ? ch->top_level
			: (int) (ch->pcdata->learned[gsn_spacecraft]);

	if ( location->level != 0 )
	chance -= ( location->level * 10 );

	roll = number_percent();

	if ( roll >= chance )
	{
		send_to_char("> &Ryou failed the audit&w\n\r", ch);
		return;
	}

	margin = chance - roll;

	if ( !str_cmp( argument, "fw" ) )
	{
		ch_printf( ch, "> &Gaudit for: %s - type: %s&w\n\r", planet->name, argument );

		for ( location = planet->area->first_room ; location ; location = location->next_in_area )
		{
			if ( IS_SET( location->room_flags, ROOM_BARRACKS ) )
			{
				ch_printf( ch , "   %-15d  %s\n\r", location->vnum, location->name);
				count++;
			}
		}

	    if ( !count )
	    {
		set_char_color( AT_BLOOD, ch);
	        send_to_char( "     no firewalls in this system\n\r", ch );
	    }
	    else
	    {
			ch_printf( ch, "\n\r   &Wtotal: %d&w\n\r", count );

	    }

	}

    return;

}

void do_sn_shortcut( CHAR_DATA *ch, char *argument )
{
	char buf [MAX_STRING_LENGTH];
	char arg[MAX_INPUT_LENGTH];
	char arg1[MAX_INPUT_LENGTH];
	ROOM_INDEX_DATA	*location;
	AREA_DATA *area;
	PLANET_DATA *planet;
	OBJ_DATA *obj;
	bool ch_snippet;
	int chance, roll, margin, count = 0;
	int destinationid;

	argument = one_argument( argument , arg );
	argument = one_argument( argument , arg1 );

	location = ch->in_room;
	planet = ch->in_room->area->planet;
	area = ch->in_room->area;

	if ( IS_NPC(ch) || !ch->pcdata )
	   {
	       send_to_char ( "huh?\n\r" , ch );
	       return;
	   }


		if ( ch->position <= POS_SLEEPING )
		{
			send_to_char( "> you are hibernating\n\r" , ch );
			return;
		}

		if ( ch->fighting )
		{
			send_to_char( "> shortcut cannot be used in combat\n\r", ch );
			return;
		}

		if ( IS_SET( ch->in_room->room_flags, ROOM_SAFE ) )
		{
			send_to_char( "> &Rshortcut cannot be used in safe nodes&w\n\r", ch );
			return;
		}

		if ( ch->in_room->vnum <= 20 )
		{
			send_to_char( "> &Ryou cannot use this in the tutorial&w\n\r", ch );
			return;
		}


		if ( arg[0] == '\0' )
		{
			send_to_char("> &Rsyntax: shortcut [type] [nodeid]&w\n\r", ch);
			send_to_char("> &Wconnect to specified node&w\n\r", ch);
			send_to_char("> &Woptions: fw (firewall)&w\n\r", ch);
			return;
		}

		if ( arg1[0] == '\0' )
		{
			send_to_char("> &Rerror: no nodeid specified&w\n\r", ch);
			return;
		}

		ch_snippet = FALSE;

		for (obj = ch->last_carrying; obj; obj = obj->prev_content) {
			if (obj->item_type == ITEM_SNIPPET && !strcmp(obj->name,
					"shortcut")) {
				ch_snippet = TRUE;

				obj->value[0] -= 1;

				if (obj->value[0] < 1)
				{
				separate_obj(obj);
				obj_from_char(obj);
				extract_obj( obj );
				send_to_char("> &Rshortcut application has expired&w\n\r", ch);
				}
			}

		}

	if (!ch_snippet) {
		send_to_char("> &Rshortcut application needed&w\n\r", ch);
		return;
	}

	WAIT_STATE( ch, skill_table[gsn_propaganda]->beats );

    sprintf(buf,"> &y%s uses a shortcut application", ch->name);
    act(AT_WHITE,buf, ch, NULL, NULL, TO_ROOM);

	chance = IS_NPC(ch) ? ch->top_level
			: (int) (ch->pcdata->learned[gsn_spacecraft]);

	if ( location->level != 0 )
	chance -= ( location->level * 10 );

	roll = number_percent();

	if ( roll >= chance )
	{
		send_to_char("> &Ryou failed to connect&w\n\r", ch);
		return;
	}

	margin = chance - roll;

	if ( !str_cmp( arg, "fw" ) )
	{
		for ( location = planet->area->first_room ; location ; location = location->next_in_area )
		{
			if ( IS_SET( location->room_flags, ROOM_BARRACKS ) )
			{

				destinationid = atoi(arg1);
				if ( location->vnum == destinationid )
				count++;

			}
		}

	    if ( !count )
	    {
		set_char_color( AT_BLOOD, ch);
	        send_to_char( "> &Rdestination not found&w\n\r", ch );
	    }
	    else
	    {
	    	send_to_char( "> &Gyou connect to the destination node&w\n\r", ch );
			char_from_room( ch );
			char_to_room( ch, get_room_index( destinationid ) );
			do_look( ch, "auto" );
	    }

	}

    return;

}

void do_sn_checkout( CHAR_DATA *ch, char *argument )
{
	char buf [MAX_STRING_LENGTH];
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	ROOM_INDEX_DATA	*location;
	AREA_DATA *area;
	PLANET_DATA *planet;
	OBJ_DATA *obj;
	bool ch_snippet;
	int chance, roll, margin;

	location = ch->in_room;
	planet = ch->in_room->area->planet;
	area = ch->in_room->area;

	argument = one_argument( argument, arg );

	if ( IS_NPC(ch) || !ch->pcdata )
	   {
	       send_to_char ( "huh?\n\r" , ch );
	       return;
	   }


		if ( ch->position <= POS_SLEEPING )
		{
			send_to_char( "> you are hibernating\n\r" , ch );
			return;
		}

		if ( ch->fighting )
		{
			send_to_char( "> checkout cannot be used in combat\n\r", ch );
			return;
		}

		if ( IS_SET( ch->in_room->room_flags, ROOM_SAFE ) )
		{
			send_to_char( "> &Rcheckout cannot be used in safe nodes&w\n\r", ch );
			return;
		}


		if ( arg[0] == '\0' )
		{
			send_to_char("> &Rsyntax: checkout [program]&w\n\r", ch);
			send_to_char("> &Wgain resource from program&w\n\r", ch);
			return;
		}

	    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
	    {
		send_to_char( "> &Rthat program is not here&w\n\r", ch);
		return;
	    }

	    if ( !IS_NPC(victim) )
	    {
		send_to_char( "> &Rnot on players&w\n\r", ch );
		return;
	    }

	    if ( victim->pIndexData->vnum != 56 && victim->pIndexData->vnum != 57 && victim->pIndexData->vnum != 58
	    		&& victim->pIndexData->vnum != 59)
	    {
		send_to_char( "> &Rthat is not a program&w\n\r", ch );
		return;
	    }

		ch_snippet = FALSE;

		for (obj = ch->last_carrying; obj; obj = obj->prev_content) {
			if (obj->item_type == ITEM_SNIPPET && !strcmp(obj->name,
					"checkout")) {
				ch_snippet = TRUE;

				obj->value[0] -= 1;

				if (obj->value[0] < 1)
				{
				separate_obj(obj);
				obj_from_char(obj);
				extract_obj( obj );
				send_to_char("> &Rcheckout application has expired&w\n\r", ch);
				}

			}

		}

	if (!ch_snippet) {
		send_to_char("> &Rcheckout application needed&w\n\r", ch);
		return;
	}

	WAIT_STATE( ch, skill_table[gsn_propaganda]->beats );

    sprintf(buf,"> &y%s uses a checkout application", ch->name);
    act(AT_WHITE,buf, ch, NULL, NULL, TO_ROOM);

	chance = IS_NPC(ch) ? ch->top_level
			: (int) (ch->pcdata->learned[gsn_spacecraft]);

	if ( location->level != 0 )
	chance -= ( location->level * 10 );

	roll = number_percent();

	if ( roll >= chance )
	{
		send_to_char("> &Ryou failed the checkout&w\n\r", ch);
		return;
	}

	margin = chance - roll;

    sh_int decrease = victim->top_level / 20;
	int chargain;

	if ( decrease == 1 )
		chargain = 1;
	else if ( decrease == 2 )
		chargain = 2;
	else if ( decrease == 3 )
		chargain = 4;
	else if ( decrease == 4 )
		chargain = 8;
	else if ( decrease == 5 )
		chargain = 16;
	else
		chargain = 1;

	switch(victim->pIndexData->vnum) {

	default:
		break;

	case 56:
		ch->pcdata->rentertain += chargain;

  	 victim->in_room->area->planet->entertain_count = victim->in_room->area->planet->entertain_count - decrease;
  	 victim->in_room->area->planet->entertain_count = UMAX( victim->in_room->area->planet->entertain_count , 0 );
		break;

	case 57:
		ch->pcdata->rmultimedia += chargain;
  	 victim->in_room->area->planet->multimedia_count = victim->in_room->area->planet->multimedia_count - decrease;
  	 victim->in_room->area->planet->multimedia_count = UMAX( victim->in_room->area->planet->multimedia_count , 0 );
		break;

	case 58:
		ch->pcdata->rfinance += chargain;
  	 victim->in_room->area->planet->finance_count = victim->in_room->area->planet->finance_count - decrease;
  	 victim->in_room->area->planet->finance_count = UMAX( victim->in_room->area->planet->finance_count , 0 );
		break;

	case 59:
		ch->pcdata->rproduct += chargain;
  	 victim->in_room->area->planet->product_count = victim->in_room->area->planet->product_count - decrease;
  	 victim->in_room->area->planet->product_count = UMAX( victim->in_room->area->planet->product_count , 0 );
		break;
	}

	ch_printf( ch , "> &Gyou gain %d repos from %s&w\n\r", chargain, victim->short_descr);
	act( AT_PLAIN, "> $n checks out a program", ch,
			NULL, argument , TO_ROOM );

	extract_char( victim, TRUE );

    return;

}
