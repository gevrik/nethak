/* Dump the muds helpfiles into a manageable web directory for online viewing.
 * The concept is simple, all the helpfiles are linked, and therefore you can redump
 * the current helpfiles from the mud into a database. Its a fairly simple idea. Some
 * concepts have been taken from the webwho.c snippet available for download.
 * Due to very little changing, instead of being an automatic procedure, the command
 * must be used to update the webfiles. Please note that this module was designed to
 * ignore background characters ^, as well as use the & as the foreground color.
 * Update to your hearts content.
 * -Lajos
 */

#if defined(macintosh)
#include <types.h>
#else
#include <sys/types.h>
#endif
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <malloc.h>
#include "mud.h"

#if !defined(macintosh)
extern  int     _filbuf         args( (FILE *) );
#endif

extern OBJ_INDEX_DATA *obj_index_hash[MAX_KEY_HASH];
extern MOB_INDEX_DATA *mob_index_hash[MAX_KEY_HASH];

extern int html_colour args (( char type, char *string ));
extern void html_colourconv args (( char *buffer, const char *txt, CHAR_DATA *ch ));
/* These functions are modified versions from the webwho snippet available for download.
 * If you do not have this function it has been included in the webcolor.c file for
 * convenience.
 * -Lajos
 */
void add_linebreaks     args(( char *buffer, char const *txt) ); 
/* Used to add linebreaks to helpfiles to maintain structure in html transfer. */

void do_makehelpoff( CHAR_DATA *ch, char *argument )
{
   FILE *fp;
   HELP_DATA *pHelp;
   char buf[MAX_INPUT_LENGTH];
   char buf2[MAX_STRING_LENGTH];
   char buf3[MAX_STRING_LENGTH*2];

   sh_int number = 0; /* Which helpfile we're on, allows me to link up the helpfiles by a number. */
   fclose(fpReserve);

   if ( (fp = fopen("../../public_html/nethak/helps/index.html", "w") ) == NULL)
   {
      bug("helps: fopen", 0);
      perror( "index.html");
   }
   else
   {
      fprintf(fp, "<HTML>\n");
      fprintf(fp, "<HEAD>\n");
      fprintf(fp, "<TITLE>");
      fprintf(fp, "Helpfile Database");
      fprintf(fp, "</TITLE>\n");


      fprintf(fp, "<BODY TEXT=""#C0C0C0"" BGCOLOR=""#000000"" BACKGROUND=""./images/tile.jpg"" LINK=""#00FFFF""");
      fprintf(fp, "VLINK=""#FFFFFF"" ALINK=""#008080"">\n\n");

      fprintf(fp, "<font face=""Times New Roman"">\n");
      fprintf(fp, "<TABLE BORDER=0 BGCOLOR=""#000000"">\n");
      fprintf(fp, "<TR>\n");
      fprintf(fp, "<TD><P ALIGN=""CENTER"">Neuromancer Helpfiles</P></TD></TR>");
      fprintf(fp, "</TABLE>\n");
      fprintf(fp, "<HR>\n");
      fprintf(fp, "<TABLE BORDER=0 BGCOLOR=""#000000"">\n");
      fprintf(fp, "<TR><TD WITDTH=""50"">Level</TD><TD>Keyword</TD></TR>\n");

      /* Create a sample table page to link up the helpfiles to. */
      for ( pHelp = first_help; pHelp; pHelp = pHelp->next )
      {
         if( pHelp->level > 199 || pHelp->level == -1 ) /* To keep immortal only helpfiles from the database */
            continue;
         number++;
         fprintf(fp, "\n<TR><TD WIDTH=""50""> %d </TD>\n", pHelp->level);
         fprintf(fp, "<TD><A HREF=""%d.html"">%s</A></TD>\n", number, pHelp->keyword);
         fprintf(fp, "</TD></TR>\n");
      }
      fprintf(fp, "</TABLE>\n");
      fprintf(fp, "</P><P ALIGN=""CENTER"">\n");
      fprintf(fp, "This database was last updated: %s\n", (char *) ctime( &current_time) );
      fprintf(fp, "</P></BODY></HTML>\n");
      fclose(fp);
      number = 0;
      /* Now create the actual helpfiles. */
      for( pHelp = first_help; pHelp; pHelp = pHelp->next )
      {
         if( pHelp->level > 199 || pHelp->level == -1 ) /* To keep immortal only helpfiles from the database */
            continue;
         number++;
         sprintf(buf, "../../public_html/nethak/helps/%d.html", number);
         if ( (fp = fopen(buf, "w") ) == NULL)
         {
            bug("helps: fopen", 0);
         }
         else
         {
            fprintf(fp, "<HTML>\n");
            fprintf(fp, "<HEAD>\n");
            fprintf(fp, "<TITLE>");
            fprintf(fp, "%s Helpfile", pHelp->keyword );
            fprintf(fp, "</TITLE>\n");
            
            fprintf(fp, "<BODY TEXT=""#C0C0C0"" BGCOLOR=""#000000"" BACKGROUND=""./images/tile.jpg"" LINK=""#00FFFF""");
            fprintf(fp, "VLINK=""#FFFFFF"" ALINK=""#008080"">\n\n");
            
            fprintf(fp, "<font face=""Times New Roman"">\n");
            fprintf(fp, "<P>%s</P>\n", pHelp->keyword );
            
            html_colourconv(buf2, pHelp->text, ch);
            add_linebreaks( buf3, buf2 );
            fprintf(fp, "<P>%s</P>", buf3);
            fprintf(fp, "<P><A HREF=""index.html"">Back to Database</A></P>\n");
            fclose(fp);
         }
      }
      fpReserve = fopen( NULL_FILE, "r" );
   }
}

