
#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include "mud.h"

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
	send_to_char( "> skills: aid, backstab, blades, blasters, codeblade, codeblaster, codecomlink, codecontainer, codedef, codelight, codeshield, codeutil, damboost, disguise, dodge, dualwield, firstaid, hide, peek, picklock, poisonmod, postguard, propaganda, quicktalk, reinforcements, second attack, steal, throw\n\r",	ch );
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
	send_to_char( "> you cannot recall in combat\n\r", ch );
	return;
	}

	if( !ch->plr_home )
	{
	send_to_char( "> you do not have a home node\n\r", ch );
	return;
	}

	send_to_char( "> you connect to your home node\n\r", ch );
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
    //int seccode;
    PLANET_DATA * planet;
    //bool pfound = FALSE;
    ROOM_INDEX_DATA * room;
    //ROOM_INDEX_DATA * target_room;
    ROOM_INDEX_DATA * in_room;
    bool rfound = FALSE;

	argument = one_argument( argument , arg );
	argument = one_argument( argument , arg1 );
	argument = one_argument( argument , arg2 );

    switch( ch->substate )
    {
	default:

	in_room = ch->in_room;

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


    if ( !IS_SET( ch->in_room->room_flags, ROOM_CAN_LAND ) )
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

	if ( arg1[0] != '\0' )
	{
		for ( room = planet->area->first_room ; room ; room = room->next_in_area )
		{
			if ( IS_SET( room->room_flags , ROOM_CAN_LAND ) && !str_prefix( argument , room->name) )
			{
			rfound = TRUE;
			//target_room = room->vnum;
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
		ch_printf( ch , "%-15d  %s\n\r", room->vnum, room->name);
		
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
        
        return;
}

//done for Neuro
