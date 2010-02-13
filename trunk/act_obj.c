#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "mud.h"
#include "bet.h"

/*double sqrt( double x );*/

/*
 * External functions
 */

void    show_list_to_char  args( ( OBJ_DATA *list, CHAR_DATA *ch,
				bool fShort, bool fShowNothing ) );
/*
 * Local functions.
 */
void	get_obj		args( ( CHAR_DATA *ch, OBJ_DATA *obj,
			    OBJ_DATA *container ) );
bool	remove_obj	args( ( CHAR_DATA *ch, int iWear, bool fReplace ) );
void	wear_obj	args( ( CHAR_DATA *ch, OBJ_DATA *obj, bool fReplace, sh_int wear_bit ) );
bool    job_trigger     args( ( CHAR_DATA *victim, CHAR_DATA *ch, OBJ_DATA *obj ) );
bool    agent_trigger     args( ( CHAR_DATA *victim, CHAR_DATA *ch, OBJ_DATA *obj ) );


/*
 * how resistant an object is to damage				-Thoric
 */
sh_int get_obj_resistance( OBJ_DATA *obj )
{
    sh_int resist;

    resist = number_fuzzy(MAX_ITEM_IMPACT);

    /* magical items are more resistant */
    if ( IS_OBJ_STAT( obj, ITEM_MAGIC ) )
      resist += number_fuzzy(12);
    /* blessed objects should have a little bonus */
    if ( IS_OBJ_STAT( obj, ITEM_BLESS ) )
      resist += number_fuzzy(5);
    /* lets make store inventory pretty tough */
    if ( IS_OBJ_STAT( obj, ITEM_INVENTORY ) )
      resist += 20;

    /* okay... let's add some bonus/penalty for item level... */
    resist += (obj->level / 10);

    /* and lasty... take armor or weapon's condition into consideration */
    if (obj->item_type == ITEM_ARMOR || obj->item_type == ITEM_WEAPON)
      resist += (obj->value[0]);

    return URANGE(10, resist, 99);
}


void get_obj( CHAR_DATA *ch, OBJ_DATA *obj, OBJ_DATA *container )
{
    int weight;

    if ( !CAN_WEAR(obj, ITEM_TAKE)
       && !IS_IMMORTAL(ch)  )
    {
	send_to_char( "> you cannot take that\n\r", ch );
	return;
    }

    if ( IS_OBJ_STAT( obj, ITEM_PROTOTYPE )
    &&  !can_take_proto( ch ) )
    {
	send_to_char( "> a force prevents you from getting close to it\n\r", ch );
	return;
    }

    if ( ch->carry_number + get_obj_number( obj ) > can_carry_n( ch ) )
    {
	act( AT_PLAIN, "> $d: you cannot carry that many items",
		ch, NULL, obj->name, TO_CHAR );
	return;
    }

    if ( IS_OBJ_STAT( obj, ITEM_COVERING ) )
      weight = obj->weight;
    else
      weight = get_obj_weight( obj );

    if ( ch->carry_weight + weight > can_carry_w( ch ) )
    {
	act( AT_PLAIN, "> $d: you cannot carry that much weight",
		ch, NULL, obj->name, TO_CHAR );
	return;
    }

    if ( container )
    {
	act( AT_ACTION, IS_OBJ_STAT(container, ITEM_COVERING) ?
		"> you get $p from beneath $P." : "> you get $p from $P",
		ch, obj, container, TO_CHAR );
	act( AT_ACTION, IS_OBJ_STAT(container, ITEM_COVERING) ?
		"> $n gets $p from beneath $P." : "> $n gets $p from $P",
		ch, obj, container, TO_ROOM );
	obj_from_obj( obj );
    }
    else
    {
	act( AT_ACTION, "> you get $p", ch, obj, container, TO_CHAR );
	act( AT_ACTION, "> $n gets $p", ch, obj, container, TO_ROOM );
	obj_from_room( obj );
    }


    if ( obj->item_type == ITEM_MONEY )
    {
	ch->gold += obj->value[0];
	extract_obj( obj );
    }
    else
    {
	obj = obj_to_char( obj, ch );
    }

    if ( char_died(ch) || obj_extracted(obj) )
      return;
    oprog_get_trigger(ch, obj);
    return;
}


void do_get( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    OBJ_DATA *obj_next;
    OBJ_DATA *container;
    sh_int number;
    bool found;

    argument = one_argument( argument, arg1 );
    if ( is_number(arg1) )
    {
	number = atoi(arg1);
	if ( number < 1 )
	{
	    send_to_char( "> that was easy\n\r", ch );
	    return;
	}
	if ( (ch->carry_number + number) > can_carry_n(ch) )
	{
	    send_to_char( "> you cannot carry that many\n\r", ch );
	    return;
	}
	argument = one_argument( argument, arg1 );
    }
    else
	number = 0;
    argument = one_argument( argument, arg2 );
    /* munch optional words */
    if ( !str_cmp( arg2, "from" ) && argument[0] != '\0' )
	argument = one_argument( argument, arg2 );

    /* Get type. */
    if ( arg1[0] == '\0' )
    {
	send_to_char( "> get [object]\n\r", ch );
	return;
    }

    if ( ms_find_obj(ch) )
	return;

    if ( arg2[0] == '\0' )
    {
	if ( number <= 1 && str_cmp( arg1, "all" ) && str_prefix( "all", arg1 ) )
	{
	    /* 'get obj' */
	    obj = get_obj_list( ch, arg1, ch->in_room->first_content );
	    if ( !obj )
	    {
		act( AT_PLAIN, "> $T not found", ch, NULL, arg1, TO_CHAR );
		return;
	    }
	    separate_obj(obj);
	    get_obj( ch, obj, NULL );
	    if ( char_died(ch) )
		return;
	    if ( IS_SET( sysdata.save_flags, SV_GET ) )
		save_char_obj( ch );
	}
	else
	{
	    sh_int cnt = 0;
	    bool fAll;
	    char *chk;

	    if ( !str_cmp(arg1, "all") )
		fAll = TRUE;
	    else
		fAll = FALSE;
	    if ( number > 1 )
		chk = arg1;
	    else
		chk = &arg1[4];
	    /* 'get all' or 'get all.obj' */
	    found = FALSE;
	    for ( obj = ch->in_room->first_content; obj; obj = obj_next )
	    {
		obj_next = obj->next_content;
		if ( ( fAll || nifty_is_name( chk, obj->name ) )
		&&   can_see_obj( ch, obj ) )
		{
		    found = TRUE;
		    if ( number && (cnt + obj->count) > number )
			split_obj( obj, number - cnt );
		    cnt += obj->count;
		    get_obj( ch, obj, NULL );
		    if ( char_died(ch)
		    ||   ch->carry_number >= can_carry_n( ch )
		    ||   ch->carry_weight >= can_carry_w( ch )
		    ||   (number && cnt >= number) )
		    {
			if ( IS_SET(sysdata.save_flags, SV_GET)
			&&  !char_died(ch) )
			    save_char_obj(ch);
			return;
		    }
		}
	    }

	    if ( !found )
	    {
		if ( fAll )
		  send_to_char( "> object not found\n\r", ch );
		else
		  act( AT_PLAIN, "> $T not found", ch, NULL, chk, TO_CHAR );
	    }
	    else
	    if ( IS_SET( sysdata.save_flags, SV_GET ) )
		save_char_obj( ch );
	}
    }
    else
    {
	/* 'get ... container' */
	if ( !str_cmp( arg2, "all" ) || !str_prefix( "all", arg2 ) )
	{
	    send_to_char( "> invalid command\n\r", ch );
	    return;
	}

	if ( ( container = get_obj_here( ch, arg2 ) ) == NULL )
	{
	    act( AT_PLAIN, "> $T not found", ch, NULL, arg2, TO_CHAR );
	    return;
	}

	switch ( container->item_type )
	{
	default:
	    if ( !IS_OBJ_STAT( container, ITEM_COVERING ) )
	    {
		send_to_char( "> object not a container\n\r", ch );
		return;
	    }
	    if ( ch->carry_weight + container->weight > can_carry_w( ch ) )
	    {
		send_to_char( "> object too large\n\r", ch );
		return;
	    }
	    break;

	case ITEM_CONTAINER:
	case ITEM_DROID_CORPSE:
	case ITEM_CORPSE_PC:
	case ITEM_CORPSE_NPC:
	    break;
	}

	if ( !IS_OBJ_STAT(container, ITEM_COVERING )
	&&    IS_SET(container->value[1], CONT_CLOSED) )
	{
	    act( AT_PLAIN, "> $d is closed", ch, NULL, container->name, TO_CHAR );
	    return;
	}

	if ( number <= 1 && str_cmp( arg1, "all" ) && str_prefix( "all", arg1 ) )
	{
	    /* 'get obj container' */
	    obj = get_obj_list( ch, arg1, container->first_content );
	    if ( !obj )
	    {
		act( AT_PLAIN, IS_OBJ_STAT(container, ITEM_COVERING) ?
			"> object not beneath $T" :
			"> object not in $T",
			ch, NULL, arg2, TO_CHAR );
		return;
	    }
	    separate_obj(obj);
	    get_obj( ch, obj, container );

	    if ( IS_SET( sysdata.save_flags, SV_GET ) )
		save_char_obj( ch );
	}
	else
	{
	    int cnt = 0;
	    bool fAll;
	    char *chk;

	    /* 'get all container' or 'get all.obj container' */
	    if ( IS_OBJ_STAT( container, ITEM_DONATION ) )
	    {
		send_to_char( "> invalid command\n\r", ch );
		return;
	    }
	    if ( !str_cmp(arg1, "all") )
		fAll = TRUE;
	    else
		fAll = FALSE;
	    if ( number > 1 )
		chk = arg1;
	    else
		chk = &arg1[4];
	    found = FALSE;
        for( obj = container->first_content; obj; obj = obj_next )
        {
           obj_next = obj->next_content;
           if( ( fAll || nifty_is_name( chk, obj->name ) ) && can_see_obj( ch, obj ) )
           {
              found = TRUE;
              if( number && ( cnt + obj->count ) > number )
                 split_obj( obj, number - cnt );
              cnt += obj->count;
              get_obj( ch, obj, container );
              if( char_died( ch )
                  || ch->carry_number >= can_carry_n( ch )
                  || ch->carry_weight >= can_carry_w( ch ) || ( number && cnt >= number ) )
              {
                 if( container->item_type == ITEM_CORPSE_PC )
                    write_corpses( NULL, container->short_descr + 14 );
                 if( found && IS_SET( sysdata.save_flags, SV_GET ) )
                    save_char_obj( ch );
                 return;
              }
           }
        }

	    if ( !found )
	    {
		if ( fAll )
		  act( AT_PLAIN, IS_OBJ_STAT(container, ITEM_COVERING) ?
			"> object not beneath $T" :
			"> object not in $T",
			ch, NULL, arg2, TO_CHAR );
		else
		  act( AT_PLAIN, IS_OBJ_STAT(container, ITEM_COVERING) ?
			"> object not beneath $T" :
			"> object not in $T",
			ch, NULL, arg2, TO_CHAR );
	    }

	    if ( char_died(ch) )
		return;
	    if ( found && IS_SET( sysdata.save_flags, SV_GET ) )
		save_char_obj( ch );
	}
    }
    return;
}



