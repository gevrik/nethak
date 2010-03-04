/*****************************************************************************
 * DikuMUD (C) 1990, 1991 by:                                                *
 *   Sebastian Hammer, Michael Seifert, Hans Henrik Staefeldt, Tom Madsen,   *
 *   and Katja Nyboe.                                                        *
 *---------------------------------------------------------------------------*
 * MERC 2.1 (C) 1992, 1993 by:                                               *
 *   Michael Chastain, Michael Quan, and Mitchell Tse.                       *
 *---------------------------------------------------------------------------*
 * SMAUG 1.4 (C) 1994, 1995, 1996, 1998 by: Derek Snider.                    *
 *   Team: Thoric, Altrag, Blodkai, Narn, Haus, Scryn, Rennard, Swordbearer, *
 *         gorog, Grishnakh, Nivek, Tricops, and Fireblade.                  *
 *---------------------------------------------------------------------------*
 * SMAUG 1.7 FUSS by: Samson and others of the SMAUG community.              *
 *                    Their contributions are greatly appreciated.           *
 *---------------------------------------------------------------------------*
 * LoP (C) 2006, 2007, 2008, 2009 by: the LoP team.                          *
 *****************************************************************************/

#include <stdio.h>
#include <string.h>
#include "mud.h"

/*
 * This was done and tested on SmaugFUSS1.9
 * NOTICE: I haven't done the const char *argument fix (no compiler to try it on)
 *
 * In mud.h for struct pc_data toss in these.
 * ------------------------------------------
 * char dis_sudoku[9][9];
 * char sudoku[9][9];
 * time_t sstarttime, sfastesttime, sslowesttime, slasttime;
 * unsigned swins, squits;
 * ------------------------------------------
 * In your makefile add for it to compile in sudoku.c.
 * 
 * Add in for the command do_sudoku how ever you would any other command.
 *
 * If you wish to have it save them, make use of fwrite_sudoku and fread_sudoku.
 *
 * I'm assuming most now how to do those so I left them vague.
 *    If you don't feel free to ask and if alot of people seem to have
 *    trouble doing those I'll go into more details.
 *
 * Hope at least one person finds it useful in some way - Remcon.
 *
 * You are free to use it how you want, but be sure as always to give credit
 *    where it's due. If you think you have found a better way of handling
 *    something feel free to share it with myself and others :)
 *
 * This is the help file I've went with so far.
 *
 * Syntax: sudoku
 * Syntax: sudoku <new/puzzpossible/possibilities>
 * Syntax: sudoku set <row[1-9]><col[1-9]><value[1-9]>
 *
 * Sudoku with no argument will display your current sudoku puzzle.
 * Using the argument 'new' will start a new puzzle.
 * Using the argument 'puzzpossible' will display your current
 *    sudoku puzzle and the possibilities in empty blocks.
 * Using the argument 'possibilities' will display possibilities
 *    for your current sudoku puzzle.
 * Using the argument set with a valid row, column, and value will
 *    allow you to set that block to the desired value.
 */

void show_sudoku( CHAR_DATA *ch, bool possible );

char *stime_display( time_t time )
{
   static char *stime;
   char buf[MAX_STRING_LENGTH];
   time_t ttime;
   int days = 0, hours = 0, mins = 0, seconds = 0;

   ttime = time;
   days = ( int )( ttime / 43200 );
   ttime -= ( days * 43200 );
   hours = ( int )( ttime / 3600 );
   ttime -= ( hours * 3600 );
   mins = ( int )( ttime / 60 );
   ttime -= ( mins * 60 );
   seconds = ( int )( ttime );
   buf[0] = '\0';
   if( days > 0 )
      snprintf( buf + strlen( buf ), sizeof( buf ) - strlen( buf ), "%d day%s", days, days > 1 ? "s" : "" );
   if( hours > 0 )
      snprintf( buf + strlen( buf ), sizeof( buf ) - strlen( buf ), "%s%d hour%s",
         buf[0] != '\0' ? " " : "", hours, hours > 1 ? "s" : "" );
   if( mins > 0 )
      snprintf( buf + strlen( buf ), sizeof( buf ) - strlen( buf ), "%s%d minute%s",
         buf[0] != '\0' ? " " : "", mins, mins > 1 ? "s" : "" );
   if( ( days <= 0 && hours <= 0 && mins <= 0 ) || seconds > 0 )
      snprintf( buf + strlen( buf ), sizeof( buf ) - strlen( buf ), "%s%d second%s",
         buf[0] != '\0' ? " " : "", seconds, seconds != 1 ? "s" : "" );
   stime = buf;
   return stime;
}

