#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include "mud.h"

#define RESTORE_INTERVAL 21600

char * const save_flag[] =
{ "death", "kill", "passwd", "drop", "put", "give", "auto", "zap",
"auction", "get", "receive", "idle", "backup", "r13", "r14", "r15", "r16",
"r17", "r18", "r19", "r20", "r21", "r22", "r23", "r24", "r25", "r26", "r27",
"r28", "r29", "r30", "r31" };


/* from comm.c */
bool	write_to_descriptor	args( ( int desc, char *txt, int length ) );

/*
 * Local functions.
 */
ROOM_INDEX_DATA * find_location	args( ( CHAR_DATA *ch, char *arg ) );
void              save_banlist  args( ( void ) );
void              close_area    args( ( AREA_DATA *pArea ) );

int               get_color (char *argument); /* function proto */

/*
 * Global variables.
 */

char reboot_time[50];
time_t new_boot_time_t;
extern struct tm new_boot_struct;
extern OBJ_INDEX_DATA *obj_index_hash[MAX_KEY_HASH];
extern MOB_INDEX_DATA *mob_index_hash[MAX_KEY_HASH];

int get_saveflag( char *name )
{
    int x;

    for ( x = 0; x < sizeof(save_flag) / sizeof(save_flag[0]); x++ )
      if ( !str_cmp( name, save_flag[x] ) )
        return x;
    return -1;
}

void do_wizhelp( CHAR_DATA *ch, char *argument )
{
    CMDTYPE * cmd;
    int col, hash;

    col = 0;
    set_pager_color( AT_PLAIN, ch );
    for ( hash = 0; hash < 126; hash++ )
	for ( cmd = command_hash[hash]; cmd; cmd = cmd->next )
	    if ( cmd->level > 1 &&
	    cmd->level <= get_trust( ch ) )
	    {
		pager_printf( ch, "%-12s", cmd->name );
		if ( ++col % 6 == 0 )
		    send_to_pager( "\n\r", ch );
	    }

    if ( col % 6 != 0 )
	send_to_pager( "\n\r", ch );
    return;
}

void do_restrict( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    sh_int level, hash;
    CMDTYPE *cmd;
    bool found;

    found = FALSE;

    argument = one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
	send_to_char( "Restrict which command?\n\r", ch );
	return;
    }

    argument = one_argument ( argument, arg2 );
    if ( arg2[0] == '\0' )
      level = get_trust( ch );
    else
      level = atoi( arg2 );

    level = UMAX( UMIN( get_trust( ch ), level ), 0 );

    hash = arg[0] % 126;
    for ( cmd = command_hash[hash]; cmd; cmd = cmd->next )
    {
	if ( !str_prefix( arg, cmd->name )
	&&    cmd->level <= get_trust( ch ) )
	{
	    found = TRUE;
	    break;
	}
    }

    if ( found )
    {
    	if ( !str_prefix( arg2, "show" ) )
    	{
    		sprintf(buf, "%s show", cmd->name);
    		do_cedit(ch, buf);
/*    		ch_printf( ch, "%s is at level %d.\n\r", cmd->name, cmd->level );*/
    		return;
    	}
	cmd->level = level;
	ch_printf( ch, "You restrict %s to level %d\n\r",
	   cmd->name, level );
	sprintf( buf, "%s restricting %s to level %d",
	     ch->name, cmd->name, level );
	log_string( buf );
    }
    else
    	send_to_char( "You may not restrict that command.\n\r", ch );

    return;
}

/* 
 * Check if the name prefix uniquely identifies a char descriptor
 */
CHAR_DATA *get_waiting_desc( CHAR_DATA *ch, char *name )
{
  DESCRIPTOR_DATA *d;
  CHAR_DATA       *ret_char;
  static unsigned int number_of_hits;

  number_of_hits = 0;
  for ( d = first_descriptor; d; d = d->next )
    {
    if ( d->character && (!str_prefix( name, d->character->name )) &&
         IS_WAITING_FOR_AUTH(d->character) )
      {
      if ( ++number_of_hits > 1 )
         {
         ch_printf( ch, "%s does not uniquely identify a char.\n\r", name );
         return NULL;
         }
      ret_char = d->character;       /* return current char on exit */
      }
    }
  if ( number_of_hits==1 )
     return ret_char;
  else
     {
     send_to_char( "No one like that waiting for authorization.\n\r", ch );
     return NULL;
     }
}

void do_authorize( CHAR_DATA *ch, char *argument )
{
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  char buf[MAX_STRING_LENGTH];
  CHAR_DATA *victim;
  DESCRIPTOR_DATA *d;

  argument = one_argument( argument, arg1 );
  argument = one_argument( argument, arg2 );

  if ( arg1[0] == '\0' )
     {
     send_to_char( "Usage:  authorize <player> <yes|name|no/deny>\n\r", ch );
     send_to_char( "Pending authorizations:\n\r", ch );
     send_to_char( " Chosen Character Name\n\r", ch );
     send_to_char( "---------------------------------------------\n\r", ch );
     for ( d = first_descriptor; d; d = d->next )
         if ( (victim = d->character) != NULL && IS_WAITING_FOR_AUTH(victim) )
	    ch_printf( ch, " %s@%s new charcter...\n\r",
               victim->name,
               victim->desc->host );
    return;
    }

  victim = get_waiting_desc( ch, arg1 );
  if ( victim == NULL )
     return;

  if ( arg2[0]=='\0' || !str_cmp( arg2,"accept" ) || !str_cmp( arg2,"yes" ))
     {
	victim->pcdata->auth_state = 3;
	REMOVE_BIT(victim->pcdata->flags, PCFLAG_UNAUTHED);
	if ( victim->pcdata->authed_by )
	   STRFREE( victim->pcdata->authed_by );
	victim->pcdata->authed_by = QUICKLINK( ch->name );
	sprintf( buf, "%s authorized %s", ch->name,
					  victim->name );
	to_channel( buf, CHANNEL_MONITOR, "Monitor", ch->top_level );
	ch_printf( ch, "You have authorized %s.\n\r", victim->name);

     /* Below sends a message to player when name is accepted - Brittany   */

        ch_printf( victim,                                            /* B */
        "The MUD Administrators have accepted the name %s.\n\r"       /* B */
        "You are now fully authorized to play.\n\r",victim->name);                               /* B */
     return;
     }
     else if ( !str_cmp( arg2, "no" ) || !str_cmp( arg2, "deny" ) )
     {
	send_to_char( "You have been denied access.\n\r", victim);
	sprintf( buf, "%s denied authorization to %s", ch->name,
						       victim->name );
	to_channel( buf, CHANNEL_MONITOR, "Monitor", ch->top_level );
	ch_printf( ch, "You have denied %s.\n\r", victim->name);
	do_quit(victim, "");
     }

     else if ( !str_cmp( arg2, "name" ) || !str_cmp(arg2, "n" ) )
     {
	sprintf( buf, "%s has denied %s's name", ch->name,
						       victim->name );
	to_channel( buf, CHANNEL_MONITOR, "Monitor", ch->top_level );
        ch_printf (victim,
          "The MUD Administrators have found the name %s "
          "to be unacceptable.\n\r"
          "Use 'name' to change it to something more apropriate.\n\r", victim->name);
	ch_printf( ch, "You requested %s change names.\n\r", victim->name);
	victim->pcdata->auth_state = 2;
	return;
     }

     else
     {
	send_to_char("Invalid argument.\n\r", ch);
	return;
     }
}

void do_bamfin( CHAR_DATA *ch, char *argument )
{
    if ( !IS_NPC(ch) )
    {
	smash_tilde( argument );
	DISPOSE( ch->pcdata->bamfin );
	ch->pcdata->bamfin = str_dup( argument );
	send_to_char( "Ok.\n\r", ch );
    }
    return;
}



void do_bamfout( CHAR_DATA *ch, char *argument )
{
    if ( !IS_NPC(ch) )
    {
	smash_tilde( argument );
	DISPOSE( ch->pcdata->bamfout );
	ch->pcdata->bamfout = str_dup( argument );
	send_to_char( "Ok.\n\r", ch );
    }
    return;
}

void do_rank( CHAR_DATA *ch, char *argument )
{
  if ( IS_NPC(ch) )
    return;

  if ( !argument || argument[0] == '\0' )
  {
    send_to_char( "Usage: rank <string>.\n\r", ch );
    send_to_char( "   or: rank none.\n\r", ch );
    return;
  }

  smash_tilde( argument );
  DISPOSE( ch->pcdata->rank );
  if ( !str_cmp( argument, "none" ) )
    ch->pcdata->rank = str_dup( "" );
  else
    ch->pcdata->rank = str_dup( argument );
  send_to_char( "Ok.\n\r", ch );

  return;
}


void do_retire( CHAR_DATA *ch, char *argument )
{
}

