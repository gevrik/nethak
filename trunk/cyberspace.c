#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include "mud.h"

bool	check_parse_name	args( ( char *name ) );
void	write_clan_list	args( ( void ) );

extern char * const cargo_names[CARGO_MAX];

void do_layout( CHAR_DATA *ch, char *argument )
{

	CLAN_DATA * clan;
	OBJ_DATA *obj;
	OBJ_DATA *obj_next;
	ROOM_INDEX_DATA * location;

	if ( IS_NPC(ch) || !ch->pcdata )
		return;

	if ( !ch->desc )
		return;

	switch( ch->substate )
	{
	default:
		break;
	case SUB_ROOM_DESC:
		location = ch->dest_buf;
		if ( !location )
		{
			bug( "landscape: sub_room_desc: NULL ch->dest_buf", 0 );
			location = ch->in_room;
		}
		STRFREE( location->description );
		location->description = copy_buffer( ch );
		stop_editing( ch );
		ch->substate = ch->tempnum;
		if ( strlen( location->description ) > 50 ) {
			learn_from_success( ch , gsn_landscape );
			learn_from_success( ch , gsn_landscape );
		}
		else
		{
			send_to_char( "That node description is too short.\n\r", ch );
			send_to_char( "Your skill level diminishes with your laziness.\n\r", ch );
			if ( ch->pcdata->learned[gsn_landscape] > 50 )
				ch->pcdata->learned[gsn_landscape] -= 1;
		}
		SET_BIT( location->area->flags , AFLAG_MODIFIED );
		for ( obj = ch->in_room->first_content; obj; obj = obj_next )
		{
			obj_next = obj->next_content;
			extract_obj( obj );
		}
		echo_to_room( AT_WHITE, location , "You begin coding..." );
		return;
	}

	location = ch->in_room;
	clan = ch->pcdata->clan;

	if ( IS_SET( ch->in_room->room_flags , ROOM_NOPEDIT ) )
	{

		if ( !clan )
		{
			send_to_char( "You need to be part of an clan before you can do that!\n\r", ch );
			return;
		}

		if ( (ch->pcdata && ch->pcdata->bestowments
				&&    is_name("build", ch->pcdata->bestowments))
				|| nifty_is_name( ch->name, clan->leaders  ) )
			;
		else
		{
			send_to_char( "Your clan hasn't given you permission to edit their complexes!\n\r", ch );
			return;
		}

		if ( !location->area || !location->area->planet ||
				clan != location->area->planet->governed_by  )
		{
			send_to_char( "You may only code nodes in complexes that your clan controls!\n\r", ch );
			return;
		}

	}



	if( strcmp(location->owner, ch->name) )
	{
		send_to_char( "&R> this is not your node&w\n\r", ch );
		return;
	}


	if ( IS_SET( location->room_flags , ROOM_NOPEDIT ) )
	{
		send_to_char( "> &Rsorry, but you may not edit this room\n\r", ch );
		return;
	}

	SET_BIT( ch->in_room->area->flags , AFLAG_MODIFIED );

	ch->substate = SUB_ROOM_DESC;
	ch->dest_buf = location;
	start_editing( ch, location->description );
	return;

}


void do_buyskill( CHAR_DATA *ch, char *argument )
{

	char arg[MAX_INPUT_LENGTH];
	//CHAR_DATA *victim;
	int cost = 5000;
	int sn;

	one_argument( argument, arg );

	if ( arg[0] == '\0' )
	{
		send_to_char( "> syntax: buyskill <skill>\n\r",	ch );
		send_to_char( "> cost: 5,000c\n\r",	ch );
		send_to_char( "> skills: aid, backstab, blades, blasters, circle, codeapp, codeblade, codeblaster, codecontainer, codedef, codemed, codeshield, codeutil,"
				" damboost, disarm, disguise, dodge, dualwield, firstaid, gouge, hide, kick, parry, peek, picklock, poisonmod, postguard, propaganda, quicktalk,"
				" reinforcements, rescue, second attack, sneak, steal, throw, trace, inquire, slicebank\n\r",	ch );
		return;
	}

	if ( ch->gold < cost )
	{
		send_to_char( "> insufficient funds\n\r", ch );
		return;
	}


	sn   = 0;
	if ( ( sn = skill_lookup( arg ) ) < 0 )
	{
		send_to_char( "> no such skillsoft\n\r", ch );
		return;
	}

	if ( ch->pcdata->learned[sn] < 50 && ch->pcdata->learned[sn] != 0 )
	{
		send_to_char( "> upgrading to level 50\n\r", ch );
		ch->pcdata->learned[sn] = 50;
		return;
	}

	if ( ch->pcdata->learned[sn] >= 50 )
	{
		send_to_char( "> you already know that skill\n\r", ch );
		return;
	}

	if ( ch->pcdata->num_skills >= get_curr_int(ch) )
	{
		send_to_char( "> not intelligent enough yet\n\r", ch );
		return;
	}

	if ( (ch->pcdata->num_skills - ch->pcdata->adept_skills) >= 10 )
	{
		send_to_char( "> you need to perfect another skill first\n\r", ch );
		return;
	}

	ch->gold     -= cost;
	ch->pcdata->num_skills++;	
	ch->pcdata->learned[sn] = 50;

	send_to_char( "> skillsoft learned\n\r", ch );

	return;
}

void do_homerecall( CHAR_DATA *ch, char *argument )
{

	if ( ch->fighting )
	{
		send_to_char( "> you to try flee from combat\n\r", ch );
		do_flee( ch, "" );
		return;
	}

	if ( IS_SET( ch->in_room->room_flags, ROOM_ARENA ) )
	{
		send_to_char( "> &Rfinish the current match first&w\n\r", ch );
		return;
	}
	
	if ( ch->in_room->sector_type == SECT_RAINFOREST )
	{
		send_to_char( "> &Rfind a safehouse in the construct first&w\n\r", ch );
		return;
	}

	if ( get_age(ch) > 20 )
	{
		if ( !IS_SET( ch->in_room->room_flags, ROOM_SAFE ) && !IS_SET( ch->in_room->room_flags, ROOM_NO_MOB )
				&& !IS_SET( ch->in_room->room_flags, ROOM_BANK ) && !IS_SET( ch->in_room->room_flags, ROOM_HOTEL)
				&& !IS_SET( ch->in_room->room_flags, ROOM_CAN_LAND ) && !IS_SET( ch->in_room->room_flags, ROOM_PUBLICIO)
				&& !IS_SET( ch->in_room->room_flags, ROOM_EMPLOYMENT) )
		{
			send_to_char( "> &Rfind a safe node to connect home&w\n\r", ch );
			return;
		}
	}

	if ( ch->in_room->vnum <= 20 )
	{
		send_to_char( "> &Ryou cannot use this command in the tutorial&w\n\r", ch );
		return;
	}

	if( !ch->plr_home )
	{
		send_to_char( "> you connect to straylight\n\r", ch );
		act(AT_GREEN, "> $n connects to straylight", ch, NULL, NULL, TO_ROOM );
		char_from_room( ch );
		char_to_room( ch, get_room_index( ROOM_VNUM_STRAY ) );
		do_look( ch, "auto" );
		return;
	}

	send_to_char( "> you connect to your home node\n\r", ch );
	act(AT_GREEN, "> $n connects to their home node", ch, NULL, NULL, TO_ROOM );
	char_from_room( ch );    
	char_to_room( ch, ch->plr_home );
	do_look( ch, "auto" );

	return;

}

void do_homehall( CHAR_DATA *ch, char *argument )
{

	if ( ch->fighting )
	{
		send_to_char( "> you to try flee from combat\n\r", ch );
		do_flee( ch, "" );
		return;
	}

	if ( IS_SET( ch->in_room->room_flags, ROOM_ARENA ) )
	{
		send_to_char( "> &Rfinish the current match first&w\n\r", ch );
		return;
	}

	if ( ch->in_room->sector_type == SECT_RAINFOREST )
	{
		send_to_char( "> &Rfind a safehouse in the construct first&w\n\r", ch );
		return;
	}

	if ( !IS_SET( ch->in_room->room_flags, ROOM_SAFE ) && !IS_SET( ch->in_room->room_flags, ROOM_NO_MOB )
			&& !IS_SET( ch->in_room->room_flags, ROOM_BANK ) && !IS_SET( ch->in_room->room_flags, ROOM_HOTEL)
			&& !IS_SET( ch->in_room->room_flags, ROOM_CAN_LAND ) && !IS_SET( ch->in_room->room_flags, ROOM_PUBLICIO)
			&& !IS_SET( ch->in_room->room_flags, ROOM_EMPLOYMENT) )
	{
		send_to_char( "> &Rfind a safe node to connect to consumer review&w\n\r", ch );
		return;
	}

	send_to_char( "> you connect to consumer review\n\r", ch );
	act(AT_GREEN, "> $n connects to consumer review", ch, NULL, NULL, TO_ROOM );
	char_from_room( ch );
	char_to_room( ch, get_room_index( 11072 ) );
	do_look( ch, "auto" );

	return;

}

void do_homestray( CHAR_DATA *ch, char *argument )
{

	if ( ch->fighting )
	{
		send_to_char( "> you to try flee from combat\n\r", ch );
		do_flee( ch, "" );
		return;
	}

	if ( IS_SET( ch->in_room->room_flags, ROOM_ARENA ) )
	{
		send_to_char( "> &Rfinish the current match first&w\n\r", ch );
		return;
	}

	if ( ch->in_room->sector_type == SECT_RAINFOREST )
	{
		send_to_char( "> &Rfind a safehouse in the construct first&w\n\r", ch );
		return;
	}

	if ( !IS_SET( ch->in_room->room_flags, ROOM_SAFE ) && !IS_SET( ch->in_room->room_flags, ROOM_NO_MOB )
			&& !IS_SET( ch->in_room->room_flags, ROOM_BANK ) && !IS_SET( ch->in_room->room_flags, ROOM_HOTEL)
			&& !IS_SET( ch->in_room->room_flags, ROOM_CAN_LAND ) && !IS_SET( ch->in_room->room_flags, ROOM_PUBLICIO)
			&& !IS_SET( ch->in_room->room_flags, ROOM_EMPLOYMENT) )
	{
		send_to_char( "> &Rfind a safe node to connect to straylight&w\n\r", ch );
		return;
	}

	send_to_char( "> you connect to straylight\n\r", ch );
	act(AT_GREEN, "> $n connects to straylight", ch, NULL, NULL, TO_ROOM );
	char_from_room( ch );
	char_to_room( ch, get_room_index( ROOM_VNUM_STRAY ) );
	do_look( ch, "auto" );

	return;

}

void do_constructportal( CHAR_DATA *ch, char *argument )
{

	if ( ch->fighting )
	{
		send_to_char( "> &Ryou can't do that in combat&w\n\r", ch );
		return;
	}

	if ( IS_SET( ch->in_room->room_flags, ROOM_ARENA ) )
	{
		send_to_char( "> &Rfinish the current match first&w\n\r", ch );
		return;
	}

	if ( ch->in_room->sector_type == SECT_RAINFOREST )
	{
		send_to_char( "> &Rfind a safehouse in the construct first&w\n\r", ch );
		return;
	}

	if ( !IS_SET( ch->in_room->room_flags, ROOM_SAFE ) && !IS_SET( ch->in_room->room_flags, ROOM_NO_MOB )
			&& !IS_SET( ch->in_room->room_flags, ROOM_BANK ) && !IS_SET( ch->in_room->room_flags, ROOM_HOTEL)
			&& !IS_SET( ch->in_room->room_flags, ROOM_CAN_LAND ) && !IS_SET( ch->in_room->room_flags, ROOM_PUBLICIO)
			&& !IS_SET( ch->in_room->room_flags, ROOM_EMPLOYMENT) )
	{
		send_to_char( "> &Rfind a safe node to connect to your construct&w\n\r", ch );
		return;
	}

	if ( ch->pcdata->roomconstruct == 0 )
	{
		send_to_char( "> &Ryou do not have a construct yet&w\n\r", ch );
		send_to_char( "> &Rcreate one for 5,000c with MAKECONSTRUCT&w\n\r", ch );
		return;
	}

	send_to_char( "> you connect to your construct\n\r", ch );
	act(AT_GREEN, "> $n connects to their construct", ch, NULL, NULL, TO_ROOM );
	char_from_room( ch );
	char_to_room( ch, get_room_index( ch->pcdata->roomconstruct ) );
	do_look( ch, "auto" );

	return;

}

void do_connect( CHAR_DATA *ch, char *argument )
{
	char arg[MAX_INPUT_LENGTH];
	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];

	PLANET_DATA * planet;
	ROOM_INDEX_DATA * room;
	ROOM_INDEX_DATA * in_room;
	bool rfound = FALSE;

	argument = one_argument( argument , arg );
	argument = one_argument( argument , arg1 );
	argument = one_argument( argument , arg2 );

	switch( ch->substate )
	{
	default:

		in_room = ch->in_room;
		room = find_location( ch, arg1 );


		if ( ( room = find_location( ch, arg1 ) ) == in_room )
		{

			send_to_char( "&RConnection already established.\n\r", ch );
			return;

		}

		if ( ch->fighting )
		{
			send_to_char( "> you cannot connect in combat\n\r", ch );
			return;
		}


		if ( !IS_SET( ch->in_room->room_flags, ROOM_CAN_LAND ) && !IS_SET( ch->in_room->room_flags, ROOM_PUBLICIO ) )
		{
			send_to_char( "&R> you need to be in an io node\n\r", ch );
			return;
		}

		if ( arg[0] == '\0' )
		{
			send_to_char( "> syntax: connect <system> <lobby>\n\r",	ch );
			send_to_char( "> specify <system>\n\r",	ch );
			return;
		}

		planet = get_planet( arg );

		if ( !planet )
		{
			send_to_char( "> no such system\n\r", ch );
			return;
		}

		if ( atoi(arg1) != '\0' )
		{
			for ( room = planet->area->first_room ; room ; room = room->next_in_area )
			{
				if ( ( IS_SET( room->room_flags , ROOM_CAN_LAND ) && ( atoi(arg1) == room->vnum ) ) || ( IS_SET( room->room_flags , ROOM_PUBLICIO ) && ( atoi(arg1) == room->vnum ) ) )
				{

					rfound = TRUE;

					break;
				}
			}
		}

		if ( !rfound )
		{
			send_to_char("&C> specify lobby\n\r",ch);
			ch_printf( ch , "> choices for %s:\n\r\n\r", planet->name);
			for ( room = planet->area->first_room ; room ; room = room->next_in_area )
				if ( IS_SET( room->room_flags, ROOM_PUBLICIO ) )
					ch_printf( ch , "%-15d  %s &C[&Gpublic&C]\n\r", room->vnum, room->name);
			for ( room = planet->area->first_room ; room ; room = room->next_in_area )
				if ( IS_SET( room->room_flags, ROOM_CAN_LAND ) )
					ch_printf( ch , "%-15d  %s &C[&Rprot&C]\n\r", room->vnum, room->name);


			return;
		}

		add_timer( ch, TIMER_DO_FUN, 2,
				do_connect, 1 );
		send_to_char( "> you begin to connect\n\r", ch );
		ch->dest_buf = str_dup( arg );
		ch->dest_buf_2 = str_dup( arg1 );
		ch->dest_buf_3 = str_dup( arg2 );
		//send_to_char( "DEBUG after dest_buf\n\r", ch );
		return;

	case 1:
		//send_to_char( "DEBUG in case 1\n\r", ch );
		if ( !ch->dest_buf )
		{
			send_to_char( "> your connection was interrupted\n\r", ch );
			bug( "do_connect: dest_buf NULL", 0 );
			return;
		}
		if ( !ch->dest_buf_2 )
		{
			send_to_char( "> your connection was interrupted\n\r", ch );
			bug( "do_connect: dest_buf_2 NULL", 0 );
			return;
		}
		if ( !ch->dest_buf_3 )
		{
			send_to_char( "> your connection was interrupted\n\r", ch );
			bug( "do_connect: dest_buf_3 NULL", 0 );
			return;
		}
		//send_to_char( "DEBUG after debugs in case 1\n\r", ch );

		strcpy( arg, ch->dest_buf );
		DISPOSE( ch->dest_buf );
		strcpy( arg1, ch->dest_buf_2 );
		DISPOSE( ch->dest_buf_2 );
		strcpy( arg2, ch->dest_buf_3 );
		DISPOSE( ch->dest_buf_3 );

		send_to_char( "&Y> connection check complete\n\r", ch );
		break;

	case SUB_TIMER_DO_ABORT:
		DISPOSE( ch->dest_buf );
		DISPOSE( ch->dest_buf_2 );
		DISPOSE( ch->dest_buf_3 );
		ch->substate = SUB_NONE;
		send_to_char( "> you stop connecting\n\r", ch );
		return;
	}

	//send_to_char( "DEBUG: before substate\n\r", ch );

	ch->substate = SUB_NONE;

	in_room = ch->in_room;

	if ( ( room = find_location( ch, arg1 ) ) == NULL )
	{

		send_to_char( "&R> connection could not be established.\n\r", ch );
		return;

	}

	if ( room->seccode != 0  && atoi(arg2) != room->seccode)
	{
		send_to_char( "&R> invalid node security code&w\n\r", ch );
		return;
	}

	send_to_char( "&Y> connection established\n\r\n\r", ch);

	ch->regoto = ch->in_room->vnum;

	act(AT_GREEN, "> $n connects to another system", ch, NULL, NULL, TO_ROOM );

	char_from_room( ch );

	char_to_room( ch, room );

	do_look( ch, "auto" );

	return;
}

void do_coding( CHAR_DATA *ch, char *argument )
{
	OBJ_INDEX_DATA *pObjIndex = NULL;
	OBJ_DATA * obj = NULL;
	//PLANET_DATA * planet;
	ROOM_INDEX_DATA * in_room;

	switch( ch->substate )
	{
	default:

		in_room = ch->in_room;

		if ( ch->fighting )
		{
			send_to_char( "&R> you cannot code in combat&w\n\r", ch );
			return;
		}


		if ( !IS_SET( ch->in_room->room_flags, ROOM_RESTAURANT ) )
		{
			send_to_char( "&R> you need to be in a development node\n\r", ch );
			return;
		}

		add_timer( ch, TIMER_DO_FUN, 10,
				do_coding, 1 );
		send_to_char( "> you begin to code\n\r", ch );
		return;

	case 1:

		send_to_char( "&Y> coding complete&w\n\r", ch );
		break;

	case SUB_TIMER_DO_ABORT:
		ch->substate = SUB_NONE;
		send_to_char( "&R> you stop coding&w\n\r", ch );
		return;
	}

	ch->substate = SUB_NONE;

	send_to_char( "&Y> you coded:\n\r\n\r", ch);

	pObjIndex = get_obj_index( number_range( OBJ_VNUM_FIRST_FABRIC , OBJ_VNUM_LAST_FABRIC  ) );

	obj = create_object(pObjIndex, 1);
	SET_BIT(obj->extra_flags, ITEM_INVENTORY);
	obj = obj_to_char(obj, ch);

	ch_printf( ch , "%s:\n\r\n\r", obj->name);

	return;
}