void do_makehelp( CHAR_DATA *ch, char *argument )
{
   FILE *fp;
   OBJ_INDEX_DATA *pObjIndex;
   //OBJ_DATA *pObjIndex;
   int hash;

   sh_int number = 0; /* Which helpfile we're on, allows me to link up the helpfiles by a number. */
   fclose(fpReserve);

   if ( (fp = fopen("./objectslist/index.html", "w") ) == NULL)
   {
      bug("helps: fopen", 0);
      perror( "index.html");
   }
   else
   {
      fprintf(fp, "<HTML>\n");
      fprintf(fp, "<HEAD>\n");
      fprintf(fp, "<TITLE>");
      fprintf(fp, "Object Database");
      fprintf(fp, "</TITLE>\n");


      fprintf(fp, "<BODY TEXT=""#C0C0C0"" BGCOLOR=""#000000"" BACKGROUND=""./images/tile.jpg"" LINK=""#00FFFF""");
      fprintf(fp, "VLINK=""#FFFFFF"" ALINK=""#008080"">\n\n");

      fprintf(fp, "<font face=""Times New Roman"">\n");
      fprintf(fp, "<TABLE BORDER=0 BGCOLOR=""#000000"">\n");
      fprintf(fp, "<TR>\n");
      fprintf(fp, "<TD><P ALIGN=""CENTER"">Neuromancer Helpfiles</P></TD></TR>");
      fprintf(fp, "</TABLE>\n");
      fprintf(fp, "<HR>\n");
      fprintf(fp, "<TABLE BORDER=0 BGCOLOR=""#000000"">\n");
      fprintf(fp, "<TR><TD WITDTH=""50"">VNUM</TD><TD>Name</TD><TD>Type</TD><TD>Cost</TD><TD>0</TD><TD>1</TD><TD>2</TD><TD>3</TD><TD>4</TD><TD>5</TD><TD>Wear</TD></TR>\n");

      /* Create a sample table page to link up the helpfiles to. */
      //for ( pObjIndex = first_object; pObjIndex; pObjIndex = pObjIndex->next )
    for ( hash = 0; hash < MAX_KEY_HASH; hash++ )
	for ( pObjIndex = obj_index_hash[hash];
	      pObjIndex;
	      pObjIndex = pObjIndex->next )
      {
        // if( pHelp->level > 199 || pHelp->level == -1 ) /* To keep immortal only helpfiles from the database */
         //continue;
         number++;
         fprintf(fp, "\n<TR><TD WIDTH=""50""> %ld </TD>\n", pObjIndex->vnum);
         fprintf(fp, "<TD>%s</A></TD>\n", pObjIndex->name);
         //fprintf(fp, "<TD>%s</A></TD>\n", aoran( item_type_name( pObjIndex ) ));
         fprintf(fp, "<TD>%d</A></TD>\n", pObjIndex->item_type);
	 fprintf(fp, "<TD>%d</A></TD>\n", pObjIndex->cost);
	 fprintf(fp, "<TD>%d</A></TD>\n", pObjIndex->value[0]);
	 fprintf(fp, "<TD>%d</A></TD>\n", pObjIndex->value[1]);
	 fprintf(fp, "<TD>%d</A></TD>\n", pObjIndex->value[2]);
	 fprintf(fp, "<TD>%d</A></TD>\n", pObjIndex->value[3]);
	 fprintf(fp, "<TD>%d</A></TD>\n", pObjIndex->value[4]);
	 fprintf(fp, "<TD>%d</A></TD>\n", pObjIndex->value[5]);
         fprintf(fp, "<TD>%s</A></TD>\n", flag_string(pObjIndex->wear_flags, w_flags));
         fprintf(fp, "</TD></TR>\n");
      }
      fprintf(fp, "</TABLE>\n");
      fprintf(fp, "</P><P ALIGN=""CENTER"">\n");
      fprintf(fp, "This database was last updated: %s\n", (char *) ctime( &current_time) );
      fprintf(fp, "</P></BODY></HTML>\n");
      fclose(fp);
      number = 0;
      /* Now create the actual helpfiles.
      for( pHelp = first_help; pHelp; pHelp = pHelp->next )
      {
         if( pHelp->level > 199 || pHelp->level == -1 ) 
            continue;
         number++;
         sprintf(buf, "./helps/%d.html", number);
         if ( (fp = fopen(buf, "w") ) == NULL)
         {
            bug("helps: fopen", 0);
         }
         else
         {
            fprintf(fp, "<HTML>\n");
            fprintf(fp, "<HEAD>\n");
            fprintf(fp, "<TITLE>");
            fprintf(fp, "%s Helpfile", pHelp->keyword );
            fprintf(fp, "</TITLE>\n");
            
            fprintf(fp, "<BODY TEXT=""#C0C0C0"" BGCOLOR=""#000000"" BACKGROUND=""./images/tile.jpg"" LINK=""#00FFFF""");
            fprintf(fp, "VLINK=""#FFFFFF"" ALINK=""#008080"">\n\n");
            
            fprintf(fp, "<font face=""Times New Roman"">\n");
            fprintf(fp, "<P>%s</P>\n", pHelp->keyword );
            
            html_colourconv(buf2, pHelp->text, ch);
            add_linebreaks( buf3, buf2 );
            fprintf(fp, "<P>%s</P>", buf3);
            fprintf(fp, "<P><A HREF=""index.html"">Back to Database</A></P>\n");
            fclose(fp);
         }
      } 
    */
      fpReserve = fopen( NULL_FILE, "r" );
   }
}

