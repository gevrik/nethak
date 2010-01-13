#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
/* #include <stdlib.h> */
#include <time.h>
#include "mud.h"

extern int top_area;
extern int top_r_vnum;
void write_area_list();
void write_starsystem_list();
extern const   char *  sector_name     [SECT_MAX];

PLANET_DATA * first_planet;
PLANET_DATA * last_planet;

GUARD_DATA * first_guard;
GUARD_DATA * last_guard;

/* from build.c */
int	get_pflag( char *flag );

/* local routines */
void	fread_planet	args( ( PLANET_DATA *planet, FILE *fp ) );
bool	load_planet_file	args( ( char *planetfile ) );
void	write_planet_list	args( ( void ) );

/*
Parse Planet Names
*/

bool check_parse_pname( char *name )
{
    /* Good idea to prevent players from making inappropiate names, but that isn't going to stop everything. */

    if ( is_name( name, "faggot fag chink nigger fuck ass bitch retard gaylord axid dixa craxid urine wintermute neuromancer shit poop limbo" ) )
	return FALSE;

    if ( strlen(name) > 20 )
	return FALSE;

    /*
     * Alphanumerics only.
     */
    {
	char *pc;

	for ( pc = name; *pc != '\0'; pc++ )
	    if ( !isalpha(*pc) )
		return FALSE;
    }

    return TRUE;
}


PLANET_DATA *get_planet( char *name )
{
    PLANET_DATA *planet;

    if ( name[0] == '\0' )
       return NULL;

    for ( planet = first_planet; planet; planet = planet->next )
       if ( !str_cmp( name, planet->name ) )
         return planet;

    for ( planet = first_planet; planet; planet = planet->next )
       if ( nifty_is_name( name, planet->name ) )
         return planet;

    for ( planet = first_planet; planet; planet = planet->next )
       if ( !str_prefix( name, planet->name ) )
         return planet;

    for ( planet = first_planet; planet; planet = planet->next )
       if ( nifty_is_name_prefix( name, planet->name ) )
         return planet;

    return NULL;
}

void write_planet_list( )
{
    PLANET_DATA *tplanet;
    FILE *fpout;
    char filename[256];

    sprintf( filename, "%s%s", PLANET_DIR, PLANET_LIST );
    fpout = fopen( filename, "w" );
    if ( !fpout )
    {
	bug( "FATAL: cannot open planet.lst for writing!\n\r", 0 );
 	return;
    }
    for ( tplanet = first_planet; tplanet; tplanet = tplanet->next )
	fprintf( fpout, "%s\n", tplanet->filename );
    fprintf( fpout, "$\n" );
    fclose( fpout );
}

void save_planet( PLANET_DATA *planet )
{
    FILE *fp;
    char filename[256];
    char buf[MAX_STRING_LENGTH];

    if ( !planet )
    {
	bug( "save_planet: null planet pointer!", 0 );
	return;
    }

    if ( !planet->filename || planet->filename[0] == '\0' )
    {
	sprintf( buf, "save_planet: %s has no filename", planet->name );
	bug( buf, 0 );
	return;
    }

    sprintf( filename, "%s%s", PLANET_DIR, planet->filename );

    fclose( fpReserve );
    if ( ( fp = fopen( filename, "w" ) ) == NULL )
    {
    	bug( "save_planet: fopen", 0 );
    	perror( filename );
    }
    else
    {
        AREA_DATA *pArea;

	fprintf( fp, "#PLANET\n" );
	fprintf( fp, "Name         %s~\n",	planet->name		);
	fprintf( fp, "Filename     %s~\n",	planet->filename        );
	fprintf( fp, "X            %d\n",	planet->x               );
	fprintf( fp, "Y            %d\n",	planet->y               );
	fprintf( fp, "Z            %d\n",	planet->z               );
	fprintf( fp, "Sector       %d\n",	planet->sector          );
	fprintf( fp, "PopSupport   %d\n",	(int) (planet->pop_support)      );
	if ( planet->starsystem && planet->starsystem->name )
        	fprintf( fp, "Starsystem   %s~\n",	planet->starsystem->name);
	if ( planet->governed_by && planet->governed_by->name )
        	fprintf( fp, "GovernedBy   %s~\n",	planet->governed_by->name);
	pArea = planet->area;
	if (pArea->filename)
         	fprintf( fp, "Area         %s~\n",	pArea->filename  );
	fprintf( fp, "Flags	   %d\n",	planet->flags		);
	fprintf( fp, "End\n\n"						);
	fprintf( fp, "#END\n"						);
    }
    fclose( fp );
    fpReserve = fopen( NULL_FILE, "r" );
    return;
}

#if defined(KEY)
#undef KEY
#endif

#define KEY( literal, field, value )					\
				if ( !str_cmp( word, literal ) )	\
				{					\
				    field  = value;			\
				    fMatch = TRUE;			\
				    break;				\
				}


