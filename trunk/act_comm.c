#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "mud.h"

/*
 *  Externals
 */
void send_obj_page_to_char(CHAR_DATA * ch, OBJ_INDEX_DATA * idx, char page);
void send_room_page_to_char(CHAR_DATA * ch, ROOM_INDEX_DATA * idx, char page);
void send_page_to_char(CHAR_DATA * ch, MOB_INDEX_DATA * idx, char page);
void send_control_page_to_char(CHAR_DATA * ch, char page);
void sportschan(char *);

/*
 * Local functions.
 */
void	talk_channel	args( ( CHAR_DATA *ch, char *argument,
			    int channel, const char *verb ) );

char *  scramble        args( ( const char *argument, int modifier ) );			    
char *  drunk_speech    args( ( const char *argument, CHAR_DATA *ch ) ); 
void    channel_noise   args( ( ) );

#define	MAX_NOISE	1

char * noise_string[ MAX_NOISE ] =
{
    "A Human: Doh.",    
}; 

void sound_to_room( ROOM_INDEX_DATA *room , char *argument )
{
   CHAR_DATA *vic;

        if ( room == NULL ) return;
        
        for ( vic = room->first_person; vic; vic = vic->next_in_room )
	   if ( !IS_NPC(vic) && IS_SET( vic->act, PLR_SOUND ) )
	     send_to_char( argument, vic );
     
}


void do_beep( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    char arg[MAX_STRING_LENGTH];
    char buf[MAX_STRING_LENGTH];
    OBJ_DATA *obj;
    bool ch_comlink, victim_comlink;
    
    argument = one_argument( argument, arg );
    
    REMOVE_BIT( ch->deaf, CHANNEL_TELLS );
                                
    if (!IS_NPC(ch)
        && ( IS_SET(ch->act, PLR_SILENCE)
        ||   IS_SET(ch->act, PLR_NO_TELL) ) )
    {
         send_to_char( "> invalid command\n\r", ch );
         return;
    }
                                    
    if ( arg[0] == '\0' )
    {
         send_to_char( "> specify user\n\r", ch );
         return;
    }
                            
    if ( ( victim = get_char_world( ch, arg ) ) == NULL 
       || ( IS_NPC(victim) && victim->in_room != ch->in_room ) 
       || (!NOT_AUTHED(ch) && NOT_AUTHED(victim) && !IS_IMMORTAL(ch) ) )
    {
	send_to_char( "> user not found\n\r", ch );
	return;
    }
    
      ch_comlink = FALSE;
      victim_comlink = FALSE;
      
      if ( IS_IMMORTAL( ch ) )
      {
         ch_comlink = TRUE;
         victim_comlink = TRUE;
      }
      
      if ( IS_IMMORTAL( victim ) )
         victim_comlink = TRUE;
      
      for ( obj = ch->last_carrying; obj; obj = obj->prev_content )
      {
        if (obj->pIndexData->item_type == ITEM_COMLINK)
        ch_comlink = TRUE;
      }
    
      if ( !ch_comlink )
      {
	send_to_char( "> comlink module needed\n\r", ch);
	return;
      }
      
      for ( obj = victim->last_carrying; obj; obj = obj->prev_content )
      {
        if (obj->pIndexData->item_type == ITEM_COMLINK)
        victim_comlink = TRUE;
      }

      if ( !victim_comlink )
      {
	send_to_char( "> recipient needs comlink module\n\r", ch);
	return;
      }
    
    if (NOT_AUTHED(ch) && !NOT_AUTHED(victim) && !IS_IMMORTAL(victim) )
    {
	send_to_char( "> they cannot hear you because you are not authorized\n\r", ch);
	return;
    }

    if ( !IS_NPC( victim ) && ( victim->switched ) 
	&& IS_IMMORTAL( ch ) )
    {
      send_to_char( "> that user is switched\n\r", ch );
      return;
    }

   else if ( !IS_NPC( victim ) && ( !victim->desc ) )
    {
      send_to_char( "> that user is link-dead\n\r", ch );
      return;
    }

    if ( IS_SET( victim->deaf, CHANNEL_TELLS ) 
    && ( !IS_IMMORTAL( ch ) || ( get_trust( ch ) < get_trust( victim ) ) ) )
    {
      act( AT_PLAIN, "> $E has $S tells turned off", ch, NULL, victim,
		TO_CHAR );
      return;
    }

    if ( !IS_NPC (victim) && ( IS_SET (victim->act, PLR_SILENCE ) ) )
      {
      send_to_char( "> that user is silenced - they will receive your message but can not respond\n\r", ch );
      }   

    if ( !IS_IMMORTAL(ch) && !IS_AWAKE(victim) )
    {
    act( AT_PLAIN, "> $E cannot receive your message", ch, 0, victim, TO_CHAR );
	return;
    }

    if ( victim->desc		/* make sure desc exists first  -Thoric */
    &&   victim->desc->connected == CON_EDITING 
    &&   IS_IMMORTAL(ch) )
    {
	act( AT_PLAIN, "> $E is currently in a writing buffer", ch, 0, victim, TO_CHAR );
        return;
    }

    ch_printf(ch , "&W[BEEP] You <-> %s: %s\n\r\a" , victim->name, argument );
    send_to_char("\a",victim);    

    sprintf( buf , "[BEEP] %s: '$t'" , ch->name );   

    act( AT_WHITE, buf, ch, argument, victim, TO_VICT );
}

/* Text scrambler -- Altrag */
char *scramble( const char *argument, int modifier )
{
    static char arg[MAX_INPUT_LENGTH];
    sh_int position;
    sh_int conversion = 0;
    
	modifier %= number_range( 80, 300 ); /* Bitvectors get way too large #s */
    for ( position = 0; position < MAX_INPUT_LENGTH; position++ )
    {
    	if ( argument[position] == '\0' )
    	{
    		arg[position] = '\0';
    		return arg;
    	}
    	else if ( argument[position] >= 'A' && argument[position] <= 'Z' )
	    {
	    	conversion = -conversion + position - modifier + argument[position] - 'A';
	    	conversion = number_range( conversion - 5, conversion + 5 );
	    	while ( conversion > 25 )
	    		conversion -= 26;
	    	while ( conversion < 0 )
	    		conversion += 26;
	    	arg[position] = conversion + 'A';
	    }
	    else if ( argument[position] >= 'a' && argument[position] <= 'z' )
	    {
	    	conversion = -conversion + position - modifier + argument[position] - 'a';
	    	conversion = number_range( conversion - 5, conversion + 5 );
	    	while ( conversion > 25 )
	    		conversion -= 26;
	    	while ( conversion < 0 )
	    		conversion += 26;
	    	arg[position] = conversion + 'a';
	    }
	    else if ( argument[position] >= '0' && argument[position] <= '9' )
	    {
	    	conversion = -conversion + position - modifier + argument[position] - '0';
	    	conversion = number_range( conversion - 2, conversion + 2 );
	    	while ( conversion > 9 )
	    		conversion -= 10;
	    	while ( conversion < 0 )
	    		conversion += 10;
	    	arg[position] = conversion + '0';
	    }
	    else
	    	arg[position] = argument[position];
	}
	arg[position] = '\0';
	return arg;	     
}