void do_makehelpmobs( CHAR_DATA *ch, char *argument )
{
   FILE *fp;
   MOB_INDEX_DATA *mob;
   int hash;

   sh_int number = 0; /* Which helpfile we're on, allows me to link up the helpfiles by a number. */
   fclose(fpReserve);

   if ( (fp = fopen("./mobslist/index.html", "w") ) == NULL)
   {
      bug("helps: fopen", 0);
      perror( "index.html");
   }
   else
   {
      fprintf(fp, "<HTML>\n");
      fprintf(fp, "<HEAD>\n");
      fprintf(fp, "<TITLE>");
      fprintf(fp, "Mob Database");
      fprintf(fp, "</TITLE>\n");


      fprintf(fp, "<BODY TEXT=""#C0C0C0"" BGCOLOR=""#000000"" BACKGROUND=""./images/tile.jpg"" LINK=""#00FFFF""");
      fprintf(fp, "VLINK=""#FFFFFF"" ALINK=""#008080"">\n\n");

      fprintf(fp, "<font face=""Times New Roman"">\n");
      fprintf(fp, "<TABLE BORDER=0 BGCOLOR=""#000000"">\n");
      fprintf(fp, "<TR>\n");
      fprintf(fp, "<TD><P ALIGN=""CENTER"">Neuromancer Helpfiles</P></TD></TR>");
      fprintf(fp, "</TABLE>\n");
      fprintf(fp, "<HR>\n");
      fprintf(fp, "<TABLE BORDER=0 BGCOLOR=""#000000"">\n");
      fprintf(fp, "<TR><TD WITDTH=""50"">VNUM</TD><TD>Name</TD><TD>str(bod)</TD><TD>con(eva)</TD><TD>dex(mas)</TD><TD>int(sen)</TD><TD>gold</TD><TD>hitnodice</TD><TD>hitsizedice</TD><TD>hitplus</TD><TD>damnodice</TD><TD>damsizedice</TD><TD>damplus</TD></TR>\n");

      /* Create a sample table page to link up the helpfiles to. */
      //for ( pObjIndex = first_object; pObjIndex; pObjIndex = pObjIndex->next )
        for ( hash = 0; hash < MAX_KEY_HASH; hash++ )
          for ( mob = mob_index_hash[hash]; mob ; mob = mob->next )
      {
        // if( pHelp->level > 199 || pHelp->level == -1 ) /* To keep immortal only helpfiles from the database */
         //continue;
         number++;
         fprintf(fp, "\n<TR><TD WIDTH=""50""> %ld </TD>\n", mob->vnum);
         fprintf(fp, "<TD><A HREF=""%d.html"">%s</A></TD>\n", number, mob->player_name);
	 fprintf(fp, "<TD>%d</TD>\n", mob->perm_str);
 	 fprintf(fp, "<TD>%d</TD>\n", mob->perm_con);
	 fprintf(fp, "<TD>%d</TD>\n", mob->perm_dex);
	 fprintf(fp, "<TD>%d</TD>\n", mob->perm_int);
	 fprintf(fp, "<TD>%d</TD>\n", mob->gold);
	 fprintf(fp, "<TD>%d</TD>\n", mob->hitnodice);
	 fprintf(fp, "<TD>%d</TD>\n", mob->hitsizedice);
	 fprintf(fp, "<TD>%d</TD>\n", mob->hitplus);
	 fprintf(fp, "<TD>%d</TD>\n", mob->damnodice);
	 fprintf(fp, "<TD>%d</TD>\n", mob->damsizedice);
	 fprintf(fp, "<TD>%d</TD>\n", mob->damplus);
         fprintf(fp, "</TD></TR>\n");
      }
      fprintf(fp, "</TABLE>\n");
      fprintf(fp, "</P><P ALIGN=""CENTER"">\n");
      fprintf(fp, "This database was last updated: %s\n", (char *) ctime( &current_time) );
      fprintf(fp, "</P></BODY></HTML>\n");
      fclose(fp);
      number = 0;
      /* Now create the actual helpfiles.
      for( pHelp = first_help; pHelp; pHelp = pHelp->next )
      {
         if( pHelp->level > 199 || pHelp->level == -1 ) 
            continue;
         number++;
         sprintf(buf, "./helps/%d.html", number);
         if ( (fp = fopen(buf, "w") ) == NULL)
         {
            bug("helps: fopen", 0);
         }
         else
         {
            fprintf(fp, "<HTML>\n");
            fprintf(fp, "<HEAD>\n");
            fprintf(fp, "<TITLE>");
            fprintf(fp, "%s Helpfile", pHelp->keyword );
            fprintf(fp, "</TITLE>\n");
            
            fprintf(fp, "<BODY TEXT=""#C0C0C0"" BGCOLOR=""#000000"" BACKGROUND=""./images/tile.jpg"" LINK=""#00FFFF""");
            fprintf(fp, "VLINK=""#FFFFFF"" ALINK=""#008080"">\n\n");
            
            fprintf(fp, "<font face=""Times New Roman"">\n");
            fprintf(fp, "<P>%s</P>\n", pHelp->keyword );
            
            html_colourconv(buf2, pHelp->text, ch);
            add_linebreaks( buf3, buf2 );
            fprintf(fp, "<P>%s</P>", buf3);
            fprintf(fp, "<P><A HREF=""index.html"">Back to Database</A></P>\n");
            fclose(fp);
         }
      } 
    */
      fpReserve = fopen( NULL_FILE, "r" );
   }
}

void add_linebreaks( char *buffer, char const *txt )
{
   const char *i;
   for( i = txt; *i; *i++)
   {
      if( *i == '\n' )
      {  
        *buffer   = '<';
        *++buffer = 'b';
        *++buffer = 'r';
        *++buffer = '>';
        *++buffer = '\0';
      }
      *buffer = *i;
      *++buffer = '\0';
    }
    *buffer = '\0';
    return;
}