void fread_planet( PLANET_DATA *planet, FILE *fp )
{
    char buf[MAX_STRING_LENGTH];
    char *word;
    bool fMatch;

    for ( ; ; )
    {
	word   = feof( fp ) ? "End" : fread_word( fp );
	fMatch = FALSE;

	switch ( UPPER(word[0]) )
	{
	case '*':
	    fMatch = TRUE;
	    fread_to_eol( fp );
	    break;

	case 'A':
	    if ( !str_cmp( word, "Area" ) )
	    {
	        char aName[MAX_STRING_LENGTH];
                AREA_DATA *pArea;

	     	sprintf (aName, fread_string(fp));
		for( pArea = first_area ; pArea ; pArea = pArea->next )
	          if (pArea->filename && !str_cmp(pArea->filename , aName ) )
	          {
	             ROOM_INDEX_DATA *room;

	             planet->size = 0;
	             planet->citysize = 0;
	             planet->wilderness = 0;
	             planet->farmland = 0;
	             planet->barracks = 0;
	             planet->controls = 0;
	             pArea->planet = planet;
	             planet->area = pArea;
	             for( room = pArea->first_room ; room ; room = room->next_in_area )
                     {
                       	  planet->size++;
                          if ( room->sector_type <= SECT_CITY )
                             planet->citysize++;
                          else if ( room->sector_type == SECT_FARMLAND )
                             planet->farmland++;
                          else if ( room->sector_type != SECT_DUNNO )
                             planet->wilderness++;

                          if ( IS_SET( room->room_flags , ROOM_CONTROL ))
                             planet->controls++;
                          if ( IS_SET( room->room_flags , ROOM_BARRACKS ))
                             planet->barracks++;
                     }
	          }
                fMatch = TRUE;
	    }
	    break;

	case 'E':
	    if ( !str_cmp( word, "End" ) )
	    {
		if (!planet->name)
		  planet->name		= STRALLOC( "" );
		return;
	    }
	    break;

	case 'F':
	    KEY( "Filename",	planet->filename,		fread_string_nohash( fp ) );
	    KEY( "Flags",	planet->flags,			fread_number( fp ) );
	    break;

	case 'G':
	    if ( !str_cmp( word, "GovernedBy" ) )
	    {
	     	planet->governed_by = get_clan ( fread_string(fp) );
                fMatch = TRUE;
	    }
	    break;

	case 'N':
	    KEY( "Name",	planet->name,		fread_string( fp ) );
	    break;

	case 'P':
	    KEY( "PopSupport",	planet->pop_support,		fread_float( fp ) );
	    break;

	case 'S':
	    KEY( "Sector",	planet->sector,		fread_number( fp ) );
	    if ( !str_cmp( word, "Starsystem" ) )
	    {
	     	planet->starsystem = starsystem_from_name ( fread_string(fp) );
                if (planet->starsystem)
                {
                     SPACE_DATA *starsystem = planet->starsystem;

                     LINK( planet , starsystem->first_planet, starsystem->last_planet, next_in_system, prev_in_system );
                }
                fMatch = TRUE;
	    }
	    break;

	case 'X':
	    KEY( "X",	planet->x,		fread_number( fp ) );
	    break;

	case 'Y':
	    KEY( "Y",	planet->y,		fread_number( fp ) );
	    break;

	case 'Z':
	    KEY( "Z",	planet->z,		fread_number( fp ) );
	    break;

	}

	if ( !fMatch )
	{
	    sprintf( buf, "Fread_planet: no match: %s", word );
	    bug( buf, 0 );
	}

    }
}

bool load_planet_file( char *planetfile )
{
    char filename[256];
    PLANET_DATA *planet;
    FILE *fp;
    bool found;

    CREATE( planet, PLANET_DATA, 1 );

    planet->governed_by = NULL;
    planet->next_in_system = NULL;
    planet->prev_in_system = NULL;
    planet->starsystem = NULL ;
    planet->area = NULL;
    planet->first_guard = NULL;
    planet->last_guard = NULL;

    found = FALSE;
    sprintf( filename, "%s%s", PLANET_DIR, planetfile );

    if ( ( fp = fopen( filename, "r" ) ) != NULL )
    {

	found = TRUE;
	for ( ; ; )
	{
	    char letter;
	    char *word;

	    letter = fread_letter( fp );
	    if ( letter == '*' )
	    {
		fread_to_eol( fp );
		continue;
	    }

	    if ( letter != '#' )
	    {
		bug( "Load_planet_file: # not found.", 0 );
		break;
	    }

	    word = fread_word( fp );
	    if ( !str_cmp( word, "PLANET"	) )
	    {
	    	fread_planet( planet, fp );
	    	break;
	    }
	    else
	    if ( !str_cmp( word, "END"	) )
	        break;
	    else
	    {
		char buf[MAX_STRING_LENGTH];

		sprintf( buf, "Load_planet_file: bad section: %s.", word );
		bug( buf, 0 );
		break;
	    }
	}
	fclose( fp );
    }

    if ( !found )
      DISPOSE( planet );
    else
      LINK( planet, first_planet, last_planet, next, prev );

    return found;
}