void do_deny( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
	send_to_char( "Deny whom?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( IS_NPC(victim) )
    {
	send_to_char( "Not on NPC's.\n\r", ch );
	return;
    }

    if ( get_trust( victim ) >= get_trust( ch ) )
    {
	send_to_char( "You failed.\n\r", ch );
	return;
    }

    SET_BIT(victim->act, PLR_DENY);
    send_to_char( "You are denied access!\n\r", victim );
    send_to_char( "OK.\n\r", ch );
    do_quit( victim, "" );

    return;
}



void do_disconnect( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    DESCRIPTOR_DATA *d;
    CHAR_DATA *victim;

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
	send_to_char( "Disconnect whom?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( victim->desc == NULL )
    {
    act( AT_PLAIN, "$N doesn't have a descriptor.", ch, NULL, victim, TO_CHAR );
	return;
    }

    if ( get_trust(ch) <= get_trust( victim ) )
    {
	send_to_char( "They might not like that...\n\r", ch );
	return;
    }

    for ( d = first_descriptor; d; d = d->next )
    {
	if ( d == victim->desc )
	{
	    close_socket( d, FALSE );
	    send_to_char( "Ok.\n\r", ch );
	    return;
	}
    }

    bug( "Do_disconnect: *** desc not found ***.", 0 );
    send_to_char( "Descriptor not found!\n\r", ch );
    return;
}

void do_forceclose( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    DESCRIPTOR_DATA *d;
    int desc;

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
	send_to_char( "Usage: forceclose <descriptor#>\n\r", ch );
	return;
    }
    desc = atoi( arg );

    for ( d = first_descriptor; d; d = d->next )
    {
	if ( d->descriptor == desc )
	{
	    if ( d->character && get_trust(d->character) >= get_trust(ch) )
	    {
		send_to_char( "They might not like that...\n\r", ch );
		return;
	    }
	    close_socket( d, FALSE );
	    send_to_char( "Ok.\n\r", ch );
	    return;
	}
    }

    send_to_char( "Not found!\n\r", ch );
    return;
}


void echo_to_all( sh_int AT_COLOR, char *argument, sh_int tar )
{
    DESCRIPTOR_DATA *d;

    if ( !argument || argument[0] == '\0' )
	return;

    for ( d = first_descriptor; d; d = d->next )
    {
        /* Added showing echoes to players who are editing, so they won't
           miss out on important info like upcoming reboots. --Narn */
	if ( d->connected == CON_PLAYING || d->connected == CON_EDITING )
	{
	    /* This one is kinda useless except for switched.. */
	    if ( tar == ECHOTAR_PC && IS_NPC(d->character) )
	      continue;
	    else if ( tar == ECHOTAR_IMM && !IS_IMMORTAL(d->character) )
	      continue;
	    set_char_color( AT_COLOR, d->character );
	    send_to_char( argument, d->character );
	    send_to_char( "\n\r",   d->character );
	}
    }
    return;
}

void echo_to_area( AREA_DATA *area , sh_int AT_COLOR, char *argument, sh_int tar )
{
    DESCRIPTOR_DATA *d;

    if ( !area )
       return;

    if ( !argument || argument[0] == '\0' )
	return;

    for ( d = first_descriptor; d; d = d->next )
    {
	if ( d->connected == CON_PLAYING )
	{
	    CHAR_DATA *ech;

	    if ( ( ech = d->character ) == NULL )
	      continue;
	    if ( tar == ECHOTAR_IMM && !IS_IMMORTAL(ech) )
	      continue;
	    if ( !ech->in_room || !ech->in_room->area || ech->in_room->area != area )
	      continue;
	    set_char_color( AT_COLOR, ech );
	    send_to_char( argument, ech );
	    send_to_char( "\n\r",   ech );
	}
    }
    return;
}

void do_echo( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    sh_int color;
    int target;
    char *parg;

    if ( IS_SET(ch->act, PLR_NO_EMOTE) )
    {
        send_to_char( "> you are noemoted and cannot echo\n\r", ch );
	return;
    }

    if ( argument[0] == '\0' )
    {
	send_to_char( "> echo what\n\r", ch );
	return;
    }

    if ( (color = get_color(argument)) )
      argument = one_argument(argument, arg);
    parg = argument;
    argument = one_argument(argument, arg);
    if ( !str_cmp( arg, "PC" )
    ||   !str_cmp( arg, "player" ) )
      target = ECHOTAR_PC;
    else if ( !str_cmp( arg, "imm" ) )
      target = ECHOTAR_IMM;
    else
    {
      target = ECHOTAR_ALL;
      argument = parg;
    }
    if ( !color && (color = get_color(argument)) )
      argument = one_argument(argument, arg);
    if ( !color )
      color = AT_IMMORT;
    one_argument(argument, arg);
    if ( !str_cmp( arg, "Wintermute" ))
    {
	ch_printf( ch, "> i do not think %s would like that\n\r", arg );
	return;
    }
    echo_to_all ( color, argument, target );
}

void echo_to_room( sh_int AT_COLOR, ROOM_INDEX_DATA *room, char *argument )
{
    CHAR_DATA *vic;

    if ( room == NULL )
    	return;


    for ( vic = room->first_person; vic; vic = vic->next_in_room )
    {
	set_char_color( AT_COLOR, vic );
	send_to_char( argument, vic );
	send_to_char( "\n\r",   vic );
    }
}

void do_recho( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    sh_int color;

    if ( IS_SET(ch->act, PLR_NO_EMOTE) )
    {
        send_to_char( "You are noemoted and can not recho.\n\r", ch );
	return;
    }

    if ( argument[0] == '\0' )
    {
	send_to_char( "Recho what?\n\r", ch );
	return;
    }

    one_argument( argument, arg );
    if ( !str_cmp( arg, "Thoric" )
    ||   !str_cmp( arg, "Dominus" )
    ||   !str_cmp( arg, "Circe" )
    ||   !str_cmp( arg, "Haus" )
    ||   !str_cmp( arg, "Narn" )
    ||   !str_cmp( arg, "Scryn" )
    ||   !str_cmp( arg, "Blodkai" )
    ||   !str_cmp( arg, "Damian" ) )
    {
	ch_printf( ch, "I don't think %s would like that!\n\r", arg );
	return;
    }
    if ( (color = get_color ( argument )) )
       {
       argument = one_argument ( argument, arg );
       echo_to_room ( color, ch->in_room, argument );
       }
    else
       echo_to_room ( AT_IMMORT, ch->in_room, argument );
}


ROOM_INDEX_DATA *find_location( CHAR_DATA *ch, char *arg )
{
    CHAR_DATA *victim;
    OBJ_DATA *obj;

    if ( is_number(arg) )
	return get_room_index( atoi( arg ) );

    if ( ( victim = get_char_world( ch, arg ) ) != NULL )
	return victim->in_room;

    if ( ( obj = get_obj_world( ch, arg ) ) != NULL )
	return obj->in_room;

    return NULL;
}



void do_transfer( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    ROOM_INDEX_DATA *location;
    DESCRIPTOR_DATA *d;
    CHAR_DATA *victim;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' )
    {
	send_to_char( "Transfer whom (and where)?\n\r", ch );
	return;
    }

    if ( !str_cmp( arg1, "all" ) )
    {
	for ( d = first_descriptor; d; d = d->next )
	{
	    if ( d->connected == CON_PLAYING
	    &&   d->character != ch
	    &&   d->character->in_room
	    &&   d->newstate != 2
	    &&   can_see( ch, d->character ) )
	    {
		char buf[MAX_STRING_LENGTH];
		sprintf( buf, "%s %s", d->character->name, arg2 );
		do_transfer( ch, buf );
	    }
	}
	return;
    }

    /*
     * Thanks to Grodyn for the optional location parameter.
     */
    if ( arg2[0] == '\0' )
    {
	location = ch->in_room;
    }
    else
    {
	if ( ( location = find_location( ch, arg2 ) ) == NULL )
	{
	    send_to_char( "No such location.\n\r", ch );
	    return;
	}

	if ( room_is_private( ch, location ) )
	{
	    send_to_char( "That room is private right now.\n\r", ch );
	    return;
	}
    }

    if ( ( victim = get_char_world( ch, arg1 ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if (NOT_AUTHED(victim))
    {
	send_to_char( "They are not authorized yet!\n\r", ch);
	return;
    }

    if ( !victim->in_room )
    {
	send_to_char( "They are in limbo.\n\r", ch );
	return;
    }

    if ( victim->fighting )
	stop_fighting( victim, TRUE );
    act( AT_MAGIC, "$n disappears in a cloud of swirling colors.", victim, NULL, NULL, TO_ROOM );
    victim->retran = victim->in_room->vnum;
    char_from_room( victim );
    char_to_room( victim, location );
    act( AT_MAGIC, "$n arrives from a puff of smoke.", victim, NULL, NULL, TO_ROOM );
    if ( ch != victim )
    act( AT_IMMORT, "$n has transferred you.", ch, NULL, victim, TO_VICT );
    do_look( victim, "auto" );
    send_to_char( "Ok.\n\r", ch );
}

void do_retran( CHAR_DATA *ch, char *argument )
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	char buf[MAX_STRING_LENGTH];

	argument = one_argument( argument, arg );
	if ( arg[0] == '\0' )
	{
		send_to_char("Retransfer whom?\n\r", ch );
		return;
	}
	if ( !(victim = get_char_world(ch, arg)) )
	{
		send_to_char("They aren't here.\n\r", ch );
		return;
	}
	sprintf(buf, "'%s' %d", victim->name, victim->retran);
	do_transfer(ch, buf);
	return;
}

void do_regoto( CHAR_DATA *ch, char *argument )
{
	char buf[MAX_STRING_LENGTH];

	sprintf(buf, "%d", ch->regoto);
	do_goto(ch, buf);
	return;
}

void do_at( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    ROOM_INDEX_DATA *location;
    ROOM_INDEX_DATA *original;
    CHAR_DATA *wch;

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' || argument[0] == '\0' )
    {
	send_to_char( "At where what?\n\r", ch );
	return;
    }

    if ( ( location = find_location( ch, arg ) ) == NULL )
    {
	send_to_char( "No such location.\n\r", ch );
	return;
    }

    if ( room_is_private( ch, location ) )
    {
      if ( !IS_IMMORTAL( ch )  )
      {
	send_to_char( "That room is private right now.\n\r", ch );
	return;
      }
      else
      {
	send_to_char( "Overriding private flag!\n\r", ch );
      }

    }

    original = ch->in_room;
    char_from_room( ch );
    char_to_room( ch, location );
    interpret( ch, argument );

    /*
     * See if 'ch' still exists before continuing!
     * Handles 'at XXXX quit' case.
     */
    for ( wch = first_char; wch; wch = wch->next )
    {
	if ( wch == ch )
	{
	    char_from_room( ch );
	    char_to_room( ch, original );
	    break;
	}
    }

    return;
}

void do_rat( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    ROOM_INDEX_DATA *location;
    ROOM_INDEX_DATA *original;
    int Start, End, vnum;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' || arg2[0] == '\0' || argument[0] == '\0' )
    {
	send_to_char( "Syntax: rat <start> <end> <command>\n\r", ch );
	return;
    }

    Start = atoi( arg1 );	End = atoi( arg2 );

    if ( Start < 1 || End < Start || Start > End || Start == End || End > 32767 )
    {
	send_to_char( "Invalid range.\n\r", ch );
	return;
    }

    if ( !str_cmp( argument, "quit" ) )
    {
	send_to_char( "I don't think so!\n\r", ch );
	return;
    }

    original = ch->in_room;
    for ( vnum = Start; vnum <= End; vnum++ )
    {
	if ( (location = get_room_index(vnum)) == NULL )
	  continue;
	char_from_room( ch );
	char_to_room( ch, location );
	interpret( ch, argument );
    }

    char_from_room( ch );
    char_to_room( ch, original );
    send_to_char( "Done.\n\r", ch );
    return;
}


void do_rstat( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    ROOM_INDEX_DATA *location;
    OBJ_DATA *obj;
    CHAR_DATA *rch;
    EXIT_DATA *pexit;
    int cnt;
    static char *dir_text[] = { "n", "e", "s", "w", "u", "d", "ne", "nw", "se", "sw", "?" };

    one_argument( argument, arg );

    if ( !str_cmp( arg, "exits" ) )
    {
	location = ch->in_room;

	ch_printf( ch, "Exits for room '%s.' vnum %ld\n\r",
		location->name,
		location->vnum );

	for ( cnt = 0, pexit = location->first_exit; pexit; pexit = pexit->next )
	    ch_printf( ch,
		"%2d) %2s to %-5d.  Key: %d  Flags: %d  Keywords: '%s'.\n\rDescription: %sExit links back to vnum: %ld  Exit's RoomVnum: %ld  Distance: %d\n\r",
		++cnt,
		dir_text[pexit->vdir],
		pexit->to_room ? pexit->to_room->vnum : 0,
		pexit->key,
		pexit->exit_info,
		pexit->keyword,
		pexit->description[0] != '\0'
		    ? pexit->description : "(none).\n\r",
		pexit->rexit ? pexit->rexit->vnum : 0,
		pexit->rvnum,
		pexit->distance );
	return;
    }
    location = ( arg[0] == '\0' ) ? ch->in_room : find_location( ch, arg );
    if ( !location )
    {
	send_to_char( "No such location.\n\r", ch );
	return;
    }

    if ( ch->in_room != location && room_is_private( ch, location ) )
    {
      if ( !IS_IMMORTAL( ch ) )
      {
        send_to_char( "That room is private right now.\n\r", ch );
        return;
      }
      else
      {
        send_to_char( "Overriding private flag!\n\r", ch );
      }

    }

    ch_printf( ch, "Name: %s.\n\rArea: %s  Filename: %s.\n\r",
	location->name,
	location->area ? location->area->name : "None????",
	location->area ? location->area->filename : "None????" );

    ch_printf( ch,
	"Vnum: %ld.  Sector: %d.  Light: %d.  TeleDelay: %d.  TeleVnum: %ld  Tunnel: %d Seccode: %d\n\r",
	location->vnum,
	location->sector_type,
	location->light,
	location->tele_delay,
	location->tele_vnum,
	location->tunnel,
	location->seccode );

    ch_printf( ch,
	"Level: %d.  Owner: %s.\n\r",
	location->level,
	location->owner );

    ch_printf( ch, "Room flags: %s\n\r",
	flag_string(location->room_flags, r_flags) );
    ch_printf( ch, "&GRoom flags2: &W%s\n\r",
    flag_string(location->room_flags2, r_flags2) );

    ch_printf( ch, "Description:\n\r%s", location->description );

    if ( location->first_extradesc )
    {
	EXTRA_DESCR_DATA *ed;

	send_to_char( "Extra description keywords: '", ch );
	for ( ed = location->first_extradesc; ed; ed = ed->next )
	{
	    send_to_char( ed->keyword, ch );
	    if ( ed->next )
		send_to_char( " ", ch );
	}
	send_to_char( "'.\n\r", ch );
    }

    send_to_char( "Characters:", ch );
    for ( rch = location->first_person; rch; rch = rch->next_in_room )
    {
	if ( can_see( ch, rch ) )
	{
	  send_to_char( " ", ch );
	  one_argument( rch->name, buf );
	  send_to_char( buf, ch );
	}
    }

    send_to_char( ".\n\rObjects:   ", ch );
    for ( obj = location->first_content; obj; obj = obj->next_content )
    {
	send_to_char( " ", ch );
	one_argument( obj->name, buf );
	send_to_char( buf, ch );
    }
    send_to_char( ".\n\r", ch );

    if ( location->first_exit )
	send_to_char( "------------------- EXITS -------------------\n\r", ch );
    for ( cnt = 0, pexit = location->first_exit; pexit; pexit = pexit->next )
	ch_printf( ch,
	  "%2d) %-2s to %-5ld.  Key: %d  Flags: %d  Keywords: %s.\n\r",
		++cnt,
		dir_text[pexit->vdir],
		pexit->to_room ? pexit->to_room->vnum : 0,
		pexit->key,
		pexit->exit_info,
		pexit->keyword[0] != '\0' ? pexit->keyword : "(none)" );
    return;
}



void do_ostat( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    AFFECT_DATA *paf;
    OBJ_DATA *obj;
    char *pdesc;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Ostat what?\n\r", ch );
	return;
    }
    if ( arg[0] != '\'' && arg[0] != '"' && strlen(argument) > strlen(arg) )
	strcpy( arg, argument );

    if ( ( obj = get_obj_world( ch, arg ) ) == NULL )
    {
	send_to_char( "Nothing like that in hell, earth, or heaven.\n\r", ch );
	return;
    }

    ch_printf( ch, "Name: %s.\n\r",
	obj->name );

    pdesc=get_extra_descr(arg, obj->first_extradesc);
    if ( !pdesc )
       pdesc=get_extra_descr(arg, obj->pIndexData->first_extradesc);
    if ( !pdesc )
       pdesc = get_extra_descr( obj->name, obj->first_extradesc );
    if ( !pdesc )
       pdesc = get_extra_descr( obj->name, obj->pIndexData->first_extradesc );
    if ( pdesc )
       send_to_char( pdesc, ch );


    ch_printf( ch, "Vnum: %ld.  Type: %s.  Count: %d  Gcount: %d\n\r",
	obj->pIndexData->vnum, item_type_name( obj ), obj->pIndexData->count,
	obj->count );

    ch_printf( ch, "Serial#: %d  TopIdxSerial#: %d  TopSerial#: %d\n\r",
	obj->serial, obj->pIndexData->serial, cur_obj_serial );

    ch_printf( ch, "Short description: %s.\n\rLong description: %s\n\r",
	obj->short_descr, obj->description );

    if ( obj->action_desc[0] != '\0' )
	ch_printf( ch, "Action description: %s.\n\r", obj->action_desc );

    ch_printf( ch, "Wear flags : %s\n\r", flag_string(obj->wear_flags, w_flags) );
    ch_printf( ch, "Extra flags: %s\n\r", flag_string(obj->extra_flags, o_flags) );

    ch_printf( ch, "Number: %d/%d.  Weight: %d/%d.  Layers: %d\n\r",
	1,           get_obj_number( obj ),
	obj->weight, get_obj_weight( obj ), obj->pIndexData->layers );

    ch_printf( ch, "Cost: %d.  Timer: %d.  Level: %d.\n\r",
	obj->cost, obj->timer, obj->level );

    ch_printf( ch,
	"In room: %ld.  In object: %s.  Carried by: %s.  Wear_loc: %d.\n\r",
	obj->in_room    == NULL    ?        0 : obj->in_room->vnum,
	obj->in_obj     == NULL    ? "(none)" : obj->in_obj->short_descr,
	obj->carried_by == NULL    ? "(none)" : obj->carried_by->name,
	obj->wear_loc );

    ch_printf( ch, "Index Values : %d %d %d %d %d %d.\n\r",
	obj->pIndexData->value[0], obj->pIndexData->value[1],
	obj->pIndexData->value[2], obj->pIndexData->value[3],
	obj->pIndexData->value[4], obj->pIndexData->value[5] );
    ch_printf( ch, "Object Values: %d %d %d %d %d %d.\n\r",
	obj->value[0], obj->value[1], obj->value[2], obj->value[3], obj->value[4], obj->value[5] );

    if ( obj->pIndexData->first_extradesc )
    {
	EXTRA_DESCR_DATA *ed;

	send_to_char( "Primary description keywords:   '", ch );
	for ( ed = obj->pIndexData->first_extradesc; ed; ed = ed->next )
	{
	    send_to_char( ed->keyword, ch );
	    if ( ed->next )
		send_to_char( " ", ch );
	}
	send_to_char( "'.\n\r", ch );
    }
    if ( obj->first_extradesc )
    {
	EXTRA_DESCR_DATA *ed;

	send_to_char( "Secondary description keywords: '", ch );
	for ( ed = obj->first_extradesc; ed; ed = ed->next )
	{
	    send_to_char( ed->keyword, ch );
	    if ( ed->next )
		send_to_char( " ", ch );
	}
	send_to_char( "'.\n\r", ch );
    }

    for ( paf = obj->first_affect; paf; paf = paf->next )
	ch_printf( ch, "Affects %s by %d. (extra)\n\r",
	    affect_loc_name( paf->location ), paf->modifier );

    for ( paf = obj->pIndexData->first_affect; paf; paf = paf->next )
	ch_printf( ch, "Affects %s by %d.\n\r",
	    affect_loc_name( paf->location ), paf->modifier );

    return;
}



void do_mstat( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    AFFECT_DATA *paf;
    CHAR_DATA *victim;
    SKILLTYPE *skill;

    set_char_color( AT_PLAIN, ch );

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
	send_to_char( "Mstat whom?\n\r", ch );
	return;
    }
    if ( arg[0] != '\'' && arg[0] != '"' && strlen(argument) > strlen(arg) )
	strcpy( arg, argument );

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }
    if ( get_trust( ch ) < get_trust( victim ) && !IS_NPC(victim) )
    {
	set_char_color( AT_IMMORT, ch );
	send_to_char( "Their godly glow prevents you from getting a good look.\n\r", ch );
	return;
    }
    ch_printf( ch, "Name: %s     Organization: %s\n\r",
	victim->name,
	( IS_NPC( victim ) || !victim->pcdata->clan ) ? "(none)"
			       : victim->pcdata->clan->name );
    if( IS_IMMORTAL(ch) && !IS_NPC(victim) && victim->desc )
	ch_printf( ch, "User: %s@%s   Descriptor: %d   Trust: %d   AuthedBy: %s\n\r",
		victim->desc->user, victim->desc->host, victim->desc->descriptor,
		victim->trust, victim->pcdata->authed_by[0] != '\0'
		? victim->pcdata->authed_by : "(unknown)" );
    if ( !IS_NPC(victim) && victim->pcdata->release_date != 0 )
      ch_printf(ch, "Helled until %24.24s by %s.\n\r",
              ctime(&victim->pcdata->release_date),
              victim->pcdata->helled_by);

    ch_printf( ch, "Vnum: %ld   Sex: %s   Room: %ld   Count: %d\n\r",
	IS_NPC(victim) ? victim->pIndexData->vnum : 0,
	victim->sex == SEX_MALE    ? "male"   :
	victim->sex == SEX_FEMALE  ? "female" : "neutral",
	victim->in_room == NULL    ?        0 : victim->in_room->vnum,
	IS_NPC(victim) ? victim->pIndexData->count : 1 );
    ch_printf( ch, "Str: %d  Int: %d  Wis: %d  Dex: %d  Con: %d  Cha: %d  Lck: %d  Frc: %d\n\r",
	get_curr_str(victim),
	get_curr_int(victim),
	get_curr_wis(victim),
	get_curr_dex(victim),
	get_curr_con(victim),
	get_curr_cha(victim),
	get_curr_lck(victim),
	get_curr_frc(victim) );
    ch_printf( ch, "Hps: %d/%d  Force: %d/%d   Move: %d/%d\n\r",
        victim->hit,         victim->max_hit,
        victim->mana,        victim->max_mana,
        victim->move,        victim->max_move );

ch_printf( ch,
	"Top Level: %d     Align: %d  AC: %d  Gold: %d\n\r",
	victim->top_level,   victim->alignment,
	GET_AC(victim),      victim->gold );
    ch_printf( ch, "Hitroll: %d   Damroll: %d   Position: %d   Wimpy: %d \n\r",
	GET_HITROLL(victim), GET_DAMROLL(victim),
	victim->position,    victim->wimpy );
    ch_printf( ch, "Fighting: %s    Master: %s    Leader: %s\n\r",
	victim->fighting ? victim->fighting->who->name : "(none)",
        victim->master      ? victim->master->name   : "(none)",
        victim->leader      ? victim->leader->name   : "(none)" );
    if ( !IS_NPC(victim) )
	ch_printf( ch,
	    "Thirst: %d   Full: %d   Drunk: %d     Glory: %d/%d\n\rbank: %ld\n\r",
	    victim->pcdata->condition[COND_THIRST],
	    victim->pcdata->condition[COND_FULL],
	    victim->pcdata->condition[COND_DRUNK],
	    victim->pcdata->quest_curr,
	    victim->pcdata->quest_accum,
	    victim->pcdata->bank);
    else
	ch_printf( ch, "Hit dice: %dd%d+%d.  Damage dice: %dd%d+%d.\n\r",
		victim->pIndexData->hitnodice,
		victim->pIndexData->hitsizedice,
		victim->pIndexData->hitplus,
		victim->pIndexData->damnodice,
		victim->pIndexData->damsizedice,
		victim->pIndexData->damplus );
    ch_printf( ch, "MentalState: %d   EmotionalState: %d\n\r",
 		victim->mental_state, victim->emotional_state );
    ch_printf( ch, "Saving throws: %d %d %d %d %d.\n\r",
		victim->saving_poison_death,
		victim->saving_wand,
		victim->saving_para_petri,
		victim->saving_breath,
		victim->saving_spell_staff  );
    ch_printf( ch, "Carry figures: items (%d/%d)  weight (%d/%d)   Numattacks: %d\n\r",
	victim->carry_number, can_carry_n(victim), victim->carry_weight, can_carry_w(victim), victim->numattacks );
    ch_printf( ch, "Years: %d   Seconds Played: %d   Timer: %d   Act: %d\n\r",
	get_age( victim ), (int) victim->played, victim->timer, victim->act );
    if ( IS_NPC( victim ) )
    {
	ch_printf( ch, "Act flags: %s\n\r", flag_string(victim->act, act_flags) );
    }
    else
    {
	ch_printf( ch, "Player flags: %s\n\r",
		flag_string(victim->act, plr_flags) );
	ch_printf( ch, "Pcflags: %s\n\r",
		flag_string(victim->pcdata->flags, pc_flags) );
    }
    ch_printf( ch, "Affected by: %s\n\r",
	affect_bit_name( victim->affected_by ) );
    send_to_char( "\n\r", ch );
    if ( victim->pcdata && victim->pcdata->bestowments
         && victim->pcdata->bestowments[0] != '\0' )
      ch_printf( ch, "Bestowments: %s\n\r", victim->pcdata->bestowments );
    ch_printf( ch, "Short description: %s\n\rLong  description: %s",
	victim->short_descr,
	victim->long_descr[0] != '\0' ? victim->long_descr : "(none)\n\r" );
    if ( IS_NPC(victim) && ( victim->spec_fun || victim->spec_2 ) )
	ch_printf( ch, "Mobile has spec fun: %s %s\n\r",
		lookup_spec( victim->spec_fun ),
		victim->spec_2 ? lookup_spec( victim->spec_2 ) : "" );
    ch_printf( ch, "Body Parts : %s\n\r",
	flag_string(victim->xflags, part_flags) );
    ch_printf( ch, "Resistant  : %s\n\r",
	flag_string(victim->resistant, ris_flags) );
    ch_printf( ch, "Immune     : %s\n\r",
	flag_string(victim->immune, ris_flags) );
    ch_printf( ch, "Susceptible: %s\n\r",
	flag_string(victim->susceptible, ris_flags) );
    ch_printf( ch, "Attacks    : %s\n\r",
	flag_string(victim->attacks, attack_flags) );
    ch_printf( ch, "Defenses   : %s\n\r",
	flag_string(victim->defenses, defense_flags) );
    for ( paf = victim->first_affect; paf; paf = paf->next )
	if ( (skill=get_skilltype(paf->type)) != NULL )
	  ch_printf( ch,
	    "%s: '%s' modifies %s by %d for %d rounds with bits %s.\n\r",
	    skill_tname[skill->type],
	    skill->name,
	    affect_loc_name( paf->location ),
	    paf->modifier,
	    paf->duration,
	    affect_bit_name( paf->bitvector )
	    );
    return;
}