/* I'll rewrite this later if its still needed.. -- Altrag */
char *translate( CHAR_DATA *ch, CHAR_DATA *victim, const char *argument )
{
	return "";
}

char *drunk_speech( const char *argument, CHAR_DATA *ch )
{
  const char *arg = argument;
  static char buf[MAX_INPUT_LENGTH*2];
  char buf1[MAX_INPUT_LENGTH*2];
  sh_int drunk;
  char *txt;
  char *txt1;  

  if ( IS_NPC( ch ) || !ch->pcdata ) return (char *) argument;

  drunk = ch->pcdata->condition[COND_DRUNK];

  if ( drunk <= 0 )
    return (char *) argument;

  buf[0] = '\0';
  buf1[0] = '\0';

  if ( !argument )
  {
     bug( "Drunk_speech: NULL argument", 0 );
     return "";
  }

  /*
  if ( *arg == '\0' )
    return (char *) argument;
  */

  txt = buf;
  txt1 = buf1;

  while ( *arg != '\0' )
  {
    if ( toupper(*arg) == 'S' )
    {
	if ( number_percent() < ( drunk * 2 ) )		/* add 'h' after an 's' */
	{
	   *txt++ = *arg;
	   *txt++ = 'h';
	}
       else
	*txt++ = *arg;
    }
   else if ( toupper(*arg) == 'X' )
    {
	if ( number_percent() < ( drunk * 2 / 2 ) )
	{
	  *txt++ = 'c', *txt++ = 's', *txt++ = 'h';
	}
       else
	*txt++ = *arg;
    }
   else if ( number_percent() < ( drunk * 2 / 5 ) )  /* slurred letters */
    {
      sh_int slurn = number_range( 1, 2 );
      sh_int currslur = 0;	

      while ( currslur < slurn )
	*txt++ = *arg, currslur++;
    }
   else
    *txt++ = *arg;

    arg++;
  };

  *txt = '\0';

  txt = buf;

  while ( *txt != '\0' )   /* Let's mess with the string's caps */
  {
    if ( number_percent() < ( 2 * drunk / 2.5 ) )
    {
      if ( isupper(*txt) )
        *txt1 = tolower( *txt );
      else
      if ( islower(*txt) )
        *txt1 = toupper( *txt );
      else
        *txt1 = *txt;
    }
    else
      *txt1 = *txt;

    txt1++, txt++;
  };

  *txt1 = '\0';
  txt1 = buf1;
  txt = buf;

  while ( *txt1 != '\0' )   /* Let's make them stutter */
  {
    if ( *txt1 == ' ' )  /* If there's a space, then there's gotta be a */
    {			 /* along there somewhere soon */

      while ( *txt1 == ' ' )  /* Don't stutter on spaces */
        *txt++ = *txt1++;

      if ( ( number_percent() < ( 2 * drunk / 4 ) ) && *txt1 != '\0' )
      {
	sh_int offset = number_range( 0, 2 );
	sh_int pos = 0;

	while ( *txt1 != '\0' && pos < offset )
	  *txt++ = *txt1++, pos++;

	if ( *txt1 == ' ' )  /* Make sure not to stutter a space after */
	{		     /* the initial offset into the word */
	  *txt++ = *txt1++;
	  continue;
	}

	pos = 0;
	offset = number_range( 2, 4 );	
	while (	*txt1 != '\0' && pos < offset )
	{
	  *txt++ = *txt1;
	  pos++;
	  if ( *txt1 == ' ' || pos == offset )  /* Make sure we don't stick */ 
	  {		               /* A hyphen right before a space	*/
	    txt1--;
	    break;
	  }
	  *txt++ = '-';
	}
	if ( *txt1 != '\0' )
	  txt1++;
      }     
    }
   else
    *txt++ = *txt1++;
  }

  *txt = '\0';

  return buf;
}

/*
 * Generic channel function.
 */