void load_planets( )
{
    FILE *fpList;
    char *filename;
    char planetlist[256];
    char buf[MAX_STRING_LENGTH];

    first_planet	= NULL;
    last_planet	= NULL;

    log_string( "Loading planets..." );

    sprintf( planetlist, "%s%s", PLANET_DIR, PLANET_LIST );
    fclose( fpReserve );
    if ( ( fpList = fopen( planetlist, "r" ) ) == NULL )
    {
	perror( planetlist );
	exit( 1 );
    }

    for ( ; ; )
    {
	filename = feof( fpList ) ? "$" : fread_word( fpList );
	log_string( filename );
	if ( filename[0] == '$' )
	  break;

	if ( !load_planet_file( filename ) )
	{
	  sprintf( buf, "Cannot load planet file: %s", filename );
	  bug( buf, 0 );
	}
    }
    fclose( fpList );
    log_string(" Done planets " );
    fpReserve = fopen( NULL_FILE, "r" );
    return;
}

void do_setplanet( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char arg3[MAX_INPUT_LENGTH];
    PLANET_DATA *planet;
    int value;

    if ( IS_NPC( ch ) )
    {
	send_to_char( "Huh?\n\r", ch );
	return;
    }

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' )
    {
	send_to_char( "Usage: setsystem <system> <field> [value]\n\r", ch );
	send_to_char( "\n\rField being one of:\n\r", ch );
	send_to_char( " name filename starsystem governed_by\n\r", ch );
	return;
    }

    planet = get_planet( arg1 );
    if ( !planet )
    {
	send_to_char( "> no such system\n\r", ch );
	return;
    }


    if ( !strcmp( arg2, "name" ) )
    {
	STRFREE( planet->name );
	planet->name = STRALLOC( argument );
	send_to_char( "Done\n\r", ch );
	save_planet( planet );
	return;
    }

    if ( !strcmp( arg2, "popsupport" ) )
    {
	planet->pop_support = atoi( argument );
	send_to_char( "Done\n\r", ch );
	save_planet( planet );
	return;
    }

    if ( !strcmp( arg2, "sector" ) )
    {
	planet->sector = atoi(argument);
	send_to_char( "Done\n\r", ch );
	save_planet( planet );
	return;
    }

    if ( !strcmp( arg2, "governed_by" ) )
    {
        CLAN_DATA *clan;
        clan = get_clan( argument );
        if ( clan )
        {
           planet->governed_by = clan;
           send_to_char( "Done\n\r", ch );
       	   save_planet( planet );
        }
        else
           send_to_char( "> no such clan\n\r", ch );
	return;
    }

    if ( !strcmp( arg2, "starsystem" ) )
    {
        SPACE_DATA *starsystem;

        if ((starsystem=planet->starsystem) != NULL)
          UNLINK(planet, starsystem->first_planet, starsystem->last_planet, next_in_system, prev_in_system);
	if ( (planet->starsystem = starsystem_from_name(argument)) )
        {
           starsystem = planet->starsystem;
           LINK(planet, starsystem->first_planet, starsystem->last_planet, next_in_system, prev_in_system);
           send_to_char( "Done\n\r", ch );
	}
	else
	       	send_to_char( "> no such starsystem\n\r", ch );
	save_planet( planet );
	return;
    }

    if ( !strcmp( arg2, "filename" ) )
    {
	DISPOSE( planet->filename );
	planet->filename = str_dup( argument );
	send_to_char( "Done\n\r", ch );
	save_planet( planet );
	write_planet_list( );
	return;
    }


    do_setplanet( ch, "" );

    if( !strcmp( arg2, "flags" ) )
    {
        if( argument[0] == '\0' )
	{
	   send_to_char("USAGE: setcomplex <complex> flags <flag>\n\r", ch );
	   send_to_char("Flags may be one of the following:\n\r\n\r", ch );
	   send_to_char("nocap, noinvade, nopedit, hidden, explorable,\n\r", ch );
	   send_to_char("shut, notarget\n\r", ch );
	}
	while ( argument[0] != '\0' )
	{
	   argument = one_argument( argument, arg3 );
	   value = get_pflag( arg3 );
	   if ( value < 0 || value > 31 )
	     ch_printf( ch, "Unknown flag: %s\n\r", arg3 );
	   else
	   {
	       TOGGLE_BIT( planet->flags, 1 << value );
		   save_planet( planet );
		ch_printf( ch, "Flag set: %s\n\r", arg3 );
	   }
	}
        return;
    }


    do_setplanet( ch, "" );


    return;
}

