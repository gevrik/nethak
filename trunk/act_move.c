#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include "mud.h"


const	sh_int	movement_loss	[SECT_MAX]	=
{
    1, 2, 2, 3, 4, 6, 4, 1, 6, 10, 6, 5, 7, 4, 6, 4, 2, 3, 6, 3, 3, 5, 4, 3, 2, 3, 7
};

char *	const	dir_name	[]		=
{
    "north", "east", "south", "west", "up", "down",
    "northeast", "northwest", "southeast", "southwest", "somewhere"
};


const	sh_int	rev_dir		[]		=
{
    2, 3, 0, 1, 5, 4, 9, 8, 7, 6, 10
};




char *	const		sect_names[SECT_MAX][2] =
{
    { "in a room","inside"	},	{ "a default node","default nodes"      },
    { "in a field","fields"	},	{ "in a forest","forests"	},
    { "hill",	"hills"		},	{ "on a mountain","mountains"	},
    { "in the water","waters"	},	{ "in rough water","waters"	},
    { "underwater", "underwaters" },	{ "in the air",	"air"		},
    { "in a desert","deserts"	},	{ "somewhere",	"unknown"	},
    { "ocean floor", "ocean floor" },	{ "underground", "underground"	},
    { "scrub", "scrub" },		{ "rocky terrain", "rocky"	},
    { "savanna", "savanna" },		{ "tundra", "tundra"	},
    { "glacier", "glacial" },		{ "rainforest", "rainforest"	},
    { "jungle", "jungle" },		{ "swamp", "swamp"	},
    { "wetlands", "wetlands" },		{ "brush", "brush"	},
    { "steppe", "steppe" },		{ "farmland", "farmland"	},    
    { "volcano", "volcanic" }    
};


const	int		sent_total[SECT_MAX]		=
{
    4, 24, 4, 4, 1, 1, 1, 1, 1, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 
};

char *	const		room_sents[SECT_MAX][25]	=
{
    {
	"The rough hewn walls are made of granite.",
	"> you see an occasional spider crawling around.",
	"> you notice signs of a recent battle from the bloodstains on the floor.",
	"This place has a damp musty odour not unlike rotting vegetation."
    },

    {
	"> you notice the occasional stray looking for food.",
	"Tall buildings loom on either side of you stretching to the sky.",
	"Some street people are putting on an interesting display of talent trying to earn some credits.",
	"Two people nearby shout heated words of argument at one another.",
	"> you think you can make out several shady figures talking down a dark alleyway."
        "A slight breeze blows through the tall buildings.",
        "A small crowd of people have gathered at one side of the street.",
        "Clouds far above you obscure the tops of the highest skyscrapers.",
        "A speeder moves slowly through the street avoiding pedestrians.",
        "A cloudcar flys by overhead.",
        "The air is thick and hard to breath.",
        "The many smells of the city assault your senses.",
        "> you hear a scream far of in the distance.",
        "The buildings around you seem endless in number.",
        "The city stretches seemingly endless in all directions.",
        "The street is wide and long.",
        "A swoop rider passes quickly by weaving in and out of pedestrians and other vehicles.",
        "The surface of the road is worn from many travellers.",
        "> you feel it would be very easy to get lost in such an enormous city.",
        "> you can see other streets above and bellow this one running in many directions.",
        "There are entrances to several buildings at this level.",
        "Along the edge of the street railings prevent pedestrians from falling to their death.",
        "In between the many towers you can see down into depths of the lower city.",
        "A grate in the street prevents rainwater from building up.",
        "> you can see you reflection in several of the transparisteel windows as you pass by."
        "> you hear a scream far off in the distance.",
    },

    {
	"> you notice sparce patches of brush and shrubs.",
	"There is a small cluster of trees far off in the distance.",
	"Around you are grassy fields as far as the eye can see.",
	"Throughout the plains a wide variety of weeds and wildflowers are scattered."
    },

    {
	"Tall, dark evergreens prevent you from seeing very far.",
	"Many huge oak trees that look several hundred years old are here.",
	"> you notice a solitary lonely weeping willow.",
	"To your left is a patch of bright white birch trees, slender and tall."
    },

    {
	"The rolling hills are lightly speckled with violet wildflowers."
    },

    {
	"The rocky mountain pass offers many hiding places."
    },

    {
	"The water is smooth as glass."
    },

    {
	"Rough waves splash about angrily."
    },

    {
	"A small school of fish swims by."
    },

    {
	"The land is far far below.",
	"A misty haze of clouds drifts by."
    },

    {
	"Around you is sand as far as the eye can see.",
	"> you think you see an oasis far in the distance."
    },

    {
	"> you notice nothing unusual."
    },

    { 
        "There are many rocks and coral which litter the ocean floor."
    },

    {
	"> you stand in a lengthy tunnel of rock."
    },

    {
	"Hmm..."
    },


    {
	"Hmm..."
    },

    {
	"Hmm..."
    },

    {
	"Hmm..."
    },

    {
	"Hmm..."
    },

    {
	"Hmm..."
    },

    {
	"Hmm..."
    },

    {
	"Hmm..."
    },

    {
	"Hmm..."
    },

    {
	"Hmm..."
    },

    {
	"Hmm..."
    },

    {
	"Hmm..."
    },

    {
	"Hmm..."
    }

};

int wherehome( CHAR_DATA *ch)
{
    SHIP_DATA *ship;
    PLANET_DATA * planet;
    CLAN_DATA *clan;
    ROOM_INDEX_DATA *room;
        
    if( ch->plr_home )
      return ch->plr_home->vnum;

    if ( IS_IMMORTAL(ch) )
       return ROOM_IMMORTAL_START;

    if ( ch->pcdata && ch->pcdata->clan )
       for ( planet = first_planet ; planet ; planet = planet->next )
           if ( planet->governed_by == clan && planet->area )
              for( room = planet->area->first_room ; room ; room = room->next_in_area )
		if ( IS_SET( room->room_flags, ROOM_HOTEL )  && !IS_SET( room->room_flags, ROOM_PLR_HOME )  )
		   return room->vnum;
    
    for ( ship = first_ship ; ship ; ship = ship->next )
    {
        if ( !str_cmp( ship->owner , ch->name ) && ship->shipstate == SHIP_DOCKED )
          return ship->lastdoc;
    } 

    for ( planet = first_planet ; planet ; planet = planet->next )
           if ( planet->area )
              for( room = planet->area->first_room ; room ; room = room->next_in_area )
		if ( IS_SET( room->room_flags, ROOM_HOTEL )  && !IS_SET( room->room_flags, ROOM_PLR_HOME )  )
		   return room->vnum;
    
    return ROOM_VNUM_LIMBO;   
}

char *grab_word( char *argument, char *arg_first )
{
    char cEnd;
    sh_int count;

    count = 0;

    while ( isspace(*argument) )
	argument++;

    cEnd = ' ';
    if ( *argument == '\'' || *argument == '"' )
	cEnd = *argument++;

    while ( *argument != '\0' || ++count >= 255 )
    {
	if ( *argument == cEnd )
	{
	    argument++;
	    break;
	}
	*arg_first++ = *argument++;
    }
    *arg_first = '\0';

    while ( isspace(*argument) )
	argument++;

    return argument;
}

