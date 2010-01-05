/***************************************************************************
 *  Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,        *
 *  Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.   *
 *                                                                         *
 *  Merc Diku Mud improvments copyright (C) 1992, 1993 by Michael          *
 *  Chastain, Michael Quan, and Mitchell Tse.                              *
 *                                                                         *
 *  In order to use any part of this Merc Diku Mud, you must comply with   *
 *  both the original Diku license in 'license.doc' as well the Merc       *
 *  license in 'license.txt'.  In particular, you may not remove either of *
 *  these copyright notices.                                               *
 *                                                                         *
 *  Much time and thought has gone into this software and you are          *
 *  benefitting.  We hope that you share your changes too.  What goes      *
 *  around, comes around.                                                  *
 ***************************************************************************/

#if defined(macintosh)
#include <types.h>
#else
#include <sys/types.h>
#endif
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "mud.h"


/*
 
 Note Board system, (c) 1995-96 Erwin S. Andreasen, erwin@pip.dknet.dk
 =====================================================================
 
 Basically, the notes are split up into several boards. The boards do not
 exist physically, they can be read anywhere and in any position.
 
 Each of the note boards has its own file. Each of the boards can have its own
 "rights": who can read/write.
 
 Each character has an extra field added, namele the timestamp of the last note
 read by him/her on a certain board.
 
 The note entering system is changed too, making it more interactive. When
 entering a note, a character is put AFK and into a special CON_ state.
 Everything typed goes into the note.
 
 For the immortals it is possible to purge notes based on age. An Archive
 options is available which moves the notes older than X days into a special
 board. The file of this board should then be moved into some other directory
 during e.g. the startup script and perhaps renamed depending on date.
 
 Note that write_level MUST be >= read_level or else there will be strange
 output in certain functions.
 
 Board DEFAULT_BOARD must be at least readable by *everyone*.
 
*/ 

#define L_SUP (MAX_LEVEL - 1) /* if not already defined */


GLOBAL_BOARD_DATA boards[MAX_BOARD] =
{
    { "General",  	"General discussion",            0,     1,     "all", DEF_INCLUDE, 60, NULL, FALSE },
    { "News",		"News of recent events",         0,     200, "all", DEF_NORMAL, 60, NULL, FALSE },
    { "Questions",	"FAQ's, Questions, Help",        0,     1,     "all", DEF_NORMAL, 60, NULL, FALSE },
    { "Announce", 	"Announcements from AI",		 0,     200, "all", DEF_NORMAL, 60, NULL, FALSE },
    { "Immortal",   "AI discussion",		       200,	    200, "all", DEF_INCLUDE, 21, NULL, FALSE },
    { "Personal", 	"Personal messages",	      	 0,     1,     "all", DEF_EXCLUDE,28, NULL, FALSE },
};

/* The prompt that the character is given after finishing a note with ~ or END */
const char * szFinishPrompt = "&B(&CC&B)&Continue, &B(&CV&B)&Ciew, &B(&CP&B)&Cost or &B(&CF&B)&Corget it?";

long last_note_stamp = 0; /* To generate unique timestamps on notes */

#define BOARD_NOACCESS -1
#define BOARD_NOTFOUND -1

bool next_board (CHAR_DATA *ch);


/* recycle a note */
void free_global_note (NOTE_DATA *note)
{
    if( note->sender )
        STRFREE( note->sender );
    if( note->to_list )
        STRFREE( note->to_list );
    if( note->subject )
        STRFREE( note->subject );
    if( note->date ) /* was note->datestamp for some reason */
        STRFREE( note->date  );
    if( note->text )
        STRFREE( note->text );

    note->next = note_free;
    note_free = note;
}

/* allocate memory for a new note or recycle */
/*
 * This should be redone, btw. It should work as it is, but smaug really
 * doesn't really bother much with recyling... why start now?
 * Also means redoing function above... Not difficult to redo, but your
 * choice out there.
 */
NOTE_DATA *new_note ()
{
    NOTE_DATA *note;

    if( note_free )
    {
        note = note_free;
        note_free = note_free->next;
    }
    else
        CREATE( note, NOTE_DATA, 1 );

    note->next = NULL;
    note->sender = NULL;
    note->expire = 0;
    note->to_list = NULL;
    note->subject = NULL;
    note->date = NULL;
    note->date_stamp = 0;
    note->text = NULL;

    return note;
}

/* append this note to the given file */
void append_note (FILE *fp, NOTE_DATA *note)
{
    fprintf (fp, "Sender  %s~\n", note->sender);
    fprintf (fp, "Date    %s~\n", note->date);
    fprintf (fp, "Stamp   %ld\n", note->date_stamp);
    fprintf (fp, "Expire  %ld\n", note->expire);
    fprintf (fp, "To      %s~\n", note->to_list);
    fprintf (fp, "Subject %s~\n", note->subject);
    fprintf (fp, "Text\n%s~\n\n", note->text);
}