void do_decompile( CHAR_DATA *ch, char *argument )
{
	char arg[MAX_INPUT_LENGTH];
	OBJ_INDEX_DATA *pObjIndex = NULL;
	int level, chance;
	bool checksew, checkfab;
	OBJ_DATA *obj;
	OBJ_DATA *material;
	int cost;

	argument = one_argument( argument, arg );
	//strcpy( arg2 , argument );


	switch( ch->substate )
	{
	default:



		if ( str_cmp( arg, "def" )
				&& str_cmp( arg, "blaster" )
				&& str_cmp( arg, "blade" )
				&& str_cmp( arg, "function" )
				&& str_cmp( arg, "util" )
				&& str_cmp( arg, "patch" )
				&& str_cmp( arg, "app" )
				&& str_cmp( arg, "snippet" )
				&& str_cmp( arg, "datacube" ))
		{
			send_to_char( "&R> you cannot decompile that, try:\n\r&w", ch);
			send_to_char( "> def, blaster, blade, function,\n\r", ch);
			send_to_char( "> util, app, patch, snippet or datacube\n\r", ch);
			return;
		}


		if ( str_cmp( arg, "datacube" ) && str_cmp( arg, "snippet" ) )
		{
			if ( !IS_SET( ch->in_room->room_flags, ROOM_RESTAURANT ) )
			{
				send_to_char( "> &Ryou need to be in a coding node\n\r", ch );
				return;
			}

		}

		checksew = FALSE;
		checkfab = FALSE;

		for ( obj = ch->last_carrying; obj; obj = obj->prev_content )
		{
			if (obj->item_type == ITEM_FOOD || obj->item_type == ITEM_RESOURCE)
				checkfab = TRUE;
			if (obj->item_type == ITEM_OVEN)
				checksew = TRUE;
		}

		if ( !checkfab )
		{
			send_to_char( "&R> you need some sort of file\n\r", ch);
			return;
		}

		if ( !checksew )
		{
			send_to_char( "&R> you need a compiler\n\r", ch);
			return;
		}

		chance = IS_NPC(ch) ? ch->top_level
				: (int) (ch->pcdata->learned[gsn_spacecraft]);
		if ( number_percent( ) < chance )
		{
			send_to_char( "> &Gyou begin the long process of decompiling\n\r", ch);
			act( AT_PLAIN, "> $n takes $s compiler as well as some code and begins to work", ch,
					NULL, argument , TO_ROOM );
			if ( !str_cmp( arg, "snippet" ) || !str_cmp( arg, "datacube" ) )
			add_timer ( ch , TIMER_DO_FUN , 1 , do_decompile , 1 );
			else
			add_timer ( ch , TIMER_DO_FUN , 1 , do_decompile , 1 );
			ch->dest_buf = str_dup(arg);
			return;
		}
		send_to_char("> &Ryou cannot figure out what to do\n\r",ch);
		learn_from_failure( ch, gsn_spacecraft );
		return;

	case 1:
		if ( !ch->dest_buf )
			return;
		strcpy(arg, ch->dest_buf);
		DISPOSE( ch->dest_buf);
		break;

	case SUB_TIMER_DO_ABORT:
		DISPOSE( ch->dest_buf );
		ch->substate = SUB_NONE;
		send_to_char("> &Ryou are interrupted and fail to finish your work\n\r", ch);
		return;
	}

	ch->substate = SUB_NONE;

	level = IS_NPC(ch) ? ch->top_level : (int) (ch->pcdata->learned[gsn_spacecraft]);

	checksew = FALSE;
	checkfab = FALSE;

	for ( obj = ch->last_carrying; obj; obj = obj->prev_content )
	{
		if (obj->item_type == ITEM_OVEN)
			checksew = TRUE;
		if ((obj->item_type == ITEM_FOOD && checkfab == FALSE) || (obj->item_type == ITEM_RESOURCE && checkfab == FALSE))
		{
			checkfab = TRUE;
			separate_obj( obj );
			obj_from_char( obj );
			material = obj;
			cost = obj->cost;
		}
	}

	if ( ( !checkfab ) || ( !checksew ) )
	{
		send_to_char( "> &Ryou could not decompile anything useful\n\r", ch);
		learn_from_failure( ch, gsn_spacecraft );
		return;
	}

	int afumble = number_range(1,10);

	if ( afumble < 2 )
	{
		send_to_char( "> &Ryou could not decompile anything useful\n\r", ch);
		learn_from_failure( ch, gsn_spacecraft );
		return;
	}

	obj = material;

	send_to_char( "> &Yyou decompiled:&w\n\r\n\r", ch);

	if ( !str_cmp( arg, "def" ) )
	{
		int decklevel = ( ch->pcdata->learned[gsn_spacecraft] / 10 );
		pObjIndex = get_obj_index( number_range( OBJ_VNUM_FIRST_FABRIC , ( 89 + decklevel) ) );
		ch_printf( ch , "%d\n\r\n\r", decklevel);
	}
	else if ( !str_cmp( arg, "blaster" ) )
	{
		pObjIndex = get_obj_index( 31 );
	}
	else if ( !str_cmp( arg, "blade" ) )
	{
		pObjIndex = get_obj_index( 34 );
	}
	else if ( !str_cmp( arg, "function" ) )
	{

		int anumber = number_range(0,1);
		if ( anumber == 0 )
			pObjIndex = get_obj_index( 37 );
		else
			pObjIndex = get_obj_index( 38 );

	}
	else if ( !str_cmp( arg, "util" ) )
	{
		pObjIndex = get_obj_index( 35 );
	}
	else if ( !str_cmp( arg, "patch" ) )
	{

		int anumber = number_range(0,1);
		if ( anumber == 0 )
			pObjIndex = get_obj_index( 61 );
		else
			pObjIndex = get_obj_index( 28 );
		//pObjIndex = get_obj_index( 61 );
	}
	else if ( !str_cmp( arg, "app" ) )
	{
		pObjIndex = get_obj_index( 59 );
	}
	else if ( !str_cmp( arg, "datacube" ) )
	{
		pObjIndex = get_obj_index( 103 );
	}
	else if ( !str_cmp( arg, "snippet" ) )
	{
		//int randomamount = number_range(1, 2);
		int amountsnips = cost;
		ch->snippets = ch->snippets + amountsnips;
		ch_printf( ch, "%d snippets\n\r", amountsnips );
		send_to_char( "> &Gyou finish decompiling&w\n\r", ch);
		act( AT_PLAIN, "> $n finishes decompiling", ch, NULL, argument , TO_ROOM );
		return;
	}

	obj = create_object(pObjIndex, 1);
	SET_BIT(obj->extra_flags, ITEM_INVENTORY);
	obj = obj_to_char(obj, ch);

	ch_printf( ch , "%s\n\r\n\r", obj->name);

	send_to_char( "> &Gyou finish decompiling&w\n\r", ch);
	act( AT_PLAIN, "> $n finishes decompiling", ch, NULL, argument , TO_ROOM );

	if ( !str_cmp( arg, "snippet" ) )
	{

		if ( number_percent() == 23 )
		{
			send_to_char( "> you feel more intelligent than before\n\r", ch );
			ch->perm_int++;
			ch->perm_int = UMIN( ch->perm_int , 25 );
		}

		learn_from_success( ch, gsn_spacecraft );
	}
	else
	{
		int snipchance = number_range(1, 5);
		if ( snipchance <= 2 )
		learn_from_success( ch, gsn_spacecraft );
	}
	return;
}


void do_renamenode( CHAR_DATA *ch, char *argument )
{
	ROOM_INDEX_DATA	*location;
	int cost = 100;

	location = ch->in_room;

	if( strcmp(location->owner, ch->name) )
	{
		send_to_char( "&R> this is not your node&w\n\r", ch );
		return;
	}

	if ( ch->gold < cost )
	{
		send_to_char( "> &Rinsufficient funds [100c needed]&w\n\r", ch );
		return;
	}

	if ( argument[0] == '\0' )
	{
		send_to_char( "> set the node name. costs 100c.\n\r", ch );
		send_to_char( "> a very brief single line node description.\n\r", ch );
		send_to_char( "> syntax: rename <name>\n\r", ch );
		return;
	}

	if ( !check_parse_name( argument ) )
	{
		send_to_char( "> &Rinvalid name&w\n\r", ch );
		return;
	}

	ch->gold     -= cost;

	STRFREE( location->name );
	location->name = STRALLOC( argument );
	send_to_char( "> &Gnode name set&w [100c spent]\n\r", ch);
	SET_BIT( ch->in_room->area->flags , AFLAG_MODIFIED );
	return;

}

void do_securenode( CHAR_DATA *ch, char *argument )
{
	ROOM_INDEX_DATA	*location;
	int chance;

	location = ch->in_room;

	if ( !argument || argument[0] == '\0' )
	{
		send_to_char( "sets the security code for this node\n\r", ch );
		send_to_char( "syntax: secure <seccode>\n\r", ch );
		return;
	}

	if( !IS_IMMORTAL(ch) )
	{
		if ( IS_SET( ch->in_room->room_flags , ROOM_NOPEDIT ) )
		{
			send_to_char( "> you may not edit this node\n\r", ch );
			return;
		}
	}

	if( strcmp(location->owner, ch->name) )
	{
		send_to_char( "&R> this is not your node&w\n\r", ch );
		return;
	}

	if ( ch->gold < 1000 )
	{
		send_to_char( "> &Rinsufficient funds [1000 needed]&w\n\r", ch );
		return;
	}

	if ( !IS_SET( ch->in_room->room_flags, ROOM_CAN_LAND ) )
	{
		send_to_char( "&R> you need to be in an io node\n\r", ch );
		return;
	}

	if ( IS_SET( ch->in_room->room_flags, ROOM_PUBLICIO ) )
	{
		send_to_char( "&R> this is a public io node\n\r", ch );
		return;
	}

	chance = (int) (ch->pcdata->learned[gsn_landscape]);
	if ( number_percent( ) > chance )
	{
		send_to_char( "> &Ryou fail. try again!&w\n\r", ch );
		return;
	}

	ch->gold -= 1000;

	location->seccode = atoi(argument);
	send_to_char( "> &Gcode set. 1000c spent.&w\n\r", ch );

	SET_BIT( ch->in_room->area->flags , AFLAG_MODIFIED );

	learn_from_success( ch , gsn_landscape );
	return;

}

void do_examineobject( CHAR_DATA *ch, char *argument )
{
	char arg[MAX_INPUT_LENGTH];
	AFFECT_DATA *paf;
	OBJ_DATA *obj;

	one_argument( argument, arg );

	if ( arg[0] == '\0' )
	{
		send_to_char( "> &Yspecify object&w\n\r", ch );
		return;
	}
	if ( arg[0] != '\'' && arg[0] != '"' && strlen(argument) > strlen(arg) )
		strcpy( arg, argument );

	if ( (obj = find_obj(ch, arg, FALSE)) == NULL )
	{

		return;
	}

	ch_printf( ch, "> &Gname:&W %s&w\n\r", obj->short_descr );
	ch_printf( ch, "> &Gtype:&W %s&w  &Gsize:&W %d/%d&w\n\r", item_type_name( obj ), obj->weight, get_obj_weight( obj ) );
	ch_printf( ch, "> &Gcost:&W %d&w\n\r", obj->cost );

	if ( obj->value[3] == WEAPON_BLASTER )
	{

		ch_printf( ch, "> &Gcond:&W %d  &Gldmg:&W %d  &Ghdam:&W %d\n\r", obj->value[0], obj->value[1], obj->value[2] );

		for ( paf = obj->first_affect; paf; paf = paf->next )
			ch_printf( ch, "> &Yaffects %s by %d (extra)&w\n\r",
					affect_loc_name( paf->location ), paf->modifier );

		for ( paf = obj->pIndexData->first_affect; paf; paf = paf->next )
			ch_printf( ch, "> &Yaffects %s by %d&w\n\r",
					affect_loc_name( paf->location ), paf->modifier );
	}

	if ( obj->value[3] == WEAPON_VIBRO_BLADE )
	{

		ch_printf( ch, "> &Gcond:&W %d  &Gldmg:&W %d  &Ghdam:&W %d\n\r", obj->value[0], obj->value[1], obj->value[2] );

		for ( paf = obj->first_affect; paf; paf = paf->next )
			ch_printf( ch, "> &Yaffects %s by %d (extra)&w\n\r",
					affect_loc_name( paf->location ), paf->modifier );

		for ( paf = obj->pIndexData->first_affect; paf; paf = paf->next )
			ch_printf( ch, "> &Yaffects %s by %d&w\n\r",
					affect_loc_name( paf->location ), paf->modifier );
	}

	if ( obj->item_type == ITEM_ARMOR )
	{

		sh_int defencebonus = (obj->value[0] * 3);

		ch_printf( ch, "> &Gcond:&W %d/%d&w\n\r", obj->value[0], obj->value[1] );
		ch_printf( ch, "> &Gdefense:&W %d&w\n\r", defencebonus );
		ch_printf( ch, "> &Gcoder:&W %s&w\n\r", obj->description );
		
	}

	if ( obj->item_type == ITEM_CONTAINER )
	{

		ch_printf( ch, "> &Gcond:&W %d/10&w\n\r", obj->value[3] );
		ch_printf( ch, "> &Gslots:&W %d&w\n\r", obj->value[0] );

	}
	
		if ( obj->item_type == ITEM_MEDPAC )
	{

		ch_printf( ch, "> &Gcharges:&W %d&w\n\r", obj->value[0] );

	}

		if ( obj->item_type == ITEM_SNIPPET )
	{

			if ( nifty_is_name("anchor", obj->name) || !strcmp(obj->name, "audit" ) || !strcmp(obj->name, "shortcut") || !strcmp(obj->name, "checkout") ) {
					ch_printf( ch, "> &Gcharges:&W %d&w\n\r", obj->value[0] );
			}
	}

	int anumber = number_range(1,100);
	if ( anumber == 23 )
		learn_from_success( ch , gsn_spacecraft );

	return;
}

void do_foundorg( CHAR_DATA *ch, char *argument )
{
	char filename[256];
	CLAN_DATA *clan;
	bool found;
	int cost = 50000;

	if ( !argument || argument[0] == '\0' )
	{
		send_to_char( "> &Ysyntax: foundorg <org name>&w\n\r", ch );
		send_to_char( "> note: you need 50.000 credits\n\r", ch );
		return;
	}

	if ( ch->gold < cost )
	{
		send_to_char( "> &Rinsufficient funds [50.000c needed]&w\n\r", ch );
		return;
	}

	if ( !check_parse_name( argument ) )
	{
		send_to_char( "> &Rinvalid organization name&w\n\r", ch );
		return;
	}

	if ( ch->pcdata->clan )
	{
		send_to_char( "> &Ryou already belong to an organization!&w\n\r", ch );
		return;
	}

	found = FALSE;
	sprintf( filename, "%s%s", CLAN_DIR, strlower(argument) );

	CREATE( clan, CLAN_DATA, 1 );
	LINK( clan, first_clan, last_clan, next, prev );
	clan->name		= STRALLOC( argument );
	clan->description	= STRALLOC( "" );
	clan->leaders	= STRALLOC( ch->name );
	clan->atwar		= STRALLOC( "" );
	clan->tmpstr	= STRALLOC( "" );
	clan->funds         = cost;
	clan->salary        = 0;
	clan->members       = 0;

	clan->filename = str_dup( argument );
	save_clan( clan );
	write_clan_list( );

	ch->gold     -= cost;

	send_to_char( "> &Gorganization created!&w\n\r", ch );

	clan->members++;

	ch->pcdata->clan = clan;
	STRFREE(ch->pcdata->clan_name);
	ch->pcdata->clan_name = QUICKLINK( clan->name );
	save_char_obj( ch );

	return;

}

void do_codesnippet( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_STRING_LENGTH], buf[MAX_STRING_LENGTH];
    OBJ_DATA *obj, *obj_next, *cont;
    bool checkcont = FALSE, checkchem = FALSE, checktool = FALSE;
    int chance, level, wearbit = get_wflag("hold");

    if( !IS_NPC(ch) && ch->pcdata->learned[gsn_spacecraft] == 0 )
    {
	 send_to_char("Huh?\n\r",ch);
	 return;
	}


    switch( ch->substate )
    {
    case 0:
 if( argument[0] == '\0' )
 {
     send_to_char("> syntax: codemed <medmod name>\n\r",ch);
     return;
 }

 for( obj = ch->first_carrying; obj; obj = obj->next_content )
 {
     if( obj->item_type == ITEM_CONTAINER && !checkcont )
     {
    	 if( obj->value[1] <= 0 )
         checkcont = TRUE;
         continue;
     }

     if( obj->item_type == ITEM_MIRROR && !checkchem )
     {
  checkchem = TRUE;
  continue;
     }

     if( obj->item_type == ITEM_TOOLKIT && !checktool )
  checktool = TRUE;
 }

 if( !checkcont )
 {
     send_to_char("> &Ryou need an empty container module&w\n\r",ch);
     return;
 }

 if( !checkchem )
 {
     send_to_char("> &Ryou need a wilderspace subroutine&w\n\r",ch);
    return;
 }

 if( !checktool )
 {
    send_to_char("> &Ryou need a devkit&w\n\r",ch);
    return;
 }

 chance = IS_NPC(ch) ? ch->top_level : ch->pcdata->learned[gsn_codemed];

 if( number_percent() <= chance )
 {
    send_to_char("> you begin making a med module\n\r",ch);
    act( AT_PLAIN, "> $n begins making a med module.", ch, NULL, NULL, TO_ROOM );
    ch->dest_buf = str_dup(argument);
    add_timer( ch, TIMER_DO_FUN, 2, do_codemed, 1 );
    return;
 }
 send_to_char("> &Ryou fail creating a mod module - try again&w\n\r",ch);
 return;

    case 1:
 if( !ch->dest_buf ) return;
 strcpy( arg, (const char*) ch->dest_buf );
 DISPOSE( ch->dest_buf );
 break;

    case SUB_TIMER_DO_ABORT:
 DISPOSE( ch->dest_buf );
 send_to_char("> &Ryour work is interrupted and you fail&w\n\r",ch);
        return;
    }

    for( obj = ch->first_carrying; obj; obj = obj_next )
    {
 obj_next = obj->next_content;

 if( obj->item_type == ITEM_CONTAINER && !checkcont )
 {
     if( obj->value[1] > 0 ) continue;
     cont = obj;
     checkcont = TRUE;
     continue;
 }

 if( obj->item_type == ITEM_MIRROR && !checkchem )
 {
     obj_from_char( obj );
     extract_obj( obj );
     checkchem = TRUE;
     continue;
 }

 if( obj->item_type == ITEM_TOOLKIT && !checktool )
     checktool = TRUE;
    }

    level = chance = IS_NPC(ch) ? ch->top_level : ch->pcdata->learned[gsn_codemed];

    if( number_percent() > chance || !checkcont || !checkchem || !checktool )
    {
		send_to_char( "> &Ryou fail to code the blade module&w\n\r", ch);
		return;
    }

    cont->item_type = ITEM_MEDPAC;
    cont->value[0] = level/10;
    sprintf( buf, "%s [medmod]", arg );
    STRFREE( cont->name );
    cont->name = STRALLOC( buf );
    STRFREE( cont->short_descr );
    cont->short_descr = STRALLOC( buf );
    //sprintf( buf, " was left here.");
    STRFREE( cont->description );
    cont->description = STRALLOC( buf );
    if( !CAN_WEAR( cont, 1 << wearbit ) )
    SET_BIT( cont->wear_flags, 1 << wearbit );

    send_to_char("&G> you hold up your new med module\n\r",ch);
    act( AT_PLAIN, "> $n finished their new med module",ch,NULL,NULL,TO_ROOM);


    learn_from_success( ch, gsn_codemed );

}

