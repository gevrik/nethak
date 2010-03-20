#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "mud.h"

/*
 *  Locals
 */
char *tiny_affect_loc_name(int location);

extern char * const cargo_names[CARGO_MAX];

void do_gold(CHAR_DATA * ch, char *argument)
{
   set_char_color( AT_GOLD, ch );
   ch_printf( ch,  "> credits: %-15d bank: %ld\n\r", ch->gold, ch->pcdata->bank );
   return;
}

void do_resources(CHAR_DATA * ch, char *argument)
{
   set_char_color( AT_GOLD, ch );
   ch_printf( ch, "&Wentertainment   :  &G%-15d &Wmultimedia      :  &G%d\n\r" , ch->pcdata->rentertain, ch->pcdata->rmultimedia  );
   ch_printf( ch, "&Wfinance         :  &G%-15d &Wproductivity    :  &G%d\n\r" , ch->pcdata->rfinance, ch->pcdata->rproduct );
   return;
}

void do_score(CHAR_DATA * ch, char *argument)
{
    if ( IS_NPC(ch) || !ch->pcdata )
       do_oldscore(ch, "");
    
    if ( IS_AFFECTED(ch, AFF_POSSESS) )
    {   
       send_to_char("> you cannot do that in your current state of mind\n\r", ch);
       return;
    }

    ch_printf( ch, "&G%s\n\r", ch->pcdata->title );
    	ch_printf( ch, "&W-----------------------------------------------------\n\r" );

	ch_printf( ch, "&Wname: &G%s   &Wuptime: &G%d\n\r", ch->name, get_age(ch) );
	ch_printf( ch, "&W--stats----------------------------------------------\n\r" );

	ch_printf( ch, "&Wstrength:  &G%d", get_curr_str( ch ) );
	ch_printf( ch, "   &Wconstitution: &G%d", get_curr_con( ch ) );
	ch_printf( ch, "   &Wwisdom:   &G%d\n\r", get_curr_wis( ch ) );

	ch_printf( ch, "&Wdexterity: &G%d", get_curr_dex( ch ) );
	ch_printf( ch, "   &Wintelligence: &G%d", get_curr_int( ch ) );
	ch_printf( ch, "   &Wcharisma: &G%d\n\r", get_curr_cha( ch ) );
    	ch_printf( ch, "&W--info-----------------------------------------------\n\r" );
	ch_printf( ch, "&Whealth:  &G%d", ch->hit );
	ch_printf( ch, "   &Wenergy: &G%d", ch->move );
	ch_printf( ch, "   &Wwillpower: &G%d\n\r", ch->mental_state );
	ch_printf( ch, "&Warmor dam mod: &G%d\n\r", ch->armor / 10 );
	ch_printf( ch, "&W--resources------------------------------------------\n\r" );
    	ch_printf( ch, "&Wsnippets :  &G%-12d &Wentertainment   :  &G%d\n\r" , ch->snippets, ch->pcdata->rentertain );
    	ch_printf( ch, "&Wcredits  :  &G%-12d &Wmultimedia      :  &G%d\n\r" , ch->gold, ch->pcdata->rmultimedia );
    	ch_printf( ch, "&Wbank     :  &G%-12ld &Wfinance         :  &G%d\n\r" , ch->pcdata->bank, ch->pcdata->rfinance );
    	ch_printf( ch, "&Wnodes    :  &G%-12d &Wproductivity    :  &G%d\n\r" , ch->pcdata->qtaxnodes, ch->pcdata->rproduct );
    	ch_printf( ch, "&W--storage memory-------------------------------------\n\r" );
    	ch_printf( ch, "slots: (%d/%d)  size: (%d/%d)\n\r",
    		ch->carry_number, can_carry_n(ch), ch->carry_weight, can_carry_w(ch) );
    	send_to_char( "&W--arena----------------------------------------------\n\r", ch);
    	ch_printf(ch, "arena:   wins: %d   losses: %d\n\r",
           ch->arenawin, ch->arenaloss );

    	ch_printf( ch, "&W-----------------------------------------------------\n\r" );
    
    if ( !IS_NPC( ch ) && IS_IMMORTAL( ch ) )
    {
	ch_printf( ch, "&WWizInvis level: &G%d   &WWizInvis is &G%s\n\r",
			ch->pcdata->wizinvis,
			IS_SET( ch->act, PLR_WIZINVIS ) ? "ON" : "OFF" );
    }

    switch ( ch->position )
    {
    case POS_DEAD:
	send_to_char( "&Gyou are DEAD   ",		ch );
	break;
    case POS_MORTAL:
	send_to_char( "&Gyou are mortally wounded   ",	ch );
	break;
    case POS_INCAP:
	send_to_char( "&Gyou are incapacitated   ",	ch );
	break;
    case POS_STUNNED:
	send_to_char( "&Gyou are stunned   ",		ch );
	break;
    case POS_SLEEPING:
	send_to_char( "&Gyou are sleeping   ",		ch );
	break;
    case POS_RESTING:
	send_to_char( "&Gyou are resting   ",		ch );
	break;
    case POS_STANDING:
	send_to_char( "&Gyou are standing   ",		ch );
	break;
    case POS_FIGHTING:
	send_to_char( "&Gyou are fighting   ",		ch );
	break;
    case POS_MOUNTED:
	send_to_char( "&Gyou are mounted   ",			ch );
	break;
    case POS_SHOVE:
	send_to_char( "&Gyou are being shoved   ",		ch );
	break;
    case POS_DRAG:
	send_to_char( "&Gyou are being dragged   ",		ch );
	break;
    default:
	break;
    }


    if (!IS_NPC(ch) && ch->pcdata->condition[COND_THIRST] == 0 && !IS_SET(ch->pcdata->cyber, CYBER_REACTOR))
	send_to_char( "&Gyou are thirsty   ", ch );
    if (!IS_NPC(ch) && ch->pcdata->condition[COND_FULL] == 0 && !IS_SET(ch->pcdata->cyber, CYBER_REACTOR))
	send_to_char( "&Gyou are hungry   ",  ch );
    
    send_to_char( "\n\r",		ch );


    return;

}