/* Save a note in a given board */
void finish_note ( GLOBAL_BOARD_DATA *board, NOTE_DATA *note )
{
    FILE *fp;
    NOTE_DATA *p;
    char filename[200];

    /* The following is done in order to generate unique date_stamps */

    if (last_note_stamp >= current_time)
        note->date_stamp = ++last_note_stamp;
    else
    {
        note->date_stamp = current_time;
        last_note_stamp = current_time;
    }

    if (board->note_first) /* are there any notes in there now? */
    {
        for (p = board->note_first; p->next; p = p->next )
            ; /* empty */

        p->next = note;
    }
    else /* nope. empty list. */
        board->note_first = note;

    /* append note to note file */

    sprintf (filename, "%s%s", NOTE_DIR, board->short_name);

    fp = fopen (filename, "a");
    if (!fp)
    {
        bug ("Could not open one of the note files in append mode",0);
        board->changed = TRUE; /* set it to TRUE hope it will be OK later? */
        return;
    }

    append_note (fp, note);
    fclose (fp);
}

/* Find the number of a board */
int board_number( const GLOBAL_BOARD_DATA *board )
{
    int i;

    for (i = 0; i < MAX_BOARD; i++)
        if (board == &boards[i])
            return i;

    return -1;
}

/* Find a board number based on  a string */
int board_lookup (const char *name)
{
    int i;

    for (i = 0; i < MAX_BOARD; i++)
        if (!str_cmp (boards[i].short_name, name))
            return i;

    return -1;
}

/* Remove list from the list. Do not free note */
void unlink_note( GLOBAL_BOARD_DATA *board, NOTE_DATA *note )
{
    NOTE_DATA *p;

    if (board->note_first == note)
        board->note_first = note->next;
    else
    {
        for (p = board->note_first; p && p->next != note; p = p->next);
        if (!p)
            bug ("unlink_note: could not find note.",0);
        else
            p->next = note->next;
    }
}

/* Find the nth note on a board. Return NULL if ch has no access to that note */
NOTE_DATA* find_note (CHAR_DATA *ch, GLOBAL_BOARD_DATA *board, int num)
{
    int count = 0;
    NOTE_DATA *p;

    for (p = board->note_first; p ; p = p->next)
        if (++count == num)
            break;

    if ( (count == num) && gb_is_note_to (ch, p))
        return p;
    else
        return NULL;

}

/* save a single board */
void save_board( GLOBAL_BOARD_DATA *board )
{
    FILE *fp;
    char filename[200];
    char buf[200];
    NOTE_DATA *note;

    sprintf (filename, "%s%s", NOTE_DIR, board->short_name);

    fp = fopen (filename, "w");
    if (!fp)
    {
        sprintf (buf, "Error writing to: %s", filename);
        bug (buf, 0);
    }
    else
    {
        for (note = board->note_first; note ; note = note->next)
            append_note (fp, note);
			
        fclose (fp);
    }
}

/* Show one note to a character */
void show_note_to_char (CHAR_DATA *ch, NOTE_DATA *note, int num)
{
    char buf[4*MAX_STRING_LENGTH];

    /* Ugly colors ? */
    sprintf (buf,
             "&B(&W%4d&B) &W&W%s&w&B: &W%s&w\n\r"
             "&W&CDate&B:&W  %s\n\r"
             "&W&CTo&B:&W    %s\n\r"
             "&B---------------------------------------------------------------------------&W\n\r"
             "&W%s\n\r"
	     "&B---------------------------------------------------------------------------&W\n\r",
             num, note->sender, note->subject,
             note->date,
             note->to_list,
             note->text );

    send_to_char( "&B---------------------------------------------------------------------------&W\n\r", ch );
    send_to_char_color (buf,ch);
}

/* Save changed boards */
void save_notes ()
{
    int i;

    for (i = 0; i < MAX_BOARD; i++)
        if (boards[i].changed) /* only save changed boards */
            save_board (&boards[i]);
}

