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
extern int top_r_vnum;

void do_jobs( CHAR_DATA *ch, char *argument )
{

	// 1 : FEDEX
	// 2 : RESOURCE HUNT

	ROOM_INDEX_DATA *location;
    PLANET_DATA * dPlanet = NULL;
    OBJ_INDEX_DATA	*obj;
    int pCount = 0;
    int rCount;
	int chance;
	int resourceneeded;
	int penalty;

    if ( IS_NPC(ch) || !ch->pcdata )
       return;

    if ( !str_cmp( argument, "clear" )){
    	if( IS_IMMORTAL(ch) )
    	{
    		ch->pcdata->mission_active = 0;
    		ch->pcdata->mission_type = 0;
    		ch->pcdata->mission_targetid = 0;
    		STRFREE( ch->pcdata->mission_target );
    		ch->pcdata->mission_target = STRALLOC( "" );
    		send_to_char( "> &Gmission cleared&w\n\r", ch );
    		return;

    	}
    	else
    	{
    		if (ch->pcdata->mission_fails >= 10)
    		{
    			send_to_char( "> &Rmission clearance level reached&w\n\r", ch );
    			return;
    		}

    		penalty = ((ch->pcdata->mission_fails + 1) * 100);

    		if (ch->gold < penalty)
    		{
    			ch_printf( ch, "> &Rit would cost %dc to abort your current mission&w\n\r", penalty );
    			return;
    		}
    		else
    		{
    			ch->gold -= UMIN( penalty , ch->gold );
        		ch->pcdata->mission_active = 0;
        		ch->pcdata->mission_type = 0;
        		ch->pcdata->mission_targetid = 0;
        		STRFREE( ch->pcdata->mission_target );
        		ch->pcdata->mission_target = STRALLOC( "" );
        		ch->pcdata->mission_fails++;
        		ch_printf( ch, "> &G%dc paid to clear mission&w\n\r", penalty );
        		return;
    		}
    	}
    }

    location = ch->in_room;

    if (ch->pcdata->mission_active != 0)
    {
    	send_to_char( "> &Ycomplete your current mission:&w\n\r", ch );

        switch( ch->pcdata->mission_type )
        {

        default:
        	send_to_char("> &Rinvalid data - contact Wintermute&w\n\r", ch );
        	return;
        break;

        case 1:
        	ch_printf( ch, "> &Gtype:&W delivery&w &Gtarget:&W %s&w &Greward:&W 100c&w\n\r", ch->pcdata->mission_target );
        break;

        case 2:
        	obj = get_obj_index( ch->pcdata->mission_targetid );
        	ch_printf( ch, "> &Gtype:&W %s&w &Gtarget:&W %s&w &Greward:&W %dc&w\n\r", obj->name, ch->pcdata->mission_target, (obj->cost * 50) );
        break;

        }

        return;
    }

    if (IS_SET(location->room_flags, ROOM_EMPLOYMENT ))
    {

        if ( get_age(ch) <= 20 )
        	chance = number_range(3, 5);
        else
        	chance = number_range(1, 5);


    	if (chance == 1){

       		// 28 - 38

     	       ch->pcdata->mission_active = 1;
     	       ch->pcdata->mission_type = 2;

        		resourceneeded = number_range(28, 38);

        		if (resourceneeded == 23)
        		{
        	    	send_to_char( "> &Rno missions available currently&w\n\r", ch );
        	    	return;
        		}

        		ch->pcdata->mission_targetid = resourceneeded;
        		ch->pcdata->mission_target = STRALLOC( ch->in_room->area->planet->name );

        		obj = get_obj_index( resourceneeded );

        		ch_printf( ch, "> &Gmission:&W resource [%s] to [%s]&w\n\r", obj->name, ch->pcdata->mission_target );

    	}
    	else if (chance == 2){

    		// 76 - 97

 	       ch->pcdata->mission_active = 1;
 	       ch->pcdata->mission_type = 2;

    		resourceneeded = number_range(76, 97);
    		ch->pcdata->mission_targetid = resourceneeded;
    		ch->pcdata->mission_target = STRALLOC( ch->in_room->area->planet->name );

    		obj = get_obj_index( resourceneeded );

    		ch_printf( ch, "> &Gmission:&W resource [%s] to [%s]&w\n\r", obj->name, ch->pcdata->mission_target );

    	}
    	else if (chance == 3){

    		// 110 - 113

 	       ch->pcdata->mission_active = 1;
 	       ch->pcdata->mission_type = 2;

    		resourceneeded = number_range(110, 113);
    		ch->pcdata->mission_targetid = resourceneeded;
    		ch->pcdata->mission_target = STRALLOC( ch->in_room->area->planet->name );

    		obj = get_obj_index( resourceneeded );

    		ch_printf( ch, "> &Gmission:&W resource [%s] to [%s]&w\n\r", obj->name, ch->pcdata->mission_target );

    	}
    	else{

    		// FEDEX

    	       for ( dPlanet = first_planet ; dPlanet ; dPlanet = dPlanet->next )
    	           pCount++;

    	       rCount = number_range( 1 , pCount );

    	       pCount = 0;

    	       for ( dPlanet = first_planet ; dPlanet ; dPlanet = dPlanet->next )
    	           if ( ++pCount == rCount )
    	               break;

    	       if( !dPlanet || dPlanet == ch->in_room->area->planet || dPlanet == first_planet || IS_SET( dPlanet->flags, PLANET_HIDDEN)
    	    		   || IS_SET( dPlanet->flags, PLANET_NOCAP ) )
    	       {
    	    	send_to_char( "> &Rno missions available currently&w\n\r", ch );
    	    	return;
    	       }

    	       ch->pcdata->mission_active = 1;
    	       ch->pcdata->mission_type = 1;
    	       ch->pcdata->mission_targetid = 0;

    	       STRFREE( ch->pcdata->mission_target );
    	       ch->pcdata->mission_target = STRALLOC( dPlanet->name );

    	       ch_printf( ch, "> &Gmission:&W package delivery to %s&w\n\r", dPlanet->name );
    	}

    }
    /*
    else if (IS_SET(location->room_flags, ROOM_HOTEL ))
    {
    	chance = number_range(1, 2);

    	if (chance == 1){
    		send_to_char( "> &YVIRUS MISSION&w\n\r", ch );
    	}
    	else if (chance == 2){
    		send_to_char( "> &YKILL MISSION&w\n\r", ch );
    	}
    	else{
    		send_to_char( "> &Rsomething went wrong - contact Wintermute&w\n\r", ch );
    	}
    }
    */
    else
    {
    	send_to_char( "> &Rgo to an employment node&w\n\r", ch );
    }

    return;

}

