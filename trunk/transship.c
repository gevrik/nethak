/*************************************************
**************************************************
**			Transfering Ships By Command		**
**												**
**			By Gavin Mogan						**
**************************************************
**************************************************
**			Star Wars: Unknown Regions			**
**			Telnet://swruk.dhs.org:5555			**
**			Http://halkeye@halkeye.net			**
**			Email: halkeye@halkeye.net		        **
**************************************************
**************************************************
**			Put do_transship in tables.c		**
**			and in mud.h						**
**			and if nesseccary, any where else	**
**************************************************
**************************************************/


void do_transship( CHAR_DATA *ch ,char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	int arg3;
    SHIP_DATA *ship;
    
    if ( IS_NPC( ch ) )
    {
	send_to_char( "!Invalid Input!\n\r", ch );
	return;
    }

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    ship = get_ship( arg1 );
	if ( !ship )
    {
	send_to_char( "No such deck.\n\r", ch );
	return;
    }

	arg3 = atoi( arg2 );
	
	 if ( arg1[0] == '\0' || arg2[0] == '\0' || arg1[0] == '\0' )
    {
	send_to_char( "Usage: transship <ship> <vnum>\n\r", ch );
	return;
    }

	 ship->shipyard = arg3;
     ship->shipstate = SHIP_READY;
     
     if ( ship->class != SHIP_PLATFORM && ship->type != MOB_SHIP )
     {
           extract_ship( ship );
           ship_to_room( ship , ship->shipyard ); 
     
           ship->location = ship->shipyard;
           ship->lastdoc = ship->shipyard; 
           ship->shipstate = SHIP_DOCKED;
     }
     
     if (ship->starsystem)
        ship_from_starsystem( ship, ship->starsystem );  
     
     save_ship(ship);               
	 send_to_char( "Deck Transfered.\n\r", ch );
}