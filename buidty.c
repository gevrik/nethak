/********************************************************************************
 **    ( .-""-.   )+-+-+-+-+-+ +-+-+-+-+-+-+-+-+-+-+-+ +-+-+-+-+(   .-""-. )   **
 **     / _  _ \ ( | | | | | | |D|A|R|K| |R|E|A|L|M|S| | | | | | ) / _  _ \    **
 **     |(_\/_)|  )+-+-+-+-+-+ +-+-+-+-+-+-+-+-+-+-+-+ +-+-+-+-+(  |/_)(_\|    **
 **     (_ /\ _)           MULTIVERSE Distribution v1.0            (_ /\ _)    **
 **      |v==v| (       Multiverse source created by Exodus       ) |mmmm|     **
 **      '-..-'           with Aeone, Kain, Kyros, and Kast         '-..-'     **
 **----------------------------------------------------------------------------**
 ** SMAUG 1.4 written by : Thoric (Derek Snider) with Altrag, Blodkai,         **
 **                        Haus, Narn, Scryn, Swordbearer, Tricops, Gorog,     **
 **                        Rennard, Grishnakh, Fireblade and Nivek.            **
 **----------------------------------------------------------------------------**
 ** Original MERC 2.1 by : Hatchet, Furey, and Kahn.                           **
 ** Original Diku MUD by : Hans Staerfeldt, Katja Nyboe, Tom Madsen,           **
 **                        Michael Seifert & Sebastian Hammer.                 **
 **----------------------------------------------------------------------------**
 *		           Bug/Idea/Typo Tracking Module		        *
 ********************************************************************************/

#include <time.h>
#include "mud.h"


BUG_DATA	*	firstBug;
BUG_DATA	*	lastBug;
IDEA_DATA	*	firstIdea;
IDEA_DATA	*	lastIdea;
TYPO_DATA	*	firstTypo;
TYPO_DATA	*	lastTypo;



char *  timeStamp		args( ( void ) );
void 	checkBuidty		args( ( CHAR_DATA *ch ) );
void 	nstralloc		args( ( char **pointer ) );



struct	bugData
{
  BUG_DATA *	next;
  BUG_DATA * 	prev;
  char	*	bugDesc;
  char 	*	foundBy;
  char	*	foundWhen;
  char	*	fixedBy;
  char	*	fixedWhen;
  char	*	fixDesc;
  short		type;
  short		bonus;
  bool		reward;
  int		room;
};


struct	ideaData
{
  IDEA_DATA *	next;
  IDEA_DATA *	prev;
  char	*	ideaDesc;
  char 	*	madeBy;
  char	*	madeWhen;
  char	*	usedBy;
  char	*	usedWhen;
  char	*	useDesc;
  short		type;
  short		bonus;
  bool		reward;
  int		room;
};


struct	typoData
{
  TYPO_DATA *	next;
  TYPO_DATA *	prev;
  char	*	typoDesc;
  char 	*	foundBy;
  char	*	foundWhen;
  char	*	fixedBy;
  char	*	fixedWhen;
  short		type;
  short		bonus;
  bool		reward;
  int		room;
};




void freeOneBug( BUG_DATA *Bug )
{
  UNLINK(Bug, firstBug, lastBug, next, prev);
  STRFREE(Bug->bugDesc);
  STRFREE(Bug->foundBy);
  STRFREE(Bug->foundWhen);
  STRFREE(Bug->fixedBy);
  STRFREE(Bug->fixedWhen);
  STRFREE(Bug->fixDesc);
  DISPOSE(Bug);
}


void freeBugs( void )
{
  BUG_DATA *Bug;

  for( Bug = firstBug; Bug; Bug = Bug->next )
    freeOneBug(Bug);
}


// Quick timestamp function. Feel free to change the format if you wish --Exo
char *timeStamp( void )
{
   static char buf[MSL];
   struct tm *t = localtime(&current_time);

   snprintf(buf, MSL, "%02d-%02d-%04d", (t->tm_mon + 1),  t->tm_mday, (t->tm_year + 1900) );
   return buf;
}


void saveBugs( void )
{
  FILE *fp = NULL;
  BUG_DATA *Bug = NULL;
  char fname[MFL];

  snprintf(fname, MFL, "%s%s", SYSTEM_DIR, PBUG_FILE );
  if( !(fp = fopen(fname, "w")) )
  {
    bug( "%s: Couldn't open %s for writing!", __FUNCTION__, fname );
    return;
  }

  for( Bug = firstBug; Bug; Bug = Bug->next )
  {
    fprintf(fp, "#BUG\n" );
    fprintf(fp, "FoundBy       %s~\n",	Bug->foundBy	);
    fprintf(fp, "FoundWhen     %s~\n",  Bug->foundWhen  );
    fprintf(fp, "BugDesc       %s~\n",  Bug->bugDesc	);
    fprintf(fp, "Type          %d\n",	Bug->type	);
    fprintf(fp, "FixedBy       %s~\n",  Bug->fixedBy	);
    fprintf(fp, "FixedWhen     %s~\n",  Bug->fixedWhen  );
    fprintf(fp, "FixDesc       %s~\n",  Bug->fixDesc	);
    fprintf(fp, "Reward        %d\n",   Bug->reward	);
    fprintf(fp, "RewardBonus   %d\n",   Bug->bonus	);
    fprintf(fp, "Room          %d\n",   Bug->room 	);
    fprintf(fp, "End\n\n"                               );
  }
  fprintf(fp, "#END\n");
  FCLOSE(fp);
  return;
}


void fReadBug( BUG_DATA *Bug, FILE *fp )
{
  const char *word;
  bool fMatch;

  for( ; ; )
  {
    word = feof(fp) ? "End" : fread_word(fp);
    fMatch = FALSE;
    switch(UPPER(word[0]))
    {
      case '*':
	fMatch = TRUE;
        fread_to_eol(fp);
        break;

      case 'B':
        KEY( "BugDesc",		Bug->bugDesc,		fread_string(fp) );
        break;

      case 'E':
        if( !str_cmp(word, "End") )
        {
          nstralloc(&Bug->bugDesc);
          nstralloc(&Bug->foundBy);
          nstralloc(&Bug->foundWhen);
          nstralloc(&Bug->fixedBy);
          nstralloc(&Bug->fixedWhen);
          nstralloc(&Bug->fixDesc);
          return;
        }
        break;

      case 'F':
	KEY( "FixDesc",		Bug->fixDesc,		fread_string(fp) );
        KEY( "FixedBy",		Bug->fixedBy,		fread_string(fp) );
	KEY( "FixedWhen",	Bug->fixedWhen,		fread_string(fp) );
	KEY( "FoundBy",		Bug->foundBy,		fread_string(fp) );
	KEY( "FoundWhen",	Bug->foundWhen,		fread_string(fp) );
        break;

      case 'R':
        KEY( "Reward",		Bug->reward,		fread_number(fp) );
        KEY( "RewardBonus",	Bug->bonus,		fread_number(fp) );
        KEY( "Room",		Bug->room,		fread_number(fp) );
        break;

      case 'T':
        KEY( "Type",		Bug->type,		fread_number(fp) );
        break;
    }

    if( !fMatch )
      bug( "%s: no match: %s", __FUNCTION__, word );
  }
}