/*
 * Return ascii name of an affect location.
 */
char           *
tiny_affect_loc_name(int location)
{
	switch (location) {
	case APPLY_NONE:		return "NIL";
	case APPLY_STR:			return " STR  ";
	case APPLY_DEX:			return " DEX  ";
	case APPLY_INT:			return " INT  ";
	case APPLY_WIS:			return " WIS  ";
	case APPLY_CON:			return " CON  ";
	case APPLY_CHA:			return " CHA  ";
	case APPLY_LCK:			return " LCK  ";
	case APPLY_SEX:			return " SEX  ";
	case APPLY_LEVEL:		return " LVL  ";
	case APPLY_AGE:			return " AGE  ";
	case APPLY_MANA:		return " MANA ";
	case APPLY_HIT:			return " HV   ";
	case APPLY_MOVE:		return " MOVE ";
	case APPLY_GOLD:		return " GOLD ";
	case APPLY_EXP:			return " EXP  ";
	case APPLY_AC:			return " AC   ";
	case APPLY_HITROLL:		return " HITRL";
	case APPLY_DAMROLL:		return " DAMRL";
	case APPLY_SAVING_POISON:	return "SV POI";
	case APPLY_SAVING_ROD:		return "SV ROD";
	case APPLY_SAVING_PARA:		return "SV PARA";
	case APPLY_SAVING_BREATH:	return "SV BRTH";
	case APPLY_SAVING_SPELL:	return "SV SPLL";
	case APPLY_HEIGHT:		return "HEIGHT";
	case APPLY_WEIGHT:		return "WEIGHT";
	case APPLY_AFFECT:		return "AFF BY";
	case APPLY_RESISTANT:		return "RESIST";
	case APPLY_IMMUNE:		return "IMMUNE";
	case APPLY_SUSCEPTIBLE:		return "SUSCEPT";
	case APPLY_WEAPONSPELL:		return " WEAPON";
	case APPLY_BACKSTAB:		return "BACKSTB";
	case APPLY_PICK:		return " PICK  ";
	case APPLY_TRACK:		return " TRACE ";
	case APPLY_STEAL:		return " STEAL ";
	case APPLY_SNEAK:		return " SNEAK ";
	case APPLY_HIDE:		return " HIDE  ";
	case APPLY_PALM:		return " PALM  ";
	case APPLY_DETRAP:		return " DETRAP";
	case APPLY_DODGE:		return " DODGE ";
	case APPLY_PEEK:		return " PEEK  ";
	case APPLY_SCAN:		return " SCAN  ";
	case APPLY_GOUGE:		return " GOUGE ";
	case APPLY_SEARCH:		return " SEARCH";
	case APPLY_MOUNT:		return " MOUNT ";
	case APPLY_DISARM:		return " DISARM";
	case APPLY_KICK:		return " KICK  ";
	case APPLY_PARRY:		return " PARRY ";
	case APPLY_BASH:		return " BASH  ";
	case APPLY_STUN:		return " STUN  ";
	case APPLY_PUNCH:		return " PUNCH ";
	case APPLY_CLIMB:		return " CLIMB ";
	case APPLY_GRIP:		return " GRIP  ";
	case APPLY_SCRIBE:		return " SCRIBE";
	case APPLY_BREW:		return " BREW  ";
	case APPLY_WEARSPELL:		return " WEAR  ";
	case APPLY_REMOVESPELL:		return " REMOVE";
	case APPLY_EMOTION:		return "EMOTION";
	case APPLY_MENTALSTATE:		return " MENTAL";
	case APPLY_STRIPSN:		return " DISPEL";
	case APPLY_REMOVE:		return " REMOVE";
	case APPLY_DIG:			return " DIG   ";
	case APPLY_FULL:		return " HUNGER";
	case APPLY_THIRST:		return " THIRST";
	case APPLY_DRUNK:		return " DRUNK ";
	case APPLY_BLOOD:		return " BLOOD ";
	}

	bug("Affect_location_name: unknown location %d.", location);
	return "(?)";
}