void talk_channel( CHAR_DATA *ch, char *argument, int channel, const char *verb )
{
    char buf[MAX_STRING_LENGTH];
    DESCRIPTOR_DATA *d;
    int position;
    CLAN_DATA *clan = NULL;
    PLANET_DATA * planet;        
    bool  ch_comlink = FALSE;
    
    if ( channel != CHANNEL_YELL && channel != CHANNEL_IMMTALK && channel != CHANNEL_OOC 
         && channel != CHANNEL_NEWBIE && channel != CHANNEL_WCHAT && channel != CHANNEL_TRADE && channel != CHANNEL_HINT && channel != CHANNEL_SYSTEM && channel != CHANNEL_SHIP )
    {
      OBJ_DATA *obj;
      
      if ( IS_IMMORTAL( ch ) )
          ch_comlink = TRUE;
      //if ( ch->pcdata->cyber & CYBER_COMM )
      //   ch_comlink = TRUE;
      else
        for ( obj = ch->last_carrying; obj; obj = obj->prev_content )
        {
           if (obj->pIndexData->item_type == ITEM_COMLINK)
           ch_comlink = TRUE;
        }
    
      if ( !ch_comlink )
      {
	send_to_char( "> comlink module needed\n\r", ch);
	return;
      }

    }
      
     
    if ( IS_NPC( ch ) && channel == CHANNEL_CLAN )
    {
	send_to_char( "> ICE cannot be in clans\n\r", ch );
	return;
    }

    if ( channel == CHANNEL_PNET && 
    ( !ch->in_room->area || (planet = ch->in_room->area->planet) == NULL ) )
    {
	send_to_char( "> system chat only works in systems\n\r", ch );
	return;
    }

    if ( channel == CHANNEL_CLAN )
           clan = ch->pcdata->clan;

    if ( IS_NPC( ch ) && IS_AFFECTED( ch, AFF_CHARM ) )
    {
      if ( ch->master )
	send_to_char( "> illegal operation\n\r", ch->master );
      return;
    }

    if ( argument[0] == '\0' )
    {
	sprintf( buf, "> specify message for %s\n\r", verb );
	buf[0] = UPPER(buf[0]);
	send_to_char( buf, ch );	/* where'd this line go? */
	return;
    }

    if ( !IS_NPC(ch) && IS_SET(ch->act, PLR_SILENCE) )
    {
	ch_printf( ch, "> invalid command for %s.\n\r", verb );
	return;
    }

    REMOVE_BIT(ch->deaf, channel);

    switch ( channel )
    {
    default:
	set_char_color( AT_GOSSIP, ch );
	ch_printf( ch, "(%s) %s: %s\n\r", verb, ch->name, argument );
	sprintf( buf, "(%s) %s: $t", verb, IS_IMMORTAL(ch) ? "$n" : ch->name  );
	break;
    case CHANNEL_CLANTALK:
	set_char_color( AT_CLAN, ch );
	ch_printf( ch, "[ORG] %s: '%s'\n\r", ch->name, argument );
	sprintf( buf, "[ORG] %s: '$t'", IS_IMMORTAL(ch) ? "$n" : ch->name  );
	break;
    case CHANNEL_SHIP:
        set_char_color( AT_SHIP, ch );
	ch_printf( ch, "[DECK] %s: '%s'\n\r", ch->name, argument );
	sprintf( buf, "[DECK] %s: '$t'", IS_IMMORTAL(ch) ? "$n" : ch->name   );
	break;
    case CHANNEL_YELL:
        set_char_color( AT_GOSSIP, ch );
	ch_printf( ch, "You %s, '%s'\n\r", verb, argument );
	sprintf( buf, "%s %ss, '$t'", IS_IMMORTAL(ch) ? "$n" : ch->name ,     verb );
	break;
    case CHANNEL_NEWBIE:
        set_char_color( AT_OOC, ch );
	ch_printf( ch, "[NEWBIE] %s: %s\n\r", ch->name, argument );
	sprintf( buf, "[NEWBIE] %s: $t", IS_IMMORTAL(ch) ? "$n" : ch->name  );
	break;
    case CHANNEL_TRADE:
        set_char_color( AT_OOC, ch );
	ch_printf( ch, "[TRADE] %s: %s\n\r", ch->name, argument );
	sprintf( buf, "[TRADE] %s: $t", IS_IMMORTAL(ch) ? "$n" : ch->name  );
	break;
    case CHANNEL_WCHAT:
    set_char_color( AT_WARTALK, ch );
	ch_printf( ch, "[WAR] %s: %s\n\r", ch->name, argument );
	sprintf( buf, "[WAR] %s: $t", IS_IMMORTAL(ch) ? "$n" : ch->name  );
	break;
    case CHANNEL_HINT:
        set_char_color( AT_CLAN, ch );
	ch_printf( ch, "[HINT] %s: %s\n\r", ch->name, argument );
	sprintf( buf, "[HINT] %s: $t", IS_IMMORTAL(ch) ? "$n" : ch->name  );
	break;
    case CHANNEL_OOC:
        set_char_color( AT_OOC, ch );
        sprintf( buf, "[OOC] %s: $t", IS_IMMORTAL(ch) ? "$n" : ch->name  );
        position        = ch->position;
        ch->position    = POS_STANDING;        
        act( AT_OOC, buf, ch, argument, NULL, TO_CHAR );
	ch->position    = position;
	break;
    case CHANNEL_IMMTALK:
             	sprintf( buf, "%s> $t", IS_IMMORTAL(ch) ? "$n" : ch->name  );
	position	= ch->position;
	ch->position	= POS_STANDING;
        act( AT_IMMORT , buf, ch, argument, NULL, TO_CHAR );
	ch->position	= position;
	break;
    }

    for ( d = first_descriptor; d; d = d->next )
    {
	CHAR_DATA *och;
	CHAR_DATA *vch;

	och = d->original ? d->original : d->character;
	vch = d->character;

	if ( d->connected == CON_PLAYING
	&&   vch != ch
	&&  !IS_SET(och->deaf, channel) )
	{
            char *sbuf = argument;
  	    ch_comlink = FALSE;
    
            if ( channel != CHANNEL_YELL && channel != CHANNEL_IMMTALK && channel != CHANNEL_OOC 
            && channel != CHANNEL_NEWBIE && channel != CHANNEL_TRADE && channel != CHANNEL_WCHAT && channel != CHANNEL_HINT
            && channel != CHANNEL_SHIP && channel != CHANNEL_SYSTEM )
            {
               OBJ_DATA *obj;
      
               if ( IS_IMMORTAL( och ) )
               ch_comlink = TRUE;
               else
               for ( obj = och->last_carrying; obj; obj = obj->prev_content )
               {
                  if (obj->pIndexData->item_type == ITEM_COMLINK)
                  ch_comlink = TRUE;
               }
    
               if ( !ch_comlink )
                 continue;
            }
  	    
	    if ( channel == CHANNEL_IMMTALK && !IS_IMMORTAL(och) )
		continue;

	   
	   if ( channel == CHANNEL_YELL )
	   {
	      if ( ch->in_room != vch->in_room )
	         continue;
	   }

	   if ( channel == CHANNEL_PNET )
	   {
	      if ( !vch->in_room || !vch->in_room->area || ( vch->in_room->area->planet != planet ) )
	         continue;
	   }

	    if ( channel == CHANNEL_CLAN )
	    {
		
		if ( IS_NPC( vch ) )
		  continue;
		
		if ( !vch->pcdata->clan )
	    	  continue;
		
		if ( vch->pcdata->clan != clan )
	    	  continue;
	    }

            if ( channel == CHANNEL_SYSTEM )
            {
                SHIP_DATA *ship = ship_from_cockpit( ch->in_room );
                SHIP_DATA *target;
                
                if ( !ship )
                   continue;
                
                if ( !vch->in_room )
                    continue;
                
                target = ship_from_cockpit( vch->in_room );
                
                if (!target) continue;
                
                if ( channel == CHANNEL_SYSTEM )
                   if (target->starsystem != ship->starsystem )
                      continue;                            
            }

            if ( channel == CHANNEL_SHIP  )
            {
                SHIP_DATA *ship = ship_from_room( ch->in_room );
                SHIP_DATA *target;
                
                if ( !ship )
                   continue;
                
                if ( !vch->in_room )
                    continue;
                
                target = ship_from_room( vch->in_room );
                
                if (!target) continue;
                
                if (target != ship )
                      continue;                            
            }
            
	    position		= vch->position;
	    if ( channel != CHANNEL_YELL )
		vch->position	= POS_STANDING;
	    MOBtrigger = FALSE;
	    if ( channel == CHANNEL_IMMTALK )
	      act( AT_IMMORT , buf, ch, sbuf, vch, TO_VICT );
	    else if (channel == CHANNEL_OOC || channel == CHANNEL_NEWBIE || channel == CHANNEL_TRADE )
              act( AT_OOC, buf, ch, sbuf, vch, TO_VICT );
	    else if ( channel == CHANNEL_SHIP )
	      act( AT_SHIP, buf, ch, sbuf, vch, TO_VICT );
	    else if ( channel == CHANNEL_CLAN || channel == CHANNEL_HINT )
	      act( AT_CLAN, buf, ch, sbuf, vch, TO_VICT );
	    else
	      act( AT_GOSSIP, buf, ch, sbuf, vch, TO_VICT );
	    vch->position	= position;
	}
    }

    return;
}