/* Load a single board */
void load_board( GLOBAL_BOARD_DATA *board )
{
    FILE *fp, *fp_archive;
    NOTE_DATA *last_note;
    char filename[200];

    sprintf (filename, "%s%s", NOTE_DIR, board->short_name);

    fp = fopen (filename, "r");

    /* Silently return */
    if (!fp)
        return;

    /* Start note fetching. copy of db.c:load_notes() */

    last_note = NULL;

    for ( ; ; )
    {
        NOTE_DATA *pnote;
        char letter;

        do
        {
            letter = getc( fp );
            if ( feof(fp) )
            {
                fclose( fp );
                return;
            }
        }
        while ( isspace(letter) );
        ungetc( letter, fp );

        CREATE( pnote, NOTE_DATA, sizeof( *pnote ) );

        if ( str_cmp( fread_word( fp ), "sender" ) )
            break;
        pnote->sender     = fread_string( fp );

        if ( str_cmp( fread_word( fp ), "date" ) )
            break;
        pnote->date       = fread_string( fp );

        if ( str_cmp( fread_word( fp ), "stamp" ) )
            break;
        pnote->date_stamp = fread_number( fp );

        if ( str_cmp( fread_word( fp ), "expire" ) )
            break;
        pnote->expire = fread_number( fp );

        if ( str_cmp( fread_word( fp ), "to" ) )
            break;
        pnote->to_list    = fread_string( fp );

        if ( str_cmp( fread_word( fp ), "subject" ) )
            break;
        pnote->subject    = fread_string( fp );

        if ( str_cmp( fread_word( fp ), "text" ) )
            break;
        pnote->text       = fread_string( fp );
        
        pnote->next = NULL; /* jic */

        /* Should this note be archived right now ? */

        if (pnote->expire < current_time)
        {
            char archive_name[200];

            sprintf (archive_name, "%s%s.old", NOTE_DIR, board->short_name);
            fp_archive = fopen (archive_name, "a");
            if (!fp_archive)
                bug ("Could not open archive boards for writing",0);
            else
            {
                append_note (fp_archive, pnote);
                fclose (fp_archive); /* it might be more efficient to close this later */
            }

            free_global_note (pnote);
            board->changed = TRUE;
            continue;

        }


        if ( board->note_first == NULL )
            board->note_first = pnote;
        else
            last_note->next     = pnote;

        last_note         = pnote;
    }

    bug( "Load_notes: bad key word.", 0 );
    return; /* just return */
}

/* Initialize structures. Load all boards. */
void load_global_boards ()
{
    int i;

    for (i = 0; i < MAX_BOARD; i++)
        load_board (&boards[i]);
}

/* Returns TRUE if the specified note is address to ch */
bool gb_is_note_to (CHAR_DATA *ch, NOTE_DATA *note)
{
    if (!str_cmp (ch->name, note->sender))
        return TRUE;

    if (is_name ("all", note->to_list))
        return TRUE;

    if (IS_IMMORTAL(ch) && (
                            is_name ("imm", note->to_list) ||
                            is_name ("imms", note->to_list) ||
                            is_name ("immortal", note->to_list) ||
                            is_name ("god", note->to_list) ||
                            is_name ("gods", note->to_list) ||
                            is_name ("immortals", note->to_list)))
        return TRUE;

    if ((get_trust(ch) == MAX_LEVEL) && (
                                         is_name ("imp", note->to_list) ||
                                         is_name ("imps", note->to_list) ||
                                         is_name ("implementor", note->to_list) ||
                                         is_name ("implementors", note->to_list)))
        return TRUE;

    if (is_name (ch->name, note->to_list))
        return TRUE;

    /* Allow a note to e.g. 40 to send to characters level 40 and above */
    if (is_number(note->to_list) && get_trust(ch) >= atoi(note->to_list))
        return TRUE;

    return FALSE;
}

/* Return the number of unread notes 'ch' has in 'board' */
/* Returns BOARD_NOACCESS if ch has no access to board */
int unread_notes (CHAR_DATA *ch, GLOBAL_BOARD_DATA *board)
{
    NOTE_DATA *note;
    time_t last_read;
    int count = 0;

    if (board->read_level > get_trust(ch))
        return BOARD_NOACCESS;

    last_read = ch->pcdata->last_note[board_number(board)];

    for (note = board->note_first; note; note = note->next)
        if (gb_is_note_to(ch, note) && ((long)last_read < (long)note->date_stamp))
            count++;

    return count;
}

/*
 * COMMANDS
 */