void do_oldscore( CHAR_DATA *ch, char *argument )
{
    if ( IS_AFFECTED(ch, AFF_POSSESS) )
    {   
       send_to_char("> you cannot do that in your current state of mind\n\r", ch);
       return;
    }

    set_char_color( AT_SCORE, ch );
    ch_printf( ch,
	"> you are %s, the %d year old\n\r",
	IS_NPC(ch) ? ch->name : ch->pcdata->title,
	get_age(ch));
    
    send_to_char( "> you are ", ch );
	 if ( ch->hit >= 100 ) send_to_char( "healthy.", ch );
    else if ( ch->hit > 90 ) send_to_char( "slightly wounded.", ch );
    else if ( ch->hit > 75 ) send_to_char( "hurt.",    ch );
    else if ( ch->hit > 50 ) send_to_char( "in pain.",    ch );
    else if ( ch->hit > 35   ) send_to_char( "severely wounded.", ch );
    else if ( ch->hit > 20   ) send_to_char( "writhing in pain.",    ch );
    else if ( ch->hit > 0    ) send_to_char( "barely conscious.",    ch );
    else if ( ch->hit > -300 ) send_to_char( "unconscious.", ch );
    else                       send_to_char( "nearly DEAD.", ch );


    send_to_char( "   you are ", ch );
	 if ( ch->move > 500 ) send_to_char( "energetic\n\r", ch );
    else if ( ch->move > 100 ) send_to_char( "rested\n\r", ch );
    else if ( ch->move > 50 ) send_to_char( "worn out\n\r",    ch );
    else if ( ch->move > 0 ) send_to_char( "exhausted\n\r", ch );
    else                        send_to_char( "to tired to move\n\r", ch );


    if ( !IS_NPC(ch) && ch->pcdata->condition[COND_DRUNK]   > 10 )
	send_to_char( "> you are drunk\n\r",   ch );
    if ( !IS_NPC(ch) && ch->pcdata->condition[COND_THIRST] ==  0 )
	send_to_char( "> you are thirsty\n\r", ch );
    if ( !IS_NPC(ch) && ch->pcdata->condition[COND_FULL]   ==  0 )
	send_to_char( "> you are hungry\n\r",  ch );

    switch( ch->mental_state / 10 )
    {
        default:   send_to_char( "> you're completely messed up!", ch ); break;
        case -10:  send_to_char( "> you're barely conscious.", ch ); break;
        case  -9:  send_to_char( "> you can barely keep your eyes open.", ch ); break;
        case  -8:  send_to_char( "> you're extremely drowsy.", ch ); break;
        case  -7:  send_to_char( "> you feel very unmotivated.", ch ); break;
        case  -6:  send_to_char( "> you feel sedated.", ch ); break;
        case  -5:  send_to_char( "> you feel sleepy.", ch ); break;
        case  -4:  send_to_char( "> you feel tired.", ch ); break;
        case  -3:  send_to_char( "> you could use a rest.", ch ); break;
        case  -2:  send_to_char( "> you feel a little under the weather.", ch ); break;
        case  -1:  send_to_char( "> you feel fine.", ch ); break;
        case   0:  send_to_char( "> you feel great.", ch ); break;
        case   1:  send_to_char( "> you feel energetic.", ch ); break;
        case   2:  send_to_char( "> your mind is racing.", ch ); break;
        case   3:  send_to_char( "> you cannot think straight.", ch ); break;
        case   4:  send_to_char( "> your mind is going 100 miles an hour.", ch ); break;
        case   5:  send_to_char( "> you're high as a kite.", ch ); break;
        case   6:  send_to_char( "> your mind and body are slipping appart.", ch ); break;
        case   7:  send_to_char( "Reality is slipping away.", ch ); break;
        case   8:  send_to_char( "> you have no idea what is real, and what is not.", ch ); break;
        case   9:  send_to_char( "> you feel immortal.", ch ); break;
        case  10:  send_to_char( "> you are a Supreme Entity.", ch ); break;
    }

    switch ( ch->position )
    {
    case POS_DEAD:
	send_to_char( "   you are DEAD!\n\r",		ch );
	break;
    case POS_MORTAL:
	send_to_char( "   you are mortally wounded\n\r",	ch );
	break;
    case POS_INCAP:
	send_to_char( "   you are incapacitated\n\r",	ch );
	break;
    case POS_STUNNED:
	send_to_char( "   you are stunned\n\r",		ch );
	break;
    case POS_SLEEPING:
	send_to_char( "   you are sleeping\n\r",		ch );
	break;
    case POS_RESTING:
	send_to_char( "   you are resting\n\r",		ch );
	break;
    case POS_STANDING:
	send_to_char( "   you are standing\n\r",		ch );
	break;
    case POS_FIGHTING:
	send_to_char( "   you are fighting\n\r",		ch );
	break;
    case POS_MOUNTED:
	send_to_char( "   Mounted\n\r",			ch );
	break;
    case POS_SHOVE:
	send_to_char( "   Being shoved\n\r",		ch );
	break;
    case POS_DRAG:
	send_to_char( "   Being dragged\n\r",		ch );
	break;
    default:
    	send_to_char( "\n\r",		ch );
	break;
    }

    send_to_char( "> you are ", ch );
	 if ( GET_AC(ch) >=  101 ) send_to_char( "WORSE than naked!", ch );
    else if ( GET_AC(ch) >=   80 ) send_to_char( "naked.",            ch );
    else if ( GET_AC(ch) >=   60 ) send_to_char( "wearing clothes.",  ch );
    else if ( GET_AC(ch) >=   40 ) send_to_char( "slightly armored.", ch );
    else if ( GET_AC(ch) >=   20 ) send_to_char( "somewhat armored.", ch );
    else if ( GET_AC(ch) >=    0 ) send_to_char( "armored.",          ch );
    else if ( GET_AC(ch) >= - 20 ) send_to_char( "well armored.",     ch );
    else if ( GET_AC(ch) >= - 40 ) send_to_char( "strongly armored.", ch );
    else if ( GET_AC(ch) >= - 60 ) send_to_char( "heavily armored.",  ch );
    else if ( GET_AC(ch) >= - 80 ) send_to_char( "superbly armored.", ch );
    else if ( GET_AC(ch) >= -100 ) send_to_char( "divinely armored.", ch );
    else                           send_to_char( "invincible!",       ch );

    send_to_char( "   you are ", ch );
	 if ( ch->alignment >  900 ) send_to_char( "angelic\n\r", ch );
    else if ( ch->alignment >  700 ) send_to_char( "saintly\n\r", ch );
    else if ( ch->alignment >  350 ) send_to_char( "good\n\r",    ch );
    else if ( ch->alignment >  100 ) send_to_char( "kind\n\r",    ch );
    else if ( ch->alignment > -100 ) send_to_char( "neutral\n\r", ch );
    else if ( ch->alignment > -350 ) send_to_char( "mean\n\r",    ch );
    else if ( ch->alignment > -700 ) send_to_char( "evil\n\r",    ch );
    else if ( ch->alignment > -900 ) send_to_char( "demonic\n\r", ch );
    else                             send_to_char( "satanic\n\r", ch );

    ch_printf( ch,
	"> you have have %d credits\n\r" , ch->gold );

    ch_printf( ch,
	"Autoexit: %s   Autoloot: %s   Autosac: %s   Autocred: %s\n\r",
	(!IS_NPC(ch) && IS_SET(ch->act, PLR_AUTOEXIT)) ? "yes" : "no",
	(!IS_NPC(ch) && IS_SET(ch->act, PLR_AUTOLOOT)) ? "yes" : "no",
	(!IS_NPC(ch) && IS_SET(ch->act, PLR_AUTOSAC) ) ? "yes" : "no",
  	(!IS_NPC(ch) && IS_SET(ch->act, PLR_AUTOGOLD)) ? "yes" : "no" );

    ch_printf( ch, "Wimpy set to %d percent\n\r", ch->wimpy );
    
    if ( !IS_NPC(ch) && ch->pcdata )
    {
    	int sn;
    	
        send_to_char( "> you are skilled at: " , ch );
    	for ( sn = 0; sn < top_sn ; sn++ )
    	   if ( ch->pcdata->learned[sn] > 0 && ch->pcdata->learned[sn] < 100 )
               ch_printf( ch,  "%s  ", skill_table[sn]->name );    	     	 
        send_to_char( "\n\r" , ch );
    
        send_to_char( "> you are an adept of: " , ch );
    	for ( sn = 0; sn < top_sn ; sn++ )
    	   if ( ch->pcdata->learned[sn] >= 100 )
               ch_printf( ch,  "%s  ", skill_table[sn]->name );    	     	 
        send_to_char( "\n\r" , ch );
    
    }           
    
    if ( !IS_NPC( ch ) && IS_IMMORTAL( ch ) )
    {
	ch_printf( ch, "WizInvis level: %d   WizInvis is %s\n\r",
			ch->pcdata->wizinvis,
			IS_SET( ch->act, PLR_WIZINVIS ) ? "ON" : "OFF" );
    }

    return;
}

