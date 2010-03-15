#include <sys/types.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <fcntl.h>
#include <signal.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "mud.h"

/*
 * Socket and TCP/IP stuff.
 */
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <arpa/telnet.h>
#include <netdb.h>

#define MAX_NEST	100
static	OBJ_DATA *	rgObjNest	[MAX_NEST];


const	char	echo_off_str	[] = { IAC, WILL, TELOPT_ECHO, '\0' };
const	char	echo_on_str	[] = { IAC, WONT, TELOPT_ECHO, '\0' };
const	char 	go_ahead_str	[] = { IAC, GA, '\0' };

void    send_auth args( ( struct descriptor_data *d ) );
void    read_auth args( ( struct descriptor_data *d ) );
void    start_auth args( ( struct descriptor_data *d ) );
void    save_sysdata args( ( SYSTEM_DATA sys ) );

/*  from act_info?  */
void    show_condition( CHAR_DATA *ch, CHAR_DATA *victim );
void    write_ship_list args( ( void ) );
void 	substitute_alias(DESCRIPTOR_DATA *d, char *argument);

/*
 * Global variables.
 */
DESCRIPTOR_DATA *   first_descriptor;	/* First descriptor		*/
DESCRIPTOR_DATA *   last_descriptor;	/* Last descriptor		*/
DESCRIPTOR_DATA *   d_next;		/* Next descriptor in loop	*/
int		    num_descriptors;
FILE *		    fpReserve;		/* Reserved file handle		*/
bool		    mud_down;		/* Shutdown			*/
bool		    wizlock;		/* Game is wizlocked		*/
time_t              boot_time;
HOUR_MIN_SEC  	    set_boot_time_struct;
HOUR_MIN_SEC *      set_boot_time;
struct tm *         new_boot_time;
struct tm           new_boot_struct;
char		    str_boot_time[MAX_INPUT_LENGTH];
char		    lastplayercmd[MAX_INPUT_LENGTH*2];
time_t		    current_time;	/* Time of this pulse		*/
int		    control;		/* Controlling descriptor	*/
int		    newdesc;		/* New descriptor		*/
fd_set		    in_set;		/* Set of desc's for reading	*/
fd_set		    out_set;		/* Set of desc's for writing	*/
fd_set		    exc_set;		/* Set of desc's with errors	*/
int 		    maxdesc;

/*
 * OS-dependent local functions.
 */
void	game_loop		args( ( ) );
int	init_socket		args( ( int port ) );
void	new_descriptor		args( ( int new_desc ) );
bool	read_from_descriptor	args( ( DESCRIPTOR_DATA *d ) );
bool	write_to_descriptor	args( ( int desc, char *txt, int length ) );


#define  TELOPT_MXP        '\x5B'

const unsigned char will_mxp_str  [] = { IAC, WILL, TELOPT_MXP, '\0' };
const unsigned char start_mxp_str [] = { IAC, SB, TELOPT_MXP, IAC, SE, '\0' };
const unsigned char do_mxp_str    [] = { IAC, DO, TELOPT_MXP, '\0' };
const unsigned char dont_mxp_str  [] = { IAC, DONT, TELOPT_MXP, '\0' };


/*
 * Other local functions (OS-independent).
 */
bool	check_parse_name	args( ( char *name ) );
bool	check_reconnect		args( ( DESCRIPTOR_DATA *d, char *name,
		bool fConn ) );
bool	check_playing		args( ( DESCRIPTOR_DATA *d, char *name, bool kick ) );
bool	check_multi		args( ( DESCRIPTOR_DATA *d, char *name ) );
int	main			args( ( int argc, char **argv ) );
void	nanny			args( ( DESCRIPTOR_DATA *d, char *argument ) );
bool	flush_buffer		args( ( DESCRIPTOR_DATA *d, bool fPrompt ) );
void	read_from_buffer	args( ( DESCRIPTOR_DATA *d ) );
void	stop_idling		args( ( CHAR_DATA *ch ) );
void	free_desc		args( ( DESCRIPTOR_DATA *d ) );
void	display_prompt		args( ( DESCRIPTOR_DATA *d ) );
int	make_color_sequence	args( ( const char *col, char *buf,
		DESCRIPTOR_DATA *d ) );
void	set_pager_input		args( ( DESCRIPTOR_DATA *d,
		char *argument ) );
bool	pager_output		args( ( DESCRIPTOR_DATA *d ) );



void	mail_count		args( ( CHAR_DATA *ch ) );

/*
* Count number of mxp tags need converting
*    ie. < becomes &lt;
*        > becomes &gt;
*        & becomes &amp;
*/

int count_mxp_tags (const int bMXP, const char *txt, int length)
  {
  char c;
  const char * p;
  int count;
  int bInTag = FALSE;
  int bInEntity = FALSE;

  for (p = txt, count = 0;
       length > 0;
       p++, length--)
    {
    c = *p;

    if (bInTag)  /* in a tag, eg. <send> */
      {
      if (!bMXP)
        count--;     /* not output if not MXP */
      if (c == MXP_ENDc)
        bInTag = FALSE;
      } /* end of being inside a tag */
    else if (bInEntity)  /* in a tag, eg. <send> */
      {
      if (!bMXP)
        count--;     /* not output if not MXP */
      if (c == ';')
        bInEntity = FALSE;
      } /* end of being inside a tag */
    else switch (c)
      {

      case MXP_BEGc:
        bInTag = TRUE;
        if (!bMXP)
          count--;     /* not output if not MXP */
        break;

      case MXP_ENDc:   /* shouldn't get this case */
        if (!bMXP)
          count--;     /* not output if not MXP */
        break;

      case MXP_AMPc:
        bInEntity = TRUE;
        if (!bMXP)
          count--;     /* not output if not MXP */
        break;

      default:
        if (bMXP)
          {
          switch (c)
            {
            case '<':       /* < becomes &lt; */
            case '>':       /* > becomes &gt; */
              count += 3;
              break;

            case '&':
              count += 4;    /* & becomes &amp; */
              break;

            case '"':        /* " becomes &quot; */
              count += 5;
              break;

            } /* end of inner switch */
          }   /* end of MXP enabled */
      } /* end of switch on character */

     }   /* end of counting special characters */

  return count;
  } /* end of count_mxp_tags */

void convert_mxp_tags (const int bMXP, char * dest, const char *src, int length)
  {
char c;
const char * ps;
char * pd;
int bInTag = FALSE;
int bInEntity = FALSE;

  for (ps = src, pd = dest;
       length > 0;
       ps++, length--)
    {
    c = *ps;
    if (bInTag)  /* in a tag, eg. <send> */
      {
      if (c == MXP_ENDc)
        {
        bInTag = FALSE;
        if (bMXP)
          *pd++ = '>';
        }
      else if (bMXP)
        *pd++ = c;  /* copy tag only in MXP mode */
      } /* end of being inside a tag */
    else if (bInEntity)  /* in a tag, eg. <send> */
      {
      if (bMXP)
        *pd++ = c;  /* copy tag only in MXP mode */
      if (c == ';')
        bInEntity = FALSE;
      } /* end of being inside a tag */
    else switch (c)
      {
      case MXP_BEGc:
        bInTag = TRUE;
        if (bMXP)
          *pd++ = '<';
        break;

      case MXP_ENDc:    /* shouldn't get this case */
        if (bMXP)
          *pd++ = '>';
        break;

      case MXP_AMPc:
        bInEntity = TRUE;
        if (bMXP)
          *pd++ = '&';
        break;

      default:
        if (bMXP)
          {
          switch (c)
            {
            case '<':
              memcpy (pd, "&lt;", 4);
              pd += 4;
              break;

            case '>':
              memcpy (pd, "&gt;", 4);
              pd += 4;
              break;

            case '&':
              memcpy (pd, "&amp;", 5);
              pd += 5;
              break;

            case '"':
              memcpy (pd, "&quot;", 6);
              pd += 6;
              break;

            default:
              *pd++ = c;
              break;  /* end of default */

            } /* end of inner switch */
          }
        else
          *pd++ = c;  /* not MXP - just copy character */
        break;

      } /* end of switch on character */

    }   /* end of converting special characters */
  } /* end of convert_mxp_tags */

/* set up MXP */
void turn_on_mxp (DESCRIPTOR_DATA *d)
  {
  d->mxp = TRUE;  /* turn it on now */
 	write_to_buffer( d, start_mxp_str, 0 );
	write_to_buffer( d, MXPMODE (6), 0 );   /* permanent secure mode */
  write_to_buffer( d, MXPTAG ("!-- Set up MXP elements --"), 0);
  /* Exit tag */
  write_to_buffer( d, MXPTAG ("!ELEMENT Ex '<send>' FLAG=RoomExit"), 0);
  /* Room description tag */
  write_to_buffer( d, MXPTAG ("!ELEMENT rdesc '<p>' FLAG=RoomDesc"), 0);
  /* Get an item tag (for things on the ground) */
  write_to_buffer( d, MXPTAG
      ("!ELEMENT Get \"<send href='"
           "get &#39;&name;&#39;|"
           "eval &#39;&name;&#39;"
           //"drink &#39;&name;&#39;"
       "' "
       "hint='RH mouse click to use this object|"
           "Get &desc;|"
           "Eval &desc;"
           //"Drink from &desc;"
       "'>\" ATT='name desc'"),
      0);
  /* Drop an item tag (for things in the inventory) */
  write_to_buffer( d, MXPTAG
      ("!ELEMENT Drop \"<send href='"
           "drop &#39;&name;&#39;|"
           "eval &#39;&name;&#39;|"
           "look in &#39;&name;&#39;|"
           "load &#39;&name;&#39;"
           //"eat &#39;&name;&#39;|"
           //"drink &#39;&name;&#39;"
       "' "
       "hint='RH mouse click to use this object|"
           "Drop &desc;|"
           "Eval &desc;|"
           "Look inside &desc;|"
           "Load &desc;"
           //"Eat &desc;|"
           //"Drink &desc;"
       "'>\" ATT='name desc'"),
      0);
  /* Bid an item tag (for things in the auction) */
  write_to_buffer( d, MXPTAG
      ("!ELEMENT Bid \"<send href='bid &#39;&name;&#39;' "
       "hint='Bid for &desc;'>\" "
       "ATT='name desc'"),
      0);
  /* List an item tag (for things in a shop) */
  write_to_buffer( d, MXPTAG
      ("!ELEMENT List \"<send href='buy &#39;&name;&#39;' "
       "hint='Buy &desc;'>\" "
       "ATT='name desc'"),
      0);
  /* Player tag (for who lists, tells etc.) */
  write_to_buffer( d, MXPTAG
      ("!ELEMENT Player \"<send href='tell &#39;&name;&#39; ' "
       "hint='Send a message to &name;' prompt>\" "
       "ATT='name'"),
      0);
  } /* end of turn_on_mxp */