/* Start writing a note */
void do_gwrite (CHAR_DATA *ch, char *argument)
{
    char *strtime;
    char buf[200];

    if (IS_NPC(ch)) /* NPC cannot post notes */
        return;

    if ( NOT_AUTHED(ch) )
    {
      send_to_char("&RYou must be authorized to use the Global Boards.\n\r", ch);
      return;
    }

    if (get_trust(ch) < ch->pcdata->board->write_level)
    {
        send_to_char_color ("&RYou cannot post notes on this board.\n\r",ch);
        return;
    }

    /* continue previous note, if any text was written*/
    if (ch->pcdata->in_progress && (!ch->pcdata->in_progress->text))
    {
        send_to_char_color ("&RNote in progress cancelled because you did not manage to write any text before losing link.\n\r\n\r",ch);
        free_global_note (ch->pcdata->in_progress);
        ch->pcdata->in_progress = NULL;
    }


    if (!ch->pcdata->in_progress)
    {
        ch->pcdata->in_progress = new_note();
        ch->pcdata->in_progress->sender = STRALLOC(ch->name);

        /* convert to ascii. ctime returns a string which last character is \n, so remove that */
        strtime = ctime (&current_time);
        strtime[strlen(strtime)-1] = '\0';

        ch->pcdata->in_progress->date = STRALLOC (strtime);
    }

    act (AT_GREEN, "$n started writing a note.&w", ch, NULL, NULL, TO_ROOM);

    /* Begin writing the note ! */
    sprintf (buf, "&CYou are now &G%s &Ca new note on the &W%s&C board.\n\r"
             "&R*&YIf you are using tintin, type #verbose to turn off alias expansion!&R*&W\n\r\n\r",
             ch->pcdata->in_progress->text ? "continuing" : "posting",
             ch->pcdata->board->short_name);
    send_to_char_color (buf,ch);

    sprintf (buf, "&z&CFrom&B:&W    %s\n\r\n\r", ch->name);
    send_to_char_color (buf,ch);

    if (!ch->pcdata->in_progress->text) /* Are we continuing an old note or not? */
    {
        switch (ch->pcdata->board->force_type)
        {
        case DEF_NORMAL:
            sprintf (buf, "&CIf you press &GReturn&C, default recipient \"&W%s&C\" will be chosen.&W\n\r",
                     ch->pcdata->board->names);
            break;
        case DEF_INCLUDE:
            sprintf (buf, "&CThe recipient list &RMUST &Cinclude \"&W%s&C\". If not, it will be added automatically.\n\r",
                     ch->pcdata->board->names);
            break;

        case DEF_EXCLUDE:
            sprintf (buf, "&CThe recipient of this note must &RNOT &Cinclude&B: \"&W%s&C\".",
                     ch->pcdata->board->names);

            break;
        }

        send_to_char_color (buf,ch);
        send_to_char_color ("\n\r&z&CTo&B:&W      ",ch);

        ch->desc->connected = CON_NOTE_TO;
        /* nanny takes over from here */

    }
    else /* we are continuing, print out all the fields and the note so far*/
    {
        sprintf (buf, "&z&CTo&B:&W      %s\n\r"
                 "&z&CExpires&B:&W %s\n\r"
                 "&z&CSubject&B:&W %s\n\r",
                 ch->pcdata->in_progress->to_list,
                 ctime(&ch->pcdata->in_progress->expire),
                 ch->pcdata->in_progress->subject);
        send_to_char_color (buf,ch);
        send_to_char_color ("&CYour note so far&B:&W\n\r&w",ch);
        send_to_char_color (ch->pcdata->in_progress->text,ch);

        send_to_char_color ("\n\r&CEnter text. Type &W~&C or &WEND&C on an empty line to end note.\n\r"
		      "&Y*&CKeep the line of text as long as the border below.&W\n\r"
                      "&B--------------------------------------------------------------------------------&W\n\r",ch);


        ch->desc->connected = CON_NOTE_TEXT;

    }

}


/* Read next note in current group. If no more notes, go to next board */
void do_gread (CHAR_DATA *ch, char *argument)
{
    NOTE_DATA *p;
    int count = 0, number;
    time_t *last_note = &ch->pcdata->last_note[board_number(ch->pcdata->board)];

    if ( NOT_AUTHED(ch) )
    {
      send_to_char("&RYou must be authorized to use the Global Boards.&W\n\r", ch);
      return;
    }

    if (!str_cmp(argument, "again"))
    { /* read last note again */

    }
    else if (is_number (argument))
    {
        number = atoi(argument);

        for (p = ch->pcdata->board->note_first; p; p = p->next)
            if (++count == number)
                break;

        if (!p || !gb_is_note_to(ch, p))
            send_to_char_color ("&RNo such note.&W\n\r",ch);
        else
        {
            show_note_to_char (ch,p,count);
            *last_note =  UMAX (*last_note, p->date_stamp);
        }
    }
    else /* just next one */
    {
        char buf[200];

        count = 1;
        for (p = ch->pcdata->board->note_first; p ; p = p->next, count++)
            if ((p->date_stamp > *last_note) && gb_is_note_to(ch,p))
            {
                show_note_to_char (ch,p,count);
                /* Advance if new note is newer than the currently newest for that char */
                *last_note =  UMAX (*last_note, p->date_stamp);
                return;
            }

        send_to_char_color ("&RNo new notes in this board.&W\n\r",ch);

        if (next_board (ch))
            sprintf (buf, "&CChanged to next board, &W%s&C.&W\n\r", ch->pcdata->board->short_name);
        else
            sprintf (buf, "&RThere are no more boards.&W\n\r");

        send_to_char_color (buf,ch);
    }
}