void do_showplanet( CHAR_DATA *ch, char *argument )
{
    GUARD_DATA * guard;
    PLANET_DATA *planet;
    char buf[MAX_STRING_LENGTH];
    int num_guards = 0;
    int pf = 0;
    int pc = 0;
    int pw = 0;

    if ( IS_NPC( ch ) )
    {
	send_to_char( "Huh?\n\r", ch );
	return;
    }

    if ( argument[0] == '\0' )
    {
	send_to_char( "Usage: showsystem <system>\n\r", ch );
	return;
    }

    planet = get_planet( argument );
    if ( !planet )
    {
	send_to_char( "> no such system\n\r", ch );
	return;
    }

    if( !IS_IMMORTAL( ch ) && ( IS_SET( planet->flags, PLANET_HIDDEN ) || IS_SET( planet->flags, PLANET_EXPLORABLE) ) )
    {
	send_to_char( "> no such system\n\r", ch );
	return;
    }

    if( planet->governed_by )
	sprintf( buf, "%s", planet->governed_by->name );
    else
	sprintf( buf, "[neutral]" );


    for ( guard = planet->first_guard ; guard ; guard = guard->next_on_planet )
        num_guards++;

    if ( planet->size > 0 )
    {
       float tempf;

       tempf = planet->citysize;
       pc = tempf / planet->size *  100;

       tempf = planet->wilderness;
       pw = tempf / planet->size *  100;

       tempf = planet->farmland;
       pf = tempf / planet->size *  100;
    }

    ch_printf( ch, "&W%s\n\r", planet->name);
    if ( IS_IMMORTAL(ch) )
          ch_printf( ch, "&WFilename: &G%s\n\r", planet->filename);

    ch_printf( ch, "&Wterrain: &G%s\n\r",
                   sector_name[planet->sector]  );
    ch_printf( ch, "&Wowner: &G%s\n\r",
                   planet->governed_by ? planet->governed_by->name : "" );
    ch_printf( ch, "&Wsize: &G%d\n\r",
                   planet->size );
    ch_printf( ch, "&Wterminals: &G%d\n\r", pc ) ;
    ch_printf( ch, "&Wdatabase: &G%d\n\r", pw ) ;
    ch_printf( ch, "&Wsubservers: &G%d\n\r", pf ) ;
    ch_printf( ch, "&Wfirewalls: &G%d\n\r", planet->barracks );
    //ch_printf( ch, "&Wtowers: &G%d\n\r", planet->controls );
    ch_printf( ch, "&Wblack ICE: &G%d&W/%d\n\r", num_guards , planet->barracks*5 );
    ch_printf( ch, "&Wtotal ICE: &G%d&W/%d\n\r", planet->population , max_population( planet ) );
    ch_printf( ch, "&Wcpu: &G%.2f\n\r",
                   planet->pop_support );
    ch_printf( ch, "&Wmonthly revenue: &G%ld\n\r",
                   get_taxes( planet) );

    if ( IS_IMMORTAL(ch) )
    {
      if( IS_SET( planet->flags, PLANET_HIDDEN ) )
	ch_printf( ch, "&Wsystem is hidden\n\r" );
      if( IS_SET( planet->flags, PLANET_EXPLORABLE ) )
	ch_printf( ch, "&system is not yet explored\n\r" );
    }
    if( IS_SET( planet->flags, PLANET_NOCAP ) )
	ch_printf( ch, "&WPlanet is not capturable\n\r" );
    if( IS_SET( planet->flags, PLANET_NOPEDIT ) )
	ch_printf( ch, "&WPlanet is not editable by players\n\r" );
    if( IS_SET( planet->flags, PLANET_NOINVADE ) )
  	ch_printf( ch, "&WPlanet is not invadable\n\r" );
    if( IS_SET( planet->flags, PLANET_NOTARGET ) )
  	ch_printf( ch, "&WPlanet is not targetable\n\r" );

    if ( IS_IMMORTAL(ch) && !planet->area )
    {
          ch_printf( ch, "&RWarning - this planet is not attached to an area!&G");
          ch_printf( ch, "\n\r" );
    }

    return;
}