void to_channel( const char *argument, int channel, const char *verb, sh_int level )
{
    char buf[MAX_STRING_LENGTH];
    DESCRIPTOR_DATA *d;

    if ( !first_descriptor || argument[0] == '\0' )
      return;

    sprintf(buf, "%s: %s\r\n", verb, argument );

    for ( d = first_descriptor; d; d = d->next )
    {
	CHAR_DATA *och;
	CHAR_DATA *vch;

	och = d->original ? d->original : d->character;
	vch = d->character;

	if ( !och || !vch )
	  continue;

	if ( !IS_IMMORTAL(vch)
	&& ( channel == CHANNEL_LOG || channel == CHANNEL_COMM ) )
	  continue;

	if ( d->connected == CON_PLAYING
	&&  !IS_SET(och->deaf, channel)
	&&   get_trust( vch ) >= level )
	{
	  set_char_color( AT_LOG, vch );
	  send_to_char( buf, vch );
	}
    }

    return;
}

void do_gnet( CHAR_DATA *ch, char *argument )
{
      talk_channel( ch, argument, CHANNEL_GNET , "GCHAT" );
      return;
}

void do_pnet( CHAR_DATA *ch, char *argument )
{
      talk_channel( ch, argument, CHANNEL_PNET , "SCHAT" );
	  return;
}

void do_shiptalk( CHAR_DATA *ch, char *argument )
{
     SHIP_DATA *ship;

     if ( (ship = ship_from_cockpit(ch->in_room)) == NULL )  
     {
    	  send_to_char("&R> must be at the console of a deck\n\r",ch);
    	  return;
     }
     talk_channel( ch, argument, CHANNEL_SHIP, "decktalk" );
     return;
}    	        

void do_systemtalk( CHAR_DATA *ch, char *argument )
{
     SHIP_DATA *ship;
     
     if ( (ship = ship_from_cockpit(ch->in_room)) == NULL )  
     {
    	  send_to_char("&R> must be at the console of a deck\n\r",ch);
    	  return;
     }
     talk_channel( ch, argument, CHANNEL_SYSTEM, "Regiontalk" );
     return;
}    	        

void do_ooc( CHAR_DATA *ch, char *argument )
{
    talk_channel( ch, argument, CHANNEL_OOC, "ooc" );
    return;
}

void do_clantalk( CHAR_DATA *ch, char *argument )
{
    if (NOT_AUTHED(ch))
    {
      send_to_char("Huh?\n\r", ch);
      return;
    }

    if ( IS_NPC( ch ) || !ch->pcdata->clan )
    {
	send_to_char( "Huh?\n\r", ch );
	return;
    }
    talk_channel( ch, argument, CHANNEL_CLAN, "orgtalk" );
    return;
}

void do_newbiechat( CHAR_DATA *ch, char *argument )
{
    talk_channel( ch, argument, CHANNEL_NEWBIE, "newbiechat" );
    return;
}

void do_tradechat( CHAR_DATA *ch, char *argument )
{
    talk_channel( ch, argument, CHANNEL_TRADE, "tradechat" );
    return;
}

void do_wchat( CHAR_DATA *ch, char *argument )
{
    talk_channel( ch, argument, CHANNEL_WCHAT, "wchat" );
    return;
}

void do_hintchat( CHAR_DATA *ch, char *argument )
{
    talk_channel( ch, argument, CHANNEL_HINT, "hintchat" );
    return;
}


void do_yell( CHAR_DATA *ch, char *argument )
{
    if (NOT_AUTHED(ch))
    {
      send_to_char("Huh?\n\r", ch);
      return;
    }
  talk_channel( ch, drunk_speech( argument, ch ), CHANNEL_YELL, "yell" );
  return;
}



void do_immtalk( CHAR_DATA *ch, char *argument )
{
    if (NOT_AUTHED(ch))
    {
      send_to_char("Huh?\n\r", ch);
      return;
    }
    talk_channel( ch, argument, CHANNEL_IMMTALK, "aitalk" );
    return;
}


void do_say( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *vch;
    int actflags;

    if ( argument[0] == '\0' )
    {
	send_to_char( "> specify message\n\r", ch );
	return;
    }

    actflags = ch->act;
    if ( IS_NPC( ch ) ) REMOVE_BIT( ch->act, ACT_SECRETIVE );
	for ( vch = ch->in_room->first_person; vch; vch = vch->next_in_room )
	{
		char *sbuf = argument;

		if ( vch == ch )
			continue;
 	      sbuf = drunk_speech( sbuf, ch );

		MOBtrigger = FALSE;
		act( AT_SAY, "[SAY] $n : '$t'", ch, sbuf, vch, TO_VICT );
	}
/*    MOBtrigger = FALSE;
    act( AT_SAY, "$n says '$T'", ch, NULL, argument, TO_ROOM );*/
    ch->act = actflags;
    MOBtrigger = FALSE;
    act( AT_SAY, "[SAY] you: '$T'", ch, NULL, drunk_speech( argument, ch ), TO_CHAR ); 
    mprog_speech_trigger( argument, ch );
    if ( char_died(ch) )
      return;
    oprog_speech_trigger( argument, ch ); 
    if ( char_died(ch) )
      return;
    rprog_speech_trigger( argument, ch ); 
    return;
}



