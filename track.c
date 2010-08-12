#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "mud.h"

#define BFS_ERROR	   -1
#define BFS_ALREADY_THERE  -2
#define BFS_NO_PATH	   -3
#define BFS_MARK         BV01

#define TRACK_THROUGH_DOORS

bool mob_snipe( CHAR_DATA *ch , CHAR_DATA *victim);
ch_ret  one_hit             args( ( CHAR_DATA *ch, CHAR_DATA *victim, int dt ) );

/* You can define or not define TRACK_THOUGH_DOORS, above, depending on
   whether or not you want track to find paths which lead through closed
   or hidden doors.
*/

struct bfs_queue_struct {
   ROOM_INDEX_DATA *room;
   char   dir;
   struct bfs_queue_struct *next;
};

static struct bfs_queue_struct	*queue_head = NULL,
				*queue_tail = NULL,
				*room_queue = NULL;

/* Utility macros */
#define MARK(room)	(SET_BIT(	(room)->room_flags, BFS_MARK) )
#define UNMARK(room)	(REMOVE_BIT(	(room)->room_flags, BFS_MARK) )
#define IS_MARKED(room)	(IS_SET(	(room)->room_flags, BFS_MARK) )

ROOM_INDEX_DATA *toroom( ROOM_INDEX_DATA *room, sh_int door )
{
    return (get_exit( room, door )->to_room);
}

bool valid_edge( ROOM_INDEX_DATA *room, sh_int door )
{
    EXIT_DATA *pexit;
    ROOM_INDEX_DATA *to_room;

    pexit = get_exit( room, door );
    if ( pexit
    &&  (to_room = pexit->to_room) != NULL
#ifndef TRACK_THROUGH_DOORS
    &&  !IS_SET( pexit->exit_info, EX_CLOSED )
#endif
    &&  !IS_MARKED( to_room ) )
      return TRUE;
    else
      return FALSE;
}

void bfs_enqueue(ROOM_INDEX_DATA *room, char dir)
{
   struct bfs_queue_struct *curr;

   CREATE( curr, struct bfs_queue_struct, 1 );
   curr->room = room;
   curr->dir = dir;
   curr->next = NULL;

   if (queue_tail) {
      queue_tail->next = curr;
      queue_tail = curr;
   } else
      queue_head = queue_tail = curr;
}


void bfs_dequeue(void)
{
   struct bfs_queue_struct *curr;

   curr = queue_head;

   if (!(queue_head = queue_head->next))
      queue_tail = NULL;
   DISPOSE(curr);
}


void bfs_clear_queue(void) 
{
   while (queue_head)
      bfs_dequeue();
}

void room_enqueue(ROOM_INDEX_DATA *room)
{
   struct bfs_queue_struct *curr;

   CREATE( curr, struct bfs_queue_struct, 1 );
   curr->room = room;
   curr->next = room_queue;

   room_queue = curr;
}

void clean_room_queue(void) 
{
   struct bfs_queue_struct *curr, *curr_next;

   for (curr = room_queue; curr; curr = curr_next )
   {
      UNMARK( curr->room );
      curr_next = curr->next;
      DISPOSE( curr );
   }
   room_queue = NULL;
}


