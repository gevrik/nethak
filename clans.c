#include <sys/stat.h>

#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "mud.h"

extern int top_r_vnum;

#define MAX_NEST	100

CLAN_DATA * first_clan;
CLAN_DATA * last_clan;

/* local routines */
void	fread_clan	args( ( CLAN_DATA *clan, FILE *fp ) );
bool	load_clan_file	args( ( char *clanfile ) );
void	write_clan_list	args( ( void ) );


/*
 * Get pointer to clan structure from clan name.
 */
CLAN_DATA *get_clan( char *name )
{
    CLAN_DATA *clan;

    for ( clan = first_clan; clan; clan = clan->next )
       if ( !str_cmp( name, clan->name ) )
         return clan;

    for ( clan = first_clan; clan; clan = clan->next )
       if ( !str_prefix( name, clan->name ) )
         return clan;

    for ( clan = first_clan; clan; clan = clan->next )
       if ( nifty_is_name( name, clan->name ) )
         return clan;

    for ( clan = first_clan; clan; clan = clan->next )
       if ( nifty_is_name_prefix( name, clan->name ) )
         return clan;

    return NULL;
}

void write_clan_list( )
{
    CLAN_DATA *tclan;
    FILE *fpout;
    char filename[256];

    sprintf( filename, "%s%s", CLAN_DIR, CLAN_LIST );
    fpout = fopen( filename, "w" );
    if ( !fpout )
    {
	bug( "FATAL: cannot open clan.lst for writing\n\r", 0 );
 	return;
    }
    for ( tclan = first_clan; tclan; tclan = tclan->next )
	fprintf( fpout, "%s\n", tclan->filename );
    fprintf( fpout, "$\n" );
    fclose( fpout );
}

/*
 * Save a clan's data to its data file
 */
void save_clan( CLAN_DATA *clan )
{
    FILE *fp;
    char filename[256];
    char buf[MAX_STRING_LENGTH];

    if ( !clan )
    {
	bug( "save_clan: null clan pointer", 0 );
	return;
    }

    if ( !clan->filename || clan->filename[0] == '\0' )
    {
	sprintf( buf, "save_clan: %s has no filename", clan->name );
	bug( buf, 0 );
	return;
    }

    sprintf( filename, "%s%s", CLAN_DIR, clan->filename );

    fclose( fpReserve );
    if ( ( fp = fopen( filename, "w" ) ) == NULL )
    {
    	bug( "save_clan: fopen", 0 );
    	perror( filename );
    }
    else
    {
	fprintf( fp, "#CLAN\n" );
	fprintf( fp, "Name         %s~\n",	clan->name		);
	fprintf( fp, "Filename     %s~\n",	clan->filename		);
	fprintf( fp, "Description  %s~\n",	clan->description	);
	fprintf( fp, "Leaders      %s~\n",	clan->leaders		);
	fprintf( fp, "Atwar        %s~\n",	clan->atwar		);
	fprintf( fp, "Members      %d\n",	clan->members		);
	fprintf( fp, "Funds        %ld\n",	clan->funds		);
	fprintf( fp, "Salary       %d\n",	clan->salary		);
	fprintf( fp, "Enlist       %d\n", 	clan->enlistroom );
	fprintf( fp, "Allowenlist  %d\n", 	clan->allowenlist );
	fprintf( fp, "End\n\n"						);
	fprintf( fp, "#END\n"						);

	fclose( fp );
	fp = NULL;
    }

    fpReserve = fopen( NULL_FILE, "r" );
    return;
}

/*
 * Read in actual clan data.
 */

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

void fread_clan( CLAN_DATA *clan, FILE *fp )
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
		KEY( "Allowenlist",	clan->allowenlist,	fread_number( fp ) );
	    KEY( "Atwar",	clan->atwar,	fread_string( fp ) );
	    break;

	case 'D':
	    KEY( "Description",	clan->description,	fread_string( fp ) );
	    break;

	case 'E':
	    if ( !str_cmp( word, "End" ) )
	    {
		if (!clan->name)
		  clan->name		= STRALLOC( "" );
		if (!clan->leaders)
		  clan->leaders		= STRALLOC( "" );
		if (!clan->atwar)
		  clan->atwar		= STRALLOC( "" );
		if (!clan->description)
		  clan->description 	= STRALLOC( "" );
		if (!clan->tmpstr)
		  clan->tmpstr		= STRALLOC( "" );
		return;
	    }
	    KEY( "Enlist",	clan->enlistroom,		fread_number( fp ) );
	    break;

	case 'F':
	    KEY( "Funds",	clan->funds,		fread_number( fp ) );
	    KEY( "Filename",	clan->filename,		fread_string_nohash( fp ) );
	    break;

	case 'L':
	    KEY( "Leaders",	clan->leaders,		fread_string( fp ) );
	    break;

	case 'M':
	    KEY( "Members",	clan->members,		fread_number( fp ) );
	    break;

	case 'N':
	    KEY( "Name",	clan->name,		fread_string( fp ) );
	    break;

	case 'S':
	    KEY( "Salary",	clan->salary,		fread_number( fp ) );
	    break;

	}

	if ( !fMatch )
	{
	    sprintf( buf, "Fread_clan: no match: %s", word );
	    bug( buf, 0 );
	}

    }
}

/*
 * Load a clan file
 */

bool load_clan_file( char *clanfile )
{
    char filename[256];
    CLAN_DATA *clan;
    FILE *fp;
    bool found;

    CREATE( clan, CLAN_DATA, 1 );

    found = FALSE;
    sprintf( filename, "%s%s", CLAN_DIR, clanfile );

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
		bug( "Load_clan_file: # not found", 0 );
		break;
	    }

	    word = fread_word( fp );
	    if ( !str_cmp( word, "CLAN"	) )
	    {
	    	fread_clan( clan, fp );
	    	break;
	    }
	    else
	    if ( !str_cmp( word, "END"	) )
	        break;
	    else
	    {
		char buf[MAX_STRING_LENGTH];

		sprintf( buf, "Load_clan_file: bad section: %s", word );
		bug( buf, 0 );
		break;
	    }
	}
	fclose( fp );
    }

    if ( found )
    {
	LINK( clan, first_clan, last_clan, next, prev );
        return found;
    }
    else
      DISPOSE( clan );

    return found;
}


/*
 * Load in all the clan files.
 */