void loadBugs( void )
{
  char fname[MFL];
  BUG_DATA *Bug;
  FILE *fp;
     
  firstBug = NULL;
  lastBug = NULL;
  snprintf(fname, MFL, "%s%s", SYSTEM_DIR, PBUG_FILE );
  if( (fp = fopen(fname, "r")) != NULL )
  {
    for( ; ; )
    {
      char letter;
      const char *word;

      letter = fread_letter(fp);
      if( letter == '*' )
      {
 	fread_to_eol(fp);
	continue;
      }

      if( letter != '#' )
      {
	bug( "%s: # not found: (%c)", __FUNCTION__, letter );
	break;
      }

      word = fread_word(fp);
      if( !str_cmp(word, "BUG") )
      {
	CREATE(Bug, BUG_DATA, 1);
	fReadBug(Bug, fp);
	LINK(Bug, firstBug, lastBug, next, prev);
	continue;
      }
      else if( !str_cmp(word, "END") )
	break;
      else
      {
	bug( "%s: bad section: %s", __FUNCTION__, word );
	continue;
      }
    }
    FCLOSE(fp);
  }
  return;
}


char * bugSeverity( short type )
{
  switch(type)
  {
    case 1:  return "Lesser";		break;
    case 2:  return "Minor";		break;
    case 3:  return "Major";		break;
    case 4:  return "Severe";		break;
    case 5:  return "Critical";		break;
    default: return "Unknown??";	break;
  }
}


short severityArg( char *arg )
{
  if( !str_cmp(arg, "lesser") )		return 1;
  else if( !str_cmp(arg, "minor") )	return 2;
  else if( !str_cmp(arg, "major") )	return 3;
  else if( !str_cmp(arg, "severe") )	return 4;
  else if( !str_cmp(arg, "critical") )	return 5;
  else return -1;
}


void bugReward( CHAR_DATA *ch, BUG_DATA *error )
{
  int gold = 0;
  int glory = 0;
  char buf1[MSL], buf2[MSL];

  if( error->reward || IS_NPC(ch) )
    return;

  if( !str_cmp(error->foundBy, ch->name) )
  {
    switch(error->type)
    {
      case 1: gold = 100000; glory = 10;  break;
      case 2: gold = 200000; glory = 25;  break;
      case 3: gold = 300000; glory = 50;  break;
      case 4: gold = 400000; glory = 100; break;
      case 5: gold = 500000; glory = 250; break;
      default: break;
    }

    if( error->bonus == 1 )
    {
      gold = gold + (gold / 2);
      glory = glory + (glory / 2);
    }
    else if( error->bonus == 2 )
    {
      gold = gold * 2;
      glory = glory * 2;
    }
    else if( error->bonus == 3 )
    {
      gold = (gold * 2) + (gold / 2);
      glory = (glory * 2) + (glory / 2);
    }

    snprintf(buf1, MSL, "%s", num_punct(gold) );
    snprintf(buf2, MSL, "%s", num_punct(glory) );
    if( gold == 0 && glory == 0 )
    {
      ch_printf( ch, "The Immortals did not find it fit to reward you for your reported bug!\n\r" );
      ch_printf( ch, "The bug was probably submitted prior to your discovery.  Keep looking though!\n\r" );
    }
    else
      ch_printf(ch, "The Immortals have rewarded you %s gold and %s glory for finding a %s bug!\n\r",
        buf1, buf2, bugSeverity(error->type) );

    error->reward = TRUE;
    ch->pcdata->num_bugs++;
    ch->pcdata->bugReward1 += gold;
    ch->pcdata->bugReward2 += glory;
    ch->gold += gold;
    ch->pcdata->quest_curr += glory;
    send_to_pager( "&CNote from the Immortals\n\r&B----------------------------------------------------------------------&W\n\r", ch );
    send_to_pager(error->bugDesc, ch);
    send_to_pager( "\n\r", ch);
    save_char_obj(ch);
    saveBugs();
    return;
  }
  return;
}