/*								-Thoric
 * Display your current exp, level, and surrounding level exp requirements
 */
void do_level( CHAR_DATA *ch, char *argument )
{ 
}


void do_affected ( CHAR_DATA *ch, char *argument )
{
    char arg [MAX_INPUT_LENGTH];
    AFFECT_DATA *paf;
    SKILLTYPE *skill;
 
    if ( IS_NPC(ch) )
        return;

    argument = one_argument( argument, arg );

    if ( !str_cmp( arg, "by" ) )
    {
        set_char_color( AT_BLUE, ch );
        send_to_char( "\n\r> imbued with:\n\r", ch );
	set_char_color( AT_SCORE, ch );
	ch_printf( ch, "%s\n\r", affect_bit_name( ch->affected_by ) );
	return;      
    }

    if ( !ch->first_affect )
    {
        set_char_color( AT_SCORE, ch );
        send_to_char( "\n\r> no skill affects you\n\r", ch );
    }
    else
    {
	send_to_char( "\n\r", ch );
        for (paf = ch->first_affect; paf; paf = paf->next)
	    if ( (skill=get_skilltype(paf->type)) != NULL )
        {
            set_char_color( AT_BLUE, ch );
            send_to_char( "> affected:  ", ch );
            set_char_color( AT_SCORE, ch );
            ch_printf( ch, "%-18s\n\r", skill->name );
        }
    }
    return;
}