void do_sn_randomizer( CHAR_DATA *ch, char *argument )
{
	char buf [MAX_STRING_LENGTH];
	ROOM_INDEX_DATA	*location;
	int defmax;

	location = ch->in_room;

	if ( ch->snippets < 1 )
	{
		send_to_char( "> &Rinsufficient snippets [1 needed]&w\n\r", ch );
		return;
	}

	if ( argument[0] == '\0' )
		defmax = 100;
	else
		defmax = atoi(argument);

	int result = number_range(1, defmax);

	ch->snippets     -= 1;

	ch_printf( ch, "> &Grandomizer (1-%d):&W %d&w\n\r", defmax, result );
    sprintf(buf,"> &G%s's randomizer (1-%d):&W %d&w", ch->name, defmax, result);
    act(AT_WHITE,buf, ch, NULL, NULL, TO_ROOM);
	return;

}

void do_codeapp( CHAR_DATA *ch, char *argument )
{
	char arg[MAX_INPUT_LENGTH];
	char buf[MAX_STRING_LENGTH];
	char buf1[MAX_STRING_LENGTH];
	int level, chance;
	bool checksew, checkfab;
	OBJ_DATA *obj;
	OBJ_DATA *material;
	int cost;
	PLANET_DATA * planet;

	strcpy( arg , strlower(argument) );


	switch( ch->substate )
	{
	default:

//		if ( !IS_SET( ch->in_room->room_flags, ROOM_RESTAURANT ) )
//		{
//			send_to_char( "&R> you need to be in a coding node\n\r", ch );
//			return;
//		}

		if ( str_cmp( arg, "jackhammer" )
				&& str_cmp( arg, "krash" )
				&& str_cmp( arg, "spun" )
				&& str_cmp( arg, "reconstruct")
				&& str_cmp( arg, "dropline" )
				&& str_cmp( arg, "uninstall" )
				&& str_cmp( arg, "anchor" )
				&& str_cmp( arg, "audit" )
				&& str_cmp( arg, "shortcut" )
				&& str_cmp( arg, "checkout" )
				&& str_cmp( arg, "emp") )
		{
			send_to_char( "> &Ryou cannot code that app, try:\n\r&w", ch);
			send_to_char( "> jackhammer, krash, spun, reconstruct\n\r", ch);
			send_to_char( "> dropline, uninstall, anchor, audit\n\r", ch);
			send_to_char( "> shortcut, checkout, emp\n\r", ch);
			return;
		}

		if ( !str_cmp( arg, "jackhammer" ) )
		{
			cost = 500;
		}
		else if ( !str_cmp( arg, "krash" ) )
		{
			cost = 250;
		}
		else if ( !str_cmp( arg, "spun" ) )
		{
			cost = 25;
		}
		else if ( !str_cmp( arg, "reconstruct" ) )
		{
			cost = 50;
		}
		else if ( !str_cmp( arg, "dropline" ) )
		{
			cost = 10;
		}
		else if ( !str_cmp( arg, "uninstall" ) )
		{
			cost = 1000;
		}
		else if ( !str_cmp( arg, "checkout" ) )
		{
			cost = 10;
		}
		else if ( !str_cmp( arg, "emp" ) )
		{
			cost = 100;
		}
		else if ( !str_cmp( arg, "anchor" ) )
		{
			cost = 75;

				if ( IS_SET( ch->in_room->room_flags , ROOM_NOPEDIT ) && ch->in_room->vnum != ch->pcdata->roomconstruct )
				{
					send_to_char( "> &Ryou can not anchor into this node&w\n\r", ch );
					return;
				}

		}
		else if ( !str_cmp( arg, "audit" ) )
		{
			cost = 100;
		}
		else if ( !str_cmp( arg, "shortcut" ) )
		{
			cost = 100;
		}
		else
		{
			send_to_char( "> &Ryou cannot code that app, try:\n\r&w", ch);
			send_to_char( "> jackhammer, krash, spun, reconstruct\n\r", ch);
			send_to_char( "> dropline, uninstall, anchor, audit\n\r", ch);
			send_to_char( "> shortcut, checkout, emp\n\r", ch);
			return;
		}

		if ( ch->snippets < cost )
		{
			ch_printf( ch, "> &Rinsufficient snippets [%d needed]&w\n\r", cost );
			return;
		}

		checksew = FALSE;
		checkfab = FALSE;

		for ( obj = ch->last_carrying; obj; obj = obj->prev_content )
		{
			if (obj->item_type == ITEM_DATACUBE)
				checkfab = TRUE;
			if (obj->item_type == ITEM_OVEN)
				checksew = TRUE;
		}

		if ( !checkfab )
		{
			send_to_char( "&R> you need a datacube\n\r", ch);
			return;
		}

		if ( !checksew )
		{
			send_to_char( "&R> you need a compiler\n\r", ch);
			return;
		}

		chance = IS_NPC(ch) ? ch->top_level
				: (int) (ch->pcdata->learned[gsn_codeapp]);
		if ( number_percent( ) < chance )
		{
			send_to_char( "> &Gyou begin coding an application\n\r", ch);
			add_timer ( ch , TIMER_DO_FUN , 1 , do_codeapp , 1 );
			ch->dest_buf = str_dup(arg);
			return;
		}
		send_to_char("> &Ryou cannot figure out what to do\n\r",ch);
		learn_from_failure( ch, gsn_codeapp );
		return;

	case 1:
		if ( !ch->dest_buf )
			return;
		strcpy(arg, ch->dest_buf);
		DISPOSE( ch->dest_buf);
		break;

	case SUB_TIMER_DO_ABORT:
		DISPOSE( ch->dest_buf );
		ch->substate = SUB_NONE;
		send_to_char("> &Ryou are interrupted and fail to finish your work\n\r", ch);
		return;
	}

	ch->substate = SUB_NONE;

	level = IS_NPC(ch) ? ch->top_level : (int) (ch->pcdata->learned[gsn_codeapp]);

	checksew = FALSE;
	checkfab = FALSE;

	for ( obj = ch->last_carrying; obj; obj = obj->prev_content )
	{
		if (obj->item_type == ITEM_OVEN)
			checksew = TRUE;
		if ( (obj->item_type == ITEM_DATACUBE && checkfab == FALSE) )
		{
			checkfab = TRUE;
			separate_obj( obj );
			obj_from_char( obj );
			material = obj;
		}
	}

	if ( ( !checkfab ) || ( !checksew ) )
	{
		send_to_char( "> &Ryou could not code the application\n\r", ch);
		learn_from_failure( ch, gsn_codeapp );
		return;
	}

	int afumble = number_range(1,5);

	if ( afumble == 1 )
	{
		send_to_char( "> &Ryou fail to code the application\n\r", ch);
		learn_from_failure( ch, gsn_codeapp );
		ch->snippets     -= 1;
		return;
	}

	if ( !str_cmp( arg, "jackhammer" ) )
	{
		cost = 500;
	}
	else if ( !str_cmp( arg, "krash" ) )
	{
		cost = 250;
	}
	else if ( !str_cmp( arg, "spun" ) )
	{
		cost = 25;
	}
	else if ( !str_cmp( arg, "reconstruct" ) )
	{
		cost = 50;
	}
	else if ( !str_cmp( arg, "dropline" ) )
	{
		cost = 10;
	}
	else if ( !str_cmp( arg, "uninstall" ) )
	{
		cost = 1000;
	}
	else if ( !str_cmp( arg, "audit" ) )
	{
		cost = 100;
	}
	else if ( !str_cmp( arg, "emp" ) )
	{
		cost = 100;
	}
	else if ( !str_cmp( arg, "anchor" ) )
	{
		cost = 75;
	}
	else if ( !str_cmp( arg, "checkout" ) )
	{
		cost = 10;
	}
	else if ( !str_cmp( arg, "shortcut" ) )
	{
		cost = 100;
	}
	else
	{
		send_to_char( "> &Rsomething went wrong\n\r", ch);
		learn_from_failure( ch, gsn_codeapp );
		ch->snippets     -= 1;
		return;
	}

	ch->snippets     -= cost;

	obj = material;

	obj->item_type = ITEM_SNIPPET;
	obj->level = level;
	STRFREE( obj->name );
	strcpy( buf, arg );
	obj->name = STRALLOC( buf );
	strcat( buf, " [app]" );
	STRFREE( obj->short_descr );
	obj->short_descr = STRALLOC( buf );
	STRFREE( obj->description );
	obj->description = STRALLOC( buf );
	obj->value[0] = level;
	obj->cost = cost;

	if ( !str_cmp( arg, "anchor" ) )
		{
			obj->value[1] = ch->in_room->vnum;
			obj->value[0] = level / 10;
		    planet = ch->in_room->area->planet;
		    STRFREE( obj->name );
		    sprintf( buf1 , "anchor %ld" , ch->in_room->vnum );
		    obj->name = STRALLOC( buf1 );
		    STRFREE( obj->short_descr );
		    sprintf( buf , "anchor [%s] [%ld]" , planet->name, ch->in_room->vnum );
		    obj->short_descr = STRALLOC( buf );
			obj->cost = ( level / 10 ) * 10;
		}

	if ( !str_cmp( arg, "audit" ) )
		{
			obj->value[0] = level / 10;
			obj->cost = ( level / 10 ) * 10;
		}

	if ( !str_cmp( arg, "checkout" ) )
		{
			obj->value[0] = level / 10;
			obj->cost = ( level / 10 ) * 10;
		}

	if ( !str_cmp( arg, "shortcut" ) )
		{
			obj->value[0] = level / 10;
			obj->cost = ( level / 10 ) * 50;
		}

	obj = obj_to_char( obj, ch );

	send_to_char( "> &Gyou finish coding and look at your newly created application&w\n\r", ch);
	act( AT_PLAIN, "> $n finishes coding a new application", ch,
			NULL, argument , TO_ROOM );

	if ( number_percent() == 23 )
	{
		send_to_char( "> you feel more intelligent than before\n\r", ch );
		ch->perm_int++;
		ch->perm_int = UMIN( ch->perm_int , 25 );
	}

	learn_from_success( ch, gsn_codeapp );

	return;
}

void do_workmate( CHAR_DATA *ch, char *argument )
{
	char arg[MAX_INPUT_LENGTH];
	char arg1[MAX_INPUT_LENGTH];
	CHAR_DATA *och;
	int chance, count;
	
	argument = one_argument( argument , arg );
	argument = one_argument( argument , arg1 );

	if ( IS_NPC( ch ) || !ch->pcdata )
		return;

	if ( ch->pcdata->learned[gsn_spacecraft] <= 0  )
	{
		send_to_char(
				"> your mind races as you realize you have no idea how to do that\n\r", ch );
		return;
	}
	
	if ( ( ch->pcdata->wm_name == '\0' ) && str_cmp( arg, "name" ) )
	{
		send_to_char("> please set the name of your workmate with 'workmate name <your desired name>'\n\r", ch );
		return;
	}

	if ( !str_cmp( arg, "name") )
	{
		if ( !check_parse_name( arg1 ) )
		{
			send_to_char( "> &Rinvalid Workmate name&w\n\r", ch );
			return;
		}

		ch->pcdata->wm_name = arg1;
		send_to_char( "> &GWorkmate named&w\n\r", ch );
	}
	else if ( !str_cmp( arg, "update") )
	{
			send_to_char( "> &Rspecify which Workmate stat you want to upgrade&w\n\r", ch );
	}
	else if ( !str_cmp( arg, "dismiss") )
	{

		count = 0;
		for ( och = ch->in_room->first_person; och; och = och->next_in_room )
		{
			if ( IS_AFFECTED(och, AFF_CHARM)
					&&   och->master == ch )
				count++;
		}
		if( count < 1 )
		{
			send_to_char("> no pet program loaded\n\r", ch );
			return;
		}

			send_to_char( "> &Yyour workmate is unloaded&w\n\r", ch );
			extract_char( och, TRUE );
			return;
	}
	else
	{
	count = 0;
	for ( och = ch->in_room->first_person; och; och = och->next_in_room )
	{
		if ( IS_AFFECTED(och, AFF_CHARM)
				&&   och->master == ch )
			count++;
	}
	if( count >= 1 )
	{
		send_to_char("> you already have a pet program loaded\n\r", ch );
		return;
	}

	strcpy( arg, argument );

	switch( ch->substate )
	{
	default:
		if ( ch->backup_wait )
		{
			send_to_char( "> &Ryour workmate is already loading\n\r", ch );
			return;
		}
		chance = (int) (ch->pcdata->learned[gsn_spacecraft]);
		if ( number_percent( ) < chance )
		{
			send_to_char( "> &Gyou activate your workmate&w\n\r", ch);
			act( AT_PLAIN, "> $n begins activating $s workmate", ch,
					NULL, argument , TO_ROOM );
			add_timer ( ch , TIMER_DO_FUN , 1 , do_workmate , 1 );
			ch->dest_buf = str_dup(arg);
			return;
		}
		send_to_char("> &Ryou activate your workmate, but it does not respond&w\n\r",ch);
		learn_from_failure( ch, gsn_spacecraft );
		return;

	case 1:
		if ( !ch->dest_buf )
			return;
		strcpy(arg, ch->dest_buf);
		DISPOSE( ch->dest_buf);
		break;

	case SUB_TIMER_DO_ABORT:
		DISPOSE( ch->dest_buf );
		ch->substate = SUB_NONE;
		send_to_char("> &Ryou are interrupted before you can finish&w\n\r", ch);
		return;
	}

	ch->substate = SUB_NONE;

	send_to_char( "> &Gyour workmate is loading...&w\n\r", ch);

	if ( number_percent() == 23 )
	{
		send_to_char( "> you feel smarter than before\n\r", ch );
		ch->perm_int++;
		ch->perm_int = UMIN( ch->perm_int , 25 );
	}

	learn_from_success( ch, gsn_spacecraft );

	ch->backup_mob = MOB_VNUM_WORKMATE;

	ch->backup_wait = 1;

	return;
	}
	

		
}


const struct cards allcards[208]={
        {2, "2 of clubs"}, {2, "2 of hearts"}, {2, "2 of diamonds"}, {2, "2 of spades"},
        {3, "3 of clubs"}, {3, "3 of hearts"}, {3, "3 of diamonds"}, {3, "3 of spades"},
        {4, "4 of clubs"}, {4, "4 of hearts"}, {4, "4 of diamonds"}, {4, "4 of spades"},
        {5, "5 of clubs"}, {5, "5 of hearts"}, {5, "5 of diamonds"}, {5, "5 of spades"},
        {6, "6 of clubs"}, {6, "6 of hearts"}, {6, "6 of diamonds"}, {6, "6 of spades"},
        {7, "7 of clubs"}, {7, "7 of hearts"}, {7, "7 of diamonds"}, {7, "7 of spades"},
        {8, "8 of clubs"}, {8, "8 of hearts"}, {8, "8 of diamonds"}, {8, "8 of spades"},
        {9, "9 of clubs"}, {9, "9 of hearts"}, {9, "9 of diamonds"}, {9, "9 of spades"},
        {10, "10 of clubs"}, {10, "10 of hearts"}, {10, "10 of diamonds"}, {10, "10 of spades"},
        {10, "Jack of clubs"}, {10, "Jack of hearts"}, {10, "Jack of diamonds"}, {10, "Jack of spades"},
        {10, "Queen of clubs"}, {10, "Queen of hearts"}, {10, "Queen of diamonds"}, {10, "Queen of spades"},
        {10, "King of clubs"}, {10, "King of hearts"}, {10, "King of diamonds"}, {10, "King of spades"},
        {11, "Ace of clubs"}, {11, "Ace of hearts"}, {11, "Ace of diamonds"}, {11, "Ace of spades"},

        {2, "2 of clubs"}, {2, "2 of hearts"}, {2, "2 of diamonds"}, {2, "2 of spades"},
        {3, "3 of clubs"}, {3, "3 of hearts"}, {3, "3 of diamonds"}, {3, "3 of spades"},
        {4, "4 of clubs"}, {4, "4 of hearts"}, {4, "4 of diamonds"}, {4, "4 of spades"},
        {5, "5 of clubs"}, {5, "5 of hearts"}, {5, "5 of diamonds"}, {5, "5 of spades"},
        {6, "6 of clubs"}, {6, "6 of hearts"}, {6, "6 of diamonds"}, {6, "6 of spades"},
        {7, "7 of clubs"}, {7, "7 of hearts"}, {7, "7 of diamonds"}, {7, "7 of spades"},
        {8, "8 of clubs"}, {8, "8 of hearts"}, {8, "8 of diamonds"}, {8, "8 of spades"},
        {9, "9 of clubs"}, {9, "9 of hearts"}, {9, "9 of diamonds"}, {9, "9 of spades"},
        {10, "10 of clubs"}, {10, "10 of hearts"}, {10, "10 of diamonds"}, {10, "10 of spades"},
        {10, "Jack of clubs"}, {10, "Jack of hearts"}, {10, "Jack of diamonds"}, {10, "Jack of spades"},


        {10, "Queen of clubs"}, {10, "Queen of hearts"}, {10, "Queen of diamonds"}, {10, "Queen of spades"},
        {10, "King of clubs"}, {10, "King of hearts"}, {10, "King of diamonds"}, {10, "King of spades"},
        {11, "Ace of clubs"}, {11, "Ace of hearts"}, {11, "Ace of diamonds"}, {11, "Ace of spades"},

        {2, "2 of clubs"}, {2, "2 of hearts"}, {2, "2 of diamonds"}, {2, "2 of spades"},
        {3, "3 of clubs"}, {3, "3 of hearts"}, {3, "3 of diamonds"}, {3, "3 of spades"},
        {4, "4 of clubs"}, {4, "4 of hearts"}, {4, "4 of diamonds"}, {4, "4 of spades"},
        {5, "5 of clubs"}, {5, "5 of hearts"}, {5, "5 of diamonds"}, {5, "5 of spades"},
        {6, "6 of clubs"}, {6, "6 of hearts"}, {6, "6 of diamonds"}, {6, "6 of spades"},
        {7, "7 of clubs"}, {7, "7 of hearts"}, {7, "7 of diamonds"}, {7, "7 of spades"},
        {8, "8 of clubs"}, {8, "8 of hearts"}, {8, "8 of diamonds"}, {8, "8 of spades"},
        {9, "9 of clubs"}, {9, "9 of hearts"}, {9, "9 of diamonds"}, {9, "9 of spades"},
        {10, "10 of clubs"}, {10, "10 of hearts"}, {10, "10 of diamonds"}, {10, "10 of spades"},
        {10, "Jack of clubs"}, {10, "Jack of hearts"}, {10, "Jack of diamonds"}, {10, "Jack of spades"},
        {10, "Queen of clubs"}, {10, "Queen of hearts"}, {10, "Queen of diamonds"}, {10, "Queen of spades"},
        {10, "King of clubs"}, {10, "King of hearts"}, {10, "King of diamonds"}, {10, "King of spades"},
        {11, "Ace of clubs"}, {11, "Ace of hearts"}, {11, "Ace of diamonds"}, {11, "Ace of spades"},

        {2, "2 of clubs"}, {2, "2 of hearts"}, {2, "2 of diamonds"}, {2, "2 of spades"},
        {3, "3 of clubs"}, {3, "3 of hearts"}, {3, "3 of diamonds"}, {3, "3 of spades"},
        {4, "4 of clubs"}, {4, "4 of hearts"}, {4, "4 of diamonds"}, {4, "4 of spades"},
        {5, "5 of clubs"}, {5, "5 of hearts"}, {5, "5 of diamonds"}, {5, "5 of spades"},
        {6, "6 of clubs"}, {6, "6 of hearts"}, {6, "6 of diamonds"}, {6, "6 of spades"},
        {7, "7 of clubs"}, {7, "7 of hearts"}, {7, "7 of diamonds"}, {7, "7 of spades"},
        {8, "8 of clubs"}, {8, "8 of hearts"}, {8, "8 of diamonds"}, {8, "8 of spades"},
        {9, "9 of clubs"}, {9, "9 of hearts"}, {9, "9 of diamonds"}, {9, "9 of spades"},
        {10, "10 of clubs"}, {10, "10 of hearts"}, {10, "10 of diamonds"}, {10, "10 of spades"},
        {10, "Jack of clubs"}, {10, "Jack of hearts"}, {10, "Jack of diamonds"}, {10, "Jack of spades"},
        {10, "Queen of clubs"}, {10, "Queen of hearts"}, {10, "Queen of diamonds"}, {10, "Queen of spades"},
        {10, "King of clubs"}, {10, "King of hearts"}, {10, "King of diamonds"}, {10, "King of spades"},
        {11, "Ace of clubs"}, {11, "Ace of hearts"}, {11, "Ace of diamonds"}, {11, "Ace of spades"}
};

