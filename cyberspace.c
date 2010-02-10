
#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include "mud.h"

void do_layout( CHAR_DATA *ch, char *argument )
{

    CLAN_DATA * clan;
    OBJ_DATA *obj;
    OBJ_DATA *obj_next;
    EXIT_DATA *xit;
    ROOM_INDEX_DATA * location;
    int chance;
    char arg[MAX_INPUT_LENGTH];

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
          if ( strlen( location->description ) > 150 ) {
   	     learn_from_success( ch , gsn_landscape );
         learn_from_success( ch , gsn_landscape );
          }
          else
          {
  	     send_to_char( "That node description is too short.\n\r", ch );
  	     send_to_char( "Your skill level diminishes with your laziness.\n\r", ch );
             if ( ch->pcdata->learned[gsn_landscape] > 50 )
                ch->pcdata->learned[gsn_landscape] -= 5;
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

   if( strcmp(location->owner, ch->name) )
   {
	send_to_char( "&R> this is not your node&w\n\r", ch );
	return;
   }

   if ( !location->area || !location->area->planet ||
   clan != location->area->planet->governed_by  )
   {
	send_to_char( "You may only code nodes in complexes that your clan controls!\n\r", ch );
	return;
   }

   if ( IS_SET( location->room_flags , ROOM_NOPEDIT ) )
   {
	send_to_char( "Sorry, but you may not code this room.\n\r", ch );
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
	send_to_char( "> skills: aid, backstab, blades, blasters, codeblade, codeblaster, codecontainer, codedef, codeshield, codeutil, damboost, disguise, dodge, dualwield, firstaid, hide, peek, picklock, poisonmod, postguard, propaganda, quicktalk, reinforcements, second attack, sneak, steal, throw\n\r",	ch );
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

	if ( ch->pcdata->learned[sn] >= 20 )
	{
	send_to_char( "> you already know that skill\n\r", ch );
	return;
	}

	if ( ch->pcdata->num_skills >= get_curr_int(ch) )
	{
	    send_to_char( "> not intelligent enough yet\n\r", ch );
	    return;
	}

	if ( (ch->pcdata->num_skills - ch->pcdata->adept_skills) >= 7 )
	{
	    send_to_char( "> you need to perfect another skill first\n\r", ch );
	    return;
	}

	ch->gold     -= cost;
	ch->pcdata->num_skills++;	
	ch->pcdata->learned[sn] = 20;

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

	if( !ch->plr_home )
	{
		send_to_char( "> you connect to the straylight lobby\n\r", ch );
		act(AT_GREEN, "> $n connects to straylight lobby", ch, NULL, NULL, TO_ROOM );
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
		if ( IS_SET( room->room_flags, ROOM_CAN_LAND ) )
		ch_printf( ch , "%-15d  %s &C[&Rprot&C]&w\n\r", room->vnum, room->name);
		for ( room = planet->area->first_room ; room ; room = room->next_in_area )
		if ( IS_SET( room->room_flags, ROOM_PUBLICIO ) )
		ch_printf( ch , "%-15d  %s &C[&Gpublic&C]&w\n\r", room->vnum, room->name);
		
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
    char arg2[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    OBJ_INDEX_DATA *pObjIndex = NULL;
    int level, chance;
    bool checksew, checkfab;
    OBJ_DATA *obj;
    OBJ_DATA *material;
    int value;

    argument = one_argument( argument, arg );
    //strcpy( arg2 , argument );


    switch( ch->substate )
    {
    	default:

    	    if ( !IS_SET( ch->in_room->room_flags, ROOM_RESTAURANT ) )
    	    {
    	  	send_to_char( "&R> you need to be in a coding node\n\r", ch );
    		return;
    	    }

    		if ( str_cmp( arg, "def" )
    		&& str_cmp( arg, "blaster" )
    		&& str_cmp( arg, "blade" )
    		&& str_cmp( arg, "function" )
    		&& str_cmp( arg, "util" )
    		&& str_cmp( arg, "patch" )
    		&& str_cmp( arg, "app" ) )
    		{
        		send_to_char( "&R> you cannot decompile that, try:\n\r&w", ch);
        		send_to_char( "&R> def, blaster, blade, function,\n\r&w", ch);
        		send_to_char( "&R> util, app or patch\n\r&w", ch);
        		return;
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
    		   send_to_char( "&G> you begin the long process of decompiling\n\r", ch);
    		   act( AT_PLAIN, "> $n takes $s compiler as well as some code and begins to work", ch,
		        NULL, argument , TO_ROOM );
    		   add_timer ( ch , TIMER_DO_FUN , 10 , do_decompile , 1 );
    		   ch->dest_buf = str_dup(arg);
    		   return;
	        }
	        send_to_char("&R> you cannot figure out what to do\n\r",ch);
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
    	        send_to_char("&R> you are interrupted and fail to finish your work\n\r", ch);
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
       }
    }

    if ( ( !checkfab ) || ( !checksew ) )
    {
       send_to_char( "&R> you could not decompile anything useful\n\r", ch);
       learn_from_failure( ch, gsn_spacecraft );
       return;
    }

    int afumble = number_range(0,5);

    if ( afumble < 2 )
    {
       send_to_char( "&R> you could not decompile anything useful\n\r", ch);
       learn_from_failure( ch, gsn_spacecraft );
       return;
    }

    obj = material;

	send_to_char( "&Y> you coded:\n\r\n\r", ch);

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
				int anumber = number_range(0,5);
				if ( anumber == 0 )
					pObjIndex = get_obj_index( 58 );
				else
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
			pObjIndex = get_obj_index( 61 );
			}
	else if ( !str_cmp( arg, "app" ) )
			{
			pObjIndex = get_obj_index( 59 );
			}

    obj = create_object(pObjIndex, 1);
    SET_BIT(obj->extra_flags, ITEM_INVENTORY);
    obj = obj_to_char(obj, ch);

    ch_printf( ch , "%s\n\r\n\r", obj->name);

    send_to_char( "&G> you finish decompiling&w\n\r", ch);
    act( AT_PLAIN, "> $n finishes decompiling", ch,
         NULL, argument , TO_ROOM );

        learn_from_success( ch, gsn_spacecraft );

}


void do_renamenode( CHAR_DATA *ch, char *argument )
{
    char arg [MAX_INPUT_LENGTH];
    char buf [MAX_STRING_LENGTH];
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
    char arg [MAX_INPUT_LENGTH];
    char buf [MAX_STRING_LENGTH];
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

//done for Neuro