char *wordwrap( char *txt, sh_int wrap )
{
    static char buf[MAX_STRING_LENGTH];
    char *bufp;

    buf[0] = '\0';
    bufp = buf;
    if ( txt != NULL )
    {
        char line[MAX_STRING_LENGTH];
        char temp[MAX_STRING_LENGTH];
        char *ptr, *p;
        int ln, x;

	++bufp;
        line[0] = '\0';
        ptr = txt;
        while ( *ptr )
        {
	  ptr = grab_word( ptr, temp );
          ln = strlen( line );  x = strlen( temp );
          if ( (ln + x + 1) < wrap )
          {
	    if ( line[ln-1] == '.' )
              strcat( line, "  " );
	    else
              strcat( line, " " );
            strcat( line, temp );
            p = strchr( line, '\n' );
            if ( !p )
              p = strchr( line, '\r' );
            if ( p )
            {
                strcat( buf, line );
                line[0] = '\0';
            }
          }
          else
          {
            strcat( line, "\r\n" );
            strcat( buf, line );
            strcpy( line, temp );
          }
        }
        if ( line[0] != '\0' )
            strcat( buf, line );
    }
    return bufp;
}

void decorate_room( ROOM_INDEX_DATA *room )
{
    char buf[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];
    int nRand;
    int iRand, len;
    int previous[8];
    int sector = room->sector_type;

    if ( room->name )
      STRFREE( room->name );
    if ( room->description )
      STRFREE( room->description );

    room->name	= STRALLOC( sect_names[sector][0] );
    buf[0] = '\0';
    nRand = number_range( 1, UMIN(8,sent_total[sector]) );

    for ( iRand = 0; iRand < nRand; iRand++ )
	previous[iRand] = -1;

    for ( iRand = 0; iRand < nRand; iRand++ )
    {
	while ( previous[iRand] == -1 )
	{
	    int x, z;

	    x = number_range( 0, sent_total[sector]-1 );

	    for ( z = 0; z < iRand; z++ )
		if ( previous[z] == x )
		  break;

	    if ( z < iRand )
		  continue;

	    previous[iRand] = x;

	    len = strlen(buf);
	    sprintf( buf2, "%s", room_sents[sector][x] );
	    if ( len > 5 && buf[len-1] == '.' )
	    {
		strcat( buf, "  " );
		buf2[0] = UPPER(buf2[0] );
	    }
	    else
	    if ( len == 0 )
	        buf2[0] = UPPER(buf2[0] );
	    strcat( buf, buf2 );
	}
    }
    sprintf( buf2, "%s\n\r", wordwrap(buf, 78) );
    room->description = STRALLOC( buf2 );
}

void clear_vrooms( )
{
}

/*
 * Function to get the equivelant exit of DIR 0-MAXDIR out of linked list.
 * Made to allow old-style diku-merc exit functions to work.	-Thoric
 */
EXIT_DATA *get_exit( ROOM_INDEX_DATA *room, sh_int dir )
{
    EXIT_DATA *xit;

    if ( !room )
    {
	bug( "Get_exit: NULL room", 0 );
	return NULL;
    }

    for (xit = room->first_exit; xit; xit = xit->next )
       if ( xit->vdir == dir )
         return xit;
    return NULL;
}

/*
 * Function to get an exit, leading the the specified room
 */
EXIT_DATA *get_exit_to( ROOM_INDEX_DATA *room, sh_int dir, long vnum )
{
    EXIT_DATA *xit;

    if ( !room )
    {
	bug( "Get_exit: NULL room", 0 );
	return NULL;
    }

    for (xit = room->first_exit; xit; xit = xit->next )
       if ( xit->vdir == dir && xit->vnum == vnum )
         return xit;
    return NULL;
}

/*
 * Function to get the nth exit of a room			-Thoric
 */
EXIT_DATA *get_exit_num( ROOM_INDEX_DATA *room, sh_int count )
{
    EXIT_DATA *xit;
    int cnt;

    if ( !room )
    {
	bug( "Get_exit: NULL room", 0 );
	return NULL;
    }

    for (cnt = 0, xit = room->first_exit; xit; xit = xit->next )
       if ( ++cnt == count )
         return xit;
    return NULL;
}


/*
 * Modify movement due to encumbrance				-Thoric
 */
sh_int encumbrance( CHAR_DATA *ch, sh_int move )
{
    int cur, max;

    max = can_carry_w(ch);
    cur = ch->carry_weight;
    if ( cur >= max )
      return move * 4;
    else
    if ( cur >= max * 0.95 )
      return move * 3.5;
    else
    if ( cur >= max * 0.90 )
      return move * 3;
    else
    if ( cur >= max * 0.85 )
      return move * 2.5;
    else
    if ( cur >= max * 0.80 )
      return move * 2;
    else
    if ( cur >= max * 0.75 )
      return move * 1.5;
    else
      return move;
}


/*
 * Check to see if a character can fall down, checks for looping   -Thoric
 */
bool will_fall( CHAR_DATA *ch, int fall )
{
    if ( IS_SET( ch->in_room->room_flags, ROOM_NOFLOOR )
    &&   CAN_GO(ch, DIR_DOWN)
    && (!IS_AFFECTED( ch, AFF_FLYING )
    || ( ch->mount && !IS_AFFECTED( ch->mount, AFF_FLYING ) ) ) )
    {
	if ( fall > 80 )
	{
	   bug( "Falling (in a loop?) more than 80 rooms: vnum %d", ch->in_room->vnum );
	   char_from_room( ch );
	   char_to_room( ch, get_room_index( wherehome(ch) ) );
	   fall = 0;
	   return TRUE;
	}
	set_char_color( AT_FALLING, ch );
	send_to_char( "> you are falling down\n\r", ch );
	move_char( ch, get_exit(ch->in_room, DIR_DOWN), ++fall );
	return TRUE;
    }
    return FALSE;
}


