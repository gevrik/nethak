#include "trivia.h"
#include <string.h>

void do_trivia(CHAR_DATA *ch, char *argument)
 {
   char arg0[MAX_INPUT_LENGTH];
   char arg1[MAX_INPUT_LENGTH];
   char buf[MAX_STRING_LENGTH];
   char buf2[32];
   int i;

   /*   struct winner_struct *win1;
   struct winner_struct *win2;
   struct winner_struct *win3;*/
   struct winner_struct *ws;
   struct winner_struct *ws2;
   struct player_struct *p;
   struct player_struct *p2;

   OBJ_DATA *obj;

   argument = one_argument(argument,arg0);
   argument = one_argument(argument,arg1);

   if (IS_NPC(ch))
     {
       send_to_char("Mobs can't start a trivia game.\r\n",ch);
       return;
     }
   
   if (!IS_IMMORTAL(ch))
     {
       send_to_char("You must be immortal to run a trivia game.\n\r",ch);
     }

   if(arg0[0]=='\0')
   {
      send_to_char("Usage: trivia <start | end> [reward]\r\n",ch);
      return;
   }

   if (!strcmp(arg0,"start")) 
     {
       if (g_trivia != NULL) 
	 {
	   send_to_char("There is currently a trivia game in progress.\n\r",ch);
	   return;
	 }
       CREATE(g_trivia,struct trivia_struct,1);
       g_trivia->current_question = 0;
       g_trivia->asker = ch;
       g_trivia->winners = NULL;
       g_trivia->players = NULL;
       if (arg1[0] != '\0' && is_number(arg1)) 
	 {
	   g_trivia->prize = atoi(arg1); 
	   if (get_obj_index( g_trivia->prize ) == NULL)
	     {
	       send_to_char("That object does not exist.\n\r",ch);
	       free(g_trivia);
	       g_trivia = NULL;
	       return;	       
	     }
	   obj = create_object( get_obj_index( g_trivia->prize ), 100 );	   
	   if (obj == NULL) 
	     {
	       send_to_char("That object does not exist.\n\r",ch);
	       free(g_trivia);
	       g_trivia = NULL;
	       return;
	     }
	   send_to_char("Use tquestion to ask a question\n\rUse twinner to announce the winner\n\rUse trivia end to end the game\n\r",ch);
	   echo_to_all(AT_WHITE, "A new trivia game has begun!\n\rType &Ctjoin&W to play!\n\r", ECHOTAR_ALL); 
	   sprintf(buf,"Todays prize is %s!\n\r",obj->short_descr);
	   extract_obj(obj);
	   echo_to_all(AT_WHITE, buf, ECHOTAR_ALL);
	 }
       else
	 {
	   g_trivia->prize = -1;
	   echo_to_all(AT_WHITE, "A new trivia game has begun!\n\rType &Ctjoin&W to play!\n\r", ECHOTAR_ALL); 	   
	   echo_to_all(AT_WHITE, "Todays prize is a mystery!\n\r", ECHOTAR_ALL);
	 }
     } 
   else
     {
       if (!strcmp(arg0,"end"))
	 {
	   send_to_trivia("The Trivia game has ended!\n\r");
	   /*	   win1 = win2 = win3 = NULL;
	   for(ws = g_trivia->winners; ws; ws=ws->next) 
	     {
	       if (win1 == NULL || ws->correct > win1->correct) 
		 {
		   win3 = win2;
		   win2 = win1;
		   win1 = ws;
		 }
	       else
		 {
		   if (win2 == NULL || ws->correct > win2->correct) 
		     {
		       win3 = win2;
		       win2 = ws;
		     }
		   else
		     {
		       if (win3 == NULL || ws->correct > win3->correct) 
			 {
			   win3 = ws;
			 }
		     }
		 }
	     }
	   sprintf(buf,"The Winners are:\n\r1st: %s with %d correct answers\n\r"
		   "2nd: %s with %d correct answers\n\r3rd: %s with %d correct answers\n\r",
		   win1!=NULL?win1->ch->name:"",win1!=NULL?win1->correct:0,
		   win2!=NULL?win2->ch->name:"",win2!=NULL?win2->correct:0, 
		   win3!=NULL?win3->ch->name:"",win3!=NULL?win3->correct:0);
		   send_to_trivia(buf);*/

	   sprintf(buf,"Trivia Standings\n\r");
	   
	   i = 0;
	   for(ws=g_trivia->winners; ws; ws = ws->next)
	     {
	       i++;
	       sprintf(buf2,"%2d. %-15s ",i,ws->ch->name);
	       strcat(buf, buf2);
	       if(i % 3 == 0)
		 strcat(buf,"\n\r");
	     }
	   strcat(buf,"\n\r");
	   send_to_trivia(buf);
	   ws = g_trivia->winners;
	   while(ws) 
	     {
	       ws2 = ws;
	       ws = ws->next;
	       free(ws2);
	     }
	   p = g_trivia->players;
	   while(p)
	     {
	       p2 = p;
	       p = p->next;
	       free(p2);
	     }
	   free(g_trivia);
	   g_trivia = NULL;
	 }
     }
 }