void do_tell( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    int position;
    CHAR_DATA *switched_victim;
    bool ch_comlink;
    bool victim_comlink;
    OBJ_DATA *obj;
    
    switched_victim = NULL;

    if ( IS_SET( ch->deaf, CHANNEL_TELLS ) 
    && !IS_IMMORTAL( ch ) )
    {
      act( AT_PLAIN, "> tells turned off - try chan +tells", ch, NULL, NULL,
		TO_CHAR );
      return;
    }

    if (!IS_NPC(ch)
    && ( IS_SET(ch->act, PLR_SILENCE)
    ||   IS_SET(ch->act, PLR_NO_TELL) ) )
    {
	send_to_char( "> invalid command\n\r", ch );
	return;
    }

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' || argument[0] == '\0' )
    {
	send_to_char( "> specify user and message\n\r", ch );
	return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL 
       || ( IS_NPC(victim) && victim->in_room != ch->in_room ) 
       || (!NOT_AUTHED(ch) && NOT_AUTHED(victim) && !IS_IMMORTAL(ch) ) )
    {
	send_to_char( "> user not found\n\r", ch );
	return;
    }

    if ( ch == victim )
    {
	send_to_char( "> you cannot chat with yourself\n\r", ch );
	return;
    }
    
    if (victim->in_room != ch->in_room && !IS_IMMORTAL(ch) )
    {
      ch_comlink = FALSE;
      victim_comlink = FALSE;
    
      for ( obj = ch->last_carrying; obj; obj = obj->prev_content )
      {
        if (obj->pIndexData->item_type == ITEM_COMLINK)
        ch_comlink = TRUE;
      }
    
      if ( !ch_comlink )
      {
	send_to_char( "> comlink module needed\n\r", ch);
	return;
      }
      
      if ( IS_IMMORTAL ( victim ) )
         victim_comlink = TRUE; 
      
      for ( obj = victim->last_carrying; obj; obj = obj->prev_content )
      {
        if (obj->pIndexData->item_type == ITEM_COMLINK)
        victim_comlink = TRUE;
      }

      if ( !victim_comlink )
      {
	send_to_char( "> comlink module needed\n\r", ch);
	return;
      }
    
    }    
    
    if (NOT_AUTHED(ch) && !NOT_AUTHED(victim) && !IS_IMMORTAL(victim) )
    {
	send_to_char( "> user needs to finish the academy first\n\r", ch);
	return;
    }

    if ( !IS_NPC( victim ) && ( victim->switched ) 
	&& ( IS_IMMORTAL( ch ) ) 
        && !IS_SET(victim->switched->act, ACT_POLYMORPHED)
	&& !IS_AFFECTED(victim->switched, AFF_POSSESS) )
    {
      send_to_char( "> that user is switched\n\r", ch );
      return;
    }

   else if ( !IS_NPC( victim ) && ( victim->switched ) 
        && (IS_SET(victim->switched->act, ACT_POLYMORPHED) 
 	||  IS_AFFECTED(victim->switched, AFF_POSSESS) ) )
     switched_victim = victim->switched;

   else if ( !IS_NPC( victim ) && ( !victim->desc ) )
    {
      send_to_char( "> that user is link-dead\n\r", ch );
      return;
    }

    if ( !IS_NPC (victim) && ( IS_SET (victim->act, PLR_AFK ) ) )
      {
      send_to_char( "> that user is afk\n\r", ch );
      return;
      }

    if ( IS_SET( victim->deaf, CHANNEL_TELLS ) 
    && ( !IS_IMMORTAL( ch ) || ( get_trust( ch ) < get_trust( victim ) ) ) )
    {
      act( AT_PLAIN, "> $E has $S tells turned off", ch, NULL, victim,
		TO_CHAR );
      return;
    }

    if ( !IS_NPC (victim) && ( IS_SET (victim->act, PLR_SILENCE ) ) )
      {
      send_to_char( "> that user is silenced - they will receive your message but cannot respond\n\r", ch );
      }   

    if ( !IS_IMMORTAL(ch) && !IS_AWAKE(victim)  )
    {
    act( AT_PLAIN, "> $E cannot receive your message", ch, 0, victim, TO_CHAR );
	return;
    }

    if ( victim->desc		/* make sure desc exists first  -Thoric */
    &&   victim->desc->connected == CON_EDITING 
    &&   IS_IMMORTAL(ch) )
    {
	act( AT_PLAIN, "> $E is currently in a writing buffer", ch, 0, victim, TO_CHAR );
        return;
    }

 
    if(switched_victim)
      victim = switched_victim;

    sprintf( buf , "[TELL] You <-> %s: '$t'" , victim->name );   
    act( AT_TELL, buf , ch, argument, victim, TO_CHAR );
    position		= victim->position;
    victim->position	= POS_STANDING;
    sprintf( buf , "[TELL] %s%s%s <-> you: '$t'" , MXPTAG ("player $n"), ch->name, MXPTAG ("/player") );
    act( AT_TELL, buf, ch, argument, victim, TO_VICT );
    victim->position	= position;
    victim->reply	= ch;
    mprog_speech_trigger( argument, ch );
    return;
}



void do_reply( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;
    int position;


    REMOVE_BIT( ch->deaf, CHANNEL_TELLS );

    if ( !IS_NPC(ch) && IS_SET(ch->act, PLR_SILENCE) )
    {
	send_to_char( "> your message did not get through\n\r", ch );
	return;
    }

    if ( ( victim = ch->reply ) == NULL )
    {
	send_to_char( "> user not found\n\r", ch );
	return;
    }

    if ( !IS_NPC( victim ) && ( victim->switched )
	&& can_see( ch, victim ) && IS_IMMORTAL( ch )  )
    {
      send_to_char( "> that user is switched\n\r", ch );
      return;
    }
   else if ( !IS_NPC( victim ) && ( !victim->desc ) )
    {
      send_to_char( "> that user is link-dead\n\r", ch );
      return;
    }

    if ( !IS_NPC (victim) && ( IS_SET (victim->act, PLR_AFK ) ) )
      {
      send_to_char( "> that user is afk\n\r", ch );
      return;
      }

    if ( IS_SET( victim->deaf, CHANNEL_TELLS ) 
    && ( !IS_IMMORTAL( ch ) || ( get_trust( ch ) < get_trust( victim ) ) ) )
    {
      act( AT_PLAIN, "$E has $S tells turned off.", ch, NULL, victim,
	TO_CHAR );
      return;
    }

    if (  !IS_IMMORTAL(ch) && !IS_AWAKE(victim) )
    {
    act( AT_PLAIN, "$E can't hear you.", ch, 0, victim, TO_CHAR );
	return;
    }

    sprintf( buf , "[TELL] You <-> %s: '$t'" , victim->name );   
    act( AT_TELL, buf , ch, argument, victim, TO_CHAR );
    position		= victim->position;
    victim->position	= POS_STANDING;
    sprintf( buf , "[TELL] %s <-> you: '$t'" , ch->name );
    act( AT_TELL, buf, ch, argument, victim, TO_VICT );
    victim->position	= position;
    victim->reply	= ch;

    return;
}