int total_cards( CHAR_DATA *ch )
{
   int i;
   int aces=0, total=0;
   for ( i = 0; i < ch->hand->num_cards;i++){
	total += allcards[ch->hand->card[i]].total;
	if ( allcards[ch->hand->card[i]].total == 11 )
		aces++;
   }
   if ( total > 21 && aces ){
	total -= 10;
	aces--;
   }
   if ( total > 21 && aces ){
	total -= 10;
	aces--;
   }
   if ( total > 21 && aces ){
	total -= 10;
	aces--;
   }
   if ( total > 21 && aces ){
	total -= 10;
	aces--;
   }
   return total;
}

int total_dealer_cards( BLACKJACK_DATA *game ) {
   int i;
   int aces=0, total=0;

   for ( i = 0; i < game->num_cards;i++){
	total += allcards[game->dealerhand->card[i]].total;
	if ( allcards[game->dealerhand->card[i]].total == 11 )
		aces++;
   }
   if ( total > 21 && aces ){
	total -= 10;
	aces--;
   }
   if ( total > 21 && aces ){
	total -= 10;
	aces--;
   }
   if ( total > 21 && aces ){
	total -= 10;
	aces--;
   }
   if ( total > 21 && aces ){
	total -= 10;
	aces--;
   }
   return total;
}

void reset_deck( BLACKJACK_DATA *game )
{
    int x;

    if ( game )
    {
	for ( x = 0; x < 208; x++ )
	{
	    game->deck[x] = -1;
	}
    }
}

void shuffle_cards ( BLACKJACK_DATA *game )
{
    int i,j;
    int count=0;
    bool found = FALSE;

    game->cards_drawn = 0;

    for ( i = 0; i < 208;i++)
    {
	game->deck[i] = -1;
    }

    while ( count < 208 )
    {
	i = number_range( 0, 207 );
	for ( j = 0; j < 208; j++)
	{
	   if ( game->deck[j] == i )
	   {
		found = TRUE;
		break;
	   }
	   else if ( game->deck[j] == -1 )
		break;
  	}
	if ( !found )
	{
	     game->deck[count] = i;
	     count++;
	}
	found = FALSE;
    }
    return;
}

void reset_cards( CARD_DATA *hand )
{
    int x;

    if ( hand )
    {
    	for ( x = 0; x < MAX_HELD_CARDS; x++ )
    	{
	    hand->card[x] = 0;
    	}
    }
}

void do_blackjack( CHAR_DATA *ch, char *argument )
{
    BLACKJACK_DATA *game;
    CHAR_DATA *tmp;
    char arg[MAX_STRING_LENGTH];
    char arg2[MAX_STRING_LENGTH];
    char buf[MAX_STRING_LENGTH];

    if ( IS_NPC(ch) )
	return;

//    if ( !IS_SET( ch->in_room->nroom_flags , NROOM_BLACKJACKROOM ) )
//    {
//	send_to_char( "You are not in a blackjack room.\n\r", ch );
//	return;
//    }

    if ( argument[0] == '\0' )
    {
	if ( ch->cur_blackjack )
	{
	    game = ch->cur_blackjack;

	    if ( game->hold && ch == game->pturn )
	    {
		ch_printf( ch, "The game is waiting for you to %s.\n\r",
			game->status == STATUS_TAKEBETS ? "place a bet" :
			game->status == STATUS_DEALEXTRA ? "choose whether to hit or to stay." :
			"<unkown option:contact gohan>" );
		ch_printf( ch, "You have %d minutes %d seconds left to decide.\n\r\n\r",
			ch->count/60, ch->count%60 );
	    }
	    else if ( ch->dealout )
	    {
		send_to_char( "You have either been dealt out of this hand or you busted.\n\r\n\r", ch );
	    }
	    display_status(ch);
	}
	else
	{
	    send_to_char( "Syntax: blackjack <field>\n\r", ch );
	    send_to_char( "Where field is on of the following:\n\r", ch );
	    send_to_char( " create, join, leave, list, bet, stay, hit\n\r", ch );
	}
	return;
    }

    argument = one_argument(argument,arg);

    if ( !str_cmp(arg,"say") )
    {
	do_say(ch,argument);
	display_status(ch);
	return;
    }

    if ( !str_cmp(arg,"ooc") )
    {
        do_ooc(ch,argument);
        display_status(ch);
        return;
    }

    if ( IS_IMMORTAL( ch ) && !str_cmp( arg, "/!" ) )
    {
        DO_FUN *last_cmd;
        int substate = ch->substate;

        last_cmd = ch->last_cmd;
        ch->substate = SUB_RESTRICTED;
        interpret(ch, argument);
        ch->substate = substate;
        ch->last_cmd = last_cmd;
        return;
    }

    argument = one_argument(argument,arg2);

    if ( !str_cmp(arg,"create") )
    {
	if ( ch->cur_blackjack )
	{
	    send_to_char( "You are already in a blackjack game, leave before creating a new one.\n\r", ch );
	    return;
	}

	CREATE( game, BLACKJACK_DATA, 1 );
	game->player[1] = ch;
	game->created_by = ch;
	LINK( ch, game->first_gambler, game->last_gambler, next_gambler, prev_gambler );
	game->hold = TRUE;
	game->status = STATUS_HOLD;
	game->count = 60;
	game->num_cards = 0;
	shuffle_cards(game);
	ch->dealout = FALSE;
	game->num_players = 1; // Not including the dealer of course
	ch->cur_blackjack = game;
	if ( last_blackjack )
	    game->game_num = last_blackjack->game_num++;
	else if ( first_blackjack )
	    game->game_num = first_blackjack->game_num++;
	else
	    game->game_num = 1;
	LINK( game, first_blackjack, last_blackjack, next, prev );
	act( AT_PLAIN, "You have created a new blackjack game, the game will commence in 30 seconds.", ch, NULL, NULL, TO_CHAR );
	sprintf( buf, "$n has created a new blackjack game numbered %d.", game->game_num );
	act( AT_PLAIN, buf, ch, NULL, NULL, TO_ROOM );
	sprintf( buf, "NEW BLACKJACK GAME #%d: Created by %s.", game->game_num, ch->name );
	//game_info( AT_RED, buf );
	ch->desc->connected = CON_BLACKJACK;
	return;
    }

    if ( !str_cmp(arg,"join") )
    {
	int num;
	bool found = FALSE;

	if ( !is_number(arg2) )
	{
	    send_to_char( "Syntax: blackjack join <game number>\n\r", ch );
	    send_to_char( "Type blackjack list to view created games and their numbers.\n\r", ch );
	    return;
	}

	num = atoi(arg2);

	for ( game = first_blackjack; game; game = game->next )
	{
	    if ( game->game_num == num )
	    {
		found = TRUE;
		break;
	    }
	}

	if ( found )
	{
	    if ( game->num_players == MAX_BLACKJACK_PLAYERS )
	    {
		send_to_char( "Sorry but that blackjack game is full.\n\r", ch );
		return;
	    }

	    ch_printf( ch, "You have joined GAME #%d\n\r", game->game_num );
	    game->num_players++;
	    ch->cur_blackjack = game;
	    if ( game->status == STATUS_HOLD )
		ch_printf( ch, "The game will start in %d seconds.\n\r", game->count );
	    else
	    {
		send_to_char( "The current hand is still in progress, you will join in when the hand finishes.\n\r", ch );
		ch->dealout = TRUE;
	    }
	    for ( tmp = game->first_gambler; tmp; tmp = tmp->next_gambler )
	    	ch_printf( tmp, "%s has entered the blackjack game.\n\r", ch->name );
	    LINK( ch, game->first_gambler, game->last_gambler, next_gambler, prev_gambler );
	    ch->desc->connected = CON_BLACKJACK;
	    sprintf( buf, "%s has joined blackjack game #%d", ch->name, game->game_num );
	    //game_info( AT_RED, buf );
	}
	else
	{
	    send_to_char( "A blackjack game with that game number could not be located.\n\r", ch );
	    send_to_char( "Type 'blackjack list' to view created games and their numbers.\n\r", ch );
	}
	return;
    }

    if ( !str_cmp( arg, "leave" ) )
    {
	if ( !ch->cur_blackjack )
	{
	    send_to_char( "You are not currently in a blackjack game.\n\r", ch );
	    return;
	}

	game = ch->cur_blackjack;

	game->num_players--;

	for ( tmp = game->first_gambler; tmp; tmp = tmp->next_gambler )
	{
	    if ( tmp != ch )
	    	ch_printf( tmp, "%s has left the game.\n\r", ch->name );
	}
	UNLINK( ch, game->first_gambler, game->last_gambler, next_gambler, prev_gambler );

	if ( game->num_players == 0 )
	{
	    UNLINK( game, first_blackjack, last_blackjack, next, prev );
	}

	if ( ch->blackjack_bet > 0 )
	{
	    ch->gold -= ch->blackjack_bet;
	}
	ch->cur_blackjack = NULL;
	ch_printf( ch, "You have left the blackjack game.\n\r" );
	ch->desc->connected = CON_PLAYING;
	return;
    }

    if ( !str_cmp( arg, "bet" ) )
    {
	int bet;

	if ( !ch->cur_blackjack )
	{
	    send_to_char( "You are not currently in a blackjack game.\n\r", ch );
	    return;
	}

	if ( ch->dealout )
	{
	    send_to_char( "You have been dealt out of this hand, wait until the next one.\n\r", ch );
	    return;
	}

	game = ch->cur_blackjack;

	if ( game->status != STATUS_TAKEBETS )
	{
	    send_to_char( "The game is not at the betting phase yet.\n\r", ch );
	    return;
	}

	if ( game->pturn != ch )
	{
	    send_to_char( "It is not your turn to bet.\n\r", ch );
	    return;
	}

	if ( !is_number(arg2) )
	{
	    send_to_char( "You must enter an amount to bet.\n\r", ch );
	    return;
	}

	bet = atoi(arg2);

	if ( bet < 1 || bet > 10000 )
	{
	    send_to_char( "Place a bet between 1 and 10,000 coins.\n\r", ch );
	    return;
	}

	if ( ch->gold < bet )
	{
	    send_to_char( "You don't have that much gold to bet\n\r", ch );
	    return;
	}

	ch->blackjack_bet = bet;

	ch_printf( ch, "You placed a bet of %d on this hand of blackjack.\n\r", bet );
	ch->count = 0;

	for ( tmp = game->first_gambler; tmp; tmp = tmp->next_gambler )
	{
	    if ( !tmp || !tmp->desc || tmp == ch )
		continue;

	    ch_printf( tmp, "%s has placed a bet of %d on this blackjack game.\n\r", ch->name, bet );
	}
	game->hold = FALSE;
	return;
    }

    if ( !str_cmp(arg,"hit") )
    {
	if ( !ch->cur_blackjack )
	{
	    send_to_char( "You are not currently in a blackjack game.\n\r", ch );
	    return;
	}

	if ( ch->dealout )
	{
	    send_to_char( "You have been dealt out of this hand, wait until the next one.\n\r", ch );
	    return;
	}

	game = ch->cur_blackjack;

	if ( game->status != STATUS_DEALEXTRA )
	{
	    send_to_char( "The game is not at the hit/stay phase yet.\n\r", ch );
	    return;
	}

	if ( game->pturn != ch )
	{
	    send_to_char( "It is not your turn to hit/stay.\n\r", ch );
	    return;
	}

    	if ( game->cards_drawn > 207 )
	{
 	    shuffle_cards(game);
	    for ( tmp = game->first_gambler; tmp; tmp = tmp->next_gambler )
	    {
		if ( tmp && tmp->desc )
		    ch_printf( tmp, "The dealer reshuffles the deck.\n\r" );
	    }
    	}

        for ( tmp = game->first_gambler; tmp; tmp = tmp->next_gambler )
        {
            if ( tmp && tmp->desc )
                ch_printf( tmp, "%s get%s a %s from the dealer.\n\r",
                        tmp == ch ? "You" : ch->name, tmp == ch ? "" : "s",
			allcards[game->deck[game->cards_drawn]].name );
        }

    	ch->hand->card[ch->hand->num_cards] = game->deck[game->cards_drawn];
    	ch->hand->num_cards++;
    	game->cards_drawn++;
    	if ( total_cards( ch ) > 21 )
	{
 	    send_to_char( "You busted, maybe you'll win next game.\n\r", ch );
 	    ch->dealout = TRUE;
	    ch->gold -= ch->blackjack_bet;
            ch->blackjack_bet = 0;
	    if ( game->num_players <= 1 )
	    {
                game->hold = TRUE;
                game->count = 60;
                game->status = STATUS_HOLD;
		return;
	    }
	    for ( tmp = game->first_gambler; tmp; tmp = tmp->next_gambler )
	    {
		if ( tmp && tmp->desc && (tmp != ch) )
		    ch_printf( tmp, "%s busted.\n\r", ch->name );
	    }
	    game->hold = FALSE;
    	    return;
        }
        ch_printf( ch, "Your hand total is now %d.\n\r", total_cards(ch));
	if ( ch->hand->num_cards == MAX_HELD_CARDS )
	{
	    send_to_char( "You have reached the maximum allowed cards for a player.\n\r", ch );
	    do_blackjack( ch, "stay" );
	}
	return;
    }

    if ( !str_cmp(arg,"stay") )
    {
	if ( !ch->cur_blackjack )
	{
	    send_to_char( "You are not currently in a blackjack game.\n\r", ch );
	    return;
	}

	if ( ch->dealout )
	{
	    send_to_char( "You have been dealt out of this hand, wait until the next one.\n\r", ch );
	    return;
	}

	game = ch->cur_blackjack;

	if ( game->status != STATUS_DEALEXTRA )
	{
	    send_to_char( "The game is not at the hit/stay phase yet.\n\r", ch );
	    return;
	}

	if ( game->pturn != ch )
	{
	    send_to_char( "It is not your turn to hit/stay.\n\r", ch );
	    return;
	}

	for ( tmp = game->first_gambler; tmp; tmp = tmp->next_gambler )
	{
	    if ( !tmp || tmp == ch )
		continue;

	    ch_printf( tmp, "%s chooses to stay.\n\r", ch->name );
	}
	game->hold = FALSE;
	return;
    }

    if ( !str_cmp(arg,"list") )
    {
	int cnt = 0;
        for ( game = first_blackjack; game; game = game->next )
        {
	    cnt++;
	    ch_printf( ch, "#%d:  Created by: %s\n\r", game->game_num, game->created_by->name );
	    ch_printf( ch, "Current Gamblers: " );
	    for ( tmp = game->first_gambler; tmp; tmp = tmp->next_gambler )
	    {
		if ( !tmp || !tmp->desc )
		    continue;
		ch_printf( ch, "%s%s", tmp->name, tmp == game->last_gambler ? "" : ", " );
	    }
	    send_to_char( "\n\r--------------------------------------------------------------\n\r", ch );
        }
	ch_printf( ch, "Total of %d blackjack games in progress.\n\r", cnt );
	return;
    }

    do_blackjack(ch, "");
    return;
}