void do_mfind( CHAR_DATA *ch, char *argument )
{
/*  extern int top_mob_index; */
    char arg[MAX_INPUT_LENGTH];
    MOB_INDEX_DATA *pMobIndex;
/*  int vnum; */
    int hash;
    int nMatch;
    bool fAll;

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
	send_to_char( "Mfind whom?\n\r", ch );
	return;
    }

    fAll	= !str_cmp( arg, "all" );
    nMatch	= 0;
    set_pager_color( AT_PLAIN, ch );

    /*
     * Yeah, so iterating over all vnum's takes 10,000 loops.
     * Get_mob_index is fast, and I don't feel like threading another link.
     * Do you?
     * -- Furey
     */
/*  for ( vnum = 0; nMatch < top_mob_index; vnum++ )
    {
	if ( ( pMobIndex = get_mob_index( vnum ) ) != NULL )
	{
	    if ( fAll || is_name( arg, pMobIndex->player_name ) )
	    {
		nMatch++;
		sprintf( buf, "[%5d] %s\n\r",
		    pMobIndex->vnum, capitalize( pMobIndex->short_descr ) );
		send_to_char( buf, ch );
	    }
	}
    }
     */

    /*
     * This goes through all the hash entry points (1024), and is therefore
     * much faster, though you won't get your vnums in order... oh well. :)
     *
     * Tests show that Furey's method will usually loop 32,000 times, calling
     * get_mob_index()... which loops itself, an average of 1-2 times...
     * So theoretically, the above routine may loop well over 40,000 times,
     * and my routine bellow will loop for as many index_mobiles are on
     * your mud... likely under 3000 times.
     * -Thoric
     */
    for ( hash = 0; hash < MAX_KEY_HASH; hash++ )
	for ( pMobIndex = mob_index_hash[hash];
	      pMobIndex;
	      pMobIndex = pMobIndex->next )
	    if ( fAll || nifty_is_name( arg, pMobIndex->player_name ) )
	    {
		nMatch++;
		pager_printf( ch, "[%5d] %s\n\r",
		    pMobIndex->vnum, pMobIndex->short_descr );
	    }

    if ( nMatch )
	pager_printf( ch, "Number of matches: %d\n", nMatch );
    else
	send_to_char( "Nothing like that in hell, earth, or heaven.\n\r", ch );

    return;
}



void do_ofind( CHAR_DATA *ch, char *argument )
{
/*  extern int top_obj_index; */
    char arg[MAX_INPUT_LENGTH];
    OBJ_INDEX_DATA *pObjIndex;
/*  int vnum; */
    int hash;
    int nMatch;
    bool fAll;

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
	send_to_char( "Ofind what?\n\r", ch );
	return;
    }

    set_pager_color( AT_PLAIN, ch );
    fAll	= !str_cmp( arg, "all" );
    nMatch	= 0;
/*  nLoop	= 0; */

    /*
     * Yeah, so iterating over all vnum's takes 10,000 loops.
     * Get_obj_index is fast, and I don't feel like threading another link.
     * Do you?
     * -- Furey
    for ( vnum = 0; nMatch < top_obj_index; vnum++ )
    {
	nLoop++;
	if ( ( pObjIndex = get_obj_index( vnum ) ) != NULL )
	{
	    if ( fAll || nifty_is_name( arg, pObjIndex->name ) )
	    {
		nMatch++;
		sprintf( buf, "[%5d] %s\n\r",
		    pObjIndex->vnum, capitalize( pObjIndex->short_descr ) );
		send_to_char( buf, ch );
	    }
	}
    }
     */

    /*
     * This goes through all the hash entry points (1024), and is therefore
     * much faster, though you won't get your vnums in order... oh well. :)
     *
     * Tests show that Furey's method will usually loop 32,000 times, calling
     * get_obj_index()... which loops itself, an average of 2-3 times...
     * So theoretically, the above routine may loop well over 50,000 times,
     * and my routine bellow will loop for as many index_objects are on
     * your mud... likely under 3000 times.
     * -Thoric
     */
    for ( hash = 0; hash < MAX_KEY_HASH; hash++ )
	for ( pObjIndex = obj_index_hash[hash];
	      pObjIndex;
	      pObjIndex = pObjIndex->next )
	    if ( fAll || nifty_is_name( arg, pObjIndex->name ) )
	    {
		nMatch++;
		pager_printf( ch, "[%5d] %s\n\r",
		    pObjIndex->vnum, pObjIndex->short_descr );
	    }

    if ( nMatch )
	pager_printf( ch, "Number of matches: %d\n", nMatch );
    else
	send_to_char( "Nothing like that in hell, earth, or heaven.\n\r", ch );

    return;
}



void do_mwhere( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    bool found;

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
	send_to_char( "Mwhere whom?\n\r", ch );
	return;
    }

    set_pager_color( AT_PLAIN, ch );
    found = FALSE;
    for ( victim = first_char; victim; victim = victim->next )
    {
	if ( IS_NPC(victim)
	&&   victim->in_room
	&&   nifty_is_name( arg, victim->name ) )
	{
	    found = TRUE;
	    pager_printf( ch, "[%5d] %-28s [%5d] %s\n\r",
		victim->pIndexData->vnum,
		victim->short_descr,
		victim->in_room->vnum,
		victim->in_room->name );
	}
    }

    if ( !found )
	act( AT_PLAIN, "You didn't find any $T.", ch, NULL, arg, TO_CHAR );

    return;
}


void do_bodybag( CHAR_DATA *ch, char *argument )
{
    char buf2[MAX_STRING_LENGTH];
    char buf3[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    bool found;

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
	send_to_char( "Bodybag whom?\n\r", ch );
	return;
    }

    /* make sure the buf3 is clear? */
    sprintf(buf3, " ");
    /* check to see if vict is playing? */
    sprintf(buf2,"the corpse of %s",arg);
    found = FALSE;
    for ( obj = first_object; obj; obj = obj->next )
    {
	if ( obj->in_room
        && !str_cmp( buf2, obj->short_descr )
        && (obj->pIndexData->vnum == 11 ) )
	{
	    found = TRUE;
	    ch_printf( ch, "Bagging body: [%5d] %-28s [%5d] %s\n\r",
		obj->pIndexData->vnum,
		obj->short_descr,
		obj->in_room->vnum,
		obj->in_room->name );
            obj_from_room(obj);
            obj = obj_to_char(obj, ch);
	    obj->timer = -1;
            save_char_obj( ch );
	}
    }

    if ( !found )
	ch_printf(ch," You couldn't find any %s\n\r",buf2);
    return;
}


/* New owhere by Altrag, 03/14/96 */
void do_owhere( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    char arg1[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    bool found;
    int icnt = 0;

    argument = one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
	send_to_char( "Owhere what?\n\r", ch );
	return;
    }
    argument = one_argument(argument, arg1);

    set_pager_color( AT_PLAIN, ch );
    if ( arg1[0] != '\0' && !str_prefix(arg1, "nesthunt") )
    {
      if ( !(obj = get_obj_world(ch, arg)) )
      {
        send_to_char( "Nesthunt for what object?\n\r", ch );
        return;
      }
      for ( ; obj->in_obj; obj = obj->in_obj )
      {
	pager_printf(ch, "[%5d] %-28s in object [%5d] %s\n\r",
                obj->pIndexData->vnum, obj_short(obj),
                obj->in_obj->pIndexData->vnum, obj->in_obj->short_descr);
	++icnt;
      }
      sprintf(buf, "[%5ld] %-28s in ", obj->pIndexData->vnum,
		obj_short(obj));
      if ( obj->carried_by )
        sprintf(buf+strlen(buf), "invent [%5ld] %s\n\r",
                (IS_NPC(obj->carried_by) ? obj->carried_by->pIndexData->vnum
                : 0), PERS(obj->carried_by, ch));
      else if ( obj->in_room )
        sprintf(buf+strlen(buf), "room   [%5ld] %s\n\r",
                obj->in_room->vnum, obj->in_room->name);
      else if ( obj->in_obj )
      {
        bug("do_owhere: obj->in_obj after NULL!",0);
        strcat(buf, "object??\n\r");
      }
      else
      {
        bug("do_owhere: object doesnt have location!",0);
        strcat(buf, "nowhere??\n\r");
      }
      send_to_pager(buf, ch);
      ++icnt;
      pager_printf(ch, "Nested %d levels deep.\n\r", icnt);
      return;
    }

    found = FALSE;
    for ( obj = first_object; obj; obj = obj->next )
    {
        if ( !nifty_is_name( arg, obj->name ) )
            continue;
        found = TRUE;

        sprintf(buf, "(%3d) [%5ld] %-28s in ", ++icnt, obj->pIndexData->vnum,
                obj_short(obj));
        if ( obj->carried_by )
          sprintf(buf+strlen(buf), "invent [%5ld] %s\n\r",
                  (IS_NPC(obj->carried_by) ? obj->carried_by->pIndexData->vnum
                  : 0), PERS(obj->carried_by, ch));
        else if ( obj->in_room )
          sprintf(buf+strlen(buf), "room   [%5ld] %s\n\r",
                  obj->in_room->vnum, obj->in_room->name);
        else if ( obj->in_obj )
          sprintf(buf+strlen(buf), "object [%5ld] %s\n\r",
                  obj->in_obj->pIndexData->vnum, obj_short(obj->in_obj));
        else
        {
          bug("do_owhere: object doesnt have location!",0);
          strcat(buf, "nowhere??\n\r");
        }
        send_to_pager(buf, ch);
    }

    if ( !found )
      act( AT_PLAIN, "You didn't find any $T.", ch, NULL, arg, TO_CHAR );
    else
      pager_printf(ch, "%d matches.\n\r", icnt);

    return;
}