short get_region( short row, short col )
{
   if( row >= 0 && row <= 2 && col >= 0 && col <= 2 )
      return 0;
   if( row >= 0 && row <= 2 && col >= 3 && col <= 5 )
      return 1;
   if( row >= 0 && row <= 2 && col >= 6 && col <= 8 )
      return 2;
   if( row >= 3 && row <= 5 && col >= 0 && col <= 2 )
      return 3;
   if( row >= 3 && row <= 5 && col >= 3 && col <= 5 )
      return 4;
   if( row >= 3 && row <= 5 && col >= 6 && col <= 8 )
      return 5;
   if( row >= 6 && row <= 8 && col >= 0 && col <= 2 )
      return 6;
   if( row >= 6 && row <= 8 && col >= 3 && col <= 5 )
      return 7;
   if( row >= 6 && row <= 8 && col >= 6 && col <= 8 )
      return 8;
   return -1;
}

bool can_use_sudoku( CHAR_DATA *ch, short value, short onrow, short oncol )
{
   short row, col, region, onregion;

   if( !ch || !ch->pcdata )
      return FALSE;
   onregion = get_region( onrow, oncol );
   for( row = 0; row < 9; row++ )
   {
      for( col = 0; col < 9; col++ )
      {
         /* Only really need to check the same row, col or main block */
         if( onrow == row || oncol == col || ( region = get_region( row, col ) ) == onregion )
         {
            /* If its already used in one of them can't use it now */
            if( ch->pcdata->sudoku[row][col] == value )
               return FALSE;
         }
      }
   }
   return TRUE;
}

bool could_use_sudoku( CHAR_DATA *ch, short value, short onrow, short oncol )
{
   short row, col, region, onregion;

   if( !ch || !ch->pcdata )
      return FALSE;
   onregion = get_region( onrow, oncol );
   for( row = 0; row < 9; row++ )
   {
      for( col = 0; col < 9; col++ )
      {
         region = get_region( row, col );
         /* Only really need to check the same row, col or main block */
         if( onrow == row || oncol == col || region == onregion )
         {
            /* They should be allowed to reset it to the same number in that spot if they wish */
            if( onrow == row && oncol == col )
               continue;
            /* If its already used in one of them can't use it now */
            if( ch->pcdata->dis_sudoku[row][col] == value )
               return FALSE;
         }
      }
   }
   return TRUE;
}

void set_sudoku( CHAR_DATA *ch, char *argument )
{
   short row = -1, col = -1, value = -1;

   if( !ch || !ch->pcdata )
      return;

   if( argument[0] != '\0' )
   {
      row = ( short ) ( argument[0] - '0' );
      row--; /* Have to subtract a row */
      if( argument[1] != '\0' )
      {
         col = ( short ) ( argument[1] - '0' );
         col--; /* Have to subtract a col */
         if( argument[2] != '\0' )
            value = ( short ) ( argument[2] - '0' );
      }
   }

   if( row < 0 || row > 8 )
   {
      send_to_char( "Invalid row, please specify a row 1-9.\r\n", ch );
      return;
   }
   if( col < 0 || col > 8 )
   {
      send_to_char( "Invalid column, please specify a row 1-9.\r\n", ch );
      return;
   }
   if( value < 0 || value > 9 )
   {
      send_to_char( "Invalid number, please specify a number 0-9.\r\n", ch );
      return;
   }
   if( !could_use_sudoku( ch, value, row, col ) && value != 0 )
   {
      send_to_char( "That value isn't even a possibility in that row and column.\r\n", ch );
      return;
   }
   if( ch->pcdata->dis_sudoku[row][col] == ch->pcdata->sudoku[row][col] && ch->pcdata->sudoku[row][col] != 0 )
   {
      send_to_char( "That value is a base value and can't be changed!\r\n", ch );
      return;
   }
   ch->pcdata->dis_sudoku[row][col] = value;
   ch_printf( ch, "You have set row %d, column %d to %d.\r\n", ( row + 1 ), ( col + 1 ), value );
   show_sudoku( ch, FALSE );
}