int find_first_step(ROOM_INDEX_DATA *src, ROOM_INDEX_DATA *target, int maxdist )
{
   int curr_dir, count;

   if ( !src || !target )
   {
      bug("Illegal value passed to find_first_step (track.c)", 0 );
      return BFS_ERROR;
   }

   if (src == target)
      return BFS_ALREADY_THERE;

   if ( src->area != target->area )
      return BFS_NO_PATH;

   room_enqueue( src );
   MARK(src);

   /* first, enqueue the first steps, saving which direction we're going. */
   for (curr_dir = 0; curr_dir < 10; curr_dir++)
      if (valid_edge(src, curr_dir))
      {
         MARK(toroom(src, curr_dir));
	 room_enqueue(toroom(src, curr_dir));
         bfs_enqueue(toroom(src, curr_dir), curr_dir);
      }

   count = 0;
   while (queue_head) {
      if ( ++count > maxdist )
      {
	bfs_clear_queue();
	clean_room_queue();
	return BFS_NO_PATH;
      }
      if (queue_head->room == target) {
	 curr_dir = queue_head->dir;
	 bfs_clear_queue();
	 clean_room_queue();
	 return curr_dir;
      } else {
         for (curr_dir = 0; curr_dir < 10; curr_dir++)
            if (valid_edge(queue_head->room, curr_dir)) {
               MARK(toroom(queue_head->room, curr_dir));
	       room_enqueue(toroom(queue_head->room, curr_dir));
	       bfs_enqueue(toroom(queue_head->room, curr_dir),queue_head->dir);
            }
         bfs_dequeue();
      }
   }
   clean_room_queue();

   return BFS_NO_PATH;
}


void do_track( CHAR_DATA *ch, char *argument )
{
   CHAR_DATA *vict;
   char arg[MAX_INPUT_LENGTH];
   char buf[MAX_STRING_LENGTH];
   int dir, maxdist;

   if ( !IS_NPC(ch) && !ch->pcdata->learned[gsn_track] )
   {
	send_to_char("> &Ryou do not know this skillsoft yet&w\n\r", ch );
	return;
   }

   one_argument(argument, arg);
   if ( arg[0]=='\0' ) {
      send_to_char("> &Rwhom are you trying to trace?&w\n\r", ch);
      return;
   }

   WAIT_STATE( ch, skill_table[gsn_track]->beats );

   if (!(vict = get_char_world(ch, arg))) {
      send_to_char("> &Ryou cannot find a trace of anyone like that&w\n\r", ch);
      return;
   }

   maxdist = 100 + ch->top_level * 30;

   if ( !IS_NPC(ch) )
     maxdist = (maxdist * ch->pcdata->learned[gsn_track]) / 100;

   dir = find_first_step(ch->in_room, vict->in_room, maxdist);
   switch(dir) {
      case BFS_ERROR:
         send_to_char("> &Rhmm... something seems to be wrong&w\n\r", ch);
         break;
      case BFS_ALREADY_THERE:
         send_to_char("> &Ryou're already in the same room&w\n\r", ch);
         break;
      case BFS_NO_PATH:
         sprintf(buf, "> &Ryou can't trace them from here\n\r" );
         send_to_char(buf, ch);
         learn_from_failure( ch, gsn_track );
         break;
      default:
         ch_printf(ch, "> &Yyou trace them %s from here...&w\n\r", dir_name[dir]);
	 learn_from_success( ch, gsn_track );
         break;
   }
}

void do_npctrack( CHAR_DATA *ch, char *argument )
{
   CHAR_DATA *vict;
   char arg[MAX_INPUT_LENGTH];
   char buf[MAX_STRING_LENGTH];
   int dir, maxdist;

   if ( !IS_NPC(ch) && !ch->pcdata->learned[gsn_npctrack] )
   {
	send_to_char("> &Ryou do not know this skillsoft yet&w\n\r", ch );
	return;
   }

   one_argument(argument, arg);
   if ( arg[0]=='\0' ) {
      send_to_char("> &Rwhom are you trying to trace?&w\n\r", ch);
      return;
   }

   WAIT_STATE( ch, skill_table[gsn_npctrack]->beats );

   if (!(vict = get_char_world(ch, arg))) {
      send_to_char("> &Ryou cannot find a trace of anyone like that&w\n\r", ch);
      return;
   }

   if ( !IS_NPC(vict) ){

	      send_to_char("> &Ryou cannot use systrace on players. only on system entities.&w\n\r", ch);
	      return;

   }

   maxdist = 100 + ch->top_level * 30;

   if ( !IS_NPC(ch) )
     maxdist = (maxdist * ch->pcdata->learned[gsn_npctrack]) / 100;

   dir = find_first_step(ch->in_room, vict->in_room, maxdist);

   switch(dir) {
      case BFS_ERROR:
         send_to_char("> &Rhmm... something seems to be wrong&w\n\r", ch);
         break;
      case BFS_ALREADY_THERE:
         send_to_char("> &Ryou're already in the same room&w\n\r", ch);
         break;
      case BFS_NO_PATH:
         sprintf(buf, "> &Ryou can't trace them from here\n\r" );
         send_to_char(buf, ch);
         learn_from_failure( ch, gsn_npctrack );
         break;
      default:
         ch_printf(ch, "> &Yyou trace them %s from here...&w\n\r", dir_name[dir]);
	 learn_from_success( ch, gsn_npctrack );
         break;
   }
}