void display_status( CHAR_DATA *ch )
{
    BLACKJACK_DATA *game;

    if ( !ch || !ch->desc )
	return;

    if ( !ch->cur_blackjack )
	return;

    game = ch->cur_blackjack;

    switch ( game->status )
    {

	case STATUS_HOLD:
		ch_printf( ch, "&W[&RGame Status&W] Waiting for next game to start!\n\r" );
		ch_printf( ch, "&W[&RNumber of Players&W] %d player%s\n\r",
			game->num_players, game->num_players == 1 ? "" : "s" );
		break;
	case STATUS_TAKEBETS:
                ch_printf( ch, "&W[&RGame Status&W] Taking Bets!\n\r" );
		if ( game->num_players > 1 )
		{
		    if ( game->pturn == game->last_gambler )
                    	ch_printf( ch, "&W[&RLast Better&W] %s\n\r", game->pturn->name );
		    else
			ch_printf( ch, "&W[&RCurrent Better&W] %s  [&RNext Better&W] %s\n\r",
				game->pturn->name, game->pturn->next_gambler->name );
		}
		if ( ch->blackjack_bet )
		    ch_printf( ch, "&W[&RYour Bet&W] %d\n\r", ch->blackjack_bet );
		if ( game->pturn == ch )
		    ch_printf( ch, "\n\rTYPE: BET <1-10,000> to place your bet\n\r\n\r" );
		break;
	case STATUS_DEAL:
	case STATUS_DEALEXTRA:
                ch_printf( ch, "&W[&RGame Status&W] Taking Bets!\n\r" );
                if ( game->num_players > 1 )
                {
                    if ( game->pturn == game->last_gambler )
                        ch_printf( ch, "&W[&RLast Player&W] %s\n\r", game->pturn->name );
                    else
                        ch_printf( ch, "&W[&RCurrent Player&W] %s  [&RNext Player&W] %s\n\r",
                                game->pturn->name, game->pturn->next_gambler->name );
                }
                ch_printf( ch, "&W[&RYour Bet&W] %d   &W[&RHand Worth&W] %d\n\r",
			ch->blackjack_bet, total_cards(ch) );
                if ( game->pturn == ch )
                    ch_printf( ch, "\n\rTYPE: HIT or STAY\n\r\n\r" );
		break;
	case STATUS_FINISH:
                ch_printf( ch, "&W[&RGame Status&W] Finishing Hand!\n\r\n\r" );
		break;
	default:
		break;
    }

    ch_printf( ch, "&W[&ROther Options&W] LEAVE, SAY, OOC\n\r" );
}

//void do_reverseengineer( CHAR_DATA *ch, char *argument )
//{
//	OBJ_INDEX_DATA *pObjIndex = NULL;
//	int level, chance;
//	bool checksew, checkfab;
//	OBJ_DATA *obj;
//
//
//	switch( ch->substate )
//	{
//	default:
//
//
//		if ( argument[0] == '\0' )
//		{
//			send_to_char( "> &Yspecify object&w\n\r", ch );
//			return;
//		}
//
//		if ( (obj = find_obj(ch, argument, TRUE)) == NULL )
//		{
//
//			return;
//		}
//
//
//		if ( !IS_SET( ch->in_room->room_flags, ROOM_RESTAURANT ) )
//		{
//			send_to_char( "> &Ryou need to be in a coding node\n\r", ch );
//			return;
//		}
//
//		checksew = FALSE;
//
//		for ( obj = ch->last_carrying; obj; obj = obj->prev_content )
//		{
//			if (obj->item_type == ITEM_OVEN)
//				checksew = TRUE;
//		}
//
//		if ( !checksew )
//		{
//			send_to_char( "&R> you need a compiler\n\r", ch);
//			return;
//		}
//
//		chance = IS_NPC(ch) ? ch->top_level
//				: (int) (ch->pcdata->learned[gsn_spacecraft]);
//		if ( number_percent( ) < chance )
//		{
//			send_to_char( "> &Gyou begin the long process of reverse engineering\n\r", ch);
//			act( AT_PLAIN, "> $n takes $s compiler as well as a module and begins to work", ch,
//					NULL, argument , TO_ROOM );
//			add_timer ( ch , TIMER_DO_FUN , 3 , do_reverseengineer , 1 );
//			ch->dest_buf = str_dup(argument);
//			return;
//		}
//		send_to_char("> &Ryou cannot figure out what to do\n\r",ch);
//		learn_from_failure( ch, gsn_spacecraft );
//		return;
//
//	case 1:
//		if ( !ch->dest_buf )
//			return;
//		strcpy(argument, ch->dest_buf);
//		DISPOSE( ch->dest_buf);
//		break;
//
//	case SUB_TIMER_DO_ABORT:
//		DISPOSE( ch->dest_buf );
//		ch->substate = SUB_NONE;
//		send_to_char("> &Ryou are interrupted and fail to finish your work\n\r", ch);
//		return;
//	}
//
//	ch->substate = SUB_NONE;
//
//	level = IS_NPC(ch) ? ch->top_level : (int) (ch->pcdata->learned[gsn_spacecraft]);
//
//	checksew = FALSE;
//	checkfab = FALSE;
//
//	for ( obj = ch->last_carrying; obj; obj = obj->prev_content )
//	{
//		if (obj->item_type == ITEM_OVEN)
//			checksew = TRUE;
//
//		if ( (obj = find_obj(ch, argument, TRUE)) != NULL )
//		{
//			checkfab = TRUE;
//		}
//
//	}
//
//	if ( ( !checkfab ) || ( !checksew ) )
//	{
//		send_to_char( "> &Ryou could not reverse engineer anything&w\n\r", ch);
//		learn_from_failure( ch, gsn_spacecraft );
//		return;
//	}
//
//	if ( obj->value[3] == WEAPON_BLASTER )
//	{
//
//		separate_obj( obj );
//		extract_obj(obj);
//
//		if ( number_range(1, 100) > level )
//		{
//			pObjIndex = get_obj_index( 31 );
//			obj = create_object(pObjIndex, 1);
//			SET_BIT(obj->extra_flags, ITEM_INVENTORY);
//			obj = obj_to_char(obj, ch);
//		}
//
//		if ( number_range(1, 100) > level )
//		{
//			pObjIndex = get_obj_index( 37 );
//			obj = create_object(pObjIndex, 1);
//			SET_BIT(obj->extra_flags, ITEM_INVENTORY);
//			obj = obj_to_char(obj, ch);
//		}
//
//		if ( number_range(1, 100) > level )
//		{
//			pObjIndex = get_obj_index( 38 );
//			obj = create_object(pObjIndex, 1);
//			SET_BIT(obj->extra_flags, ITEM_INVENTORY);
//			obj = obj_to_char(obj, ch);
//		}
//
//
//	}
//
//	if ( obj->value[3] == WEAPON_VIBRO_BLADE )
//	{
//
//		separate_obj( obj );
//		extract_obj(obj);
//
//		if ( number_range(1, 100) > level )
//		{
//			pObjIndex = get_obj_index( 34 );
//			obj = create_object(pObjIndex, 1);
//			SET_BIT(obj->extra_flags, ITEM_INVENTORY);
//			obj = obj_to_char(obj, ch);
//		}
//
//	}
//
//	if ( obj->item_type == ITEM_ARMOR )
//	{
//
//		int objnum = obj->pIndexData->vnum;
//		separate_obj( obj );
//		extract_obj(obj);
//
//		if ( number_range(1, 100) > level )
//		{
//			pObjIndex = get_obj_index( objnum );
//			obj = create_object(pObjIndex, 1);
//			SET_BIT(obj->extra_flags, ITEM_INVENTORY);
//			obj = obj_to_char(obj, ch);
//		}
//
//	}
//
//	if ( obj->item_type == ITEM_CONTAINER )
//	{
//
//		int objnum = obj->pIndexData->vnum;
//		separate_obj( obj );
//		extract_obj(obj);
//
//		if ( number_range(1, 100) > level )
//		{
//			pObjIndex = get_obj_index( objnum );
//			obj = create_object(pObjIndex, 1);
//			SET_BIT(obj->extra_flags, ITEM_INVENTORY);
//			obj = obj_to_char(obj, ch);
//		}
//
//	}
//
//	if ( obj->item_type == ITEM_MEDPAC )
//	{
//
//		int objnum = obj->pIndexData->vnum;
//		separate_obj( obj );
//		extract_obj(obj);
//
//		if ( number_range(1, 100) > level )
//		{
//			pObjIndex = get_obj_index( objnum );
//			obj = create_object(pObjIndex, 1);
//			SET_BIT(obj->extra_flags, ITEM_INVENTORY);
//			obj = obj_to_char(obj, ch);
//		}
//
//	}
//
//	int anumber = number_range(1,100);
//	if ( anumber == 23 )
//	learn_from_success( ch , gsn_spacecraft );
//
//	send_to_char( "> &Gyou finish reverse engineering&w\n\r", ch);
//	return;
//
//}

void do_reverseengineer( CHAR_DATA * ch, char *argument )
{
   OBJ_DATA *obj, *obj_next;
   OBJ_INDEX_DATA *pObjIndex = NULL;
   bool found = FALSE;
   int level;

   if( !argument || argument[0] == '\0' )
   {
      send_to_char( "> revengi what?\r\n", ch );
      return;
   }

   for( obj = ch->first_carrying; obj; obj = obj_next )
   {
      obj_next = obj->next_content;
      if( ( nifty_is_name( argument, obj->name ) ) && can_see_obj( ch, obj ) && obj->wear_loc == WEAR_NONE )
      {
         found = TRUE;
         break;
      }
   }

   if( found )
   {
      if( !can_drop_obj( ch, obj ) && ch->top_level < 200 )
      {
         send_to_char( "> you cannot delete that - it is corrupted!\r\n", ch );
         return;
      }

      if( ( obj->item_type == ITEM_CONTAINER ) )
      {
         send_to_char( "> you cannot reverse engineer containers\r\n", ch );
         return;
      }

      if( ( obj->pIndexData->vnum == 100 && ch->top_level < 200 ) )
      {
         send_to_char( "> you cannot reverse engineer that\r\n", ch );
         return;
      }

      if( ( obj->pIndexData->vnum == 101 && ch->top_level < 200 ) )
      {
         send_to_char( "> you cannot reverse engineer that\r\n", ch );
         return;
      }

      if( ( obj->pIndexData->vnum == 15 && ch->top_level < 200 ) )
      {
         send_to_char( "> you delete the message\r\n", ch );
         separate_obj( obj );
         obj_from_char( obj );
         extract_obj( obj );
         return;
      }


      level = IS_NPC(ch) ? ch->top_level : (int) (ch->pcdata->learned[gsn_spacecraft]);

      act( AT_ACTION, "> $n reverse engineered $p", ch, obj, NULL, TO_ROOM );
      act( AT_GREEN, "> you reverse engineered $p", ch, obj, NULL, TO_CHAR );

      separate_obj( obj );
      obj_from_char( obj );
      extract_obj( obj );

  	if ( obj->value[3] == WEAPON_BLASTER )
  	{

  		if ( number_range(1, 100) > level )
  		{
  			pObjIndex = get_obj_index( 31 );
  			obj = create_object(pObjIndex, 1);
  			SET_BIT(obj->extra_flags, ITEM_INVENTORY);
  			obj = obj_to_char(obj, ch);

  	  		if ( number_range(1, 100) < level )
  	  		{
  				pObjIndex = get_obj_index( 55 );
  				obj = create_object(pObjIndex, 1);
  				SET_BIT(obj->extra_flags, ITEM_INVENTORY);
  				obj = obj_to_char(obj, ch);
  	  		}

  		}
  		else
  		{
  			send_to_char( "> class failed\r\n", ch );
  	  		if ( number_range(1, 100) < level )
  	  		{
  				pObjIndex = get_obj_index( 55 );
  				obj = create_object(pObjIndex, 1);
  				SET_BIT(obj->extra_flags, ITEM_INVENTORY);
  				obj = obj_to_char(obj, ch);
  	  		}
  		}

  		if ( number_range(1, 100) > level )
  		{
  			pObjIndex = get_obj_index( 37 );
  			obj = create_object(pObjIndex, 1);
  			SET_BIT(obj->extra_flags, ITEM_INVENTORY);
  			obj = obj_to_char(obj, ch);
  		}
  		else
  		{
  			send_to_char( "> var1 failed\r\n", ch );
  	  		if ( number_range(1, 100) < level )
  	  		{
  				pObjIndex = get_obj_index( 55 );
  				obj = create_object(pObjIndex, 1);
  				SET_BIT(obj->extra_flags, ITEM_INVENTORY);
  				obj = obj_to_char(obj, ch);
  	  		}
  		}

  		if ( number_range(1, 100) > level )
  		{
  			pObjIndex = get_obj_index( 38 );
  			obj = create_object(pObjIndex, 1);
  			SET_BIT(obj->extra_flags, ITEM_INVENTORY);
  			obj = obj_to_char(obj, ch);
  		}
  		else
  	  		{
  	  			send_to_char( "> var2 failed\r\n", ch );
  	  	  		if ( number_range(1, 100) < level )
  	  	  		{
  	  				pObjIndex = get_obj_index( 55 );
  	  				obj = create_object(pObjIndex, 1);
  	  				SET_BIT(obj->extra_flags, ITEM_INVENTORY);
  	  				obj = obj_to_char(obj, ch);
  	  	  		}
  	  		}


  	}

  	else if ( obj->value[3] == WEAPON_VIBRO_BLADE )
  	{

  		if ( number_range(1, 100) < level )
  		{
  			pObjIndex = get_obj_index( 34 );
  			obj = create_object(pObjIndex, 1);
  			SET_BIT(obj->extra_flags, ITEM_INVENTORY);
  			obj = obj_to_char(obj, ch);

  			if ( number_range(1, 100) < level )
  	  		{
  				pObjIndex = get_obj_index( 55 );
  				obj = create_object(pObjIndex, 1);
  				SET_BIT(obj->extra_flags, ITEM_INVENTORY);
  				obj = obj_to_char(obj, ch);
  	  		}

  		}
  		else
  		{
  			send_to_char( "> class failed\r\n", ch );
  	  		if ( number_range(1, 100) < level )
  	  		{
  				pObjIndex = get_obj_index( 55 );
  				obj = create_object(pObjIndex, 1);
  				SET_BIT(obj->extra_flags, ITEM_INVENTORY);
  				obj = obj_to_char(obj, ch);
  	  		}
  		}

  	}

  	else if ( obj->item_type == ITEM_ARMOR )
  	{

  		int objnum = obj->pIndexData->vnum;

  		if ( number_range(1, 100) < level )
  		{
  			pObjIndex = get_obj_index( objnum );
  			obj = create_object(pObjIndex, 1);
  			SET_BIT(obj->extra_flags, ITEM_INVENTORY);
  			obj = obj_to_char(obj, ch);

  	  		if ( number_range(1, 100) < level )
  	  		{
  				pObjIndex = get_obj_index( 55 );
  				obj = create_object(pObjIndex, 1);
  				SET_BIT(obj->extra_flags, ITEM_INVENTORY);
  				obj = obj_to_char(obj, ch);
  	  		}

  		}
  		else
  		{
  			send_to_char( "> class failed\r\n", ch );
  	  		if ( number_range(1, 100) < level )
  	  		{
  				pObjIndex = get_obj_index( 55 );
  				obj = create_object(pObjIndex, 1);
  				SET_BIT(obj->extra_flags, ITEM_INVENTORY);
  				obj = obj_to_char(obj, ch);
  	  		}
  		}

  	}

  	else if ( obj->item_type == ITEM_CONTAINER )
  	{

  		int objnum = obj->pIndexData->vnum;

  		if ( number_range(1, 100) < level )
  		{
  			pObjIndex = get_obj_index( objnum );
  			obj = create_object(pObjIndex, 1);
  			SET_BIT(obj->extra_flags, ITEM_INVENTORY);
  			obj = obj_to_char(obj, ch);

  	  		if ( number_range(1, 100) < level )
  	  		{
  				pObjIndex = get_obj_index( 55 );
  				obj = create_object(pObjIndex, 1);
  				SET_BIT(obj->extra_flags, ITEM_INVENTORY);
  				obj = obj_to_char(obj, ch);
  	  		}

  		}
  		else
  		{
  			send_to_char( "> class failed\r\n", ch );
  	  		if ( number_range(1, 100) < level )
  	  		{
  				pObjIndex = get_obj_index( 55 );
  				obj = create_object(pObjIndex, 1);
  				SET_BIT(obj->extra_flags, ITEM_INVENTORY);
  				obj = obj_to_char(obj, ch);
  	  		}
  		}

  	}

  	else if ( obj->item_type == ITEM_MEDPAC )
  	{

  		int objnum = obj->pIndexData->vnum;

  		if ( number_range(1, 100) < level )
  		{
  			pObjIndex = get_obj_index( objnum );
  			obj = create_object(pObjIndex, 1);
  			SET_BIT(obj->extra_flags, ITEM_INVENTORY);
  			obj = obj_to_char(obj, ch);

  	  		if ( number_range(1, 100) < level )
  	  		{
  				pObjIndex = get_obj_index( 55 );
  				obj = create_object(pObjIndex, 1);
  				SET_BIT(obj->extra_flags, ITEM_INVENTORY);
  				obj = obj_to_char(obj, ch);
  	  		}

  		}
  		else
  		{
  			send_to_char( "> class failed\r\n", ch );
  	  		if ( number_range(1, 100) < level )
  	  		{
  				pObjIndex = get_obj_index( 55 );
  				obj = create_object(pObjIndex, 1);
  				SET_BIT(obj->extra_flags, ITEM_INVENTORY);
  				obj = obj_to_char(obj, ch);
  	  		}
  		}

  	}

  	else
  	{
  		ch->gold += obj->cost / 2;
  		ch->snippets += obj->cost / 2;

  		ch_printf( ch, "> &Gcredits:&W %d  &Gsnippets:&W %d&w&w\n\r", obj->cost / 2, obj->cost / 2 );

  		if ( number_range(1, 100) < level && obj->pIndexData->vnum != 55 )
  		{

  			if ( number_range(1, 100) > 50)
  			{
			pObjIndex = get_obj_index( 55 );
			obj = create_object(pObjIndex, 1);
			SET_BIT(obj->extra_flags, ITEM_INVENTORY);
			obj = obj_to_char(obj, ch);
			send_to_char( "> &Gyou received an unfinished function&w\r\n", ch );
  			}
  		}

  	}

   }
   else
   send_to_char( "> revengi what?\r\n", ch );

   return;
}

void do_unload_cargo( CHAR_DATA *ch, char *argument)
{

   int cost;
   PLANET_DATA  *planet;

   if ( ch->pcdata->cargo == 0 )
   {
      send_to_char("> &Ryou don't have any data&w\n\r",ch);
      return;
   }

   if ( ch->in_room->sector_type == SECT_FARMLAND )
   {
      send_to_char("> &Ryou can't do that here&w\n\r", ch);
      return;
   }

   planet = ch->in_room->area->planet;

   if (planet->import[ch->pcdata->cargotype] < 1)
   {
      send_to_char("You can't deliver that here.\r\n",ch);
      return;
   }

   cost = ch->pcdata->cargo;
   cost *= planet->import[ch->pcdata->cargotype];

   ch->gold += cost;
   ch->pcdata->cargo = 0;
   ch_printf(ch,"You recieve %d credits for a load of %s.\r\n", cost, cargo_names[ch->pcdata->cargotype]);
   ch->pcdata->cargotype = CARGO_NONE;
   return;
}