bool playing_sudoku( CHAR_DATA *ch )
{
   short row, col;

   if( !ch || !ch->pcdata )
      return FALSE;
   for( row = 0; row < 9; row++ )
   {
      for( col = 0; col < 9; col++ )
      {
         if( ch->pcdata->sudoku[row][col] != 0 )
            return TRUE;
      }
   }
   return FALSE;
}

void quit_sudoku( CHAR_DATA *ch )
{
   short row, col;

   if( !ch || !ch->pcdata )
      return;
   /* Set the puzzle to nothing */
   for( row = 0; row < 9; row++ )
   {
      for( col = 0; col < 9; col++ )
      {
         ch->pcdata->dis_sudoku[row][col] = 0;
         ch->pcdata->sudoku[row][col] = 0;
      }
   }
}

void completed_sudoku( CHAR_DATA *ch )
{
   short row, col;

   if( !ch || !ch->pcdata )
      return;
   for( row = 0; row < 9; row++ )
   {
      for( col = 0; col < 9; col++ )
      {
         if( ch->pcdata->dis_sudoku[row][col] == 0 )
            return;
         if( !could_use_sudoku( ch, ch->pcdata->dis_sudoku[row][col], row, col ) )
            return;
      }
   }
   send_to_char( "You have completed that sudoku puzzle.\r\n", ch );
   quit_sudoku( ch );
   ch->pcdata->swins++;
   ch->pcdata->slasttime = ( current_time - ch->pcdata->sstarttime );
   if( ch->pcdata->slasttime > ch->pcdata->sslowesttime || ch->pcdata->sslowesttime == 0 )
      ch->pcdata->sslowesttime = ch->pcdata->slasttime;
   if( ch->pcdata->slasttime < ch->pcdata->sfastesttime || ch->pcdata->sfastesttime == 0 )
      ch->pcdata->sfastesttime = ch->pcdata->slasttime;
}

void show_possibilities( CHAR_DATA *ch )
{
   char buf[MAX_STRING_LENGTH];
   short value = 0, row = 0, col = 0, ccol = 0;

   if( !ch || !ch->pcdata )
      return;
   /* It takes 3 different checks for this stuff for each thing we go to set */
   for( row = 0; row < 9; row++ )
   {
      for( col = 0; col < 9; col++ )
      {
         if( ch->pcdata->dis_sudoku[row][col] != 0 )
            continue;
         ch_printf( ch, " [%d] [%d]", ( row + 1 ), ( col + 1 ) );
         buf[0] = '\0';
         for( value = 1; value <= 9; value++ )
         {
            if( could_use_sudoku( ch, value, row, col ) )
               snprintf( buf + strlen( buf ), sizeof( buf ) - strlen( buf ), " %d ", value );
         }
         ch_printf( ch, "%-20s", buf );
         if( ++ccol == 3 )
         {
            send_to_char( "\r\n", ch );
            ccol = 0;
         }
      }
   }
   if( ccol != 0 )
      send_to_char( "\r\n", ch );
}