void do_put( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    OBJ_DATA *container;
    OBJ_DATA *obj;
    OBJ_DATA *obj_next;
    sh_int	count;
    int		number;
    bool	save_char = FALSE;

    argument = one_argument( argument, arg1 );
    if ( is_number(arg1) )
    {
	number = atoi(arg1);
	if ( number < 1 )
	{
	    send_to_char( "> that was easy\n\r", ch );
	    return;
	}
	argument = one_argument( argument, arg1 );
    }
    else
	number = 0;
    argument = one_argument( argument, arg2 );
    /* munch optional words */
    if ( (!str_cmp(arg2, "into") || !str_cmp(arg2, "inside") || !str_cmp(arg2, "in"))
    &&   argument[0] != '\0' )
	argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' || arg2[0] == '\0' )
    {
	send_to_char( "> put [object] in [container]\n\r", ch );
	return;
    }

    if ( ms_find_obj(ch) )
	return;

    if ( !str_cmp( arg2, "all" ) || !str_prefix( "all", arg2 ) )
    {
	send_to_char( "> invalid command\n\r", ch );
	return;
    }

    if ( ( container = get_obj_here( ch, arg2 ) ) == NULL )
    {
	act( AT_PLAIN, "> $T not found", ch, NULL, arg2, TO_CHAR );
	return;
    }

    if ( !container->carried_by && IS_SET( sysdata.save_flags, SV_PUT ) )
	save_char = TRUE;

    if ( IS_OBJ_STAT(container, ITEM_COVERING) )
    {
	if ( ch->carry_weight + container->weight > can_carry_w( ch ) )
	{
	    send_to_char( "> object is too large\n\r", ch );
	    return;
	}
    }
    else
    {
	if ( container->item_type != ITEM_CONTAINER )
	{
	    send_to_char( "> object is not a container\n\r", ch );
	    return;
	}

	if ( IS_SET(container->value[1], CONT_CLOSED) )
	{
	    act( AT_PLAIN, "> the $d is closed", ch, NULL, container->name, TO_CHAR );
	    return;
	}
    }

    if ( number <= 1 && str_cmp( arg1, "all" ) && str_prefix( "all", arg1 ) )
    {
	/* 'put obj container' */
	if ( ( obj = get_obj_carry( ch, arg1 ) ) == NULL )
	{
	    send_to_char( "> object not found\n\r", ch );
	    return;
	}

	if ( obj == container )
	{
	    send_to_char( "> you cannot fold it into itself\n\r", ch );
	    return;
	}

	if ( !can_drop_obj( ch, obj ) )
	{
	    send_to_char( "> you cannot let go of it\n\r", ch );
	    return;
	}

	if ( (IS_OBJ_STAT(container, ITEM_COVERING)
	&&   (get_obj_weight( obj ) / obj->count)
	  > ((get_obj_weight( container ) / container->count)
	  -   container->weight)) )
	{
	    send_to_char( "> it will not fit under there\n\r", ch );
	    return;
	}

	if ( (get_obj_weight( obj ) / obj->count)
	   + (get_obj_weight( container ) / container->count)
	   >  container->value[0] )
	{
	    send_to_char( "> it will not fit\n\r", ch );
	    return;
	}

	separate_obj(obj);
	separate_obj(container);
	obj_from_char( obj );
	obj = obj_to_obj( obj, container );
	count = obj->count;
	obj->count = 1;
 	act( AT_ACTION, IS_OBJ_STAT( container, ITEM_COVERING )
 		? "> $n hides $p beneath $P." : "> $n puts $p in $P",
		ch, obj, container, TO_ROOM );
	act( AT_ACTION, IS_OBJ_STAT( container, ITEM_COVERING )
		? "> you hide $p beneath $P." : "> you put $p in $P",
		ch, obj, container, TO_CHAR );
	obj->count = count;

	if ( save_char )
	  save_char_obj(ch);
    }
    else
    {
	bool found = FALSE;
	int cnt = 0;
	bool fAll;
	char *chk;

	if ( !str_cmp(arg1, "all") )
	    fAll = TRUE;
	else
	    fAll = FALSE;
	if ( number > 1 )
	    chk = arg1;
	else
	    chk = &arg1[4];

	separate_obj(container);
	/* 'put all container' or 'put all.obj container' */
	for ( obj = ch->first_carrying; obj; obj = obj_next )
	{
	    obj_next = obj->next_content;

	    if ( ( fAll || nifty_is_name( chk, obj->name ) )
	    &&   can_see_obj( ch, obj )
	    &&   obj->wear_loc == WEAR_NONE
	    &&   obj != container
	    &&   can_drop_obj( ch, obj )
	    &&   get_obj_weight( obj ) + get_obj_weight( container )
		 <= container->value[0] )
	    {
		if ( number && (cnt + obj->count) > number )
		    split_obj( obj, number - cnt );
		cnt += obj->count;
		obj_from_char( obj );
		act( AT_ACTION, "> $n puts $p in $P", ch, obj, container, TO_ROOM );
		act( AT_ACTION, "> you put $p in $P", ch, obj, container, TO_CHAR );
		obj = obj_to_obj( obj, container );
		found = TRUE;

		if ( number && cnt >= number )
		  break;
	    }
	}

	/*
	 * Don't bother to save anything if nothing was dropped   -Thoric
	 */
	if ( !found )
	{
	    if ( fAll )
	      act( AT_PLAIN, "> you are not carrying anything",
		    ch, NULL, NULL, TO_CHAR );
	    else
	      act( AT_PLAIN, "> you are not carrying any $T",
		    ch, NULL, chk, TO_CHAR );
	    return;
	}

	if ( save_char )
	    save_char_obj(ch);
    }

    return;
}