void do_reboo( CHAR_DATA *ch, char *argument )
{
    send_to_char( "If you want to REBOOT, spell it out.\n\r", ch );
    return;
}



void do_reboot( CHAR_DATA *ch, char *argument )
{
    extern bool mud_down;
    CHAR_DATA *vch;

    if ( str_cmp( argument, "mud now" )
    &&   str_cmp( argument, "nosave" )
    &&   str_cmp( argument, "and sort skill table" ) )
    {
	send_to_char( "Syntax: 'reboot mud now' or 'reboot nosave'\n\r", ch );
	return;
    }

    if ( auction->item )
	do_auction( ch, "stop");

    do_echo( ch, "Rebooting mud... See you soon." );

    if ( !str_cmp(argument, "and sort skill table") )
    {
	sort_skill_table();
	save_skill_table();
    }

    /* Save all characters before booting. */
    if ( str_cmp(argument, "nosave") )
	for ( vch = first_char; vch; vch = vch->next )
	    if ( !IS_NPC( vch ) )
		save_char_obj( vch );

    if ( str_cmp(argument, "nosave") )
                        save_some_areas();

    mud_down = TRUE;
    return;
}



void do_shutdow( CHAR_DATA *ch, char *argument )
{
    send_to_char( "If you want to SHUTDOWN, spell it out.\n\r", ch );
    return;
}



void do_shutdown( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    extern bool mud_down;
    CHAR_DATA *vch;

    if ( str_cmp( argument, "mud now" ) && str_cmp(argument, "nosave") )
    {
	send_to_char( "Syntax: 'shutdown mud now' or 'shutdown nosave'\n\r", ch );
	return;
    }

    if ( auction->item )
	do_auction( ch, "stop");

    sprintf( buf, "Shutdown by %s.", ch->name );
    append_file( ch, SHUTDOWN_FILE, buf );
    strcat( buf, "\n\r" );
    do_echo( ch, buf );

    /* Save all characters before booting. */
    if ( str_cmp(argument, "nosave") )
	for ( vch = first_char; vch; vch = vch->next )
	    if ( !IS_NPC( vch ) )
		save_char_obj( vch );
    mud_down = TRUE;
    return;
}


void do_snoop( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    DESCRIPTOR_DATA *d;
    CHAR_DATA *victim;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Snoop whom?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( !victim->desc )
    {
	send_to_char( "No descriptor to snoop.\n\r", ch );
	return;
    }

    if ( victim == ch )
    {
	send_to_char( "Cancelling all snoops.\n\r", ch );
	for ( d = first_descriptor; d; d = d->next )
	    if ( d->snoop_by == ch->desc )
		d->snoop_by = NULL;
	return;
    }

    if ( victim->desc->snoop_by )
    {
	send_to_char( "Busy already.\n\r", ch );
	return;
    }

    /*
     * Minimum snoop level... a secret mset value
     * makes the snooper think that the victim is already being snooped
     */
    if ( get_trust( victim ) >= get_trust( ch )
    ||  (victim->pcdata && victim->pcdata->min_snoop > get_trust( ch )) )
    {
	send_to_char( "Busy already.\n\r", ch );
	return;
    }

    if ( ch->desc )
    {
	for ( d = ch->desc->snoop_by; d; d = d->snoop_by )
	    if ( d->character == victim || d->original == victim )
	    {
		send_to_char( "No snoop loops.\n\r", ch );
		return;
	    }
    }

    victim->desc->snoop_by = ch->desc;
    send_to_char( "Ok.\n\r", ch );
    return;
}



void do_switch( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Switch into whom?\n\r", ch );
	return;
    }

    if ( !ch->desc )
	return;

    if ( ch->desc->original )
    {
	send_to_char( "You are already switched.\n\r", ch );
	return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( victim == ch )
    {
	send_to_char( "Ok.\n\r", ch );
	return;
    }

    if ( victim->desc )
    {
	send_to_char( "Character in use.\n\r", ch );
	return;
    }

    ch->desc->character = victim;
    ch->desc->original  = ch;
    victim->desc        = ch->desc;
    ch->desc            = NULL;
    ch->switched	= victim;
    send_to_char( "Ok.\n\r", victim );
    return;
}



void do_return( CHAR_DATA *ch, char *argument )
{
    if ( !ch->desc )
	return;

    if ( !ch->desc->original )
    {
	send_to_char( "You aren't switched.\n\r", ch );
	return;
    }

    if (IS_SET(ch->act, ACT_POLYMORPHED))
    {
      send_to_char("Use revert to return from a polymorphed mob.\n\r", ch);
      return;
    }

    send_to_char( "You return to your original body.\n\r", ch );
	if ( IS_NPC( ch ) && IS_AFFECTED( ch, AFF_POSSESS ) )
	{
		affect_strip( ch, gsn_possess );
		REMOVE_BIT( ch->affected_by, AFF_POSSESS );
	}
/*    if ( IS_NPC( ch->desc->character ) )
      REMOVE_BIT( ch->desc->character->affected_by, AFF_POSSESS );*/
    ch->desc->character       = ch->desc->original;
    ch->desc->original        = NULL;
    ch->desc->character->desc = ch->desc;
    ch->desc->character->switched = NULL;
    ch->desc                  = NULL;
    return;
}



void do_minvoke( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    MOB_INDEX_DATA *pMobIndex;
    CHAR_DATA *victim;
    long vnum;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Syntax: minvoke <vnum>.\n\r", ch );
	return;
    }

    if ( !is_number( arg ) )
    {
	char arg2[MAX_INPUT_LENGTH];
	int  hash, cnt;
	int  count = number_argument( arg, arg2 );

	vnum = -1;
	for ( hash = cnt = 0; hash < MAX_KEY_HASH; hash++ )
	    for ( pMobIndex = mob_index_hash[hash];
		  pMobIndex;
		  pMobIndex = pMobIndex->next )
	    if ( nifty_is_name( arg2, pMobIndex->player_name )
	    &&   ++cnt == count )
	    {
		vnum = pMobIndex->vnum;
		break;
	    }
	if ( vnum == -1 )
	{
	    send_to_char( "No such mobile exists.\n\r", ch );
	    return;
	}
    }
    else
	vnum = atoi( arg );


    if ( ( pMobIndex = get_mob_index( vnum ) ) == NULL )
    {
	send_to_char( "No mobile has that vnum.\n\r", ch );
	return;
    }

    victim = create_mobile( pMobIndex );
    char_to_room( victim, ch->in_room );
    act( AT_IMMORT, "$n has created $N!", ch, NULL, victim, TO_ROOM );
    send_to_char( "Ok.\n\r", ch );
    return;
}



void do_oinvoke( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    OBJ_INDEX_DATA *pObjIndex;
    OBJ_DATA *obj;
    long vnum;
    int level;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' )
    {
	send_to_char( "Syntax: oinvoke <vnum> <level>.\n\r", ch );
	return;
    }

    if ( arg2[0] == '\0' )
    {
	level = get_trust( ch );
    }
    else
    {
	if ( !is_number( arg2 ) )
	{
	    send_to_char( "Syntax: oinvoke <vnum> <level>.\n\r", ch );
	    return;
	}
	level = atoi( arg2 );
	if ( level < 0 || level > get_trust( ch ) )
	{
	    send_to_char( "Limited to your trust level.\n\r", ch );
	    return;
        }
    }

    if ( !is_number( arg1 ) )
    {
	char arg[MAX_INPUT_LENGTH];
	int  hash, cnt;
	int  count = number_argument( arg1, arg );

	vnum = -1;
	for ( hash = cnt = 0; hash < MAX_KEY_HASH; hash++ )
	    for ( pObjIndex = obj_index_hash[hash];
		  pObjIndex;
		  pObjIndex = pObjIndex->next )
	    if ( nifty_is_name( arg, pObjIndex->name )
	    &&   ++cnt == count )
	    {
		vnum = pObjIndex->vnum;
		break;
	    }
	if ( vnum == -1 )
	{
	    send_to_char( "No such object exists.\n\r", ch );
	    return;
	}
    }
    else
	vnum = atoi( arg1 );

    if ( ( pObjIndex = get_obj_index( vnum ) ) == NULL )
    {
	send_to_char( "No object has that vnum.\n\r", ch );
	return;
    }

/* Commented out by Narn, it seems outdated
    if ( IS_OBJ_STAT( pObjIndex, ITEM_PROTOTYPE )
    &&	 pObjIndex->count > 5 )
    {
	send_to_char( "That object is at its limit.\n\r", ch );
	return;
    }
*/

    obj = create_object( pObjIndex, level );
    if ( CAN_WEAR(obj, ITEM_TAKE) )
    {
	obj = obj_to_char( obj, ch );
    }
    else
    {
	obj = obj_to_room( obj, ch->in_room );
	act( AT_IMMORT, "$n has created $p!", ch, obj, NULL, TO_ROOM );
    }
    send_to_char( "Ok.\n\r", ch );
    return;
}



void do_purge( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    OBJ_DATA *obj;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	/* 'purge' */
	CHAR_DATA *vnext;
	OBJ_DATA  *obj_next;

	for ( victim = ch->in_room->first_person; victim; victim = vnext )
	{
	    vnext = victim->next_in_room;
	    if ( IS_NPC(victim) && victim != ch && !IS_SET(victim->act, ACT_POLYMORPHED))
		extract_char( victim, TRUE );
	}

	for ( obj = ch->in_room->first_content; obj; obj = obj_next )
	{
	    obj_next = obj->next_content;
	    extract_obj( obj );
	}

	act( AT_IMMORT, "$n purges the room!", ch, NULL, NULL, TO_ROOM);
	send_to_char( "Ok.\n\r", ch );
	return;
    }
    victim = NULL; obj = NULL;

    /* fixed to get things in room first -- i.e., purge portal (obj),
     * no more purging mobs with that keyword in another room first
     * -- Tri */
    if ( ( victim = get_char_room( ch, arg ) ) == NULL
    && ( obj = get_obj_here( ch, arg ) ) == NULL )
    {
      if ( ( victim = get_char_world( ch, arg ) ) == NULL
      &&   ( obj = get_obj_world( ch, arg ) ) == NULL )  /* no get_obj_room */
      {
	send_to_char( "They aren't here.\n\r", ch );
	return;
      }
    }

/* Single object purge in room for high level purge - Scryn 8/12*/
    if ( obj )
    {
	separate_obj( obj );
	act( AT_IMMORT, "$n purges $p.", ch, obj, NULL, TO_ROOM);
	act( AT_IMMORT, "You make $p disappear in a puff of smoke!", ch, obj, NULL, TO_CHAR);
	extract_obj( obj );
	return;
    }


    if ( !IS_NPC(victim) )
    {
	send_to_char( "Not on PC's.\n\r", ch );
	return;
    }

    if ( victim == ch )
    {
    	send_to_char( "You cannot purge yourself!\n\r", ch );
    	return;
    }

    if (IS_SET(victim->act, ACT_POLYMORPHED))
    {
      send_to_char("You cannot purge a polymorphed player.\n\r", ch);
      return;
    }
    act( AT_IMMORT, "$n purges $N.", ch, NULL, victim, TO_NOTVICT );
    extract_char( victim, TRUE );
    return;
}


void do_low_purge( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    OBJ_DATA *obj;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Purge what?\n\r", ch );
	return;
    }

    victim = NULL; obj = NULL;
    if ( ( victim = get_char_room( ch, arg ) ) == NULL
    &&	 ( obj    = get_obj_here ( ch, arg ) ) == NULL )
    {
	send_to_char( "You can't find that here.\n\r", ch );
	return;
    }

    if ( obj )
    {
	separate_obj( obj );
	act( AT_IMMORT, "$n purges $p!", ch, obj, NULL, TO_ROOM );
	act( AT_IMMORT, "You make $p disappear in a puff of smoke!", ch, obj, NULL, TO_CHAR );
	extract_obj( obj );
	return;
    }

    if ( !IS_NPC(victim) )
    {
	send_to_char( "Not on PC's.\n\r", ch );
	return;
    }

    if ( victim == ch )
    {
    	send_to_char( "You cannot purge yourself!\n\r", ch );
    	return;
    }

    act( AT_IMMORT, "$n purges $N.", ch, NULL, victim, TO_NOTVICT );
    act( AT_IMMORT, "You make $N disappear in a puff of smoke!", ch, NULL, victim, TO_CHAR );
    extract_char( victim, TRUE );
    return;
}


void do_balzhur( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;
    int sn;

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Who is deserving of such a fate?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't playing.\n\r", ch);
	return;
    }

    if ( IS_NPC(victim) )
    {
	send_to_char( "Not on NPC's.\n\r", ch );
	return;
    }

    if ( get_trust( victim ) >= get_trust( ch ) )
    {
	send_to_char( "I wouldn't even think of that if I were you...\n\r", ch );
	return;
    }

	set_char_color( AT_WHITE, ch );
	send_to_char( "You summon the demon Balzhur to wreak your wrath!\n\r", ch );
	send_to_char( "Balzhur sneers at you evilly, then vanishes in a puff of smoke.\n\r", ch );
	set_char_color( AT_IMMORT, victim );
	send_to_char( "You hear an ungodly sound in the distance that makes your blood run cold!\n\r", victim );
	sprintf( buf, "Balzhur screams, 'You are MINE %s!!!'", victim->name );
	echo_to_all( AT_IMMORT, buf, ECHOTAR_ALL );
	victim->top_level =1;
	victim->trust	 = 0;
	victim->max_hit  = 100;
	victim->max_mana = 0;
	victim->max_move = 500;
	for ( sn = 0; sn < top_sn; sn++ )
	    victim->pcdata->learned[sn] = 0;
	victim->hit      = victim->max_hit;
	victim->mana     = victim->max_mana;
	victim->move     = victim->max_move;

	do_help(victim, "M_BALZHUR_" );
	set_char_color( AT_WHITE, victim );
	send_to_char( "You awake after a long period of time...\n\r", victim );
	while ( victim->first_carrying )
	     extract_obj( victim->first_carrying );
        return;
}

void do_advance( CHAR_DATA *ch, char *argument )
{
}

void do_immortalize( CHAR_DATA *ch, char *argument )
{
}



void do_trust( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    int level;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' || arg2[0] == '\0' || !is_number( arg2 ) )
    {
	send_to_char( "Syntax: trust <char> <level>.\n\r", ch );
	return;
    }

    if ( ( victim = get_char_room( ch, arg1 ) ) == NULL )
    {
	send_to_char( "That player is not here.\n\r", ch);
	return;
    }

    if ( ( level = atoi( arg2 ) ) < 0 || level > MAX_LEVEL )
    {
	send_to_char( "Level must be 0 (reset) or 1 to 60.\n\r", ch );
	return;
    }

    if ( level > get_trust( ch ) )
    {
	send_to_char( "Limited to your own trust.\n\r", ch );
	return;
    }

    if ( get_trust( victim ) >= get_trust( ch ) )
    {
	send_to_char( "> invalid command\n\r", ch );
	return;
    }

    victim->trust = level;
    send_to_char( "Ok.\n\r", ch );
    return;
}