void do_load_cargo( CHAR_DATA *ch, char *argument)
{
   int cost,cargo, i;
   PLANET_DATA  *planet;
   char arg1[MAX_INPUT_LENGTH];

   argument = one_argument(argument, arg1);

   //target = ship_in_room( ch->in_room , arg1 );

   if ( ch->in_room->sector_type == SECT_FARMLAND )
   {
      send_to_char("> &Ryou can't do that here&w\n\r", ch);
      return;
   }

   planet = ch->in_room->area->planet;

   if (ch->pcdata->maxcargo - ch->pcdata->cargo <= 0)
   {
      send_to_char("> no room for more data.\r\n", ch);
      return;
   }
   for(i = 1; i < CARGO_MAX; i++)
   {
      if(!strcmp(argument, cargo_names[i]))
         cargo = i;
   }

   if ((ch->pcdata->cargo > 0) && (ch->pcdata->cargotype != cargo))
   {
      send_to_char("> &deliver your current data&w\r\n",ch);
      return;
   }

   if (planet->export[cargo] < 1)
   {
      send_to_char("> resource not available here\r\n", ch);
      return;
   }

   if (planet->resource[cargo] < 1)
   {
      send_to_char("> &Rnone left for sale&w\r\n", ch);
      return;
   }

   if ((ch->pcdata->maxcargo - ch->pcdata->cargo) >= planet->resource[cargo])
      cost = planet->resource[cargo];
   else
      cost = (ch->pcdata->maxcargo - ch->pcdata->cargo);

   cost *= planet->export[cargo];

   if (ch->gold < cost)
   {
      send_to_char("You can't afford it!\r\n", ch);
      return;
   }
   ch->gold -= cost;

   if ((ch->pcdata->maxcargo - ch->pcdata->cargo) >= planet->resource[cargo])
   {

	 ch->pcdata->cargo += planet->resource[cargo];
     planet->resource[cargo] = 0;
     ch->pcdata->cargotype = cargo;
   }
   else
   {
     planet->resource[cargo] -= ch->pcdata->maxcargo - ch->pcdata->cargo;
     ch->pcdata->cargo = ch->pcdata->maxcargo;
     ch->pcdata->cargotype = cargo;
   }

   ch_printf(ch,"You pay %d credits for a load of %s.\r\n", cost, cargo_names[cargo]);
   return;
}

void do_inquire( CHAR_DATA * ch, char *argument )
{
   DESCRIPTOR_DATA *d;
   bool checkdata;
   OBJ_DATA *obj;
   int x;
//   long xpgain;
   char arg[MAX_INPUT_LENGTH];
   char buf[MAX_INPUT_LENGTH];
   int schance;

   strcpy( arg, argument );
   checkdata = FALSE;
   switch ( ch->substate )
   {
      default:

         for( obj = ch->last_carrying; obj; obj = obj->prev_content )
         {
            if( obj->item_type == ITEM_TOOLKIT )
               checkdata = TRUE;
         }

         if( ( checkdata == FALSE ) )
         {
            send_to_char( "> &Ryou need a devkit module to slice into the banking computer system&w\n\r", ch );
            return;
         }
         if( !IS_SET( ch->in_room->room_flags2, ROOM_INTRUSION ) )
         {
            send_to_char( "> &Ryou must be in an intrusion node&w\n\r", ch );
            return;
         }

         schance = IS_NPC( ch ) ? ch->top_level : ( int )( ch->pcdata->learned[gsn_inquire] );
         //schance = schance + ( ( ch->in_room->seccode + 1 ) * 10 );

         if( number_percent(  ) < schance )
         {

            send_to_char( "> &Gyou begin the long process of trying to slice into the banking computer system\n\r", ch );
            sprintf( buf, "$n takes $s browser and slots into a data port." );
            act( AT_PLAIN, buf, ch, NULL, argument, TO_ROOM );
            add_timer( ch, TIMER_DO_FUN, 10, do_inquire, 1 );
            return;

         }
         send_to_char( "> &Ryou are unable to find the banking computer system\n\r", ch );
         learn_from_failure( ch, gsn_inquire );
         return;

      case 1:
         break;
      case SUB_TIMER_DO_ABORT:
         ch->substate = SUB_NONE;
         send_to_char( "> &Ryou are interrupted and fail to finish slicing into the banking computer system\n\r", ch );
         return;
   }

   ch->substate = SUB_NONE;

   schance = IS_NPC( ch ) ? ch->top_level : ( int )( ch->pcdata->learned[gsn_inquire] );

   x = number_percent(  );

   if( number_percent(  ) > schance )
   {
      ch_printf( ch, "&z|+---------------------------------------+|&w\n\r" );
      ch_printf( ch, "&z|^g&x Welcome to the Bank Database.           ^x&z|\n\r" );
      ch_printf( ch, "&z|^g                                         ^x|\n\r" );
      ch_printf( ch, "&z|^g&x Login: %d                            ^x&z|\n\r",
                 number_range( 11111, 99999 ) );
      ch_printf( ch, "&z|^g&x Passcode: *********                     ^x&z|\n\r" );
      ch_printf( ch, "&z|^g                                         ^x|\n\r" );
      ch_printf( ch, "&z|^g&x Invalid passcode.                       ^x&z|\n\r" );
      ch_printf( ch, "&z|+---------------------------------------+|&w\n\r" );
      learn_from_failure( ch, gsn_inquire );

		if (ch->pcdata->threataction < 1) {
		send_to_char( "> &Wthreat status changed to: &btraced&w\n\r",        ch );
		ch->pcdata->threataction = 1;
		}

		ch->pcdata->threatlevel += 1;
		if ( ch->pcdata->threatlevel > 10 )
			ch->pcdata->threatlevel = 10;

      return;
   }

	  ch_printf( ch, "&z|+---------------------------------------+|&w\n\r" );
   ch_printf( ch, "&z|^g&x Welcome to the Bank Database.           ^x&z|\n\r" );
	  ch_printf( ch, "&z|^g                                         ^x|\n\r" );
   ch_printf( ch, "&z|^g&x Login: %d                            ^x&z|\n\r",
              number_range( 11111, 99999 ) );
   ch_printf( ch, "&z|^g&x Passcode: *********                     ^x&z|\n\r" );
   ch_printf( ch, "&z|^g                                         ^x|\n\r" );
   ch_printf( ch, "&z|^g&x Login accepted...retrieving account     ^x&z|\n\r" );
   ch_printf( ch, "&z|^g                                         ^x&z|\n\r" );
   ch_printf( ch, "&z|^g&x _______  Account  _______ Savings _____ &z^x|\n\r" );
   for( d = first_descriptor; d; d = d->next )
   {
      if( !d->character )
         continue;
      if( d->connected != CON_PLAYING )
         continue;
      if( IS_IMMORTAL( d->character ) )
         continue;
      ch_printf( ch, "&z|^g&x         # %s             %-9.9d     ^x&z|\n\r", d->character->name,
                 d->character->pcdata->bank );
   }
   ch_printf( ch, "&z|+---------------------------------------+|&w\n\r" );

   learn_from_success( ch, gsn_inquire );
   return;

}

void do_slicebank( CHAR_DATA * ch, char *argument )
{
   DESCRIPTOR_DATA *d;
   bool checkdata;
   OBJ_DATA *obj;
//   long xpgain;
//   char opfer, ort;
   char arg[MAX_INPUT_LENGTH];
   char arg2[MAX_INPUT_LENGTH];
   char buf[MAX_INPUT_LENGTH];
   long steal;
   int schance, x;
   bool found;

	if ( !IS_NPC(ch) && !ch->pcdata->learned[gsn_slicebank] )
	{
		send_to_char("> you do not know the slicebank skillsoft\n\r", ch );
		return;
	}

   if ( IS_AFFECTED(ch, AFF_HIDE) ){
       send_to_char( "> &Ryou can not slice bank account when hidden&w\n\r", ch );
       return;
   }

   argument = one_argument( argument, arg2 );
   strcpy( arg, argument );
   checkdata = FALSE;
   switch ( ch->substate )
   {
      default:
         if( arg[0] == '\0' || arg2[0] == '\0' )
         {
            send_to_char( "> &Wsyntax: slicebank <account> <amount>&w\n\r", ch );
            return;
         }

         if( ch->fighting )
         {
            send_to_char( "> &Ryou are fighting&w\n\r", ch );
            return;
         }


         if( !IS_SET( ch->in_room->room_flags2, ROOM_INTRUSION ) )
         {
            send_to_char( "> &Ryou must be in an intrusion node to slice someone's account&w\n\r", ch );
            return;
         }

         for( obj = ch->last_carrying; obj; obj = obj->prev_content )
         {
            if( obj->item_type == ITEM_TOOLKIT )
               checkdata = TRUE;
         }
         if( ( checkdata == FALSE ) )
         {
            send_to_char( "> &Ryou need a devkit module to slice into the banking computer system&w\n\r", ch );
            return;
         }
         if( !str_cmp( arg2, ch->name ) )
         {
            send_to_char( "> &Rthat's your account - insurance fraud is not possible\n\r", ch );
            return;
         }

         if( atoi( arg ) < 0 )
         {
            send_to_char( "> &Rwhy don't you just GIVE them the money?&w\n\r", ch );
            return;
         }

         ch->dest_buf = str_dup( arg );
         ch->dest_buf_2 = str_dup( arg2 );
         send_to_char( "> &GYou begin the long process of trying to slice into the banking computer system&w\n\r", ch );
         sprintf( buf, "> $n takes $s browser and hooks it into a data port" );
         act( AT_PLAIN, buf, ch, NULL, argument, TO_ROOM );
         add_timer( ch, TIMER_DO_FUN, 10, do_slicebank, 1 );
         return;

      case 1:
         if( !ch->dest_buf )
            return;
         if( !ch->dest_buf_2 )
            return;

         strcpy( arg, ch->dest_buf );
         strcpy( arg2, ch->dest_buf_2 );
         DISPOSE( ch->dest_buf );
         DISPOSE( ch->dest_buf_2 );
         break;

      case SUB_TIMER_DO_ABORT:
         DISPOSE( ch->dest_buf );
         DISPOSE( ch->dest_buf_2 );
         ch->substate = SUB_NONE;
         send_to_char( "> &Ryou are interrupted and fail to finish slicing into the banking computer system&w\n\r", ch );
         return;
   }

   ch->substate = SUB_NONE;

   for( obj = ch->last_carrying; obj; obj = obj->prev_content )
   {
      if( obj->item_type == ITEM_TOOLKIT )
         checkdata = TRUE;
   }

   schance = IS_NPC( ch ) ? ch->top_level : ( int )( ch->pcdata->learned[gsn_slicebank] );
   //schance = schance - ch->in_room->seccode;
      schance = UMIN( schance, 70 );
   found = FALSE;

   for( d = first_descriptor; d; d = d->next )
   {
      if( !d->character )
         continue;
      if( d->connected != CON_PLAYING )
         continue;
      if( IS_IMMORTAL( d->character ) )
         continue;

      if( !str_cmp( arg2, d->character->name ) )
      {
         found = TRUE;
         break;
      }
   }

   x = number_percent(  );

   if( x > schance )
   {
      ch_printf( ch, "&z|+---------------------------------------+|&w\n\r" );
      ch_printf( ch, "&z|^g&x Welcome to the Bank Database.           ^x&z|\n\r" );
      ch_printf( ch, "&z|^g                                         ^x|\n\r" );
      ch_printf( ch, "&z|^g&x Login: %d                            ^x&z|\n\r",
                 number_range( 11111, 99999 ) );
      ch_printf( ch, "&z|^g&x Passcode: *********                     ^x&z|\n\r" );
      ch_printf( ch, "&z|^g                                         ^x|\n\r" );
      ch_printf( ch, "&z|^g&x Invalid passcode.                       ^x&z|\n\r" );
      ch_printf( ch, "&z|+---------------------------------------+|&w\n\r" );
      learn_from_failure( ch, gsn_slicebank );

      if (found) {
          send_to_char( "> &R[&YBank: &WALERT&R] &WA hack attempt was made on your bank account\n\r", d->character );
          ch_printf( d->character, "> &R[&YBank: &WALERT&R] &WFrom Complex: %s - node: %ld\n\r", ch->in_room->area->planet->name, ch->in_room->vnum );
      }

		if (ch->pcdata->threataction < 1) {
		send_to_char( "> &Wthreat status changed to: &btraced&w\n\r",        ch );
		ch->pcdata->threataction = 1;
		}

		ch->pcdata->threatlevel += 1;
		if ( ch->pcdata->threatlevel > 10 )
			ch->pcdata->threatlevel = 10;

      return;
   }

   if( !found )
   {
      ch_printf( ch, "&z|+---------------------------------------+|&w\n\r" );
      ch_printf( ch, "&z|^g&x Welcome to the Bank Database.           ^x&z|\n\r" );
      ch_printf( ch, "&z|^g                                         ^x|\n\r" );
      ch_printf( ch, "&z|^g&x Login: %d                            ^x&z|\n\r",
                 number_range( 11111, 99999 ) );
      ch_printf( ch, "&z|^g&x Passcode: *********                     ^x&z|\n\r" );
      ch_printf( ch, "&z|^g                                         ^x|\n\r" );
      ch_printf( ch, "&z|^g&x Login accepted...retrieving account     ^x&z|\n\r" );
      ch_printf( ch, "&z|^g                                         ^x&z|\n\r" );
      ch_printf( ch, "&z|^g&x Account %-15.15s is not active.  ^x&z|\n\r", arg2 );
      ch_printf( ch, "&z|+---------------------------------------+|&w\n\r" );
      return;
   }

   steal = atoi( arg );

   if( steal == 1 && d->character->pcdata->bank > 0 )
   {
      ch_printf( ch, "&z|+---------------------------------------+|&w\n\r" );
      ch_printf( ch, "&z|^g&x Welcome to the Bank Database.           ^x&z|\n\r" );
      ch_printf( ch, "&z|^g                                         ^x|\n\r" );
      ch_printf( ch, "&z|^g&x Login: %d                            ^x&z|\n\r",
                 number_range( 11111, 99999 ) );
      ch_printf( ch, "&z|^g&x Passcode: *********                     ^x&z|\n\r" );
      ch_printf( ch, "&z|^g                                         ^x|\n\r" );
      ch_printf( ch, "&z|^g&x Login accepted...retrieving account     ^x&z|\n\r" );
      ch_printf( ch, "&z|^g&x Processing request, stand by.           ^x&z|\n\r" );
      ch_printf( ch, "&z|^g                                         ^x&z|\n\r" );
      ch_printf( ch, "&z|^g&x Request ACCEPTED.                   ^x&z|\n\r" );
      ch_printf( ch, "&z|+---------------------------------------+|&w\n\r" );

      ch->gold += steal;
      d->character->pcdata->bank -= steal;
      if (number_range(1, 10) == 1)
      learn_from_success( ch, gsn_slicebank );
      return;
   }

   if( steal > d->character->pcdata->bank / 1000 )
   {
      ch_printf( ch, "&z|+---------------------------------------+|&w\n\r" );
      ch_printf( ch, "&z|^g&x Welcome to the Bank Database.           ^x&z|\n\r" );
      ch_printf( ch, "&z|^g                                         ^x|\n\r" );
      ch_printf( ch, "&z|^g&x Login: %d                            ^x&z|\n\r",
                 number_range( 11111, 99999 ) );
      ch_printf( ch, "&z|^g&x Passcode: *********                     ^x&z|\n\r" );
      ch_printf( ch, "&z|^g                                         ^x|\n\r" );
      ch_printf( ch, "&z|^g&x Login accepted...retrieving account     ^x&z|\n\r" );
      ch_printf( ch, "&z|^g&x Processing request, stand by.           ^x&z|\n\r" );
      ch_printf( ch, "&z|^g                                         ^x&z|\n\r" );
      ch_printf( ch, "&z|^g&x Request DENIED, transfer too high.   ^x&z|\n\r" );
      ch_printf( ch, "&z|+---------------------------------------+|&w\n\r" );
      learn_from_failure( ch, gsn_slicebank );

      if (found) {
	  send_to_char( "> &R[&YBank: &WALERT&R] &WA hack attempt was made on your bank account.\n\r", d->character );
      ch_printf( d->character, "> &R[&YBank: &WALERT&R] &WFrom Complex: %s - node: %ld\n\r", ch->in_room->area->planet->name,  ch->in_room->vnum );
      }

		if (ch->pcdata->threataction < 1) {
		send_to_char( "> &Wthreat status changed to: &btraced&w\n\r",        ch );
		ch->pcdata->threataction = 1;
		}

		ch->pcdata->threatlevel += 1;
		if ( ch->pcdata->threatlevel > 10 )
			ch->pcdata->threatlevel = 10;

      return;
   }

   ch_printf( ch, "&z|+---------------------------------------+|&w\n\r" );
   ch_printf( ch, "&z|^g&x Welcome to the Bank Database.           ^x&z|\n\r" );
   ch_printf( ch, "&z|^g                                         ^x|\n\r" );
   ch_printf( ch, "&z|^g&x Login: %d                            ^x&z|\n\r",
              number_range( 11111, 99999 ) );
   ch_printf( ch, "&z|^g&x Passcode: *********                     ^x&z|\n\r" );
   ch_printf( ch, "&z|^g                                         ^x|\n\r" );
   ch_printf( ch, "&z|^g&x Login accepted...retrieving account     ^x&z|\n\r" );
   ch_printf( ch, "&z|^g&x Processing request, stand by.           ^x&z|\n\r" );
   ch_printf( ch, "&z|^g                                         ^x&z|\n\r" );
   ch_printf( ch, "&z|^g&x Request accepted. Credits transferred   ^x&z|\n\r" );
   ch_printf( ch, "&z|+---------------------------------------+|&w\n\r" );

   ch->gold += steal;
   d->character->pcdata->bank -= steal;
   learn_from_success( ch, gsn_slicebank );
   return;

}