void do_completejob( CHAR_DATA *ch, char *argument )
{

	//PLANET_DATA *planet;
	int gain;
	bool checkresource;
	OBJ_DATA *obj;
	OBJ_INDEX_DATA	*tobj;

    if ( IS_NPC(ch) || !ch->pcdata )
       return;

    if (ch->pcdata->mission_active == 0){
    	send_to_char( "> &Rno active mission&w\n\r", ch );
    	return;
    }

    switch( ch->pcdata->mission_type )
    {

    default:
    	send_to_char("> &Rinvalid data - contact Wintermute&w\n\r", ch );
    	return;
    break;

    case 1:

    	if (str_cmp(ch->in_room->area->planet->name, ch->pcdata->mission_target))
    	{
        	send_to_char("> &Rwrong destination system&w\n\r", ch );
        	return;
    	}

    	if ( !IS_SET( ch->in_room->room_flags , ROOM_EMPLOYMENT ) )
    	{
    		send_to_char( "> &Rfind an employment node to deliver the package&w\n\r", ch );
    		return;
    	}

		ch->pcdata->mission_active = 0;
		ch->pcdata->mission_type = 0;
		ch->pcdata->mission_targetid = 0;
		STRFREE( ch->pcdata->mission_target );
		ch->pcdata->mission_target = STRALLOC( "" );
		ch->pcdata->mission_fails = 0;
		ch->gold += 100;
		gain = 100;
		ch_printf( ch, "> &G%dc received for mission&w\n\r", gain );


    break;

    case 2:

    	if (str_cmp(ch->in_room->area->planet->name, ch->pcdata->mission_target))
    	{
        	send_to_char("> &Rwrong destination system&w\n\r", ch );
        	return;
    	}

    	if ( !IS_SET( ch->in_room->room_flags , ROOM_EMPLOYMENT ) )
    	{
    		send_to_char( "> &Rfind an employment node to deliver the resource&w\n\r", ch );
    		return;
    	}

    	checkresource = FALSE;
    	tobj = get_obj_index( ch->pcdata->mission_targetid );

		for ( obj = ch->last_carrying; obj; obj = obj->prev_content )
		{
			if ( obj->pIndexData->vnum == tobj->vnum )
			checkresource = TRUE;
			gain = obj->cost * 50;
			separate_obj( obj );
			obj_from_char( obj );
			extract_obj( obj );
		}

		if ( !checkresource )
		{
			send_to_char( "> &Rrequired resource needed to complete mission&w\n\r", ch);
			return;
		}

		ch->pcdata->mission_active = 0;
		ch->pcdata->mission_type = 0;
		ch->pcdata->mission_targetid = 0;
		STRFREE( ch->pcdata->mission_target );
		ch->pcdata->mission_target = STRALLOC( "" );
		ch->pcdata->mission_fails = 0;
		ch->gold += gain;
		ch_printf( ch, "> &G%dc received for mission&w\n\r", gain );

    break;

    }

}