int main( int argc, char **argv )
{
	struct timeval now_time;
	int port;

	/*
	 * Memory debugging if needed.
	 */
#if defined(MALLOC_DEBUG)
	malloc_debug( 2 );
#endif

	num_descriptors		= 0;
	first_descriptor		= NULL;
	last_descriptor		= NULL;
	sysdata.NO_NAME_RESOLVING	= TRUE;
	sysdata.WAIT_FOR_AUTH	= TRUE;

	/*
	 * Init time.
	 */
	gettimeofday( &now_time, NULL );
	current_time = (time_t) now_time.tv_sec;
	/*  gettimeofday( &boot_time, NULL);   okay, so it's kludgy, sue me :) */
	boot_time = time(0);         /*  <-- I think this is what you wanted */
	strcpy( str_boot_time, ctime( &current_time ) );

	/*
	 * Init boot time.
	 */
	set_boot_time = &set_boot_time_struct;
	/*  set_boot_time->hour   = 6;
    set_boot_time->min    = 0;
    set_boot_time->sec    = 0;*/
	set_boot_time->manual = 0;

	new_boot_time = update_time(localtime(&current_time));
	/* Copies *new_boot_time to new_boot_struct, and then points
       new_boot_time to new_boot_struct again. -- Alty */
	new_boot_struct = *new_boot_time;
	new_boot_time = &new_boot_struct;
	new_boot_time->tm_mday += 1;
	if(new_boot_time->tm_hour > 12)
		new_boot_time->tm_mday += 1;
	new_boot_time->tm_sec = 0;
	new_boot_time->tm_min = 0;
	new_boot_time->tm_hour = 6;

	/* Update new_boot_time (due to day increment) */
	new_boot_time = update_time(new_boot_time);
	new_boot_struct = *new_boot_time;
	new_boot_time = &new_boot_struct;

	/* Set reboot time string for do_time */
	get_reboot_string();

	/*
	 * Reserve two channels for our use.
	 */
	if ( ( fpReserve = fopen( NULL_FILE, "r" ) ) == NULL )
	{
		perror( NULL_FILE );
		exit( 1 );
	}
	if ( ( fpLOG = fopen( NULL_FILE, "r" ) ) == NULL )
	{
		perror( NULL_FILE );
		exit( 1 );
	}

	/*
	 * Get the port number.
	 */
	port = 4000;
	if ( argc > 1 )
	{
		if ( !is_number( argv[1] ) )
		{
			fprintf( stderr, "Usage: %s [port #]\n", argv[0] );
			exit( 1 );
		}
		else if ( ( port = atoi( argv[1] ) ) <= 1024 )
		{
			fprintf( stderr, "Port number must be above 1024.\n" );
			exit( 1 );
		}
	}

	/*
	 * Run the game.
	 */
	log_string("Booting Database");
	boot_db( );
	log_string("Initializing socket");
	control  = init_socket( port   );
	sprintf( log_buf, "NEURO ready on port %d", port );
	log_string( log_buf );
	game_loop( );
	close( control  );
	/*
	 * That's all, folks.
	 */
	log_string( "Normal termination of game." );
	exit( 0 );
	return 0;
}

int closed( int d )
{
  return close(d);
}

int init_socket( int port )
{
	char hostname[64];
	struct sockaddr_in	 sa;
	struct hostent	*hp;
	struct servent	*sp;
	int x = 1;
	int fd;

	gethostname(hostname, sizeof(hostname));


	if ( ( fd = socket( AF_INET, SOCK_STREAM, 0 ) ) < 0 )
	{
		perror( "Init_socket: socket" );
		exit( 1 );
	}

	if ( setsockopt( fd, SOL_SOCKET, SO_REUSEADDR,
			(void *) &x, sizeof(x) ) < 0 )
	{
		perror( "Init_socket: SO_REUSEADDR" );
		close( fd );
		exit( 1 );
	}

#if defined(SO_DONTLINGER) && !defined(SYSV)
	{
		struct	linger	ld;

		ld.l_onoff  = 1;
		ld.l_linger = 1000;

		if ( setsockopt( fd, SOL_SOCKET, SO_DONTLINGER,
				(void *) &ld, sizeof(ld) ) < 0 )
		{
			perror( "Init_socket: SO_DONTLINGER" );
			close( fd );
			exit( 1 );
		}
	}
#endif

	hp = gethostbyname( hostname );
	sp = getservbyname( "service", "mud" );
	memset(&sa, '\0', sizeof(sa));
	sa.sin_family   = AF_INET; /* hp->h_addrtype; */
	sa.sin_port	    = htons( port );

	if ( bind( fd, (struct sockaddr *) &sa, sizeof(sa) ) == -1 )
	{
		perror( "Init_socket: bind" );
		close( fd );
		exit( 1 );
	}

	if ( listen( fd, 50 ) < 0 )
	{
		perror( "Init_socket: listen" );
		close( fd );
		exit( 1 );
	}

	return fd;
}

/*
static void SegVio()
{
  CHAR_DATA *ch;
  char buf[MAX_STRING_LENGTH];

  log_string( "SEGMENTATION VIOLATION" );
  log_string( lastplayercmd );
  for ( ch = first_char; ch; ch = ch->next )
  {
    sprintf( buf, "%cPC: %-20s room: %d", IS_NPC(ch) ? 'N' : ' ',
    		ch->name, ch->in_room->vnum );
    log_string( buf );
  }
  exit(0);
}
 */

/*
 * LAG alarm!							-Thoric

static void caught_alarm()
{
	char buf[MAX_STRING_LENGTH];
	bug( "ALARM CLOCK!" );
	strcpy( buf, "> the hideous malevalent entity known only as 'LAG' rises once more!\n\r" );
	echo_to_all( AT_IMMORT, buf, ECHOTAR_ALL );
	if ( newdesc )
	{
		FD_CLR( newdesc, &in_set );
		FD_CLR( newdesc, &out_set );
		log_string( "clearing newdesc" );
	}
	game_loop( );
	close( control );

	log_string( "Normal termination of game." );
	exit( 0 );
}
*/

/*
 * LAG alarm!							-Thoric
 */
static void caught_alarm()
{
	char buf[MAX_STRING_LENGTH];
	bug( "ALARM CLOCK!" );
	strcpy( buf, "> the hideous malevalent entity known only as 'LAG' rises once more!\n\r" );
	echo_to_all( AT_IMMORT, buf, ECHOTAR_ALL );
}

bool check_bad_desc( int desc )
{
	if ( FD_ISSET( desc, &exc_set ) )
	{
		FD_CLR( desc, &in_set );
		FD_CLR( desc, &out_set );
		log_string( "Bad FD caught and disposed." );
		return TRUE;
	}
	return FALSE;
}


void accept_new( int ctrl )
{
	static struct timeval null_time;
	DESCRIPTOR_DATA *d;
	/* int maxdesc; Moved up for use with id.c as extern */

#if defined(MALLOC_DEBUG)
	if ( malloc_verify( ) != 1 )
		abort( );
#endif

	/*
	 * Poll all active descriptors.
	 */
	FD_ZERO( &in_set  );
	FD_ZERO( &out_set );
	FD_ZERO( &exc_set );
	FD_SET( ctrl, &in_set );
	maxdesc	= ctrl;
	newdesc = 0;
	for ( d = first_descriptor; d; d = d->next )
	{
		maxdesc = UMAX( maxdesc, d->descriptor );
		FD_SET( d->descriptor, &in_set  );
		FD_SET( d->descriptor, &out_set );
		FD_SET( d->descriptor, &exc_set );
		if (d->auth_fd != -1)
		{
			maxdesc = UMAX( maxdesc, d->auth_fd );
			FD_SET(d->auth_fd, &in_set);
			if (IS_SET(d->auth_state, FLAG_WRAUTH))
				FD_SET(d->auth_fd, &out_set);
		}
		if ( d == last_descriptor )
			break;
	}

	if ( select( maxdesc+1, &in_set, &out_set, &exc_set, &null_time ) < 0 )
	{
		perror( "accept_new: select: poll" );
		exit( 1 );
	}

	if ( FD_ISSET( ctrl, &exc_set ) )
	{
		bug( "Exception raise on controlling descriptor %d", ctrl );
		FD_CLR( ctrl, &in_set );
		FD_CLR( ctrl, &out_set );
	}
	else
		if ( FD_ISSET( ctrl, &in_set ) )
		{
			newdesc = ctrl;
			new_descriptor( newdesc );
		}
}

void game_loop( )
{
	struct timeval	  last_time;
	char cmdline[MAX_INPUT_LENGTH];
	DESCRIPTOR_DATA *d;
//  AREA_DATA *pArea;

	/*  time_t	last_check = 0;  */

	signal( SIGPIPE, SIG_IGN );
    signal( SIGALRM, (void (*) (int) )caught_alarm );
	/* signal( SIGSEGV, SegVio ); */
	gettimeofday( &last_time, NULL );
	current_time = (time_t) last_time.tv_sec;

	/* Main loop */
	while ( !mud_down )
	{
		accept_new( control  );
		/*
		 * Kick out descriptors with raised exceptions
		 * or have been idle, then check for input.
		 */
		for ( d = first_descriptor; d; d = d_next )
		{
			if ( d == d->next )
			{
				bug( "descriptor_loop: loop found & fixed" );
				d->next = NULL;
			}
			d_next = d->next;

			d->idle++;	/* make it so a descriptor can idle out */
			if ( FD_ISSET( d->descriptor, &exc_set ) )
			{
				FD_CLR( d->descriptor, &in_set  );
				FD_CLR( d->descriptor, &out_set );
				if ( d->character
						&& ( d->connected == CON_PLAYING
								||   d->connected == CON_EDITING ) )
					save_char_obj( d->character );
				d->outtop	= 0;
				close_socket( d, TRUE );
				continue;
			}
			else
				if ( (!d->character && d->idle > 360)		  /* 2 mins */
						||   ( d->connected != CON_PLAYING && d->idle > 1200) /* 5 mins */
						||     d->idle > 28800 )				  /* 2 hrs  */
				{
					write_to_descriptor( d->descriptor,
							"> idle timeout - disconnecting\n\r", 0 );
					d->outtop	= 0;
					close_socket( d, TRUE );
					continue;
				}
				else
				{
					d->fcommand	= FALSE;

					if ( FD_ISSET( d->descriptor, &in_set ) )
					{
						d->idle = 0;
						if ( d->character )
							d->character->timer = 0;
						if ( !read_from_descriptor( d ) )
						{
							FD_CLR( d->descriptor, &out_set );
							if ( d->character
									&& ( d->connected == CON_PLAYING
											||   d->connected == CON_EDITING ) )
								save_char_obj( d->character );
							d->outtop	= 0;
							close_socket( d, FALSE );
							continue;
						}
					}

					/* IDENT authentication */
					if ( ( d->auth_fd == -1 ) && ( d->atimes < 20 )
							&& !str_cmp( d->user, "unknown" ) )
						start_auth( d );

					if ( d->auth_fd != -1)
					{
						if ( FD_ISSET( d->auth_fd, &in_set ) )
						{
							read_auth( d );
							/* if ( !d->auth_state )
			    check_ban( d );*/
						}
						else
							if ( FD_ISSET( d->auth_fd, &out_set )
									&& IS_SET( d->auth_state, FLAG_WRAUTH) )
							{
								send_auth( d );
								/* if ( !d->auth_state )
			  check_ban( d );*/
							}
					}
					if ( d->character && d->character->wait > 0 )
					{
						--d->character->wait;
						continue;
					}

					read_from_buffer( d );
					if ( d->incomm[0] != '\0' )
					{
						d->fcommand	= TRUE;
						stop_idling( d->character );

						strcpy( cmdline, d->incomm );
						d->incomm[0] = '\0';

						if ( d->character )
							set_cur_char( d->character );

						if ( d->pagepoint )
							set_pager_input(d, cmdline);
						else
							switch( d->connected )
							{
							default:
								nanny( d, cmdline );
								break;
							case CON_PLAYING:
								d->character->cmd_recurse = 0;
								interpret( d->character, cmdline );
								//substitute_alias( d, cmdline );
								break;
							case CON_EDITING:
								edit_buffer( d->character, cmdline );
								break;
							 case CON_BLACKJACK:
											do_blackjack( d->character, cmdline );
											break;
							}
					}
				}
			if ( d == last_descriptor )
				break;
		}

		/*
		 * Autonomous game motion.
		 */
		update_handler( );

		/*
		 * Check REQUESTS pipe
		 */
		check_requests( );

		/*
		 * Output.
		 */
		for ( d = first_descriptor; d; d = d_next )
		{
			d_next = d->next;

			if ( ( d->fcommand || d->outtop > 0 )
					&&   FD_ISSET(d->descriptor, &out_set) )
			{
				if ( d->pagepoint )
				{
					if ( !pager_output(d) )
					{
						if ( d->character
								&& ( d->connected == CON_PLAYING
										||   d->connected == CON_EDITING ) )
							save_char_obj( d->character );
						d->outtop = 0;
						close_socket(d, FALSE);
					}
				}
				else if ( !flush_buffer( d, TRUE ) )
				{
					if ( d->character
							&& ( d->connected == CON_PLAYING
									||   d->connected == CON_EDITING ) )
						save_char_obj( d->character );
					d->outtop	= 0;
					close_socket( d, FALSE );
				}
			}
			if ( d == last_descriptor )
				break;
		}

		/*
		 * Synchronize to a clock.
		 * Sleep( last_time + 1/PULSE_PER_SECOND - now ).
		 * Careful here of signed versus unsigned arithmetic.
		 */
		{
			struct timeval now_time;
			long secDelta;
			long usecDelta;

			gettimeofday( &now_time, NULL );
			usecDelta	= ((int) last_time.tv_usec) - ((int) now_time.tv_usec)
					+ 1000000 / PULSE_PER_SECOND;
			secDelta	= ((int) last_time.tv_sec ) - ((int) now_time.tv_sec );
			while ( usecDelta < 0 )
			{
				usecDelta += 1000000;
				secDelta  -= 1;
			}

			while ( usecDelta >= 1000000 )
			{
				usecDelta -= 1000000;
				secDelta  += 1;
			}

			if ( secDelta > 0 || ( secDelta == 0 && usecDelta > 0 ) )
			{
				struct timeval stall_time;

				stall_time.tv_usec = usecDelta;
				stall_time.tv_sec  = secDelta;
				if ( select( 0, NULL, NULL, NULL, &stall_time ) < 0 )
				{
					perror( "game_loop: select: stall" );
					exit( 1 );
				}
			}
		}

		gettimeofday( &last_time, NULL );
		current_time = (time_t) last_time.tv_sec;

		/* Check every 5 seconds...  (don't need it right now)
	if ( last_check+5 < current_time )
	{
	  CHECK_LINKS(first_descriptor, last_descriptor, next, prev,
	      DESCRIPTOR_DATA);
	  last_check = current_time;
	}
		 */
	}
	return;
}