void do_drop( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    OBJ_DATA *obj_next;
    bool found;
    int number;

    argument = one_argument( argument, arg );
    if ( is_number(arg) )
    {
	number = atoi(arg);
	if ( number < 1 )
	{
	    send_to_char( "> that was easy\n\r", ch );
	    return;
	}
	argument = one_argument( argument, arg );
    }
    else
	number = 0;

    if ( arg[0] == '\0' )
    {
	send_to_char( "> drop what\n\r", ch );
	return;
    }

    if ( ms_find_obj(ch) )
	return;

    if ( number > 0 )
    {
	/* 'drop NNNN coins' */

	if ( !str_cmp( arg, "credits" ) || !str_cmp( arg, "credit" ) )
	{

		if( !IS_IMMORTAL(ch) )
	    {
	    send_to_char("&R> you would not want to throw away your money\n\r", ch );
	    return;
	    }

	    if ( ch->gold < number )
	    {
		send_to_char( "> you have not got that many credits\n\r", ch );
		return;
	    }

	    ch->gold -= number;

	    for ( obj = ch->in_room->first_content; obj; obj = obj_next )
	    {
		obj_next = obj->next_content;

		switch ( obj->pIndexData->vnum )
		{
		case OBJ_VNUM_MONEY_ONE:
		   number += 1;
		   extract_obj( obj );
		   break;

		case OBJ_VNUM_MONEY_SOME:
		   number += obj->value[0];
		   extract_obj( obj );
		   break;
		}
	    }

	    act( AT_ACTION, "> $n drops some credits", ch, NULL, NULL, TO_ROOM );
	    obj_to_room( create_money( number ), ch->in_room );
	    send_to_char( "> ok\n\r", ch );
	    if ( IS_SET( sysdata.save_flags, SV_DROP ) )
		save_char_obj( ch );
	    return;
	}
    }

    if ( number <= 1 && str_cmp( arg, "all" ) && str_prefix( "all", arg ) )
    {
	/* 'drop obj' */
	if ( ( obj = get_obj_carry( ch, arg ) ) == NULL )
	{
	    send_to_char( "> you do not have that item\n\r", ch );
	    return;
	}

	if ( !can_drop_obj( ch, obj ) )
	{
	    send_to_char( "> you cannot let go of it\n\r", ch );
	    return;
	}

	separate_obj( obj );
	act( AT_ACTION, "> $n drops $p", ch, obj, NULL, TO_ROOM );
	act( AT_ACTION, "> you drop $p", ch, obj, NULL, TO_CHAR );

	obj_from_char( obj );
	obj = obj_to_room( obj, ch->in_room );
	oprog_drop_trigger ( ch, obj );   /* mudprogs */

        if( char_died(ch) || obj_extracted(obj) )
          return;

    }
    else
    {
	int cnt = 0;
	char *chk;
	bool fAll;

	if ( !str_cmp(arg, "all") )
	    fAll = TRUE;
	else
	    fAll = FALSE;
	if ( number > 1 )
	    chk = arg;
	else
	    chk = &arg[4];
	found = FALSE;
	for ( obj = ch->first_carrying; obj; obj = obj_next )
	{
	    obj_next = obj->next_content;

	    if ( (fAll || nifty_is_name( chk, obj->name ) )
	    &&   can_see_obj( ch, obj )
	    &&   obj->wear_loc == WEAR_NONE
	    &&   can_drop_obj( ch, obj ) )
	    {
		found = TRUE;
		if ( obj->pIndexData->progtypes & DROP_PROG && obj->count > 1 )
		{
		   ++cnt;
		   separate_obj( obj );
		   obj_from_char( obj );
		   if ( !obj_next )
		     obj_next = ch->first_carrying;
		}
		else
		{
		   if ( number && (cnt + obj->count) > number )
		     split_obj( obj, number - cnt );
		   cnt += obj->count;
		   obj_from_char( obj );
		}
		act( AT_ACTION, "> $n drops $p", ch, obj, NULL, TO_ROOM );
		act( AT_ACTION, "> you drop $p", ch, obj, NULL, TO_CHAR );
		obj = obj_to_room( obj, ch->in_room );
		oprog_drop_trigger( ch, obj );		/* mudprogs */
                if ( char_died(ch) )
                    return;
		if ( number && cnt >= number )
		    break;
	    }
	}


	if ( !found )
	{
	    if ( fAll )
	      act( AT_PLAIN, "> you are not carrying anything",
		    ch, NULL, NULL, TO_CHAR );
	    else
	      act( AT_PLAIN, "> you are not carrying any $T",
		    ch, NULL, chk, TO_CHAR );
	}
    }
    if ( IS_SET( sysdata.save_flags, SV_DROP ) )
	save_char_obj( ch );	/* duping protector */
    return;
}