ch_ret move_char( CHAR_DATA *ch, EXIT_DATA *pexit, int fall )
{
    ROOM_INDEX_DATA *in_room;
    ROOM_INDEX_DATA *to_room;
    ROOM_INDEX_DATA *from_room;
    char buf[MAX_STRING_LENGTH];
    char *txt;
    char *dtxt;
    ch_ret retcode;
    sh_int door, distance;
    bool drunk = FALSE;
    bool brief = FALSE;

    if ( !IS_NPC( ch ) )
      if ( IS_DRUNK( ch, 2 ) && ( ch->position != POS_SHOVE )
	&& ( ch->position != POS_DRAG ) )
	drunk = TRUE;

    if ( drunk && !fall )
    {
      door = number_door();
      pexit = get_exit( ch->in_room, door );
    }

#ifdef DEBUG
    if ( pexit )
    {
	sprintf( buf, "move_char: %s to door %d", ch->name, pexit->vdir );
	log_string( buf );
    }
#endif

    retcode = rNONE;
    txt = NULL;

    if ( IS_NPC(ch) && IS_SET( ch->act, ACT_MOUNTED ) )
      return retcode;

    in_room = ch->in_room;
    from_room = in_room;
    if ( !pexit || (to_room = pexit->to_room) == NULL )
    {
	if ( drunk )
	  send_to_char( "> you hit a wall in your drunken state\n\r", ch );
	 else
	  send_to_char( "> you cannot go that way\n\r", ch );
	return rNONE;
    }

    door = pexit->vdir;
    distance = pexit->distance;

    /*
     * Exit is only a "window", there is no way to travel in that direction
     * unless it's a door with a window in it		-Thoric
     */
    if ( IS_SET( pexit->exit_info, EX_WINDOW )
    &&  !IS_SET( pexit->exit_info, EX_ISDOOR ) )
    {
	send_to_char( "> you cannot go that way\n\r", ch );
	return rNONE;
    }

    if (  IS_SET(pexit->exit_info, EX_PORTAL) 
       && IS_NPC(ch) )
    {
        act( AT_PLAIN, "> ICE cannot use portals\n\r", ch, NULL, NULL, TO_CHAR );
	return rNONE;
    }

    if ( IS_SET(pexit->exit_info, EX_NOMOB)
	&& IS_NPC(ch) )
    {
	act( AT_PLAIN, "> ICE cannot enter there\n\r", ch, NULL, NULL, TO_CHAR );
	return rNONE;
    }

    if ( IS_SET(pexit->exit_info, EX_CLOSED)
    && (!IS_AFFECTED(ch, AFF_PASS_DOOR)
    ||   IS_SET(pexit->exit_info, EX_NOPASSDOOR)) )
    {
	if ( !IS_SET( pexit->exit_info, EX_SECRET )
	&&   !IS_SET( pexit->exit_info, EX_DIG ) )
	{
	  if ( drunk )
	  {
	    act( AT_PLAIN, "> $n runs into the $d in $s drunken state\n\r", ch,
		NULL, pexit->keyword, TO_ROOM );
	    act( AT_PLAIN, "> you run into the $d in your drunken state\n\r", ch,
		NULL, pexit->keyword, TO_CHAR ); 
	  }
	 else
	  act( AT_PLAIN, "> the $d is closed\n\r", ch, NULL, pexit->keyword, TO_CHAR );
	}
       else
	{
	  if ( drunk )
	    send_to_char( "> you hit a wall in your drunken state\n\r", ch );
	   else
	    send_to_char( "> you cannot go that way\n\r", ch );
	}

	return rNONE;
    }

    if ( !fall
    &&   IS_AFFECTED(ch, AFF_CHARM)
    &&   ch->master
    &&   in_room == ch->master->in_room )
    {
	send_to_char( "> you cannot leave your master\n\r", ch );
	return rNONE;
    }

    if ( room_is_private( ch, to_room ) )
    {
	send_to_char( "> that room is private right now\n\r", ch );
	return rNONE;
    }

    if ( !fall && !IS_NPC(ch) )
    {
	/*int iClass;*/
	int move;
/* Pretty sure we don't need to check for guilds anymore now that we have
   the new dh. -- Narn
*/
/*
	for ( iClass = 0; iClass < MAX_CLASS; iClass++ )
	{
	    if ( iClass != ch->class
	    &&   to_room->vnum == class_table[iClass]->guild )
	    {
		send_to_char( "> you aren't allowed in there\n\r", ch );
		return rNONE;
	    }
	}
*/

/* Prevent deadlies from entering a nopkill-flagged area from a 
   non-flagged area, but allow them to move around if already
   inside a nopkill area. - Blodkai
*/

	if ( in_room->sector_type == SECT_AIR
	||   to_room->sector_type == SECT_AIR
	||   IS_SET( pexit->exit_info, EX_FLY ) )
	{
	    if ( ch->mount && !IS_AFFECTED( ch->mount, AFF_FLYING ) )
	    {
		send_to_char( "> your mount cannot fly\n\r", ch );
		return rNONE;
	    }
	    if ( !ch->mount && !IS_AFFECTED(ch, AFF_FLYING) )
	    {
		send_to_char( "> you would need to fly to go there\n\r", ch );
		return rNONE;
	    }
	}

	if ( in_room->sector_type == SECT_WATER_NOSWIM
	||   to_room->sector_type == SECT_WATER_NOSWIM )
	{
	    bool found;

	    found = FALSE;
	    if ( ch->mount )
	    {
		if ( IS_AFFECTED( ch->mount, AFF_FLYING )
		||   IS_AFFECTED( ch->mount, AFF_FLOATING ) )
		  found = TRUE;
	    }
	    else
	    if ( IS_AFFECTED(ch, AFF_FLYING)
	    ||   IS_AFFECTED(ch, AFF_FLOATING) )
		found = TRUE;

	    if ( !found )
	    {
		send_to_char( "> you would need a boat to go there\n\r", ch );
		return rNONE;
	    }
	}

	if ( IS_SET( pexit->exit_info, EX_CLIMB ) )
	{
	    bool found;

	    found = FALSE;
	    if ( ch->mount && IS_AFFECTED( ch->mount, AFF_FLYING ) )
	      found = TRUE;
	    else
	    if ( IS_AFFECTED(ch, AFF_FLYING) )
	      found = TRUE;

	    if ( !found && !ch->mount )
	    {
		if ( ( !IS_NPC(ch) && number_percent( ) > ch->pcdata->learned[gsn_climb] )
		||      drunk || ch->mental_state < -90 )
		{
		   send_to_char( "> you start to climb but lose your grip and fall\n\r", ch);
		   learn_from_failure( ch, gsn_climb );
		   if ( pexit->vdir == DIR_DOWN )
		   {
			retcode = move_char( ch, pexit, 1 );
			return retcode;
		   }
		   set_char_color( AT_HURT, ch );
		   send_to_char( "> you hit the ground\n\r", ch );
		   WAIT_STATE( ch, 20 );
		   retcode = damage( ch, ch, (pexit->vdir == DIR_UP ? 10 : 5),
					TYPE_UNDEFINED );
		   return retcode;
		}
		found = TRUE;
		learn_from_success( ch, gsn_climb );
		WAIT_STATE( ch, skill_table[gsn_climb]->beats );
		txt = "climbs";
	    }

	    if ( !found )
	    {
		send_to_char( "> you cannot climb\n\r", ch );
		return rNONE;
	    }
	}

	if ( ch->mount )
	{
	  switch (ch->mount->position)
	  {
	    case POS_DEAD:
            send_to_char( "> your mount is dead\n\r", ch );
	    return rNONE;
            break;
        
            case POS_MORTAL:
            case POS_INCAP:
            send_to_char( "> your mount is hurt far too badly to move\n\r", ch );
	    return rNONE;
            break;
            
            case POS_STUNNED:
            send_to_char( "> your mount is too stunned to do that\n\r", ch );
     	    return rNONE;
            break;
       
            case POS_SLEEPING:
            send_to_char( "> your mount is sleeping\n\r", ch );
	    return rNONE;
            break;
         
            case POS_RESTING:
            send_to_char( "> your mount is resting\n\r", ch);
	    return rNONE;
            break;
        
            case POS_SITTING:
            send_to_char( "> your mount is sitting down\n\r", ch);
	    return rNONE;
            break;

	    default:
	    break;
  	  }

	  if ( !IS_AFFECTED(ch->mount, AFF_FLYING)
	  &&   !IS_AFFECTED(ch->mount, AFF_FLOATING) )
	    move = movement_loss[UMIN(SECT_MAX-1, in_room->sector_type)];
	  else
	    move = 1;
	  if ( ch->mount->move < move )
	  {
	    send_to_char( "> your mount is too exhausted\n\r", ch );
	    return rNONE;
	  }
	}
	else
	{
	  if ( !IS_AFFECTED(ch, AFF_FLYING)
	  &&   !IS_AFFECTED(ch, AFF_FLOATING) )
	    move = encumbrance( ch, movement_loss[UMIN(SECT_MAX-1, in_room->sector_type)] );
	  else
	    move = 1;
	  if ( ch->move < move )
	  {
	    send_to_char( "> you are too exhausted\n\r", ch );
	    return rNONE;
	  }
	}

	WAIT_STATE( ch, move );
	if ( ch->mount )
	  ch->mount->move -= move;
	else
	  ch->move -= move;
    }

    /*
     * Check if player can fit in the room
     */
    if ( to_room->tunnel > 0 )
    {
	CHAR_DATA *ctmp;
	int count = ch->mount ? 1 : 0;
	
	for ( ctmp = to_room->first_person; ctmp; ctmp = ctmp->next_in_room )
	  if ( ++count >= to_room->tunnel )
	  {
		if ( ch->mount && count == to_room->tunnel )
		  send_to_char( "> there is no room for both you and your mount in there\n\r", ch );
		else
		  send_to_char( "> there is no room for you in there\n\r", ch );
		return rNONE;
	  }
    }


    if ( !IS_AFFECTED(ch, AFF_SNEAK)
    && ( IS_NPC(ch) || !IS_SET(ch->act, PLR_WIZINVIS) ) )
    {
      if ( fall )
        txt = "falls";
      else
      if ( !txt )
      {
        if ( ch->mount )
        {
	  if ( IS_AFFECTED( ch->mount, AFF_FLOATING ) )
	    txt = "floats";
 	  else
	  if ( IS_AFFECTED( ch->mount, AFF_FLYING ) )
	    txt = "flys";
	  else
	    txt = "rides";
        }
        else
        {
	  if ( IS_AFFECTED( ch, AFF_FLOATING ) )
	  {
	    if ( drunk )
	      txt = "floats unsteadily";
	     else
	      txt = "floats";
	  }
	  else
	  if ( IS_AFFECTED( ch, AFF_FLYING ) )
	  {
	    if ( drunk )
	      txt = "flys shakily";
	     else
	      txt = "flys";
	  }
	  else
          if ( ch->position == POS_SHOVE )
            txt = "is shoved";
 	  else
	  if ( ch->position == POS_DRAG )
            txt = "is dragged";
  	  else
	  {
	    if ( drunk )
	      txt = "stumbles drunkenly";
	     else
	      txt = "leaves";
	  }
        }
      }
      if ( ch->mount )
      {
	sprintf( buf, "> $n %s %s upon $N", txt, dir_name[door] );
	act( AT_ACTION, buf, ch, NULL, ch->mount, TO_NOTVICT );
      }
      else
      {
	sprintf( buf, "> $n %s $T", txt );
	act( AT_ACTION, buf, ch, NULL, dir_name[door], TO_ROOM );
      }
    }

    rprog_leave_trigger( ch );
    if( char_died(ch) )
      return global_retcode;

    char_from_room( ch );
    if ( ch->mount )
    {
      rprog_leave_trigger( ch->mount );
      if( char_died(ch) )
        return global_retcode;
      if( ch->mount )
      {
        char_from_room( ch->mount );
        char_to_room( ch->mount, to_room );
      }
    }


    char_to_room( ch, to_room );
    if ( !IS_AFFECTED(ch, AFF_SNEAK)
    && ( IS_NPC(ch) || !IS_SET(ch->act, PLR_WIZINVIS) ) )
    {
      if ( fall )
        txt = "falls";
      else
      if ( ch->mount )
      {
	if ( IS_AFFECTED( ch->mount, AFF_FLOATING ) )
	  txt = "floats in";
	else
	if ( IS_AFFECTED( ch->mount, AFF_FLYING ) )
	  txt = "flys in";
	else
	  txt = "rides in";
      }
      else
      {
	if ( IS_AFFECTED( ch, AFF_FLOATING ) )
	{
	  if ( drunk )
	    txt = "floats in unsteadily";
	   else
	    txt = "floats in";
	}
	else
	if ( IS_AFFECTED( ch, AFF_FLYING ) )
	{
	  if ( drunk )
	    txt = "flys in shakily";
	   else
	    txt = "flys in";
	}
	else
	if ( ch->position == POS_SHOVE )
          txt = "is shoved in";
	else
	if ( ch->position == POS_DRAG )
	  txt = "is dragged in";
  	else
	{
	  if ( drunk )
	    txt = "stumbles drunkenly in";
	   else
	    txt = "arrives";
	}
      }
      switch( door )
      {
      default: dtxt = "somewhere";	break;
      case 0:  dtxt = "the south";	break;
      case 1:  dtxt = "the west";	break;
      case 2:  dtxt = "the north";	break;
      case 3:  dtxt = "the east";	break;
      case 4:  dtxt = "below";		break;
      case 5:  dtxt = "above";		break;
      case 6:  dtxt = "the south-west";	break;
      case 7:  dtxt = "the south-east";	break;
      case 8:  dtxt = "the north-west";	break;
      case 9:  dtxt = "the north-east";	break;
      }
      if ( ch->mount )
      {
	sprintf( buf, "> $n %s from %s upon $N", txt, dtxt );
	act( AT_ACTION, buf, ch, NULL, ch->mount, TO_ROOM );
      }
      else
      {
	sprintf( buf, "> $n %s from %s", txt, dtxt );
	act( AT_ACTION, buf, ch, NULL, NULL, TO_ROOM );
      }
    }

    do_look( ch, "auto" );
    if ( brief ) 
      SET_BIT( ch->act, PLR_BRIEF );


    /* BIG ugly looping problem here when the character is mptransed back
       to the starting room.  To avoid this, check how many chars are in 
       the room at the start and stop processing followers after doing
       the right number of them.  -- Narn
    */
    if ( !fall )
    {
      CHAR_DATA *fch;
      CHAR_DATA *nextinroom;
      int chars = 0, count = 0;

      for ( fch = from_room->first_person; fch; fch = fch->next_in_room )
        chars++;

      for ( fch = from_room->first_person; fch && ( count < chars ); fch = nextinroom )
      {
	nextinroom = fch->next_in_room;
        count++;
	if ( fch != ch		/* loop room bug fix here by Thoric */
	&& fch->master == ch
	&& fch->position == POS_STANDING )
	{
	act( AT_ACTION, "> you follow $N", fch, NULL, ch, TO_CHAR );
	    move_char( fch, pexit, 0 );
	}
      }
    }


    if ( char_died(ch) )
      return retcode;

    mprog_entry_trigger( ch );
    if ( char_died(ch) )
      return retcode;

    rprog_enter_trigger( ch );
    if ( char_died(ch) )
       return retcode;

    mprog_greet_trigger( ch );
    if ( char_died(ch) )
       return retcode;

    oprog_greet_trigger( ch );
    if ( char_died(ch) )
       return retcode;

    if (!will_fall( ch, fall )
    &&   fall > 0 )
    {
	if (!IS_AFFECTED( ch, AFF_FLOATING )
	|| ( ch->mount && !IS_AFFECTED( ch->mount, AFF_FLOATING ) ) )
	{
	  set_char_color( AT_HURT, ch );
	  send_to_char( "> you hit the ground\n\r", ch );
	  WAIT_STATE( ch, 20 );
	  retcode = damage( ch, ch, 50 * fall, TYPE_UNDEFINED );
	}
	else
	{
	  set_char_color( AT_MAGIC, ch );
	  send_to_char( "> you lightly float down to the ground\n\r", ch );
	}
    }
    return retcode;
}