void new_descriptor( int new_desc )
{
	char buf[MAX_STRING_LENGTH];
	DESCRIPTOR_DATA *dnew;
	//BAN_DATA *pban;
	struct hostent  *from;
	char *hostname;
	struct sockaddr_in sock;
	int desc;
	int size;

	set_alarm( 20 );
	size = sizeof(sock);
	if ( check_bad_desc( new_desc ) )
	{
		set_alarm( 0 );
		return;
	}
	set_alarm( 20 );
    if ( ( desc = accept( new_desc, (struct sockaddr *) &sock, (socklen_t *)&size) ) < 0 )
	{
		perror( "New_descriptormsg: accept");
		set_alarm( 0 );
		return;
	}
	if ( check_bad_desc( new_desc ) )
	{
		set_alarm( 0 );
		return;
	}
#if !defined(FNDELAY)
#define FNDELAY O_NDELAY
#endif

	set_alarm( 20 );
	if ( fcntl( desc, F_SETFL, FNDELAY ) == -1 )
	{
		perror( "New_descriptor: fcntl: FNDELAY" );
		set_alarm( 0 );
		return;
	}
	if ( check_bad_desc( new_desc ) )
		return;

	CREATE( dnew, DESCRIPTOR_DATA, 1 );
	dnew->next		= NULL;
	dnew->descriptor	= desc;
	dnew->mxp = FALSE;   /* Initially MXP is off */
	dnew->connected	= CON_GET_NAME;
	dnew->outsize	= 2000;
	dnew->idle		= 0;
	dnew->lines		= 0;
	dnew->scrlen	= 24;
	dnew->port		= ntohs( sock.sin_port );
	dnew->user 		= STRALLOC("unknown");
	dnew->auth_fd	= -1;
	dnew->auth_inc	= 0;
	dnew->auth_state	= 0;
	dnew->newstate	= 0;
	dnew->prevcolor	= 0x07;
	dnew->original      = NULL;
	dnew->character     = NULL;

	CREATE( dnew->outbuf, char, dnew->outsize );

	strcpy( buf, inet_ntoa( sock.sin_addr ) );
	sprintf( log_buf, "Sock.sinaddr:  %s, port %hd",
			buf, dnew->port );
	log_string_plus( log_buf, LOG_COMM );

	dnew->host = STRALLOC( buf );

	from = gethostbyaddr( (char *) &sock.sin_addr,
			sizeof(sock.sin_addr), AF_INET );
	hostname = STRALLOC( (char *)( from ? from->h_name : "") );


	if ( !sysdata.NO_NAME_RESOLVING )
	{
		STRFREE ( dnew->host);
		dnew->host = STRALLOC( (char *)( from ? from->h_name : buf) );
	}

	/*
	 * Init descriptor data.
	 */

	if ( !last_descriptor && first_descriptor )
	{
		DESCRIPTOR_DATA *d;

		bug( "New_descriptor: last_desc is NULL, but first_desc is not! ...fixing" );
		for ( d = first_descriptor; d; d = d->next )
			if ( !d->next )
				last_descriptor = d;
	}

	LINK( dnew, first_descriptor, last_descriptor, next, prev );

	/*
	 * Send the greeting.
	 */
	{
		extern char * help_greeting;
		if ( help_greeting[0] == '.' )
			write_to_buffer( dnew, help_greeting+1, 0 );
		else
			write_to_buffer( dnew, help_greeting  , 0 );
	}

	start_auth( dnew ); /* Start username authorization */

	if ( ++num_descriptors > sysdata.maxplayers )
		sysdata.maxplayers = num_descriptors;
	if ( sysdata.maxplayers > sysdata.alltimemax )
	{
		if ( sysdata.time_of_max )
			DISPOSE(sysdata.time_of_max);
		sprintf(buf, "%24.24s", ctime(&current_time));
		sysdata.time_of_max = str_dup(buf);
		sysdata.alltimemax = sysdata.maxplayers;
		sprintf( log_buf, "> broke all-time maximum player record: %d", sysdata.alltimemax );
		log_string_plus( log_buf, LOG_COMM );
		to_channel( log_buf, CHANNEL_MONITOR, "Monitor", 2 );
		save_sysdata( sysdata );
	}
	set_alarm(0);
	return;
}

void free_desc( DESCRIPTOR_DATA *d )
{
	close( d->descriptor );
	STRFREE( d->host );
	DISPOSE( d->outbuf );
	STRFREE( d->user );    /* identd */
	if ( d->pagebuf )
		DISPOSE( d->pagebuf );
	DISPOSE( d );
	--num_descriptors;
	return;
}

void close_socket( DESCRIPTOR_DATA *dclose, bool force )
{
	CHAR_DATA *ch;
	DESCRIPTOR_DATA *d;
	bool DoNotUnlink = FALSE;

	/* flush outbuf */
	if ( !force && dclose->outtop > 0 )
		flush_buffer( dclose, FALSE );

	/* say bye to whoever's snooping this descriptor */
	if ( dclose->snoop_by )
		write_to_buffer( dclose->snoop_by,
				"> your victim has left the game\n\r", 0 );

	/* stop snooping everyone else */
	for ( d = first_descriptor; d; d = d->next )
		if ( d->snoop_by == dclose )
			d->snoop_by = NULL;

	/* Check for switched people who go link-dead. -- Altrag */
	if ( dclose->original )
	{
		if ( ( ch = dclose->character ) != NULL )
			do_return(ch, "");
		else
		{
			bug( "Close_socket: dclose->original without character %s",
					(dclose->original->name ? dclose->original->name : "unknown") );
			dclose->character = dclose->original;
			dclose->original = NULL;
		}
	}

	ch = dclose->character;

	/* sanity check :( */
	if ( !dclose->prev && dclose != first_descriptor )
	{
		DESCRIPTOR_DATA *dp, *dn;
		bug( "Close_socket: %s desc:%p != first_desc:%p and desc->prev = NULL!",
				ch ? ch->name : d->host, dclose, first_descriptor );
		dp = NULL;
		for ( d = first_descriptor; d; d = dn )
		{
			dn = d->next;
			if ( d == dclose )
			{
				bug( "Close_socket: %s desc:%p found, prev should be:%p, fixing",
						ch ? ch->name : d->host, dclose, dp );
				dclose->prev = dp;
				break;
			}
			dp = d;
		}
		if ( !dclose->prev )
		{
			bug( "Close_socket: %s desc:%p could not be found!",
					ch ? ch->name : dclose->host, dclose );
			DoNotUnlink = TRUE;
		}
	}
	if ( !dclose->next && dclose != last_descriptor )
	{
		DESCRIPTOR_DATA *dp, *dn;
		bug( "Close_socket: %s desc:%p != last_desc:%p and desc->next = NULL!",
				ch ? ch->name : d->host, dclose, last_descriptor );
		dn = NULL;
		for ( d = last_descriptor; d; d = dp )
		{
			dp = d->prev;
			if ( d == dclose )
			{
				bug( "Close_socket: %s desc:%p found, next should be:%p, fixing",
						ch ? ch->name : d->host, dclose, dn );
				dclose->next = dn;
				break;
			}
			dn = d;
		}
		if ( !dclose->next )
		{
			bug( "Close_socket: %s desc:%p could not be found!",
					ch ? ch->name : dclose->host, dclose );
			DoNotUnlink = TRUE;
		}
	}

	if ( dclose->character )
	{
		sprintf( log_buf, "> closing link to %s", ch->name );
		log_string_plus( log_buf, LOG_COMM );
		if ( (dclose->connected == CON_PLAYING
				|| dclose->connected == CON_EDITING)
				||(dclose->connected >= CON_NOTE_TO
						&& dclose->connected <= CON_NOTE_FINISH) )
		{
			//act( AT_ACTION, "> $n has lost $s link", ch, NULL, NULL, TO_ROOM );

			//char bufText[MAX_STRING_LENGTH];
			//sprintf( bufText , "> %s has lost link" , ch->name );
			//echo_to_all( AT_LBLUE , bufText , ECHOTAR_ALL );

			ch->desc = NULL;
			//extract_char( ch, TRUE );

			//dclose->character->desc = NULL;
			//free_char( dclose->character );

		}
		else
		{
			/* clear descriptor pointer to get rid of bug message in log */
			dclose->character->desc = NULL;
			free_char( dclose->character );
		}
	}


	if ( !DoNotUnlink )
	{
		/* make sure loop doesn't get messed up */
		if ( d_next == dclose )
			d_next = d_next->next;
		UNLINK( dclose, first_descriptor, last_descriptor, next, prev );
	}

	if ( dclose->descriptor == maxdesc )
		--maxdesc;
	if ( dclose->auth_fd != -1 )
		close( dclose->auth_fd );

	free_desc( dclose );
	return;
}

int readd( int handle, char *buffer, int length )
{
  return read( handle, buffer, length );
}

bool read_from_descriptor( DESCRIPTOR_DATA *d )
{
    unsigned int iStart;

	/* Hold horses if pending command already. */
	if ( d->incomm[0] != '\0' )
		return TRUE;

	/* Check for overflow. */
	iStart = strlen(d->inbuf);
	if ( iStart >= sizeof(d->inbuf) - 10 )
	{
		sprintf( log_buf, "%s input overflow!", d->host );
		log_string( log_buf );
		write_to_descriptor( d->descriptor,
				"\n\r>>> SPAM REPORTED\n\r", 0 );
		return FALSE;
	}

	for ( ; ; )
	{
		int nRead;

		nRead = read( d->descriptor, d->inbuf + iStart,
				sizeof(d->inbuf) - 10 - iStart );
		if ( nRead > 0 )
		{
			iStart += nRead;
			if ( d->inbuf[iStart-1] == '\n' || d->inbuf[iStart-1] == '\r' )
				break;
		}
		else if ( nRead == 0 )
		{
			log_string_plus( "EOF encountered on read", LOG_COMM );
			return FALSE;
		}
		else if ( errno == EWOULDBLOCK )
			break;
		else
		{
			perror( "Read_from_descriptor" );
			return FALSE;
		}
	}

	d->inbuf[iStart] = '\0';
	return TRUE;
}



/*
 * Transfer one line from input buffer to input line.
 */