void do_give( CHAR_DATA *ch, char *argument )
{
    char arg1 [MAX_INPUT_LENGTH];
    char arg2 [MAX_INPUT_LENGTH];
    char buf  [MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    OBJ_DATA  *obj;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    if ( !str_cmp( arg2, "to" ) && argument[0] != '\0' )
	argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' || arg2[0] == '\0' )
    {
	send_to_char( "> give what to whom\n\r", ch );
	return;
    }

    if ( ms_find_obj(ch) )
	return;

    if ( is_number( arg1 ) )
    {
	/* 'give NNNN coins victim' */
	int amount;

	amount   = atoi(arg1);
	if ( amount <= 0
	|| ( str_cmp( arg2, "credits" ) && str_cmp( arg2, "credit" ) ) )
	{
	    send_to_char( "> invalid command\n\r", ch );
	    return;
	}

	argument = one_argument( argument, arg2 );
	if ( !str_cmp( arg2, "to" ) && argument[0] != '\0' )
	  argument = one_argument( argument, arg2 );
	if ( arg2[0] == '\0' )
	{
	    send_to_char( "> give what to whom\n\r", ch );
	    return;
	}

	if ( ( victim = get_char_room( ch, arg2 ) ) == NULL )
	{
	    send_to_char( "> they are not here\n\r", ch );
	    return;
	}

	if ( !victim->desc && !IS_NPC(victim) )
	{
	    send_to_char( "> you cannot give that to them\n\r", ch );
	    return;
	}

	if ( ch->gold < amount )
	{
	    send_to_char( "> you do not have that many credits\n\r", ch );
	    return;
	}

	ch->gold     -= amount;
	victim->gold += amount;
        strcpy(buf, "> $n gives you ");
        strcat(buf, arg1);
        strcat(buf, (amount > 1) ? " credits." : " credit.");

	act( AT_GOLD, buf, ch, NULL, victim, TO_VICT    );
	act( AT_GOLD, "> $n gives $N some credits",  ch, NULL, victim, TO_NOTVICT );
	act( AT_GOLD, "> you give $N some credits",  ch, NULL, victim, TO_CHAR    );
	send_to_char( "> ok\n\r", ch );
	mprog_bribe_trigger( victim, ch, amount );
	if ( IS_SET( sysdata.save_flags, SV_GIVE ) && !char_died(ch) )
	  save_char_obj(ch);
	if ( IS_SET( sysdata.save_flags, SV_RECEIVE ) && !char_died(victim) )
	  save_char_obj(victim);
	return;
    }

    if ( ( obj = get_obj_carry( ch, arg1 ) ) == NULL )
    {
	send_to_char( "> you do not have that item\n\r", ch );
	return;
    }

    if ( obj->wear_loc != WEAR_NONE )
    {
	send_to_char( "> you must remove it first\n\r", ch );
	return;
    }

    if ( ( victim = get_char_room( ch, arg2 ) ) == NULL )
    {
	send_to_char( "> they are not here\n\r", ch );
	return;
    }

    if ( !victim->desc && !IS_NPC(victim) )
    {
	send_to_char( "> you cannot give code to them\n\r", ch );
	return;
    }

    if ( !can_drop_obj( ch, obj ) )
    {
	send_to_char( "> you cannot let go of it\n\r", ch );
	return;
    }

    if ( victim->carry_number + (get_obj_number(obj)/obj->count) > can_carry_n( victim ) )
    {
	act( AT_PLAIN, "> $N has $S hands full", ch, NULL, victim, TO_CHAR );
	return;
    }

    if ( victim->carry_weight + (get_obj_weight(obj)/obj->count) > can_carry_w( victim ) )
    {
	act( AT_PLAIN, "> $N cannot carry that much weight", ch, NULL, victim, TO_CHAR );
	return;
    }

    if ( !can_see_obj( victim, obj ) )
    {
	act( AT_PLAIN, "> $N cannot see it", ch, NULL, victim, TO_CHAR );
	return;
    }

    if (IS_OBJ_STAT( obj, ITEM_PROTOTYPE ) && !can_take_proto( victim ) )
    {
	act( AT_PLAIN, "> you cannot give that to $N", ch, NULL, victim, TO_CHAR );
	return;
    }

    separate_obj( obj );
    obj_from_char( obj );
    act( AT_ACTION, "> $n gives $p to $N", ch, obj, victim, TO_NOTVICT );
    act( AT_ACTION, "> $n gives you $p",   ch, obj, victim, TO_VICT    );
    act( AT_ACTION, "> you give $p to $N", ch, obj, victim, TO_CHAR    );
    obj = obj_to_char( obj, victim );

    if ( job_trigger( victim, ch, obj ) == FALSE  &&  agent_trigger( victim, ch, obj ) == FALSE )
         mprog_give_trigger( victim, ch, obj );

    if ( IS_SET( sysdata.save_flags, SV_GIVE ) && !char_died(ch) )
	save_char_obj(ch);
    if ( IS_SET( sysdata.save_flags, SV_RECEIVE ) && !char_died(victim) )
	save_char_obj(victim);
    return;
}

/*
 * Damage an object.						-Thoric
 * Affect player's AC if necessary.
 * Make object into scraps if necessary.
 * Send message about damaged object.
 */
obj_ret damage_obj( OBJ_DATA *obj )
{
    CHAR_DATA *ch;
    obj_ret objcode;

    ch = obj->carried_by;
    objcode = rNONE;

    separate_obj( obj );
    if ( ch )
      act( AT_OBJECT, "($p gets damaged)", ch, obj, NULL, TO_CHAR );
    else
    if ( obj->in_room && ( ch = obj->in_room->first_person ) != NULL )
    {
	act( AT_OBJECT, "($p gets damaged)", ch, obj, NULL, TO_ROOM );
	act( AT_OBJECT, "($p gets damaged)", ch, obj, NULL, TO_CHAR );
	ch = NULL;
    }

    oprog_damage_trigger(ch, obj);
    if ( obj_extracted(obj) )
      return global_objcode;

    switch( obj->item_type )
    {
	default:
	  make_scraps( obj );
	  objcode = rOBJ_SCRAPPED;
	  break;
	case ITEM_CONTAINER:
	  if (--obj->value[3] <= 0)
	  {
		make_scraps( obj );
		objcode = rOBJ_SCRAPPED;
	  }
	  break;
	case ITEM_ARMOR:
	  if ( ch && obj->value[0] >= 1 )
	    ch->armor += apply_ac( obj, obj->wear_loc );
	  if (--obj->value[0] <= 0)
	  {
		make_scraps( obj );
		objcode = rOBJ_SCRAPPED;
	  }
	  else
	  if ( ch && obj->value[0] >= 1 )
	    ch->armor -= apply_ac( obj, obj->wear_loc );
	  break;
	case ITEM_WEAPON:
	  if (--obj->value[0] <= 0)
	  {
		make_scraps( obj );
		objcode = rOBJ_SCRAPPED;
	  }
	  break;
    }
    return objcode;
}


/*
 * Remove an object.
 */
bool remove_obj( CHAR_DATA *ch, int iWear, bool fReplace )
{
    OBJ_DATA *obj, *tmpobj;

    if ( ( obj = get_eq_char( ch, iWear ) ) == NULL )
	return TRUE;

    if ( !fReplace
    &&   ch->carry_number + get_obj_number( obj ) > can_carry_n( ch ) )
    {
	act( AT_PLAIN, "> $d: you cannot carry that many items",
	    ch, NULL, obj->name, TO_CHAR );
	return FALSE;
    }

    if ( !fReplace )
	return FALSE;

    if ( IS_OBJ_STAT(obj, ITEM_NOREMOVE) )
    {
	act( AT_PLAIN, "> you cannot remove $p", ch, obj, NULL, TO_CHAR );
	return FALSE;
    }

    if ( obj == get_eq_char( ch, WEAR_WIELD )
    && ( tmpobj = get_eq_char( ch, WEAR_DUAL_WIELD)) != NULL )
       tmpobj->wear_loc = WEAR_WIELD;

    unequip_char( ch, obj );

    act( AT_ACTION, "> $n stops using $p", ch, obj, NULL, TO_ROOM );
    act( AT_ACTION, "> you stop using $p", ch, obj, NULL, TO_CHAR );
    oprog_remove_trigger( ch, obj );
    return TRUE;
}

/*
 * See if char could be capable of dual-wielding		-Thoric
 */
bool could_dual( CHAR_DATA *ch )
{
  if ( IS_NPC(ch) )
    return TRUE;
  if ( ch->pcdata->learned[gsn_dual_wield] )
    return TRUE;

  return FALSE;
}

/*
 * See if char can dual wield at this time			-Thoric
 */
bool can_dual( CHAR_DATA *ch )
{
   if ( !could_dual(ch) )
     return FALSE;

   if ( get_eq_char( ch, WEAR_DUAL_WIELD ) )
   {
	send_to_char( "> you are already wielding two weapons\n\r", ch );
	return FALSE;
   }
   if ( get_eq_char( ch, WEAR_HOLD ) )
   {
	send_to_char( "> you cannot dual wield while holding something\n\r", ch );
	return FALSE;
   }
   return TRUE;
}


/*
 * Check to see if there is room to wear another object on this location
 * (Layered clothing support)
 */
bool can_layer( CHAR_DATA *ch, OBJ_DATA *obj, sh_int wear_loc )
{
    OBJ_DATA   *otmp;
    sh_int	bitlayers = 0;
    sh_int	objlayers = obj->pIndexData->layers;

    for ( otmp = ch->first_carrying; otmp; otmp = otmp->next_content )
	if ( otmp->wear_loc == wear_loc )
	{
	    if ( !otmp->pIndexData->layers )
		return FALSE;
	    else
		bitlayers |= otmp->pIndexData->layers;
	}
    if ( (bitlayers && !objlayers) || bitlayers > objlayers )
	return FALSE;
    if ( !bitlayers || ((bitlayers & ~objlayers) == bitlayers) )
	return TRUE;
    return FALSE;
}

/*
 * Wear one object.
 * Optional replacement of existing objects.
 * Big repetitive code, ick.
 * Restructured a bit to allow for specifying body location	-Thoric
 */
void wear_obj( CHAR_DATA *ch, OBJ_DATA *obj, bool fReplace, sh_int wear_bit )
{
    char buf[MAX_STRING_LENGTH];
    OBJ_DATA *tmpobj;
    sh_int bit, tmp;

    separate_obj( obj );

    if ( wear_bit > -1 )
    {
	bit = wear_bit;
	if ( !CAN_WEAR(obj, 1 << bit) )
	{
	    if ( fReplace )
	    {
		switch( 1 << bit )
		{
		    case ITEM_HOLD:
			send_to_char( "> you cannot hold that\n\r", ch );
			break;
		    case ITEM_WIELD:
			send_to_char( "> you cannot wield that\n\r", ch );
			break;
		    default:
			sprintf( buf, "> you cannot wear that on your %s\n\r",
				w_flags[bit] );
			send_to_char( buf, ch );
		}
	    }
	    return;
	}
    }
    else
    {
	for ( bit = -1, tmp = 1; tmp < 31; tmp++ )
	{
	    if ( CAN_WEAR(obj, 1 << tmp) )
	    {
		bit = tmp;
		break;
	    }
	}
    }


    /* currently cannot have a light in non-light position */
    if ( obj->item_type == ITEM_LIGHT )
    {
	if ( !remove_obj( ch, WEAR_LIGHT, fReplace ) )
	    return;
	if ( !oprog_use_trigger( ch, obj, NULL, NULL, NULL ) )
        {
          if ( !obj->action_desc || obj->action_desc[0]=='\0' )
  	  {
  	    act( AT_ACTION, "> $n holds $p as a light", ch, obj, NULL, TO_ROOM );
	    act( AT_ACTION, "> you hold $p as your light",  ch, obj, NULL, TO_CHAR );
           }
	   else
	    actiondesc( ch, obj, NULL );
        }
        equip_char( ch, obj, WEAR_LIGHT );
        oprog_wear_trigger( ch, obj );
	return;
    }

    if ( bit == -1 )
    {
	if ( fReplace )
	  send_to_char( "> you cannot wear, wield, or hold that\n\r", ch );
	return;
    }

    switch ( 1 << bit )
    {
	default:
	    bug( "wear_obj: uknown/unused item_wear bit %d", bit );
	    if ( fReplace )
	      send_to_char( "> you cannot wear, wield, or hold that\n\r", ch );
	    return;

	case ITEM_WEAR_FINGER:
	    if ( get_eq_char( ch, WEAR_FINGER_L )
	    &&   get_eq_char( ch, WEAR_FINGER_R )
	    &&   !remove_obj( ch, WEAR_FINGER_L, fReplace )
	    &&   !remove_obj( ch, WEAR_FINGER_R, fReplace ) )
	      return;

	    if ( !get_eq_char( ch, WEAR_FINGER_L ) )
	    {
                if ( !oprog_use_trigger( ch, obj, NULL, NULL, NULL ) )
                {
                 if ( !obj->action_desc || obj->action_desc[0]=='\0' )
                 {
                  act( AT_ACTION, "> $n slips $s left finger into $p",    ch, obj, NULL, TO_ROOM );
		  act( AT_ACTION, "> you slip your left finger into $p",  ch, obj, NULL, TO_CHAR );
                 }
        	 else
	          actiondesc( ch, obj, NULL );
                }
		equip_char( ch, obj, WEAR_FINGER_L );
		oprog_wear_trigger( ch, obj );
		return;
	    }

	    if ( !get_eq_char( ch, WEAR_FINGER_R ) )
	    {
                if ( !oprog_use_trigger( ch, obj, NULL, NULL, NULL ) )
                {
                 if ( !obj->action_desc || obj->action_desc[0]=='\0' )
                 {
      		  act( AT_ACTION, "> $n slips $s right finger into $p",   ch, obj, NULL, TO_ROOM );
		  act( AT_ACTION, "> you slip your right finger into $p", ch, obj, NULL, TO_CHAR );
                 }
        	 else
	          actiondesc( ch, obj, NULL );
                }
		equip_char( ch, obj, WEAR_FINGER_R );
		oprog_wear_trigger( ch, obj );
		return;
	    }

	    bug( "Wear_obj: no free finger", 0 );
	    send_to_char( "> you already wear something on both fingers\n\r", ch );
	    return;

	case ITEM_WEAR_NECK:
	    if ( get_eq_char( ch, WEAR_NECK_1 ) != NULL
	    &&   get_eq_char( ch, WEAR_NECK_2 ) != NULL
	    &&   !remove_obj( ch, WEAR_NECK_1, fReplace )
	    &&   !remove_obj( ch, WEAR_NECK_2, fReplace ) )
	      return;

	    if ( !get_eq_char( ch, WEAR_NECK_1 ) )
	    {
                if ( !oprog_use_trigger( ch, obj, NULL, NULL, NULL ) )
                {
                 if ( !obj->action_desc || obj->action_desc[0]=='\0' )
                 {
      		  act( AT_ACTION, "> $n wears $p around $s neck",   ch, obj, NULL, TO_ROOM );
		  act( AT_ACTION, "> you wear $p around your neck", ch, obj, NULL, TO_CHAR );
                 }
        	 else
	          actiondesc( ch, obj, NULL );
                }
		equip_char( ch, obj, WEAR_NECK_1 );
		oprog_wear_trigger( ch, obj );
		return;
	    }

	    if ( !get_eq_char( ch, WEAR_NECK_2 ) )
	    {
                if ( !oprog_use_trigger( ch, obj, NULL, NULL, NULL ) )
                {
                 if ( !obj->action_desc || obj->action_desc[0]=='\0' )
                 {
      		  act( AT_ACTION, "> $n wears $p around $s neck",   ch, obj, NULL, TO_ROOM );
		  act( AT_ACTION, "> you wear $p around your neck", ch, obj, NULL, TO_CHAR );
                 }
        	 else
	          actiondesc( ch, obj, NULL );
                }
		equip_char( ch, obj, WEAR_NECK_2 );
		oprog_wear_trigger( ch, obj );
		return;
	    }

	    bug( "Wear_obj: no free neck", 0 );
	    send_to_char( "> you already wear two neck items\n\r", ch );
	    return;

	case ITEM_WEAR_BODY:
	/*
	    if ( !remove_obj( ch, WEAR_BODY, fReplace ) )
	      return;
	*/
	    if ( !can_layer( ch, obj, WEAR_BODY ) )
	    {
		send_to_char( "> it will not fit overtop of what you are already wearing\n\r", ch );
		return;
	    }
            if ( !oprog_use_trigger( ch, obj, NULL, NULL, NULL ) )
            {
             if ( !obj->action_desc || obj->action_desc[0]=='\0' )
             {
      	      act( AT_ACTION, "> $n fits $p on $s body",   ch, obj, NULL, TO_ROOM );
	      act( AT_ACTION, "> you fit $p on your body", ch, obj, NULL, TO_CHAR );
             }
             else
	      actiondesc( ch, obj, NULL );
            }
	    equip_char( ch, obj, WEAR_BODY );
	    oprog_wear_trigger( ch, obj );
	    return;

	case ITEM_WEAR_HEAD:
	    if ( !remove_obj( ch, WEAR_HEAD, fReplace ) )
	      return;
            if ( !oprog_use_trigger( ch, obj, NULL, NULL, NULL ) )
            {
             if ( !obj->action_desc || obj->action_desc[0]=='\0' )
             {
      	      act( AT_ACTION, "> $n dons $p upon $s head",   ch, obj, NULL, TO_ROOM );
	      act( AT_ACTION, "> you don $p upon your head", ch, obj, NULL, TO_CHAR );
             }
             else
	      actiondesc( ch, obj, NULL );
            }
	    equip_char( ch, obj, WEAR_HEAD );
	    oprog_wear_trigger( ch, obj );
	    return;

	case ITEM_WEAR_EYES:
	    if ( !remove_obj( ch, WEAR_EYES, fReplace ) )
	      return;
            if ( !oprog_use_trigger( ch, obj, NULL, NULL, NULL ) )
            {
             if ( !obj->action_desc || obj->action_desc[0]=='\0' )
             {
      	      act( AT_ACTION, "> $n places $p on $s eyes",   ch, obj, NULL, TO_ROOM );
	      act( AT_ACTION, "> you place $p on your eyes", ch, obj, NULL, TO_CHAR );
             }
             else
	      actiondesc( ch, obj, NULL );
           }
	    equip_char( ch, obj, WEAR_EYES );
	    oprog_wear_trigger( ch, obj );
	    return;

	case ITEM_WEAR_EARS:
	    if ( !remove_obj( ch, WEAR_EARS, fReplace ) )
	      return;
            if ( !oprog_use_trigger( ch, obj, NULL, NULL, NULL ) )
            {
             if ( !obj->action_desc || obj->action_desc[0]=='\0' )
             {
      	      act( AT_ACTION, "> $n wears $p on $s ears",   ch, obj, NULL, TO_ROOM );
	      act( AT_ACTION, "> you wear $p on your ears", ch, obj, NULL, TO_CHAR );
             }
             else
             actiondesc( ch, obj, NULL );
            }
	    equip_char( ch, obj, WEAR_EARS );
	    oprog_wear_trigger( ch, obj );
	    return;

	case ITEM_WEAR_LEGS:
/*
	    if ( !remove_obj( ch, WEAR_LEGS, fReplace ) )
	      return;
*/
	    if ( !can_layer( ch, obj, WEAR_LEGS ) )
	    {
		send_to_char( "> it will not fit overtop of what you are already wearing\n\r", ch );
		return;
	    }
            if ( !oprog_use_trigger( ch, obj, NULL, NULL, NULL ) )
            {
             if ( !obj->action_desc || obj->action_desc[0]=='\0' )
             {
      	      act( AT_ACTION, "> $n slips into $p",   ch, obj, NULL, TO_ROOM );
	      act( AT_ACTION, "> you slip into $p", ch, obj, NULL, TO_CHAR );
             }
             else
             actiondesc( ch, obj, NULL );
            }
	    equip_char( ch, obj, WEAR_LEGS );
	    oprog_wear_trigger( ch, obj );
	    return;

	case ITEM_WEAR_FEET:
/*
	    if ( !remove_obj( ch, WEAR_FEET, fReplace ) )
	      return;
*/
	    if ( !can_layer( ch, obj, WEAR_FEET ) )
	    {
		send_to_char( "> it will not fit overtop of what you are already wearing\n\r", ch );
		return;
	    }
            if ( !oprog_use_trigger( ch, obj, NULL, NULL, NULL ) )
            {
             if ( !obj->action_desc || obj->action_desc[0]=='\0' )
             {
      	      act( AT_ACTION, "> $n wears $p on $s feet",   ch, obj, NULL, TO_ROOM );
	      act( AT_ACTION, "> you wear $p on your feet", ch, obj, NULL, TO_CHAR );
             }
             else
             actiondesc( ch, obj, NULL );
            }
	    equip_char( ch, obj, WEAR_FEET );
	    oprog_wear_trigger( ch, obj );
	    return;

	case ITEM_WEAR_HANDS:
/*
	    if ( !remove_obj( ch, WEAR_HANDS, fReplace ) )
	      return;
*/
	    if ( !can_layer( ch, obj, WEAR_HANDS ) )
	    {
		send_to_char( "> it will not fit overtop of what you are already wearing\n\r", ch );
		return;
	    }
            if ( !oprog_use_trigger( ch, obj, NULL, NULL, NULL ) )
            {
             if ( !obj->action_desc || obj->action_desc[0]=='\0' )
             {
      	      act( AT_ACTION, "> $n wears $p on $s hands",   ch, obj, NULL, TO_ROOM );
	      act( AT_ACTION, "> you wear $p on your hands", ch, obj, NULL, TO_CHAR );
             }
             else
             actiondesc( ch, obj, NULL );
            }
	    equip_char( ch, obj, WEAR_HANDS );
	    oprog_wear_trigger( ch, obj );
	    return;

	case ITEM_WEAR_ARMS:
/*
	    if ( !remove_obj( ch, WEAR_ARMS, fReplace ) )
	      return;
*/
	    if ( !can_layer( ch, obj, WEAR_ARMS ) )
	    {
		send_to_char( "> it will not fit overtop of what you are already wearing\n\r", ch );
		return;
	    }
	    if ( !oprog_use_trigger( ch, obj, NULL, NULL, NULL ) )
            {
             if ( !obj->action_desc || obj->action_desc[0]=='\0' )
             {
      	      act( AT_ACTION, "> $n wears $p on $s arms",   ch, obj, NULL, TO_ROOM );
	      act( AT_ACTION, "> you wear $p on your arms", ch, obj, NULL, TO_CHAR );
             }
             else
             actiondesc( ch, obj, NULL );
            }
	    equip_char( ch, obj, WEAR_ARMS );
	    oprog_wear_trigger( ch, obj );
	    return;

	case ITEM_WEAR_ABOUT:
	/*
	    if ( !remove_obj( ch, WEAR_ABOUT, fReplace ) )
	      return;
	*/
	    if ( !can_layer( ch, obj, WEAR_ABOUT ) )
	    {
		send_to_char( "> it will not fit overtop of what you are already wearing\n\r", ch );
		return;
	    }
            if ( !oprog_use_trigger( ch, obj, NULL, NULL, NULL ) )
            {
             if ( !obj->action_desc || obj->action_desc[0]=='\0' )
             {
      	      act( AT_ACTION, "> $n wears $p about $s body",   ch, obj, NULL, TO_ROOM );
	      act( AT_ACTION, "> you wear $p about your body", ch, obj, NULL, TO_CHAR );
             }
             else
             actiondesc( ch, obj, NULL );
            }
	    equip_char( ch, obj, WEAR_ABOUT );
	    oprog_wear_trigger( ch, obj );
	    return;

	case ITEM_WEAR_WAIST:
/*
	    if ( !remove_obj( ch, WEAR_WAIST, fReplace ) )
	      return;
*/
	    if ( !can_layer( ch, obj, WEAR_WAIST ) )
	    {
		send_to_char( "> it will not fit overtop of what you are already wearing\n\r", ch );
		return;
	    }
            if ( !oprog_use_trigger( ch, obj, NULL, NULL, NULL ) )
            {
             if ( !obj->action_desc || obj->action_desc[0]=='\0' )
             {
      	      act( AT_ACTION, "> $n wears $p about $s waist",   ch, obj, NULL, TO_ROOM );
	      act( AT_ACTION, "> you wear $p about your waist", ch, obj, NULL, TO_CHAR );
             }
             else
             actiondesc( ch, obj, NULL );
            }
	    equip_char( ch, obj, WEAR_WAIST );
	    oprog_wear_trigger( ch, obj );
	    return;

	case ITEM_WEAR_WRIST:
	    if ( get_eq_char( ch, WEAR_WRIST_L )
	    &&   get_eq_char( ch, WEAR_WRIST_R )
	    &&   !remove_obj( ch, WEAR_WRIST_L, fReplace )
	    &&   !remove_obj( ch, WEAR_WRIST_R, fReplace ) )
	       return;

	    if ( !get_eq_char( ch, WEAR_WRIST_L ) )
	    {
		if ( !oprog_use_trigger( ch, obj, NULL, NULL, NULL ) )
		{
                 if ( !obj->action_desc || obj->action_desc[0]=='\0' )
                 {
      		   act( AT_ACTION, "> $n fits $p around $s left wrist",
			ch, obj, NULL, TO_ROOM );
		   act( AT_ACTION, "> you fit $p around your left wrist",
			ch, obj, NULL, TO_CHAR );
                 }
                 else
                  actiondesc( ch, obj, NULL );
		}
		equip_char( ch, obj, WEAR_WRIST_L );
		oprog_wear_trigger( ch, obj );
		return;
	    }

	    if ( !get_eq_char( ch, WEAR_WRIST_R ) )
	    {
              if ( !oprog_use_trigger( ch, obj, NULL, NULL, NULL ) )
              {
               if ( !obj->action_desc || obj->action_desc[0]=='\0' )
               {
      		act( AT_ACTION, "> $n fits $p around $s right wrist",
			ch, obj, NULL, TO_ROOM );
		act( AT_ACTION, "> you fit $p around your right wrist",
			ch, obj, NULL, TO_CHAR );
               }
               else
                actiondesc( ch, obj, NULL );
              }
		equip_char( ch, obj, WEAR_WRIST_R );
		oprog_wear_trigger( ch, obj );
		return;
	    }

	    bug( "Wear_obj: no free wrist", 0 );
	    send_to_char( "> you already wear two wrist items\n\r", ch );
	    return;

	case ITEM_WEAR_SHIELD:
	    if ( !remove_obj( ch, WEAR_SHIELD, fReplace ) )
	      return;
            if ( !oprog_use_trigger( ch, obj, NULL, NULL, NULL ) )
            {
             if ( !obj->action_desc || obj->action_desc[0]=='\0' )
             {
      	      act( AT_ACTION, "> $n uses $p as an energy shield", ch, obj, NULL, TO_ROOM );
	      act( AT_ACTION, "> you use $p as an energy shield", ch, obj, NULL, TO_CHAR );
             }
             else
              actiondesc( ch, obj, NULL );
            }
	    equip_char( ch, obj, WEAR_SHIELD );
	    oprog_wear_trigger( ch, obj );
	    return;

	case ITEM_WIELD:
	    if ( (tmpobj  = get_eq_char( ch, WEAR_WIELD )) != NULL
	    &&   !could_dual(ch) )
	    {
		send_to_char( "> you are already wielding something\n\r", ch );
		return;
	    }

	    if ( tmpobj )
	    {
		if ( can_dual(ch) )
		{
		    if ( get_obj_weight( obj ) + get_obj_weight( tmpobj ) > str_app[get_curr_str(ch)].wield )
		    {
			send_to_char( "> it is too heavy for you to wield\n\r", ch );
			return;
	      	    }
                    if ( !oprog_use_trigger( ch, obj, NULL, NULL, NULL ) )
                    {
                     if ( !obj->action_desc || obj->action_desc[0]=='\0' )
                     {
      	  	      act( AT_ACTION, "> $n dual-wields $p", ch, obj, NULL, TO_ROOM );
		      act( AT_ACTION, "> you dual-wield $p", ch, obj, NULL, TO_CHAR );
                     }
                     else
                      actiondesc( ch, obj, NULL );
                    }
		    equip_char( ch, obj, WEAR_DUAL_WIELD );
		    oprog_wear_trigger( ch, obj );
		}
	        return;
	    }

	    if ( get_obj_weight( obj ) > str_app[get_curr_str(ch)].wield )
	    {
		send_to_char( "> it is too heavy for you to wield\n\r", ch );
		return;
	    }

            if ( !oprog_use_trigger( ch, obj, NULL, NULL, NULL ) )
            {
             if ( !obj->action_desc || obj->action_desc[0]=='\0' )
             {
      	      act( AT_ACTION, "> $n wields $p", ch, obj, NULL, TO_ROOM );
	      act( AT_ACTION, "> you wield $p", ch, obj, NULL, TO_CHAR );
             }
             else
              actiondesc( ch, obj, NULL );
            }
	    equip_char( ch, obj, WEAR_WIELD );
	    oprog_wear_trigger( ch, obj );
	    return;

	case ITEM_HOLD:
	    if ( get_eq_char( ch, WEAR_DUAL_WIELD ) )
	    {
		send_to_char( "> you cannot hold something AND two weapons\n\r", ch );
		return;
	    }
	    if ( !remove_obj( ch, WEAR_HOLD, fReplace ) )
	      return;
            if ( obj->item_type == ITEM_DEVICE
               || obj->item_type == ITEM_FOOD
               || obj->item_type == ITEM_DRINK_CON
               || !oprog_use_trigger( ch, obj, NULL, NULL, NULL ) )
            {
	      act( AT_ACTION, "> $n holds $p in $s hands",   ch, obj, NULL, TO_ROOM );
	      act( AT_ACTION, "> you hold $p in your hands", ch, obj, NULL, TO_CHAR );
            }
	    equip_char( ch, obj, WEAR_HOLD );
	    oprog_wear_trigger( ch, obj );
	    return;
    }
}


void do_wear( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    sh_int wear_bit;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    if ( (!str_cmp(arg2, "on")  || !str_cmp(arg2, "upon") || !str_cmp(arg2, "around"))
    &&   argument[0] != '\0' )
	argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' )
    {
	send_to_char( "> wear, wield, or hold what\n\r", ch );
	return;
    }

    if ( ms_find_obj(ch) )
	return;

    if ( !str_cmp( arg1, "all" ) )
    {
	OBJ_DATA *obj_next;

	for ( obj = ch->first_carrying; obj; obj = obj_next )
	{
	    obj_next = obj->next_content;
	    if ( obj->wear_loc == WEAR_NONE && can_see_obj( ch, obj ) )
		wear_obj( ch, obj, FALSE, -1 );
	}
	return;
    }
    else
    {
	if ( ( obj = get_obj_carry( ch, arg1 ) ) == NULL )
	{
	    send_to_char( "> you do not have that item\n\r", ch );
	    return;
	}
	if ( arg2[0] != '\0' )
	  wear_bit = get_wflag(arg2);
	else
	  wear_bit = -1;
	wear_obj( ch, obj, TRUE, wear_bit );
    }

    return;
}



void do_remove( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj, *obj_next;


    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "> remove what\n\r", ch );
	return;
    }

    if ( ms_find_obj(ch) )
	return;

   if ( !str_cmp( arg, "all" ) )  /* SB Remove all */
    {
      for ( obj = ch->first_carrying; obj != NULL ; obj = obj_next )
      {
        obj_next = obj->next_content;
        if ( obj->wear_loc != WEAR_NONE && can_see_obj ( ch, obj ) )
          remove_obj ( ch, obj->wear_loc, TRUE );
      }
      return;
    }

    if ( ( obj = get_obj_wear( ch, arg ) ) == NULL )
    {
	send_to_char( "> you are not using that item\n\r", ch );
	return;
    }
    if ( (obj_next=get_eq_char(ch, obj->wear_loc)) != obj )
    {
	act( AT_PLAIN, "> you must remove $p first", ch, obj_next, NULL, TO_CHAR );
	return;
    }

    remove_obj( ch, obj->wear_loc, TRUE );
    return;
}


