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
//			      send_to_char( "> &R[&YALERT&R]&W enemy activity! jackhammer used!\n\r", d->character );
//			      ch_printf( d->character, "> &R[&YALERT&R]&W in system: %s&w\n\r", ch->in_room->area->planet->name );
//
//		      }
//		   }

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
		echo_to_room(AT_RED, ch->in_room, buf);

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

	DESCRIPTOR_DATA *d;
	OBJ_DATA *obj;
	EXIT_DATA *xit, *texit;
	int edir;
	ROOM_INDEX_DATA *location;
	char buf[MAX_STRING_LENGTH];
	PLANET_DATA *planet;
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