void do_sn_nodescanner( CHAR_DATA *ch, char *argument )
{
   CHAR_DATA *vict;
   char buf [MAX_STRING_LENGTH];
   char arg[MAX_INPUT_LENGTH];
   int dir, maxdist, chance;
   ROOM_INDEX_DATA *location;
   bool ch_snippet;
   OBJ_DATA *obj;

   if ( !IS_NPC(ch) && !ch->pcdata->learned[gsn_spacecraft] )
   {
	send_to_char("> &Ryou can not trace nodes without decking&w\n\r", ch );
	return;
   }

   one_argument(argument, arg);
   if ( arg[0]=='\0' ) {
      send_to_char("> &Yplease specify the id of the target node&w\n\r", ch);
      return;
   }

   if (atoi(arg) < 0){

	      send_to_char("> &Rinvalid node id&w\n\r", ch);
	      return;

   }

	ch_snippet = FALSE;

	for (obj = ch->last_carrying; obj; obj = obj->prev_content) {
		if (obj->item_type == ITEM_SNIPPET && !strcmp(obj->name,
				"nodescanner") && ch_snippet == FALSE) {
			ch_snippet = TRUE;

			obj->value[0] -= 1;

			if (obj->value[0] < 1)
			{
			separate_obj(obj);
			obj_from_char(obj);
			extract_obj( obj );
			send_to_char("> &Rnodescanner application has expired&w\n\r", ch);
			}

		}

	}

if (!ch_snippet) {
	send_to_char("> &Rnodescanner application needed&w\n\r", ch);
	return;
}

sprintf(buf,"> &y%s uses a nodescanner application", ch->name);
act(AT_WHITE,buf, ch, NULL, NULL, TO_ROOM);

   WAIT_STATE( ch, skill_table[gsn_propaganda]->beats );

	chance = IS_NPC(ch) ? ch->top_level
			: (int) (ch->pcdata->learned[gsn_spacecraft]);

	if (number_range(1, 100) > chance){

	      send_to_char("> &Ryou fail to use the nodescanner application&w\n\r", ch);
	      return;

	}

   location = get_room_index( atoi( arg ) );

   if (location == ch->in_room){

	      send_to_char("> &Yyou are already in that node&w\n\r", ch);
	      return;
   }

   if (location == NULL){

	      send_to_char("> &Rinvalid node id&w\n\r", ch);
	      return;
   }

   if (location->area->planet != ch->in_room->area->planet){

	      send_to_char("> &Rtarget node is not in system&w\n\r", ch);
	      return;
   }

   if (!location->area->planet->governed_by){

	      send_to_char("> &Ryou can not trace nodes in this system&w\n\r", ch);
	      return;

   }

   maxdist = 100 + ch->top_level * 30;

   vict = create_mobile(get_mob_index(9));

   char_to_room(vict, location);

   dir = find_first_step(ch->in_room, vict->in_room, maxdist);

   extract_char( vict, TRUE );

   switch(dir) {
      case BFS_ERROR:
         send_to_char("> &Rhmm... something seems to be wrong&w\n\r", ch);
         break;
      case BFS_ALREADY_THERE:
         send_to_char("> &Ryou're already in that node&w\n\r", ch);
         break;
      case BFS_NO_PATH:
         sprintf(buf, "> &Ryou can't trace that node from here\n\r" );
         send_to_char(buf, ch);
         learn_from_failure( ch, gsn_spacecraft );
         break;
      default:
         ch_printf(ch, "> &Yyou trace that node %s from here...&w\n\r", dir_name[dir]);
	 learn_from_success( ch, gsn_spacecraft );
         break;
   }
}