void load_clans( )
{
    FILE *fpList;
    char *filename;
    char clanlist[256];
    char buf[MAX_STRING_LENGTH];

    first_clan	= NULL;
    last_clan	= NULL;

    log_string( "Loading clans..." );

    sprintf( clanlist, "%s%s", CLAN_DIR, CLAN_LIST );
    fclose( fpReserve );
    if ( ( fpList = fopen( clanlist, "r" ) ) == NULL )
    {
	perror( clanlist );
	exit( 1 );
    }

    for ( ; ; )
    {
	filename = feof( fpList ) ? "$" : fread_word( fpList );
	log_string( filename );
	if ( filename[0] == '$' )
	  break;

	if ( !load_clan_file( filename ) )
	{
	  sprintf( buf, "Cannot load clan file: %s", filename );
	  bug( buf, 0 );
	}
    }
    fclose( fpList );
    log_string(" Done clans\n\r" );
    fpReserve = fopen( NULL_FILE, "r" );

    return;
}

void do_make( CHAR_DATA *ch, char *argument )
{
	send_to_char( "> invalid command\n\r", ch );
	return;
}

void do_induct( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    CLAN_DATA *clan;

    if ( IS_NPC( ch ) || !ch->pcdata->clan )
    {
	send_to_char( "> invalid command\n\r", ch );
	return;
    }

    clan = ch->pcdata->clan;

    if ( (ch->pcdata && ch->pcdata->bestowments
    &&    is_name("induct", ch->pcdata->bestowments))
    ||   nifty_is_name( ch->name, clan->leaders  ))
	;
    else
    {
	send_to_char( "> invalid command\n\r", ch );
	return;
    }

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "> induct whom\n\r", ch );
	return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
	send_to_char( "> that user is not here\n\r", ch);
	return;
    }

    if ( IS_NPC(victim) )
    {
	send_to_char( "> not on ICE\n\r", ch );
	return;
    }

    if ( victim->pcdata->clan )
    {
	if ( victim->pcdata->clan == clan )
	  send_to_char( "> this user already belongs to your organization\n\r", ch );
	else
	  send_to_char( "> this user already belongs to an organization\n\r", ch );
	return;
    }

    clan->members++;

    victim->pcdata->clan = clan;
    STRFREE(victim->pcdata->clan_name);
    victim->pcdata->clan_name = QUICKLINK( clan->name );
    act( AT_MAGIC, "> you induct $N into $t", ch, clan->name, victim, TO_CHAR );
    act( AT_MAGIC, "> $n inducts $N into $t", ch, clan->name, victim, TO_NOTVICT );
    act( AT_MAGIC, "> $n inducts you into $t", ch, clan->name, victim, TO_VICT );
    victim->pcdata->bestowments = str_dup( "build" );

    sprintf(buf, "> %s has joined %s", victim->name, clan->name);
    echo_to_all(AT_MAGIC, buf, ECHOTAR_ALL);

    save_char_obj( victim );
    return;
}

void do_outcast( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    CLAN_DATA *clan;
    char buf[MAX_STRING_LENGTH];

    if ( IS_NPC( ch ) || !ch->pcdata->clan )
    {
	send_to_char( "> invalid command\n\r", ch );
	return;
    }

    clan = ch->pcdata->clan;

    if ( (ch->pcdata && ch->pcdata->bestowments
    &&    is_name("outcast", ch->pcdata->bestowments))
    ||   nifty_is_name( ch->name, clan->leaders  ))
	;
    else
    {
	send_to_char( "> invalid command\n\r", ch );
	return;
    }


    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "> outcast whom\n\r", ch );
	return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
	send_to_char( "> that player is not here\n\r", ch);
	return;
    }

    if ( IS_NPC(victim) )
    {
	send_to_char( "> not on ICE\n\r", ch );
	return;
    }

    if ( victim == ch )
    {
	    send_to_char( "> confirm your kick out of your own clan\n\r", ch );
	    return;
    }

    if ( victim->pcdata->clan != ch->pcdata->clan )
    {
	    send_to_char( "> this player does not belong to your clan\n\r", ch );
	    return;
    }

    if ( nifty_is_name( victim->name, victim->pcdata->clan->leaders ) )
    {
       char tc[MAX_STRING_LENGTH];
       char on[MAX_STRING_LENGTH];
       char * leadership = victim->pcdata->clan->leaders;

       strcpy ( tc , "" );

       while ( leadership[0] != '\0' )
       {
          leadership = one_argument( leadership , on );
          if ( str_cmp ( victim->name, on )
          && (strlen(on) + strlen(tc)) < (MAX_STRING_LENGTH+1) )
          {
            if ( strlen(tc) != 0 )
                strcat ( tc , " " );
            strcat ( tc , on );
          }
       }

       STRFREE( victim->pcdata->clan->leaders );
       victim->pcdata->clan->leaders = STRALLOC( tc );
    }


    --clan->members;

    if( clan->members < 0 )
       clan->members = 0;

    victim->pcdata->clan = NULL;
    STRFREE(victim->pcdata->clan_name);
    victim->pcdata->clan_name = STRALLOC( "" );
    act( AT_MAGIC, "> you outcast $N from $t", ch, clan->name, victim, TO_CHAR );
    act( AT_MAGIC, "> $n outcasts $N from $t", ch, clan->name, victim, TO_ROOM );
    act( AT_MAGIC, "> $n outcasts you from $t", ch, clan->name, victim, TO_VICT );
    sprintf(buf, "> %s has been outcast from %s", victim->name, clan->name);
    echo_to_all(AT_MAGIC, buf, ECHOTAR_ALL);

    DISPOSE( victim->pcdata->bestowments );
    victim->pcdata->bestowments = str_dup("");

    save_char_obj( victim );	/* clan gets saved when pfile is saved */
    return;
}