/* Remove a note */
void do_gremove (CHAR_DATA *ch, char *argument)
{
    NOTE_DATA *p;

    if (!is_number(argument))
    {
        send_to_char_color ("&CRemove which note?&W\n\r",ch);
        return;
    }

    p = find_note (ch, ch->pcdata->board, atoi(argument));
    if (!p)
    {
        send_to_char_color ("&RNo such note.&W\n\r",ch);
        return;
    }

    if (str_cmp(ch->name,p->sender) && (get_trust(ch) < MAX_LEVEL))
    {
        send_to_char_color ("&RYou are not authorized to remove this note.&W\n\r",ch);
        return;
    }

    unlink_note (ch->pcdata->board,p);
    free_global_note (p);
    send_to_char_color ("&CNote removed!&W\n\r",ch);

    save_board(ch->pcdata->board); /* save the board */
}


/* List all notes or if argument given, list N of the last notes */
/* Shows REAL note numbers! */
void do_glist (CHAR_DATA *ch, char *argument)
{
    int count= 0, show = 0, num = 0, has_shown = 0;
    time_t last_note;
    NOTE_DATA *p;
    char buf[MAX_STRING_LENGTH];


    if (is_number(argument))	 /* first, count the number of notes */
    {
        show = atoi(argument);

        for (p = ch->pcdata->board->note_first; p; p = p->next)
            if (gb_is_note_to(ch,p))
                count++;
    }
	

    send_to_char_color ( "&CNotes on this board&B:&W\n\r"
                   "&CNum&B: &CAuthor        Subject            Board&w\n\r",ch);

    last_note = ch->pcdata->last_note[board_number (ch->pcdata->board)];

    for (p = ch->pcdata->board->note_first; p; p = p->next)
    {
        num++;
        if (gb_is_note_to(ch,p))
        {
            has_shown++; /* note that we want to see X VISIBLE note, not just last X */
            if (!show || ((count-show) < has_shown))
            {
                sprintf (buf, "&W%3d&B: &W%c %-13s %-15s %-10s\n\r",
                         num,
                         last_note < p->date_stamp ? '*' : ' ',
                         p->sender, p->subject,
			 ch->pcdata->board->short_name );
                send_to_char_color (buf,ch);
            }
        }

    }
}

/* catch up with some notes */
void do_gcatchup (CHAR_DATA *ch, char *argument)
{
    NOTE_DATA *p;

    if ( NOT_AUTHED(ch) )
    {
      send_to_char("You must be authorized to use the Global Boards.\n\r", ch);
      return;
    }

    /* Find last note */
    for (p = ch->pcdata->board->note_first; p && p->next; p = p->next);

    if (!p)
        send_to_char_color ("&RThere are no notes on that board.&W\n\r",ch);
    else
    {
        ch->pcdata->last_note[board_number(ch->pcdata->board)] = p->date_stamp;
        send_to_char_color ("&CAll messages skipped.&W\n\r",ch);
    }
}

/* Dispatch function for backwards compatibility */
void do_global_note (CHAR_DATA *ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH];

    if (IS_NPC(ch))
        return;

    if ( NOT_AUTHED(ch) )
    {
      send_to_char("&RYou must be authorized to use the Global Boards.&W\n\r", ch);
      return;
    }

    argument = one_argument (argument, arg);

    if (!ch->pcdata->board)
	ch->pcdata->board = &boards[DEFAULT_BOARD];
	
    if ((!arg[0]) || (!str_cmp(arg, "read"))) /* 'note' or 'note read X' */
        do_gread (ch, argument);

    else if (!str_cmp (arg, "list"))
        do_glist (ch, argument);

    else if (!str_cmp (arg, "write"))
        do_gwrite (ch, argument);

    else if (!str_cmp (arg, "catchup"))
        do_gcatchup (ch, argument);
    else
        do_help (ch, "note");
}

/* Show all accessible boards with their numbers of unread messages OR
 change board. New board name can be given as a number or as a name (e.g.
 board personal or board 4 */