void do_emote( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char *plast;
    int actflags;

    if ( !IS_NPC(ch) && IS_SET(ch->act, PLR_NO_EMOTE) )
    {
	send_to_char( "> invalid command\n\r", ch );
	return;
    }

    if ( argument[0] == '\0' )
    {
	send_to_char( "> specify emote text\n\r", ch );
	return;
    }

    actflags = ch->act;
    if ( IS_NPC( ch ) ) REMOVE_BIT( ch->act, ACT_SECRETIVE );
    for ( plast = argument; *plast != '\0'; plast++ )
	;

    strcpy( buf, argument );
    if ( isalpha(plast[-1]) )
	strcat( buf, "." );

    MOBtrigger = FALSE;
    act( AT_ACTION, "$n $T", ch, NULL, buf, TO_ROOM );
    MOBtrigger = FALSE;
    act( AT_ACTION, "$n $T", ch, NULL, buf, TO_CHAR );
    ch->act = actflags;
    return;
}


void do_bug( CHAR_DATA *ch, char *argument )
{
	   if( !argument || argument[0] == '\0' )
   {
      send_to_char( "&w> syntax&B:&C BUG &B<&wmessage&B>\n\r", ch );
      return;
   }
    append_file( ch, TYPO_FILE, argument );
    send_to_char( "> bug submitted\n\r", ch );
    return;
}


void do_typo( CHAR_DATA *ch, char *argument )
{
	   if( !argument || argument[0] == '\0' )
   {
      send_to_char( "&w> syntax&B:&C TYPO|IDEA &B<&wmessage&B>\n\r", ch );
      return;
   }
    append_file( ch, TYPO_FILE, argument );
    send_to_char( "> &Gmessage submitted&w\n\r", ch );
    return;
}



void do_qui( CHAR_DATA *ch, char *argument )
{
    set_char_color( AT_RED, ch );
    send_to_char( "> spell out QUIT\n\r", ch );
    return;
}

void do_quit( CHAR_DATA *ch, char *argument )
{
/*  OBJ_DATA *obj; */ /* Unused */
    int x, y;
    int level;
    DESCRIPTOR_DATA *d;

    char qbuf[MAX_INPUT_LENGTH];
    CHAR_DATA *fch;

    if ( IS_NPC(ch) && IS_SET(ch->act, ACT_POLYMORPHED))
    { 
      send_to_char("> you cannot quit while polymorphed\n\r", ch);
      return;
    }

    if ( IS_NPC(ch) )
	return;

    if ( ch->position == POS_FIGHTING )
    {
	set_char_color( AT_RED, ch );
	send_to_char( "> you are fighting\n\r", ch );
	return;
    }

    if ( ch->position  < POS_STUNNED  )
    {
	set_char_color( AT_BLOOD, ch );
	send_to_char( "> you are not DEAD yet\n\r", ch );
	return;
    }

    if ( auction->item != NULL && ((ch == auction->buyer) || (ch == auction->seller) ) )
    {
	send_to_char("> wait until you have bought/sold the item on auction\n\r", ch);
	return;
    }

/*    
    if ( !IS_IMMORTAL(ch) && ch->in_room && !IS_SET( ch->in_room->room_flags , ROOM_HOTEL ) && !NOT_AUTHED(ch) )
    {
	send_to_char("> you may not quit here\n\r", ch);
	send_to_char("> you will have to find a safe node such as an io port\n\r", ch);
	send_to_char("> use EMERGENCY (20c fee) to be forwarded to the nearest io port\n\r", ch);
	return;
    }
*/
    
    set_char_color( AT_WHITE, ch );
    send_to_char( "> cyberspace begins to fade as you are pushed through a vortex of colors\n\rYou find yourself back in the meat of your body\n\r\n\r", ch );
    act( AT_SAY, "> come back soon, $n...'", ch, NULL, NULL, TO_CHAR );
    act( AT_BYE, "> $n has left cyberspace", ch, NULL, NULL, TO_ROOM );
    set_char_color( AT_GREY, ch);

    if ( ch->challenged )
    {
      sprintf(qbuf,"> %s has quit! challenge is void!",ch->name);
      ch->challenged=NULL;
      sportschan(qbuf);
    }

    for ( fch = first_char; fch; fch = fch->next )
    {
	if(IS_NPC(fch))
	  continue;

	if(fch->challenged && fch->challenged == ch)
	  fch->challenged=NULL;
    }

    //char bufText[MAX_STRING_LENGTH];
    //sprintf( bufText , "> %s has quit" , ch->name );
    //echo_to_all( AT_LBLUE , bufText , ECHOTAR_ALL );

    sprintf( log_buf, "> %s has quit", ch->name );

	if ( ch->desc )
    ch->pcdata->logouttime = time(NULL);

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
    			if ( str_cmp(temp->name, "Wintermute") )
    			{
		 set_char_color(AT_NOTIFY,vch);
		 ch_printf(vch,"> %s has left cyberspace\n\r",temp->name);
    			}
		 break;
		}
	    }
	}
    }

    quitting_char = ch;
    save_char_obj( ch );
    save_home(ch);
    saving_char = NULL;

    level = get_trust(ch);
    /*
     * After extract_char the ch is no longer valid!
     */
    extract_char( ch, TRUE );
    for ( x = 0; x < MAX_WEAR; x++ )
	for ( y = 0; y < MAX_LAYERS; y++ )
	    save_equipment[x][y] = NULL;

    /* don't show who's logging off to leaving player */
    log_string_plus( log_buf, LOG_COMM );
    return;
}


void send_ansi_title( CHAR_DATA *ch )
{
    FILE *rpfile;
    int num=0;
    char BUFF[MAX_STRING_LENGTH*2];

    if ((rpfile = fopen(ANSITITLE_FILE,"r")) !=NULL) {
      while ((BUFF[num]=fgetc(rpfile)) != EOF)
	 num++;
      fclose(rpfile);
      BUFF[num] = 0;
      write_to_buffer(ch->desc,BUFF,num);
    }
}

void send_ascii_title( CHAR_DATA *ch )
{
    FILE *rpfile;
    int num=0;
    char BUFF[MAX_STRING_LENGTH];

    if ((rpfile = fopen(ASCTITLE_FILE,"r")) !=NULL) {
      while ((BUFF[num]=fgetc(rpfile)) != EOF)
	 num++;
      fclose(rpfile);
      BUFF[num] = 0;
      write_to_buffer(ch->desc,BUFF,num);
    }
}