void do_setclan( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    CLAN_DATA *clan;

    if ( IS_NPC(ch) || !ch->pcdata )
    	return;

    if ( !ch->desc )
	return;

    switch( ch->substate )
    {
	default:
	  break;
	case SUB_CLANDESC:
	  clan = ch->dest_buf;
	  if ( !clan )
	  {
		bug( "setclan: sub_clandesc: NULL ch->dest_buf", 0 );
		stop_editing( ch );
     	        ch->substate = ch->tempnum;
		send_to_char( "&RError: clan lost\n\r" , ch );
		return;
	  }
	  STRFREE( clan->description );
	  clan->description = copy_buffer( ch );
	  stop_editing( ch );
	  ch->substate = ch->tempnum;
          save_clan( clan );
	  return;
    }

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' )
    {
	send_to_char( "Usage: setclan <clan> <field> <player>\n\r", ch );
	send_to_char( "\n\rField being one of:\n\r", ch );
	send_to_char( "members funds\n\r", ch );
	send_to_char( "leaders name filename desc atwar enlistroom\n\r", ch );
	return;
    }

    clan = get_clan( arg1 );
    if ( !clan )
    {
	send_to_char( "No such clan\n\r", ch );
	return;
    }

    if ( !strcmp( arg2, "members" ) )
    {
	clan->members = atoi( argument );
	send_to_char( "Done\n\r", ch );
	save_clan( clan );
	return;
    }

    if ( !strcmp( arg2, "wages" ) )
    {
	clan->salary = atoi( argument );
	send_to_char( "Done\n\r", ch );
	save_clan( clan );
	return;
    }

    if ( !strcmp( arg2, "funds" ) )
    {
	clan->funds = atoi( argument );
	send_to_char( "Done\n\r", ch );
	save_clan( clan );
	return;
    }

	if ( !strcmp( arg2, "enlistroom" ) )
	{
		clan->enlistroom = atoi( argument );
		send_to_char( "Done\n\r", ch );
		save_clan( clan );
		return;
	}

    if ( !strcmp( arg2, "leaders" ) )
    {
	STRFREE( clan->leaders );
	clan->leaders = STRALLOC( argument );
	send_to_char( "Done\n\r", ch );
	save_clan( clan );
	return;
    }

    if ( !strcmp( arg2, "atwar" ) )
    {
	STRFREE( clan->atwar );
	clan->atwar = STRALLOC( argument );
	send_to_char( "Done\n\r", ch );
	save_clan( clan );
	return;
    }


    if ( !strcmp( arg2, "name" ) )
    {
	STRFREE( clan->name );
	clan->name = STRALLOC( argument );
	send_to_char( "Done\n\r", ch );
	save_clan( clan );
	return;
    }

    if ( !strcmp( arg2, "filename" ) )
    {
	DISPOSE( clan->filename );
	clan->filename = str_dup( argument );
	send_to_char( "Done\n\r", ch );
	save_clan( clan );
	write_clan_list( );
	return;
    }

    if ( !str_cmp( arg2, "desc" ) )
    {
        ch->substate = SUB_CLANDESC;
        ch->dest_buf = clan;
        start_editing( ch, clan->description );
	return;
    }

    do_setclan( ch, "" );
    return;
}

void do_showclan( CHAR_DATA *ch, char *argument )
{
    CLAN_DATA *clan;
    PLANET_DATA *planet;
    int pCount = 0;
    int support;
    long revenue;

    if ( IS_NPC( ch ) )
    {
	send_to_char( "> invalid command\n\r", ch );
	return;
    }

    if ( argument[0] == '\0' )
    {
	send_to_char( "> syntax: showorg <clan>\n\r", ch );
	return;
    }

    clan = get_clan( argument );
    if ( !clan )
    {
	send_to_char( "> no such organization\n\r", ch );
	return;
    }

        pCount = 0;
        support = 0;
        revenue = 0;

        for ( planet = first_planet ; planet ; planet = planet->next )
          if ( clan == planet->governed_by )
          {
            support += planet->pop_support;
            pCount++;
            revenue += get_taxes(planet);
          }

        if ( pCount > 1 )
           support /= pCount;


    ch_printf( ch, "&W%s      %s\n\r",
    			clan->name,
    			IS_IMMORTAL(ch) ? clan->filename : "" );
    ch_printf( ch, "&Wdescription:&G\n\r%s\n\r&Wleaders: &G%s\n\r&Wat war: &G%s\n\r",
    			clan->description,
    			clan->leaders,
    			clan->atwar );
    ch_printf( ch, "&Wmembers: &G%d\n\r",
    			clan->members);
//    ch_printf( ch, "&Wdecks: &G%d\n\r",
//    			clan->spacecraft);
//    ch_printf( ch, "&Wboosters: &G%d\n\r",
//    			clan->vehicles);
    ch_printf( ch, "&Wsystems: &G%d\n\r",
    			pCount);
    ch_printf( ch, "&Waverage cpu health: &G%d\n\r",
    			support);
    ch_printf( ch, "&Wmonthly revenue: &G%ld\n\r",
    			revenue);
    ch_printf( ch, "&Whourly wages: &G%d\n\r",
    			clan->salary);
    ch_printf( ch, "&Wtotal funds: &G%ld\n\r",
    			clan->funds );

    if (clan->allowenlist == 1)
    	send_to_char( "&Wenlist: &Gon&w\n\r", ch );
    else
    	send_to_char( "&Wenlist: &Roff&w\n\r", ch );
//    ch_printf( ch, "&Wenlist room: &G%ld\n\r",
//    			clan->enlistroom );

    return;
}

void do_makeclan( CHAR_DATA *ch, char *argument )
{
    char filename[256];
    CLAN_DATA *clan;
    bool found;

    if ( !argument || argument[0] == '\0' )
    {
	send_to_char( "Usage: makeclan <clan name>\n\r", ch );
	return;
    }

    found = FALSE;
    sprintf( filename, "%s%s", CLAN_DIR, strlower(argument) );

    CREATE( clan, CLAN_DATA, 1 );
    LINK( clan, first_clan, last_clan, next, prev );
    clan->name		= STRALLOC( argument );
    clan->description	= STRALLOC( "" );
    clan->leaders	= STRALLOC( "" );
    clan->atwar		= STRALLOC( "" );
    clan->tmpstr	= STRALLOC( "" );
    clan->funds         = 0;
    clan->salary        = 0;
    clan->members       = 0;
}

void do_clans( CHAR_DATA *ch, char *argument )
{
    CLAN_DATA *clan;
    PLANET_DATA *planet;
    int count = 0;
    int pCount, revenue;

    ch_printf( ch, "&Worganization             systems   score\n\r");
    for ( clan = first_clan; clan; clan = clan->next )
    {
        pCount = 0;
        revenue = 0;

        for ( planet = first_planet ; planet ; planet = planet->next )
          if ( clan == planet->governed_by )
          {
            pCount++;
            revenue += get_taxes(planet)/720;
          }

        ch_printf( ch, "&Y%-24s %-3d       %d\n\r",
                  clan->name,  pCount, revenue );
        count++;
    }

    if ( !count )
    {
	set_char_color( AT_BLOOD, ch);
        send_to_char( "> there are no organizations currently formed\n\r", ch );
    }

    set_char_color( AT_WHITE, ch );
    send_to_char( "\n\rfor more information use: SHOWORG\n\r", ch );
    send_to_char( "See also: SYSTEMS\n\r", ch );

}