CMDF do_bug( CHAR_DATA *ch, char *argument )
{
  char arg1[MIL];
  char arg2[MIL];
  BUG_DATA *Bug, *nBug;
  short cnt = 0;
  DESCRIPTOR_DATA *d;

  if( IS_NPC(ch) )
  {
    send_to_char( "Huh?\n\r", ch );
    return;
  }

  switch(ch->substate)
  {
    default:
      break;

    case SUB_RESTRICTED:
      send_to_char( "You cannot do this while in another command.\n\r", ch );
      return;

    case SUB_BUG_DESC:
      nBug = (BUG_DATA *) ch->dest_buf;
      STRFREE(nBug->bugDesc);
      nBug->bugDesc = copy_buffer(ch);
      stop_editing(ch);
      saveBugs();
      ch->substate = ch->tempnum;
      send_to_char( "Thanks, your bug report has been recorded.\n\r", ch );
      send_to_char( "You will be notified when the bug is fixed.\n\r", ch );
      log_printf_plus( LOG_NORMAL, LEVEL_IMMORTAL, "%s has reported a bug.", ch->name );
      return;

    case SUB_BUG_FIXDESC:
      Bug = (BUG_DATA *) ch->dest_buf;
      STRFREE(Bug->fixDesc);
      Bug->fixDesc = copy_buffer(ch);
      stop_editing(ch);
      saveBugs();
      ch->substate = ch->tempnum;
      send_to_char( "Ok, the bug has been fixed and the player who reported it will be notified.\n\r", ch );
      for( d = first_descriptor; d; d = d->next )
      {
        if( !d->character )
          continue;
	if( !str_cmp(Bug->foundBy, d->character->name) && !NULLSTR(Bug->fixedBy) )
          checkBuidty(d->character);
      }
      return;
  }

  argument = one_argument(argument, arg1);
  argument = one_argument(argument, arg2);
  if( NULLSTR(arg1) )
  {
    if( IS_IMMORTAL(ch) )
    {
      send_to_char( "&CNum   Room    Date        Name\n\r", ch );
      send_to_char( "&B--------------------------------------\n\r", ch );
      for( Bug = firstBug; Bug; Bug = Bug->next )
      {
        cnt++;
        ch_printf(ch, "%s%3d&C>  &W%-6d  %-10s  %-10s\n\r",
          NULLSTR(Bug->fixedBy) ? "&R" : "&W", cnt, Bug->room, Bug->foundWhen, Bug->foundBy );
      }

      if( cnt <= 0 )
      {
        send_to_char( "No bugs have been reported.\n\r", ch );
        return;
      }
      send_to_char( "&B--------------------------------------\n\r", ch );
      send_to_char( "\n\r&WSyntax: bug\n\rSyntax: bug report\n\r"
                    "Syntax: bug show <number>\n\rSyntax: bug fixed <number> <severity> <bonus>\n\r", ch );
    }
    else
    {
      send_to_char( "&CBugs\n\rLv Degree    Gold   Glory\n\r", ch );
      send_to_char( "&B--------------------------\n\r", ch );
      send_to_char( "&W1  lesser    100K   10\n\r", ch );
      send_to_char( "&W2  minor     200K   25\n\r", ch );
      send_to_char( "&W3  major     300K   50\n\r", ch );
      send_to_char( "&W4  severe    400K   100\n\r", ch );
      send_to_char( "&W5  critical  500K   250\n\r", ch );
      send_to_char( "&B--------------------------\n\r", ch );
      send_to_char( "&wRewards are increased 50% if crash cause or security risk is found and doubled if it causes data corruption.\n\r", ch );
      send_to_char( "These stack, so a critical bug that crashes AND causes data corruption would be a reward of 1,250 gold and 625 glory.\n\r\n\r", ch );
      send_to_char( "If you want to report a bug, type bug report\n\r", ch );
      send_to_char( "Your name, location and the current date are automatically recorded.\n\r", ch );
    }
    return;
  }

  if( !str_cmp(arg1, "report") )
  {
    CREATE(nBug, BUG_DATA, 1);
    LINK(nBug, firstBug, lastBug, next, prev);
    nBug->foundBy	= STRALLOC(ch->name);
    nBug->foundWhen	= STRALLOC(timeStamp());
    nBug->type		= -1;
    nBug->reward	= FALSE;
    nBug->room		= ch->in_room ? ch->in_room->vnum : -1;
    nBug->fixedBy	= STRALLOC("");
    nBug->fixedWhen	= STRALLOC("");
    nBug->fixDesc	= STRALLOC("");
    nBug->bugDesc	= STRALLOC("");

    if( ch->substate == SUB_REPEATCMD )
      ch->tempnum = SUB_REPEATCMD;
    else
      ch->tempnum = SUB_NONE;
    ch->substate = SUB_BUG_DESC;
    ch->dest_buf = nBug;
    start_editing(ch, nBug->bugDesc, MSL);
    saveBugs();
    return;
  }

  if( !IS_IMMORTAL(ch) )
  {
    send_to_char( "Huh?\n\r", ch );
    return;
  }

  if( !str_cmp(arg1, "show") )
  {
    if( NULLSTR(arg2) )
    {
      send_to_char( "Show which bug?\n\rSyntax: bug show <number>\n\r", ch );
      return;
    }

    cnt = 0;
    for( Bug = firstBug; Bug; Bug = Bug->next )
    {
      cnt++;
      if( cnt == atoi(arg2) )
        break;
    }

    if( !Bug )
    {
      send_to_char( "No such bug.\n\r", ch );
      return;
    }
    send_to_char( "&CNum   Room    Date        Name\n\r", ch );
    send_to_char( "&B----------------------------------------------------------------------\n\r", ch );
    ch_printf(ch, "%s%3d&C>  &W%-6d  %-10s  %-10s\n\r",
      NULLSTR(Bug->fixedBy) ? "&R" : "&W", cnt, Bug->room, Bug->foundWhen, Bug->foundBy );
    send_to_char( "&B----------------------------------------------------------------------\n\r", ch );
    ch_printf(ch, "&CDescription\n\r&W%s\n\r", Bug->bugDesc );
    send_to_char( "&B----------------------------------------------------------------------\n\r", ch );

    if( !NULLSTR(Bug->fixedBy) )
    {
      ch_printf(ch, "&CFixed by: &W%s\n\r", Bug->fixedBy );
      ch_printf(ch, "&CWhen    : &W%s\n\r", Bug->fixedWhen );
      ch_printf(ch, "&CSeverity: &W%s\n\r", bugSeverity(Bug->type) );
      ch_printf(ch, "&CRewarded: &W%s\n\r", Bug->reward ? "Yes" : "No" );

      if( Bug->bonus > 0 )
        ch_printf(ch, "&CBonus   : &W%s\n\r", Bug->bonus == 1 ? "+50%" : Bug->bonus == 1 ? "x2" :
          Bug->bonus > 1 ? "+50% x2" : "None" );
      send_to_char( "&B----------------------------------------------------------------------\n\r", ch );
      ch_printf(ch, "&CNotes\n\r&W%s\n\r", Bug->fixDesc );
      send_to_char( "&B----------------------------------------------------------------------\n\r", ch );
    }
    return;
  }
  else if( !str_cmp(arg1, "fixed") )
  {
    char arg3[MIL];
    char arg4[MIL];

    argument = one_argument(argument, arg3);
    argument = one_argument(argument, arg4);
    if( NULLSTR(arg2) )
    {
      send_to_char( "&WWhich bug is fixed?\n\rSyntax: bug fixed <number> <severity> <bonus>\n\r", ch );
      send_to_char( "Severity can be:\n\r  &wlesser minor major severe critical\n\r", ch );
      send_to_char( "&WBonus can be either: &wnone bonus1 bonus2 &Wor &wboth\n\r", ch );
      send_to_char( "\n\r&RDo not set a bug as fixed until you are positive the issue is resolved.\n\r", ch );
      return;
    }

    cnt = 0;
    for( Bug = firstBug; Bug; Bug = Bug->next )
    {
      cnt++;
      if( cnt == atoi(arg2) )
        break;
    }

    if( !Bug )
    {
      send_to_char( "No such bug.\n\r", ch );
      return;
    }

    if( NULLSTR(arg3) )
    {
      send_to_char( "How severe is the bug you are fixing?\n\rValid types are:  none lesser minor major severe critical\n\r", ch );
      return;
    }

    if( NULLSTR(arg4) )
    {
      send_to_char( "Are you giving a bonus for this bug?\n\rValid types are: bonus1 bonus2 both\n\rIf not, use none.\n\r", ch );
      return;
    }

    if( !str_cmp(arg4, "bonus1") )
      Bug->bonus = 1;
    else if( !str_cmp(arg4, "bonus2") )
      Bug->bonus = 2;
    else if( !str_cmp(arg4, "both") )
      Bug->bonus = 3;
    else if( !str_cmp(arg4, "none" ) )
      Bug->bonus = -1;
    else
    {
      send_to_char( "If you're giving a bonus to the bug reporter, use bonus1 or bonus2. If not, use none.\n\r", ch );
      return;
    }

    if( !NULLSTR(Bug->fixedBy) )
    {
      send_to_char( "That bug has already been fixed.\n\r", ch );
      return;
    }

    Bug->fixedBy	= STRALLOC(ch->name);
    Bug->fixedWhen	= STRALLOC(timeStamp());
    Bug->type		= severityArg(arg3);
    Bug->reward		= FALSE;
    send_to_char( "Please enter a description of how the issue was resolved.\n\r", ch );
    if( ch->substate == SUB_REPEATCMD )
      ch->tempnum = SUB_REPEATCMD;
    else
      ch->tempnum = SUB_NONE;

    ch->substate = SUB_BUG_FIXDESC;
    ch->dest_buf = Bug;
    start_editing(ch, Bug->fixDesc, MSL);
    saveBugs();
    return;
  }
  else if( !str_cmp(arg1, "delete") )
  {
    if( NULLSTR(arg2) )
    {
      send_to_char( "Delete which bug?\n\rSyntax: bug delete <number>\n\r", ch );
      return;
    }
    cnt = 0;

    for( Bug = firstBug; Bug; Bug = Bug->next )
    {
      cnt++;
      if( cnt == atoi(arg2) )
        break;
    }

    if( !Bug )
    {
      send_to_char( "No such bug.\n\r", ch );
      return;
    }
    ch_printf(ch, "Bug #%d deleted.\n\r", cnt );
    freeOneBug(Bug);
    saveBugs();
    return;
  }
  else
  {
    send_to_char( "\n\r&WSyntax: bug\n\rSyntax: bug report\n\r"
                  "Syntax: bug show <number>\n\rSyntax: bug fixed <number> <severity> <bonus>\n\r", ch );
    return;
  }
  return;
}