void do_north( CHAR_DATA *ch, char *argument )
{
    move_char( ch, get_exit(ch->in_room, DIR_NORTH), 0 );
    return;
}


void do_east( CHAR_DATA *ch, char *argument )
{
    move_char( ch, get_exit(ch->in_room, DIR_EAST), 0 );
    return;
}


void do_south( CHAR_DATA *ch, char *argument )
{
    move_char( ch, get_exit(ch->in_room, DIR_SOUTH), 0 );
    return;
}


void do_west( CHAR_DATA *ch, char *argument )
{
    move_char( ch, get_exit(ch->in_room, DIR_WEST), 0 );
    return;
}


void do_up( CHAR_DATA *ch, char *argument )
{
    move_char( ch, get_exit(ch->in_room, DIR_UP), 0 );
    return;
}


void do_down( CHAR_DATA *ch, char *argument )
{
    move_char( ch, get_exit(ch->in_room, DIR_DOWN), 0 );
    return;
}

void do_northeast( CHAR_DATA *ch, char *argument )
{
    move_char( ch, get_exit(ch->in_room, DIR_NORTHEAST), 0 );
    return;
}

void do_northwest( CHAR_DATA *ch, char *argument )
{
    move_char( ch, get_exit(ch->in_room, DIR_NORTHWEST), 0 );
    return;
}