void read_from_buffer( DESCRIPTOR_DATA *d )
{
	int i, j, k;
	unsigned char * p;

	/*
	 * Hold horses if pending command already.
	 */
	if ( d->incomm[0] != '\0' )
		return;

	/*
	  Look for incoming telnet negotiation
	*/


	  for (p = d->inbuf; *p; p++)
	    if (*p == IAC)
	      {
	      if (memcmp (p, do_mxp_str, strlen (do_mxp_str)) == 0)
	        {
	        turn_on_mxp (d);
	        /* remove string from input buffer */
	        memmove (p, &p [strlen (do_mxp_str)], strlen (&p [strlen (do_mxp_str)]) + 1);
	        p--; /* adjust to allow for discarded bytes */
	        } /* end of turning on MXP */
	      else  if (memcmp (p, dont_mxp_str, strlen (dont_mxp_str)) == 0)
	        {
	        d->mxp = FALSE;
	        /* remove string from input buffer */
	        memmove (p, &p [strlen (dont_mxp_str)], strlen (&p [strlen (dont_mxp_str)]) + 1);
	        p--; /* adjust to allow for discarded bytes */
	        } /* end of turning off MXP */
	      } /* end of finding an IAC */


	/*
	 * Look for at least one new line.
	 */
	for ( i = 0; d->inbuf[i] != '\n' && d->inbuf[i] != '\r' && i<MAX_INBUF_SIZE;
			i++ )
	{
		if ( d->inbuf[i] == '\0' )
			return;
	}

	/*
	 * Canonical input processing.
	 */
	for ( i = 0, k = 0; d->inbuf[i] != '\n' && d->inbuf[i] != '\r'; i++ )
	{
		if ( k >= 254 )
		{
			write_to_descriptor( d->descriptor, "Line too long\n\r", 0 );

			/* skip the rest of the line */
			/*
	    for ( ; d->inbuf[i] != '\0' || i>= MAX_INBUF_SIZE ; i++ )
	    {
		if ( d->inbuf[i] == '\n' || d->inbuf[i] == '\r' )
		    break;
	    }
			 */
			d->inbuf[i]   = '\n';
			d->inbuf[i+1] = '\0';
			break;
		}

		if ( d->inbuf[i] == '\b' && k > 0 )
			--k;
		else if ( isascii(d->inbuf[i]) && isprint(d->inbuf[i]) )
			d->incomm[k++] = d->inbuf[i];
	}

	/*
	 * Finish off the line.
	 */
	if ( k == 0 )
		d->incomm[k++] = ' ';
	d->incomm[k] = '\0';

	/*
	 * Deal with bozos with #repeat 1000 ...
	 */
	if ( k > 1 || d->incomm[0] == '!' )
	{
		if ( d->incomm[0] != '!' && strcmp( d->incomm, d->inlast ) )
		{
			d->repeat = 0;
		}
		else
		{
	    if ( ++d->repeat >= 100 )
			{
				/*		sprintf( log_buf, "%s input spamming!", d->host );
		log_string( log_buf );
				 */
				write_to_descriptor( d->descriptor,
						"\n\r>>> SPAM REPORTED\n\r", 0 );
			}
		}
	}

	/*
	 * Do '!' substitution.
	 */
	if ( d->incomm[0] == '!' )
		strcpy( d->incomm, d->inlast );
	else
		strcpy( d->inlast, d->incomm );

	/*
	 * Shift the input buffer.
	 */
	while ( d->inbuf[i] == '\n' || d->inbuf[i] == '\r' )
		i++;
	for ( j = 0; ( d->inbuf[j] = d->inbuf[i+j] ) != '\0'; j++ )
		;
	return;
}



/*
 * Low level output function.
 */
bool flush_buffer( DESCRIPTOR_DATA *d, bool fPrompt )
{
	char buf[MAX_INPUT_LENGTH];
	extern bool mud_down;
	CHAR_DATA *ch;

	ch = d->original ? d->original : d->character;
	if( ch && ch->fighting && ch->fighting->who )
		show_condition( ch, ch->fighting->who );

	/*
	 * If buffer has more than 4K inside, spit out .5K at a time   -Thoric
	 */
	if ( !mud_down && d->outtop > 4096 )
	{
		memcpy( buf, d->outbuf, 512 );
		memmove( d->outbuf, d->outbuf + 512, d->outtop - 512 );
		d->outtop -= 512;
		if ( d->snoop_by )
		{
			char snoopbuf[MAX_INPUT_LENGTH];

			buf[512] = '\0';
			if ( d->character && d->character->name )
			{
				if (d->original && d->original->name)
					sprintf( snoopbuf, "%s (%s)", d->character->name, d->original->name );
				else
					sprintf( snoopbuf, "%s", d->character->name);
				write_to_buffer( d->snoop_by, snoopbuf, 0);
			}
			write_to_buffer( d->snoop_by, "% ", 2 );
			write_to_buffer( d->snoop_by, buf, 0 );
		}
		if ( !write_to_descriptor( d->descriptor, buf, 512 ) )
		{
			d->outtop = 0;
			return FALSE;
		}
		return TRUE;
	}


	/*
	 * Bust a prompt.
	 */
	if ( fPrompt && !mud_down && d->connected == CON_PLAYING )
	{
		ch = d->original ? d->original : d->character;
		if ( IS_SET(ch->act, PLR_BLANK) )
			write_to_buffer( d, "\n\r", 2 );

		if ( IS_SET(ch->act, PLR_PROMPT) )
			display_prompt(d);
		if ( IS_SET(ch->act, PLR_TELNET_GA) )
			write_to_buffer( d, go_ahead_str, 0 );
	}

	/*
	 * Short-circuit if nothing to write.
	 */
	if ( d->outtop == 0 )
		return TRUE;

	/*
	 * Snoop-o-rama.
	 */
	if ( d->snoop_by )
	{
		/* without check, 'force mortal quit' while snooped caused crash, -h */
		if ( d->character && d->character->name )
		{
			/* Show original snooped names. -- Altrag */
			if ( d->original && d->original->name )
				sprintf( buf, "%s (%s)", d->character->name, d->original->name );
			else
				sprintf( buf, "%s", d->character->name);
			write_to_buffer( d->snoop_by, buf, 0);
		}
		write_to_buffer( d->snoop_by, "% ", 2 );
		write_to_buffer( d->snoop_by, d->outbuf, d->outtop );
	}

	/*
	 * OS-dependent output.
	 */
	if ( !write_to_descriptor( d->descriptor, d->outbuf, d->outtop ) )
	{
		d->outtop = 0;
		return FALSE;
	}
	else
	{
		d->outtop = 0;
		return TRUE;
	}
}



/*
 * Append onto an output buffer.
 */
void write_to_buffer( DESCRIPTOR_DATA *d, const char *txt, int length )
{

	int origlength;

	if ( !d )
	{
		bug( "Write_to_buffer: NULL descriptor" );
		return;
	}

	/*
	 * Normally a bug... but can happen if loadup is used.
	 */
	if ( !d->outbuf )
		return;

	/*
	 * Find length in case caller didn't.
	 */
	if ( length <= 0 )
		length = strlen(txt);

	/* Uncomment if debugging or something
    if ( length != strlen(txt) )
    {
	bug( "Write_to_buffer: length(%d) != strlen(txt)!", length );
	length = strlen(txt);
    }
	 */

	  origlength = length;
	  /* work out how much we need to expand/contract it */
	  length += count_mxp_tags (d->mxp, txt, length);

	/*
	 * Initial \n\r if needed.
	 */
	if ( d->outtop == 0 && !d->fcommand )
	{
		d->outbuf[0]	= '\n';
		d->outbuf[1]	= '\r';
		d->outtop	= 2;
	}

	/*
	 * Expand the buffer as needed.
	 */
    while ( d->outtop + (unsigned int ) length >= d->outsize )
	{
		if (d->outsize > 32000)
		{
			/* empty buffer */
			d->outtop = 0;
			close_socket(d, TRUE);
			bug("Buffer overflow. Closing (%s)", d->character ? d->character->name : "???" );
			return;
		}
		d->outsize *= 2;
		RECREATE( d->outbuf, char, d->outsize );
	}

	/*
	 * Copy.
	 */
	strncpy( d->outbuf + d->outtop, txt, length );
	convert_mxp_tags (d->mxp, d->outbuf + d->outtop, txt, origlength );
	d->outtop += length;
	d->outbuf[d->outtop] = '\0';
	return;
}


/*
 * Lowest level output function.
 * Write a block of text to the file descriptor.
 * If this gives errors on very long blocks (like 'ofind all'),
 *   try lowering the max block size.
 */
bool write_to_descriptor( int desc, char *txt, int length )
{
	int iStart;
	int nWrite;
	int nBlock;

	if ( length <= 0 )
		length = strlen(txt);

	for ( iStart = 0; iStart < length; iStart += nWrite )
	{
		nBlock = UMIN( length - iStart, 4096 );
		if ( ( nWrite = write( desc, txt + iStart, nBlock ) ) < 0 )
		{ perror( "Write_to_descriptor" ); return FALSE; }
	}

	return TRUE;
}



void show_title( DESCRIPTOR_DATA *d )
{


	//    CHAR_DATA *ch;

	//    ch = d->character;

	//    if ( !IS_SET( ch->pcdata->flags, PCFLAG_NOINTRO ) )
	//    {
	//	if (IS_SET(ch->act, PLR_ANSI))
	//	  send_ansi_title(ch);
	//	else
	//	  send_ascii_title(ch);
	//    }
	//    else
	//    {
	write_to_buffer( d, "> press [ENTER]\n\r", 0 );
	//    }

	d->connected = CON_PRESS_ENTER;

}

/*
 * Deal with sockets that haven't logged in yet.
 */