void do_bury( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    bool shovel;
    sh_int move;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        send_to_char( "> what do you wish to bury\n\r", ch );
        return;
    }

    if ( ms_find_obj(ch) )
        return;

    shovel = FALSE;
    for ( obj = ch->first_carrying; obj; obj = obj->next_content )
      if ( obj->item_type == ITEM_SHOVEL )
      {
	  shovel = TRUE;
	  break;
      }

    obj = get_obj_list_rev( ch, arg, ch->in_room->last_content );
    if ( !obj )
    {
        send_to_char( "> you cannot find it\n\r", ch );
        return;
    }

    separate_obj(obj);
    if ( !CAN_WEAR(obj, ITEM_TAKE) )
    {
		act( AT_PLAIN, "> you cannot bury $p", ch, obj, 0, TO_CHAR );
        	return;
    }

    switch( ch->in_room->sector_type )
    {
	case SECT_CITY:
	case SECT_INSIDE:
	    send_to_char( "> the floor is too hard to dig through\n\r", ch );
	    return;
	case SECT_WATER_SWIM:
	case SECT_WATER_NOSWIM:
	case SECT_UNDERWATER:
	    send_to_char( "> you cannot bury something here\n\r", ch );
	    return;
	case SECT_AIR:
	    send_to_char( "> you cannot do that in the air\n\r", ch );
	    return;
    }

    if ( obj->weight > (UMAX(5, (can_carry_w(ch) / 10)))
    &&  !shovel )
    {
	send_to_char( "> you would need a shovel to bury something that big\n\r", ch );
	return;
    }

    move = (obj->weight * 50 * (shovel ? 1 : 5)) / UMAX(1, can_carry_w(ch));
    move = URANGE( 2, move, 1000 );
    if ( move > ch->move )
    {
	send_to_char( "> you do not have the energy to bury something of that size\n\r", ch );
	return;
    }
    ch->move -= move;

    act( AT_ACTION, "> you solemnly bury $p", ch, obj, NULL, TO_CHAR );
    act( AT_ACTION, "> $n solemnly buries $p", ch, obj, NULL, TO_ROOM );
    SET_BIT( obj->extra_flags, ITEM_BURRIED );
    WAIT_STATE( ch, URANGE( 10, move / 2, 100 ) );
    return;
}