void do_southeast( CHAR_DATA *ch, char *argument )
{
    move_char( ch, get_exit(ch->in_room, DIR_SOUTHEAST), 0 );
    return;
}

void do_southwest( CHAR_DATA *ch, char *argument )
{
    move_char( ch, get_exit(ch->in_room, DIR_SOUTHWEST), 0 );
    return;
}



EXIT_DATA *find_door( CHAR_DATA *ch, char *arg, bool quiet )
{
    EXIT_DATA *pexit;
    int door;

    if (arg == NULL || !str_cmp(arg,""))
      return NULL;

    pexit = NULL;
	 if ( !str_cmp( arg, "n"  ) || !str_cmp( arg, "north"	  ) ) door = 0;
    else if ( !str_cmp( arg, "e"  ) || !str_cmp( arg, "east"	  ) ) door = 1;
    else if ( !str_cmp( arg, "s"  ) || !str_cmp( arg, "south"	  ) ) door = 2;
    else if ( !str_cmp( arg, "w"  ) || !str_cmp( arg, "west"	  ) ) door = 3;
    else if ( !str_cmp( arg, "u"  ) || !str_cmp( arg, "up"	  ) ) door = 4;
    else if ( !str_cmp( arg, "d"  ) || !str_cmp( arg, "down"	  ) ) door = 5;
    else if ( !str_cmp( arg, "ne" ) || !str_cmp( arg, "northeast" ) ) door = 6;
    else if ( !str_cmp( arg, "nw" ) || !str_cmp( arg, "northwest" ) ) door = 7;
    else if ( !str_cmp( arg, "se" ) || !str_cmp( arg, "southeast" ) ) door = 8;
    else if ( !str_cmp( arg, "sw" ) || !str_cmp( arg, "southwest" ) ) door = 9;
    else
    {
	for ( pexit = ch->in_room->first_exit; pexit; pexit = pexit->next )
	{
	    if ( (quiet || IS_SET(pexit->exit_info, EX_ISDOOR))
	    &&    pexit->keyword
	    &&    nifty_is_name( arg, pexit->keyword ) )
		return pexit;
	}
	if ( !quiet )
	  act( AT_PLAIN, "> you see no $T here\n\r", ch, NULL, arg, TO_CHAR );
	return NULL;
    }

    if ( (pexit = get_exit( ch->in_room, door )) == NULL )
    {
	if ( !quiet)
	  act( AT_PLAIN, "> you see no $T here\n\r", ch, NULL, arg, TO_CHAR );
	return NULL;
    }

    if ( quiet )
	return pexit;

    if ( IS_SET(pexit->exit_info, EX_SECRET) )
    {
	act( AT_PLAIN, "> you see no $T here\n\r", ch, NULL, arg, TO_CHAR );
	return NULL;
    }

    if ( !IS_SET(pexit->exit_info, EX_ISDOOR) )
    {
	send_to_char( "> invalid command\n\r", ch );
	return NULL;
    }

    return pexit;
}


void toggle_bexit_flag( EXIT_DATA *pexit, int flag )
{
    EXIT_DATA *pexit_rev;

    TOGGLE_BIT(pexit->exit_info, flag);
    if ( (pexit_rev = pexit->rexit) != NULL
    &&   pexit_rev != pexit )
	TOGGLE_BIT( pexit_rev->exit_info, flag );
}

void set_bexit_flag( EXIT_DATA *pexit, int flag )
{
    EXIT_DATA *pexit_rev;

    SET_BIT(pexit->exit_info, flag);
    if ( (pexit_rev = pexit->rexit) != NULL
    &&   pexit_rev != pexit )
	SET_BIT( pexit_rev->exit_info, flag );
}

void remove_bexit_flag( EXIT_DATA *pexit, int flag )
{
    EXIT_DATA *pexit_rev;

    REMOVE_BIT(pexit->exit_info, flag);
    if ( (pexit_rev = pexit->rexit) != NULL
    &&   pexit_rev != pexit )
	REMOVE_BIT( pexit_rev->exit_info, flag );
}

void do_open( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    EXIT_DATA *pexit;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	do_openhatch( ch , "" );
	return;
    }

    if ( ( pexit = find_door( ch, arg, TRUE ) ) != NULL )
    {
	/* 'open door' */
	EXIT_DATA *pexit_rev;

	if ( !IS_SET(pexit->exit_info, EX_ISDOOR) )
	    { send_to_char( "> invalid command\n\r",      ch ); return; }
	if ( !IS_SET(pexit->exit_info, EX_CLOSED) )
	    { send_to_char( "> it is already open\n\r",      ch ); return; }
	if (  IS_SET(pexit->exit_info, EX_LOCKED) )
	    { send_to_char( "> it is locked\n\r",            ch ); return; }

	if ( !IS_SET(pexit->exit_info, EX_SECRET)
	||   (pexit->keyword && nifty_is_name( arg, pexit->keyword )) )
	{
	    act( AT_ACTION, "> $n opens the $d\n\r", ch, NULL, pexit->keyword, TO_ROOM );
	    act( AT_ACTION, "> you open the $d\n\r", ch, NULL, pexit->keyword, TO_CHAR );
	    if ( (pexit_rev = pexit->rexit) != NULL
	    &&   pexit_rev->to_room == ch->in_room )
	    {
		CHAR_DATA *rch;

		for ( rch = pexit->to_room->first_person; rch; rch = rch->next_in_room )
		    act( AT_ACTION, "> the $d opens", rch, NULL, pexit_rev->keyword, TO_CHAR );
	        sound_to_room( pexit->to_room , "!!SOUND(door)" );
	    }
	    remove_bexit_flag( pexit, EX_CLOSED );
            return;
	}
    }

    if ( ( obj = get_obj_here( ch, arg ) ) != NULL )
    {
	/* 'open object' */
	if ( obj->item_type != ITEM_CONTAINER )
	{ 
          ch_printf( ch, "> %s isn't a container\n\r", obj->short_descr ); 
          return;
        } 
	if ( !IS_SET(obj->value[1], CONT_CLOSED) )
	{ 
          ch_printf( ch, "> %s is already open\n\r", obj->short_descr ); 
          return;
        } 
	if ( !IS_SET(obj->value[1], CONT_CLOSEABLE) )
	{ 
          ch_printf( ch, "> %s cannot be opened or closed\n\r", obj->short_descr ); 
          return;
        } 
	if ( IS_SET(obj->value[1], CONT_LOCKED) )
	{ 
          ch_printf( ch, "> %s is locked\n\r", obj->short_descr ); 
          return;
        } 

	REMOVE_BIT(obj->value[1], CONT_CLOSED);
	act( AT_ACTION, "> you open $p\n\r", ch, obj, NULL, TO_CHAR );
	act( AT_ACTION, "> $n opens $p\n\r", ch, obj, NULL, TO_ROOM );
	return;
    }

    if ( !str_cmp(arg , "hatch") );
    {
      do_openhatch( ch , argument );
      return;
    }

    do_openhatch( ch , arg );
    return;
}