void do_makeplanet( CHAR_DATA *ch, char *argument )
{
    AREA_DATA *pArea;
    PLANET_DATA *planet;
    char arg1[MAX_STRING_LENGTH];
    char arg3[MAX_STRING_LENGTH];
    char buf[MAX_STRING_LENGTH];
    char filename[MAX_STRING_LENGTH];
    char pname[MAX_STRING_LENGTH];
    char * description = NULL;
    bool destok = TRUE;
    int rnum, sector, px, py, pz;
    ROOM_INDEX_DATA *location;
    ROOM_INDEX_DATA *troom;
    EXIT_DATA * xit;
    SPACE_DATA *starsystem;

    switch( ch->substate )
    {
	default:
	  break;
	case SUB_ROOM_DESC:
	  pArea = (AREA_DATA *) ch->dest_buf;
	  if ( !pArea )
	  {
		bug( "makep: sub_room_desc: NULL ch->dest_buf", 0 );
		destok = FALSE;
	  }
	  planet = (PLANET_DATA *) ch->dest_buf_2;
	  if ( !planet )
	  {
		bug( "make: sub_room_desc: NULL ch->dest_buf2", 0 );
		destok = FALSE;
	  }
	  description = copy_buffer( ch );
	  stop_editing( ch );
	  ch->substate = ch->tempnum;
	  if ( !destok )
	     return;

         for ( rnum = 1 ; rnum <= 26 ; rnum ++ )
         {
      	     location = make_room( ++top_r_vnum );
      	     planet->size++;

	     if ( !location )
	     {
	        bug( "makep: make_room failed", 0 );
	        return;
	     }

	     location->area = pArea;
	     STRFREE( location->description );
	     STRFREE( location->name );

         if ( rnum == 12 )
	     {
	        location->name = STRALLOC( "supply" );
                strcpy( buf , "this is where you can buy and sell.\n\r" );
	        location->description = STRALLOC(buf);
             	location->sector_type = SECT_INSIDE;
		SET_BIT( location->room_flags , ROOM_INDOORS );
		SET_BIT( location->room_flags , ROOM_NO_MOB );
		SET_BIT( location->room_flags , ROOM_SAFE );
		SET_BIT( location->room_flags , ROOM_NOPEDIT );
		SET_BIT( location->room_flags , ROOM_MAIL );
		SET_BIT( location->room_flags , ROOM_TRADE );
		SET_BIT( location->room_flags , ROOM_BANK );
	        planet->citysize++;
	     }
	     else if ( rnum == 13 )
	     {
	        strcpy( buf , "lobby" );
	        location->name = STRALLOC( buf );
                strcpy( buf , "the system's lobby node.\n\r" );
	        location->description = STRALLOC(buf);
             	location->sector_type = SECT_INSIDE;
		SET_BIT( location->room_flags , ROOM_INDOORS );
		SET_BIT( location->room_flags , ROOM_NO_MOB );
		SET_BIT( location->room_flags , ROOM_SAFE );
		SET_BIT( location->room_flags , ROOM_NOPEDIT );
		SET_BIT( location->room_flags , ROOM_INFO );
	        planet->citysize++;
	     }
	     else if ( rnum == 14 )
	     {
	        location->name = STRALLOC( "io" );
                strcpy( buf , "this is where you connect or disconnect.\n\r" );
	        location->description = STRALLOC(buf);
             	location->sector_type = SECT_CITY;
		SET_BIT( location->room_flags , ROOM_SHIPYARD );
		SET_BIT( location->room_flags , ROOM_CAN_LAND );
		SET_BIT( location->room_flags , ROOM_NO_MOB );
		SET_BIT( location->room_flags , ROOM_NOPEDIT );
	        planet->citysize++;
	     }
	     else if ( rnum == 18 )
	     {
	        //strcpy( buf , planet->name );
	        strcpy( buf , "agent" );
	        location->name = STRALLOC( buf );
                strcpy( buf , "you can get missions here.\n\r" );
	        location->description = STRALLOC(buf);
             	location->sector_type = SECT_INSIDE;
		SET_BIT( location->room_flags , ROOM_INDOORS );
		SET_BIT( location->room_flags , ROOM_HOTEL );
		SET_BIT( location->room_flags , ROOM_SAFE );
		SET_BIT( location->room_flags , ROOM_NO_MOB );
		SET_BIT( location->room_flags , ROOM_NOPEDIT );
	        planet->citysize++;
	     }
	     else if ( rnum == 26 )
	     {
	        //strcpy( buf , planet->name );
	        strcpy( buf , "employment" );
            location->name = STRALLOC( buf );
            strcpy( buf , "get jobs here.\n\r" );
	        location->description = STRALLOC(buf);
            location->sector_type = SECT_INSIDE;
		SET_BIT( location->room_flags , ROOM_EMPLOYMENT );
		SET_BIT( location->room_flags , ROOM_NOPEDIT );
		SET_BIT( location->room_flags , ROOM_NO_MOB );
	        planet->citysize++;
	     }
	     else
	     {
	     	location->description = STRALLOC(description);
	     	//location->name = STRALLOC( planet->name );
		location->name = STRALLOC( "database" );
             	location->sector_type = planet->sector;
             	planet->wilderness++;
             }

	     LINK( location , pArea->first_room , pArea->last_room , next_in_area , prev_in_area );

             if ( rnum > 5 && rnum != 23 && rnum != 17 && rnum != 19
                  && rnum != 12 && rnum != 14 && rnum != 26)
             {
                 troom = get_room_index( top_r_vnum - 5 );
                 xit = make_exit( location, troom, 0 );
	         xit->keyword		= STRALLOC( "" );
	         xit->description		= STRALLOC( "" );
	         xit->key			= -1;
	         xit->exit_info		= 0;
                 xit = make_exit( troom, location, 2 );
	         xit->keyword		= STRALLOC( "" );
	         xit->description	= STRALLOC( "" );
	         xit->key		= -1;
	         xit->exit_info		= 0;
	     }
             if ( rnum != 1 && rnum != 6 && rnum != 11 && rnum != 16
                  && rnum != 21 && rnum != 12 && rnum != 15
                  && rnum != 18 && rnum != 19 && rnum != 26)
             {
                 troom = get_room_index( top_r_vnum - 1 );
                 xit = make_exit( location, troom, 3 );
	         xit->keyword		= STRALLOC( "" );
	         xit->description		= STRALLOC( "" );
	         xit->key			= -1;
	         xit->exit_info		= 0;
                 xit = make_exit( troom, location, 1 );
	         xit->keyword		= STRALLOC( "" );
	         xit->description	= STRALLOC( "" );
	         xit->key		= -1;
	         xit->exit_info		= 0;
	     }

             if ( rnum == 26 )
             {
                 troom = get_room_index( top_r_vnum - 13 );
                 xit = make_exit( location, troom, DIR_UP );
	         xit->keyword		= STRALLOC( "" );
	         xit->description		= STRALLOC( "" );
	         xit->key			= -1;
	         xit->exit_info		= 0;
                 xit = make_exit( troom, location, DIR_DOWN );
	         xit->keyword		= STRALLOC( "" );
	         xit->description	= STRALLOC( "" );
	         xit->key		= -1;
	         xit->exit_info		= 0;
	     }

         }

         planet->flags = 0;

         save_planet( planet );
         fold_area( pArea , pArea->filename , FALSE );
         write_area_list();
         write_planet_list();
         sprintf( buf , "%d" , top_r_vnum - 17 );
	 //do_goto( ch , buf );
	 reset_all();

	 return;
    }

    if ( IS_NPC(ch) || !ch->pcdata )
       return;

    if (!ch->in_room || !IS_SET( ch->in_room->room_flags, ROOM_SHIPYARD ) )
    {
    	send_to_char( "> exploration code can only be launched from an io port\n\r", ch );
	return;
    }

    if ( ch->gold < 100000 )
    {
    	send_to_char( "> it costs 100000 credits to launch an exploration code\n\r", ch );
	return;
    }

    if ( !argument || argument[0] == '\0' )
    {
	send_to_char( "> would you like to explore an existing region or a new one\n\r\n\r", ch );
	send_to_char( "> syntax: explore <region name> <system name>\n\r", ch );
	send_to_char( "> note: the first word in the system's name MUST be unique\n\r", ch );
	return;
    }

    argument = one_argument( argument , arg1 );

	/*
	Parse name added for Neuromancer
	*/

	if( !check_parse_pname( arg1 ) )
    {
	send_to_char("> illegal region name - please try another one\n\r", ch );
	return;
    }

    starsystem = starsystem_from_name(arg1);

    if( !IS_IMMORTAL(ch) )
    {
         PLANET_DATA *tp;
		 int pcount = 0;

		if( !ch->pcdata->clan )
		{
		   send_to_char("> you must be in an organization to do that\n\r", ch );
		   return;
		}


    if ( starsystem )
    {

         if ( starsystem == starsystem_from_name( NEWBIE_STARSYSTEM ) )
         {
        	ch_printf( ch, "> you cannot explore in that region\n\r", tp->governed_by->name );
        	return;
         }

         for ( tp = starsystem->first_planet ; tp ; tp = tp->next_in_system )
           if ( tp->governed_by &&
           ( !ch->pcdata->clan || ch->pcdata->clan != tp->governed_by ) )

           {
        	ch_printf( ch, "> you cannot explore in that region without permission from %s\n\r", tp->governed_by->name );
        	return;
           }

      for( tp = first_planet; tp; tp = tp->next )
         if( tp->starsystem == starsystem )
	   pcount++;

  	  if( pcount >= MAX_PLANET_SYSTEM )
	  {
		send_to_char("> there are too many systems in that region\n\r", ch );
	 	return;
	  }
	  pcount = 0;
         }
      for( tp = first_planet; tp; tp = tp->next )
         if( tp->governed_by && tp->governed_by == ch->pcdata->clan )
	   pcount++;

      if( pcount >= MAX_PLANET_CLAN )


           {
    	  send_to_char("> your clan cannot explore any more systems\n\r", ch );
        	return;
           }

    }

    if ( !argument || argument[0] == '\0' )
    {
	send_to_char( "> what would you call the new system if you found it\n\r\n\r", ch );
	send_to_char( "> syntax: explore <region name> <system name>\n\r", ch );
	send_to_char( "> note: the first word in the system's name MUST be original\n\r", ch );
	return;
    }

    sector = 0;
    while ( sector == 0 || sector == SECT_WATER_SWIM
    || sector == SECT_WATER_NOSWIM || sector == SECT_UNDERWATER
    || sector == SECT_FARMLAND )
        sector = number_range( SECT_FIELD , SECT_MAX-1 );

    strcpy( pname , argument );
    argument = one_argument( argument , arg3 );

    if( !check_parse_pname( pname ) )
    {
	send_to_char( "> illegal system name - please try another\n\r", ch );
	return;
    }

    for ( pArea = first_area; pArea; pArea = pArea->next )
    {
	if ( !str_cmp( pArea->filename, arg3 ) )
	{
	  send_to_char( "> first word in the planets name MUST be original\n\r", ch );
	  return;
        }
    }

    strcpy ( buf , strlower(arg3) );
    strcat ( buf , ".planet" );

    for ( planet = first_planet ; planet; planet = planet->next )
    {
	if ( !str_cmp( planet->filename, buf ) )
	{
	  send_to_char( "> a system with that filename already exists\n\r", ch );
	  send_to_char( "> the first word in the system's name must be original\n\r", ch );
	  return;
        }
    }


    ch->gold -= 100000;

    send_to_char( "> you spend 100000 credits to launch an explore code\n\r", ch );
    echo_to_room( AT_WHITE , ch->in_room, "> a small probe lifts off into space" );

//    if (  number_percent() < 20 )
//	return;

    sector = 18;

/*
    while ( sector == 0 || sector == 0 || sector == SECT_WATER_SWIM
    || sector == SECT_WATER_NOSWIM || sector == SECT_UNDERWATER
    || sector == SECT_FARMLAND )
        sector = number_range( SECT_FIELD , SECT_MAX-1 );
*/

    pArea = NULL;
    planet = NULL;
    px = number_range ( 1 , 21 );
    py = number_range ( 1 , 13 );
    pz = number_range ( -10 , 10 ) * 1000;
    CREATE( planet, PLANET_DATA, 1 );
    LINK( planet, first_planet, last_planet, next, prev );
    planet->governed_by = NULL;
    planet->next_in_system = NULL;
    planet->prev_in_system = NULL;
    planet->first_guard = NULL;
    planet->last_guard = NULL;
    planet->name		= STRALLOC( pname );
    planet->sector = sector;
    planet->size = 0;
    planet->citysize = 0;
    planet->wilderness = 0;
    planet->farmland = 0;
    planet->barracks = 0;
    planet->controls = 0;
    planet->pop_support = 0;
    planet->x      = px;
    planet->y      = py;
    planet->z      = pz;
    planet->governed_by = ch->pcdata->clan;

    if ( starsystem )
    {
           planet->starsystem = starsystem;
           starsystem = planet->starsystem;
           LINK(planet, starsystem->first_planet, starsystem->last_planet, next_in_system, prev_in_system);
    }
    else
    {
        CREATE( starsystem, SPACE_DATA, 1 );
        LINK( starsystem, first_starsystem, last_starsystem, next, prev );
        starsystem->name		= STRALLOC( arg1 );
        starsystem->star1            = STRALLOC( arg1 );
        starsystem->star2            = STRALLOC( "" );
        starsystem->first_planet = planet;
        starsystem->last_planet = planet;
        starsystem->first_ship = NULL;
        starsystem->last_ship = NULL;
        starsystem->first_missile = NULL;
        starsystem->last_missile = NULL;
        sprintf( filename, "%s.system" , strlower(arg1) );
        starsystem->filename = str_dup( filename );
        save_starsystem( starsystem );
        write_starsystem_list();
        planet->starsystem = starsystem;
    }

    CREATE( pArea, AREA_DATA, 1 );
    pArea->first_room	= NULL;
    pArea->last_room	= NULL;
    pArea->planet       = NULL;
    pArea->planet       = planet;
    pArea->name	        = STRALLOC( pname );

    planet->area = pArea;

    LINK( pArea, first_area, last_area, next, prev );
    top_area++;

    pArea->filename	= str_dup( arg3 );
    sprintf( filename, "%s.planet" , strlower(arg3) );
    planet->filename = str_dup( filename );

    send_to_char( "\n\r&Y> your code has discovered a new system\n\r", ch );
    send_to_char( "> the terrain: &W", ch );
    send_to_char( sector_name[sector], ch );
    send_to_char( "&Y\n\r", ch );
    send_to_char( "\n\r> please enter a description for your system\n\r", ch );
    //send_to_char( "> It should be a short paragraph of 5 or more lines\n\r", ch );
    send_to_char( "> this will be used as the system's default node descriptions\n\r\n\r", ch );

    description = STRALLOC( "" );

/* save them now just in case... */

    save_planet( planet );
    fold_area( pArea , pArea->filename , FALSE );
    write_area_list();
    write_planet_list();

	if ( ch->substate == SUB_REPEATCMD )
	  ch->tempnum = SUB_REPEATCMD;
	else
	  ch->tempnum = SUB_NONE;
	ch->substate = SUB_ROOM_DESC;
	ch->dest_buf = (void *) pArea;
	ch->dest_buf_2 = (void *) planet;
	start_editing( ch, description );
	return;

}