void do_inventory( CHAR_DATA *ch, char *argument )
{
    set_char_color( AT_RED, ch );
    send_to_char( "> content of storage memory:\n\r", ch );
    show_list_to_char( ch->first_carrying, ch, TRUE, TRUE, eItemDrop );
    return;
}


void do_equipment( CHAR_DATA *ch, char *argument )
{
    OBJ_DATA *obj;
    int iWear, dam;
    bool found;
    char buf[MAX_STRING_LENGTH];
    
    set_char_color( AT_RED, ch );
    send_to_char( "> loaded into active memory:\n\r", ch );
    found = FALSE;
    set_char_color( AT_OBJECT, ch );
    for ( iWear = 0; iWear < MAX_WEAR; iWear++ )
    {
	for ( obj = ch->first_carrying; obj; obj = obj->next_content )
	   if ( obj->wear_loc == iWear )
	   {
		send_to_char( where_name[iWear], ch );
		if ( can_see_obj( ch, obj ) )
		{
		    send_to_char( format_obj_to_char( obj, ch, TRUE ), ch );
		    strcpy( buf , "" );
		    switch ( obj->item_type )
	            {
	                default:
	                break;

	                case ITEM_ARMOR:
	    		    if ( obj->value[1] == 0 )
	      			obj->value[1] = obj->value[0];
	    		    if ( obj->value[1] == 0 )
	      			obj->value[1] = 1;
	    		    dam = (sh_int) ((obj->value[0] * 10) / obj->value[1]);
			    if (dam >= 10) strcat( buf, " (superb) ");
			    else if (dam >=  7) strcat( buf, " (good) ");
 		            else if (dam >=  5) strcat( buf, " (worn) ");
			    else if (dam >=  3) strcat( buf, " (poor) ");
			    else if (dam >=  1) strcat( buf, " (awful) ");
			    else if (dam ==  0) strcat( buf, " (broken) ");
                  	    send_to_char( buf, ch );
	                    break;

	                 case ITEM_WEAPON:
	                   dam = INIT_WEAPON_CONDITION - obj->value[0];
	                   if (dam < 2) strcat( buf, " (superb) ");
                           else if (dam < 4) strcat( buf, " (good) ");
                           else if (dam < 7) strcat( buf, " (worn) ");
                           else if (dam < 10) strcat( buf, " (poor) ");
                           else if (dam < 12) strcat( buf, " (awful) ");
                           else if (dam ==  12) strcat( buf, " (broken) ");
                	   send_to_char( buf, ch );
	                   if (obj->value[3] == WEAPON_BLASTER )
	                   {
		            if (obj->blaster_setting == BLASTER_FULL)
	    		      ch_printf( ch, "FULL");
	  	            else if (obj->blaster_setting == BLASTER_HIGH)
	    		      ch_printf( ch, "HIGH");
	  	            else if (obj->blaster_setting == BLASTER_NORMAL)
	    		      ch_printf( ch, "NORMAL");
	  	            else if (obj->blaster_setting == BLASTER_HALF)
	    		      ch_printf( ch, "HALF");
	  	            else if (obj->blaster_setting == BLASTER_LOW)
	    		      ch_printf( ch, "LOW");
	  	            else if (obj->blaster_setting == BLASTER_STUN)
	    		      ch_printf( ch, "STUN");
	  	            ch_printf( ch, " %d", obj->value[4] );
	                   }
	                   else if (     ( obj->value[3] == WEAPON_LIGHTSABER || 
		           obj->value[3] == WEAPON_VIBRO_BLADE ) )
	                   {
		             ch_printf( ch, "%d", obj->value[4] );
	                   }        
	                   break;
                    }   
		    send_to_char( "\n\r", ch );
		}
		else
		    send_to_char( "something\n\r", ch );
		found = TRUE;
	   }
    }

    if ( !found )
	send_to_char( "nothing\n\r", ch );

    return;
}