void nanny( DESCRIPTOR_DATA *d, char *argument )
{
	char buf[MAX_STRING_LENGTH];
	CHAR_DATA *ch;
	//CLAN_DATA *clan;
	char *pwdnew;
	char *p;
	//int sn;
	int i;
	BAN_DATA *pban;
	bool fOld, chk;
	OBJ_DATA *obj;

	if( d->connected != CON_NOTE_TEXT )
	{
		while ( isspace(*argument) )
			argument++;
	}

	while ( isspace(*argument) )
		argument++;

	ch = d->character;

	switch ( d->connected )
	{

	default:
		bug( "Nanny: bad d->connected %d", d->connected );
		close_socket( d, TRUE );
		return;

	case CON_GET_NAME:
		if ( argument[0] == '\0' )
		{
			close_socket( d, FALSE );
			return;
		}

		for(i = 0; argument[ i ]; i++)
			argument[i] = tolower(argument[ i ]);

		argument[0] = UPPER(argument[0]);
		if ( !check_parse_name( argument ) )
		{
			write_to_buffer( d, "> illegal name - try again\n\rnickname: ", 0 );
			return;
		}

		if ( !str_cmp( argument, "New" ) )
		{
			if (d->newstate == 0)
			{
				/* New player */
				/* Don't allow new players if DENY_NEW_PLAYERS is true */
				if (sysdata.DENY_NEW_PLAYERS == TRUE)
				{
					sprintf( buf, "> the game is currently preparing for a reboot\n\r" );
					write_to_buffer( d, buf, 0 );
					sprintf( buf, "> new users are not accepted during this time\n\r" );
					write_to_buffer( d, buf, 0 );
					sprintf( buf, "> please try again in a few minutes\n\r" );
					write_to_buffer( d, buf, 0 );
					close_socket( d, FALSE );
				}
				for ( pban = first_ban; pban; pban = pban->next )
				{
					if (
							( !str_prefix( pban->name, d->host )
									|| !str_suffix( pban->name, d->host ) ) )
					{
						write_to_buffer( d,
								"> your site has been banned from this game\n\r", 0 );
						close_socket( d, FALSE );
						return;
					}
				}
				for ( pban = first_tban; pban; pban = pban->next )
				{
					if (
							( !str_prefix( pban->name, d->host )
									|| !str_suffix( pban->name, d->host ) ) )
					{
						write_to_buffer( d,
								"> new players have been temporarily banned from your IP\n\r", 0 );
						close_socket( d, FALSE );
						return;
					}
				}
				sprintf( buf, "\n\r> choosing a name is one of the most important parts of this game\n\r"
						"> make sure to pick a name appropriate to the character you are going\n\r"
						"> to role-play, and be sure that it suits our theme\n\r"
						"> if the name you select is not acceptable, you will be asked to choose\n\r"
						"> another one\n\r\n\r>please choose a nickname for your character: ");

				write_to_buffer( d, buf, 0 );
				d->newstate++;
				d->connected = CON_GET_NAME;
				return;
			}
			else
			{
				write_to_buffer(d, "> invalid name - try again\n\rnickname: ", 0);
				return;
			}
		}

		if ( check_playing( d, argument, FALSE ) == BERR )
		{
			write_to_buffer( d, "> nickname: ", 0 );
			return;
		}

		fOld = load_char_obj( d, argument, TRUE );
		if ( !d->character )
		{
			sprintf( log_buf, "Bad player file %s@%s", argument, d->host );
			log_string( log_buf );
			write_to_buffer( d, "> your user file is corrupt >>> please notify gevrik@gmail.com\n\r", 0 );
			close_socket( d, FALSE );
			return;
		}
		ch   = d->character;

		for ( pban = first_ban; pban; pban = pban->next )
		{
			if (
					( !str_prefix( pban->name, d->host )
							|| !str_suffix( pban->name, d->host ) )
							&& pban->level >= ch->top_level )
			{
				write_to_buffer( d,
						"> your site has been banned from this game\n\r", 0 );
				close_socket( d, FALSE );
				return;
			}
		}
		if ( IS_SET(ch->act, PLR_DENY) )
		{
			sprintf( log_buf, "> denying access to %s@%s", argument, d->host );
			log_string_plus( log_buf, LOG_COMM );
			if (d->newstate != 0)
			{
				write_to_buffer( d, "> that name is already taken - choose again: ", 0 );
				d->connected = CON_GET_NAME;
				return;
			}
			write_to_buffer( d, "> you are denied access\n\r", 0 );
			close_socket( d, FALSE );
			return;
		}
		for ( pban = first_tban; pban; pban = pban->next )
		{
			if (
					( !str_prefix( pban->name, d->host )
							|| !str_suffix( pban->name, d->host ) )
							&& ch->top_level == 0 )
			{
				write_to_buffer( d,
						"> you have been temporarily banned from creating new characters\n\r", 0 );
				close_socket( d, FALSE );
				return;
			}
		}

		chk = check_reconnect( d, argument, FALSE );
		if ( chk == BERR )
		{
			close_socket( d, FALSE);
			return;
		}


		if ( chk )
		{
			fOld = TRUE;
		}
		else
		{
			if ( wizlock && !IS_IMMORTAL(ch) )
			{
				write_to_buffer( d, "> the game is locked - only admins can connect now\n\r", 0 );
				write_to_buffer( d, "> please try again later\n\r", 0 );
				close_socket( d, FALSE );
				return;
			}
		}

		if ( fOld )
		{
			if (d->newstate != 0)
			{
				write_to_buffer( d, "> that name is already taken - choose another: ", 0 );
				d->connected = CON_GET_NAME;
				return;
			}
			/* Old player */
			write_to_buffer( d, "> password: ", 0 );
			write_to_buffer( d, echo_off_str, 0 );
			d->connected = CON_GET_OLD_PASSWORD;
			return;
		}
		else
		{
			write_to_buffer( d, "\n\r> you are a new user\n\r", 0 );
			sprintf( buf, "> desired name: %s [y/n] ", argument );
			write_to_buffer( d, buf, 0 );
			d->connected = CON_CONFIRM_NEW_NAME;
			return;
		}
		break;

	case CON_GET_OLD_PASSWORD:
		write_to_buffer( d, "\n\r", 2 );

		if ( strcmp( crypt( argument, ch->pcdata->pwd ), ch->pcdata->pwd ) )
		{
			write_to_buffer( d, "> incorrect password\n\r", 0 );
			/* clear descriptor pointer to get rid of bug message in log */
			d->character->desc = NULL;
			close_socket( d, FALSE );
			return;
		}

		write_to_buffer( d, echo_on_str, 0 );

		if ( check_playing( d, ch->name, TRUE ) )
			return;

		chk = check_reconnect( d, ch->name, TRUE );
		if ( chk == BERR )
		{
			if ( d->character && d->character->desc )
				d->character->desc = NULL;
			close_socket( d, FALSE );
			return;
		}
		if ( chk == TRUE )
			return;

		if ( check_multi( d , ch->name  ) )
		{
			close_socket( d, FALSE );
			return;
		}

		sprintf( buf, "%s", ch->name );
		d->character->desc = NULL;
		free_char( d->character );
		fOld = load_char_obj( d, buf, FALSE );
		ch = d->character;
		sprintf( log_buf, "%s@%s(%s) has connected", ch->name, d->host,
				d->user );
		log_string_plus( log_buf, LOG_COMM );
		show_title(d);
		//ch->pcdata->quest_curr = ch->pcdata->queststatus;
		if ( ch->pcdata->area )
			do_loadarea (ch , "" );


		break;

	case CON_CONFIRM_NEW_NAME:
		switch ( *argument )
		{
		case 'y': case 'Y':
			sprintf( buf, "\n\r> pick a good password"
					"\n\r> password for %s: %s",
					ch->name, echo_off_str );
			write_to_buffer( d, buf, 0 );
			d->connected = CON_GET_NEW_PASSWORD;
			break;

		case 'n': case 'N':
			write_to_buffer( d, "> ok - what IS it then? ", 0 );
			/* clear descriptor pointer to get rid of bug message in log */
			d->character->desc = NULL;
			free_char( d->character );
			d->character = NULL;
			d->connected = CON_GET_NAME;
			break;

		default:
			write_to_buffer( d, "> please type yes or no ", 0 );
			break;
		}
		break;

		case CON_GET_NEW_PASSWORD:
			write_to_buffer( d, "\n\r", 2 );

			if ( strlen(argument) < 4 )
			{
				write_to_buffer( d,
						"> password must be at least four characters long\n\r> password: ",
						0 );
				return;
			}

			pwdnew = crypt( argument, ch->name );
			for ( p = pwdnew; *p != '\0'; p++ )
			{
				if ( *p == '~' )
				{
					write_to_buffer( d,
							"> new password not acceptable - try again\n\r> password: ",
							0 );
					return;
				}
			}

			DISPOSE( ch->pcdata->pwd );
			ch->pcdata->pwd	= str_dup( pwdnew );
			write_to_buffer( d, "\n\r> please retype the password to confirm: ", 0 );
			d->connected = CON_CONFIRM_NEW_PASSWORD;
			break;

		case CON_CONFIRM_NEW_PASSWORD:
			write_to_buffer( d, "\n\r", 2 );

			if ( strcmp( crypt( argument, ch->pcdata->pwd ), ch->pcdata->pwd ) )
			{
				write_to_buffer( d, "> passwords do not match\n\r> retype password: ",
						0 );
				d->connected = CON_GET_NEW_PASSWORD;
				return;
			}

			write_to_buffer( d, echo_on_str, 0 );
			write_to_buffer( d, "\n\r> what is your gender [m/f/n] ", 0 );
			d->connected = CON_GET_NEW_SEX;
			break;

		case CON_GET_NEW_SEX:
			switch ( argument[0] )
			{
			case 'm': case 'M': ch->sex = SEX_MALE;    break;
			case 'f': case 'F': ch->sex = SEX_FEMALE;  break;
			case 'n': case 'N': ch->sex = SEX_NEUTRAL; break;
			default:
				write_to_buffer( d, "> that is not a gender\n\r> what is your gender: ", 0 );
				return;
			}

			//d->connected = CON_ADD_SKILLS;
			ch->pcdata->num_skills = 0;
			ch->pcdata->adept_skills = 0;
			ch->perm_frc = number_range(-2000, 20);
			//break;

			//case CON_ADD_SKILLS:

			ch->pcdata->learned[gsn_survey] = 50;
			ch->pcdata->learned[gsn_landscape] = 50;
			ch->pcdata->learned[gsn_construction] = 50;
			ch->pcdata->learned[gsn_bridge] = 50;
			ch->pcdata->learned[gsn_codeapp] = 50;
			ch->pcdata->learned[gsn_spacecraft] = 50;
			ch->pcdata->num_skills = 6;
			sprintf( buf, "%s",ch->name );
			set_title( ch, buf );

			ch->perm_str = 10;
			ch->perm_int = 10;
			ch->perm_wis = 10;
			ch->perm_dex = 10;
			ch->perm_con = 10;
			ch->perm_cha = 10;


			//d->connected = CON_GET_WANT_RIPANSI;
			//break;

			//case CON_GET_WANT_RIPANSI:

			SET_BIT(ch->act,PLR_ANSI);

			sprintf( log_buf, "%s@%s new character", ch->name, d->host);
			log_string_plus( log_buf, LOG_COMM);
			to_channel( log_buf, CHANNEL_MONITOR, "Monitor", 2 );

			char logbufText[MAX_STRING_LENGTH];
			sprintf( logbufText , "> a new runner has connected: %s" , ch->name );
			echo_to_all( AT_BLUE , logbufText , ECHOTAR_ALL );

			//show_title(d);
			ch->top_level = 0;
			ch->position = POS_STANDING;
			write_to_buffer( d, "> registration complete\n\r", 0 );
			write_to_buffer( d, "> press [ENTER]\n\r", 0 );
			d->connected = CON_PRESS_ENTER;
			return;
			break;

			case CON_PRESS_ENTER:
				write_to_buffer( d, "\n\r> message of the day:\n\r", 0 );
				do_help( ch, "motd" );

				  /* telnet negotiation to see if they support MXP */

				    write_to_buffer( d, will_mxp_str, 0 );

				//if ( strcmp(ch->pcdata->clan, "Turing") )

				write_to_buffer( d, "\n\r> press [ENTER]\n\r", 0 );
				d->connected = CON_DONE_MOTD;
				break;

			case CON_READ_IMOTD:
				write_to_buffer( d, "\n\r> ai message of the day:\n\r", 0 );
				do_help( ch, "imotd" );
				d->connected = CON_DONE_MOTD;
				break;

			case CON_READ_NMOTD:
				do_help( ch, "nmotd" );
				d->connected = CON_DONE_MOTD;
				break;

			case CON_DONE_MOTD:
				write_to_buffer( d, "> logged into cyberspace\n\r\n\r", 0 );
				add_char( ch );
				d->connected	= CON_PLAYING;


				if ( ch->top_level == 0 )
				{

					ch->gold = 10000;

					ch->perm_lck = number_range(6, 18);
					ch->perm_frc = URANGE( 0 , ch->perm_frc , 20 );

					ch->top_level = 1;
					ch->hit	= ch->max_hit;
					ch->move	= ch->max_move;
					if ( ch->perm_frc > 0 )
						ch->max_mana = 100 + 100*ch->perm_frc;
					else
						ch->max_mana = 0;
					ch->mana	= ch->max_mana;

					/* Added by Narn.  Start new characters with autoexit and autgold
               already turned on.  Very few people don't use those. */
					SET_BIT( ch->act, PLR_AUTOGOLD );
					SET_BIT( ch->act, PLR_AUTOEXIT );
					//SET_BIT( ch->act, PLR_AUTOMAP );

					SET_BIT (ch->pcdata->cyber, CYBER_REACTOR );

					obj = create_object( get_obj_index(OBJ_VNUM_SCHOOL_DAGGER), 0 );
					obj_to_char( obj, ch );
					equip_char( ch, obj, WEAR_WIELD );

					ch->pcdata->serverrevision = 2;

					//obj = create_object( get_obj_index(OBJ_VNUM_LIGHT), 0 );
					//obj_to_char( obj, ch );

					/* comlink */

					{
						OBJ_INDEX_DATA *obj_ind = get_obj_index( 23 );

						if ( obj_ind != NULL )
						{
							obj = create_object( obj_ind, 0 );
							obj_to_char( obj, ch );
						}
					}

					if (!sysdata.WAIT_FOR_AUTH)
					{
						char_to_room( ch, get_room_index( ROOM_VNUM_SCHOOL ) );
						ch->pcdata->auth_state = 3;
					}
					else
					{
						char_to_room( ch, get_room_index( ROOM_VNUM_SCHOOL ) );
						ch->pcdata->auth_state = 1;
						SET_BIT(ch->pcdata->flags, PCFLAG_UNAUTHED);
					}
					/* Display_prompt interprets blank as default */
					ch->pcdata->prompt = STRALLOC("");
				}
				else if ( ch->in_room && !IS_IMMORTAL( ch ) )
				{
					char_to_room( ch, ch->in_room );
				}
				else
				{
					char_to_room( ch, get_room_index( wherehome(ch) ) );
				}

				if ( get_timer( ch, TIMER_SHOVEDRAG ) > 0 )
					remove_timer( ch, TIMER_SHOVEDRAG );

				if ( get_timer( ch, TIMER_PKILLED ) > 0 )
					remove_timer( ch, TIMER_PKILLED );

			    for ( d = first_descriptor; d; d = d->next )
			    {
			        CHAR_DATA *vch;
			        NOTIFY_DATA *temp;

				temp = NULL;
			        vch = d->character;

			        if ( d->connected == CON_PLAYING   &&   vch != ch)
			 	{
				    for(temp = vch->pcdata->first_notify; temp; temp = temp->next)
				    {
				        if (on_notify(vch, ch) == TRUE  && temp->name == ch->name )
					{
					 set_char_color(AT_NOTIFY,vch);
					 ch_printf(vch,"> %s has entered cyberspace\n\r",temp->name);
					 break;
					}
				    }
				}
			    }

				// server revisions

				if ( ch->pcdata->serverrevision < 2)
				{
					int currqtaxnodes = ch->pcdata->qtaxnodes;
					int revcompensation = (500 * currqtaxnodes) + 25000;


					if ( revcompensation > 250000 )
						revcompensation = 250000;

					CLAN_DATA *clan;

					ch->pcdata->bank += revcompensation;

					if ( ch->plr_home != NULL )
					{
					ch->plr_home = NULL;
					}

					clan =  ch->pcdata->clan;
			        if ( clan != NULL )
			        {
			            ch->pcdata->clan = NULL;
			            STRFREE(ch->pcdata->clan_name);
			            ch->pcdata->clan_name = STRALLOC( "" );
			            DISPOSE( ch->pcdata->bestowments );
			            ch->pcdata->bestowments = str_dup("");
			        }

		            ch->pcdata->qtaxnodes = 0;
					ch->pcdata->serverrevision = 2;

					char_from_room( ch );
					char_to_room( ch, get_room_index( 33 ) );
					SET_BIT( ch->act, PLR_AUTOMAP );

					save_char_obj( ch );

				}

				if ( ch->plr_home != NULL )
				{
					char filename[256];
					FILE *fph;
					ROOM_INDEX_DATA *storeroom = ch->plr_home;
					OBJ_DATA *obj;
					OBJ_DATA *obj_next;

					for ( obj = storeroom->first_content; obj; obj = obj_next )
					{
						obj_next = obj->next_content;
						extract_obj( obj );
					}

					sprintf( filename, "%s%c/%s.home", PLAYER_DIR, tolower(ch->name[0]),
							ch->name );
					if ( ( fph = fopen( filename, "r" ) ) != NULL )
					{
						int iNest;
						bool found;
						OBJ_DATA *tobj, *tobj_next;

						rset_supermob(storeroom);
						for ( iNest = 0; iNest < MAX_NEST; iNest++ )
							rgObjNest[iNest] = NULL;

						found = TRUE;
						for ( ; ; )
						{
							char letter;
							char *word;

							letter = fread_letter( fph );
							if ( letter == '*' )
							{
								fread_to_eol( fph );
								continue;
							}

							if ( letter != '#' )
							{
								bug( "Load_plr_home: # not found", 0 );
								bug( ch->name, 0 );
								break;
							}

							word = fread_word( fph );
							if ( !str_cmp( word, "OBJECT" ) )	/* Objects	*/
								fread_obj  ( supermob, fph, OS_CARRY );
							else
								if ( !str_cmp( word, "END"    ) )	/* Done		*/
									break;
								else
								{
									bug( "Load_plr_home: bad section", 0 );
									bug( ch->name, 0 );
									break;
								}
						}

						fclose( fph );

						for ( tobj = supermob->first_carrying; tobj; tobj = tobj_next )
						{
							tobj_next = tobj->next_content;
							obj_from_char( tobj );
							obj_to_room( tobj, storeroom );
						}

						release_supermob();

					}
				}

				//char bufText[MAX_STRING_LENGTH];
				act( AT_ACTION, "> $n has entered the node", ch, NULL, NULL, TO_ROOM );
				//sprintf( bufText , "> %s has logged in" , ch->name );
				//echo_to_all( AT_LBLUE , bufText , ECHOTAR_ALL );
				//do_global_boards( ch, "" );
				do_look( ch, "auto" );
				ch->pcdata->board = &boards[DEFAULT_BOARD];
				mail_count(ch);
				break;

			case CON_NOTE_TO:
				handle_con_note_to (d, argument);
				break;

			case CON_NOTE_SUBJECT:
				handle_con_note_subject (d, argument);
				break; /* subject */

			case CON_NOTE_EXPIRE:
				handle_con_note_expire (d, argument);
				break;

			case CON_NOTE_TEXT:
				handle_con_note_text (d, argument);
				break;

			case CON_NOTE_FINISH:
				handle_con_note_finish (d, argument);
				break;

	}
	return;
}