void show_sudoku( CHAR_DATA *ch, bool possible )
{
   short row, col, value, rowdis;

   if( !ch || !ch->pcdata )
      return;
   set_char_color( AT_GREEN, ch );
   if( possible )
   {
      send_to_char( "                            Columns                        \r\n", ch );
      send_to_char( "Rows   1     2     3     4     5     6     7     8     9   \r\n", ch );
   }
   else
   {
      send_to_char( "                   Columns               \r\n", ch );
      send_to_char( "Rows  1   2   3   4   5   6   7   8   9  \r\n", ch );
   }
   for( row = 0; row < 9; row++ )
   {
      if( row == 0 || row == 3 || row == 6 )
      {
         if( possible )
            send_to_char( "    -------------------------------------------------------\r\n", ch );
         else
            send_to_char( "    -------------------------------------\r\n", ch );
      }
      else
      {
         if( possible )
            send_to_char( "    |----- ----- -----|----- ----- -----|----- ----- -----|\r\n", ch );
         else
            send_to_char( "    |--- --- ---|--- --- ---|--- --- ---|\r\n", ch );
      }
      for( rowdis = ( possible ? 0 : 1 ); rowdis < ( possible ? 3 : 2 ); rowdis++ )
      {
         if( rowdis == 1 )
            ch_printf( ch, "  %d | ", ( row + 1 ) );
         else
            send_to_char( "    | ", ch );
         for( col = 0; col < 9; col++ )
         {
            if( ch->pcdata->dis_sudoku[row][col] > 0 )
            {
               if( rowdis == 1 )
               {
                  if( possible )
                  {
                     if( ch->pcdata->dis_sudoku[row][col] == ch->pcdata->sudoku[row][col] )
                        ch_printf( ch, "|&G%d&D|", ch->pcdata->dis_sudoku[row][col] );
                     else
                        ch_printf( ch, "*&R%d&D*", ch->pcdata->dis_sudoku[row][col] );
                  }
                  else
                     ch_printf( ch, "&G%d&D", ch->pcdata->dis_sudoku[row][col] );
               }
               else
               {
                  if( ch->pcdata->dis_sudoku[row][col] == ch->pcdata->sudoku[row][col] )
                     send_to_char( "---", ch );
                  else
                     send_to_char( "***", ch );
               }
             
            }
            else if( possible )
            {
               for( value = 1; value <= 9; value++ )
               {
                   if( rowdis == 0 && ( value < 1 || value > 3 ) )
                     continue;
                  if( rowdis == 1 && ( value < 4 || value > 6 ) )
                     continue;
                  if( rowdis == 2 && ( value < 7 || value > 9 ) )
                     continue;
                  if( could_use_sudoku( ch, value, row, col ) )
                     ch_printf( ch, "&W%d&D", value );
                  else
                     send_to_char( " ", ch );
               }
            }
            else
               send_to_char( " ", ch );
            send_to_char( " | ", ch );
         }
         send_to_char( "\r\n", ch );
      }
   }
   if( possible )
      send_to_char( "    -------------------------------------------------------\r\n", ch );
   else
      send_to_char( "    -------------------------------------\r\n", ch );
   ch_printf( ch, "So far you have been working on this puzzle for %s.\r\n",
      stime_display( current_time - ch->pcdata->sstarttime ) );
}

/* Set it back to default */
void reset_sudoku( CHAR_DATA *ch )
{
   short row, col;

   if( !ch || !ch->pcdata )
      return;
   for( row = 0; row < 9; row++ )
   {
      for( col = 0; col < 9; col++ )
      {
         ch->pcdata->dis_sudoku[row][col] = ch->pcdata->sudoku[row][col];
      }
   }
}