void do_global_boards (CHAR_DATA *ch, char *argument)
{
    int i, count, number;
    char buf[200];

    if (IS_NPC(ch))
        return;

    if (!argument[0]) /* show boards */
    {
        int unread;

        count = 1;
	send_to_char("\n\r", ch );
        send_to_char_color ("&B+-------------------------- &CGlobal Board System &B--------------------------+&W\n\r",ch);
        for (i = 0; i < MAX_BOARD; i++)
        {
            unread = unread_notes (ch,&boards[i]); /* how many unread notes? */
            if (unread != BOARD_NOACCESS)
            {
                sprintf (buf, "&B| &W%2.2d&B) &W%-22.22s&B|&w &C%-32.32s&B| &CPosts&B: &W%s%2.2d &B|&W\n\r",
                         count, boards[i].short_name, boards[i].long_name, 
			 unread ? "&G" : "&W", unread);
                send_to_char_color (buf,ch);
                count++;
            } /* if has access */

        } /* for each board */
        send_to_char_color ("&B+-------------------------------------------------------------------------+&W\n\r",ch);

        if( ch->pcdata->board == NULL )
            ch->pcdata->board = &boards[DEFAULT_BOARD];
        sprintf (buf, "\n\r&CYour current board is &W%s&C.\n\r", ch->pcdata->board->short_name);
        send_to_char_color (buf,ch);

        /* Inform of rights */
        if (ch->pcdata->board->read_level > get_trust(ch))
            send_to_char_color ("&RYou cannot read or write notes on this board.&W\n\r",ch);
        else if (ch->pcdata->board->write_level > get_trust(ch))
            send_to_char_color ("&YYou can only read notes from this board.&W\n\r",ch);
        else
            send_to_char_color ("&GYou can both read and write on this board.&W\n\r",ch);

        return;
    } /* if empty argument */

    if (ch->pcdata->in_progress)
    {
        send_to_char_color ("&RPlease finish your interrupted note first.&W\n\r",ch);
        return;
    }

    /* Change board based on its number */
    if (is_number(argument))
    {
        count = 0;
        number = atoi(argument);
        for (i = 0; i < MAX_BOARD; i++)
            if (unread_notes(ch,&boards[i]) != BOARD_NOACCESS)
                if (++count == number)
                    break;

        if (count == number) /* found the board.. change to it */
        {
            ch->pcdata->board = &boards[i];
            sprintf (buf, "&CCurrent board changed to &W%s&C. &C%s.\n\r",boards[i].short_name,
                     (get_trust(ch) < boards[i].write_level)
                     ? "You can only read here"
                     : "You can both read and write here");
            send_to_char_color (buf,ch);
        }
        else /* so such board */
            send_to_char_color ("&RNo such board.&W\n\r",ch);

        return;
    }

    /* Non-number given, find board with that name */

    for (i = 0; i < MAX_BOARD; i++)
        if (!str_cmp(boards[i].short_name, argument))
            break;

    if (i == MAX_BOARD)
    {
        send_to_char_color ("&RNo such board.&W\n\r",ch);
        return;
    }

    /* Does ch have access to this board? */
    if (unread_notes(ch,&boards[i]) == BOARD_NOACCESS)
    {
        send_to_char_color ("&RNo such board.&W\n\r",ch);
        return;
    }

    ch->pcdata->board = &boards[i];
    sprintf (buf, "&CCurrent board changed to &W%s&C. &C%s.\n\r",boards[i].short_name,
             (get_trust(ch) < boards[i].write_level)
             ? "You can only read here"
             : "You can both read and write here");
    send_to_char_color (buf,ch);
}

/* Send a note to someone on the personal board */
void personal_message (const char *sender, const char *to, const char *subject, const int expire_days, const char *text)
{
    make_note ("Personal", sender, to, subject, expire_days, text);
}

void make_note (const char* board_name, const char *sender, const char *to, const char *subject, const int expire_days, const char *text)
{
    int board_index = board_lookup (board_name);
    GLOBAL_BOARD_DATA *board;
    NOTE_DATA *note;
    char *strtime;

    if (board_index == BOARD_NOTFOUND)
    {
        bug ("make_note: board not found",0);
        return;
    }

    if (strlen(text) > MAX_NOTE_TEXT)
    {
        bug ("make_note: text too long (%d bytes)", strlen(text));
        return;
    }


    board = &boards [board_index];

    note = new_note(); /* allocate new note */

    note->sender = STRALLOC ((char *)sender);
    note->to_list = STRALLOC((char *)to);
    note->subject = STRALLOC ((char *)subject);
    note->expire = current_time + expire_days * 60 * 60 * 24;
    note->text = STRALLOC ((char *)text);

    /* convert to ascii. ctime returns a string which last character is \n, so remove that */
    strtime = ctime (&current_time);
    strtime[strlen(strtime)-1] = '\0';

    note->date = STRALLOC (strtime);

    finish_note (board, note);

}

/* tries to change to the next accessible board */
bool next_board (CHAR_DATA *ch)
{
    int i = board_number (ch->pcdata->board) + 1;

    while ((i < MAX_BOARD) && (unread_notes(ch,&boards[i]) == BOARD_NOACCESS))
        i++;

    if (i == MAX_BOARD)
    {
        /*       return FALSE; */
        /* Would it be better to just start over? */
        ch->pcdata->board = &boards[0];
        return TRUE;
    }
    else
    {
        ch->pcdata->board = &boards[i];
        return TRUE;
    }
}