/*
 * Parse a name for acceptability.
 */
bool check_parse_name( char *name )
{
	/*
	 * Reserved words.
	 */
	if ( is_name( name, "all auto someone immortal self god supreme demigod dog guard cityguard cat cornholio spock hicaine hithoric death ass fuck shit piss crap quit" ) )
		return FALSE;
	if ( is_name( name, "luke darth vader skywalker han solo liea leia emporer palpatine chewie chewbacca lando anakin boba fett obiwan kenobi durga" ) )
		return FALSE;

	/*
	 * Length restrictions.
	 */
	if ( strlen(name) <  3 )
		return FALSE;

	if ( strlen(name) > 12 )
		return FALSE;

	/*
	 * Alphanumerics only.
	 * Lock out IllIll twits.
	 */
	{
		char *pc;
		bool fIll;

		fIll = TRUE;
		for ( pc = name; *pc != '\0'; pc++ )
		{
			if ( !isalpha(*pc) )
				return FALSE;
			if ( LOWER(*pc) != 'i' && LOWER(*pc) != 'l' )
				fIll = FALSE;
		}

		if ( fIll )
			return FALSE;
	}

	/*
	 * Code that followed here used to prevent players from naming
	 * themselves after mobs... this caused much havoc when new areas
	 * would go in...
	 */

	return TRUE;
}



/*
 * Look for link-dead player to reconnect.
 */
bool check_reconnect( DESCRIPTOR_DATA *d, char *name, bool fConn )
{
	CHAR_DATA *ch;

	for ( ch = first_char; ch; ch = ch->next )
	{
		if ( !IS_NPC(ch)
				&& ( !fConn || !ch->desc )
				&&    ch->name
				&&   !str_cmp( name, ch->name ) )
		{
			if ( fConn && ch->switched )
			{
				write_to_buffer( d, "> already playing\n\r> name: ", 0 );
				d->connected = CON_GET_NAME;
				if ( d->character )
				{
					/* clear descriptor pointer to get rid of bug message in log */
					d->character->desc = NULL;
					free_char( d->character );
					d->character = NULL;
				}
				return BERR;
			}
			if ( fConn == FALSE )
			{
				DISPOSE( d->character->pcdata->pwd );
				d->character->pcdata->pwd = str_dup( ch->pcdata->pwd );
			}
			else
			{
				/* clear descriptor pointer to get rid of bug message in log */
				d->character->desc = NULL;
				free_char( d->character );
				d->character = ch;
				ch->desc	 = d;
				ch->timer	 = 0;
				send_to_char( "> reconnecting\n\r", ch );
				act( AT_ACTION, "> $n has reconnected", ch, NULL, NULL, TO_ROOM );
				sprintf( log_buf, "%s@%s(%s) reconnected", ch->name, d->host, d->user );
				log_string_plus( log_buf, LOG_COMM );
				d->connected = CON_PLAYING;
			}
			return TRUE;
		}
	}

	return FALSE;
}



/*
 * Check if already playing.
 */

bool check_multi( DESCRIPTOR_DATA *d , char *name )
{
	DESCRIPTOR_DATA *dold;

	for ( dold = first_descriptor; dold; dold = dold->next )
	{
		if ( dold != d
				&& (  dold->character || dold->original )
				&&   str_cmp( name, dold->original
						? dold->original->name : dold->character->name )
						&& !str_cmp(dold->host , d->host ) )
		{
			const char *ok = "86.9.160.241";
			const char *ok2 = "86.9.160.241";
			int iloop;

			for ( iloop = 0 ; iloop < 11 ; iloop++ )
			{
				if ( ok[iloop] != d->host[iloop] )
					break;
			}
			if ( iloop >= 10 )
				return FALSE;
			for ( iloop = 0 ; iloop < 11 ; iloop++ )
			{
				if ( ok2[iloop] != d->host[iloop] )
					break;
			}
			if ( iloop >= 10 )
				return FALSE;
			return FALSE;
			write_to_buffer( d, "> sorry multi-playing is not allowed ... have you other character quit first\n\r", 0 );
			sprintf( log_buf, "%s attempting to multiplay with %s", dold->original ? dold->original->name : dold->character->name , d->character->name );
			log_string_plus( log_buf, LOG_COMM );
			d->character = NULL;
			free_char( d->character );
			return TRUE;
		}
	}

	return FALSE;

}

bool check_playing( DESCRIPTOR_DATA *d, char *name, bool kick )
{
	CHAR_DATA *ch;

	DESCRIPTOR_DATA *dold;
	int	cstate;

	for ( dold = first_descriptor; dold; dold = dold->next )
	{
		if ( dold != d
				&& (  dold->character || dold->original )
				&&   !str_cmp( name, dold->original
						? dold->original->name : dold->character->name ) )
		{
			cstate = dold->connected;
			ch = dold->original ? dold->original : dold->character;
			if ( !ch->name
					|| ( cstate != CON_PLAYING && cstate != CON_EDITING ) )
			{
				write_to_buffer( d, "> already connected - try again\n\r", 0 );
				sprintf( log_buf, "> %s already connected", ch->name );
				log_string_plus( log_buf, LOG_COMM );
				return BERR;
			}
			if ( !kick )
				return TRUE;
			write_to_buffer( d, "> already playing... kicking-off old connection\n\r", 0 );
			write_to_buffer( dold, "> kicking-off old connection - bye\n\r", 0 );
			close_socket( dold, FALSE );
			/* clear descriptor pointer to get rid of bug message in log */
			d->character->desc = NULL;
			free_char( d->character );
			d->character = ch;
			ch->desc	 = d;
			ch->timer	 = 0;
			if ( ch->switched )
				do_return( ch->switched, "" );
			ch->switched = NULL;
			send_to_char( "> reconnecting\n\r", ch );
			act( AT_ACTION, "> $n has reconnected - kicking-off old link",
					ch, NULL, NULL, TO_ROOM );
			sprintf( log_buf, "> %s@%s reconnected - kicking-off old link",
					ch->name, d->host );
			log_string_plus( log_buf, LOG_COMM );
			d->connected = cstate;
			return TRUE;
		}
	}

	return FALSE;
}



void stop_idling( CHAR_DATA *ch )
{
	if ( !ch
			||   !ch->desc
			||    ch->desc->connected != CON_PLAYING
			||   !ch->was_in_room
			||    ch->in_room != get_room_index( ROOM_VNUM_LIMBO ) )
		return;

	ch->timer = 0;
	char_from_room( ch );
	char_to_room( ch, ch->was_in_room );
	ch->was_in_room	= NULL;
	act( AT_ACTION, "$n has returned from the void", ch, NULL, NULL, TO_ROOM );
	return;
}



/*
 * Write to one char. Commented out in favour of colour
 *
void send_to_char( const char *txt, CHAR_DATA *ch )
{
    if ( !ch )
    {
      bug( "Send_to_char: NULL *ch" );
      return;
    }
    if ( txt && ch->desc )
	write_to_buffer( ch->desc, txt, strlen(txt) );
    return;
}
 */

/*
 * Same as above, but converts &color codes to ANSI sequences..
 */
void send_to_char_color( const char *txt, CHAR_DATA *ch )
{
	DESCRIPTOR_DATA *d;
	char *colstr;
	const char *prevstr = txt;
	char colbuf[20];
	int ln;

	if ( !ch )
	{
		bug( "Send_to_char_color: NULL *ch" );
		return;
	}
	if ( !txt || !ch->desc )
		return;
	d = ch->desc;
	/* Clear out old color stuff */
	/*  make_color_sequence(NULL, NULL, NULL);*/
  while ( d && ((colstr = strpbrk(prevstr, "&^")) != NULL ))
	{
		if (colstr > prevstr)
			write_to_buffer(d, prevstr, (colstr-prevstr));
		ln = make_color_sequence(colstr, colbuf, d);
		if ( ln < 0 )
		{
			prevstr = colstr+1;
			break;
		}
		else if ( ln > 0 )
			write_to_buffer(d, colbuf, ln);
		prevstr = colstr+2;
	}
	if ( *prevstr )
		write_to_buffer(d, prevstr, 0);
	return;
}