void do_close( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    EXIT_DATA *pexit;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	do_closehatch(  ch , "" );
	return;
    }

    if ( ( pexit = find_door( ch, arg, TRUE ) ) != NULL )
    {
	/* 'close door' */
	EXIT_DATA *pexit_rev;

	if ( !IS_SET(pexit->exit_info, EX_ISDOOR) )
	    { send_to_char( "> invalid command\n\r",      ch ); return; }
	if ( IS_SET(pexit->exit_info, EX_CLOSED) )
	    { send_to_char( "> it is already closed\n\r",    ch ); return; }

	act( AT_ACTION, "> $n closes the $d\n\r", ch, NULL, pexit->keyword, TO_ROOM );
	act( AT_ACTION, "> you close the $d\n\r", ch, NULL, pexit->keyword, TO_CHAR );

	/* close the other side */
	if ( ( pexit_rev = pexit->rexit ) != NULL
	&&   pexit_rev->to_room == ch->in_room )
	{
	    CHAR_DATA *rch;

	    SET_BIT( pexit_rev->exit_info, EX_CLOSED );
	    for ( rch = pexit->to_room->first_person; rch; rch = rch->next_in_room )
		act( AT_ACTION, "> the $d closes\n\r", rch, NULL, pexit_rev->keyword, TO_CHAR );
	}
	set_bexit_flag( pexit, EX_CLOSED );
        return;
    }

    if ( ( obj = get_obj_here( ch, arg ) ) != NULL )
    {
	/* 'close object' */
	if ( obj->item_type != ITEM_CONTAINER )
	{ 
          ch_printf( ch, "> %s is not a container\n\r", obj->short_descr ); 
          return;
        } 
	if ( IS_SET(obj->value[1], CONT_CLOSED) )
	{ 
          ch_printf( ch, "> %s is already closed\n\r", obj->short_descr ); 
          return;
        } 
	if ( !IS_SET(obj->value[1], CONT_CLOSEABLE) )
        { 
          ch_printf( ch, "> %s cannot be opened or closed\n\r", obj->short_descr ); 
          return;
        } 

	SET_BIT(obj->value[1], CONT_CLOSED);
	act( AT_ACTION, "> you close $p\n\r", ch, obj, NULL, TO_CHAR );
	act( AT_ACTION, "> $n closes $p\n\r", ch, obj, NULL, TO_ROOM );
	return;
    }

    if ( !str_cmp( arg , "hatch" ) )
    {
    	do_closehatch( ch , argument );
    	return;
    }

    	do_closehatch( ch , arg );
    return;
}



void do_lock( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    EXIT_DATA *pexit;

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "> lock what?\n\r", ch );
	return;
    }

    if ( ( pexit = find_door( ch, arg, TRUE ) ) != NULL )
    {
	/* 'lock door' */

	if ( !IS_SET(pexit->exit_info, EX_ISDOOR) )
	    { send_to_char( "> invalid command\n\r",      ch ); return; }
	if ( !IS_SET(pexit->exit_info, EX_CLOSED) )
	    { send_to_char( "> it is not closed\n\r",        ch ); return; }
	if ( pexit->key < 0 )
	    { send_to_char( "> it cannot be locked\n\r",     ch ); return; }
	if ( atoi(argument) != pexit->key )
	    { send_to_char( "> wrong code\n\r",       ch ); return; }
	if ( IS_SET(pexit->exit_info, EX_LOCKED) )
	    { send_to_char( "> it is already locked\n\r",    ch ); return; }

	if ( !IS_SET(pexit->exit_info, EX_SECRET)
	||   (pexit->keyword && nifty_is_name( arg, pexit->keyword )) )
	{
	    send_to_char( "> *click*\n\r", ch );
	    act( AT_ACTION, "> $n locks the $d\n\r", ch, NULL, pexit->keyword, TO_ROOM );
	    set_bexit_flag( pexit, EX_LOCKED );
            return;
        }
    }

    if ( ( obj = get_obj_here( ch, arg ) ) != NULL )
    {
	/* 'lock object' */
	if ( obj->item_type != ITEM_CONTAINER )
	    { send_to_char( "> that is not a container\n\r", ch ); return; }
	if ( !IS_SET(obj->value[1], CONT_CLOSED) )
	    { send_to_char( "> it is not closed\n\r",        ch ); return; }
	if ( obj->value[2] < 0 )
	    { send_to_char( "> it cannot be locked\n\r",     ch ); return; }
	if ( atoi(argument) != obj->value[2] )
	    { send_to_char( "> wrong code\n\r",       ch ); return; }
	if ( IS_SET(obj->value[1], CONT_LOCKED) )
	    { send_to_char( "> it is already locked\n\r",    ch ); return; }

	SET_BIT(obj->value[1], CONT_LOCKED);
	send_to_char( "> *click*\n\r", ch );
	act( AT_ACTION, "> $n locks $p\n\r", ch, obj, NULL, TO_ROOM );
	return;
    }

    ch_printf( ch, "> you see no %s here\n\r", arg );
    return;
}



void do_unlock( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    EXIT_DATA *pexit;

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "> unlock what\n\r", ch );
	return;
    }

    if ( ( pexit = find_door( ch, arg, TRUE ) ) != NULL )
    {
	/* 'unlock door' */

	if ( !IS_SET(pexit->exit_info, EX_ISDOOR) )
	    { send_to_char( "> invalid command\n\r",      ch ); return; }
	if ( !IS_SET(pexit->exit_info, EX_CLOSED) )
	    { send_to_char( "> it is not closed\n\r",        ch ); return; }
	if ( pexit->key < 0 )
	    { send_to_char( "> it cannot be unlocked\n\r",   ch ); return; }
	if ( atoi(argument) != pexit->key )
	    { send_to_char( "> wrong combination\n\r",       ch ); return; }
	if ( !IS_SET(pexit->exit_info, EX_LOCKED) )
	    { send_to_char( "> it is already unlocked\n\r",  ch ); return; }

	if ( !IS_SET(pexit->exit_info, EX_SECRET)
	||   (pexit->keyword && nifty_is_name( arg, pexit->keyword )) )
	{
	    send_to_char( "> *click*\n\r", ch );
	    act( AT_ACTION, "> $n unlocks the $d\n\r", ch, NULL, pexit->keyword, TO_ROOM );
	    remove_bexit_flag( pexit, EX_LOCKED );
            return;   
	}
    }

    if ( ( obj = get_obj_here( ch, arg ) ) != NULL )
    {
	/* 'unlock object' */
	if ( obj->item_type != ITEM_CONTAINER )
	    { send_to_char( "> that is not a container\n\r", ch ); return; }
	if ( !IS_SET(obj->value[1], CONT_CLOSED) )
	    { send_to_char( "> it is not closed\n\r",        ch ); return; }
	if ( obj->value[2] < 0 )
	    { send_to_char( "> it cannot be unlocked\n\r",   ch ); return; }
	if ( atoi(argument) != obj->value[2] )
	    { send_to_char( "> wrong combination\n\r",       ch ); return; }
	if ( !IS_SET(obj->value[1], CONT_LOCKED) )
	    { send_to_char( "> it is already unlocked\n\r",  ch ); return; }

	REMOVE_BIT(obj->value[1], CONT_LOCKED);
	send_to_char( "> *click*\n\r", ch );
	act( AT_ACTION, "> $n unlocks $p\n\r", ch, obj, NULL, TO_ROOM );
	return;
    }

    ch_printf( ch, "> you see no %s here\n\r", arg );
    return;
}