void do_shove( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    int exit_dir;
    EXIT_DATA *pexit;
    CHAR_DATA *victim;
    bool nogo;
    ROOM_INDEX_DATA *to_room;
    int chance;

    argument = one_argument( argument, arg );
    argument = one_argument( argument, arg2 );


    if ( arg[0] == '\0' )
    {
	send_to_char( "> shove whom\n\r", ch);
	return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
	send_to_char( "> user not found\n\r", ch);
	return;
    }

    if (victim == ch)
    {
	send_to_char("> you shove yourself around - to no avail\n\r", ch);
	return;
    }

    if( get_age( ch ) < 18 )
    {
	send_to_char("> wait until you get older before pushing people around\n\r", ch );
	return;
    }

    if ( (victim->position) != POS_STANDING )
    {
	act( AT_PLAIN, "> $N is not standing up", ch, NULL, victim, TO_CHAR );
	return;
    }

    if ( arg2[0] == '\0' )
    {
	send_to_char( "> shove them in which direction\n\r", ch);
	return;
    }

    exit_dir = get_dir( arg2 );
    if ( IS_SET(victim->in_room->room_flags, ROOM_SAFE)
    &&  get_timer(victim, TIMER_SHOVEDRAG) <= 0)
    {
	send_to_char("> that character cannot be shoved right now\n\r", ch);
	return;
    }
    victim->position = POS_SHOVE;
    nogo = FALSE;
    if ((pexit = get_exit(ch->in_room, exit_dir)) == NULL )
      nogo = TRUE;
    else
    if ( IS_SET(pexit->exit_info, EX_CLOSED)
    && (!IS_AFFECTED(victim, AFF_PASS_DOOR)
    ||   IS_SET(pexit->exit_info, EX_NOPASSDOOR)) )
      nogo = TRUE;
    if ( nogo )
    {
	send_to_char( "> there is no exit in that direction\n\r", ch );
        victim->position = POS_STANDING;
	return;
    }
    to_room = pexit->to_room;

    if ( IS_NPC(victim) )
    {
	send_to_char("> you can only shove player characters\n\r", ch);
	return;
    }

chance = 50;

/* Add 3 points to chance for every str point above 15, subtract for
below 15 */

chance += ((get_curr_str(ch) - 15) * 3);

chance += (ch->top_level - victim->top_level);

/* Debugging purposes - show percentage for testing */

/* sprintf(buf, "Shove percentage of %s = %d", ch->name, chance);
act( AT_ACTION, buf, ch, NULL, NULL, TO_ROOM );
*/

if (chance < number_percent( ))
{
  send_to_char("> you failed\n\r", ch);
  victim->position = POS_STANDING;
  return;
}
    act( AT_ACTION, "> you shove $M", ch, NULL, victim, TO_CHAR );
    act( AT_ACTION, "> $n shoves you", ch, NULL, victim, TO_VICT );
    move_char( victim, get_exit(ch->in_room,exit_dir), 0);
    if ( !char_died(victim) )
      victim->position = POS_STANDING;
    WAIT_STATE(ch, 12);
    /* Remove protection from shove/drag if char shoves -- Blodkai */
    if ( IS_SET(ch->in_room->room_flags, ROOM_SAFE)
    &&   get_timer(ch, TIMER_SHOVEDRAG) <= 0 )
      add_timer( ch, TIMER_SHOVEDRAG, 10, NULL, 0 );
}

void do_drag( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    int exit_dir;
    CHAR_DATA *victim;
    EXIT_DATA *pexit;
    ROOM_INDEX_DATA *to_room;
    bool nogo;
    int chance;

    argument = one_argument( argument, arg );
    argument = one_argument( argument, arg2 );

    if ( arg[0] == '\0' )
    {
	send_to_char( "> drag whom\n\r", ch);
	return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
	send_to_char( "> user not found\n\r", ch);
	return;
    }

    if ( victim == ch )
    {
	send_to_char("> you take yourself by the scruff of your neck but go nowhere\n\r", ch);
	return;
    }

    if ( IS_NPC(victim) )
    {
	send_to_char("> you can only drag player characters\n\r", ch);
	return;
    }

    if ( victim->fighting )
    {
        send_to_char( "> you cannot get close enough\n\r", ch);
        return;
    }

    if ( arg2[0] == '\0' )
    {
	send_to_char( "> drag them in which direction\n\r", ch);
	return;
    }

    exit_dir = get_dir( arg2 );

    if ( IS_SET(victim->in_room->room_flags, ROOM_SAFE)
    &&   get_timer( victim, TIMER_SHOVEDRAG ) <= 0)
    {
	send_to_char("> that character cannot be dragged right now\n\r", ch);
	return;
    }

    nogo = FALSE;
    if ((pexit = get_exit(ch->in_room, exit_dir)) == NULL )
      nogo = TRUE;
    else
    if ( IS_SET(pexit->exit_info, EX_CLOSED)
    && (!IS_AFFECTED(victim, AFF_PASS_DOOR)
    ||   IS_SET(pexit->exit_info, EX_NOPASSDOOR)) )
      nogo = TRUE;
    if ( nogo )
    {
	send_to_char( "> there is no exit in that direction\n\r", ch );
	return;
    }

    to_room = pexit->to_room;

    chance = 50;


/*
sprintf(buf, "Drag percentage of %s = %d", ch->name, chance);
act( AT_ACTION, buf, ch, NULL, NULL, TO_ROOM );
*/
if (chance < number_percent( ))
{
  send_to_char("> you failed\n\r", ch);
  victim->position = POS_STANDING;
  return;
}
    if ( victim->position < POS_STANDING )
    {
	sh_int temp;

	temp = victim->position;
	victim->position = POS_DRAG;
	act( AT_ACTION, "> you drag $M into the next room", ch, NULL, victim, TO_CHAR );
	act( AT_ACTION, "> $n grabs your hair and drags you", ch, NULL, victim, TO_VICT );
	move_char( victim, get_exit(ch->in_room,exit_dir), 0);
	if ( !char_died(victim) )
	  victim->position = temp;
/* Move ch to the room too.. they are doing dragging - Scryn */
	move_char( ch, get_exit(ch->in_room,exit_dir), 0);
	WAIT_STATE(ch, 12);
	return;
    }
    send_to_char("> you cannot do that to someone who is standing\n\r", ch);
    return;
}