void do_slicefund( CHAR_DATA * ch, char *argument )
{
  // DESCRIPTOR_DATA *d;
   bool checkdata;
   OBJ_DATA *obj;
   CLAN_DATA *clan;
//   long xpgain;
//   char opfer, ort;
   char arg[MAX_INPUT_LENGTH];
   char arg2[MAX_INPUT_LENGTH];
   char buf[MAX_INPUT_LENGTH];
   char bufa[MAX_STRING_LENGTH];
   long steal;
   int schance, x;
   bool found;

	if ( !IS_NPC(ch) && !ch->pcdata->learned[gsn_slicefund] )
	{
		send_to_char("> you do not know the slicefund skillsoft\n\r", ch );
		return;
	}

   if ( IS_AFFECTED(ch, AFF_HIDE) ){
       send_to_char( "> &Ryou can not slice organization funds when hidden&w\n\r", ch );
       return;
   }

   argument = one_argument( argument, arg2 );
   strcpy( arg, argument );
   checkdata = FALSE;
   switch ( ch->substate )
   {
      default:
         if( arg[0] == '\0' || arg2[0] == '\0' )
         {
            send_to_char( "> &Wsyntax: slicefund <organization> <amount>&w\n\r", ch );
            return;
         }

         if( ch->fighting )
         {
            send_to_char( "> &Ryou are fighting&w\n\r", ch );
            return;
         }


         if( !IS_SET( ch->in_room->room_flags2, ROOM_INTRUSION ) )
         {
            send_to_char( "> &Ryou must be in an intrusion node to slice organization funds&w\n\r", ch );
            return;
         }

         for( obj = ch->last_carrying; obj; obj = obj->prev_content )
         {
            if( obj->item_type == ITEM_TOOLKIT )
               checkdata = TRUE;
         }
         if( ( checkdata == FALSE ) )
         {
            send_to_char( "> &Ryou need a devkit module to slice into the banking computer system&w\n\r", ch );
            return;
         }

         if( !str_cmp( arg2, ch->pcdata->clan_name ) )
         {
            send_to_char( "> &Rthat's your organization - insurance fraud is not possible\n\r", ch );
            return;
         }

         clan = get_clan( arg2 );
         if ( !clan )
         {
     	send_to_char( "> &Rno such organization&w\n\r", ch );
     	return;
         }

         if( atoi( arg ) < 0 )
         {
            send_to_char( "> &Rwhy don't you just GIVE them the money?&w\n\r", ch );
            return;
         }

         ch->dest_buf = str_dup( arg );
         ch->dest_buf_2 = str_dup( arg2 );
         send_to_char( "> &GYou begin the long process of trying to slice into the banking computer system&w\n\r", ch );
         sprintf( buf, "> $n takes $s browser and hooks it into a data port" );
         act( AT_PLAIN, buf, ch, NULL, argument, TO_ROOM );
         add_timer( ch, TIMER_DO_FUN, 10, do_slicefund, 1 );
         return;

      case 1:
         if( !ch->dest_buf )
            return;
         if( !ch->dest_buf_2 )
            return;

         strcpy( arg, ch->dest_buf );
         strcpy( arg2, ch->dest_buf_2 );
         DISPOSE( ch->dest_buf );
         DISPOSE( ch->dest_buf_2 );
         break;

      case SUB_TIMER_DO_ABORT:
         DISPOSE( ch->dest_buf );
         DISPOSE( ch->dest_buf_2 );
         ch->substate = SUB_NONE;
         send_to_char( "> &Ryou are interrupted and fail to finish slicing into the banking computer system&w\n\r", ch );
         return;
   }

   ch->substate = SUB_NONE;

   for( obj = ch->last_carrying; obj; obj = obj->prev_content )
   {
      if( obj->item_type == ITEM_TOOLKIT )
         checkdata = TRUE;
   }

   schance = IS_NPC( ch ) ? ch->top_level : ( int )( ch->pcdata->learned[gsn_slicefund] );
   //schance = schance - ch->in_room->seccode;
      schance = UMIN( schance, 70 );
   found = FALSE;

   clan = get_clan( arg2 );
   if ( !clan )
   {
	send_to_char( "> &Rno such organization&w\n\r", ch );
	return;
   }

   x = number_percent(  );

   if( x > schance )
   {
      ch_printf( ch, "&z|+---------------------------------------+|&w\n\r" );
      ch_printf( ch, "&z|^g&x Welcome to the Bank Database.           ^x&z|\n\r" );
      ch_printf( ch, "&z|^g                                         ^x|\n\r" );
      ch_printf( ch, "&z|^g&x Login: %d                            ^x&z|\n\r",
                 number_range( 11111, 99999 ) );
      ch_printf( ch, "&z|^g&x Passcode: *********                     ^x&z|\n\r" );
      ch_printf( ch, "&z|^g                                         ^x|\n\r" );
      ch_printf( ch, "&z|^g&x Invalid passcode.                       ^x&z|\n\r" );
      ch_printf( ch, "&z|+---------------------------------------+|&w\n\r" );
      learn_from_failure( ch, gsn_slicefund );

		sprintf(bufa, "> &R[&YBank: &WALERT&R] &WORG fund intrusion from: %s - node: %ld",
				ch->in_room->area->planet->name, ch->in_room->vnum);

		echo_to_clan(AT_RED, bufa, ECHOTAR_ALL, clan);

		if (ch->pcdata->threataction < 1) {
		send_to_char( "> &Wthreat status changed to: &btraced&w\n\r",        ch );
		ch->pcdata->threataction = 1;
		}

		ch->pcdata->threatlevel += 1;
		if ( ch->pcdata->threatlevel > 10 )
			ch->pcdata->threatlevel = 10;

      return;
   }

   if( !clan )
   {
      ch_printf( ch, "&z|+---------------------------------------+|&w\n\r" );
      ch_printf( ch, "&z|^g&x Welcome to the Bank Database.           ^x&z|\n\r" );
      ch_printf( ch, "&z|^g                                         ^x|\n\r" );
      ch_printf( ch, "&z|^g&x Login: %d                            ^x&z|\n\r",
                 number_range( 11111, 99999 ) );
      ch_printf( ch, "&z|^g&x Passcode: *********                     ^x&z|\n\r" );
      ch_printf( ch, "&z|^g                                         ^x|\n\r" );
      ch_printf( ch, "&z|^g&x Login accepted...retrieving account     ^x&z|\n\r" );
      ch_printf( ch, "&z|^g                                         ^x&z|\n\r" );
      ch_printf( ch, "&z|^g&x Account %-15.15s is not active.  ^x&z|\n\r", arg2 );
      ch_printf( ch, "&z|+---------------------------------------+|&w\n\r" );
      return;
   }

   steal = atoi( arg );

   if( steal == 1 && clan->funds > 0 )
   {
      ch_printf( ch, "&z|+---------------------------------------+|&w\n\r" );
      ch_printf( ch, "&z|^g&x Welcome to the Bank Database.           ^x&z|\n\r" );
      ch_printf( ch, "&z|^g                                         ^x|\n\r" );
      ch_printf( ch, "&z|^g&x Login: %d                            ^x&z|\n\r",
                 number_range( 11111, 99999 ) );
      ch_printf( ch, "&z|^g&x Passcode: *********                     ^x&z|\n\r" );
      ch_printf( ch, "&z|^g                                         ^x|\n\r" );
      ch_printf( ch, "&z|^g&x Login accepted...retrieving account     ^x&z|\n\r" );
      ch_printf( ch, "&z|^g&x Processing request, stand by.           ^x&z|\n\r" );
      ch_printf( ch, "&z|^g                                         ^x&z|\n\r" );
      ch_printf( ch, "&z|^g&x Request ACCEPTED.                    ^x&z|\n\r" );
      ch_printf( ch, "&z|+---------------------------------------+|&w\n\r" );

      ch->gold += steal;
      clan->funds -= steal;

     if (number_range(1, 10) == 1)
      learn_from_success( ch, gsn_slicefund );

      return;
   }

   if( steal > clan->funds / 1000 )
   {
      ch_printf( ch, "&z|+---------------------------------------+|&w\n\r" );
      ch_printf( ch, "&z|^g&x Welcome to the Bank Database.           ^x&z|\n\r" );
      ch_printf( ch, "&z|^g                                         ^x|\n\r" );
      ch_printf( ch, "&z|^g&x Login: %d                            ^x&z|\n\r",
                 number_range( 11111, 99999 ) );
      ch_printf( ch, "&z|^g&x Passcode: *********                     ^x&z|\n\r" );
      ch_printf( ch, "&z|^g                                         ^x|\n\r" );
      ch_printf( ch, "&z|^g&x Login accepted...retrieving account     ^x&z|\n\r" );
      ch_printf( ch, "&z|^g&x Processing request, stand by.           ^x&z|\n\r" );
      ch_printf( ch, "&z|^g                                         ^x&z|\n\r" );
      ch_printf( ch, "&z|^g&x Request DENIED, transfer too high.   ^x&z|\n\r" );
      ch_printf( ch, "&z|+---------------------------------------+|&w\n\r" );
      learn_from_failure( ch, gsn_slicefund );

		sprintf(bufa, "> &R[&YBank: &WALERT&R] &WORG fund intrusion from: %s - node: %ld",
				ch->in_room->area->planet->name, ch->in_room->vnum);

		echo_to_clan(AT_RED, bufa, ECHOTAR_ALL, clan);

		if (ch->pcdata->threataction < 1) {
		send_to_char( "> &Wthreat status changed to: &btraced&w\n\r",        ch );
		ch->pcdata->threataction = 1;
		}

		ch->pcdata->threatlevel += 1;
		if ( ch->pcdata->threatlevel > 10 )
			ch->pcdata->threatlevel = 10;

      return;
   }

   ch_printf( ch, "&z|+---------------------------------------+|&w\n\r" );
   ch_printf( ch, "&z|^g&x Welcome to the Bank Database.           ^x&z|\n\r" );
   ch_printf( ch, "&z|^g                                         ^x|\n\r" );
   ch_printf( ch, "&z|^g&x Login: %d                            ^x&z|\n\r",
              number_range( 11111, 99999 ) );
   ch_printf( ch, "&z|^g&x Passcode: *********                     ^x&z|\n\r" );
   ch_printf( ch, "&z|^g                                         ^x|\n\r" );
   ch_printf( ch, "&z|^g&x Login accepted...retrieving account     ^x&z|\n\r" );
   ch_printf( ch, "&z|^g&x Processing request, stand by.           ^x&z|\n\r" );
   ch_printf( ch, "&z|^g                                         ^x&z|\n\r" );
   ch_printf( ch, "&z|^g&x Request accepted. Credits transferred   ^x&z|\n\r" );
   ch_printf( ch, "&z|+---------------------------------------+|&w\n\r" );

   ch->gold += steal;
   clan->funds -= steal;
   learn_from_success( ch, gsn_slicefund );
   return;

}

void do_nodeupgrade( CHAR_DATA *ch, char *argument )
{
	//char buf [MAX_STRING_LENGTH];
	ROOM_INDEX_DATA	*location;
	CLAN_DATA *clan;
	PLANET_DATA *planet;
	int level, cost = 0, newlevel;

	location = ch->in_room;
	clan = ch->pcdata->clan;
	planet = ch->in_room->area->planet;

	if ( !IS_SET( planet->flags, PLANET_NOCAP ) )
	{

		if ( !clan )
		{
			send_to_char( "> you need to be part of an organization before you can do that\n\r", ch );
			return;
		}

		if ( (ch->pcdata && ch->pcdata->bestowments
				&&    is_name("build", ch->pcdata->bestowments))
				|| nifty_is_name( ch->name, clan->leaders  ) )
			;
		else
		{
			send_to_char( "> your organization has not given you permission to modify their systems\n\r", ch );
			return;
		}

		if ( !location->area || !location->area->planet ||
				clan != location->area->planet->governed_by  )
		{
			send_to_char( "> you may only modify nodes in systems that your organization controls\n\r", ch );
			return;
		}

	}

	if( strcmp(location->owner, ch->name) )
	{
		send_to_char( "&R> this is not your node&w\n\r", ch );
		return;
	}

	if ( IS_SET( location->room_flags , ROOM_NOPEDIT ) )
	{
		send_to_char( "> you may not edit this node\n\r", ch );
		return;
	}

	level = location->level;

    switch( level )
    {
    default:
    	send_to_char("> &Rnode already at maximum level&w\n\r", ch );
    	return;
    break;
    case 0:  cost = 2000;	break;
    case 1:  cost = 4000;	break;
    case 2:  cost = 8000;	break;
    case 3:  cost = 16000;	break;
    case 4:  cost = 32000;	break;
    }

    if ( ch->gold < cost )
    {
    	ch_printf( ch, "> &Rinsufficient credits - costs:&W %d&w\n\r", cost );
    	return;
    }

	newlevel = level + 1;
	location->level = newlevel;
    ch->gold -= cost;

    switch( newlevel )
    {
    default:
    send_to_char("> &Rsomething went wrong (contact Wintermute)&w\n\r", ch );
    return;
    break;

    case 1:
    send_to_char("> &Wnode upgraded to:&w &Ggreen&w\n\r", ch );
    location->level = 1;

		switch( location->sector_type) {

		default:
			break;

		case SECT_DESERT:
				location->area->planet->citysize++; //++;
		break;

		case SECT_FARMLAND:
				location->area->planet->farmland++; //++;
		break;

		case SECT_GLACIAL:
				location->area->planet->wilderness++; //++;
		break;

		case SECT_FIELD:
			location->area->planet->entertain_plus++;
			location->area->planet->multimedia_minus++;
			break;

		case SECT_FOREST:
				location->area->planet->multimedia_plus++;
				location->area->planet->entertain_minus++;
			break;

		case SECT_HILLS:
				location->area->planet->finance_plus++;
				location->area->planet->product_minus++;
				break;

		case SECT_SCRUB:
				location->area->planet->product_plus++;
				location->area->planet->finance_minus++;
		break;

		}

    break;
    case 2:
    send_to_char("> &Wnode upgraded to:&w &Oorange&w\n\r", ch );
    location->level = 2;

	switch( location->sector_type) {

	default:
		break;

	case SECT_DESERT:
			location->area->planet->citysize += 2;
	break;

	case SECT_FARMLAND:
			location->area->planet->farmland += 2;
	break;

	case SECT_GLACIAL:
			location->area->planet->wilderness += 2;
	break;

	case SECT_FIELD:
		location->area->planet->entertain_plus += 2;
		location->area->planet->multimedia_minus += 2;
		break;

	case SECT_FOREST:
			location->area->planet->multimedia_plus += 2;
			location->area->planet->entertain_minus += 2;
		break;

	case SECT_HILLS:
			location->area->planet->finance_plus += 2;
			location->area->planet->product_minus += 2;
			break;

	case SECT_SCRUB:
			location->area->planet->product_plus += 2;
			location->area->planet->finance_minus += 2;
	break;

	}

    break;
    case 3:
    send_to_char("> &Wnode upgraded to:&w &Rred&w\n\r", ch );
    location->level = 3;

	switch( location->sector_type) {

	default:
		break;

	case SECT_DESERT:
			location->area->planet->citysize += 4;
	break;

	case SECT_FARMLAND:
			location->area->planet->farmland += 4;
	break;

	case SECT_GLACIAL:
			location->area->planet->wilderness += 4;
	break;

	case SECT_FIELD:
		location->area->planet->entertain_plus += 4;
		location->area->planet->multimedia_minus += 4;
		break;

	case SECT_FOREST:
			location->area->planet->multimedia_plus += 4;
			location->area->planet->entertain_minus += 4;
		break;

	case SECT_HILLS:
			location->area->planet->finance_plus += 4;
			location->area->planet->product_minus += 4;
			break;

	case SECT_SCRUB:
			location->area->planet->product_plus += 4;
			location->area->planet->finance_minus += 4;
	break;

	}

	break;
    case 4:
    send_to_char("> &Wnode upgraded to:&w &pultra-violet&w\n\r", ch );
    location->level = 4;

	switch( location->sector_type) {

	default:
		break;

	case SECT_DESERT:
			location->area->planet->citysize += 8;
	break;

	case SECT_FARMLAND:
			location->area->planet->farmland += 8;
	break;

	case SECT_GLACIAL:
			location->area->planet->wilderness += 8;
	break;

	case SECT_FIELD:
		location->area->planet->entertain_plus += 8;
		location->area->planet->multimedia_minus += 8;
		break;

	case SECT_FOREST:
			location->area->planet->multimedia_plus += 8;
			location->area->planet->entertain_minus += 8;
		break;

	case SECT_HILLS:
			location->area->planet->finance_plus += 8;
			location->area->planet->product_minus += 8;
			break;

	case SECT_SCRUB:
			location->area->planet->product_plus += 8;
			location->area->planet->finance_minus += 8;
	break;

	}


    break;
    case 5:
    send_to_char("> &Wnode upgraded to:&w &zblack&w\n\r", ch );
    location->level = 5;

	switch( location->sector_type) {

	default:
		break;

	case SECT_DESERT:
			location->area->planet->citysize += 16;
	break;

	case SECT_FARMLAND:
			location->area->planet->farmland += 16;
	break;

	case SECT_GLACIAL:
			location->area->planet->wilderness += 16;
	break;

	case SECT_FIELD:
		location->area->planet->entertain_plus += 16;
		location->area->planet->multimedia_minus += 16;
		break;

	case SECT_FOREST:
			location->area->planet->multimedia_plus += 16;
			location->area->planet->entertain_minus += 16;
		break;

	case SECT_HILLS:
			location->area->planet->finance_plus += 16;
			location->area->planet->product_minus += 16;
			break;

	case SECT_SCRUB:
			location->area->planet->product_plus += 16;
			location->area->planet->finance_minus += 16;
	break;

	}


    break;
    case 6:
    send_to_char("> &Rnode already at maximum level&w\n\r", ch );
    return;
    break;
    }

    SET_BIT( location->area->flags , AFLAG_MODIFIED );
	return;

}