void do_ansi( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "> ANSI ON or OFF\n\r", ch );
	return;
    }
    if ( (strcmp(arg,"on")==0) || (strcmp(arg,"ON") == 0) ) {
	SET_BIT(ch->act,PLR_ANSI);
	set_char_color( AT_WHITE + AT_BLINK, ch);
	send_to_char( "> ANSI ON!!!\n\r", ch);
	return;
    }

    if ( (strcmp(arg,"off")==0) || (strcmp(arg,"OFF") == 0) ) {
	REMOVE_BIT(ch->act,PLR_ANSI);
	send_to_char( "> ANSI support is now off\n\r", ch );
	return;
    }
}

void do_save( CHAR_DATA *ch, char *argument )
{
    if ( IS_NPC(ch) && IS_SET(ch->act, ACT_POLYMORPHED))
    { 
      send_to_char("> unable to save while polymorphed.\n\r", ch);
      return;
    }

    if ( IS_NPC(ch) )
	return;

    if ( NOT_AUTHED(ch) )
    { 
      send_to_char("> you must complete the academy first\n\r", ch);
      return;
    }

    save_char_obj( ch );
    save_home (ch );
    saving_char = NULL;
    send_to_char( "> ok\n\r", ch );
    return;
}


/*
 * Something from original DikuMUD that Merc yanked out.
 * Used to prevent following loops, which can cause problems if people
 * follow in a loop through an exit leading back into the same room
 * (Which exists in many maze areas)			-Thoric
 */
bool circle_follow( CHAR_DATA *ch, CHAR_DATA *victim )
{
    CHAR_DATA *tmp;

    for ( tmp = victim; tmp; tmp = tmp->master )
	if ( tmp == ch )
	  return TRUE;
    return FALSE;
}


void do_follow( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "> specify follow target\n\r", ch );
	return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
	send_to_char( "> user not found\n\r", ch );
	return;
    }

    if ( IS_AFFECTED(ch, AFF_CHARM) && ch->master )
    {
	act( AT_PLAIN, "> need to follow $N", ch, NULL, ch->master, TO_CHAR );
	return;
    }

    if ( victim == ch )
    {
	if ( !ch->master )
	{
	    send_to_char( "> already following self\n\r", ch );
	    return;
	}
	stop_follower( ch );
	return;
    }

    if ( circle_follow( ch, victim ) )
    {
	send_to_char( "> cannot follow in loops\n\r", ch );
	return;
    }

    if ( ch->master )
	stop_follower( ch );

    add_follower( ch, victim );
    return;
}



void add_follower( CHAR_DATA *ch, CHAR_DATA *master )
{
    if ( ch->master )
    {
	bug( "Add_follower: non-null master.", 0 );
	return;
    }

    ch->master        = master;
    ch->leader        = NULL;

    if ( can_see( master, ch ) )
    act( AT_ACTION, "> $n now follows you", ch, NULL, master, TO_VICT );

    act( AT_ACTION, "> you now follow $N",  ch, NULL, master, TO_CHAR );

    return;
}



void stop_follower( CHAR_DATA *ch )
{
    if ( !ch->master )
    {
	bug( "Stop_follower: null master.", 0 );
	return;
    }

    if ( IS_AFFECTED(ch, AFF_CHARM) )
    {
	REMOVE_BIT( ch->affected_by, AFF_CHARM );
	affect_strip( ch, gsn_charm_person );
    }

    if ( can_see( ch->master, ch ) )
    act( AT_ACTION, "> $n stops following you",     ch, NULL, ch->master, TO_VICT    );
    act( AT_ACTION, "> you stop following $N",      ch, NULL, ch->master, TO_CHAR    );

    ch->master = NULL;
    ch->leader = NULL;
    return;
}



void die_follower( CHAR_DATA *ch )
{
    CHAR_DATA *fch;

    if ( ch->master )
	stop_follower( ch );

    ch->leader = NULL;

    for ( fch = first_char; fch; fch = fch->next )
    {
	if ( fch->master == ch )
	    stop_follower( fch );
	if ( fch->leader == ch )
	    fch->leader = fch;
    }
    return;
}



