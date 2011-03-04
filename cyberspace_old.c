
#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include "mud.h"

void do_hax( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char arg3[MAX_INPUT_LENGTH];
    PLANET_DATA * planet;
    OBJ_DATA *wield;
    ROOM_INDEX_DATA *location;
    CHAR_DATA *fch;
    CHAR_DATA *fch_next;
    ROOM_INDEX_DATA *in_room;
    int chance, seccode, cpuchance, supporti;
    int att_bonus, roll_cpu, roll_player, xipa, xipa1;
    int hackroll, hackroll_cpu, atthack_bonus;
    int hack_chance, hack_chance_cpu, breakerbonus;
    float support;
    argument = one_argument( argument, arg );
    argument = one_argument( argument, arg2 );
    argument = one_argument( argument, arg3 );

    switch( ch->substate )
    {
	default:
	    in_room = ch->in_room;
	    if ( ( location = find_location( ch, arg ) ) == in_room )
	    {

		send_to_char( "&R> connection already established\n\r", ch );
		return;
    
	    }

    
    if ( arg[0] == '\0' )
    {
/*	if (ch->pcdata->clan->name == "Turing")
		{ location = 1048; }
	if (ch->pcdata->clan->name == "Moderns")
		{ location = 1074; }
	if (ch->pcdata->clan->name == "Freelancers")
		{ location = 1048; }
*/
	send_to_char( "&R> unable to connect\n\r", ch );
	return;
    }

    if ( !IS_SET( ch->in_room->room_flags, ROOM_CAN_LAND ) )
    {	
  	send_to_char( "&R> you need to be in an interface node to do this\n\r", ch );
	return;
    }

	    add_timer( ch, TIMER_DO_FUN, 3, do_hax, 1 );
	    ch->dest_buf = str_dup(arg);
	    ch->dest_buf_2 = str_dup(arg2);
	   ch->dest_buf_3 = str_dup(arg3);
	    send_to_char ( "> checking connection\n\r", ch );
	    return;

	case 1:
	    if ( !ch->dest_buf )
    	     return;
    	    if ( !ch->dest_buf_2 )
    	     return;
	    if ( !ch->dest_buf_3 )
    	     return;
    	    strcpy(arg, ch->dest_buf);
    	    DISPOSE( ch->dest_buf);
    	    strcpy(arg2, ch->dest_buf_2);
    	    DISPOSE( ch->dest_buf_2);
    	    strcpy(arg3, ch->dest_buf_3);
    	    DISPOSE( ch->dest_buf_3);
	    send_to_char( "&Y> connection check complete\n\r", ch );
	    break;

	case SUB_TIMER_DO_ABORT:
	DISPOSE( ch->dest_buf );
    	DISPOSE( ch->dest_buf_2 );
	DISPOSE( ch->dest_buf_3 );
    	ch->substate = SUB_NONE;    		                                   
        send_to_char( "&R> you stop connecting\n\r", ch );
        return;
    }


 ch->substate = SUB_NONE;

in_room = ch->in_room;

    if ( ( location = find_location( ch, arg ) ) == NULL )
    {

	send_to_char( "&R> connection could not be established\n\r", ch );
	return;
    
    }

    if ( !IS_SET( location->room_flags, ROOM_CAN_LAND ) )
    {
	send_to_char( "&R> connection could not be established\n\r", ch );
	return;
    }

    xipa = atoi(arg2);
    seccode = atoi(arg3);
    xipa1 = atoi(arg);

    if ( xipa != location->xipa )
	{
		ch_printf( ch, "&R> invalid xipa: %d %d\n\r", xipa1, xipa);
		return;
	}
    
    if ( location->firewall != 0 && seccode != location->seccode )
	{
		wield = get_eq_char( ch, WEAR_EXT12 );

			    if ( !wield )
				{
					ch_printf( ch, "&Rinvalid security code: %d\n\r", seccode);
					return;
				}

		if (  wield->item_type == ITEM_BREAKER )
   			{

			breakerbonus = wield->value[0];
			planet = location->area->planet;
			support = planet->pop_support;
			supporti = support;			
			hackroll = number_percent ( );
			hackroll_cpu = number_percent ( );
			atthack_bonus = get_curr_int( ch );
			hack_chance = (ch->pcdata->learned[gsn_pick_lock] + atthack_bonus + hackroll + breakerbonus);
			hack_chance_cpu = ( supporti + hackroll_cpu + location->firewall);

        	 	send_to_char( "&y> password breaker running\n\r",ch );

				if ( hack_chance_cpu > hack_chance )
    				{
	           			ch_printf( ch, "&R> the ICE is too strong for your password breaker - [DEBUG] Ro-P: %d Ro-CPU: %d Ch: %d Ch-CPU: %d BB: %d\n\r", hackroll, hackroll_cpu, hack_chance, hack_chance_cpu, breakerbonus);
    	   	   			return;	
    	   			}
				else
				{
				ch_printf( ch , "&y> success - code: %d\n", location->seccode );
				//learn_from_success( ch, gsn_pick_lock );
				return;
				}

   			}
	
    
    planet = location->area->planet;
    support = planet->pop_support;
    supporti = support;
    att_bonus= get_curr_int( ch );
    roll_cpu = number_percent( );
    roll_player = number_percent( );
    chance = (ch->pcdata->learned[gsn_spacecraft] + att_bonus + roll_player);
    cpuchance = ( supporti + roll_cpu + location->firewall);

                if ( cpuchance > chance )
    		{
	           ch_printf( ch, "&R> failed to execute command - [DEBUG] Ro-P: %d Ro-CPU: %d Ch: %d Ch-CPU: %d\n\r", roll_player, roll_cpu, chance, cpuchance);
    	   	   return;	
    	   	}
	}

	if ( IS_SET( location->room_flags, ROOM_BODCHECK ) )
  	  {
		wield = get_eq_char( ch, WEAR_EXT7 );
			if ( !wield || wield->pIndexData->vnum != 280 )
   			{
        	 	send_to_char( "&R> the defensive ICE will not let you connect - [DEBUG] BOD\n\r",ch );
        	 	return;
   			}
    	  }

	if ( IS_SET( location->room_flags, ROOM_EVACHECK ) )
  	  {
		wield = get_eq_char( ch, WEAR_EXT8 );
			if ( !wield || wield->pIndexData->vnum != 282 )
   			{
        	 	send_to_char( "&R> the defensive ICE will not let you connect - [DEBUG] EVA\n\r",ch );
        	 	return;
   			}
    	  }

	if ( IS_SET( location->room_flags, ROOM_MASCHECK ) )
  	  {
		wield = get_eq_char( ch, WEAR_EXT9 );
			if ( !wield || wield->pIndexData->vnum != 285 )
   			{
        	 	send_to_char( "&R> the defensive ICE will not let you connect - [DEBUG] MAS\n\r",ch );
        	 	return;
   			}
    	  }

	ch_printf( ch, "&Y> you connect to: %s@%s\n\r\n\r", location->name, location->area->name );
	ch_printf( ch, "&R[DEBUG] Ro-P: %d Ro-CPU: %d Ch: %d Ch-CPU: %d\n\r", roll_player, roll_cpu, chance, cpuchance);

    if ( ch->fighting )
	stop_fighting( ch, TRUE );

    if ( !IS_SET(ch->act, PLR_WIZINVIS) )
	{
         if (ch->pcdata && ch->pcdata->bamfout[0] != '\0')
               act( AT_IMMORT, "$T", ch, NULL, ch->pcdata->bamfout ,  TO_ROOM );
          else  
               act( AT_IMMORT, "$n $T", ch, NULL, "> starts connecting to another system",  TO_ROOM );
	}
	                              
    ch->regoto = ch->in_room->vnum;

    char_from_room( ch );
    
    char_to_room( ch, location );

   if ( !IS_SET(ch->act, PLR_WIZINVIS) )
	{
         if (ch->pcdata && ch->pcdata->bamfin[0] != '\0')
             act( AT_IMMORT, "$T", ch, NULL, ch->pcdata->bamfin ,  TO_ROOM );
         else  
             act( AT_IMMORT, "$n $T", ch, NULL, "connects to this complex...",  TO_ROOM );
	}                          
                               
    if ( location->firewall != 0 )
	{
    	learn_from_success( ch, gsn_spacecraft );
	}
    do_look( ch, "auto" );


    if ( ch->in_room == in_room )
      return;
    for ( fch = in_room->first_person; fch; fch = fch_next )
    {
	fch_next = fch->next_in_room;
	if ( fch->master == ch && IS_IMMORTAL(fch) )
	{
	    act( AT_ACTION, "> you follow $N", fch, NULL, ch, TO_CHAR );
	    do_goto( fch, argument );
	}
    }
    return;
}

//done for Neuro