void do_listnodes( CHAR_DATA *ch, char *argument )
{

	ROOM_INDEX_DATA * room;
	bool found = FALSE;

   if ( IS_NPC(ch) )
	   return;

   if ( !IS_NPC(ch) && !ch->pcdata->learned[gsn_spacecraft] )
   {
	send_to_char("> &Ryou can not list nodes without decking&w\n\r", ch );
	return;
   }

   if ( argument[0]=='\0' ) {
      send_to_char("> &Yplease specify the security level of the target node&w\n\r", ch);
      send_to_char("> &Y0 = blue, 1 = green, 2 = orange, 3 = red, 4 = ultra-violet&w\n\r", ch);
      return;
   }

   if (atoi(argument) < 0 && atoi(argument) < 4){

	      send_to_char("> &Rinvalid security level&w\n\r", ch);
	      return;

   }

   if ( !ch->pcdata->clan ){

	      send_to_char("> &Ryou have to be in an organization to list nodes&w\n\r", ch);
	      return;

   }

   if ( !ch->in_room->area->planet->governed_by ){

	      send_to_char("> &Ryou can not list nodes in this system&w\n\r", ch);
	      return;

   }

   if ( ch->in_room->area->planet->governed_by != ch->pcdata->clan){

	      send_to_char("> &Ryou can only list nodes in systems that your organization controls&w\n\r", ch);
	      return;

   }

   send_to_char("&pNODES&w\n\r", ch);
   send_to_char("&W==========================================&w\n\r", ch);

   int counter = 0;

   for ( room = ch->in_room->area->first_room; room ; room = room->next_in_area ){

	   if (room->level != atoi(argument))
			   continue;

	  found = TRUE;
      pager_printf( ch, "%6ld) %s\n\r", room->vnum, room->name);

      counter++;

      if (counter >= 25){

    	  send_to_char("&W==========================================&w\n\r", ch);
    	  send_to_char("&Wtoo many matches...&w\n\r", ch);
    	  break;
      }

   }

   if (!found)
	   send_to_char("> &Yno matches&w\n\r", ch);

   WAIT_STATE( ch, skill_table[gsn_propaganda]->beats );

   return;

}