void do_sresources( CHAR_DATA *ch, char *argument )
{
    PLANET_DATA *planet;
    int entertainmax, multimediamax, financemax, productmax;

    if ( IS_NPC( ch ) )
    {
	send_to_char( "Huh?\n\r", ch );
	return;
    }

    if ( argument[0] == '\0' )
    {
	send_to_char( "> &Rsyntax: sresources <system>&w\n\r", ch );
	send_to_char( "  &Wshows info about specified system's resources&w\n\r", ch );
	return;
    }

    planet = get_planet( argument );
    if ( !planet )
    {
	send_to_char( "> no such system\n\r", ch );
	return;
    }

    if( !IS_IMMORTAL( ch ) && ( IS_SET( planet->flags, PLANET_HIDDEN ) || IS_SET( planet->flags, PLANET_EXPLORABLE) || IS_SET( planet->flags, PLANET_NOCAP) ) )
    {
	send_to_char( "> no such system\n\r", ch );
	return;
    }

	entertainmax = planet->entertain_plus - planet->entertain_minus;
	multimediamax = planet->multimedia_plus - planet->multimedia_minus;
	financemax = planet->finance_plus - planet->finance_minus;
	productmax = planet->product_plus - planet->product_minus;

    ch_printf( ch, "&W%s\n\r", planet->name);
    ch_printf( ch, "&Wowner: &G%s\n\r",
                   planet->governed_by ? planet->governed_by->name : "[neutral]" );
    ch_printf( ch, "&W--repositories---------------------------------------&w\n\r");
    ch_printf( ch, "&Wentertainment : %-12d multimedia : %d\n\r", planet->entertain_amount, planet->multimedia_amount );
    ch_printf( ch, "&Wfinance       : %-12d product    : %d\n\r", planet->finance_amount, planet->product_amount );
    ch_printf( ch, "&W--produce/consume------------------------------------&w\n\r");
    ch_printf( ch, "&Wentertainment : %-12d multimedia : %d\n\r", entertainmax, multimediamax );
    ch_printf( ch, "&Wfinance       : %-12d product    : %d\n\r", financemax, productmax );
    ch_printf( ch, "&W--market---------------------------------------------&w\n\r");
    ch_printf( ch, "&Wenter-buy     : %-12d enter-sell : %d\n\r", planet->entertain_buyprice, planet->entertain_sellprice );
    ch_printf( ch, "&Wmulti-buy     : %-12d multi-sell : %d\n\r", planet->multimedia_buyprice, planet->multimedia_sellprice );
    ch_printf( ch, "&Wfinan-buy     : %-12d finan-sell : %d\n\r", planet->finance_buyprice, planet->finance_sellprice );
    ch_printf( ch, "&Wprodu-buy     : %-12d produ-sell : %d\n\r", planet->product_buyprice, planet->product_sellprice );
    ch_printf( ch, "&W--limits---------------------------------------------&w\n\r");
    ch_printf( ch, "&Wenter-min     : %-12d enter-max  : %d\n\r", planet->entertain_min, planet->entertain_max );
    ch_printf( ch, "&Wmulti-min     : %-12d multi-max  : %d\n\r", planet->multimedia_min, planet->multimedia_max );
    ch_printf( ch, "&Wfinan-min     : %-12d finan-max  : %d\n\r", planet->finance_min, planet->finance_max );
    ch_printf( ch, "&Wprodu-min     : %-12d produ-max  : %d\n\r", planet->product_min, planet->product_max );
    return;
}

void do_cy_rsell( CHAR_DATA *ch, char *argument )
{

	ROOM_INDEX_DATA	*location;
	char arg[MAX_INPUT_LENGTH];
	char arg1[MAX_INPUT_LENGTH];
	int restype, entertain_buy, entertain_max, entertain_amount;
	int multimedia_buy, multimedia_max, multimedia_amount;
	int finance_buy, finance_max, finance_amount;
	int product_buy, product_max, product_amount;

	argument = one_argument( argument , arg );
	argument = one_argument( argument , arg1 );

	if ( arg[0] == '\0' )
	{
		send_to_char("> &Rsyntax: rsell [type] [amount]&w\n\r", ch);
		send_to_char("> &Wsells specified resource amount to current system&w\n\r", ch);
		send_to_char("> &Wtypes: ente, mult, fina, prod&w\n\r", ch);
		return;
	}

	if ( atoi(arg1) <= 0){
		send_to_char("> &Ramount must be greater than 0&w\n\r", ch);
		return;
	}

	location = ch->in_room;

	if (!location->area->planet->governed_by){
		send_to_char("> &Rthis system does not buy resources&w\n\r", ch);
		return;
	}

	entertain_amount = location->area->planet->entertain_amount;
	multimedia_amount = location->area->planet->multimedia_amount;
	finance_amount = location->area->planet->finance_amount;
	product_amount = location->area->planet->product_amount;

	entertain_max = location->area->planet->entertain_max;
	multimedia_max = location->area->planet->multimedia_max;
	finance_max = location->area->planet->finance_max;
	product_max = location->area->planet->product_max;

	if ( !str_cmp( arg, "ente") )
			restype = 1;
	else if ( !str_cmp( arg, "mult") )
			restype = 2;
	else if ( !str_cmp( arg, "fina") )
			restype = 3;
	else if ( !str_cmp( arg, "prod") )
			restype = 4;
	else
			restype = 0;

	switch (restype){

	default:
		do_cy_rsell(ch, "");
		return;
	break;

	case 1: // entertainment

		if ( (entertain_amount + atoi(arg1)) > entertain_max ) {
			send_to_char( "> &Rsystem does not buy that much of this resource&w\n\r" , ch );
			return;
		}

		if ( (ch->pcdata->rentertain - atoi(arg1)) < 0){
			send_to_char("> &Ryou do not have that much&w\n\r", ch);
			return;
		}

		entertain_buy = location->area->planet->entertain_buyprice;

		if ( (entertain_buy * atoi(arg1)) > location->area->planet->governed_by->funds ){
			send_to_char("> &Rsyntax: organization has not enough funds&w\n\r", ch);
			return;
		}

		ch->gold += (atoi(arg1) * entertain_buy);
		ch->pcdata->rentertain -= atoi(arg1);

		location->area->planet->governed_by->funds -= (atoi(arg1) * entertain_buy);
		location->area->planet->entertain_amount += atoi(arg1);

		ch_printf( ch, "&W> you sold %d entertainment source for %d credits &w\n\r", atoi(arg1), atoi(arg1) * entertain_buy );

		break;

	case 2: // multimedia

		if ( (multimedia_amount + atoi(arg1)) > multimedia_max ) {
			send_to_char( "> &Rsystem does not buy that much of this resource&w\n\r" , ch );
			return;
		}

		if ( (ch->pcdata->rmultimedia - atoi(arg1)) < 0){
			send_to_char("> &Ryou do not have that much&w\n\r", ch);
			return;
		}

		multimedia_buy = location->area->planet->multimedia_buyprice;

		if ( (multimedia_buy * atoi(arg1)) > location->area->planet->governed_by->funds ){
			send_to_char("> &Rsyntax: organization has not enough funds&w\n\r", ch);
			return;
		}

		ch->gold += (atoi(arg1) * multimedia_buy);
		ch->pcdata->rmultimedia -= atoi(arg1);

		location->area->planet->governed_by->funds -= (atoi(arg1) * multimedia_buy);
		location->area->planet->multimedia_amount += atoi(arg1);

		ch_printf( ch, "&W> you sold %d multimedia source for %d credits &w\n\r", atoi(arg1), atoi(arg1) * multimedia_buy );

		break;

	case 3:  // finance

		if ( (finance_amount + atoi(arg1)) > finance_max ) {
			send_to_char( "> &Rsystem does not buy that much of this resource&w\n\r" , ch );
			return;
		}

		if ( (ch->pcdata->rfinance - atoi(arg1)) < 0){
			send_to_char("> &Ryou do not have that much&w\n\r", ch);
			return;
		}

		finance_buy = location->area->planet->finance_buyprice;

		if ( (finance_buy * atoi(arg1)) > location->area->planet->governed_by->funds ){
			send_to_char("> &Rsyntax: organization has not enough funds&w\n\r", ch);
			return;
		}

		ch->gold += (atoi(arg1) * finance_buy);
		ch->pcdata->rfinance -= atoi(arg1);

		location->area->planet->governed_by->funds -= (atoi(arg1) * finance_buy);
		location->area->planet->finance_amount += atoi(arg1);

		ch_printf( ch, "&W> you sold %d finance source for %d credits &w\n\r", atoi(arg1), atoi(arg1) * finance_buy );

		break;

	case 4:  // product

		if ( (product_amount + atoi(arg1)) > product_max ) {
			send_to_char( "> &Rsystem does not buy that much of this resource&w\n\r" , ch );
			return;
		}

		if ( (ch->pcdata->rproduct - atoi(arg1)) < 0){
			send_to_char("> &Ryou do not have that much&w\n\r", ch);
			return;
		}

		product_buy = location->area->planet->product_buyprice;

		if ( (product_buy * atoi(arg1)) > location->area->planet->governed_by->funds ){
			send_to_char("> &Rsyntax: organization has not enough funds&w\n\r", ch);
			return;
		}

		ch->gold += (atoi(arg1) * product_buy);
		ch->pcdata->rproduct -= atoi(arg1);

		location->area->planet->governed_by->funds -= (atoi(arg1) * product_buy);
		location->area->planet->product_amount += atoi(arg1);

		ch_printf( ch, "&W> you sold %d productivity source for %d credits &w\n\r", atoi(arg1), atoi(arg1) * product_buy );

		break;
	}

	return;

}

void do_cy_rbuy( CHAR_DATA *ch, char *argument )
{

	ROOM_INDEX_DATA	*location;
	char arg[MAX_INPUT_LENGTH];
	char arg1[MAX_INPUT_LENGTH];
	int restype, entertain_buy, entertain_max, entertain_amount;
	int multimedia_buy, multimedia_max, multimedia_amount;
	int finance_buy, finance_max, finance_amount;
	int product_buy, product_max, product_amount;

	argument = one_argument( argument , arg );
	argument = one_argument( argument , arg1 );

	if ( arg[0] == '\0' )
	{
		send_to_char("> &Rsyntax: rbuy [type] [amount]&w\n\r", ch);
		send_to_char("> &Wsells specified resource amount to current system&w\n\r", ch);
		send_to_char("> &Wtypes: ente, mult, fina, prod&w\n\r", ch);
		return;
	}

	if ( atoi(arg1) <= 0){
		send_to_char("> &Ramount must be greater than 0&w\n\r", ch);
		return;
	}

	location = ch->in_room;

	if (!location->area->planet->governed_by){
		send_to_char("> &Rthis system does not sell resources&w\n\r", ch);
		return;
	}

	entertain_amount = location->area->planet->entertain_amount;
	multimedia_amount = location->area->planet->multimedia_amount;
	finance_amount = location->area->planet->finance_amount;
	product_amount = location->area->planet->product_amount;

	entertain_max = location->area->planet->entertain_min;
	multimedia_max = location->area->planet->multimedia_min;
	finance_max = location->area->planet->finance_min;
	product_max = location->area->planet->product_min;

	if ( !str_cmp( arg, "ente") )
			restype = 1;
	else if ( !str_cmp( arg, "mult") )
			restype = 2;
	else if ( !str_cmp( arg, "fina") )
			restype = 3;
	else if ( !str_cmp( arg, "prod") )
			restype = 4;
	else
			restype = 0;

	switch (restype){

	default:
		do_cy_rbuy(ch, "");
		return;
	break;

	case 1: // entertainment

		if ( (entertain_amount - atoi(arg1)) < entertain_max ) {
			send_to_char( "> &Rsystem does not sell that much of this resource&w\n\r" , ch );
			return;
		}

		entertain_buy = location->area->planet->entertain_sellprice;

		if ( (entertain_buy * atoi(arg1)) > ch->gold ){
			send_to_char("> &Rsyntax: you do not have enough credits&w\n\r", ch);
			return;
		}

		ch->gold -= (atoi(arg1) * entertain_buy);
		ch->pcdata->rentertain += atoi(arg1);

		location->area->planet->governed_by->funds += (atoi(arg1) * entertain_buy);
		location->area->planet->entertain_amount -= atoi(arg1);

		ch_printf( ch, "&W> you bought %d entertainment source for %d credits &w\n\r", atoi(arg1), atoi(arg1) * entertain_buy );

		break;

	case 2: // multimedia

		if ( (multimedia_amount - atoi(arg1)) < multimedia_max ) {
			send_to_char( "> &Rsystem does not sell that much of this resource&w\n\r" , ch );
			return;
		}

		multimedia_buy = location->area->planet->multimedia_sellprice;

		if ( (multimedia_buy * atoi(arg1)) > ch->gold ){
			send_to_char("> &Rsyntax: you do not have enough credits&w\n\r", ch);
			return;
		}

		ch->gold -= (atoi(arg1) * multimedia_buy);
		ch->pcdata->rmultimedia += atoi(arg1);

		location->area->planet->governed_by->funds += (atoi(arg1) * multimedia_buy);
		location->area->planet->multimedia_amount -= atoi(arg1);

		ch_printf( ch, "&W> you bought %d multimedia source for %d credits &w\n\r", atoi(arg1), atoi(arg1) * multimedia_buy );

		break;

	case 3:  // finance

		if ( (finance_amount - atoi(arg1)) < finance_max ) {
			send_to_char( "> &Rsystem does not sell that much of this resource&w\n\r" , ch );
			return;
		}

		finance_buy = location->area->planet->finance_sellprice;

		if ( (finance_buy * atoi(arg1)) > ch->gold ){
			send_to_char("> &Rsyntax: you do not have enough credits&w\n\r", ch);
			return;
		}

		ch->gold -= (atoi(arg1) * finance_buy);
		ch->pcdata->rfinance += atoi(arg1);

		location->area->planet->governed_by->funds += (atoi(arg1) * finance_buy);
		location->area->planet->finance_amount -= atoi(arg1);

		ch_printf( ch, "&W> you bought %d finance source for %d credits &w\n\r", atoi(arg1), atoi(arg1) * finance_buy );

		break;

	case 4:  // product

		if ( (product_amount - atoi(arg1)) < product_max ) {
			send_to_char( "> &Rsystem does not sell that much of this resource&w\n\r" , ch );
			return;
		}

		product_buy = location->area->planet->product_sellprice;

		if ( (product_buy * atoi(arg1)) > ch->gold ){
			send_to_char("> &Rsyntax: you do not have enough credits&w\n\r", ch);
			return;
		}

		ch->gold -= (atoi(arg1) * product_buy);
		ch->pcdata->rproduct += atoi(arg1);

		location->area->planet->governed_by->funds += (atoi(arg1) * product_buy);
		location->area->planet->product_amount -= atoi(arg1);

		ch_printf( ch, "&W> you bought %d productivity source for %d credits &w\n\r", atoi(arg1), atoi(arg1) * product_buy );

		break;
	}

	return;

}

void do_cy_resset( CHAR_DATA *ch, char *argument )
{

	ROOM_INDEX_DATA	*location;
	CLAN_DATA *clan;
	char arg[MAX_INPUT_LENGTH];
	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	int restype, roption;

	argument = one_argument( argument , arg );
	argument = one_argument( argument , arg1 );
	argument = one_argument( argument , arg2 );

    if ( IS_NPC( ch ) || !ch->pcdata->clan )
    {
	send_to_char( "> invalid command\n\r", ch );
	return;
    }

	location = ch->in_room;

	if (!location->area->planet->governed_by){
		send_to_char("> &Rinvalid command&w\n\r", ch);
		return;
	}

    clan = ch->pcdata->clan;

    if ( (ch->pcdata && ch->pcdata->bestowments
    &&    is_name("repos", ch->pcdata->bestowments))
    ||   nifty_is_name( ch->name, clan->leaders  ))
	;
    else
    {
	send_to_char( "> invalid command\n\r", ch );
	return;
    }

	if ( !ch->in_room || !ch->in_room->area ||
			( ch->in_room->area->planet && ch->in_room->area->planet->governed_by != ch->pcdata->clan ) )
	{
		send_to_char( "> &Ryou can only use this in systems that your organization controls&w\n\r", ch );
		return;
	}

	if ( arg[0] == '\0' )
	{
		send_to_char("> &Rsyntax: resset [type] [option] [value]&w\n\r", ch);
		send_to_char("> &Wset specified option for current system&w\n\r", ch);
		send_to_char("> &Wtypes: ente, mult, fina, prod&w\n\r", ch);
		send_to_char("> &Woptions: buy, sell, min, max&w\n\r", ch);
		return;
	}

	if ( arg1[0] == '\0' )
	{
		send_to_char("> &Rsyntax: resset [type] [option] [value]&w\n\r", ch);
		send_to_char("> &Wset specified option for current system&w\n\r", ch);
		send_to_char("> &Wtypes: ente, mult, fina, prod&w\n\r", ch);
		send_to_char("> &Woptions: buy, sell, min, max&w\n\r", ch);
		return;
	}

	if ( atoi(arg2) <= 0){
		send_to_char("> &Ramount must be greater than 0&w\n\r", ch);
		return;
	}

	if ( atoi(arg2) > 1000000){
		send_to_char("> &Ramount must be smaller than that&w\n\r", ch);
		return;
	}

	if ( !str_cmp( arg, "ente") )
			restype = 1;
	else if ( !str_cmp( arg, "mult") )
			restype = 2;
	else if ( !str_cmp( arg, "fina") )
			restype = 3;
	else if ( !str_cmp( arg, "prod") )
			restype = 4;
	else
			restype = 0;

	if ( !str_cmp( arg1, "buy") )
			roption = 1;
	else if ( !str_cmp( arg1, "sell") )
		roption = 2;
	else if ( !str_cmp( arg1, "min") )
		roption = 3;
	else if ( !str_cmp( arg1, "max") )
		roption = 4;
	else
		roption = 0;

	switch (restype){

	default:
		do_cy_resset(ch, "");
		return;
	break;

	case 1: // entertainment


			switch (roption) {

			default:
				do_cy_resset(ch, "");
				return;
				break;

			case 1: //buy
				location->area->planet->entertain_buyprice = atoi(arg2);
				break;
			case 2: //sell
				location->area->planet->entertain_sellprice = atoi(arg2);
				break;
			case 3: //min
				location->area->planet->entertain_min = atoi(arg2);
				break;
			case 4: //max
				location->area->planet->entertain_max = atoi(arg2);
				break;

			}

		break;

	case 2: // multimedia

		switch (roption) {

		default:
			do_cy_resset(ch, "");
			return;
			break;

		case 1: //buy
			location->area->planet->multimedia_buyprice = atoi(arg2);
			break;
		case 2: //sell
			location->area->planet->multimedia_sellprice = atoi(arg2);
			break;
		case 3: //min
			location->area->planet->multimedia_min = atoi(arg2);
			break;
		case 4: //max
			location->area->planet->multimedia_max = atoi(arg2);
			break;

		}

		break;

	case 3:  // finance

		switch (roption) {

		default:
			do_cy_resset(ch, "");
			return;
			break;

		case 1: //buy
			location->area->planet->finance_buyprice = atoi(arg2);
			break;
		case 2: //sell
			location->area->planet->finance_sellprice = atoi(arg2);
			break;
		case 3: //min
			location->area->planet->finance_min = atoi(arg2);
			break;
		case 4: //max
			location->area->planet->finance_max = atoi(arg2);
			break;

		}

		break;

	case 4:  // product

		switch (roption) {

		default:
			do_cy_resset(ch, "");
			return;
			break;

		case 1: //buy
			location->area->planet->product_buyprice = atoi(arg2);
			break;
		case 2: //sell
			location->area->planet->product_sellprice = atoi(arg2);
			break;
		case 3: //min
			location->area->planet->product_min = atoi(arg2);
			break;
		case 4: //max
			location->area->planet->product_max = atoi(arg2);
			break;

		}

		break;
	}

	save_planet(location->area->planet);
	send_to_char( "> &Goption set&w\n\r", ch );
	return;

}