/* put an item on auction, or see the stats on the current item or bet */
void do_auction (CHAR_DATA *ch, char *argument)
{
    OBJ_DATA *obj;
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];

    argument = one_argument (argument, arg1);

    if (IS_NPC(ch)) /* NPC can be extracted at any time and thus cannot auction! */
	return;

    if ( !IS_SET( ch->in_room->room_flags , ROOM_AUCTION ) )
    {
       set_char_color ( AT_LBLUE, ch );
       send_to_char ( "\n\r> you must go to an auction hall to do that\n\r", ch );
       return;
    }

    /*
    if ( ( time_info.hour > 18 || time_info.hour < 9 ) && auction->item == NULL )
    {
        set_char_color ( AT_LBLUE, ch );
        send_to_char ( "\n\r> the auctioneer has retired for the evening\n\r", ch );
        return;
    }
    */

    if (arg1[0] == '\0')
    {
        if (auction->item != NULL)
        {
	    AFFECT_DATA *paf;
  	    obj = auction->item;

            /* show item data here */
            if (auction->bet > 0)
                sprintf (buf, "> current bid on this item is %d credits\n\r",auction->bet);
            else
                sprintf (buf, "> no bids on this item have been received\n\r");
	    set_char_color ( AT_BLUE, ch );
            send_to_char (buf,ch);
/*          spell_identify (0, LEVEL_HERO - 1, ch, auction->item); */

	    sprintf( buf,
		"> object '%s' is %s - special properties: %s %s.\n\r> its weight is %d - value is %d\n\r",
		obj->name,
		aoran( item_type_name( obj ) ),
		extra_bit_name( obj->extra_flags ),
		magic_bit_name( obj->magic_flags ),
		obj->weight,
		obj->cost );
	    set_char_color( AT_LBLUE, ch );
	    send_to_char( buf, ch );

            sprintf( buf, "> worn on: %s\n\r",
                     flag_string(obj->wear_flags -1, w_flags ) );
            send_to_char( buf, ch );

	    set_char_color( AT_BLUE, ch );

	    switch ( obj->item_type )
	    {

		case ITEM_ARMOR:
		  ch_printf( ch, "> current armor class is %d - ( based on current condition )\n\r", obj->value[0] );
	          ch_printf( ch, "> maximum armor class is %d - ( based on top condition )\n\r", obj->value[1] );
		  break;
	    }

	    for ( paf = obj->pIndexData->first_affect; paf; paf = paf->next )
		showaffect( ch, paf );

	    for ( paf = obj->first_affect; paf; paf = paf->next )
		showaffect( ch, paf );
	    if ( ( obj->item_type == ITEM_CONTAINER ) && ( obj->first_content ) )
	    {
		set_char_color( AT_OBJECT, ch );
		send_to_char( "> contents:\n\r", ch );
		show_list_to_char( obj->first_content, ch, TRUE, FALSE );
	    }

	    if (IS_IMMORTAL(ch))
	    {
		sprintf(buf, "> seller: %s -  bidder: %s -  round: %d\n\r",
                        auction->seller->name, auction->buyer->name,
                        (auction->going + 1));
		send_to_char(buf, ch);
		sprintf(buf, "> time left in round: %d\n\r", auction->pulse);
		send_to_char(buf, ch);
	    }
            return;
	}
	else
	{
	    set_char_color ( AT_LBLUE, ch );
	    send_to_char ( "\n\r> there is nothing being auctioned right now -  what would you like to auction\n\r", ch );
	    return;
	}
    }

    if ( IS_IMMORTAL(ch) && !str_cmp(arg1,"stop"))
    {
    if (auction->item == NULL)
    {
        send_to_char ("> there is no auction to stop\n\r",ch);
        return;
    }
    else /* stop the auction */
    {
	set_char_color ( AT_LBLUE, ch );
        sprintf (buf,"> sale of %s has been stopped by an AI",
                        auction->item->short_descr);
        talk_auction (buf);
        obj_to_char (auction->item, auction->seller);
	if ( IS_SET( sysdata.save_flags, SV_AUCTION ) )
	    save_char_obj(auction->seller);
        auction->item = NULL;
        if (auction->buyer != NULL && auction->buyer != auction->seller) /* return money to the buyer */
        {
            auction->buyer->gold += auction->bet;
            send_to_char ("> your money has been returned\n\r",auction->buyer);
        }
        return;
    }
    }

    if (!str_cmp(arg1,"bid") )
    {
    	if (auction->item != NULL)
        {
            int newbet;

	    if ( ch == auction->seller)
	    {
		send_to_char("> you cannot bid on your own item\n\r", ch);
		return;
	    }

            /* make - perhaps - a bet now */
            if (argument[0] == '\0')
            {
                send_to_char ("> bid how much\n\r",ch);
                return;
            }

            newbet = parsebet (auction->bet, argument);
/*	    ch_printf( ch, "Bid: %d\n\r",newbet);	*/

	    if (newbet < auction->starting)
	    {
		send_to_char("> you must place a bid that is higher than the starting bet\n\r", ch);
		return;
	    }

	    /* to avoid slow auction, use a bigger amount than 100 if the bet
 	       is higher up - changed to 10000 for our high economy
            */

            if (newbet < (auction->bet + 500))
            {
                send_to_char ("> you must at least bid 500 credits over the current bid\n\r",ch);
                return;
            }

            if (newbet > ch->gold)
            {
                send_to_char ("> you do not have that much money\n\r",ch);
                return;
            }

	    if (newbet > 2000000000)
	    {
		send_to_char("> you cannot bid over 2 billion credits\n\r", ch);
		return;
	    }

            /* the actual bet is OK! */

            /* return the gold to the last buyer, if one exists */
            if (auction->buyer != NULL && auction->buyer != auction->seller)
                auction->buyer->gold += auction->bet;

            ch->gold -= newbet; /* substract the gold - important :) */
	    if ( IS_SET( sysdata.save_flags, SV_AUCTION ) )
		save_char_obj(ch);
            auction->buyer = ch;
            auction->bet   = newbet;
            auction->going = 0;
            auction->pulse = PULSE_AUCTION; /* start the auction over again */

            sprintf (buf,"> a bid of %d credits has been received on %s\n\r",newbet,auction->item->short_descr);
            talk_auction (buf);
            return;


        }
        else
        {
            send_to_char ("> there is not anything being auctioned right now\n\r",ch);
            return;
        }
    }