void do_resign( CHAR_DATA *ch, char *argument )
{

       	CLAN_DATA *clan;
        char buf[MAX_STRING_LENGTH];

        if ( IS_NPC(ch) || !ch->pcdata )
	{
	    send_to_char( "> invalid command\n\r", ch );
	    return;
	}

        clan =  ch->pcdata->clan;

        if ( clan == NULL )
        {
	    send_to_char( "> you have to join an organization before you can quit it\n\r", ch );
	    return;
	}

       if ( nifty_is_name( ch->name, ch->pcdata->clan->leaders ) )
       {
           ch_printf( ch, "> you cannot resign from %s - you are one of the leaders\n\r", clan->name );
           return;
       }

    --clan->members;
	if( clan->members < 0 )
    clan->members = 0;
    ch->pcdata->clan = NULL;
    STRFREE(ch->pcdata->clan_name);
    ch->pcdata->clan_name = STRALLOC( "" );
    ch->pcdata->qtaxnodes = 0;
    act( AT_MAGIC, "> you resign your position in $t", ch, clan->name, NULL , TO_CHAR );
    sprintf(buf, "> %s has quit %s", ch->name, clan->name);
    echo_to_all(AT_MAGIC, buf, ECHOTAR_ALL);

    DISPOSE( ch->pcdata->bestowments );
    ch->pcdata->bestowments = str_dup("");

    save_char_obj( ch );	/* clan gets saved when pfile is saved */

    return;

}

void do_enlist( CHAR_DATA *ch, char *argument )
{

	CLAN_DATA *clan;
	ROOM_INDEX_DATA * location;
	char buf[MAX_STRING_LENGTH];
	location = ch->in_room;

	if ( IS_NPC(ch) || !ch->pcdata )
	{
	    send_to_char( "> invalid command\n\r", ch );
	    return;
	}

	if ( ch->pcdata->clan )
	{
		send_to_char( "> &Ryou already belong to a clan&w\n\r", ch );
		return;
	}

	if ( ! IS_SET( ch->in_room->room_flags , ROOM_RECRUIT ) )
	{
		send_to_char( "> you do not seem to be in a recruitment office\n\r", ch );
		return;
	}

	clan = ch->in_room->area->planet->governed_by;

//	for ( clan = first_clan; clan; clan = clan->next )
//	{
//		if ( ch->in_room->vnum == clan->enlistroom )
//		{
			//SET_BIT( ch->speaks, LANG_CLAN );

	if (clan->allowenlist < 1)
	{
		send_to_char( "> &Rthey are not recruiting right now&w\n\r", ch );
		return;
	}

			++clan->members;
			STRFREE( ch->pcdata->clan_name );
			ch->pcdata->clan_name = QUICKLINK( clan->name );
			ch->pcdata->clan = clan;
			ch->pcdata->bestowments = str_dup( "build" );	
			ch_printf( ch, "> welcome to %s\n\r", clan->name );

		    sprintf(buf, "> %s has joined %s", ch->name, clan->name);
		    echo_to_all(AT_MAGIC, buf, ECHOTAR_ALL);

		    save_char_obj( ch );

//			nRoom = make_room( ++top_r_vnum );
//   			nRoom->area = ch->in_room->area;
//  			LINK( nRoom , ch->in_room->area->first_room , ch->in_room->area->last_room , next_in_area , prev_in_area );
//   			STRFREE( nRoom->name );
//   			STRFREE( nRoom->description );
//
//			strcpy( buf , ch->name );
//			strcat( buf , "&Y.&Chome" );
//
//   			nRoom->name = STRALLOC( buf );
//   			nRoom->description = STRALLOC ( "this is your home node.\n\ruse ENTER command to leave to system lobby.\n\ruse HOME to come back here.\n\rgo UP to go to your IO node and lobby.\n\r" );
//   			nRoom->owner = STRALLOC( ch->name );
//   			nRoom->sector_type = SECT_DUNNO;
//   			SET_BIT( nRoom->room_flags , ROOM_NO_MOB );
//   			SET_BIT( lRoom->room_flags , ROOM_NOPEDIT );
//			//SET_BIT( nRoom->room_flags , ROOM_CAN_LAND );
//
//   			xit2 = make_exit( nRoom , ch->in_room  , 10 );
//   			xit2->keyword		= STRALLOC( "" );
//   			xit2->description	= STRALLOC( "" );
//   			xit2->key		= -1;
//   			xit2->exit_info	= 133120;
//
//   			ch->in_room->area->planet->size++;
//
//   			//SET_BIT( ch->in_room->area->flags , AFLAG_MODIFIED );
//
//			//send_to_char( "> new player home node created\n\r", ch );
//
//      			nRoom->area->planet->citysize++;
//      			nRoom->sector_type = SECT_INSIDE;
//      			SET_BIT( nRoom->room_flags , ROOM_PLR_HOME );
//      			SET_BIT( nRoom->room_flags , ROOM_NOPEDIT );
//      			//SET_BIT( nRoom->room_flags , ROOM_HOTEL );
//
////lobbyroom
//
//			lRoom = make_room( ++top_r_vnum );
//   			lRoom->area = ch->in_room->area;
//  			LINK( lRoom , ch->in_room->area->first_room , ch->in_room->area->last_room , next_in_area , prev_in_area );
//   			STRFREE( lRoom->name );
//   			STRFREE( lRoom->description );
//
//			strcpy( bufa , ch->name );
//		        strcat( bufa , "&Y.&Clobby" );
//
//   			lRoom->name = STRALLOC( bufa );
//   			lRoom->description = STRALLOC ( "site lobby.\n\ruse ENTER command to leave to system lobby.\n\r" );
//   			lRoom->owner = STRALLOC( ch->name );
//   			lRoom->sector_type = SECT_DUNNO;
//   			SET_BIT( lRoom->room_flags , ROOM_NO_MOB );
//			SET_BIT( lRoom->room_flags , ROOM_CAN_LAND );
//			SET_BIT( lRoom->room_flags , ROOM_NOPEDIT );
//
//   			xit3 = make_exit( lRoom , ch->in_room , 10 );
//   			xit3->keyword		= STRALLOC( "" );
//   			xit3->description	= STRALLOC( "" );
//   			xit3->key		= -1;
//   			xit3->exit_info	= 133120;
//
//   			xit4 = make_exit( lRoom , nRoom , DIR_DOWN );
//   			xit4->keyword		= STRALLOC( "" );
//   			xit4->description	= STRALLOC( "" );
//   			xit4->key		= -1;
//   			xit4->exit_info	= 0;
//
//   			xit5 = make_exit( nRoom , lRoom , DIR_UP );
//   			xit5->keyword		= STRALLOC( "" );
//   			xit5->description	= STRALLOC( "" );
//   			xit5->key		= -1;
//   			xit5->exit_info	= 0;
//
//   			ch->in_room->area->planet->size++;
//
//   			SET_BIT( ch->in_room->area->flags , AFLAG_MODIFIED );
//
//			//send_to_char( "> new home node created\n\r", ch );
//
//      			lRoom->area->planet->citysize++;
//      			lRoom->sector_type = SECT_INSIDE;
//
////endlobbyroom
//
////firstconstroom
//
//			cRoom = make_room( ++top_r_vnum );
//   			cRoom->area = ch->in_room->area;
//  			LINK( cRoom , ch->in_room->area->first_room , ch->in_room->area->last_room , next_in_area , prev_in_area );
//   			STRFREE( cRoom->name );
//   			STRFREE( cRoom->description );
//
//			strcpy( bufb , ch->name );
//		        strcat( bufb , "&Y.&Cdatabase" );
//
//   			cRoom->name = STRALLOC( bufb );
//   			cRoom->description = STRALLOC ( "database node.\n\ruse CONSTRUCT commands to build.\n\r" );
//   			cRoom->owner = STRALLOC( ch->name );
//   			cRoom->sector_type = SECT_DUNNO;
//
//   			xit6 = make_exit( cRoom , lRoom , DIR_SOUTH );
//   			xit6->keyword		= STRALLOC( "" );
//   			xit6->description	= STRALLOC( "" );
//   			xit6->key		= -1;
//   			xit6->exit_info	= 0;
//
//   			xit7 = make_exit( lRoom , cRoom , DIR_NORTH );
//   			xit7->keyword		= STRALLOC( "" );
//   			xit7->description	= STRALLOC( "" );
//   			xit7->key		= -1;
//   			xit7->exit_info	= 0;
//
//   			ch->in_room->area->planet->size++;
//
//   			SET_BIT( ch->in_room->area->flags , AFLAG_MODIFIED );
//
//			send_to_char( "> new home node created\n\r", ch );
//
//      			cRoom->area->planet->wilderness++;
//      			cRoom->sector_type = SECT_GLACIAL;
//
////endfirstconstroom

      			//ch->pcdata->qtaxnodes = ch->pcdata->qtaxnodes + 1;

//     			fold_area( nRoom->area, nRoom->area->filename, FALSE );
//
//     			ch->plr_home = nRoom;
//     			do_save( ch , "" );
//    			char_from_room( ch );
//			char_to_room( ch, nRoom );
//			do_look( ch, "auto" );
			return;
		//}
	//}

	//send_to_char( "> they do not seem to be recruiting right now\n\r", ch );
	//return;

}