void do_order( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char argbuf[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    CHAR_DATA *och;
    CHAR_DATA *och_next;
    bool found;
    bool fAll;
    int toomany = 0;
    
    strcpy( argbuf, argument );
    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' || argument[0] == '\0' )
    {
	send_to_char( "> specify target and order\n\r", ch );
	return;
    }

    if ( IS_AFFECTED( ch, AFF_CHARM ) )
    {
	send_to_char( "> you cannot give orders\n\r", ch );
	return;
    }

    if ( !str_cmp( arg, "all" ) )
    {
	fAll   = TRUE;
	victim = NULL;
    }
    else
    {
	fAll   = FALSE;
	if ( ( victim = get_char_room( ch, arg ) ) == NULL )
	{
	    send_to_char( "> user not found\n\r", ch );
	    return;
	}

	if ( victim == ch )
	{
	    send_to_char( "> following commands\n\r", ch );
	    return;
	}

	if ( !IS_AFFECTED(victim, AFF_CHARM) || victim->master != ch )
	{
	    send_to_char( "> illegal command\n\r", ch );
	    return;
	}
    }

    found = FALSE;
    for ( och = ch->in_room->first_person; och; och = och_next )
    {
	och_next = och->next_in_room;

	if ( IS_AFFECTED(och, AFF_CHARM)
	&&   och->master == ch
	&& ( fAll || och == victim ) )
	{
	    found = TRUE;
	    act( AT_ACTION, "> $n orders you to '$t'", ch, argument, och, TO_VICT );
	    interpret( och, argument );
	}
	
	if ( toomany++ > 100 )
	    break;
    }

    if ( found )
    {
        sprintf( log_buf, "> %s: order %s", ch->name, argbuf );
        log_string_plus( log_buf, LOG_NORMAL );
 	send_to_char( "> ok\n\r", ch );
        WAIT_STATE( ch, 12 );
    }
    else
	send_to_char( "> no followers here\n\r", ch );
    return;
}

/*
char *itoa(int foo)
{
  static char bar[256];

  sprintf(bar,"%d",foo);
  return(bar);

}
*/

void do_group( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	CHAR_DATA *gch;
	CHAR_DATA *leader;

	leader = ch->leader ? ch->leader : ch;
	set_char_color( AT_GREEN, ch );
	ch_printf( ch, "[GROUP] %s:\n\r", PERS(leader, ch) );

	for ( gch = first_char; gch; gch = gch->next )
	{
	    if ( is_same_group( gch, ch ) )
	    {
		set_char_color( AT_DGREEN, ch );
		  ch_printf( ch,
		    "%s\n\r", PERS(gch, ch) );
	    }
	}
	return;
    }

    if ( !strcmp( arg, "disband" ))
    {
	CHAR_DATA *gch;
	int count = 0;

	if ( ch->leader || ch->master )
	{
	    send_to_char( "> cannot disband a group if you are following someone\n\r", ch );
	    return;
	}
	
	for ( gch = first_char; gch; gch = gch->next )
	{
	    if ( is_same_group( ch, gch )
	    && ( ch != gch ) )
	    {
		gch->leader = NULL;
		gch->master = NULL;
		count++;
		send_to_char( "> group disbanded\n\r", gch );
	    }
	}

	if ( count == 0 )
	   send_to_char( "> no group members to disband\n\r", ch );
	else
	   send_to_char( "> group disbanded\n\r", ch );
	
    return;
    }

    if ( !strcmp( arg, "all" ) )
    {
	CHAR_DATA *rch;
	int count = 0;

        for ( rch = ch->in_room->first_person; rch; rch = rch->next_in_room )
	{
           if ( ch != rch
           &&   !IS_NPC( rch )
	   &&   rch->master == ch
	   &&   !ch->master
	   &&   !ch->leader
    	   &&   !is_same_group( rch, ch )
	      )
	   {
		rch->leader = ch;
		count++;
	   }
	}
	
	if ( count == 0 )
	  send_to_char( "> no eligible group members\n\r", ch );
	else
	{
        act( AT_ACTION, "> $n groups $s followers", ch, NULL, victim, TO_ROOM );
	   send_to_char( "> grouping followers\n\r", ch );
	}
    return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
	send_to_char( "> user not found\n\r", ch );
	return;
    }

    if ( ch->master || ( ch->leader && ch->leader != ch ) )
    {
	send_to_char( "> already following someone else\n\r", ch );
	return;
    }

    if ( victim->master != ch && ch != victim )
    {
    act( AT_PLAIN, "> $N is not following you", ch, NULL, victim, TO_CHAR );
	return;
    }

    if ( is_same_group( victim, ch ) && ch != victim )
    {
	victim->leader = NULL;
    act( AT_ACTION, "> $n removes $N from $s group",   ch, NULL, victim, TO_NOTVICT );
    act( AT_ACTION, "> $n removes you from $s group",  ch, NULL, victim, TO_VICT    );
    act( AT_ACTION, "> $N removed from group", ch, NULL, victim, TO_CHAR    );
	return;
    }

    victim->leader = ch;
    act( AT_ACTION, "> $N joins $n's group", ch, NULL, victim, TO_NOTVICT );
    act( AT_ACTION, "> you join $n's group", ch, NULL, victim, TO_VICT    );
    act( AT_ACTION, "> $N joins your group", ch, NULL, victim, TO_CHAR    );
    return;
}



/*
 * 'Split' originally by Gnort, God of Chaos.
 */
void do_split( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *gch;
    int members;
    int amount;
    int share;
    int extra;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "> define amount\n\r", ch );
	return;
    }

    amount = atoi( arg );

    if ( amount < 0 )
    {
	send_to_char( "> invalid amount\n\r", ch );
	return;
    }

    if ( amount == 0 )
    {
	send_to_char( "> zero amount detected\n\r", ch );
	return;
    }

    if ( ch->gold < amount )
    {
	send_to_char( "> not enough credits\n\r", ch );
	return;
    }

    members = 0;
    for ( gch = ch->in_room->first_person; gch; gch = gch->next_in_room )
    {
	if ( is_same_group( gch, ch ) )
	    members++;
    }

    
    if (( IS_SET(ch->act, PLR_AUTOGOLD)) && (members < 2))
    return;

    if ( members < 2 )
    {
	send_to_char( "> all kept\n\r", ch );
	return;
    }

    share = amount / members;
    extra = amount % members;

    if ( share == 0 )
    {
	send_to_char( "> illegal input\n\r", ch );
	return;
    }

    ch->gold -= amount;
    ch->gold += share + extra;

    set_char_color( AT_GOLD, ch );
    ch_printf( ch,
	"> you split %d credits - your share: %d\n\r",
	amount, share + extra );

    sprintf( buf, "> $n splits %d credits - your share: %d",
	amount, share );

    for ( gch = ch->in_room->first_person; gch; gch = gch->next_in_room )
    {
	if ( gch != ch && is_same_group( gch, ch ) )
	{
	    act( AT_GOLD, buf, ch, NULL, gch, TO_VICT );
	    gch->gold += share;
	}
    }
    return;
}



void do_gtell( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *gch;

    if ( argument[0] == '\0' )
    {
	send_to_char( "> specify message\n\r", ch );
	return;
    }

    if ( IS_SET( ch->act, PLR_NO_TELL ) )
    {
	send_to_char( "> message did not get through\n\r", ch );
	return;
    }

    /*
     * Note use of send_to_char, so gtell works on sleepers.
     */
/*    sprintf( buf, "%s tells the group '%s'.\n\r", ch->name, argument );*/
    for ( gch = first_char; gch; gch = gch->next )
    {
	if ( is_same_group( gch, ch ) )
	{
	    set_char_color( AT_GTELL, gch );
	    ch_printf( gch, "[GROUP] %s: '%s'.\n\r", ch->name, argument );
	}
    }

    return;
}


/*
 * It is very important that this be an equivalence relation:
 * (1) A ~ A
 * (2) if A ~ B then B ~ A
 * (3) if A ~ B  and B ~ C, then A ~ C
 */
bool is_same_group( CHAR_DATA *ach, CHAR_DATA *bch )
{
    if ( ach->leader ) ach = ach->leader;
    if ( bch->leader ) bch = bch->leader;
    return ach == bch;
}

/*
 * this function sends raw argument over the AUCTION: channel
 * I am not too sure if this method is right..
 */

void talk_auction (char *argument)
{
    DESCRIPTOR_DATA *d;
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *original;

    sprintf (buf,"> auction: %s", argument); /* last %s to reset color */

    for (d = first_descriptor; d; d = d->next)
    {
        original = d->original ? d->original : d->character; /* if switched */
        if ((d->connected == CON_PLAYING) && !IS_SET(original->deaf,CHANNEL_AUCTION) 
        && !NOT_AUTHED(original))
            act( AT_GOSSIP, buf, original, NULL, NULL, TO_CHAR );
    }
}

void    channel_noise ()
{}
 // done for neuro