void write_to_pager( DESCRIPTOR_DATA *d, const char *txt, int length )
{
	if ( length <= 0 )
		length = strlen(txt);
	if ( length == 0 )
		return;
	if ( !d->pagebuf )
	{
		d->pagesize = MAX_STRING_LENGTH;
		CREATE( d->pagebuf, char, d->pagesize );
	}
	if ( !d->pagepoint )
	{
		d->pagepoint = d->pagebuf;
		d->pagetop = 0;
		d->pagecmd = '\0';
	}
	if ( d->pagetop == 0 && !d->fcommand )
	{
		d->pagebuf[0] = '\n';
		d->pagebuf[1] = '\r';
		d->pagetop = 2;
	}
  while ( d->pagetop + (unsigned int) length >= d->pagesize )
	{
		if ( d->pagesize > 32000 )
		{
			bug( "Pager overflow.  Ignoring\n\r" );
			d->pagetop = 0;
			d->pagepoint = NULL;
			DISPOSE(d->pagebuf);
			d->pagesize = MAX_STRING_LENGTH;
			return;
		}
		d->pagesize *= 2;
		RECREATE(d->pagebuf, char, d->pagesize);
	}
	strncpy(d->pagebuf+d->pagetop, txt, length);
	d->pagetop += length;
	d->pagebuf[d->pagetop] = '\0';
	return;
}

/* commented out in favour of colour routine

void send_to_pager( const char *txt, CHAR_DATA *ch )
{
  if ( !ch )
  {
    bug( "Send_to_pager: NULL *ch" );
    return;
  }
  if ( txt && ch->desc )
  {
    DESCRIPTOR_DATA *d = ch->desc;

    ch = d->original ? d->original : d->character;
    if ( IS_NPC(ch) || !IS_SET(ch->pcdata->flags, PCFLAG_PAGERON) )
    {
	send_to_char(txt, d->character);
	return;
    }
    write_to_pager(d, txt, 0);
  }
  return;
}

 */

void send_to_pager_color( const char *txt, CHAR_DATA *ch )
{
	DESCRIPTOR_DATA *d;
	char *colstr;
	const char *prevstr = txt;
	char colbuf[20];
	int ln;

	if ( !ch )
	{
		bug( "Send_to_pager_color: NULL *ch" );
		return;
	}
	if ( !txt || !ch->desc )
		return;
	d = ch->desc;
	ch = d->original ? d->original : d->character;
	if ( IS_NPC(ch) || !IS_SET(ch->pcdata->flags, PCFLAG_PAGERON) )
	{
		send_to_char_color(txt, d->character);
		return;
	}
	/* Clear out old color stuff */
	/*  make_color_sequence(NULL, NULL, NULL);*/
	while ( (colstr = strpbrk(prevstr, "&^")) != NULL )
	{
		if ( colstr > prevstr )
			write_to_pager(d, prevstr, (colstr-prevstr));
		ln = make_color_sequence(colstr, colbuf, d);
		if ( ln < 0 )
		{
			prevstr = colstr+1;
			break;
		}
		else if ( ln > 0 )
			write_to_pager(d, colbuf, ln);
		prevstr = colstr+2;
	}
	if ( *prevstr )
		write_to_pager(d, prevstr, 0);
	return;
}

void set_char_color( sh_int AType, CHAR_DATA *ch )
{
	char buf[16];
	CHAR_DATA *och;

	if ( !ch || !ch->desc )
		return;

	och = (ch->desc->original ? ch->desc->original : ch);
	if ( !IS_NPC(och) && IS_SET(och->act, PLR_ANSI) )
	{
		if ( AType == 7 )
			strcpy( buf, "\033[m" );
		else
			sprintf(buf, "\033[0;%d;%s%dm", (AType & 8) == 8,
					(AType > 15 ? "5;" : ""), (AType & 7)+30);
		write_to_buffer( ch->desc, buf, strlen(buf) );
	}
	return;
}

void set_pager_color( sh_int AType, CHAR_DATA *ch )
{
	char buf[16];
	CHAR_DATA *och;

	if ( !ch || !ch->desc )
		return;

	och = (ch->desc->original ? ch->desc->original : ch);
	if ( !IS_NPC(och) && IS_SET(och->act, PLR_ANSI) )
	{
		if ( AType == 7 )
			strcpy( buf, "\033[m" );
		else
			sprintf(buf, "\033[0;%d;%s%dm", (AType & 8) == 8,
					(AType > 15 ? "5;" : ""), (AType & 7)+30);
		send_to_pager( buf, ch );
		ch->desc->pagecolor = AType;
	}
	return;
}


/* source: EOD, by John Booth <???> */
void ch_printf(CHAR_DATA *ch, char *fmt, ...)
{
	char buf[MAX_STRING_LENGTH*2];	/* better safe than sorry */
	va_list args;

	va_start(args, fmt);
	vsprintf(buf, fmt, args);
	va_end(args);

	send_to_char(buf, ch);
}

void pager_printf(CHAR_DATA *ch, char *fmt, ...)
{
	char buf[MAX_STRING_LENGTH*2];
	va_list args;

	va_start(args, fmt);
	vsprintf(buf, fmt, args);
	va_end(args);

	send_to_pager(buf, ch);
}



char *obj_short( OBJ_DATA *obj )
{
	static char buf[MAX_STRING_LENGTH];

	if ( obj->count > 1 )
	{
		sprintf( buf, "%s (%d)", obj->short_descr, obj->count );
		return buf;
	}
	return obj->short_descr;
}

/*
 * The primary output interface for formatted output.
 */
/* Major overhaul. -- Alty */
#define NAME(ch)	(IS_NPC(ch) ? ch->short_descr : ch->name)
char *act_string(const char *format, CHAR_DATA *to, CHAR_DATA *ch,
		const void *arg1, const void *arg2)
{
	static char * const he_she  [] = { "it",  "he",  "she" };
	static char * const him_her [] = { "it",  "him", "her" };
	static char * const his_her [] = { "its", "his", "her" };
	static char buf[MAX_STRING_LENGTH];
	char fname[MAX_INPUT_LENGTH];
	char *point = buf;
	const char *str = format;
	const char *i;
	CHAR_DATA *vch = (CHAR_DATA *) arg2;
	OBJ_DATA *obj1 = (OBJ_DATA  *) arg1;
	OBJ_DATA *obj2 = (OBJ_DATA  *) arg2;

	while ( *str != '\0' )
	{
		if ( *str != '$' )
		{
			*point++ = *str++;
			continue;
		}
		++str;
		if ( !arg2 && *str >= 'A' && *str <= 'Z' )
		{
			bug( "Act: missing arg2 for code %c:", *str );
			bug( format );
			i = " <@@@> ";
		}
		else
		{
			switch ( *str )
			{
			default:  bug( "Act: bad code %c", *str );
			i = " <@@@> ";						break;
			case 't': i = (char *) arg1;					break;
			case 'T': i = (char *) arg2;					break;
			case 'n': i = (to ? PERS( ch, to) : NAME( ch));			break;
			case 'N': i = (to ? PERS(vch, to) : NAME(vch));			break;
			case 'e': if (ch->sex > 2 || ch->sex < 0)
			{
				bug("act_string: player %s has sex set at %d!", ch->name,
						ch->sex);
				i = "it";
			}
			else
				i = he_she [URANGE(0,  ch->sex, 2)];
			break;
			case 'E': if (vch->sex > 2 || vch->sex < 0)
			{
				bug("act_string: player %s has sex set at %d!", vch->name,
						vch->sex);
				i = "it";
			}
			else
				i = he_she [URANGE(0, vch->sex, 2)];
			break;
			case 'm': if (ch->sex > 2 || ch->sex < 0)
			{
				bug("act_string: player %s has sex set at %d!", ch->name,
						ch->sex);
				i = "it";
			}
			else
				i = him_her[URANGE(0,  ch->sex, 2)];
			break;
			case 'M': if (vch->sex > 2 || vch->sex < 0)
			{
				bug("act_string: player %s has sex set at %d!", vch->name,
						vch->sex);
				i = "it";
			}
			else
				i = him_her[URANGE(0, vch->sex, 2)];
			break;
			case 's': if (ch->sex > 2 || ch->sex < 0)
			{
				bug("act_string: player %s has sex set at %d!", ch->name,
						ch->sex);
				i = "its";
			}
			else
				i = his_her[URANGE(0,  ch->sex, 2)];
			break;
			case 'S': if (vch->sex > 2 || vch->sex < 0)
			{
				bug("act_string: player %s has sex set at %d!", vch->name,
						vch->sex);
				i = "its";
			}
			else
				i = his_her[URANGE(0, vch->sex, 2)];
			break;
			case 'q': i = (to == ch) ? "" : "s";				break;
			case 'Q': i = (to == ch) ? "your" :
					his_her[URANGE(0,  ch->sex, 2)];			break;
			case 'p': i = (!to || can_see_obj(to, obj1)
					? obj_short(obj1) : "something");			break;
			case 'P': i = (!to || can_see_obj(to, obj2)
					? obj_short(obj2) : "something");			break;
			case 'd':
				if ( !arg2 || ((char *) arg2)[0] == '\0' )
					i = "door";
				else
				{
					one_argument((char *) arg2, fname);
					i = fname;
				}
				break;
			}
		}
		++str;
		while ( (*point = *i) != '\0' )
			++point, ++i;
	}
	strcpy(point, "\n\r");
	buf[0] = UPPER(buf[0]);
	return buf;
}
#undef NAME

void act( sh_int AType, const char *format, CHAR_DATA *ch, const void *arg1, const void *arg2, int type )
{
	char *txt;
	CHAR_DATA *to;
	CHAR_DATA *vch = (CHAR_DATA *)arg2;

	/*
	 * Discard null and zero-length messages.
	 */
	if ( !format || format[0] == '\0' )
		return;

	if ( !ch )
	{
		bug( "Act: null ch. (%s)", format );
		return;
	}

	if ( !ch->in_room )
		to = NULL;
	else if ( type == TO_CHAR )
		to = ch;
	else
		to = ch->in_room->first_person;

	/*
	 * ACT_SECRETIVE handling
	 */
	if ( IS_NPC(ch) && IS_SET(ch->act, ACT_SECRETIVE) && type != TO_CHAR )
		return;

	if ( type == TO_VICT )
	{
		if ( !vch )
		{
			bug( "Act: null vch with TO_VICT." );
			bug( "%s (%s)", ch->name, format );
			return;
		}
		if ( !vch->in_room )
		{
			bug( "Act: vch in NULL room!" );
			bug( "%s -> %s (%s)", ch->name, vch->name, format );
			return;
		}
		to = vch;
		/*	to = vch->in_room->first_person;*/
	}

	if ( MOBtrigger && type != TO_CHAR && type != TO_VICT && to )
	{
		OBJ_DATA *to_obj;

		txt = act_string(format, NULL, ch, arg1, arg2);
		if ( IS_SET(to->in_room->progtypes, ACT_PROG) )
			rprog_act_trigger(txt, to->in_room, ch, (OBJ_DATA *)arg1, (void *)arg2);
		for ( to_obj = to->in_room->first_content; to_obj;
				to_obj = to_obj->next_content )
			if ( IS_SET(to_obj->pIndexData->progtypes, ACT_PROG) )
				oprog_act_trigger(txt, to_obj, ch, (OBJ_DATA *)arg1, (void *)arg2);
	}

	/* Anyone feel like telling me the point of looping through the whole
       room when we're only sending to one char anyways..? -- Alty */
	for ( ; to; to = (type == TO_CHAR || type == TO_VICT)
	? NULL : to->next_in_room )
	{
	if (((!to || !to->desc) 
				&& (  IS_NPC(to) && !IS_SET(to->pIndexData->progtypes, ACT_PROG) ))
				||   !IS_AWAKE(to) )
			continue;


        if(!can_see(to, ch) && type != TO_VICT )
          continue;

		if ( type == TO_CHAR && to != ch )
			continue;
		if ( type == TO_VICT && ( to != vch || to == ch ) )
			continue;
		if ( type == TO_ROOM && to == ch )
			continue;
		if ( type == TO_NOTVICT && (to == ch || to == vch) )
			continue;

        if(!can_see(to, ch) && type != TO_VICT )
          continue;

		txt = act_string(format, to, ch, arg1, arg2);
	if (to && to->desc)
		{
			set_char_color(AType, to);
			send_to_char_color( txt, to );
		}
		if (MOBtrigger)
		{
			/* Note: use original string, not string with ANSI. -- Alty */
			mprog_act_trigger( txt, to, ch, (OBJ_DATA *)arg1, (void *)arg2 );
		}
	}
	MOBtrigger = TRUE;
	return;
}