void found_prey( CHAR_DATA *ch, CHAR_DATA *victim )
{
     char buf[MAX_STRING_LENGTH];
     char victname[MAX_STRING_LENGTH];

     

     if (victim == NULL)
     {
	bug("Found_prey: null victim", 0);
	return;
     }
     
     if (ch == NULL)
     {
	bug("Found_prey: null ch", 0);
	return;
     }

     if ( victim->in_room == NULL )
     {
        bug( "Found_prey: null victim->in_room", 0 );
        return;
     }

     strcpy( victname, IS_NPC( victim ) ? victim->short_descr : victim->name );
     //sprintf( victname, IS_NPC( victim ) ? victim->short_descr : victim->name );

     if ( !can_see(ch, victim) )
     {
        if ( number_percent( ) < 90 )
	  return;
	switch( number_bits( 2 ) )
 	{
	case 0: sprintf( buf, "Don't make me find you, %s!", victname );
		do_say( ch, buf );
	        break;
	case 1: act( AT_ACTION, "$n sniffs around the room for $N.", ch, NULL, victim, TO_NOTVICT );
		act( AT_ACTION, "You sniff around the room for $N.", ch, NULL, victim, TO_CHAR );
		act( AT_ACTION, "$n sniffs around the room for you.", ch, NULL, victim, TO_VICT );
		sprintf( buf, "I can smell your blood!" );
		do_say( ch, buf );
		break;
	case 2: sprintf( buf, "I'm going to tear %s apart!", victname );
		do_yell( ch, buf );
		break;
	case 3: do_say( ch, "Just wait until I find you...");
		break;
        }
	return;
     }

     if ( IS_SET( ch->in_room->room_flags, ROOM_SAFE ) )
     {
	if ( number_percent( ) < 90 )
	  return;
	switch( number_bits( 2 ) )
	{
	case 0:	do_say( ch, "C'mon out, you coward!" );
		sprintf( buf, "%s is a bloody coward!", victname );
		do_yell( ch, buf );
		break;
	case 1: sprintf( buf, "Let's take this outside, %s", victname );
		do_say( ch, buf );
		break;
	case 2: sprintf( buf, "%s is a yellow-bellied wimp!", victname );
		do_yell( ch, buf );
		break;
	case 3: act( AT_ACTION, "$n takes a few swipes at $N.", ch, NULL, victim, TO_NOTVICT );
		act( AT_ACTION, "You try to take a few swipes $N.", ch, NULL, victim, TO_CHAR );
		act( AT_ACTION, "$n takes a few swipes at you.", ch, NULL, victim, TO_VICT );
		break;
	}
	return;
     }

     act( AT_ACTION, "$n lunges at $N from out of nowhere!", ch, NULL, victim, TO_NOTVICT );
     act( AT_ACTION, "You lunge at $N catching $M off guard!", ch, NULL, victim, TO_CHAR );
     act( AT_ACTION, "$n lunges at you from out of nowhere!", ch, NULL, victim, TO_VICT );

     stop_hunting( ch );
     set_fighting( ch, victim );
     multi_hit(ch, victim, TYPE_UNDEFINED);
     return;
} 

void hunt_victim( CHAR_DATA *ch )
{
   bool found;
   CHAR_DATA *tmp;
   sh_int ret;

   if (!ch || !ch->hunting || !ch->hunting->who )
      return;

   /* make sure the char still exists */
   for (found = FALSE, tmp = first_char; tmp && !found; tmp = tmp->next)
      if (ch->hunting->who == tmp)
         found = TRUE;

   if (!found)
   {
      do_say(ch, "Damn!  My prey is gone!!" );
      stop_hunting( ch );
      return;
   }

   if ( ch->in_room == ch->hunting->who->in_room )
   {
     if ( ch->fighting )
       return;
     found_prey( ch, ch->hunting->who );
     return;
   }
   
/* hunting with snipe */ 
   {
	OBJ_DATA *wield;
	  
        wield = get_eq_char( ch, WEAR_WIELD );
	if ( wield != NULL && wield->value[3] == WEAPON_BLASTER  )
	{
	  if ( mob_snipe( ch, ch->hunting->who ) == TRUE ) 
	   return;
        }
        else if ( !IS_SET( ch->act, ACT_DROID ) )
           do_hide ( ch, "" );
   }
   
   ret = find_first_step(ch->in_room, ch->hunting->who->in_room, 5000);
   if ( ret == BFS_NO_PATH )
   {
       EXIT_DATA *pexit;
       int attempt;
       
       for ( attempt = 0; attempt < 25; attempt++ )
       {
	  ret = number_door( );
	  if ( ( pexit = get_exit(ch->in_room, ret) ) == NULL
	  ||   !pexit->to_room
	  || IS_SET(pexit->exit_info, EX_CLOSED)
	  || IS_SET(pexit->to_room->room_flags, ROOM_NO_MOB) )
	     continue;
       }
   }
   if ( ret < 0)
   {
      do_say( ch, "Damn!  Lost my prey!" );
      stop_hunting( ch );
      return;
   }
   else
   {
      move_char( ch, get_exit( ch->in_room, ret), FALSE );
      if ( char_died(ch) )
          return;
      if ( !ch->hunting )
      {
        if ( !ch->in_room )
        {
          char buf[MAX_STRING_LENGTH];
          sprintf( buf, "Hunt_victim: no ch->in_room!  Mob #%ld, name: %s.  Placing mob in limbo.",
                   ch->pIndexData->vnum, ch->name );
          bug( buf, 0 );
          char_to_room( ch, get_room_index( ROOM_VNUM_LIMBO ) );
          return;
        } 
	do_say( ch, "Damn!  Lost my prey!" );
	return;
      }
      if (ch->in_room == ch->hunting->who->in_room)
	found_prey( ch, ch->hunting->who );
      return;
   }
}