void do_clan_withdraw( CHAR_DATA *ch, char *argument )
{
    CLAN_DATA *clan;
    long       amount;

    if ( IS_NPC( ch ) || !ch->pcdata->clan )
    {
	send_to_char( "> you do not seem to belong to an organization to withdraw funds from\n\r", ch );
	return;
    }

    if (!ch->in_room || !IS_SET(ch->in_room->room_flags, ROOM_BANK) )
    {
       send_to_char( "> you must be in a bank to do that\n\r", ch );
       return;
    }

    clan = ch->pcdata->clan;

    if ( (ch->pcdata && ch->pcdata->bestowments
    &&    is_name("withdraw", ch->pcdata->bestowments))
    ||   nifty_is_name( ch->name, ch->pcdata->clan->leaders  ))
	;
    else
    {
   	send_to_char( "&R>your organization has not seen fit to bestow you with that ability" ,ch );
   	return;
    }

    amount = atoi( argument );

    if ( !amount )
    {
	send_to_char( "> how much would you like to withdraw\n\r", ch );
	return;
    }

    if ( amount > clan->funds )
    {
	ch_printf( ch,  "> %s doesn't have that much\n\r", clan->name );
	return;
    }

    if ( amount < 0 )
    {
	ch_printf( ch,  "> invalid amount\n\r" );
	return;
    }

	if ( amount > 10000 )
	{
	ch_printf( ch,  "> you can only withdraw 10,000 credits at a time\n\r");
	return;
	}

	if ( ch->gold > 10000)
	{
	ch_printf( ch,  "> you already have enough credits\n\r");
	return;
	}

    ch_printf( ch,  "> you withdraw %ld credits from %s's funds\n\r", amount, clan->name );

    clan->funds -= amount;
    ch->gold += amount;
    save_char_obj( ch );

}


void do_clan_donate( CHAR_DATA *ch, char *argument )
{
    CLAN_DATA *clan;
    long       amount;

    if ( IS_NPC( ch ) || !ch->pcdata->clan )
    {
	send_to_char( "> you do not seem to belong to an organization to donate to\n\r", ch );
	return;
    }

    if (!ch->in_room || !IS_SET(ch->in_room->room_flags, ROOM_BANK) )
    {
       send_to_char( "> you must be in a bank to do that\n\r", ch );
       return;
    }

    clan = ch->pcdata->clan;

    amount = atoi( argument );

    if ( !amount )
    {
	send_to_char( "> how much would you like to donate\n\r", ch );
	return;
    }

    if ( amount < 0 )
    {
	ch_printf( ch,  "> invalid amount\n\r" );
	return;
    }

    if ( amount > ch->gold )
    {
	send_to_char( "> you do not have that much\n\r", ch );
	return;
    }

    ch_printf( ch,  "> you donate %ld credits to %s's funds\n\r", amount, clan->name );

    clan->funds += amount;
    ch->gold -= amount;
    save_char_obj( ch );

}