/* finally... */
    if ( ms_find_obj(ch) )
	return;

    obj = get_obj_carry (ch, arg1); /* does char have the item ? */

    if (obj == NULL)
    {
        send_to_char ("> you are not carrying that\n\r",ch);
        return;
    }

    if (obj->timer > 0)
    {
	send_to_char ("> you cannot auction objects that are decaying\n\r", ch);
	return;
    }

    argument = one_argument (argument, arg2);

    if (arg2[0] == '\0')
    {
      auction->starting = 0;
      strcpy(arg2, "0");
    }

    if ( !is_number(arg2) )
    {
	send_to_char("> you must input a number at which to start the auction\n\r", ch);
	return;
    }

    if ( atoi(arg2) < 0 )
    {
	send_to_char("> you cannot auction something for less than 0 credits\n\r", ch);
 	return;
    }

    if (auction->item == NULL)
    switch (obj->item_type)
    {

    default:
        act (AT_TELL, "> you cannot auction $Ts",ch, NULL, item_type_name (obj), TO_CHAR);
        return;

/* insert any more item types here... items with a timer MAY NOT BE
   AUCTIONED!
*/
    case ITEM_LIGHT:
    case ITEM_RARE_METAL:
    case ITEM_CRYSTAL:
    case ITEM_FABRIC:
    case ITEM_ARMOR:
	separate_obj(obj);
	obj_from_char (obj);
	if ( IS_SET( sysdata.save_flags, SV_AUCTION ) )
	    save_char_obj(ch);
	auction->item = obj;
	auction->bet = 0;
	auction->buyer = ch;
	auction->seller = ch;
	auction->pulse = PULSE_AUCTION;
	auction->going = 0;
	auction->starting = atoi(arg2);

	if (auction->starting > 0)
	  auction->bet = auction->starting;

	sprintf (buf, "> new item auction: %s at %d credits", obj->short_descr, auction->starting);
	talk_auction (buf);

	return;

    } /* switch */
    else
    {
        act (AT_TELL, "> try again later - $p is being auctioned right now",ch,auction->item,NULL,TO_CHAR);
	WAIT_STATE( ch, 1.5 * PULSE_VIOLENCE );
        return;
    }
}