void set_title( CHAR_DATA *ch, char *title )
{
    char buf[MAX_STRING_LENGTH];

    if ( IS_NPC(ch) )
    {
	bug( "Set_title: NPC.", 0 );
	return;
    }

    if ( isalpha(title[0]) || isdigit(title[0]) )
    {
	buf[0] = ' ';
	strcpy( buf+1, title );
    }
    else
	strcpy( buf, title );

    STRFREE( ch->pcdata->title );
    ch->pcdata->title = STRALLOC( buf );
    return;
}



void do_title( CHAR_DATA *ch, char *argument )
{
    if ( IS_NPC(ch) )
	return;

    if ( IS_SET( ch->pcdata->flags, PCFLAG_NOTITLE ))
    {
        send_to_char( "> you try but the Force resists you\n\r", ch );
        return;
    }
 

    if ( argument[0] == '\0' )
    {
	send_to_char( "> change your title to what\n\r", ch );
	return;
    }

    if ( !IS_IMMORTAL(ch) && (!nifty_is_name(ch->name, argument)))
     {
       send_to_char("> you must include your name somewhere in your title", ch);
       return;
     }
 
    smash_tilde( argument );
    set_title( ch, argument );
    send_to_char( "> ok\n\r", ch );
}


void do_homepage( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];

    if ( IS_NPC(ch) )
	return;

    if ( argument[0] == '\0' )
    {
	if ( !ch->pcdata->homepage )
	  ch->pcdata->homepage = str_dup( "" );
	ch_printf( ch, "> your homepage is: %s\n\r",
		show_tilde( ch->pcdata->homepage ) );
	return;
    }

    if ( !str_cmp( argument, "clear" ) )
    {
	if ( ch->pcdata->homepage )
	  DISPOSE(ch->pcdata->homepage);
	ch->pcdata->homepage = str_dup("");
	send_to_char( "> homepage cleared\n\r", ch );
	return;
    }

    if ( strstr( argument, "://" ) )
	strcpy( buf, argument );
    else
	sprintf( buf, "http://%s", argument );
    if ( strlen(buf) > 70 )
	buf[70] = '\0';

    hide_tilde( buf );
    if ( ch->pcdata->homepage )
      DISPOSE(ch->pcdata->homepage);
    ch->pcdata->homepage = str_dup(buf);
    send_to_char( "> homepage set\n\r", ch );
}