char *default_prompt( CHAR_DATA *ch )
{
	static char buf[MAX_STRING_LENGTH];
	strcpy( buf,"" );
	//strcat(buf, "&C%h &B/&C%v");
	//strcat(buf, "&B >&w");
	strcat(buf, "&W^b%h&C^x|&W^p%v&w^x");
	strcat(buf, "&w >");
	return buf;
}

int getcolor(char clr)
{
  static const char colors[17] = "xrgObpcwzRGYBPCW";
	int r;

	for ( r = 0; r < 16; r++ )
		if ( clr == colors[r] )
			return r;
	return -1;
}

void display_prompt( DESCRIPTOR_DATA *d )
{
	CHAR_DATA *ch = d->character;
	CHAR_DATA *och = (d->original ? d->original : d->character);
	bool ansi = (!IS_NPC(och) && IS_SET(och->act, PLR_ANSI));
	const char *prompt;
	char buf[MAX_STRING_LENGTH];
	char *pbuf = buf;
	int stat;

	if ( !ch )
	{
		bug( "display_prompt: NULL ch" );
		return;
	}

	if ( !IS_NPC(ch) && ch->substate != SUB_NONE && ch->pcdata->subprompt
			&&   ch->pcdata->subprompt[0] != '\0' )
		prompt = ch->pcdata->subprompt;
	else
		if ( IS_NPC(ch) || !ch->pcdata->prompt || !*ch->pcdata->prompt )
			prompt = default_prompt(ch);
		else
			prompt = ch->pcdata->prompt;

	if ( ansi )
	{
		strcpy(pbuf, "\033[m");
		d->prevcolor = 0x07;
		pbuf += 3;
	}
	/* Clear out old color stuff */
	/*  make_color_sequence(NULL, NULL, NULL);*/
	for ( ; *prompt; prompt++ )
	{
		/*
		 * '&' = foreground color/intensity bit
		 * '^' = background color/blink bit
		 * '%' = prompt commands
		 * Note: foreground changes will revert background to 0 (black)
		 */
		if ( *prompt != '&' && *prompt != '^' && *prompt != '%' )
		{
			*(pbuf++) = *prompt;
			continue;
		}
		++prompt;
		if ( !*prompt )
			break;
		if ( *prompt == *(prompt-1) )
		{
			*(pbuf++) = *prompt;
			continue;
		}
		switch(*(prompt-1))
		{
		default:
			bug( "Display_prompt: bad command char '%c'", *(prompt-1) );
			break;
		case '&':
		case '^':
			stat = make_color_sequence(&prompt[-1], pbuf, d);
			if ( stat < 0 )
				--prompt;
			else if ( stat > 0 )
				pbuf += stat;
			break;
		case '%':
			*pbuf = '\0';
			stat = 0x80000000;
			switch(*prompt)
			{
			case '%':
			*pbuf++ = '%';
			*pbuf = '\0';
			break;
			case 'a':
				if ( IS_GOOD(ch) )
					strcpy(pbuf, "good");
				else if ( IS_EVIL(ch) )
					strcpy(pbuf, "evil");
				else
					strcpy(pbuf, "neutral");
				break;
			case 'H':
			case 'h':

				stat = ch->hit;
				break;

				/*
	if ( ch->hit >= 100 )
	  strcpy(pbuf, "healthy");
	else if (ch->hit > 90 )
	  strcpy(pbuf, "scratched");
	else if (ch->hit >= 75 )
	  strcpy(pbuf, "hurt");
	else if (ch->hit >= 50 )
	  strcpy(pbuf, "in pain");
	else if (ch->hit >= 35 )
	  strcpy(pbuf, "severely wounded");
	else if (ch->hit >= 20 )
	  strcpy(pbuf, "writhing in pain");
	else if (ch->hit >= 0 )
	  strcpy(pbuf, "barely conscious");
	else if (ch->hit >= -100 )
	  strcpy(pbuf, "unconscious");
	else
	  strcpy(pbuf, "dead");
	break;
				 */
			case 'u':
				stat = num_descriptors;
				break;
			case 'U':
				stat = sysdata.maxplayers;
				break;
			case 'v':
			case 'V':
				stat = ch->move;
				break;

				/*
	if ( ch->move > 500 )
	  strcpy(pbuf, "energetic");
	else if ( ch->move > 100 )
	  strcpy(pbuf, "rested");
	else if ( ch->move > 50 )
	  strcpy(pbuf, "worn out");
	else if ( ch->move > 0 )
	  strcpy(pbuf, "exhausted");
	else
	  strcpy(pbuf, "too tired to move");
	break;
				 */
			case 'g':
				stat = ch->gold;
				break;
			case 'r':
				if ( IS_IMMORTAL(och) )
					stat = ch->in_room->vnum;
				break;
			case 'R':
				if ( IS_SET(och->act, PLR_ROOMVNUM) )
					sprintf(pbuf, "<#%ld> ", ch->in_room->vnum);
				break;
			case 'i':
				if ( (!IS_NPC(ch) && IS_SET(ch->act, PLR_WIZINVIS)) ||
						(IS_NPC(ch) && IS_SET(ch->act, ACT_MOBINVIS)) )
					sprintf(pbuf, "[invis %d] ", (IS_NPC(ch) ? ch->mobinvis : ch->pcdata->wizinvis));
				else
					if ( IS_AFFECTED(ch, AFF_INVISIBLE) )
						sprintf(pbuf, "[invis] " );
				break;
			case 'I':
				stat = (IS_NPC(ch) ? (IS_SET(ch->act, ACT_MOBINVIS) ? ch->mobinvis : 0)
						: (IS_SET(ch->act, PLR_WIZINVIS) ? ch->pcdata->wizinvis : 0));
				break;
			}
      if ( stat != (int) 0x80000000 )
				sprintf(pbuf, "%d", stat);
			pbuf += strlen(pbuf);
			break;
		}
	}
	*pbuf = '\0';
	write_to_buffer(d, buf, (pbuf-buf));
	return;
}

int make_color_sequence(const char *col, char *buf, DESCRIPTOR_DATA *d)
{
	int ln;
	const char *ctype = col;
	unsigned char cl;
	CHAR_DATA *och;
	bool ansi;

	och = (d->original ? d->original : d->character);
	ansi = (!IS_NPC(och) && IS_SET(och->act, PLR_ANSI));
	col++;
	if ( !*col )
		ln = -1;
	else if ( *ctype != '&' && *ctype != '^' )
	{
		bug("Make_color_sequence: command '%c' not '&' or '^'", *ctype);
		ln = -1;
	}
	else if ( *col == *ctype )
	{
		buf[0] = *col;
		buf[1] = '\0';
		ln = 1;
	}
	else if ( !ansi )
		ln = 0;
	else
	{
		cl = d->prevcolor;
		switch(*ctype)
		{
		default:
			bug( "Make_color_sequence: bad command char '%c'", *ctype );
			ln = -1;
			break;
		case '&':
			if ( *col == '-' )
			{
				buf[0] = '~';
				buf[1] = '\0';
				ln = 1;
				break;
			}
		case '^':
		{
			int newcol;

			if ( (newcol = getcolor(*col)) < 0 )
			{
				ln = 0;
				break;
			}
			else if ( *ctype == '&' )
				cl = (cl & 0xF0) | newcol;
			else
				cl = (cl & 0x0F) | (newcol << 4);
		}
		if ( cl == d->prevcolor )
		{
			ln = 0;
			break;
		}
		strcpy(buf, "\033[");
		if ( (cl & 0x88) != (d->prevcolor & 0x88) )
		{
			strcat(buf, "m\033[");
			if ( (cl & 0x08) )
				strcat(buf, "1;");
			if ( (cl & 0x80) )
				strcat(buf, "5;");
			d->prevcolor = 0x07 | (cl & 0x88);
			ln = strlen(buf);
		}
		else
			ln = 2;
		if ( (cl & 0x07) != (d->prevcolor & 0x07) )
		{
			sprintf(buf+ln, "3%d;", cl & 0x07);
			ln += 3;
		}
		if ( (cl & 0x70) != (d->prevcolor & 0x70) )
		{
			sprintf(buf+ln, "4%d;", (cl & 0x70) >> 4);
			ln += 3;
		}
		if ( buf[ln-1] == ';' )
			buf[ln-1] = 'm';
		else
		{
			buf[ln++] = 'm';
			buf[ln] = '\0';
		}
		d->prevcolor = cl;
		}
	}
	if ( ln <= 0 )
		*buf = '\0';
	return ln;
}

void set_pager_input( DESCRIPTOR_DATA *d, char *argument )
{
	while ( isspace(*argument) )
		argument++;
	d->pagecmd = *argument;
	return;
}

bool pager_output( DESCRIPTOR_DATA *d )
{
	register char *last;
	CHAR_DATA *ch;
	int pclines;
	register int lines;
	bool ret;

	if ( !d || !d->pagepoint || d->pagecmd == -1 )
		return TRUE;
	ch = d->original ? d->original : d->character;
	pclines = UMAX(ch->pcdata->pagerlen, 5) - 1;
	switch(LOWER(d->pagecmd))
	{
	default:
		lines = 0;
		break;
	case 'b':
	lines = -1-(pclines*2);
	break;
	case 'r':
		lines = -1-pclines;
		break;
	case 'q':
		d->pagetop = 0;
		d->pagepoint = NULL;
		flush_buffer(d, TRUE);
		DISPOSE(d->pagebuf);
		d->pagesize = MAX_STRING_LENGTH;
		return TRUE;
	}
	while ( lines < 0 && d->pagepoint >= d->pagebuf )
		if ( *(--d->pagepoint) == '\n' )
			++lines;
	if ( *d->pagepoint == '\n' && *(++d->pagepoint) == '\r' )
		++d->pagepoint;
	if ( d->pagepoint < d->pagebuf )
		d->pagepoint = d->pagebuf;
	for ( lines = 0, last = d->pagepoint; lines < pclines; ++last )
		if ( !*last )
			break;
		else if ( *last == '\n' )
			++lines;
	if ( *last == '\r' )
		++last;
	if ( last != d->pagepoint )
	{
		if ( !write_to_descriptor(d->descriptor, d->pagepoint,
				(last-d->pagepoint)) )
			return FALSE;
		d->pagepoint = last;
	}
	while ( isspace(*last) )
		++last;
	if ( !*last )
	{
		d->pagetop = 0;
		d->pagepoint = NULL;
		flush_buffer(d, TRUE);
		DISPOSE(d->pagebuf);
		d->pagesize = MAX_STRING_LENGTH;
		return TRUE;
	}
	d->pagecmd = -1;
	if ( IS_SET( ch->act, PLR_ANSI ) )
		if ( write_to_descriptor(d->descriptor, "\033[1;36m", 7) == FALSE )
			return FALSE;
	if ( (ret=write_to_descriptor(d->descriptor,
			"[c]ontinue, [r]efresh, [b]ack, [q]uit: [c] ", 0)) == FALSE )
		return FALSE;
	if ( IS_SET( ch->act, PLR_ANSI ) )
	{
		char buf[32];

		if ( d->pagecolor == 7 )
			strcpy( buf, "\033[m" );
		else
			sprintf(buf, "\033[0;%d;%s%dm", (d->pagecolor & 8) == 8,
					(d->pagecolor > 15 ? "5;" : ""), (d->pagecolor & 7)+30);
		ret = write_to_descriptor( d->descriptor, buf, 0 );
	}
	return ret;
}

//done for Neuro