void do_appoint ( CHAR_DATA *ch , char *argument )
{

    char buf[MAX_STRING_LENGTH];
    char name[MAX_INPUT_LENGTH];
    char fname[MAX_STRING_LENGTH];
    struct stat fst;

    if ( IS_NPC( ch ) || !ch->pcdata )
      return;

    if ( !ch->pcdata->clan )
    {
	send_to_char( "> invalid command\n\r", ch );
	return;
    }

    if (  !nifty_is_name( ch->name, ch->pcdata->clan->leaders  )  )
    {
	send_to_char( "> only your leaders can do that\n\r", ch );
	return;
    }

    if ( argument[0] == '\0' )
    {
	send_to_char( "> syntax: appoint name\n\r", ch );
	return;
    }

    one_argument( argument, name );

    name[0] = UPPER(name[0]);

    sprintf( fname, "%s%c/%s", PLAYER_DIR, tolower(name[0]),
			name );

    if ( stat( fname, &fst ) == -1 )
    {
	send_to_char( "> user not found\n\r", ch );
	return;
    }

    if ( nifty_is_name( name, ch->pcdata->clan->leaders ))
    {
      send_to_char( "> they are already a leader\r\n", ch );
      return;
    }

    if ( (strlen(name) + strlen(ch->pcdata->clan->leaders)) >= MAX_STRING_LENGTH )
    {
      send_to_char( "> this clan already has too many leaders\r\n", ch);
      return;
    }

    strcpy ( buf , ch->pcdata->clan->leaders );
    strcat( buf , " ");
    strcat( buf , name );

    STRFREE( ch->pcdata->clan->leaders );
    ch->pcdata->clan->leaders = STRALLOC( buf );

    save_clan ( ch->pcdata->clan );

}

void do_demote ( CHAR_DATA *ch , char *argument )
{

/*  disabled  */

    return;

    if ( IS_NPC( ch ) || !ch->pcdata )
      return;

    if ( !ch->pcdata->clan )
    {
	send_to_char( "> invalid command\n\r", ch );
	return;
    }

    if (  !nifty_is_name( ch->name, ch->pcdata->clan->leaders  )  )
    {
	send_to_char( "> only your leaders can do that\n\r", ch );
	return;
    }

    if ( argument[0] == '\0' )
    {
	send_to_char( "> demote who\n\r", ch );
	return;
    }

    save_clan ( ch->pcdata->clan );

}

void do_empower ( CHAR_DATA *ch , char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    CLAN_DATA *clan;
    char buf[MAX_STRING_LENGTH];

    if ( IS_NPC( ch ) || !ch->pcdata->clan )
    {
	send_to_char( "> invalid command\n\r", ch );
	return;
    }

    clan = ch->pcdata->clan;

    if ( (ch->pcdata && ch->pcdata->bestowments
    &&    is_name("empower", ch->pcdata->bestowments))
    || nifty_is_name( ch->name, clan->leaders  ) )
	;
    else
    {
	send_to_char( "> you clan has not seen fit to bestow that ability to you\n\r", ch );
	return;
    }

    argument = one_argument( argument, arg );
    argument = one_argument( argument, arg2 );

    if ( arg[0] == '\0' )
    {
	send_to_char( "> empower whom to do what\n\r", ch );
	return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
	send_to_char( "> that player is not here\n\r", ch);
	return;
    }

    if( (victim->pcdata && victim->pcdata->bestowments
    &&    !is_name(arg2, victim->pcdata->bestowments)) )
      ;
    else
    {
	send_to_char( "> that person is already bestowed with that ability\n\r", ch );
	return;
    }

    if ( IS_NPC(victim) )
    {
	send_to_char( "> not on ICE\n\r", ch );
	return;
    }

    if ( victim == ch )
    {
	    send_to_char( "> invalid input\n\r", ch );
	    return;
    }

    if ( victim->pcdata->clan != ch->pcdata->clan )
    {
	    send_to_char( "> this player does not belong to your clan\n\r", ch );
	    return;
    }

    if (!victim->pcdata->bestowments)
      victim->pcdata->bestowments = str_dup("");

    if ( arg2[0] == '\0' )
        strcpy( arg2 , "HelpMeImConfused" );

    if ( !str_cmp( arg2, "list" ) )
    {
        ch_printf( ch, "> current bestowed commands on %s: %s\n\r",
                      victim->name, victim->pcdata->bestowments );
        return;
    }

    if ( !str_cmp( arg2, "none" ) )
    {
        DISPOSE( victim->pcdata->bestowments );
	victim->pcdata->bestowments = str_dup("");
        ch_printf( ch, "> bestowments removed from %s\n\r", victim->name );
        ch_printf( victim, "> %s has removed your bestowed clan abilities\n\r", ch->name );
        return;
    }
    else if ( !str_cmp( arg2, "orgdecks" ) )
    {
      sprintf( buf, "> %s %s", victim->pcdata->bestowments, arg2 );
      DISPOSE( victim->pcdata->bestowments );
      victim->pcdata->bestowments = str_dup( buf );
      ch_printf( victim, "> %s has given you permission to use organization decks\n\r",
             ch->name );
      send_to_char( "> they can now use organization decks\n\r", ch );
    }
    else if ( !str_cmp( arg2, "withdraw" ) )
    {
      sprintf( buf, "> %s %s", victim->pcdata->bestowments, arg2 );
      DISPOSE( victim->pcdata->bestowments );
      victim->pcdata->bestowments = str_dup( buf );
      ch_printf( victim, "> %s has given you permission to withdraw organization funds\n\r",
             ch->name );
      send_to_char( "> they can now withdraw organization funds\n\r", ch );
    }
    else if ( !str_cmp( arg2, "orgbuydeck" ) )
    {
      sprintf( buf, "> %s %s", victim->pcdata->bestowments, arg2 );
      DISPOSE( victim->pcdata->bestowments );
      victim->pcdata->bestowments = str_dup( buf );
      ch_printf( victim, "> %s has given you permission to buy organization decks\n\r",
             ch->name );
      send_to_char( "> they can now use ORGBUYDECK\n\r", ch );
    }
    else if ( !str_cmp( arg2, "induct" ) )
    {
      sprintf( buf, "> %s %s", victim->pcdata->bestowments, arg2 );
      DISPOSE( victim->pcdata->bestowments );
      victim->pcdata->bestowments = str_dup( buf );
      ch_printf( victim, "> %s has given you permission to induct new members\n\r",
             ch->name );
      send_to_char( "> they can now induct new members\n\r", ch );
    }
    else if ( !str_cmp( arg2, "empower" ) )
    {
      sprintf( buf, "> %s %s", victim->pcdata->bestowments, arg2 );
      DISPOSE( victim->pcdata->bestowments );
      victim->pcdata->bestowments = str_dup( buf );
      ch_printf( victim, "> %s has given you the ability to empower others\n\r",
             ch->name );
      send_to_char( "> they can now empower others\n\r", ch );
    }
    else if ( !str_cmp( arg2, "repos" ) )
    {
      sprintf( buf, "> %s %s", victim->pcdata->bestowments, arg2 );
      DISPOSE( victim->pcdata->bestowments );
      victim->pcdata->bestowments = str_dup( buf );
      ch_printf( victim, "> %s has given you the ability to set repo options\n\r",
             ch->name );
      send_to_char( "> they can now set repo options\n\r", ch );
    }
    else if ( !str_cmp( arg2, "build" ) )
    {
      sprintf( buf, "> %s %s", victim->pcdata->bestowments, arg2 );
      DISPOSE( victim->pcdata->bestowments );
      victim->pcdata->bestowments = str_dup( buf );
      ch_printf( victim, "> %s has given you permission to code and modify systems\n\r",
             ch->name );
      send_to_char( "> they can now modify and code systems\n\r", ch );
    }
    else
    {
      send_to_char( "> possible arguments:\n\r", ch );
      send_to_char(     "withdraw:     ability to withdraw clan funds\n\r", ch );
      send_to_char(     "induct:       ability to induct new members\n\r", ch );
      send_to_char(     "build:        ability to create and edit nodes\n\r", ch );
      send_to_char(     "repos:        ability to set repository options\n\r", ch );
      send_to_char(     "bestow:       ability to bestow other members (use with caution)\n\r", ch );
      send_to_char(     "none:         removes bestowed abilities\n\r", ch );
      send_to_char(     "list:         shows bestowed abilities\n\r", ch );
    }

    save_char_obj( victim );	/* clan gets saved when pfile is saved */
    return;


}