void handle_con_note_to (DESCRIPTOR_DATA *d, char * argument)
{
    char buf [MAX_INPUT_LENGTH];
    CHAR_DATA *ch = d->character;

    if (!ch->pcdata->in_progress)
    {
        d->connected = CON_PLAYING;
        bug ("nanny: In CON_NOTE_TO, but no note in progress",0);
        return;
    }

    strcpy (buf, argument);
    smash_tilde (buf); /* change ~ to - as we save this field as a string later */

    switch (ch->pcdata->board->force_type)
    {
    case DEF_NORMAL: /* default field */
        if (!buf[0]) /* empty string? */
        {
            ch->pcdata->in_progress->to_list = STRALLOC (ch->pcdata->board->names);
            sprintf (buf, "&CAssumed default recipient&B: &W%s&w\n\r", ch->pcdata->board->names);
				send_to_char_color ( buf, ch);
        }
        else
            ch->pcdata->in_progress->to_list = STRALLOC (buf);

        break;

    case DEF_INCLUDE: /* forced default */
        if (!is_name (ch->pcdata->board->names, buf))
        {
            strcat (buf, " ");
            strcat (buf, ch->pcdata->board->names);
            ch->pcdata->in_progress->to_list = STRALLOC(buf);

            sprintf (buf, "\n\r&CYou did not specify &W%s &Cas recipient, so it was automatically added.\n\r"
                      "&z&CNew To&B :  &W%s&w\n\r",
                     ch->pcdata->board->names, ch->pcdata->in_progress->to_list);
            send_to_char_color (    buf, ch);
        }
        else
            ch->pcdata->in_progress->to_list = STRALLOC (buf);
        break;

    case DEF_EXCLUDE: /* forced exclude */
        if (!buf[0])
        {
            send_to_char_color ( "You must specify a recipient.\n\r"
                             "&CTo&B:&W      ", ch);
            return;
        }

        if (is_name (ch->pcdata->board->names, buf))
        {
            sprintf (buf, "&RYou are not allowed to send notes to %s on this board. Try again.&W\n\r"
                     "&z&CTo&B:&W      ", ch->pcdata->board->names);
            send_to_char_color ( buf, ch);
            return; /* return from nanny, not changing to the next state! */
        }
        else
            ch->pcdata->in_progress->to_list = STRALLOC (buf);
        break;

    }

    send_to_char_color ( "&W&C\n\r&z&CSubject&B:&W ", ch);
    d->connected = CON_NOTE_SUBJECT;
}

void handle_con_note_subject (DESCRIPTOR_DATA *d, char * argument)
{
    char buf [MAX_INPUT_LENGTH];
    CHAR_DATA *ch = d->character;

    if (!ch->pcdata->in_progress)
    {
        d->connected = CON_PLAYING;
        bug ("nanny: In CON_NOTE_SUBJECT, but no note in progress",0);
        return;
    }

    strcpy (buf, argument);
    smash_tilde (buf); /* change ~ to - as we save this field as a string later */

    /* Do not allow empty subjects */

    if (!buf[0])
    {
        send_to_char_color ("&z&RPlease find a meaningful subject!\n\r", ch);
        send_to_char_color ("&z&CSubject&B:&W ", ch);
    }
    else  if (strlen(buf)>60)
    {
        send_to_char_color ( "&RSubect too long.&W\n\r", ch);
    }
    else
        /* advance to next stage */
    {
        ch->pcdata->in_progress->subject = STRALLOC(buf);
        if (IS_IMMORTAL(ch)) /* immortals get to choose number of expire days */
        {
            sprintf (buf,"\n\r&CHow many days do you want this note to expire in?&W\n\r"
                     "&CPress Enter for default value for this board, &W%d&C days.\n\r"
                     "&z&CExpire&B:&W  ",
                     ch->pcdata->board->purge_days);
            send_to_char_color ( buf, ch);
            d->connected = CON_NOTE_EXPIRE;
        }
        else
        {
            ch->pcdata->in_progress->expire =
                current_time + ch->pcdata->board->purge_days * 24L * 3600L;
            sprintf (buf, "&CThis note will expire &W%s&C\r",ctime(&ch->pcdata->in_progress->expire));
            send_to_char_color (buf,ch);
            send_to_char_color ( "\n\r&CEnter text. Type &W~&C or &WEND&C on an empty line to end note.\n\r"
			     "&Y*&CKeep the line of text as long as the border below.&W\n\r"
                             "&B--------------------------------------------------------------------------------&W\n\r",ch);
            d->connected = CON_NOTE_TEXT;
        }
    }
}

void handle_con_note_expire(DESCRIPTOR_DATA *d, char * argument)
{
    CHAR_DATA *ch = d->character;
    char buf[MAX_STRING_LENGTH];
    time_t expire;
    int days;

    if (!ch->pcdata->in_progress)
    {
        d->connected = CON_PLAYING;
        bug ("nanny: In CON_NOTE_EXPIRE, but no note in progress",0);
        return;
    }

    /* Numeric argument. no tilde smashing */
    strcpy (buf, argument);
    if (!buf[0]) /* assume default expire */
        days = 	ch->pcdata->board->purge_days;
    else if (!is_number(buf)) /* use this expire */
    {
        send_to_char_color ("&CWrite the number of days!\n\r",ch);
        send_to_char_color ("&CExpire&B:&W  ",ch);
        return;
    }
    else
    {
        days = atoi (buf);
        if (days <= 0)
        {
            send_to_char_color ( "&CThis is a positive MUD. Use positive numbers only! :)\n\r",ch);
            send_to_char_color ( "&CExpire&B:&W  ",ch);
            return;
        }
    }

    expire = current_time + (days*24L*3600L); /* 24 hours, 3600 seconds */

    ch->pcdata->in_progress->expire = expire;

    /* note that ctime returns XXX\n so we only need to add an \r */

    send_to_char_color ( "\n\r&CEnter text. Type &W~&C or &WEND&C on an empty line to end note.\n\r"
		     "&Y*&CKeep the line of text as long as the border below.&W\n\r"
                     "&B--------------------------------------------------------------------------------&W\n\r",ch);

    d->connected = CON_NOTE_TEXT;
}