void send_to_trivia(char *string)
{
  struct player_struct *ch;
  if (g_trivia == NULL) 
    return;
  send_to_char(string, g_trivia->asker);
  for(ch = g_trivia->players; ch; ch=ch->next) 
    {
      send_to_char(string,ch->ch);
    }
}

bool is_trivia_player(CHAR_DATA *ch)
{
  struct player_struct *vch;

  if (g_trivia == NULL)
    return 0;
  if (ch == g_trivia->asker)
    return 1;
  for(vch = g_trivia->players; vch; vch=vch->next)
    {
      if (vch->ch==ch)
	return 1;
    }
  return 0;
}


void do_trivia_score(CHAR_DATA *ch, char *argument)
{
  struct winner_struct *ws;
  char buf[MAX_STRING_LENGTH];
  char buf2[32];
  int i;

  if (g_trivia == NULL)
    {
      send_to_char("No trivia game in progress.\r\n",ch);
      return;
    }
  if (IS_NPC(ch))
    {
      send_to_char("Mobs can't play trivia.\r\n",ch);
      return;
    }
  if (!is_trivia_player(ch))
    {
      send_to_char("You are not in the trivia game.\n\r",ch);
      return;
    }

  sprintf(buf,"Trivia Standings\n\r");
  
  i = 0;
  for(ws=g_trivia->winners; ws; ws = ws->next)
    {
      i++;
      sprintf(buf2,"%2d. %-15s ",i, ws->ch->name);
      strcat(buf, buf2);
      if(i % 3 == 0)
	strcat(buf,"\n\r");
    }
  strcat(buf,"\n\r");
  send_to_char(buf,ch);
}

void do_trivia_chat(CHAR_DATA *ch, char *argument)
{
  char buf[MAX_INPUT_LENGTH];
  if (g_trivia == NULL)
    {
      send_to_char("No trivia game in progress.\r\n",ch);
      return;
    }
  if (IS_NPC(ch))
    {
      send_to_char("Mobs can't play trivia.\r\n",ch);
      return;
    }
  if (!is_trivia_player(ch))
    {
      send_to_char("You are not in the trivia game.\n\r",ch);
      return;
    }
  sprintf(buf,"&C<<&wTChat&C>> &w%s: %s\n\r",ch->name,argument);
  send_to_trivia(buf);
}

void do_trivia_join(CHAR_DATA *ch, char *argument)
{
  char buf[MAX_INPUT_LENGTH];
  struct player_struct *p;
  if (g_trivia == NULL)
    {
      send_to_char("No trivia game in progress.\r\n",ch);
      return;
    }
  if (IS_NPC(ch))
    {
      send_to_char("Mobs can't play trivia.\r\n",ch);
      return;
    }
  if (is_trivia_player(ch))
    {
      send_to_char("You have already joined the game.\n\r",ch);
      return;
    }
  send_to_char("You have joined the trivia game.\n\rType &Ctanswer&W to answer\n\rType &Ctchat&W to use Trivia chat\n\rType &Ctscore&w to see the scores\n\r",ch);
  sprintf(buf,"%s has joined the trivia game.\n\r",ch->name);
  send_to_trivia(buf);
  CREATE(p, struct player_struct, 1);
  p->ch = ch;
  p->next = g_trivia->players;
  g_trivia->players = p;
}
void do_trivia_answer(CHAR_DATA *ch, char *argument)
{
  char buf[MAX_INPUT_LENGTH];
  
  if (IS_NPC(ch))
    {
      send_to_char("Mobs can't play a trivia game.\r\n",ch);
      return;
    }
  
  if(argument[0]=='\0')
    {
      send_to_char("That is a pretty sad answer.\r\n",ch);
      return;
    }
  if (!is_trivia_player(ch))
    {
      send_to_char("You aren't playing trivia.\n\r",ch);
      return;
    }
  sprintf(buf,"&C<<&WAnswer&C>> &W%s: %s\n\r",ch->name,argument);
  send_to_trivia(buf);
}

