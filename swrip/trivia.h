#ifndef TRIVIA_H
#define TRIVIA_H
#include "mud.h"
struct trivia_struct
{
  int prize;
  int current_question;
  CHAR_DATA *asker;
  struct winner_struct *winners;
  struct player_struct *players;
};

struct winner_struct
{
  CHAR_DATA *ch;
  int correct;
  struct winner_struct *next;
};

struct player_struct
{  
  CHAR_DATA *ch;
  struct player_struct *next;
};

struct trivia_struct *g_trivia = NULL;

void do_trivia(CHAR_DATA *ch, char *argument);
void send_to_trivia(char *string);
bool is_trivia_player(CHAR_DATA *ch);
void do_trivia_join(CHAR_DATA *ch, char *argument);
void do_trivia_answer(CHAR_DATA *ch, char *argument);
void do_trivia_question(CHAR_DATA *ch, char *argument);
void do_trivia_winner(CHAR_DATA *ch, char *argument);
void do_trivia_chat(CHAR_DATA *ch, char *argument);
#endif