void freeOneIdea( IDEA_DATA *idea )
{
  UNLINK(idea, firstIdea, lastIdea, next, prev);
  STRFREE(idea->ideaDesc);
  STRFREE(idea->madeBy);
  STRFREE(idea->madeWhen);
  STRFREE(idea->usedBy);
  STRFREE(idea->usedWhen);
  STRFREE(idea->useDesc);
  DISPOSE(idea);
}


void freeIdeas( void )
{
  IDEA_DATA *idea;

  for( idea = firstIdea; idea; idea = idea->next )
    freeOneIdea(idea);
}


void saveIdeas( void )
{
  FILE *fp = NULL;
  IDEA_DATA *idea = NULL;
  char fname[MFL];

  snprintf(fname, MFL, "%s%s", SYSTEM_DIR, IDEA_FILE );
  if( !(fp = fopen(fname, "w")) )
  {
    bug( "%s: Couldn't open %s for writing", __FUNCTION__, fname );
    return;
  }

  for( idea = firstIdea; idea; idea = idea->next )
  {
    fprintf(fp, "#IDEA\n" );
    fprintf(fp, "MadeBy        %s~\n",	idea->madeBy	);
    fprintf(fp, "MadeWhen      %s~\n",  idea->madeWhen  );
    fprintf(fp, "IdeaDesc      %s~\n",  idea->ideaDesc	);
    fprintf(fp, "Type          %d\n",	idea->type	);
    fprintf(fp, "UsedBy        %s~\n",  idea->usedBy	);
    fprintf(fp, "UsedWhen      %s~\n",  idea->usedWhen  );
    fprintf(fp, "UseDesc       %s~\n",  idea->useDesc	);
    fprintf(fp, "Reward        %d\n",   idea->reward	);
    fprintf(fp, "RewardBonus   %d\n",   idea->bonus	);
    fprintf(fp, "Room          %d\n",   idea->room 	);
    fprintf(fp, "End\n\n"                               );
  }
  fprintf(fp, "#END\n");
  FCLOSE(fp);
  return;
}


void fReadIdea( IDEA_DATA *idea, FILE *fp )
{
  const char *word;
  bool fMatch;

  for( ; ; )
  {
    word = feof(fp) ? "End" : fread_word(fp);
    fMatch = FALSE;

    switch(UPPER(word[0]))
    {
      case '*':
        fMatch = TRUE;
        fread_to_eol(fp);
        break;

      case 'E':
        if( !str_cmp(word, "End") )
        {
          nstralloc(&idea->ideaDesc);
          nstralloc(&idea->madeBy);
          nstralloc(&idea->madeWhen);
          nstralloc(&idea->usedBy);
          nstralloc(&idea->usedWhen);
          nstralloc(&idea->useDesc);
          return;
        }
        break;

      case 'I':
        KEY( "IdeaDesc",        idea->ideaDesc,         fread_string(fp) );
        break;

      case 'M':
        KEY( "MadeBy",          idea->madeBy,          	fread_string(fp) );
        KEY( "MadeWhen",        idea->madeWhen,        	fread_string(fp) );
        break;

      case 'R':
        KEY( "Reward",          idea->reward,           fread_number(fp) );
        KEY( "RewardBonus",     idea->bonus,            fread_number(fp) );
        KEY( "Room",            idea->room,             fread_number(fp) );
        break;

      case 'T':
        KEY( "Type",            idea->type,             fread_number(fp) );
        break;

      case 'U':
        KEY( "UsedBy",          idea->usedBy,          	fread_string(fp) );
        KEY( "UsedWhen",        idea->usedWhen,        	fread_string(fp) );
        KEY( "UseDesc",         idea->useDesc,          fread_string(fp) );
    }

    if( !fMatch )
      bug( "%s: no match: %s", __FUNCTION__, word );
  }
}


void loadIdeas( void )
{
  char fname[MFL];
  IDEA_DATA *idea;
  FILE *fp;

  firstIdea = NULL;
  lastIdea = NULL;
  snprintf(fname, MFL, "%s%s", SYSTEM_DIR, IDEA_FILE );
  if( (fp = fopen(fname, "r")) != NULL )
  {
    for( ; ; )
    {
      char letter;
      const char *word;

      letter = fread_letter(fp);
      if( letter == '*' )
      {
        fread_to_eol(fp);
        continue;
      }

      if( letter != '#' )
      {
        bug( "%s: # not found: (%c)", __FUNCTION__, letter );
        break;
      }

      word = fread_word(fp);
      if( !str_cmp(word, "IDEA") )
      {
        CREATE(idea, IDEA_DATA, 1);
        fReadIdea(idea, fp);
        LINK(idea, firstIdea, lastIdea, next, prev);
        continue;
      }
      else if( !str_cmp(word, "END") )
        break;
      else
      {
        bug( "%s: bad section: %s", __FUNCTION__, word );
        continue;
      }
    }
    FCLOSE(fp);
  }
  return;
}


char * ideaLevel( short type )
{
  switch(type)
  {
    case 1:  return "Good";		break;
    case 2:  return "Great";		break;
    case 3:  return "Phenomonal";	break;
    default: return "Unknown??";	break;
  }
}


short ideaLevelArg( char *arg )
{
  if( !str_cmp(arg, "good") )		return 1;
  else if( !str_cmp(arg, "great") )	return 2;
  else if( !str_cmp(arg, "phenomonal") )return 3;
  else return -1;
}


void ideaReward( CHAR_DATA *ch, IDEA_DATA *idea )
{
  int gold = 0;
  int glory = 0;
  char buf1[MSL], buf2[MSL];

  if( idea->reward )
    return;

  if( !str_cmp(idea->madeBy, ch->name) )
  {
    switch(idea->type)
    {
      case 1: gold = 100000; glory = 50;  break;
      case 2: gold = 200000; glory = 100; break;
      case 3: gold = 300000; glory = 150; break;
      default: break;
    }

    if( idea->bonus == 1 )
    {
      gold = gold + (gold / 2);
      glory = glory + (glory / 2);
    }
    else if( idea->bonus == 2 )
    {
      gold = gold * 2;
      glory = glory * 2;
    }
    else if( idea->bonus == 3 )
    {
      gold = (gold * 2) + (gold / 2);
      glory = (glory * 2) + (glory / 2);
    }

    snprintf(buf1, MSL, "%s", num_punct(gold) );
    snprintf(buf2, MSL, "%s", num_punct(glory) );
    ch_printf(ch, "The Immortals have rewarded you %s gold and %s glory for your %s idea!\n\r", buf1, buf2, ideaLevel(idea->type) );
    idea->reward = TRUE;
    ch->pcdata->num_ideas++;
    ch->pcdata->ideaReward1 += gold;
    ch->pcdata->ideaReward2 += glory;
    ch->gold += gold;
    ch->pcdata->quest_curr += glory;
    send_to_pager( "&CNote from the Immortals\n\r&B----------------------------------------------------------------------&W\n\r", ch );
    send_to_pager(idea->useDesc, ch);
    send_to_pager( "\n\r", ch);
    save_char_obj(ch);
    saveIdeas();
    return;
  }
  return;
}