void do_planets( CHAR_DATA *ch, char *argument )
{
    PLANET_DATA *planet;
    int count = 0;
    SPACE_DATA *starsystem;

    set_char_color( AT_WHITE, ch );
    send_to_char( "system          region         owner\n\r" , ch );

    for ( starsystem = first_starsystem ; starsystem; starsystem = starsystem->next )

        if( starsystem->hidden != SPACE_HIDDEN )

    	for ( planet = starsystem->first_planet; planet; planet = planet->next_in_system )
     {
		 	if( !IS_SET( planet->flags, PLANET_HIDDEN ) && !IS_SET( planet->flags, PLANET_EXPLORABLE ) )
	{
        ch_printf( ch, "&G%-15s %-13s  %s    \n\r",
                   planet->name , starsystem->name ,
                   planet->governed_by ? planet->governed_by->name : "" );
        //ch_printf( ch, "%.1f\n\r", planet->pop_support );
        if ( IS_IMMORTAL(ch) && !planet->area )
        {
          ch_printf( ch, "&R> warning - this system is not attached to region!&G");
          ch_printf( ch, "\n\r" );
        }

        count++;
     }
     }

    for ( planet = first_planet; planet; planet = planet->next )

    	if( !IS_SET( planet->flags, PLANET_HIDDEN ) && !IS_SET( planet->flags, PLANET_EXPLORABLE ) )

    {
        if ( planet->starsystem )
           continue;

        ch_printf( ch, "&G%-15s %-12s  %-25s    \n\r",
                   planet->name , "",
                   planet->governed_by ? planet->governed_by->name : "" );
        //ch_printf( ch, "%.1f\n\r", planet->pop_support );
        if ( IS_IMMORTAL(ch) && !planet->area )
        {
          ch_printf( ch, "&RWarning - this planet is not attached to an area!&G");
          ch_printf( ch, "\n\r" );
        }

        count++;
    }

    if( IS_IMMORTAL(ch) )
    {
      send_to_char("\n\r========== Hidden/Unexplored ===========\n\r", ch );
      for ( planet = first_planet; planet; planet = planet->next )
      {
        if( IS_SET( planet->flags, PLANET_EXPLORABLE ) )
	   ch_printf( ch, "&R%s\n\r", planet->name );
        if( IS_SET( planet->flags, PLANET_HIDDEN ) || planet->starsystem->hidden == SPACE_HIDDEN )
	   ch_printf( ch, "&B%s\n\r", planet->name );
      }
	  send_to_char("\n\r========================================\n\r", ch );
	send_to_char("RED = Unexplored, BLUE = Hidden\n\r", ch );
    }


    if ( !count )
    {
	set_char_color( AT_BLOOD, ch);
        send_to_char( "> there are no systems\n\r", ch );
    }
    send_to_char( "&W> use SHOWSYSTEM for more information\n\r", ch );

}