/* Make objects in rooms that are nofloor fall - Scryn 1/23/96 */

void obj_fall( OBJ_DATA *obj, bool through )
{
    EXIT_DATA *pexit;
    ROOM_INDEX_DATA *to_room;
    static int fall_count;
    char buf[MAX_STRING_LENGTH];
    static bool is_falling; /* Stop loops from the call to obj_to_room()  -- Altrag */

    if ( !obj->in_room || is_falling )
    	return;

    if (fall_count > 30)
    {
    	bug( "object falling in loop more than 30 times", 0 );
	extract_obj(obj);
    	fall_count = 0;
	return;
     }

     if ( IS_SET( obj->in_room->room_flags, ROOM_NOFLOOR )
     &&   CAN_GO( obj, DIR_DOWN )
     &&   !IS_OBJ_STAT( obj, ITEM_MAGIC ) )
     {

	pexit = get_exit( obj->in_room, DIR_DOWN );
    	to_room = pexit->to_room;

    	if (through)
	  fall_count++;
	else
	  fall_count = 0;

	if (obj->in_room == to_room)
	{
	    sprintf(buf, "Object falling into same room, room %ld",
		to_room->vnum);
	    bug( buf, 0 );
	    extract_obj( obj );
            return;
	}

	if (obj->in_room->first_person)
	{
	  	act( AT_PLAIN, "> $p falls far below",
			obj->in_room->first_person, obj, NULL, TO_ROOM );
		act( AT_PLAIN, "> $p falls far below",
			obj->in_room->first_person, obj, NULL, TO_CHAR );
	}
	obj_from_room( obj );
	is_falling = TRUE;
	obj = obj_to_room( obj, to_room );
	is_falling = FALSE;

	if (obj->in_room->first_person)
	{
	  	act( AT_PLAIN, "> $p falls from above",
			obj->in_room->first_person, obj, NULL, TO_ROOM );
		act( AT_PLAIN, "> $p falls from above",
			obj->in_room->first_person, obj, NULL, TO_CHAR );
	}

 	if (!IS_SET( obj->in_room->room_flags, ROOM_NOFLOOR ) && through )
	{
/*		int dam = (int)9.81*sqrt(fall_count*2/9.81)*obj->weight/2;
*/		int dam = fall_count*obj->weight/2;
		/* Damage players */
		if ( obj->in_room->first_person && number_percent() > 15 )
		{
			CHAR_DATA *rch;
			CHAR_DATA *vch = NULL;
			int chcnt = 0;

			for ( rch = obj->in_room->first_person; rch;
				rch = rch->next_in_room, chcnt++ )
				if ( number_range( 0, chcnt ) == 0 )
					vch = rch;
			act( AT_WHITE, "> $p falls on $n", vch, obj, NULL, TO_ROOM );
			act( AT_WHITE, "> $p falls on you", vch, obj, NULL, TO_CHAR );
			damage( vch, vch, dam*vch->top_level, TYPE_UNDEFINED );
		}
    	/* Damage objects */
	    switch( obj->item_type )
     	    {
	     	case ITEM_WEAPON:
		case ITEM_ARMOR:
		    if ( (obj->value[0] - dam) <= 0 )
 		    {
   			if (obj->in_room->first_person)
			{
			act( AT_PLAIN, "> $p is destroyed by the fall",
				obj->in_room->first_person, obj, NULL, TO_ROOM );
			act( AT_PLAIN, "> $p is destroyed by the fall",
				obj->in_room->first_person, obj, NULL, TO_CHAR );
			}
			make_scraps(obj);
	 	    }
		    else
	           	obj->value[0] -= dam;
		    break;
		default:
		    if ( (dam*15) > get_obj_resistance(obj) )
		    {
	              if (obj->in_room->first_person)
		      {
 			    act( AT_PLAIN, "> $p is destroyed by the fall",
			    	obj->in_room->first_person, obj, NULL, TO_ROOM );
			    act( AT_PLAIN, "> $p is destroyed by the fall",
		    		obj->in_room->first_person, obj, NULL, TO_CHAR );
		      }
		      make_scraps(obj);
		    }
		    break;
	    }
     	}
     	obj_fall( obj, TRUE );
    }
    return;
}

bool  job_trigger( CHAR_DATA *victim, CHAR_DATA *ch, OBJ_DATA *obj )
{
     char buf[MAX_STRING_LENGTH];

     if( !IS_NPC( victim ) || !victim->pIndexData )
        return FALSE;

     if ( victim->pIndexData->vnum != MOB_VNUM_JOB_OFFICER )
        return FALSE;

     if( IS_NPC( ch ) || !ch->pcdata )
        return FALSE;

     if ( !obj->pIndexData || obj->pIndexData->vnum != OBJ_VNUM_PACKAGE )
        return FALSE;

     if ( !victim->in_room || !victim->in_room->area || !victim->in_room->area->planet )
        return FALSE;

     sprintf( buf , "package %s" , victim->in_room->area->planet->name );

     if ( str_cmp ( buf , obj->name ) )
        return FALSE;

     do_say( victim, "> thank you" );
     ch->gold += 1000;

     act( AT_GOLD, "> $N gives you 1000 credits" , ch, NULL, victim, TO_CHAR  );
     act( AT_GOLD, "> $N gives $n some credits",  ch, NULL, victim, TO_NOTVICT );
     act( AT_GOLD, "> you give $n some credits",  ch, NULL, victim, TO_VICT  );

     separate_obj( obj );
     obj_from_char( obj );
     extract_obj( obj );
     
     return TRUE;         
}

bool  agent_trigger( CHAR_DATA *victim, CHAR_DATA *ch, OBJ_DATA *obj )
{
     char buf[MAX_STRING_LENGTH];
     PLANET_DATA *planet;

     if( !IS_NPC( victim ) || !victim->pIndexData )
        return FALSE;

     if ( victim->pIndexData->vnum != MOB_VNUM_DATAMINER )
        return FALSE;

     if( IS_NPC( ch ) || !ch->pcdata )
        return FALSE;

     if ( !obj->pIndexData || obj->pIndexData->vnum != 101 )
        return FALSE;

     if ( !victim->in_room || !victim->in_room->area || !victim->in_room->area->planet )
        return FALSE;

     sprintf( buf , "virus %s" , victim->in_room->area->planet->name );

     if ( str_cmp ( buf , obj->name ) )
        return FALSE;

     do_say( victim, "memory corruption detected" );
     ch->gold += 2000;

     planet = ch->in_room->area->planet;


     separate_obj( obj );
     obj_from_char( obj );
     extract_obj( obj );
	send_to_char( "&w> the dataminer program is unloaded from the node\r\n", ch );
	send_to_char( "&w> 2,000 credits received\r\n", ch );
	extract_char( victim, TRUE );
	planet->pop_support -= 1;

    if ( planet->pop_support < -100 )
        planet->pop_support = -100;
     
     return TRUE;         
}

/* Junk command installed by Samson 1-13-98
 * Code courtesy of Stu, from the mailing list. Allows player to destroy an item in their inventory.
 * Debugged and cleaned up on 4-16-01 by Samson.
 */
void do_junk( CHAR_DATA * ch, char *argument )
{
   OBJ_DATA *obj, *obj_next;
   bool found = FALSE;

   if( !argument || argument[0] == '\0' )
   {
      send_to_char( "> delete what\r\n", ch );
      return;
   }

   for( obj = ch->first_carrying; obj; obj = obj_next )
   {
      obj_next = obj->next_content;
      if( ( nifty_is_name( argument, obj->name ) ) && can_see_obj( ch, obj ) && obj->wear_loc == WEAR_NONE )
      {
         found = TRUE;
         break;
      }
   }
   if( found )
   {
      if( !can_drop_obj( ch, obj ) && ch->top_level < 200 )
      {
         send_to_char( "> you cannot delete that - it is corrupted!\r\n", ch );
         return;
      }
      separate_obj( obj );
      obj_from_char( obj );
      extract_obj( obj );
      act( AT_ACTION, "> $n deleted $p", ch, obj, NULL, TO_ROOM );
      act( AT_ACTION, "> you junk $p", ch, obj, NULL, TO_CHAR );
   }
   return;
}

// done for Neuro