void do_bashdoor( CHAR_DATA *ch, char *argument )
{
	CHAR_DATA *gch;
	EXIT_DATA *pexit;
	char       arg [ MAX_INPUT_LENGTH ];

	if ( !IS_NPC( ch )
	&&  ch->pcdata->learned[gsn_bashdoor] <= 0  )
	{
	    send_to_char( "> you are not enough of a warrior to bash doors\n\r", ch );
	    return;
	}

	one_argument( argument, arg );

	if ( arg[0] == '\0' )
	{
	    send_to_char( "> bash what\n\r", ch );
	    return;
	}

	if ( ch->fighting )
	{
	    send_to_char( "> you cannot break off your fight\n\r", ch );
	    return;
	}

	if ( ( pexit = find_door( ch, arg, FALSE ) ) != NULL )
	{
	    ROOM_INDEX_DATA *to_room;
	    EXIT_DATA       *pexit_rev;
	    int              chance;
	    char	    *keyword;

	    if ( !IS_SET( pexit->exit_info, EX_CLOSED ) )
	    {
		send_to_char( "> it is already open\n\r", ch );
		return;
	    }

	    WAIT_STATE( ch, skill_table[gsn_bashdoor]->beats );

	    if ( IS_SET( pexit->exit_info, EX_SECRET ) )
		keyword = "wall";
	    else
		keyword = pexit->keyword;
	    if ( !IS_NPC(ch) )
		chance = ch->pcdata->learned[gsn_bashdoor] / 2;
	    else
		chance = 90;

	    if ( !IS_SET( pexit->exit_info, EX_BASHPROOF )
	    &&   ch->move >= 15
	    &&   number_percent( ) < ( chance + 4 * ( get_curr_str( ch ) - 19 ) ) )
	    {
		REMOVE_BIT( pexit->exit_info, EX_CLOSED );
		if ( IS_SET( pexit->exit_info, EX_LOCKED ) )
		REMOVE_BIT( pexit->exit_info, EX_LOCKED );
		SET_BIT( pexit->exit_info, EX_BASHED );

		act(AT_SKILL, "> you bashed open the $d", ch, NULL, keyword, TO_CHAR );
		act(AT_SKILL, "> $n bashes open the $d",          ch, NULL, keyword, TO_ROOM );
		learn_from_success(ch, gsn_bashdoor);

		if ( (to_room = pexit->to_room) != NULL
		&&   (pexit_rev = pexit->rexit) != NULL
		&&    pexit_rev->to_room	== ch->in_room )
		{
			CHAR_DATA *rch;

			REMOVE_BIT( pexit_rev->exit_info, EX_CLOSED );
			if ( IS_SET( pexit_rev->exit_info, EX_LOCKED ) )
			  REMOVE_BIT( pexit_rev->exit_info, EX_LOCKED );
			SET_BIT( pexit_rev->exit_info, EX_BASHED );

			for ( rch = to_room->first_person; rch; rch = rch->next_in_room )
			{
			    act(AT_SKILL, "> the $d crashes open\n\r",
				rch, NULL, pexit_rev->keyword, TO_CHAR );
			}
		}
		damage( ch, ch, ( ch->max_hit / 20 ), gsn_bashdoor );

	    }
	    else
	    {
		act(AT_SKILL, "> you bash against the $d but it does not budge",
			ch, NULL, keyword, TO_CHAR );
		act(AT_SKILL, "> $n bashes against the $d but it holds strong",
			ch, NULL, keyword, TO_ROOM );
		damage( ch, ch, ( ch->max_hit / 20 ) + 10, gsn_bashdoor );
		learn_from_failure(ch, gsn_bashdoor);
	    }
	}    
	else
	{
	    act(AT_SKILL, "> you bash against the wall but it does not budge",
		ch, NULL, NULL, TO_CHAR );
	    act(AT_SKILL, "> $n bashes against the wall but it holds strong",
		ch, NULL, NULL, TO_ROOM );
	    damage( ch, ch, ( ch->max_hit / 20 ) + 10, gsn_bashdoor );
	    learn_from_failure(ch, gsn_bashdoor);
	}
	if ( !char_died( ch ) )
	   for ( gch = ch->in_room->first_person; gch; gch = gch->next_in_room )
	   {
		 if ( IS_AWAKE( gch )
		 && !gch->fighting
		 && ( IS_NPC( gch ) && !IS_AFFECTED( gch, AFF_CHARM ) )
		 && ( ch->top_level - gch->top_level <= 4 )
		 && number_bits( 2 ) == 0 )
		 multi_hit( gch, ch, TYPE_UNDEFINED );
	   }

        return;
}


void do_stand( CHAR_DATA *ch, char *argument )
{
    switch ( ch->position )
    {
    case POS_SLEEPING:
	if ( IS_AFFECTED(ch, AFF_SLEEP) )
	    { send_to_char( "> you cannot seem to wake up\n\r", ch ); return; }

	send_to_char( "> you wake and climb quickly to your feet\n\r", ch );
	act( AT_ACTION, "> $n arises from $s slumber", ch, NULL, NULL, TO_ROOM );
	ch->position = POS_STANDING;
	break;

    case POS_RESTING:
        send_to_char( "> you gather yourself and stand up\n\r", ch );
	act( AT_ACTION, "> $n rises from $s rest", ch, NULL, NULL, TO_ROOM );
        ch->position = POS_STANDING;
        break;

    case POS_SITTING:
	send_to_char( "> you move quickly to your feet\n\r", ch );
	act( AT_ACTION, "> $n rises up", ch, NULL, NULL, TO_ROOM );
	ch->position = POS_STANDING;
	break;

    case POS_STANDING:
	send_to_char( "> you are already standing\n\r", ch );
	break;

    case POS_FIGHTING:
	send_to_char( "> you are already fighting\n\r", ch );
	break;
    }

    return;
}


void do_sit( CHAR_DATA *ch, char *argument )
{
    switch ( ch->position )
    {
    case POS_SLEEPING:
	if ( IS_AFFECTED(ch, AFF_SLEEP) )
            { send_to_char( "> you cannot seem to wake up\n\r", ch ); return; }

	send_to_char( "> you wake and sit up\n\r", ch );
	act( AT_ACTION, "> $n wakes and sits up\n\r", ch, NULL, NULL, TO_ROOM );
	ch->position = POS_SITTING;
	break;

    case POS_RESTING:
        send_to_char( "> you stop resting and sit up\n\r", ch );
	act( AT_ACTION, "> $n stops resting and sits up\n\r", ch, NULL, NULL, TO_ROOM );
	ch->position = POS_SITTING;
	break;

    case POS_STANDING:
        send_to_char( "> you sit down\n\r", ch );
	act( AT_ACTION, "> $n sits down\n\r", ch, NULL, NULL, TO_ROOM );
        ch->position = POS_SITTING;
        break;
    case POS_SITTING:
	send_to_char( "> you are already sitting\n\r", ch );
	return;

    case POS_FIGHTING:
	send_to_char( "> you are busy fighting\n\r", ch );
	return;
    case POS_MOUNTED:
        send_to_char( "> you are already sitting - on your mount\n\r", ch );
        return;
    }

    return;
}


void do_rest( CHAR_DATA *ch, char *argument )
{
    switch ( ch->position )
    {
    case POS_SLEEPING:
	if ( IS_AFFECTED(ch, AFF_SLEEP) )
            { send_to_char( "> you cannot seem to wake up\n\r", ch ); return; }

	send_to_char( "> you rouse from your slumber\n\r", ch );
	act( AT_ACTION, "> $n rouses from $s slumber\n\r", ch, NULL, NULL, TO_ROOM );
        ch->position = POS_RESTING;
	break;

    case POS_RESTING:
	send_to_char( "> you are already resting\n\r", ch );
	return;

    case POS_STANDING:
        send_to_char( "> you sprawl out haphazardly\n\r", ch );
	act( AT_ACTION, "> $n sprawls out haphazardly\n\r", ch, NULL, NULL, TO_ROOM );
        ch->position = POS_RESTING;
        break;

    case POS_SITTING:
        send_to_char( "> you lie back and sprawl out to rest\n\r", ch );
	act( AT_ACTION, "> $n lies back and sprawls out to rest\n\r", ch, NULL, NULL, TO_ROOM );
	ch->position = POS_RESTING;
	break;

    case POS_FIGHTING:
	send_to_char( "> you are busy fighting\n\r", ch );
	return;
    case POS_MOUNTED:
        send_to_char( "> you should dismount first\n\r", ch );
        return;
    }

    rprog_rest_trigger( ch );
    return;
}