void new_sudoku( CHAR_DATA *ch )
{
   short tries, value = 0, row = 0, col = 0, tried = 0, original = 0;
   bool valid = FALSE, restart = FALSE;

   if( !ch || !ch->pcdata )
      return;
   /* It takes 3 different checks for this stuff for each thing we go to set */
   for( tries = 0; tries < 1000; tries++ )
   {
      quit_sudoku( ch );
      restart = FALSE;

      for( row = 0; row < 9; row++ )
      {
         for( col = 0; col < 9; col++ )
         {
            tried = 0;
            /* Have to get a value they can use */
            for( original = value = number_range( 1, 9 ); value; value++ )
            {
               if( can_use_sudoku( ch, value, row, col ) )
                  break;
               if( value >= 9 )
                  value = 0;
               /* If it can't find it by this point might as well say so and return */
               if( ++tried == 11 )
               {
                  restart = TRUE;
                  break;
               }
            }
            if( restart )
               break;
            ch->pcdata->sudoku[row][col] = value;
            /* Later we will replace 0 with a difficulty setting of 0 - 10 */
            /* Always at least a 15 chance of showing or not showing */
            if( number_range( -15, 25 ) > 10 )
               ch->pcdata->dis_sudoku[row][col] = value;
            if( row == 8 && col == 8 )
               valid = TRUE;
         }
         if( restart )
            break;
      }
      if( valid )
         break;
   }
   /* Show the data */
   if( valid )
   {
      /* Set the default shown */
      for( row = 0; row < 9; row++ )
      {
         for( col = 0; col < 9; col++ )
         {
            ch->pcdata->sudoku[row][col] = ch->pcdata->dis_sudoku[row][col];
         }
      }
      ch->pcdata->sstarttime = current_time;
      show_sudoku( ch, FALSE );
   }
   else
   {
      send_to_char( "Sorry but a sudoku puzzle couldn't be created...try again.\r\n", ch );
      quit_sudoku( ch );
   }
}

void sudoku_stats( CHAR_DATA *ch, char *argument )
{
   CHAR_DATA *victim = NULL;

   if( !IS_IMMORTAL( ch ) || !argument || argument[0] == '\0' )
      victim = ch;
   else if( !( victim = get_char_world( ch, argument ) ) )
   {
      send_to_char( "Whose sudoku stats would you like to see?\r\n", ch );
      return;
   }
   if( !victim->pcdata )
   {
      send_to_char( "They don't have any sudoku data to display.\r\n", ch );
      return;
   }
   if( victim != ch )
      ch_printf( ch, "Sudoku stats for %s:\r\n", victim->name );
   else
      send_to_char( "Your Sudoku stats:\r\n", ch );
   if( playing_sudoku( victim ) )
   {
      send_to_char( "( Currently playing a Sudoku Puzzle )\r\n", ch );
      ch_printf( ch, "( Been playing it for %ld seconds )\r\n", ( current_time - victim->pcdata->sstarttime ) );
   }
   ch_printf( ch, "Won: %u, Lost: %u, Total: %u\r\n",
      victim->pcdata->swins, victim->pcdata->squits, ( victim->pcdata->swins + victim->pcdata->squits ) );
   ch_printf( ch, "Fastest Time: %s\r\n", stime_display( victim->pcdata->sfastesttime ) );
   ch_printf( ch, "Slowest Time: %s\r\n", stime_display( victim->pcdata->sslowesttime ) );
   ch_printf( ch, "Last Time:    %s\r\n", stime_display( victim->pcdata->slasttime ) );
}