bool mob_snipe( CHAR_DATA *ch, CHAR_DATA *victim )
{
   sh_int            dir, dist;
   sh_int            max_dist = 3;
   EXIT_DATA       * pexit;
   ROOM_INDEX_DATA * was_in_room;
   ROOM_INDEX_DATA * to_room;
   char              buf[MAX_STRING_LENGTH];
   bool              pfound = FALSE;

 if ( !ch->in_room || !victim->in_room )
        return FALSE;
   
 if ( IS_SET( ch->in_room->room_flags, ROOM_SAFE ) )
	return FALSE;
   
 for ( dir = 0 ; dir <= 10 ; dir++ )
 {  
   if ( ( pexit = get_exit( ch->in_room, dir ) ) == NULL )
     continue;

   if ( IS_SET( pexit->exit_info, EX_CLOSED ) )
     continue;
     
   was_in_room = ch->in_room;
   
   for ( dist = 0; dist <= max_dist; dist++ )   
   {
     if ( IS_SET( pexit->exit_info, EX_CLOSED ) )
        break; 
     
     if ( !pexit->to_room )
        break;
     
       to_room = pexit->to_room;
    
     char_from_room( ch );
     char_to_room( ch, to_room );    
     

     if ( ch->in_room == victim->in_room )
     {
        pfound = TRUE;
        break;
     }

     if ( ( pexit = get_exit( ch->in_room, dir ) ) == NULL )
        break;
            
   }
   
   char_from_room( ch );
   char_to_room( ch, was_in_room );    
       
   if ( !pfound )
   {
       char_from_room( ch );
       char_to_room( ch, was_in_room );    
       continue;
   }
   
    if ( IS_SET( victim->in_room->room_flags, ROOM_SAFE ) )
	return FALSE;
 
    if ( is_safe( ch, victim ) )
	return FALSE;
    
    if ( IS_AFFECTED(ch, AFF_CHARM) && ch->master == victim )
	return FALSE;

    if ( ch->position == POS_FIGHTING )
	return FALSE;
    
    switch ( dir )
    {
        case 0:
        case 1:
           dir += 2;
           break;
        case 2:
        case 3:
           dir -= 2;
           break;
        case 4:
        case 7:
           dir += 1;
           break;
        case 5:
        case 8:
           dir -= 1;
           break;
        case 6:
           dir += 3;
           break;
        case 9:
           dir -=3;
           break;
    }
    
    char_from_room( ch );
    char_to_room( ch, victim->in_room );    
                
       sprintf( buf , "A blaster shot fires at you from the %s." , dir_name[dir] );
       act( AT_ACTION, buf , victim, NULL, ch, TO_CHAR );      
       act( AT_ACTION, "You fire at $N.", ch, NULL, victim, TO_CHAR );         
       sprintf( buf, "A blaster shot fires at $N from the %s." , dir_name[dir] );
       act( AT_ACTION, buf, ch, NULL, victim, TO_NOTVICT );  
                                                   
       one_hit( ch, victim, TYPE_UNDEFINED );  
       
       if ( char_died(ch) )
          return TRUE;
       
       stop_fighting( ch , TRUE );

       if ( victim && !char_died(victim) && victim->hit < 0 )
       {
              stop_hunting( ch );
              stop_hating( ch );
       }
       
    char_from_room( ch );
    char_to_room( ch, was_in_room );    
     
    return TRUE;
 }  
 
 return FALSE;
}