void do_sleep( CHAR_DATA *ch, char *argument )
{
    switch ( ch->position )
    {
    case POS_SLEEPING:
	send_to_char( "> you are already sleeping\n\r", ch );
	return;

    case POS_RESTING:
	if ( ch->mental_state > 30 && (number_percent()+10) < ch->mental_state )
	{
	    send_to_char( "> you just cannot seem to calm yourself down enough to sleep\n\r", ch );
	    act( AT_ACTION, "> $n closes $s eyes for a few moments but just cannot seem to go to sleep\n\r", ch, NULL, NULL, TO_ROOM );
	    return;
	}
        send_to_char( "> you close your eyes and drift into slumber\n\r", ch );
	act( AT_ACTION, "> $n closes $s eyes and drifts into a deep slumber\n\r", ch, NULL, NULL, TO_ROOM );
        ch->position = POS_SLEEPING;
        break;

    case POS_SITTING:
	if ( ch->mental_state > 30 && (number_percent()+5) < ch->mental_state )
	{
	    send_to_char( "> you just cannot seem to calm yourself down enough to sleep\n\r", ch );
	    act( AT_ACTION, "> $n closes $s eyes for a few moments but just cannot seem to go to sleep\n\r", ch, NULL, NULL, TO_ROOM );
	    return;
	}
        send_to_char( "> you slump over and fall dead asleep\n\r", ch );
	act( AT_ACTION, "> $n nods off and slowly slumps over dead asleep\n\r", ch, NULL, NULL, TO_ROOM );
        ch->position = POS_SLEEPING;
        break;

    case POS_STANDING: 
	if ( ch->mental_state > 30 && number_percent() < ch->mental_state )
	{
	    send_to_char( "> you just cannot seem to calm yourself down enough to sleep\n\r", ch );
	    act( AT_ACTION, "> $n closes $s eyes for a few moments but just cannot seem to go to sleep\n\r", ch, NULL, NULL, TO_ROOM );
	    return;
	}
        send_to_char( "> you collapse into a deep sleep\n\r", ch );
	act( AT_ACTION, "> $n collapses into a deep sleep\n\r", ch, NULL, NULL, TO_ROOM );
        ch->position = POS_SLEEPING;
        break;

    case POS_FIGHTING:
	send_to_char( "> you are busy fighting\n\r", ch );
	return;
    case POS_MOUNTED:
        send_to_char( "> you really should dismount first\n\r", ch );
        return;
    }

    rprog_sleep_trigger( ch );
    return;
}


void do_wake( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
	{ do_stand( ch, argument ); return; }

    if ( !IS_AWAKE(ch) )
	{ send_to_char( "> you are asleep yourself\n\r",       ch ); return; }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
	{ send_to_char( "> user not found\n\r",              ch ); return; }

    if ( IS_AWAKE(victim) )
    { act( AT_PLAIN, "> $N is already awake", ch, NULL, victim, TO_CHAR ); return; }

    if ( IS_AFFECTED(victim, AFF_SLEEP) || victim->position < POS_SLEEPING )
    { act( AT_PLAIN, "> you cannot seem to wake $M!",  ch, NULL, victim, TO_CHAR );  return; }

    act( AT_ACTION, "> you wake $M", ch, NULL, victim, TO_CHAR );
    victim->position = POS_STANDING;
    act( AT_ACTION, "> $n wakes you", ch, NULL, victim, TO_VICT );
    return;
}


/*
 * teleport a character to another room
 */
void teleportch( CHAR_DATA *ch, ROOM_INDEX_DATA *room, bool show )
{
    if ( room_is_private( ch, room ) )
      return;
    act( AT_ACTION, "> $n disappears suddenly!", ch, NULL, NULL, TO_ROOM );
    char_from_room( ch );
    char_to_room( ch, room );
    act( AT_ACTION, "> $n arrives suddenly!", ch, NULL, NULL, TO_ROOM );
    if ( show )
      do_look( ch, "auto" );
}

void teleport( CHAR_DATA *ch, sh_int room, int flags )
{
    CHAR_DATA *nch, *nch_next;
    ROOM_INDEX_DATA *pRoomIndex;
    bool show;

    pRoomIndex = get_room_index( room );
    if ( !pRoomIndex )
    {
	bug( "teleport: bad room vnum %d", room );
	return;
    }

    if ( IS_SET( flags, TELE_SHOWDESC ) )
      show = TRUE;
    else
      show = FALSE;
    if ( !IS_SET( flags, TELE_TRANSALL ) )
    {
	teleportch( ch, pRoomIndex, show );
	return;
    }
    for ( nch = ch->in_room->first_person; nch; nch = nch_next )
    {
	nch_next = nch->next_in_room;
	teleportch( nch, pRoomIndex, show );
    }
}

/*
 * "Climb" in a certain direction.				-Thoric
 */
void do_climb( CHAR_DATA *ch, char *argument )
{
    EXIT_DATA *pexit;
    bool found;

    found = FALSE;
    if ( argument[0] == '\0' )
    {
	for ( pexit = ch->in_room->first_exit; pexit; pexit = pexit->next )
	    if ( IS_SET( pexit->exit_info, EX_xCLIMB ) )
	    {
		move_char( ch, pexit, 0 );
		return;
	    }
	send_to_char( "> you cannot climb here\n\r", ch );
	return;
    }

    if ( (pexit = find_door( ch, argument, TRUE )) != NULL
    &&   IS_SET( pexit->exit_info, EX_xCLIMB ))
    {
	move_char( ch, pexit, 0 );
	return;
    }
    send_to_char( "> you cannot climb there\n\r", ch );
    return;
}

/*
 * "enter" something (moves through an exit)			-Thoric
 */
void do_enter( CHAR_DATA *ch, char *argument )
{
    EXIT_DATA *pexit;
    bool found;

    found = FALSE;
    if ( argument[0] == '\0' )
    {
	for ( pexit = ch->in_room->first_exit; pexit; pexit = pexit->next )
	    if ( IS_SET( pexit->exit_info, EX_xENTER ) )
	    {
		move_char( ch, pexit, 0 );
		return;
	    }
	send_to_char( "> you cannot find an entrance here\n\r", ch );
	return;
    }

    if ( (pexit = find_door( ch, argument, TRUE )) != NULL
    &&   IS_SET( pexit->exit_info, EX_xENTER ))
    {
	move_char( ch, pexit, 0 );
	return;
    }
    do_board(ch,argument);
    return;
}

/*
 * Leave through an exit.					-Thoric
 */
void do_leave( CHAR_DATA *ch, char *argument )
{
    EXIT_DATA *pexit;
    bool found;

    found = FALSE;
    if ( argument[0] == '\0' )
    {
	for ( pexit = ch->in_room->first_exit; pexit; pexit = pexit->next )
	    if ( IS_SET( pexit->exit_info, EX_xLEAVE ) )
	    {
		move_char( ch, pexit, 0 );
		return;
	    }
	do_leaveship( ch , "" );
	return;
    }

    if ( (pexit = find_door( ch, argument, TRUE )) != NULL
    &&   IS_SET( pexit->exit_info, EX_xLEAVE ))
    {
	move_char( ch, pexit, 0 );
	return;
    }
    do_leaveship( ch, "" );
    return;
}

//done for neuro