void do_capture ( CHAR_DATA *ch , char *argument )
{
   CLAN_DATA *clan;
   PLANET_DATA *planet;
   PLANET_DATA *cPlanet;
   float support = 0.0;
   int pCount = 0;
   char buf[MAX_STRING_LENGTH];

   if ( !ch->in_room || !ch->in_room->area)
      return;

   if ( IS_NPC(ch) || !ch->pcdata )
   {
       send_to_char ( "huh?\n\r" , ch );
       return;
   }

   if ( !ch->pcdata->clan )
   {
       send_to_char ( "> you need to be a member of an organization to do that!\n\r" , ch );
       return;
   }

   clan = ch->pcdata->clan;

   if ( ( planet = ch->in_room->area->planet ) == NULL )
   {
       send_to_char ( "> you must be on a planet to capture it\n\r" , ch );
       return;
   }

   if ( IS_SET( planet->flags, PLANET_NOCAP ) )
   {
       send_to_char( "> you can't capture this planet\n\r", ch );
       return;
   }

   if ( clan == planet->governed_by )
   {
       send_to_char ( "> your organization already controls this planet\n\r" , ch );
       return;
   }

/*
   if ( planet->starsystem )
   {
       SHIP_DATA *ship;
       CLAN_DATA *sClan;

       for ( ship = planet->starsystem->first_ship ; ship ; ship = ship->next_in_starsystem )
       {
          sClan = get_clan(ship->owner);
          if ( !sClan )
             continue;
          if ( sClan == planet->governed_by )
          {
             send_to_char ( "A planet cannot be captured while protected by orbiting spacecraft\n\r" , ch );
             return;
          }
       }
   }
*/

   if ( planet->first_guard )
   {
       send_to_char ( "> this system is protected by ICE\n\r" , ch );
       send_to_char ( "> you will have to eliminate all ICE before you can capture it\n\r" , ch );
       return;
   }

   if ( planet->pop_support > 0 )
   {
       send_to_char ( "> the cpu is still too strong\n\r" , ch );
       return;
   }

   for ( cPlanet = first_planet ; cPlanet ; cPlanet = cPlanet->next )
        if ( clan == cPlanet->governed_by )
        {
            pCount++;
            support += cPlanet->pop_support;
        }

   if ( support < 0 )
   {
       send_to_char ( "> your organization needs more support for the cpu\n\r> improve support in the systems you already own\n\r" , ch );
       return;
   }

   planet->governed_by = clan;
   planet->pop_support = 50;

   sprintf( buf , "> %s has been captured by %s", planet->name, clan->name );
   echo_to_all( AT_RED , buf , 0 );

   save_planet( planet );

   return;
}