CMDF do_idea( CHAR_DATA *ch, char *argument )
{
  char arg1[MIL];
  char arg2[MIL];
  IDEA_DATA *Idea = NULL, *nIdea;
  short cnt = 0;
  DESCRIPTOR_DATA *d;

  if( IS_NPC(ch) )
  {
    send_to_char( "Huh?\n\r", ch );
    return;
  }

  switch(ch->substate)
  {
    default:
      break;

    case SUB_RESTRICTED:
      send_to_char( "You cannot do this while in another command.\n\r", ch );
      return;

    case SUB_IDEA_DESC:
      nIdea = (IDEA_DATA *) ch->dest_buf;
      STRFREE(nIdea->ideaDesc);
      nIdea->ideaDesc = copy_buffer(ch);
      stop_editing(ch);
      saveIdeas();
      ch->substate = ch->tempnum;
      send_to_char( "Thanks, your idea submission has been recorded.\n\r", ch );
      send_to_char( "You will be notified if the idea is implemented.\n\r", ch );
      log_printf_plus( LOG_NORMAL, LEVEL_IMMORTAL, "%s has submitted an idea.", ch->name );
      return;

    case SUB_IDEA_USEDESC:
      Idea = (IDEA_DATA *) ch->dest_buf;
      STRFREE(Idea->useDesc);
      Idea->useDesc = copy_buffer(ch);
      stop_editing(ch);
      saveIdeas();
      ch->substate = ch->tempnum;
      send_to_char( "Ok, the idea has been fixed and the player who reported it will be notified.\n\r", ch );
      for( d = first_descriptor; d; d = d->next )
      {
        if( !d->character )
          continue;
        checkBuidty(d->character);
      }
      return;
  }

  argument = one_argument(argument, arg1);
  argument = one_argument(argument, arg2);
  if( NULLSTR(arg1) )
  {
    if( IS_IMMORTAL(ch) )
    {
      send_to_char( "&CNum   Room    Date        Name\n\r", ch );
      send_to_char( "&B--------------------------------------\n\r", ch );
      for( Idea = firstIdea; Idea; Idea = Idea->next )
      {
        cnt++;
        ch_printf(ch, "%s%3d&C>  &W%-6d  %-10s  %-10s\n\r",
          NULLSTR(Idea->usedBy) ? "&R" : "&W", cnt, Idea->room, Idea->madeWhen, Idea->madeBy );
      }

      if( cnt <= 0 )
      {
        send_to_char( "No ideas have been submitted.\n\r", ch );
        return;
      }
      send_to_char( "&B--------------------------------------\n\r", ch );
      send_to_char( "\n\r&WSyntax: idea\n\rSyntax: idea submit\n\r"
                    "Syntax: idea show <number>\n\rSyntax: idea use <number> <level> <bonus>\n\r", ch );
    }
    else
    {
      send_to_char( "&CIdeas\n\rLv Degree     Gold   Glory\n\r", ch );
      send_to_char( "&B--------------------------\n\r", ch );
      send_to_char( "&W1  Good       100K   50\n\r", ch );
      send_to_char( "&W2  Great      200K   100\n\r", ch );
      send_to_char( "&W3  Phenomonal 300K   150\n\r", ch );
      send_to_char( "&B--------------------------\n\r", ch );
      send_to_char( "&wPhenomonal ideas are game-changing and will be credited to the originator if applicable.\n\r", ch );
      send_to_char( "If you want to submit a idea, type idea submit\n\r", ch );
      send_to_char( "Your name, location and the current date are automatically recorded.\n\r", ch );
    }
    return;
  }

  if( !str_cmp(arg1, "submit") )
  {
    CREATE(nIdea, IDEA_DATA, 1);
    LINK(nIdea, firstIdea, lastIdea, next, prev);
    nIdea->madeBy	= STRALLOC(ch->name);
    nIdea->madeWhen	= STRALLOC(timeStamp());
    nIdea->type		= -1;
    nIdea->reward	= -1;
    nIdea->room		= ch->in_room ? ch->in_room->vnum : -1;
    nIdea->usedBy	= STRALLOC("");
    nIdea->usedWhen	= STRALLOC("");
    nIdea->useDesc	= STRALLOC("");
    nIdea->ideaDesc	= STRALLOC("");

    if( ch->substate == SUB_REPEATCMD )
      ch->tempnum = SUB_REPEATCMD;
    else
      ch->tempnum = SUB_NONE;
    ch->substate = SUB_IDEA_DESC;
    ch->dest_buf = nIdea;
    start_editing(ch, nIdea->ideaDesc, MSL);
    saveIdeas();
    return;
  }

  if( !IS_IMMORTAL(ch) )
  {
    send_to_char( "Huh?\n\r", ch );
    return;
  }

  if( !str_cmp(arg1, "show") )
  {
    if( NULLSTR(arg2) )
    {
      send_to_char( "Show which idea?\n\rSyntax: idea show <number>\n\r", ch );
      return;
    }

    cnt = 0;
    for( Idea = firstIdea; Idea; Idea = Idea->next )
    {
      cnt++;
      if( cnt == atoi(arg2) )
        break;
    }

    if( !Idea )
    {
      send_to_char( "No such idea.\n\r", ch );
      return;
    }
    send_to_char( "&CNum   Room    Date        Name\n\r", ch );
    send_to_char( "&B----------------------------------------------------------------------\n\r", ch );
    ch_printf(ch, "%s%3d&C>  &W%-6d  %-10s  %-10s\n\r",
      NULLSTR(Idea->usedBy) ? "&R" : "&W", cnt, Idea->room, Idea->madeWhen, Idea->madeBy );
    send_to_char( "&B----------------------------------------------------------------------\n\r", ch );
    ch_printf(ch, "&CDescription\n\r&W%s\n\r", Idea->ideaDesc );
    send_to_char( "&B----------------------------------------------------------------------\n\r", ch );
    if( !NULLSTR(Idea->usedBy) )
    {
      ch_printf(ch, "&CUsed by : &W%s\n\r", Idea->usedBy );
      ch_printf(ch, "&CWhen    : &W%s\n\r", Idea->usedWhen );
      ch_printf(ch, "&CLevel   : &W%s\n\r", ideaLevel(Idea->type) );
      ch_printf(ch, "&CRewarded: &W%s\n\r", Idea->reward ? "Yes" : "No" );
      if( Idea->bonus > 0 )
        ch_printf(ch, "&CBonus   : &W%s\n\r", Idea->bonus == 1 ? "+50%" : Idea->bonus == 2 ? "x2" :
        Idea->bonus > 2 ? "+50% x2" : "None" );
      send_to_char( "&B----------------------------------------------------------------------\n\r", ch );
      ch_printf(ch, "&CNotes\n\r&W%s\n\r", Idea->useDesc );
      send_to_char( "&B----------------------------------------------------------------------\n\r", ch );
    }
    return;
  }
  else if( !str_cmp(arg1, "use") )
  {
    char arg3[MIL];
    char arg4[MIL];

    argument = one_argument(argument, arg3);
    argument = one_argument(argument, arg4);
    if( NULLSTR(arg2) )
    {
      send_to_char( "&WWhich idea has been used?\n\rSyntax: idea used <number> <level> <bonus>\n\r", ch );
      send_to_char( "Level can be:\n\r  good great phenomonal\n\r", ch );
      send_to_char( "Bonus can be either: &wnone bonus1 bonus2 &Wor &wboth\n\r", ch );
      send_to_char( "\n\r&RDo not set a idea as used until you are positive it has been fully implemented.\n\r", ch );
      return;
    }

    cnt = 0;
    for( Idea = firstIdea; Idea; Idea = Idea->next )
    {
      cnt++;
      if( cnt == atoi(arg2) )
        break;
    }

    if( !Idea )
    {
      send_to_char( "No such idea.\n\r", ch );
      return;
    }

    if( NULLSTR(arg3) )
    {
      send_to_char( "How good is the idea you are using?\n\rValid types are:  none good great phenomonal\n\r", ch );
      return;
    }

    if( NULLSTR(arg4) )
    {
      send_to_char( "Are you giving a bonus for this idea?\n\rValid types are: none bonus1 bonus2 both\n\r", ch );
      return;
    }

    if( !str_cmp(arg4, "bonus1") )
      Idea->bonus = 1;
    else if( !str_cmp(arg4, "bonus2") )
      Idea->bonus = 2;
    else if( !str_cmp(arg4, "both") )
      Idea->bonus = 3;
    else if( !str_cmp(arg4, "none") )
      Idea->bonus = -1;
    else
    {
      send_to_char( "If you're giving a bonus to the idea submitter, use bonus1 or bonus2. If not, use none.\n\r", ch );
      return;
    }

    if( !NULLSTR(Idea->usedBy) )
    {
      send_to_char( "That idea has already been used.\n\r", ch );
      return;
    }

    Idea->useDesc	= STRALLOC(capitalize(argument));
    Idea->usedBy	= STRALLOC(ch->name);
    Idea->usedWhen	= STRALLOC(timeStamp());
    Idea->type		= ideaLevelArg(arg3);
    Idea->reward	= FALSE;
    send_to_char( "Please enter a description of how the idea was implemented.\n\r", ch );
    if( ch->substate == SUB_REPEATCMD )
      ch->tempnum = SUB_REPEATCMD;
    else
      ch->tempnum = SUB_NONE;

    ch->substate = SUB_IDEA_USEDESC;
    ch->dest_buf = Idea;
    start_editing(ch, Idea->useDesc, MSL);
    saveIdeas();
    return;
  }
  else if( !str_cmp(arg1, "delete") )
  {
    if( NULLSTR(arg2) )
    {
      send_to_char( "Delete which idea?\n\rSyntax: idea delete <number>\n\r", ch );
      return;
    }

    cnt = 0;
    for( Idea = firstIdea; Idea; Idea = Idea->next )
    {
      cnt++;
      if( cnt == atoi(arg2) )
        break;
    }

    if( !Idea )
    {
      send_to_char( "No such idea.\n\r", ch );
      return;
    }
    ch_printf(ch, "Idea #%d deleted.\n\r", cnt );
    freeOneIdea(Idea);
    saveIdeas();
    return;
  }
  else
  {
    send_to_char( "\n\r&WSyntax: idea\n\rSyntax: idea submit\n\r"
                  "Syntax: idea show <number>\n\rSyntax: idea use <number> <level> <bonus>\n\r", ch );
    return;
  }
  return;
}