void handle_con_note_text (DESCRIPTOR_DATA *d, char * argument)
{
    CHAR_DATA *ch = d->character;
    char buf[MAX_STRING_LENGTH];
    char letter[4*MAX_STRING_LENGTH];

    if (!ch->pcdata->in_progress)
    {
        d->connected = CON_PLAYING;
        bug ("nanny: In CON_NOTE_TEXT, but no note in progress",0);
        return;
    }

    /* First, check for EndOfNote marker */

    strcpy (buf, argument);
    if ((!str_cmp(buf, "~")) || (!str_cmp(buf, "END")))
    {
        send_to_char_color ( "\n\r\n\r",ch);
        send_to_char_color ( szFinishPrompt, ch);
        send_to_char_color ( "\n\r", ch);
        d->connected = CON_NOTE_FINISH;
        return;
    }

    smash_tilde (buf); /* smash it now */

    /* Check for too long lines. Do not allow lines longer than 80 chars */

    if (strlen (buf) > MAX_LINE_LENGTH)
    {
        send_to_char_color ( "&RToo long line rejected. Do NOT go over 50 characters!&W\n\r",ch);
        return;
    }

    /* Not end of note. Copy current text into temp buffer, add new line, and copy back */

    /* How would the system react to strcpy( , NULL) ? */
    if (ch->pcdata->in_progress->text)
    {
        strcpy (letter, ch->pcdata->in_progress->text);
        STRFREE( ch->pcdata->in_progress->text );
/*        ch->pcdata->in_progress->text = NULL; */ /* be sure we don't free it twice */
    }
    else
        strcpy (letter, "");

    /* Check for overflow */

    if ((strlen(letter) + strlen (buf)) > MAX_NOTE_TEXT)
    { /* Note too long, take appropriate steps */
        send_to_char_color ( "&RNote too long!&W\n\r", ch);
        free_global_note (ch->pcdata->in_progress);
        ch->pcdata->in_progress = NULL;			/* important */
        d->connected = CON_PLAYING;
        return;
    }

    /* Add new line to the buffer */

    strcat (letter, buf);
    strcat (letter, "\r\n"); /* new line. \r first to make note files better readable */

    /* allocate dynamically */
    ch->pcdata->in_progress->text = STRALLOC (letter);
}

void handle_con_note_finish (DESCRIPTOR_DATA *d, char * argument)
{
    CHAR_DATA *ch = d->character;

    if (!ch->pcdata->in_progress)
    {
        d->connected = CON_PLAYING;
        bug ("nanny: In CON_NOTE_FINISH, but no note in progress",0);
        return;
    }

    switch (tolower(argument[0]))
    {
    case 'c': /* keep writing */
        send_to_char_color ("&CContinuing note...&W\n\r",ch);
        d->connected = CON_NOTE_TEXT;
        break;

    case 'v': /* view note so far */
        if (ch->pcdata->in_progress->text)
        {
            send_to_char_color ("&CText of your note so far&B:&W\n\r",ch);
            send_to_char_color ( ch->pcdata->in_progress->text, ch);
        }
        else
            send_to_char_color ("&RYou haven't written a thing!&W\n\r\n\r",ch);
        send_to_char_color ( szFinishPrompt, ch);
        send_to_char_color ( "\n\r",ch);
        break;

    case 'p': /* post note */
        finish_note (ch->pcdata->board, ch->pcdata->in_progress);
        send_to_char_color ( "&GNote posted.&W\n\r",ch);
        d->connected = CON_PLAYING;
        /* remove AFK status */
        ch->pcdata->in_progress = NULL;
        act( AT_GREEN, "$n finished $s note.", ch, NULL, NULL, TO_ROOM);
        break;

    case 'f':
        send_to_char_color ( "&RNote cancelled!&W\n\r",ch);
        free_global_note( ch->pcdata->in_progress );
        ch->pcdata->in_progress = NULL;
        d->connected = CON_PLAYING;
        /* remove afk status */
        break;

    default: /* invalid response */
        send_to_char_color ( "&CHuh? Valid answers are&B:&W\n\r\n\r",ch);
        send_to_char_color ( szFinishPrompt, ch);
        send_to_char_color ( "\n\r",ch);

    }
}