void do_restore( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
	send_to_char( "Restore whom?\n\r", ch );
	return;
    }

    if ( !str_cmp( arg, "all" ) )
    {
	CHAR_DATA *vch;
	CHAR_DATA *vch_next;

        if ( !ch->pcdata )
          return;

        last_restore_all_time    = current_time;
        ch->pcdata->restore_time = current_time;
        save_char_obj( ch );
        send_to_char( "Ok.\n\r", ch);
	for ( vch = first_char; vch; vch = vch_next )
	{
	    vch_next = vch->next;

	    if ( !IS_NPC( vch ) && !IS_IMMORTAL( vch ) )
	    {
   		vch->hit = vch->max_hit;
		vch->mana = vch->max_mana;
		vch->move = vch->max_move;
		vch->pcdata->condition[COND_BLOODTHIRST] = (10 + vch->top_level);
		update_pos (vch);
		act( AT_IMMORT, "$n has restored you.", ch, NULL, vch, TO_VICT);
	    }
	}
    }
    else
    {

    CHAR_DATA *victim;

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    victim->hit  = victim->max_hit;
    victim->mana = victim->max_mana;
    victim->move = victim->max_move;
    if ( victim->pcdata )
      victim->pcdata->condition[COND_BLOODTHIRST] = (10 + victim->top_level);
    update_pos( victim );
    if ( ch != victim )
      act( AT_IMMORT, "$n has restored you.", ch, NULL, victim, TO_VICT );
    send_to_char( "Ok.\n\r", ch );
    return;
    }
}

void do_restoretime( CHAR_DATA *ch, char *argument )
{
  long int time_passed;
  int hour, minute;

  if ( !last_restore_all_time )
     ch_printf( ch, "There has been no restore all since reboot\n\r");
  else
     {
     time_passed = current_time - last_restore_all_time;
     hour = (int) ( time_passed / 3600 );
     minute = (int) ( ( time_passed - ( hour * 3600 ) ) / 60 );
     ch_printf( ch, "The  last restore all was %d hours and %d minutes ago.\n\r",
                  hour, minute );
     }

  if ( !ch->pcdata )
    return;

  if ( !ch->pcdata->restore_time )
  {
    send_to_char( "You have never done a restore all.\n\r", ch );
    return;
  }

  time_passed = current_time - ch->pcdata->restore_time;
  hour = (int) ( time_passed / 3600 );
  minute = (int) ( ( time_passed - ( hour * 3600 ) ) / 60 );
  ch_printf( ch, "Your last restore all was %d hours and %d minutes ago.\n\r",
                  hour, minute );
  return;
}

void do_freeze( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Freeze whom?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( IS_NPC(victim) )
    {
	send_to_char( "Not on NPC's.\n\r", ch );
	return;
    }

    if ( get_trust( victim ) >= get_trust( ch ) )
    {
	send_to_char( "You failed.\n\r", ch );
	return;
    }

    if ( IS_SET(victim->act, PLR_FREEZE) )
    {
	REMOVE_BIT(victim->act, PLR_FREEZE);
	send_to_char( "You can play again.\n\r", victim );
	send_to_char( "FREEZE removed.\n\r", ch );
    }
    else
    {
	SET_BIT(victim->act, PLR_FREEZE);
	send_to_char( "You can't do ANYthing!\n\r", victim );
	send_to_char( "FREEZE set.\n\r", ch );
    }

    save_char_obj( victim );

    return;
}



void do_log( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Log whom?\n\r", ch );
	return;
    }

    if ( !str_cmp( arg, "all" ) )
    {
	if ( fLogAll )
	{
	    fLogAll = FALSE;
	    send_to_char( "Log ALL off.\n\r", ch );
	}
	else
	{
	    fLogAll = TRUE;
	    send_to_char( "Log ALL on.\n\r", ch );
	}
	return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( IS_NPC(victim) )
    {
	send_to_char( "Not on NPC's.\n\r", ch );
	return;
    }

    /*
     * No level check, gods can log anyone.
     */
    if ( IS_SET(victim->act, PLR_LOG) )
    {
	REMOVE_BIT(victim->act, PLR_LOG);
	send_to_char( "LOG removed.\n\r", ch );
    }
    else
    {
	SET_BIT(victim->act, PLR_LOG);
	send_to_char( "LOG set.\n\r", ch );
    }

    return;
}


void do_noemote( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Noemote whom?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( IS_NPC(victim) )
    {
	send_to_char( "Not on NPC's.\n\r", ch );
	return;
    }

    if ( get_trust( victim ) >= get_trust( ch ) )
    {
	send_to_char( "You failed.\n\r", ch );
	return;
    }

    if ( IS_SET(victim->act, PLR_NO_EMOTE) )
    {
	REMOVE_BIT(victim->act, PLR_NO_EMOTE);
	send_to_char( "You can emote again.\n\r", victim );
	send_to_char( "NO_EMOTE removed.\n\r", ch );
    }
    else
    {
	SET_BIT(victim->act, PLR_NO_EMOTE);
	send_to_char( "You can't emote!\n\r", victim );
	send_to_char( "NO_EMOTE set.\n\r", ch );
    }

    return;
}



void do_notell( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Notell whom?", ch );
	return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( IS_NPC(victim) )
    {
	send_to_char( "Not on NPC's.\n\r", ch );
	return;
    }

    if ( get_trust( victim ) >= get_trust( ch ) )
    {
	send_to_char( "You failed.\n\r", ch );
	return;
    }

    if ( IS_SET(victim->act, PLR_NO_TELL) )
    {
	REMOVE_BIT(victim->act, PLR_NO_TELL);
	send_to_char( "You can tell again.\n\r", victim );
	send_to_char( "NO_TELL removed.\n\r", ch );
    }
    else
    {
	SET_BIT(victim->act, PLR_NO_TELL);
	send_to_char( "You can't tell!\n\r", victim );
	send_to_char( "NO_TELL set.\n\r", ch );
    }

    return;
}


void do_notitle( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        send_to_char( "Notitle whom?\n\r", ch );
        return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
        send_to_char( "They aren't here.\n\r", ch );
        return;
    }

    if ( IS_NPC(victim) )
    {
        send_to_char( "Not on NPC's.\n\r", ch );
        return;
    }

    if ( get_trust( victim ) >= get_trust( ch ) )
    {
        send_to_char( "You failed.\n\r", ch );
        return;
    }

    if ( IS_SET(victim->pcdata->flags, PCFLAG_NOTITLE) )
    {
        REMOVE_BIT(victim->pcdata->flags, PCFLAG_NOTITLE);
        send_to_char( "You can set your own title again.\n\r", victim );
        send_to_char( "NOTITLE removed.\n\r", ch );
    }
    else
    {
        SET_BIT(victim->pcdata->flags, PCFLAG_NOTITLE);
        sprintf( buf, "%s", victim->name );
        set_title( victim, buf );
        send_to_char( "You can't set your own title!\n\r", victim );
        send_to_char( "NOTITLE set.\n\r", ch );
    }

    return;
}

void do_silence( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Silence whom?", ch );
	return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( IS_NPC(victim) )
    {
	send_to_char( "Not on NPC's.\n\r", ch );
	return;
    }

    if ( get_trust( victim ) >= get_trust( ch ) )
    {
	send_to_char( "You failed.\n\r", ch );
	return;
    }

    if ( IS_SET(victim->act, PLR_SILENCE) )
    {
	send_to_char( "Player already silenced, use unsilence to remove.\n\r", ch );
    }
    else
    {
	SET_BIT(victim->act, PLR_SILENCE);
	send_to_char( "You can't use channels!\n\r", victim );
	send_to_char( "SILENCE set.\n\r", ch );
    }

    return;
}

void do_unsilence( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Unsilence whom?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( IS_NPC(victim) )
    {
	send_to_char( "Not on NPC's.\n\r", ch );
	return;
    }

    if ( get_trust( victim ) >= get_trust( ch ) )
    {
	send_to_char( "You failed.\n\r", ch );
	return;
    }

    if ( IS_SET(victim->act, PLR_SILENCE) )
    {
	REMOVE_BIT(victim->act, PLR_SILENCE);
	send_to_char( "You can use channels again.\n\r", victim );
	send_to_char( "SILENCE removed.\n\r", ch );
    }
    else
    {
	send_to_char( "That player is not silenced.\n\r", ch );
    }

    return;
}




void do_peace( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *rch;

    act( AT_IMMORT, "$n booms, 'PEACE!'", ch, NULL, NULL, TO_ROOM );
    for ( rch = ch->in_room->first_person; rch; rch = rch->next_in_room )
    {
	if ( rch->fighting )
	{
	    stop_fighting( rch, TRUE );
	    do_sit( rch, "" );
	}

        /* Added by Narn, Nov 28/95 */
        stop_hating( rch );
        stop_hunting( rch );
        stop_fearing( rch );
    }

    send_to_char( "Ok.\n\r", ch );
    return;
}



BAN_DATA *		first_ban;
BAN_DATA *		last_ban;
BAN_DATA *		first_tban;
BAN_DATA *		last_tban;

void save_banlist( void )
{
  BAN_DATA *pban;
  FILE *fp;

  fclose( fpReserve );
  if ( !(fp = fopen( SYSTEM_DIR BAN_LIST, "w" )) )
  {
    bug( "Save_banlist: Cannot open " BAN_LIST, 0 );
    perror(BAN_LIST);
    fpReserve = fopen( NULL_FILE, "r" );
    return;
  }
  for ( pban = first_ban; pban; pban = pban->next )
    fprintf( fp, "%d %s~~%s~\n", pban->level, pban->name, pban->ban_time );
  fprintf( fp, "-1\n" );
  fclose( fp );
  fpReserve = fopen( NULL_FILE, "r" );
  return;
}



void do_ban( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    BAN_DATA *pban;
    int bnum;

    if ( IS_NPC(ch) )
	return;

    argument = one_argument( argument, arg );

    set_pager_color( AT_PLAIN, ch );
    if ( arg[0] == '\0' )
    {
	send_to_pager( "Banned sites:\n\r", ch );
	send_to_pager( "[ #] (Lv) Time                     Site\n\r", ch );
	send_to_pager( "---- ---- ------------------------ ---------------\n\r", ch );
	for ( pban = first_ban, bnum = 1; pban; pban = pban->next, bnum++ )
	    pager_printf(ch, "[%2d] (%2d) %-24s %s\n\r", bnum,
	            pban->level, pban->ban_time, pban->name);
	return;
    }

    /* People are gonna need .# instead of just # to ban by just last
       number in the site ip.                               -- Altrag */
    if ( is_number(arg) )
    {
      for ( pban = first_ban, bnum = 1; pban; pban = pban->next, bnum++ )
        if ( bnum == atoi(arg) )
          break;
      if ( !pban )
      {
        do_ban(ch, "");
        return;
      }
      argument = one_argument(argument, arg);
      if ( arg[0] == '\0' )
      {
        do_ban( ch, "help" );
        return;
      }
      else if ( !str_cmp(arg, "newban") )
      {
        pban->level = 0;
        send_to_char( "New characters banned.\n\r", ch );
      }
      else if ( !str_cmp(arg, "mortal") )
      {
        pban->level = 1;
        send_to_char( "All mortals banned.\n\r", ch );
      }
      else if ( !str_cmp(arg, "total") )
      {
        pban->level = 200;
        send_to_char( "Everyone banned.\n\r", ch );
      }
      else
      {
        do_ban(ch, "help");
        return;
      }
      save_banlist( );
      return;
    }

    if ( !str_cmp(arg, "help") )
    {
      send_to_char( "Syntax: ban <site address>\n\r", ch );
      send_to_char( "Syntax: ban <ban number> <newban|mortal|"
                    "total>\n\r", ch );
      return;
    }

    for ( pban = first_ban; pban; pban = pban->next )
    {
	if ( !str_cmp( arg, pban->name ) )
	{
	    send_to_char( "That site is already banned!\n\r", ch );
	    return;
	}
    }

    CREATE( pban, BAN_DATA, 1 );
    LINK( pban, first_ban, last_ban, next, prev );
    pban->name	= str_dup( arg );
    pban->level = 2;
    sprintf(buf, "%24.24s", ctime(&current_time));
    pban->ban_time = str_dup( buf );
    save_banlist( );
    send_to_char( "Ban created.  Mortals banned from site.\n\r", ch );
    return;
}


void do_allow( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    BAN_DATA *pban;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Remove which site from the ban list?\n\r", ch );
	return;
    }

    for ( pban = first_ban; pban; pban = pban->next )
    {
	if ( !str_cmp( arg, pban->name ) )
	{
	    UNLINK( pban, first_ban, last_ban, next, prev );
	    if ( pban->ban_time )
	      DISPOSE(pban->ban_time);
	    DISPOSE( pban->name );
	    DISPOSE( pban );
	    save_banlist( );
	    send_to_char( "Site no longer banned.\n\r", ch );
	    return;
	}
    }

    send_to_char( "Site is not banned.\n\r", ch );
    return;
}



void do_wizlock( CHAR_DATA *ch, char *argument )
{
    extern bool wizlock;
    wizlock = !wizlock;

    if ( wizlock )
	send_to_char( "Game wizlocked.\n\r", ch );
    else
	send_to_char( "Game un-wizlocked.\n\r", ch );

    return;
}


void do_noresolve( CHAR_DATA *ch, char *argument )
{
    sysdata.NO_NAME_RESOLVING = !sysdata.NO_NAME_RESOLVING;

    if ( sysdata.NO_NAME_RESOLVING )
	send_to_char( "Name resolving disabled.\n\r", ch );
    else
	send_to_char( "Name resolving enabled.\n\r", ch );

    return;
}


void do_users( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    DESCRIPTOR_DATA *d;
    int count;
    char arg[MAX_INPUT_LENGTH];

    one_argument (argument, arg);
    count	= 0;
    buf[0]	= '\0';

    set_pager_color( AT_PLAIN, ch );
    sprintf(buf,
"Desc|Con|Idle| Port | Player@HostIP                 ");
   strcat(buf, "\n\r");
   strcat(buf, "----+---+----+------+-------------------------------");
   strcat(buf, "\n\r");
   send_to_pager(buf, ch);

    for ( d = first_descriptor; d; d = d->next )
    {
	if (  d->character && can_see( ch, d->character ) )
	{
	    count++;
	    sprintf( buf,
	     " %3d| %2d|%4d|%6d| %s@%s ",
		d->descriptor,
		d->connected,
		d->idle / 4,
		d->port,
		d->original  ? d->original->name  :
		d->character ? d->character->name : "(none)",
		d->hostip );
	    sprintf( buf + strlen( buf ), " (%s@%s)", d->user, d->host  );
	    strcat(buf, "\n\r");
	    send_to_pager( buf, ch );
	}
    }
    pager_printf( ch, "%d user%s.\n\r", count, count == 1 ? "" : "s" );
    return;
}



/*
 * Thanks to Grodyn for pointing out bugs in this function.
 */