void freeOneTypo( TYPO_DATA *typo )
{
  UNLINK( typo, firstTypo, lastTypo, next, prev );
  STRFREE(typo->typoDesc);
  STRFREE(typo->foundBy);
  STRFREE(typo->foundWhen);
  STRFREE(typo->fixedBy);
  STRFREE(typo->fixedWhen);
  DISPOSE(typo);
}


void freeTypos( void )
{
  TYPO_DATA *typo;

  for( typo = firstTypo; typo; typo = typo->next )
    freeOneTypo(typo);
}


void saveTypos( void )
{
  FILE *fp = NULL;
  TYPO_DATA *typo = NULL;
  char fname[MFL];

  snprintf(fname, MFL, "%s%s", SYSTEM_DIR, TYPO_FILE );
  if( !(fp = fopen(fname, "w")) )
  {
    bug( "%s: Couldn't open %s for writing", __FUNCTION__, fname );
    return;
  }

  for( typo = firstTypo; typo; typo = typo->next )
  {
    fprintf(fp, "#TYPO\n" );
    fprintf(fp, "FoundBy       %s~\n",	typo->foundBy	 );
    fprintf(fp, "FoundWhen     %s~\n",  typo->foundWhen );
    fprintf(fp, "TypoDesc      %s~\n",  typo->typoDesc	 );
    fprintf(fp, "Type          %d\n",	typo->type	 );
    fprintf(fp, "FixedBy       %s~\n", 	typo->fixedBy	 );
    fprintf(fp, "FixedWhen     %s~\n", 	typo->fixedWhen );
    fprintf(fp, "Reward        %d\n",  	typo->reward	 );
    fprintf(fp, "RewardBonus   %d\n",  	typo->bonus	 );
    fprintf(fp, "Room          %d\n",  	typo->room 	 );
    fprintf(fp, "End\n\n"                                );
  }
  fprintf(fp, "#END\n");
  FCLOSE(fp);
  return;
}


void fReadTypo( TYPO_DATA *typo, FILE *fp )
{
  const char *word;
  bool fMatch;

  for( ; ; )
  {
    word = feof(fp) ? "End" : fread_word(fp);
    fMatch = FALSE;
    switch(UPPER(word[0]))
    {
      case '*':
        fMatch = TRUE;
        fread_to_eol(fp);
        break;

      case 'E':
        if( !str_cmp(word, "End") )
        {
          nstralloc(&typo->typoDesc);
          nstralloc(&typo->foundBy);
          nstralloc(&typo->foundWhen);
          nstralloc(&typo->fixedBy);
          nstralloc(&typo->fixedWhen);
          return;
        }
        break;

      case 'F':
        KEY( "FixedBy",         typo->fixedBy,         fread_string(fp) );
        KEY( "FixedWhen",       typo->fixedWhen,       fread_string(fp) );
        KEY( "FoundBy",         typo->foundBy,         fread_string(fp) );
        KEY( "FoundWhen",       typo->foundWhen,       fread_string(fp) );
        break;

      case 'R':
        KEY( "Reward",          typo->reward,           fread_number(fp) );
        KEY( "RewardBonus",     typo->bonus,            fread_number(fp) );
        KEY( "Room",            typo->room,             fread_number(fp) );
        break;

      case 'T':
        KEY( "Type",            typo->type,             fread_number(fp) );
        KEY( "TypoDesc",        typo->typoDesc,         fread_string(fp) );
        break;
    }

    if( !fMatch )
      bug( "%s: no match: %s", __FUNCTION__, word );
  }
}


void loadTypos( void )
{
  char fname[MFL];
  TYPO_DATA *typo;
  FILE *fp;

  firstTypo = NULL;
  lastTypo = NULL;
  snprintf(fname, MFL, "%s%s", SYSTEM_DIR, TYPO_FILE );
  if( (fp = fopen(fname, "r")) != NULL )
  {
    for ( ; ; )
    {
      char letter;
      const char *word;

      letter = fread_letter(fp);
      if( letter == '*' )
      {
        fread_to_eol(fp);
        continue;
      }

      if( letter != '#' )
      {
        bug( "%s: # not found.", __FUNCTION__ );
        break;
      }

      word = fread_word(fp);
      if( !str_cmp(word, "TYPO") )
      {
        CREATE(typo, TYPO_DATA, 1);
        fReadTypo(typo, fp);
        LINK(typo, firstTypo, lastTypo, next, prev);
        continue;
      }
      else if( !str_cmp(word, "END") )
        break;
      else
      {
        bug( "%s: bad section: %s", __FUNCTION__, word );
        continue;
      }
    }
    FCLOSE(fp);
  }
  return;
}