void do_trivia_question(CHAR_DATA *ch, char *argument)
{
  char buf[MAX_INPUT_LENGTH];

  if (IS_NPC(ch))
    {
      send_to_char("Mobs can't do that.\r\n",ch);
      return;
    }
  if (g_trivia == NULL) 
    {
      send_to_char("There isn't currently a trivia game running.\n\r",ch);
      return;
    }
  if (argument[0] == '\0')
    {
      send_to_char("What is the question?\n\r",ch);
      return;
    }
  if (g_trivia->asker != ch) 
    {
      send_to_char("You aren't the asker.\n\r",ch);
      return;
    }
  g_trivia->current_question++;
  sprintf(buf,"&C<<&WQuestion %d&C>> &W%s\n\r",g_trivia->current_question,argument);
  send_to_trivia(buf);
}

void do_trivia_winner(CHAR_DATA *ch, char *argument)
{
  struct player_struct *winner;
  struct winner_struct *wlist;
  struct winner_struct *prev = NULL;
  struct winner_struct *ws = NULL;
  OBJ_DATA *obj;
  char buf[MAX_INPUT_LENGTH];

  if (IS_NPC(ch))
    {
      send_to_char("Mobs can't do that.\r\n",ch);
      return;
    }
  if (g_trivia == NULL) 
    {
      send_to_char("There isn't currently a trivia game running.\n\r",ch);
      return;
    }
  if (argument[0] == '\0')
    {
      send_to_char("Who got the correct answer?\n\r",ch);
      return;
    }
  if (g_trivia->asker != ch) 
    {
      send_to_char("You aren't the asker.\n\r",ch);
      return;
    }
  //Check to see if name given is in the trivia player list
  for(winner = g_trivia->players; winner; winner = winner->next) 
    {
      if (!strcasecmp(winner->ch->name,argument)) 
	{
	  //See if they already won a question
	  for(wlist = g_trivia->winners; wlist; wlist=wlist->next)
	    {
	      if (winner->ch == wlist->ch) 
		{
		  prev->next = wlist->next;
		  wlist->correct++;
	          prev = NULL;
		  for(ws = g_trivia->winners; ws; ws=ws->next)
		    {
		      if (wlist->correct > ws->correct || ws->next == NULL)
			{
			  if (prev == NULL)
			    break;
			  wlist->next = prev->next;
			  prev->next = wlist;
			}
			prev = ws;
		    }
		  break;
		}
	      prev = wlist;
	    }
	  //Nope, they haven't one a question yet, so make an entry
	  if (wlist == NULL)
	    {
	      CREATE(ws,struct winner_struct, 1);
	      ws->correct = 1;
	      ws->ch = get_char_world(ch,argument);
	      if (ws->ch == NULL)
		{
		  send_to_char("I can't find that winner!\n\r",ch);
		  free(ws);
		  return;
		}
	      if (g_trivia->winners == NULL)
		{
		  g_trivia->winners = ws;
		  ws->next = NULL;
		} else {
		  for(wlist = g_trivia->winners; wlist; wlist=wlist->next)
		    {
		      if (wlist->next == NULL) {
			wlist->next = ws;
			ws->next = NULL;
			break;
		      }  
		    }
		}
	    }
	  break;
	}
    }
  if (winner == NULL) 
    {
      send_to_char("There is no one with that name playing\n\r",ch);
      return;
    }
  sprintf(buf,"&C<<&wWinner&C>> &w%s has won question %d!\n\r",argument,g_trivia->current_question);
  send_to_trivia(buf);
//GIVE PRIZE!
  if (g_trivia->prize != -1) 
    {
      obj = create_object( get_obj_index(g_trivia->prize), get_trust(ch) );
      if ( CAN_WEAR(obj, ITEM_TAKE) )
	{
	  obj = obj_to_char( obj, winner->ch );
	  send_to_char("The trivia gods have given you a prize for your great answer!\n\r",winner->ch);
	}      
    }
}
