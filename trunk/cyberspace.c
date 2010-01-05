
#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include "mud.h"

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

return;

}

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

    if ( ch->fighting )
    {
	send_to_char( "> you cannot connect in combat\n\r", ch );
	return;
    }

    in_room = ch->in_room;

    if ( !IS_SET( ch->in_room->room_flags, ROOM_CAN_LAND ) )
    {	
  	send_to_char( "&R> you need to be in an io node\n\r", ch );
	return;
    }

    if ( arg[0] == '\0' )
    {
	send_to_char( "&R> please specify a target system\n\r", ch );
	return;
    }

    planet = get_planet( arg );

    if ( !planet )
    {
	send_to_char( "> no such system\n\r", ch );
	return;
    }

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

	ch_printf( ch, "&Y> you connect to: %s@%s\n\r\n\r", location->name, location->area->name );

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