void do_force( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' || argument[0] == '\0' )
    {
	send_to_char( "Force whom to do what?\n\r", ch );
	return;
    }


    if ( !str_cmp( arg, "all" ) )
    {
	CHAR_DATA *vch;
	CHAR_DATA *vch_next;
	int toomany = 0;

	for ( vch = first_char; vch; vch = vch_next )
	{
	    vch_next = vch->next;

	    if ( !IS_NPC(vch) && get_trust( vch ) < get_trust( ch ) )
	    {
		act( AT_IMMORT, "$n forces you to '$t'.", ch, argument, vch, TO_VICT );
		interpret( vch, argument );
	        if ( toomany++ > 1000 )  /* i doubt we'd have that many players */
	            return;              /* so this would be an infinate loop */
	    }


	}
    }
    else
    {
	CHAR_DATA *victim;

	if ( ( victim = get_char_world( ch, arg ) ) == NULL )
	{
	    send_to_char( "They aren't here.\n\r", ch );
	    return;
	}

	if ( victim == ch )
	{
	    send_to_char( "Aye aye, right away!\n\r", ch );
	    return;
	}

	if (  get_trust( victim ) >= get_trust( ch )  )
	{
	    send_to_char( "Do it yourself!\n\r", ch );
	    return;
	}

    act( AT_IMMORT, "$n forces you to '$t'.", ch, argument, victim, TO_VICT );
	interpret( victim, argument );
    }

    send_to_char( "Ok.\n\r", ch );
    return;
}


void do_invis( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    sh_int level;

    /*
    if ( IS_NPC(ch))
	return;
    */

    argument = one_argument( argument, arg );
    if ( arg[0] != '\0' )
    {
	if ( !is_number( arg ) )
	{
	   send_to_char( "Usage: invis | invis <level>\n\r", ch );
	   return;
	}
	level = atoi( arg );
	if ( level < 2 || level > get_trust( ch ) )
	{
	    send_to_char( "Invalid level.\n\r", ch );
	    return;
	}

	if (!IS_NPC(ch))
        {
	  ch->pcdata->wizinvis = level;
	  ch_printf( ch, "Wizinvis level set to %d.\n\r", level );
        }

        if (IS_NPC(ch))
        {
          ch->mobinvis = level;
          ch_printf( ch, "Mobinvis level set to %d.\n\r", level );
        }
	return;
    }

    if (!IS_NPC(ch))
    {
    if ( ch->pcdata->wizinvis < 2 )
      ch->pcdata->wizinvis = ch->top_level;
    }

    if (IS_NPC(ch))
    {
    if ( ch->mobinvis < 2 )
      ch->mobinvis = ch->top_level;
    }

    if ( IS_SET(ch->act, PLR_WIZINVIS) )
    {
	REMOVE_BIT(ch->act, PLR_WIZINVIS);
	act( AT_IMMORT, "$n slowly fades into existence.", ch, NULL, NULL, TO_ROOM );
	send_to_char( "You slowly fade back into existence.\n\r", ch );
    }
    else
    {
	SET_BIT(ch->act, PLR_WIZINVIS);
	act( AT_IMMORT, "$n slowly fades into thin air.", ch, NULL, NULL, TO_ROOM );
	send_to_char( "You slowly vanish into thin air.\n\r", ch );
    }

    return;
}


void do_holylight( CHAR_DATA *ch, char *argument )
{
    if ( IS_NPC(ch) )
	return;

    if ( IS_SET(ch->act, PLR_HOLYLIGHT) )
    {
	REMOVE_BIT(ch->act, PLR_HOLYLIGHT);
	send_to_char( "Holy light mode off.\n\r", ch );
    }
    else
    {
	SET_BIT(ch->act, PLR_HOLYLIGHT);
	send_to_char( "Holy light mode on.\n\r", ch );
    }

    return;
}

void do_cmdtable( CHAR_DATA *ch, char *argument )
{
    int hash, cnt;
    CMDTYPE *cmd;

    set_pager_color( AT_PLAIN, ch );
    send_to_pager("Commands and Number of Uses This Run\n\r", ch);

    for ( cnt = hash = 0; hash < 126; hash++ )
	for ( cmd = command_hash[hash]; cmd; cmd = cmd->next )
	{
	    if ((++cnt)%4)
		pager_printf(ch,"%-6.6s %4d\t",cmd->name,cmd->userec.num_uses);
	    else
		pager_printf(ch,"%-6.6s %4d\n\r", cmd->name,cmd->userec.num_uses );
	}
    return;
}

/*
 * Load up a player file
 */
void do_loadup( CHAR_DATA *ch, char *argument )
{
    char fname[1024];
    char name[256];
    struct stat fst;
    bool loaded;
    DESCRIPTOR_DATA *d;
    long old_room_vnum;
    char buf[MAX_STRING_LENGTH];

    one_argument( argument, name );
    if ( name[0] == '\0' )
    {
	send_to_char( "Usage: loadup <playername>\n\r", ch );
	return;
    }

    name[0] = UPPER(name[0]);

    sprintf( fname, "%s%c/%s", PLAYER_DIR, tolower(name[0]),
			name );
    if ( stat( fname, &fst ) != -1 )
    {
	CREATE( d, DESCRIPTOR_DATA, 1 );
	d->next = NULL;
	d->prev = NULL;
	d->connected = CON_GET_NAME;
	d->outsize = 2000;
	CREATE( d->outbuf, char, d->outsize );

	loaded = load_char_obj( d, name, FALSE );
	add_char( d->character );
        old_room_vnum = d->character->in_room->vnum;
	char_to_room( d->character, ch->in_room );
	if ( get_trust(d->character) >= get_trust( ch ) )
	{
	   do_say( d->character, "Do *NOT* disturb me again!" );
	   send_to_char( "I think you'd better leave that player alone!\n\r", ch );
	   d->character->desc	= NULL;
	   do_quit( d->character, "" );
	   return;
	}
	d->character->desc	= NULL;
	d->character->retran    = old_room_vnum;
	d->character		= NULL;
	DISPOSE( d->outbuf );
	DISPOSE( d );
	ch_printf(ch, "Player %s loaded from room %ld.\n\r", name,old_room_vnum );
	sprintf(buf, "%s appears from nowhere, eyes glazed over.\n\r", name );
        act( AT_IMMORT, buf, ch, NULL, NULL, TO_ROOM );
	send_to_char( "Done.\n\r", ch );
	return;
    }
    /* else no player file */
    send_to_char( "No such player.\n\r", ch );
    return;
}

void do_fixchar( CHAR_DATA *ch, char *argument )
{
    char name[MAX_STRING_LENGTH];
    CHAR_DATA *victim;

    one_argument( argument, name );
    if ( name[0] == '\0' )
    {
	send_to_char( "Usage: fixchar <playername>\n\r", ch );
	return;
    }
    victim = get_char_room( ch, name );
    if ( !victim )
    {
	send_to_char( "They're not here.\n\r", ch );
	return;
    }
    fix_char( victim );
/*  victim->armor	= 100;
    victim->mod_str	= 0;
    victim->mod_dex	= 0;
    victim->mod_wis	= 0;
    victim->mod_int	= 0;
    victim->mod_con	= 0;
    victim->mod_cha	= 0;
    victim->mod_lck	= 0;
    victim->damroll	= 0;
    victim->hitroll	= 0;
    victim->alignment	= URANGE( -1000, victim->alignment, 1000 );
    victim->saving_spell_staff = 0; */
    send_to_char( "Done.\n\r", ch );
}

void do_newbieset( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    CHAR_DATA *victim;

    argument = one_argument( argument, arg1 );
    argument = one_argument (argument, arg2);

    if ( arg1[0] == '\0' )
    {
	send_to_char( "Syntax: newbieset <char>.\n\r", ch );
	return;
    }

    if ( ( victim = get_char_room( ch, arg1 ) ) == NULL )
    {
	send_to_char( "That player is not here.\n\r", ch);
	return;
    }

    if ( IS_NPC(victim) )
    {
	send_to_char( "Not on NPC's.\n\r", ch );
	return;
    }

    if ( ( victim->top_level < 1 ) || ( victim->top_level > 5 ) )
    {
     send_to_char( "Level of victim must be 1 to 5.\n\r", ch );
	return;
    }

     obj = create_object( get_obj_index(OBJ_VNUM_SCHOOL_DAGGER), 1 );
     obj_to_char(obj, victim);

     {
       OBJ_INDEX_DATA *obj_ind = get_obj_index( 23 );
       if ( obj_ind != NULL )
       {
         obj = create_object( obj_ind, 1 );
         obj_to_char( obj, victim );
       }
     }

/* Added the burlap sack to the newbieset.  The sack is part of sgate.are
   called Spectral Gate.  Brittany */

     {

       OBJ_INDEX_DATA *obj_ind = get_obj_index( 20 );
       if ( obj_ind != NULL )
       {
         obj = create_object( obj_ind, 1 );
         obj_to_char( obj, victim );
       }
     }

    act( AT_IMMORT, "$n has equipped you with a newbieset.", ch, NULL, victim, TO_VICT);
    ch_printf( ch, "You have re-equipped %s.\n\r", victim->name );
    return;
}

/*
 * Extract area names from "input" string and place result in "output" string
 * e.g. "aset joe.are sedit susan.are cset" --> "joe.are susan.are"
 * - Gorog
 */
void extract_area_names (char *inp, char *out)
{
char buf[MAX_INPUT_LENGTH], *pbuf=buf;
int  len;

*out='\0';
while (inp && *inp)
   {
   inp = one_argument(inp, buf);
   if ( (len=strlen(buf)) >= 5 && !strcmp(".are", pbuf+len-4) )
       {
       if (*out) strcat (out, " ");
       strcat (out, buf);
       }
   }
}

/*
 * Remove area names from "input" string and place result in "output" string
 * e.g. "aset joe.are sedit susan.are cset" --> "aset sedit cset"
 * - Gorog
 */
void remove_area_names (char *inp, char *out)
{
char buf[MAX_INPUT_LENGTH], *pbuf=buf;
int  len;

*out='\0';
while (inp && *inp)
   {
   inp = one_argument(inp, buf);
   if ( (len=strlen(buf)) < 5 || strcmp(".are", pbuf+len-4) )
       {
       if (*out) strcat (out, " ");
       strcat (out, buf);
       }
   }
}

void do_bestowarea( CHAR_DATA *ch, char *argument )
{
}

void do_bestow( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Bestow whom with what?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( IS_NPC( victim ) )
    {
	send_to_char( "You can't give special abilities to a mob!\n\r", ch );
	return;
    }

    if ( get_trust( victim ) > get_trust( ch ) )
    {
	send_to_char( "You aren't powerful enough...\n\r", ch );
	return;
    }

    if (!victim->pcdata->bestowments)
      victim->pcdata->bestowments = str_dup("");

    if ( argument[0] == '\0' || !str_cmp( argument, "list" ) )
    {
        ch_printf( ch, "Current bestowed commands on %s: %s.\n\r",
                      victim->name, victim->pcdata->bestowments );
        return;
    }

    if ( !str_cmp( argument, "none" ) )
    {
        DISPOSE( victim->pcdata->bestowments );
	victim->pcdata->bestowments = str_dup("");
        ch_printf( ch, "Bestowments removed from %s.\n\r", victim->name );
        ch_printf( victim, "%s has removed your bestowed commands.\n\r", ch->name );
        return;
    }

    sprintf( buf, "%s %s", victim->pcdata->bestowments, argument );
    DISPOSE( victim->pcdata->bestowments );
    victim->pcdata->bestowments = str_dup( buf );
    ch_printf( victim, "%s has bestowed on you the command(s): %s\n\r",
             ch->name, argument );
    send_to_char( "Done.\n\r", ch );
}

struct tm *update_time ( struct tm *old_time )
{
   time_t time;

   time = mktime(old_time);
   return localtime(&time);
}

void do_set_boot_time( CHAR_DATA *ch, char *argument)
{
   char arg[MAX_INPUT_LENGTH];
   char arg1[MAX_INPUT_LENGTH];
   bool check;

   check = FALSE;

   argument = one_argument(argument, arg);

    if ( arg[0] == '\0' )
    {
	send_to_char( "Syntax: setboot time {hour minute <day> <month> <year>}\n\r", ch);
	send_to_char( "        setboot manual {0/1}\n\r", ch);
	send_to_char( "        setboot default\n\r", ch);
        ch_printf( ch, "Boot time is currently set to %s, manual bit is set to %d\n\r"
	,reboot_time, set_boot_time->manual );
	return;
    }

    if ( !str_cmp(arg, "time") )
    {
      struct tm *now_time;

      argument = one_argument(argument, arg);
      argument = one_argument(argument, arg1);
      if ( !*arg || !*arg1 || !is_number(arg) || !is_number(arg1) )
      {
	send_to_char("You must input a value for hour and minute.\n\r", ch);
 	return;
      }
      now_time = localtime(&current_time);

      if ( (now_time->tm_hour = atoi(arg)) < 0 || now_time->tm_hour > 23 )
      {
        send_to_char("Valid range for hour is 0 to 23.\n\r", ch);
        return;
      }

      if ( (now_time->tm_min = atoi(arg1)) < 0 || now_time->tm_min > 59 )
      {
        send_to_char("Valid range for minute is 0 to 59.\n\r", ch);
        return;
      }

      argument = one_argument(argument, arg);
      if ( *arg != '\0' && is_number(arg) )
      {
        if ( (now_time->tm_mday = atoi(arg)) < 1 || now_time->tm_mday > 31 )
        {
	  send_to_char("Valid range for day is 1 to 31.\n\r", ch);
	  return;
        }
        argument = one_argument(argument, arg);
        if ( *arg != '\0' && is_number(arg) )
        {
          if ( (now_time->tm_mon = atoi(arg)) < 1 || now_time->tm_mon > 12 )
          {
            send_to_char( "Valid range for month is 1 to 12.\n\r", ch );
            return;
          }
          now_time->tm_mon--;
          argument = one_argument(argument, arg);
          if ( (now_time->tm_year = atoi(arg)-1900) < 0 ||
                now_time->tm_year > 199 )
          {
            send_to_char( "Valid range for year is 1900 to 2099.\n\r", ch );
            return;
          }
        }
      }
      now_time->tm_sec = 0;
      if ( mktime(now_time) < current_time )
      {
        send_to_char( "You can't set a time previous to today!\n\r", ch );
        return;
      }
      if (set_boot_time->manual == 0)
 	set_boot_time->manual = 1;
      new_boot_time = update_time(now_time);
      new_boot_struct = *new_boot_time;
      new_boot_time = &new_boot_struct;
      reboot_check(mktime(new_boot_time));
      get_reboot_string();

      ch_printf(ch, "Boot time set to %s\n\r", reboot_time);
      check = TRUE;
    }
    else if ( !str_cmp(arg, "manual") )
    {
      argument = one_argument(argument, arg1);
      if (arg1[0] == '\0')
      {
	send_to_char("Please enter a value for manual boot on/off\n\r", ch);
	return;
      }

      if ( !is_number(arg1))
      {
	send_to_char("Value for manual must be 0 (off) or 1 (on)\n\r", ch);
	return;
      }

      if (atoi(arg1) < 0 || atoi(arg1) > 1)
      {
	send_to_char("Value for manual must be 0 (off) or 1 (on)\n\r", ch);
	return;
      }

      set_boot_time->manual = atoi(arg1);
      ch_printf(ch, "Manual bit set to %s\n\r", arg1);
      check = TRUE;
      get_reboot_string();
      return;
    }

    else if (!str_cmp( arg, "default" ))
    {
      set_boot_time->manual = 0;
    /* Reinitialize new_boot_time */
      new_boot_time = localtime(&current_time);
      new_boot_time->tm_mday += 1;
      if (new_boot_time->tm_hour > 12)
      new_boot_time->tm_mday += 1;
      new_boot_time->tm_hour = 6;
      new_boot_time->tm_min = 0;
      new_boot_time->tm_sec = 0;
      new_boot_time = update_time(new_boot_time);

      sysdata.DENY_NEW_PLAYERS = FALSE;

      send_to_char("Reboot time set back to normal.\n\r", ch);
      check = TRUE;
    }

    if (!check)
    {
      send_to_char("Invalid argument for setboot.\n\r", ch);
      return;
    }

    else
    {
      get_reboot_string();
      new_boot_time_t = mktime(new_boot_time);
    }
}
/* Online high level immortal command for displaying what the encryption
 * of a name/password would be, taking in 2 arguments - the name and the
 * password - can still only change the password if you have access to 
 * pfiles and the correct password
 */