char * typoLevel( short type )
{
  switch(type)
  {
    case 1:  return "misspelled";	break;
    case 2:  return "misused";		break;
    case 3:  return "offensive";	break;
    default: return "Unknown??";	break;
  }
}


short typoLevelArg( char *arg )
{
  if( !str_cmp(arg, "misspelled") )	return 1;
  else if( !str_cmp(arg, "misused") )	return 2;
  else if( !str_cmp(arg, "offensive") )	return 3;
  else return -1;
}


void typoReward( CHAR_DATA *ch, TYPO_DATA *typo )
{
  int gold = 0;
  int glory = 0;
  char buf1[MSL], buf2[MSL];

  if( typo->reward )
    return;

  if( !str_cmp(typo->foundBy, ch->name) )
  {
    switch(typo->type)
    {
      case 1: gold = 10000; glory = 5;  break;
      case 2: gold = 20000; glory = 10; break;
      case 3: gold = 30000; glory = 15; break;
      default: break;
    }

    if( typo->bonus == 1 )
    {
      gold = gold + (gold / 2);
      glory = glory + (glory / 2);
    }
    else if( typo->bonus == 2 )
    {
      gold = gold * 2;
      glory = glory * 2;
    }
    else if( typo->bonus == 3 )
    {
      gold = (gold * 2) + (gold / 2);
      glory = (glory * 2) + (glory / 2);
    }
    snprintf(buf1, MSL, "%s", num_punct(gold) );
    snprintf(buf2, MSL, "%s", num_punct(glory) );
    if( gold == 0 && glory == 0 )
    {
      ch_printf( ch, "The Immortals did not find it fit to reward you for your reported typo!\n\r" );
      ch_printf( ch, "The typo was probably submitted prior to your discovery.  Keep looking though!\n\r" );
    }
    else
      ch_printf(ch, "The Immortals have rewarded you %s gold and %s glory for finding %s typo!\n\r", buf1, buf2, aoran(typoLevel(typo->type)) );

    typo->reward = TRUE;
    ch->pcdata->num_typos++;
    ch->pcdata->typoReward1 += gold;
    ch->pcdata->typoReward2 += glory;
    ch->gold += gold;
    ch->pcdata->quest_curr += glory;
    send_to_pager( "&CNote from the Immortals\n\r&B----------------------------------------------------------------------&W\n\r", ch );
    send_to_pager( typo->typoDesc, ch );
    send_to_pager( "\n\r", ch);
    save_char_obj(ch);
    saveTypos();
    return;
  }
  return;
}


CMDF do_typo( CHAR_DATA *ch, char *argument )
{
  char arg1[MIL];
  char arg2[MIL];
  TYPO_DATA *typo = NULL, *nTypo;
  short cnt = 0;
  DESCRIPTOR_DATA *d;

  if( IS_NPC(ch) )
  {
    send_to_char( "Huh?\n\r", ch );
    return;
  }

  switch(ch->substate)
  {
    default:
      break;

    case SUB_RESTRICTED:
      send_to_char( "You cannot do this while in another command.\n\r", ch );
      return;

    case SUB_TYPO_DESC:
      nTypo = (TYPO_DATA *) ch->dest_buf;
      STRFREE(nTypo->typoDesc);
      nTypo->typoDesc = copy_buffer(ch);
      stop_editing(ch);
      saveTypos();
      ch->substate = ch->tempnum;
      send_to_char( "Thanks, your typo report has been recorded.\n\r", ch );
      send_to_char( "You will be notified when the typo is fixed.\n\r", ch );
      log_printf_plus( LOG_NORMAL, LEVEL_IMMORTAL, "%s has reported a typo.", ch->name );
      return;
  }

  argument = one_argument(argument, arg1);
  argument = one_argument(argument, arg2);
  if( NULLSTR(arg1) )
  {
    if( IS_IMMORTAL(ch) )
    {
      send_to_char( "&CNum   Room    Date        Name\n\r", ch );
      send_to_char( "&B--------------------------------------\n\r", ch );
      for( typo = firstTypo; typo; typo = typo->next )
      {
        cnt++;
        ch_printf(ch, "%s%3d&C>  &W%-6d  %-10s  %-10s\n\r",
          NULLSTR(typo->fixedBy) ? "&R" : "&W", cnt, typo->room, typo->foundWhen, typo->foundBy );
      }

      if( cnt <= 0 )
      {
        send_to_char( "No typos have been reported.\n\r", ch );
        return;
      }
      send_to_char( "&B--------------------------------------\n\r", ch );
      send_to_char( "\n\r&WSyntax: typo\n\rSyntax: typo report <typo>\n\r"
                    "Syntax: typo show <number>\n\rSyntax: typo fixed <number> <level> <bonus>\n\r", ch );
    }
    else
    {
      send_to_char( "&CTypos\n\rLv Degree     Gold   Glory\n\r", ch );
      send_to_char( "&B--------------------------\n\r", ch );
      send_to_char( "&W1  misspelled 10K    5\n\r", ch );
      send_to_char( "&W2  misused    20K    10\n\r", ch );
      send_to_char( "&W3  offensive  30K    15\n\r", ch );
      send_to_char( "&B--------------------------\n\r", ch );
      send_to_char( "&wTypos are words in the game that are either misspelled, misused, or offensive.\n\r", ch );
      send_to_char( "If you want to report a typo, type &wtypo report\n\r", ch );
      send_to_char( "&WYour name, location and the current date are automatically recorded.\n\r", ch );
    }
    return;
  }

  if( !str_cmp(arg1, "report") )
  {
    CREATE(nTypo, TYPO_DATA, 1);
    LINK(nTypo, firstTypo, lastTypo, next, prev);
    nTypo->foundBy	= STRALLOC(ch->name);
    nTypo->foundWhen	= STRALLOC(timeStamp());
    nTypo->type		= -1;
    nTypo->reward	= -1;
    nTypo->room		= ch->in_room ? ch->in_room->vnum : -1;
    nTypo->fixedBy	= STRALLOC("");
    nTypo->fixedWhen	= STRALLOC("");
    nTypo->typoDesc	= STRALLOC("");
    if( ch->substate == SUB_REPEATCMD )
      ch->tempnum = SUB_REPEATCMD;
    else
      ch->tempnum = SUB_NONE;

    ch->substate = SUB_TYPO_DESC;
    ch->dest_buf = nTypo;
    start_editing(ch, nTypo->typoDesc, MSL);
    saveTypos();
    return;
  }

  if( !IS_IMMORTAL(ch) )
  {
    send_to_char( "Huh?\n\r", ch );
    return;
  }

  if( !str_cmp(arg1, "show") )
  {
    if( NULLSTR(arg2) )
    {
      send_to_char( "Show which typo?\n\rSyntax: typo show <number>\n\r", ch );
      return;
    }

    cnt = 0;
    for( typo = firstTypo; typo; typo = typo->next )
    {
      cnt++;
      if( cnt == atoi(arg2) )
        break;
    }

    if( !typo )
    {
      send_to_char( "No such typo.\n\r", ch );
      return;
    }
 
    send_to_char( "&CNum   Room    Date        Name\n\r", ch );
    send_to_char( "&B----------------------------------------------------------------------\n\r", ch );
    ch_printf(ch, "%s%3d&C>  &W%-6d  %-10s  %-10s\n\r",
      NULLSTR(typo->fixedBy) ? "&R" : "&W", cnt, typo->room, typo->foundWhen, typo->foundBy );
    send_to_char( "&B----------------------------------------------------------------------\n\r", ch );
    ch_printf(ch, "&CDescription\n\r&W%s\n\r", typo->typoDesc );
    send_to_char( "&B----------------------------------------------------------------------\n\r", ch );
    if( !NULLSTR(typo->fixedBy) )
    {
      ch_printf(ch, "&CUsed by : &W%s\n\r", typo->fixedBy );
      ch_printf(ch, "&CWhen    : &W%s\n\r", typo->fixedWhen );
      ch_printf(ch, "&CLevel   : &W%s\n\r", typoLevel(typo->type) );
      ch_printf(ch, "&CRewarded: &W%s\n\r", typo->reward ? "Yes" : "No" );
      if( typo->bonus > 0 )
        ch_printf(ch, "&CBonus   : &W%s\n\r", typo->bonus == 1 ? "+50%" : typo->bonus == 2 ? "x2" :
        typo->bonus > 2 ? "+50% x2" : "None" );
    }
    return;
  }
  else if( !str_cmp(arg1, "fixed") )
  {
    char arg3[MIL];
    char arg4[MIL];

    argument = one_argument(argument, arg3);
    argument = one_argument(argument, arg4);
    if( NULLSTR(arg2) )
    {
      send_to_char( "&WWhich typo is fixed?\n\rSyntax: typo use <number> <level> <bonus>\n\r", ch );
      send_to_char( "Level can be:\n\r  &wmisspelled misused offensive\n\r", ch );
      send_to_char( "&WBonus can be either: &wnone bonus1 bonus2 &Wor &wboth\n\r", ch );
      send_to_char( "\n\r&RDo not set a typo as fixed until you are positive it has been corrected.\n\r", ch );
      return;
    }

    cnt = 0;
    for( typo = firstTypo; typo; typo = typo->next )
    {
      cnt++;
      if( cnt == atoi(arg2) )
        break;
    }

    if( !typo )
    {
      send_to_char( "No such typo.\n\r", ch );
      return;
    }

    if( NULLSTR(arg3) )
    {
      send_to_char( "&WHow wrong was the typo you corrected?\n\rValid types are:  &wnone misspelled misused offensive\n\r", ch );
      return;
    }

    if( NULLSTR(arg4) )
    {
      send_to_char( "&WAre you giving a bonus for this typo?\n\rValid types are: &wnone bonus1 bonus2 both\n\r", ch );
      return;
    }

    if( !str_cmp(arg4, "bonus1") )
      typo->bonus = 1;
    else if( !str_cmp(arg4, "bonus2") )
      typo->bonus = 2;
    else if( !str_cmp(arg4, "both") )
      typo->bonus = 3;
    else if( !str_cmp(arg4, "none" ) )
      typo->bonus = -1;
    else
    {
      send_to_char( "If you're giving a bonus to the typo reporter, use bonus1, bonus2 or both. If not, use none.\n\r", ch );
      return;
    }

    if( !NULLSTR(typo->fixedBy) )
    {
      send_to_char( "That typo has already been used.\n\r", ch );
      return;
    }

    typo->fixedBy	= STRALLOC(ch->name);
    typo->fixedWhen	= STRALLOC(timeStamp());
    typo->type		= typoLevelArg(arg3);
    typo->reward	= FALSE;
    send_to_char( "Ok, the typo has been corrected and the player who reported it will be notified.\n\r", ch );
    saveTypos();
    for( d = first_descriptor; d; d = d->next )
    {
      if( !d->character )
        continue;
      checkBuidty(d->character);
    }
    return;
  }
  else if( !str_cmp(arg1, "delete") )
  {
    if( NULLSTR(arg2) )
    {
      send_to_char( "Delete which typo?\n\rSyntax: typo delete <number>\n\r", ch );
      return;
    }

    cnt = 0;
    for( typo = firstTypo; typo; typo = typo->next )
    {
      cnt++;
      if( cnt == atoi(arg2) )
        break;
    }

    if( !typo )
    {
      send_to_char( "No such typo.\n\r", ch );
      return;
    }

    ch_printf(ch, "typo #%d deleted.\n\r", cnt );
    freeOneTypo(typo);
    saveTypos();
    return;
  }
  else
  {
    send_to_char( "\n\r&WSyntax: typo\n\rSyntax: typo report <typo>\n\r"
                  "Syntax: typo show <number>\n\rSyntax: typo fixed <number> <level> <bonus>\n\r", ch );
    return;
  }
  return;
}