void do_overthrow( CHAR_DATA *ch , char * argument )
{
    if ( IS_NPC( ch ) )
       return;

    if ( !ch->pcdata || !ch->pcdata->clan )
    {
       send_to_char( "> you have to be part of an organization before you can claim leadership\n\r", ch );
       return;
    }

    if ( ch->pcdata->clan->leaders && ch->pcdata->clan->leaders[0] != '\0' )
    {
       send_to_char( "> your organization already has strong leadership\n\r", ch );
       return;
    }

    ch_printf( ch, "> ok - you are now a leader of %s\n\r", ch->pcdata->clan->name );

    STRFREE ( ch->pcdata->clan->leaders );
    ch->pcdata->clan->leaders = STRALLOC ( ch->name );

    save_char_obj( ch );	/* clan gets saved when pfile is saved */

}

void do_war ( CHAR_DATA *ch , char *argument )
{
    CLAN_DATA *wclan;
    CLAN_DATA *clan;
    char buf[MAX_STRING_LENGTH];

    if ( IS_NPC( ch ) || !ch->pcdata || !ch->pcdata->clan )
    {
	send_to_char( "> invalid command\n\r", ch );
	return;
    }

    clan = ch->pcdata->clan;

    if ( ( ch->pcdata->bestowments
    &&    is_name("war", ch->pcdata->bestowments))
    || nifty_is_name( ch->name, clan->leaders  ) )
	;
    else
    {
	send_to_char( "> your organization has not empowered you to declare war\n\r", ch );
	return;
    }


    if ( argument[0] == '\0' )
    {
	send_to_char( "> declare war on who\n\r", ch );
	return;
    }

    if ( ( wclan = get_clan( argument ) ) == NULL )
    {
	send_to_char( "> organization not found\n\r", ch);
	return;
    }

    if ( wclan == clan )
    {
	send_to_char( "> declare war on self\n\r", ch);
	return;
    }

    if ( nifty_is_name( wclan->name , clan->atwar ) )
    {
        CLAN_DATA *tclan;
        strcpy( buf, "" );

        for ( tclan = first_clan ; tclan ; tclan = tclan->next )
            if ( nifty_is_name( tclan->name , clan->atwar ) && tclan != wclan )
            {
                 strcat ( buf, "\n\r " );
                 strcat ( buf, tclan->name );
                 strcat ( buf, " " );
            }

        STRFREE ( clan->atwar );
        clan->atwar = STRALLOC( buf );

        sprintf( buf , "> %s has declared a ceasefire with %s" , clan->name , wclan->name );
        echo_to_all( AT_WHITE , buf , ECHOTAR_ALL );

        save_char_obj( ch );	/* clan gets saved when pfile is saved */

	return;
    }

    strcpy ( buf, clan->atwar );
    strcat ( buf, "\n\r " );
    strcat ( buf, wclan->name );
    strcat ( buf, " " );

    STRFREE ( clan->atwar );
    clan->atwar = STRALLOC( buf );

    sprintf( buf , "> %s has declared war on %s" , clan->name , wclan->name );
    echo_to_all( AT_RED , buf , ECHOTAR_ALL );

    save_char_obj( ch );	/* clan gets saved when pfile is saved */

}

void do_setwages ( CHAR_DATA *ch , char *argument )
{
    CLAN_DATA *clan;

    if ( IS_NPC( ch ) || !ch->pcdata || !ch->pcdata->clan )
    {
	send_to_char( "> invalid command\n\r", ch );
	return;
    }

    clan = ch->pcdata->clan;

    if ( ( ch->pcdata->bestowments
    &&    is_name("payroll", ch->pcdata->bestowments))
    || nifty_is_name( ch->name, clan->leaders  ) )
	;
    else
    {
	send_to_char( "> you organization has not empowered you to set wages\n\r", ch );
	return;
    }


    if ( argument[0] == '\0' )
    {
	send_to_char( "> set organization wages to what\n\r", ch );
	return;
    }

    if( atoi(argument) < 0 )
    {
	send_to_char( "> invalid amount\n\r", ch );
	return;
    }

    clan->salary = atoi( argument );

    ch_printf( ch , "> organization wages set to %d credits per hour\n\r" , clan->salary );

    save_char_obj( ch );	/* clan gets saved when pfile is saved */


}

void do_setenlist ( CHAR_DATA *ch , char *argument )
{
    CLAN_DATA *clan;

    if ( IS_NPC( ch ) || !ch->pcdata || !ch->pcdata->clan )
    {
	send_to_char( "> invalid command\n\r", ch );
	return;
    }

    clan = ch->pcdata->clan;

    if ( nifty_is_name( ch->name, clan->leaders  ) )
	;
    else
    {
	send_to_char( "> &Ronly the leader can set the enlist status&w\n\r", ch );
	return;
    }


    if ( argument[0] == '\0' )
    {
	send_to_char( "> set enlist to 0 (off) or 1 (on)\n\r", ch );
	return;
    }

    if( atoi(argument) < 0 || atoi(argument) > 1 )
    {
	send_to_char( "> invalid setting\n\r", ch );
	return;
    }

    clan->allowenlist = atoi( argument );
	send_to_char( "> &Gdone&w\n\r", ch );

    save_char_obj( ch );	/* clan gets saved when pfile is saved */


}