void do_form_password( CHAR_DATA *ch, char *argument)
{
   char arg[MAX_STRING_LENGTH];

   argument = one_argument(argument, arg);

   ch_printf(ch, "Those two arguments encrypted would result in: %s",
   crypt(arg, argument));
   return;
}

/*
 * Purge a player file.  No more player.  -- Altrag
 */
void do_destro( CHAR_DATA *ch, char *argument )
{
  set_char_color( AT_RED, ch );
  send_to_char("If you want to destroy a character, spell it out!\n\r",ch);
  return;
}

/*
 * This could have other applications too.. move if needed. -- Altrag
 */
void close_area( AREA_DATA *pArea )
{
}

void do_destroy( CHAR_DATA *ch, char *argument )
{
  CHAR_DATA *victim;
  char buf[MAX_STRING_LENGTH];
  char buf2[MAX_STRING_LENGTH];
  char arg[MAX_INPUT_LENGTH];

  one_argument( argument, arg );
  if ( arg[0] == '\0' )
  {
      send_to_char( "Destroy what player file?\n\r", ch );
      return;
  }

  for ( victim = first_char; victim; victim = victim->next )
    if ( !IS_NPC(victim) && !str_cmp(victim->name, arg) )
      break;
  if ( !victim )
  {
    DESCRIPTOR_DATA *d;

    /* Make sure they aren't halfway logged in. */
    for ( d = first_descriptor; d; d = d->next )
      if ( (victim = d->character) && !IS_NPC(victim) &&
          !str_cmp(victim->name, arg) )
        break;
    if ( d )
      close_socket( d, TRUE );
  }
  else
  {
    int x, y;

    quitting_char = victim;
    save_char_obj( victim );
    saving_char = NULL;
    extract_char( victim, TRUE );
    for ( x = 0; x < MAX_WEAR; x++ )
	for ( y = 0; y < MAX_LAYERS; y++ )
	    save_equipment[x][y] = NULL;
  }

  sprintf( buf, "%s%c/%s", PLAYER_DIR, tolower(arg[0]),
          arg );
  sprintf( buf2, "%s%c/%s", BACKUP_DIR, tolower(arg[0]),
          arg );
  if ( !rename( buf, buf2 ) )
  {
    set_char_color( AT_RED, ch );
    send_to_char( "Player destroyed.  Pfile saved in backup directory.\n\r", ch );
  }
  else if ( errno == ENOENT )
  {
    set_char_color( AT_PLAIN, ch );
    send_to_char( "Player does not exist.\n\r", ch );
  }
  else
  {
    set_char_color( AT_WHITE, ch );
    ch_printf( ch, "Unknown error #%d - %s.  Report to Thoric.\n\r",
            errno, strerror( errno ) );
    sprintf( buf, "%s destroying %s", ch->name, arg );
    perror( buf );
  }
  return;
}
extern ROOM_INDEX_DATA *       room_index_hash         [MAX_KEY_HASH]; /* db.c */


/* Super-AT command:

FOR ALL <action>
FOR MORTALS <action>
FOR GODS <action>
FOR MOBS <action>
FOR EVERYWHERE <action>


Executes action several times, either on ALL players (not including yourself),
MORTALS (including trusted characters), GODS (characters with level higher than
L_HERO), MOBS (Not recommended) or every room (not recommended either!)

If you insert a # in the action, it will be replaced by the name of the target.

If # is a part of the action, the action will be executed for every target
in game. If there is no #, the action will be executed for every room containg
at least one target, but only once per room. # cannot be used with FOR EVERY-
WHERE. # can be anywhere in the action.

Example:

FOR ALL SMILE -> you will only smile once in a room with 2 players.
FOR ALL TWIDDLE # -> In a room with A and B, you will twiddle A then B.

Destroying the characters this command acts upon MAY cause it to fail. Try to
avoid something like FOR MOBS PURGE (although it actually works at my MUD).

FOR MOBS TRANS 3054 (transfer ALL the mobs to Midgaard temple) does NOT work
though :)

The command works by transporting the character to each of the rooms with
target in them. Private rooms are not violated.

*/

/* Expand the name of a character into a string that identifies THAT
   character within a room. E.g. the second 'guard' -> 2. guard
*/
const char * name_expand (CHAR_DATA *ch)
{
	int count = 1;
	CHAR_DATA *rch;
	char name[MAX_INPUT_LENGTH]; /*  HOPEFULLY no mob has a name longer than THAT */

	static char outbuf[MAX_INPUT_LENGTH];

	if (!IS_NPC(ch))
		return ch->name;

	one_argument (ch->name, name); /* copy the first word into name */

	if (!name[0]) /* weird mob .. no keywords */
	{
		strcpy (outbuf, ""); /* Do not return NULL, just an empty buffer */
		return outbuf;
	}

	/* ->people changed to ->first_person -- TRI */
	for (rch = ch->in_room->first_person; rch && (rch != ch);rch =
	    rch->next_in_room)
		if (is_name (name, rch->name))
			count++;


	sprintf (outbuf, "%d.%s", count, name);
	return outbuf;
}


void do_for (CHAR_DATA *ch, char *argument)
{
	char range[MAX_INPUT_LENGTH];
	char buf[MAX_STRING_LENGTH];
	bool fGods = FALSE, fMortals = FALSE, fMobs = FALSE, fEverywhere = FALSE, found;
	ROOM_INDEX_DATA *room, *old_room;
	CHAR_DATA *p, *p_prev;  /* p_next to p_prev -- TRI */
	int i;

	argument = one_argument (argument, range);

	if (!range[0] || !argument[0]) /* invalid usage? */
	{
		do_help (ch, "for");
		return;
	}

	if (!str_prefix("quit", argument))
	{
		send_to_char ("Are you trying to crash the MUD or something?\n\r",ch);
		return;
	}


	if (!str_cmp (range, "all"))
	{
		fMortals = TRUE;
		fGods = TRUE;
	}
	else if (!str_cmp (range, "gods"))
		fGods = TRUE;
	else if (!str_cmp (range, "mortals"))
		fMortals = TRUE;
	else if (!str_cmp (range, "mobs"))
		fMobs = TRUE;
	else if (!str_cmp (range, "everywhere"))
		fEverywhere = TRUE;
	else
		do_help (ch, "for"); /* show syntax */

	/* do not allow # to make it easier */
	if (fEverywhere && strchr (argument, '#'))
	{
		send_to_char ("Cannot use FOR EVERYWHERE with the # thingie.\n\r",ch);
		return;
	}

	if (strchr (argument, '#')) /* replace # ? */
	{
		/* char_list - last_char, p_next - gch_prev -- TRI */
		for (p = last_char; p ; p = p_prev )
		{
			p_prev = p->prev;  /* TRI */
		/*	p_next = p->next; */ /* In case someone DOES try to AT MOBS SLAY # */
			found = FALSE;

			if (!(p->in_room) || room_is_private(p, p->in_room) || (p == ch))
				continue;

			if (IS_NPC(p) && fMobs)
				found = TRUE;
			else if (!IS_NPC(p) && IS_IMMORTAL(p) && fGods)
				found = TRUE;
			else if (!IS_NPC(p) && !IS_IMMORTAL(p) && fMortals)
				found = TRUE;

			/* It looks ugly to me.. but it works :) */
			if (found) /* p is 'appropriate' */
			{
				char *pSource = argument; /* head of buffer to be parsed */
				char *pDest = buf; /* parse into this */

				while (*pSource)
				{
					if (*pSource == '#') /* Replace # with name of target */
					{
						const char *namebuf = name_expand (p);

						if (namebuf) /* in case there is no mob name ?? */
							while (*namebuf) /* copy name over */
								*(pDest++) = *(namebuf++);

						pSource++;
					}
					else
						*(pDest++) = *(pSource++);
				} /* while */
				*pDest = '\0'; /* Terminate */

				/* Execute */
				old_room = ch->in_room;
				char_from_room (ch);
				char_to_room (ch,p->in_room);
				interpret (ch, buf);
				char_from_room (ch);
				char_to_room (ch,old_room);

			} /* if found */
		} /* for every char */
	}
	else /* just for every room with the appropriate people in it */
	{
		for (i = 0; i < MAX_KEY_HASH; i++) /* run through all the buckets */
			for (room = room_index_hash[i] ; room ; room = room->next)
			{
				found = FALSE;

				/* Anyone in here at all? */
				if (fEverywhere) /* Everywhere executes always */
					found = TRUE;
				else if (!room->first_person) /* Skip it if room is empty */
					continue;
				/* ->people changed to first_person -- TRI */

				/* Check if there is anyone here of the requried type */
				/* Stop as soon as a match is found or there are no more ppl in room */
				/* ->people to ->first_person -- TRI */
				for (p = room->first_person; p && !found; p = p->next_in_room)
				{

					if (p == ch) /* do not execute on oneself */
						continue;

					if (IS_NPC(p) && fMobs)
						found = TRUE;
					else if (!IS_NPC(p) && IS_IMMORTAL(p) && fGods)
						found = TRUE;
					else if (!IS_NPC(p) && IS_IMMORTAL(p) && fMortals)
						found = TRUE;
				} /* for everyone inside the room */

				if (found && !room_is_private(p, room)) /* Any of the required type here AND room not private? */
				{
					/* This may be ineffective. Consider moving character out of old_room
					   once at beginning of command then moving back at the end.
					   This however, is more safe?
					*/

					old_room = ch->in_room;
					char_from_room (ch);
					char_to_room (ch, room);
					interpret (ch, argument);
					char_from_room (ch);
					char_to_room (ch, old_room);
				} /* if found */
			} /* for every room in a bucket */
	} /* if strchr */
} /* do_for */

void save_sysdata  args( ( SYSTEM_DATA sys ) );

void do_cset( CHAR_DATA *ch, char *argument )
{
}

void get_reboot_string(void)
{
  sprintf(reboot_time, "%s", asctime(new_boot_time));
}



/* Vnum search command by Swordbearer */
void do_vsearch( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    bool found = FALSE;
    OBJ_DATA *obj;
    OBJ_DATA *in_obj;
    int obj_counter = 1;
    int argi;

    one_argument( argument, arg );

    if( arg[0] == '\0' )
    {
        send_to_char( "Syntax:  vsearch <vnum>.\n\r", ch );
        return;
    }

    set_pager_color( AT_PLAIN, ch );
    argi=atoi(arg);
    if (argi<0 && argi>20000)
    {
	send_to_char( "Vnum out of range.\n\r", ch);
	return;
    }
    for ( obj = first_object; obj != NULL; obj = obj->next )
    {
	if ( !can_see_obj( ch, obj ) || !( argi == obj->pIndexData->vnum ))
	  continue;

	found = TRUE;
	for ( in_obj = obj; in_obj->in_obj != NULL;
	  in_obj = in_obj->in_obj );

	if ( in_obj->carried_by != NULL )
	  pager_printf( ch, "[%2d] Level %d %s carried by %s.\n\r",
		obj_counter,
		obj->level, obj_short(obj),
		PERS( in_obj->carried_by, ch ) );
	else
	  pager_printf( ch, "[%2d] [%-5d] %s in %s.\n\r", obj_counter,
		( ( in_obj->in_room ) ? in_obj->in_room->vnum : 0 ),
		obj_short(obj), ( in_obj->in_room == NULL ) ?
		"somewhere" : in_obj->in_room->name );

	obj_counter++;
    }

    if ( !found )
      send_to_char( "Nothing like that in hell, earth, or heaven.\n\r" , ch );

    return;
}

/* 
 * Simple function to let any imm make any player instantly sober.
 * Saw no need for level restrictions on this.
 * Written by Narn, Apr/96 
 */
void do_sober( CHAR_DATA *ch, char *argument )
{
  CHAR_DATA *victim;
  char arg1 [MAX_INPUT_LENGTH];

  smash_tilde( argument );
  argument = one_argument( argument, arg1 );
  if ( ( victim = get_char_room( ch, arg1 ) ) == NULL )
  {
    send_to_char( "They aren't here.\n\r", ch );
    return;
  }

  if ( IS_NPC( victim ) )
  {
    send_to_char( "Not on mobs.\n\r", ch );
    return;
  }

  if ( victim->pcdata )
    victim->pcdata->condition[COND_DRUNK] = 0;
  send_to_char( "Ok.\n\r", ch );
  send_to_char( "You feel sober again.\n\r", victim );
  return;
}


/*
 * Free a social structure					-Thoric
 */
void free_social( SOCIALTYPE *social )
{
    if ( social->name )
      DISPOSE( social->name );
    if ( social->char_no_arg )
      DISPOSE( social->char_no_arg );
    if ( social->others_no_arg )
      DISPOSE( social->others_no_arg );
    if ( social->char_found )
      DISPOSE( social->char_found );
    if ( social->others_found )
      DISPOSE( social->others_found );
    if ( social->vict_found )
      DISPOSE( social->vict_found );
    if ( social->char_auto )
      DISPOSE( social->char_auto );
    if ( social->others_auto )
      DISPOSE( social->others_auto );
    DISPOSE( social );
}

/*
 * Remove a social from it's hash index				-Thoric
 */
void unlink_social( SOCIALTYPE *social )
{
    SOCIALTYPE *tmp, *tmp_next;
    int hash;

    if ( !social )
    {
	bug( "Unlink_social: NULL social", 0 );
	return;
    }

    if ( social->name[0] < 'a' || social->name[0] > 'z' )
	hash = 0;
    else
	hash = (social->name[0] - 'a') + 1;

    if ( social == (tmp=social_index[hash]) )
    {
	social_index[hash] = tmp->next;
	return;
    }
    for ( ; tmp; tmp = tmp_next )
    {
	tmp_next = tmp->next;
	if ( social == tmp_next )
	{
	    tmp->next = tmp_next->next;
	    return;
	}
    }
}

/*
 * Add a social to the social index table			-Thoric
 * Hashed and insert sorted
 */