void checkBuidty( CHAR_DATA *ch )
{
  BUG_DATA *Bug;
  IDEA_DATA *idea;
  TYPO_DATA *typo;
  int bcnt = 0, icnt= 0, tcnt = 0;

  if( !ch )
  {
    bug( "%s: NULL ch!", __FUNCTION__ );
    return;
  }

  for( Bug = firstBug; Bug; Bug = Bug->next )
  {
    if( NULLSTR(Bug->fixedBy) )
    {
      bcnt++;
      continue;
    }

    if( Bug->reward )
      continue;

    if( !str_cmp(Bug->foundBy, ch->name) )
      bugReward(ch, Bug);
  }

  for( idea = firstIdea; idea; idea = idea->next )
  {
    if( NULLSTR(idea->usedBy) )
    {
      icnt++;
      continue;
    }

    if( idea->reward )
      continue;

    if( !str_cmp(idea->madeBy, ch->name) )
      ideaReward(ch, idea);
  }

  for( typo = firstTypo; typo; typo = typo->next )
  {
    if( NULLSTR(typo->fixedBy) )
    {
      tcnt++;
      continue;
    }
 
    if( typo->reward )
      continue;

    if( !str_cmp(typo->foundBy, ch->name) )
      typoReward(ch, typo);
  }

  if( IS_IMMORTAL(ch) )
  {
    if( bcnt > 0 )
      ch_printf(ch, "&R+++ &WThere are }R%d &d&Wgame issues that need resolving. Type &GBUG &Wto view. &R+++&D\n\r", bcnt );

    if( icnt > 0 )
      ch_printf(ch, "&R+++ &WThere are }R%d &d&Wideas that need reviewing. Type &GIDEA &Wto view. &R+++&D\n\r", icnt );

    if( tcnt > 0 )
      ch_printf(ch, "&R+++ &WThere are }R%d &d&Wtypos that need to be fixed. Type &GTYPO &Wto view. &R+++&D\n\r", tcnt );
  }
  return;
}


void pruneBuidty( void )
{
  BUG_DATA *Bug = NULL;
  IDEA_DATA *Idea = NULL;
  TYPO_DATA *Typo = NULL;

  for( Bug = firstBug; Bug; Bug = Bug->next )
  {
    if( Bug && Bug->reward == TRUE )
      freeOneBug(Bug);
  }
  saveBugs();

  for( Idea = firstIdea; Idea; Idea = Idea->next )
  {
    if( Idea && Idea->reward == TRUE )
      freeOneIdea(Idea);
  }
  saveIdeas();

  for( Typo = firstTypo; Typo; Typo = Typo->next )
  {
    if( Typo && Typo->reward == TRUE )
      freeOneTypo(Typo);
  }
  saveTypos();
  return;
}