void do_sudoku( CHAR_DATA *ch, char *argument )
{
   char arg[MAX_INPUT_LENGTH];

   if( !ch || !ch->pcdata )
      return;

   argument = one_argument( argument, arg );

   if( !str_cmp( arg, "stats" ) )
   {
      sudoku_stats( ch, argument );
      return;
   }

   if( !str_cmp( arg, "new" ) )
   {
      if( playing_sudoku( ch ) )
      {
         send_to_char( "You already have a sudoku puzzle going. Type 'sudoku quit' to quit that one.\r\n", ch );
         return;
      }
      new_sudoku( ch );
      return;
   }

   if( !playing_sudoku( ch ) )
   {
      send_to_char( "You aren't working on a sudoku puzzle. Type 'sudoku new' to start one.\r\n", ch );
      return;
   }

   if( !str_cmp( arg, "quit" ) )
   {
      send_to_char( "You have quit the sudoku puzzle you were doing. Type 'sudoku new' to start one.\r\n", ch );
      quit_sudoku( ch );
      ch->pcdata->squits++;
      return;
   }

   if( !str_cmp( arg, "reset" ) )
   {
      send_to_char( "Your sudoku puzzle has been reset to the default.\r\n", ch );
      reset_sudoku( ch );
      return;
   }

   /* Allow them to set it */
   if( !str_cmp( arg, "set" ) )
   {
      set_sudoku( ch, argument );
      completed_sudoku( ch );
      return;
   }

   if( IS_IMMORTAL( ch ) )
   {
      /* Allow them to see the possibilities for the blank places */
      if( !str_cmp( arg, "possibilities" ) )
      {
         show_possibilities( ch );
         return;
      }

      /* Allow them to see the possibilities in the puzzle */
      if( !str_cmp( arg, "puzzpossible" ) )
      {
         show_sudoku( ch, TRUE );
         return;
      }
   }

   show_sudoku( ch, FALSE );
}

void fwrite_sudoku( CHAR_DATA *ch, FILE *fp )
{
   int row = 0, col = 0;

   if( !ch || !ch->pcdata )
      return;
   if( playing_sudoku( ch ) )
   {
      fprintf( fp, "%s        ", "Sudoku" );
      for( row = 0; row < 9; row++ )
         for( col = 0; col < 9; col++ )
            fprintf( fp, " %d", ch->pcdata->sudoku[row][col] );
      fprintf( fp, "%s", "\n" );

      fprintf( fp, "%s     ", "DisSudoku" );
      for( row = 0; row < 9; row++ )
         for( col = 0; col < 9; col++ )
            fprintf( fp, " %d", ch->pcdata->dis_sudoku[row][col] );
      fprintf( fp, "%s", "\n" );
   }
   fprintf( fp, "SudokuWins     %u\n", ch->pcdata->swins );
   fprintf( fp, "SudokuQuits    %u\n", ch->pcdata->squits );
   fprintf( fp, "SudokuStart    %ld\n", ch->pcdata->sstarttime );
   fprintf( fp, "SudokuFast     %ld\n", ch->pcdata->sfastesttime );
   fprintf( fp, "SudokuSlow     %ld\n", ch->pcdata->sslowesttime );
   fprintf( fp, "SudokuLast     %ld\n", ch->pcdata->slasttime );
}

/* Have to do this while loading the sudoku up */
void fread_sudoku( CHAR_DATA *ch, bool display, FILE *fp )
{
   char c;
   int row = 0, col = 0, value;

   if( !ch || !ch->pcdata )
   {
      fread_to_eol( fp );
      return;
   }
   /* Start with it empty */
   for( row = 0; row < 9; row++ )
   {
      for( col = 0; col < 9; col++ )
      {
         if( display )
            ch->pcdata->dis_sudoku[row][col] = 0;
         else
            ch->pcdata->sudoku[row][col] = 0;
      }
   }
   row = col = 0;
   for( ;; )
   {
      c = getc( fp );
      if( c == EOF )
      {
         bug( "%s: EOF", __FUNCTION__ );
         return;
      }
      if( c == ' ' )
         continue;
      if( c == '~' )
         return;
      if( row >= 9 || col >= 9 )
      {
         bug( "%s: row[%d], col[%d] is over the limits", __FUNCTION__, row, col );
         fread_to_eol( fp );
         return;
      }
      value = ( c - '0' );
      if( value < 0 || value > 9 )
         bug( "%s: value = %d and should be 0 - 9", __FUNCTION__, value );
      if( display )
         ch->pcdata->dis_sudoku[row][col] = value;
      else
         ch->pcdata->sudoku[row][col] = value;
      if( ++col >= 9 )
      {
         row++;
         col = 0;
      }
      if( row >= 9 )
         break;
   }
}