void add_social( SOCIALTYPE *social )
{
    int hash, x;
    SOCIALTYPE *tmp, *prev;

    if ( !social )
    {
	bug( "Add_social: NULL social", 0 );
	return;
    }

    if ( !social->name )
    {
	bug( "Add_social: NULL social->name", 0 );
	return;
    }

    if ( !social->char_no_arg )
    {
	bug( "Add_social: NULL social->char_no_arg", 0 );
	return;
    }

    /* make sure the name is all lowercase */
    for ( x = 0; social->name[x] != '\0'; x++ )
	social->name[x] = LOWER(social->name[x]);

    if ( social->name[0] < 'a' || social->name[0] > 'z' )
	hash = 0;
    else
	hash = (social->name[0] - 'a') + 1;

    if ( (prev = tmp = social_index[hash]) == NULL )
    {
	social->next = social_index[hash];
	social_index[hash] = social;
	return;
    }

    for ( ; tmp; tmp = tmp->next )
    {
	if ( (x=strcmp(social->name, tmp->name)) == 0 )
	{
	    bug( "Add_social: trying to add duplicate name to bucket %d", hash );
	    free_social( social );
	    return;
	}
	else
	if ( x < 0 )
	{
	    if ( tmp == social_index[hash] )
	    {
		social->next = social_index[hash];
		social_index[hash] = social;
		return;
	    }
	    prev->next = social;
	    social->next = tmp;
	    return;
	}
	prev = tmp;
    }

    /* add to end */
    prev->next = social;
    social->next = NULL;
    return;
}

/*
 * Social editor/displayer/save/delete				-Thoric
 */
void do_sedit( CHAR_DATA *ch, char *argument )
{
    SOCIALTYPE *social;
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];

    smash_tilde( argument );
    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    set_char_color( AT_SOCIAL, ch );

    if ( arg1[0] == '\0' )
    {
	send_to_char( "Syntax: sedit <social> [field]\n\r", ch );
	send_to_char( "Syntax: sedit <social> create\n\r", ch );
	send_to_char( "Syntax: sedit <social> delete\n\r", ch );
	send_to_char( "Syntax: sedit <save>\n\r", ch );
	send_to_char( "\n\rField being one of:\n\r", ch );
	send_to_char( "  cnoarg onoarg cfound ofound vfound cauto oauto\n\r", ch );
	return;
    }

    if ( !str_cmp( arg1, "save" ) )
    {
	save_socials();
	send_to_char( "Saved.\n\r", ch );
	return;
    }

    social = find_social( arg1 );

    if ( !str_cmp( arg2, "create" ) )
    {
	if ( social )
	{
	    send_to_char( "That social already exists!\n\r", ch );
	    return;
	}
	CREATE( social, SOCIALTYPE, 1 );
	social->name = str_dup( arg1 );
	sprintf( arg2, "You %s.", arg1 );
	social->char_no_arg = str_dup( arg2 );
	add_social( social );
	send_to_char( "Social added.\n\r", ch );
	return;
    }

    if ( !social )
    {
	send_to_char( "Social not found.\n\r", ch );
	return;
    }

    if ( arg2[0] == '\0' || !str_cmp( arg2, "show" ) )
    {
	ch_printf( ch, "Social: %s\n\r\n\rCNoArg: %s\n\r",
	    social->name,	social->char_no_arg );
	ch_printf( ch, "ONoArg: %s\n\rCFound: %s\n\rOFound: %s\n\r",
	    social->others_no_arg	? social->others_no_arg	: "(not set)",
	    social->char_found		? social->char_found	: "(not set)",
	    social->others_found	? social->others_found	: "(not set)" );
	ch_printf( ch, "VFound: %s\n\rCAuto : %s\n\rOAuto : %s\n\r",
	    social->vict_found	? social->vict_found	: "(not set)",
	    social->char_auto	? social->char_auto	: "(not set)",
	    social->others_auto	? social->others_auto	: "(not set)" );
	return;
    }

    if ( !str_cmp( arg2, "delete" ) )
    {
	unlink_social( social );
	free_social( social );
	send_to_char( "Deleted.\n\r", ch );
	return;
    }

    if ( !str_cmp( arg2, "cnoarg" ) )
    {
	if ( argument[0] == '\0' || !str_cmp( argument, "clear" ) )
	{
	    send_to_char( "You cannot clear this field.  It must have a message.\n\r", ch );
	    return;
	}
	if ( social->char_no_arg )
	    DISPOSE( social->char_no_arg );
	social->char_no_arg = str_dup( argument );
	send_to_char( "Done.\n\r", ch );
	return;
    }

    if ( !str_cmp( arg2, "onoarg" ) )
    {
	if ( social->others_no_arg )
	    DISPOSE( social->others_no_arg );
	if ( argument[0] != '\0' && str_cmp( argument, "clear" ) )
	    social->others_no_arg = str_dup( argument );
	send_to_char( "Done.\n\r", ch );
	return;
    }

    if ( !str_cmp( arg2, "cfound" ) )
    {
	if ( social->char_found )
	    DISPOSE( social->char_found );
	if ( argument[0] != '\0' && str_cmp( argument, "clear" ) )
	    social->char_found = str_dup( argument );
	send_to_char( "Done.\n\r", ch );
	return;
    }

    if ( !str_cmp( arg2, "ofound" ) )
    {
	if ( social->others_found )
	    DISPOSE( social->others_found );
	if ( argument[0] != '\0' && str_cmp( argument, "clear" ) )
	    social->others_found = str_dup( argument );
	send_to_char( "Done.\n\r", ch );
	return;
    }

    if ( !str_cmp( arg2, "vfound" ) )
    {
	if ( social->vict_found )
	    DISPOSE( social->vict_found );
	if ( argument[0] != '\0' && str_cmp( argument, "clear" ) )
	    social->vict_found = str_dup( argument );
	send_to_char( "Done.\n\r", ch );
	return;
    }

    if ( !str_cmp( arg2, "cauto" ) )
    {
	if ( social->char_auto )
	    DISPOSE( social->char_auto );
	if ( argument[0] != '\0' && str_cmp( argument, "clear" ) )
	    social->char_auto = str_dup( argument );
	send_to_char( "Done.\n\r", ch );
	return;
    }

    if ( !str_cmp( arg2, "oauto" ) )
    {
	if ( social->others_auto )
	    DISPOSE( social->others_auto );
	if ( argument[0] != '\0' && str_cmp( argument, "clear" ) )
	    social->others_auto = str_dup( argument );
	send_to_char( "Done.\n\r", ch );
	return;
    }

    if ( !str_cmp( arg2, "name" ) )
    {
	bool relocate;

	one_argument( argument, arg1 );
	if ( arg1[0] == '\0' )
	{
	    send_to_char( "Cannot clear name field!\n\r", ch );
	    return;
	}
	if ( arg1[0] != social->name[0] )
	{
	    unlink_social( social );
	    relocate = TRUE;
	}
	else
	    relocate = FALSE;
	if ( social->name )
	    DISPOSE( social->name );
	social->name = str_dup( arg1 );
	if ( relocate )
	    add_social( social );
	send_to_char( "Done.\n\r", ch );
	return;
    }

    /* display usage message */
    do_sedit( ch, "" );
}

/*
 * Free a command structure					-Thoric
 */
void free_command( CMDTYPE *command )
{
    if ( command->name )
      DISPOSE( command->name );
    DISPOSE( command );
}

/*
 * Remove a command from it's hash index			-Thoric
 */
void unlink_command( CMDTYPE *command )
{
    CMDTYPE *tmp, *tmp_next;
    int hash;

    if ( !command )
    {
	bug( "Unlink_command NULL command", 0 );
	return;
    }

    hash = command->name[0]%126;

    if ( command == (tmp=command_hash[hash]) )
    {
	command_hash[hash] = tmp->next;
	return;
    }
    for ( ; tmp; tmp = tmp_next )
    {
	tmp_next = tmp->next;
	if ( command == tmp_next )
	{
	    tmp->next = tmp_next->next;
	    return;
	}
    }
}

/*
 * Add a command to the command hash table			-Thoric
 */
void add_command( CMDTYPE *command )
{
    int hash, x;
    CMDTYPE *tmp, *prev;

    if ( !command )
    {
	bug( "Add_command: NULL command", 0 );
	return;
    }

    if ( !command->name )
    {
	bug( "Add_command: NULL command->name", 0 );
	return;
    }

    if ( !command->do_fun )
    {
	bug( "Add_command: NULL command->do_fun", 0 );
	return;
    }

    /* make sure the name is all lowercase */
    for ( x = 0; command->name[x] != '\0'; x++ )
	command->name[x] = LOWER(command->name[x]);

    hash = command->name[0] % 126;

    if ( (prev = tmp = command_hash[hash]) == NULL )
    {
	command->next = command_hash[hash];
	command_hash[hash] = command;
	return;
    }

    /* add to the END of the list */
    for ( ; tmp; tmp = tmp->next )
	if ( !tmp->next )
	{
	    tmp->next = command;
	    command->next = NULL;
	}
    return;
}

/*
 * Command editor/displayer/save/delete				-Thoric
 */
void do_cedit( CHAR_DATA *ch, char *argument )
{
    CMDTYPE *command;
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];

    smash_tilde( argument );
    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    set_char_color( AT_IMMORT, ch );

    if ( arg1[0] == '\0' )
    {
	send_to_char( "Syntax: cedit save\n\r", ch );
	    send_to_char( "Syntax: cedit <command> create [code]\n\r", ch );
	    send_to_char( "Syntax: cedit <command> delete\n\r", ch );
	    send_to_char( "Syntax: cedit <command> show\n\r", ch );
	    send_to_char( "Syntax: cedit <command> [field]\n\r", ch );
	    send_to_char( "\n\rField being one of:\n\r", ch );
	    send_to_char( "  level position log code\n\r", ch );
	return;
    }

    if ( !str_cmp( arg1, "save" ) )
    {
	save_commands();
	send_to_char( "Saved.\n\r", ch );
	return;
    }

    command = find_command( arg1 );

    if ( !str_cmp( arg2, "create" ) )
    {
	if ( command )
	{
	    send_to_char( "That command already exists!\n\r", ch );
	    return;
	}
	CREATE( command, CMDTYPE, 1 );
	command->name = str_dup( arg1 );
	command->level = get_trust(ch);
	if ( *argument )
	  one_argument(argument, arg2);
	else
	  sprintf( arg2, "do_%s", arg1 );
	command->do_fun = skill_function( arg2 );
	add_command( command );
	send_to_char( "Command added.\n\r", ch );
	if ( command->do_fun == skill_notfound )
	  ch_printf( ch, "Code %s not found.  Set to no code.\n\r", arg2 );
	return;
    }

    if ( !command )
    {
	send_to_char( "Command not found.\n\r", ch );
	return;
    }
    else
    if ( command->level > get_trust(ch) )
    {
	send_to_char( "You cannot touch this command.\n\r", ch );
	return;
    }

    if ( arg2[0] == '\0' || !str_cmp( arg2, "show" ) )
    {
	ch_printf( ch, "Command:  %s\n\rLevel:    %d\n\rPosition: %d\n\rLog:      %d\n\rCode:     %s\n\r",
	    command->name, command->level, command->position, command->log,
	    skill_name(command->do_fun) );
	if ( command->userec.num_uses )
	  send_timer(&command->userec, ch);
	return;
    }


    if ( !str_cmp( arg2, "delete" ) )
    {
	unlink_command( command );
	free_command( command );
	send_to_char( "Deleted.\n\r", ch );
	return;
    }

    if ( !str_cmp( arg2, "code" ) )
    {
	DO_FUN *fun = skill_function( argument );

	if ( fun == skill_notfound )
	{
	    send_to_char( "Code not found.\n\r", ch );
	    return;
	}
	command->do_fun = fun;
	send_to_char( "Done.\n\r", ch );
	return;
    }

    if ( !str_cmp( arg2, "level" ) )
    {
	int level = atoi( argument );

	if ( level < 0 || level > get_trust(ch) )
	{
	    send_to_char( "Level out of range.\n\r", ch );
	    return;
	}
	command->level = level;
	send_to_char( "Done.\n\r", ch );
	return;
    }

    if ( !str_cmp( arg2, "log" ) )
    {
	int log = atoi( argument );

	if ( log < 0 || log > LOG_COMM )
	{
	    send_to_char( "Log out of range.\n\r", ch );
	    return;
	}
	command->log = log;
	send_to_char( "Done.\n\r", ch );
	return;
    }

    if ( !str_cmp( arg2, "position" ) )
    {
	int position = atoi( argument );

	if ( position < 0 || position > POS_DRAG )
	{
	    send_to_char( "Position out of range.\n\r", ch );
	    return;
	}
	command->position = position;
	send_to_char( "Done.\n\r", ch );
	return;
    }

    if ( !str_cmp( arg2, "name" ) )
    {
	bool relocate;

	one_argument( argument, arg1 );
	if ( arg1[0] == '\0' )
	{
	    send_to_char( "Cannot clear name field!\n\r", ch );
	    return;
	}
	if ( arg1[0] != command->name[0] )
	{
	    unlink_command( command );
	    relocate = TRUE;
	}
	else
	    relocate = FALSE;
	if ( command->name )
	    DISPOSE( command->name );
	command->name = str_dup( arg1 );
	if ( relocate )
	    add_command( command );
	send_to_char( "Done.\n\r", ch );
	return;
    }

    /* display usage message */
    do_cedit( ch, "" );
}

void do_arrest( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg1[MAX_INPUT_LENGTH];
    ROOM_INDEX_DATA *location;
    DESCRIPTOR_DATA *d;
    CHAR_DATA *victim;
    BAN_DATA *tban;

    argument = one_argument( argument, arg1 );

    if ( arg1[0] == '\0' )
    {
	send_to_char( "Arrest who?\n\r", ch );
	return;
    }

    if ( !IS_OFFICIAL(ch) )
    {
	send_to_char( "Nice try...\n\r", ch );
	return;
    }

	if ( ( location = get_room_index( ROOM_VNUM_JAIL) ) == NULL )
	{
	    send_to_char( "The jail is missing...\n\r", ch );
	    return;
	}

    for ( d = last_descriptor; d; d = d->prev )
    {
         victim   = d->original ? d->original : d->character;
         if ( str_cmp( arg1, victim->name ) )
         {
              victim = NULL;
              continue;
         }
         if ( d->connected != CON_PLAYING )
  	 {
	    send_to_char( "Please wait till they are done editing...\n\r", ch );
	    return;
	 }
         break;
    }

    if ( victim == NULL )
    {
	send_to_char( "They aren't logged in right now.\n\r", ch );
	return;
    }

    if ( !victim->in_room )
    {
	send_to_char( "They are in limbo.\n\r", ch );
	return;
    }

    if ( victim->fighting )
	stop_fighting( victim, TRUE );
    act( AT_MAGIC, "$n disappears in a cloud of swirling colors.", victim, NULL, NULL, TO_ROOM );
    victim->retran = victim->in_room->vnum;
    char_from_room( victim );
    char_to_room( victim, location );
    act( AT_MAGIC, "$n arrives from a puff of smoke.", victim, NULL, NULL, TO_ROOM );
    if ( ch != victim )
    act( AT_IMMORT, "$n has ordered your arrest.", ch, NULL, victim, TO_VICT );
    do_look( victim, "auto" );
    send_to_char( "Player sent to jail. New characters temporarily banned from address.\n\r", ch );

    SET_BIT( victim->act, PLR_SILENCE );
    SET_BIT( victim->act, PLR_NO_EMOTE );
    SET_BIT( victim->act, PLR_NO_TELL );
    SET_BIT( victim->act, PLR_NICE );

    CREATE( tban, BAN_DATA, 1 );
    LINK( tban, first_tban, last_tban, next, prev );
    tban->name	= str_dup( d->host );
    tban->level = 1;
    sprintf(buf, "%24.24s", ctime(&current_time));
    tban->ban_time = str_dup( buf );

}

// no need for Neuro