long get_taxes( PLANET_DATA *planet )
{
      long gain;
      int resource;
      int bigships;

      if( IS_SET( planet->flags, PLANET_HIDDEN ) || IS_SET( planet->flags, PLANET_EXPLORABLE ) )
          return 0;

      resource = planet->wilderness;
      resource = UMIN( resource , planet->population );

      gain = 500*planet->population;
      gain += 10000*resource;
      gain += planet->pop_support*100;

      gain -= 5000 * planet->barracks;
      gain -= 10000 * planet->controls;

      bigships = planet->controls/5;  /* 100k for destroyers, 1 mil for battleships */
      gain -= 50000 * bigships;

      return gain;
}

int max_population( PLANET_DATA *planet )
{
     int support;
     int pmax;
     int rmax;

     support = (planet->pop_support + 200) / 3;

     pmax = planet->citysize;
     rmax = planet->wilderness/5 + 5*planet->farmland;

     pmax = pmax * support / 100;

     return UMIN( rmax , pmax );
}

void do_hideplanet( CHAR_DATA *ch, char *argument )
{
	PLANET_DATA *planet;

	if( argument[0] == '\0' )
	{
	   send_to_char("> syntax: hidesystem <system>\n\r", ch );
	   return;
	}
	planet = get_planet( argument );
	if ( !planet )
	{
	   send_to_char("> no such planet\n\r", ch );
	   return;
	}
	if ( !IS_SET( planet->flags, PLANET_HIDDEN ) )
	{
	   SET_BIT( planet->flags, PLANET_HIDDEN );
	   send_to_char("Planet is now hidden\n\r", ch );
	}
	else
	{
	   REMOVE_BIT( planet->flags, PLANET_HIDDEN );
	   send_to_char("Planet is now visible\n\r", ch );
	}
	save_planet( planet );

}

//done for Neuro