#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include "mud.h"

void do_contractpropaganda( CHAR_DATA *ch, char *argument )
{
    char        arg[MAX_INPUT_LENGTH];
    char        buf[MAX_STRING_LENGTH];
    PLANET_DATA * planet = NULL;
    OBJ_INDEX_DATA * pObjIndex;
    OBJ_DATA * obj;
    CLAN_DATA *clan;
    CLAN_DATA *tclan;
    int cost = 1000;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "> &Ryou need to specify a target system&w\n\r", ch );
	return;
    }

    if (ch->gold < 1000)
    {
		send_to_char( "> &Rcontract costs 1,000 credits&w\n\r", ch );
		return;
    }

    planet = get_planet( arg );

	if ( !planet )
	{
		send_to_char( "> &Rno such system&w\n\r", ch );
		return;
	}

       if ( ( pObjIndex = get_obj_index( 105 ) ) == NULL )
       {
    	   send_to_char( "> &rinvalid contract object - contact admin&w\n\r", ch );
          return;
       }

	   if( IS_SET( planet->flags, PLANET_HIDDEN ) )
       {
		   send_to_char( "> &Rno such system&w\n\r", ch );
          return;
       }

	   if( IS_SET( planet->flags, PLANET_NOCAP ) )
       {
		   send_to_char( "> &Rno such system&w\n\r", ch );
          return;
       }

	clan = ch->pcdata->clan;
	tclan = planet->governed_by;

	if ( !clan )
	{
		   send_to_char( "> &Ryou have to be a member of an organization&w\n\r", ch );
       return;
	}

		if( !nifty_is_name( tclan->name, clan->atwar ) )
		{
			send_to_char( "> &Ryou have to be at war with the target organization&w\n\r", ch );
			return;
		}

	       sprintf( buf , "contract for %s" , planet->name );

	       obj = create_object( pObjIndex , 1 );
	       STRFREE( obj->name );
	       obj->name = STRALLOC( buf );
	       STRFREE( obj->short_descr );
	       sprintf( buf , "contract [%s] [%s]" , planet->name, clan->name );
	       obj->short_descr = STRALLOC( buf );
	       obj->value[0] = 20;
	       obj = obj_to_char( obj, ch );

	       ch->gold -= cost;

	       send_to_char( "> &Gcontract created&w\n\r", ch );
		return;
	}