/*
 * Set your personal description				-Thoric
 */
void do_description( CHAR_DATA *ch, char *argument )
{
    if ( IS_NPC( ch ) )
    {
	send_to_char( "Monsters are too dumb to do that\n\r", ch );
	return;	  
    }

    if ( !ch->desc )
    {
	bug( "do_description: no descriptor", 0 );
	return;
    }

    switch( ch->substate )
    {
	default:
	   bug( "do_description: illegal substate", 0 );
	   return;

	case SUB_RESTRICTED:
	   send_to_char( "> you cannot use this command from within another command\n\r", ch );
	   return;

	case SUB_NONE:
	   ch->substate = SUB_PERSONAL_DESC;
	   ch->dest_buf = ch;
	   start_editing( ch, ch->description );
	   return;

	case SUB_PERSONAL_DESC:
	   STRFREE( ch->description );
	   ch->description = copy_buffer( ch );
	   stop_editing( ch );
	   return;	
    }
}

/* Ripped off do_description for whois bio's -- Scryn*/
void do_bio( CHAR_DATA *ch, char *argument )
{
    if ( IS_NPC( ch ) )
    {
	send_to_char( "ICE cannot set bio's\n\r", ch );
	return;	  
    }

    if ( !ch->desc )
    {
	bug( "do_bio: no descriptor", 0 );
	return;
    }

    switch( ch->substate )
    {
	default:
	   bug( "do_bio: illegal substate", 0 );
	   return;
	  	   
	case SUB_RESTRICTED:
	   send_to_char( "> you cannot use this command from within another command\n\r", ch );
	   return;

	case SUB_NONE:
	   ch->substate = SUB_PERSONAL_BIO;
	   ch->dest_buf = ch;
	   start_editing( ch, ch->pcdata->bio );
	   return;

	case SUB_PERSONAL_BIO:
	   STRFREE( ch->pcdata->bio );
	   ch->pcdata->bio = copy_buffer( ch );
	   stop_editing( ch );
	   return;	
    }
}



void do_report( CHAR_DATA *ch, char *argument )
{
}

void do_prompt( CHAR_DATA *ch, char *argument )
{
  char arg[MAX_INPUT_LENGTH];
  
  if ( IS_NPC(ch) )
  {
    send_to_char( "NPC's cannot change their prompt.\n\r", ch );
    return;
  }
  smash_tilde( argument );
  one_argument( argument, arg );
  if ( !*arg )
  {
    send_to_char( "> set prompt to what [try help prompt]\n\r", ch );
    return;
  }
  if (ch->pcdata->prompt)
    STRFREE(ch->pcdata->prompt);

  if ( strlen(argument) > 128 )
    argument[128] = '\0';

  /* Can add a list of pre-set prompts here if wanted.. perhaps
     'prompt 1' brings up a different, pre-set prompt */
  if ( !str_cmp(arg, "default") )
    ch->pcdata->prompt = STRALLOC("");
  else
    ch->pcdata->prompt = STRALLOC(argument);
  send_to_char( "> ok\n\r", ch );
  return;
}

void do_skills( CHAR_DATA *ch, char *argument )
{
    int sn;
    //char level[MAX_STRING_LENGTH];

    if( IS_NPC(ch) || !ch->pcdata )
	return;

    send_to_char("&Cskillsoft           level\n\r", ch );
    send_to_char("&C-------------------------\n\r", ch );

    	for ( sn = 0; sn < top_sn ; sn++ )
    	   if ( ch->pcdata->learned[sn] > 0 )
	   {
		 ch_printf( ch,  "&G%-15s     &Y%d\n\r",
		 skill_table[sn]->name,
		 ch->pcdata->learned[sn] );
	   }	     	 
        send_to_char( "\n\r" , ch );
}

//done for Neuro
