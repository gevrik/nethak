#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "mud.h"

extern int	top_exit;

void do_buyhome( CHAR_DATA *ch, char *argument )
{
	ROOM_INDEX_DATA *room;
	AREA_DATA *pArea;
	PLANET_DATA *planet;
	int cost = 0;

	if ( !ch->in_room )
		return;

	if ( !ch->in_room->area )
	{
		send_to_char( "> &Rthis is not a safe place to live&w\n\r", ch);
		return;
	}

	
	planet = ch->in_room->area->planet;
	if ( !IS_SET( planet->flags, PLANET_NOCAP ) )
	{

		if ( !ch->pcdata->clan )
		{
			send_to_char( "> &Ryou do not belong to an organization&w\n\r", ch );
			return;
		}

	}
	else
	{
		if (  ch->pcdata->clan )
		{
			send_to_char( "> &Ryou cannot buy a home node in this system&w\n\r", ch );
			return;
		}
	}
	
	if ( IS_NPC(ch) || !ch->pcdata )
		return;

	if ( ch->plr_home != NULL )
	{
		//send_to_char( "&R> you already have a home\n\r&w", ch);
		//return;
		
		cost = 10000;

		ROOM_INDEX_DATA *rooma = ch->plr_home;

		STRFREE( rooma->name );
		rooma->name = STRALLOC( "unusedhome" );

		REMOVE_BIT( rooma->room_flags , ROOM_PLR_HOME );
		REMOVE_BIT( rooma->room_flags , ROOM_NOPEDIT );
		SET_BIT( rooma->room_flags , ROOM_EMPTY_HOME );
		SET_BIT( rooma->room_flags , ROOM_SAFE );

		STRFREE( rooma->description );
		rooma->description = STRALLOC( "use BUYHOME to buy this node for 10.000 credits. if you do not have a home node yet it will be free." );

		if ( rooma->area )
			fold_area( rooma->area, rooma->area->filename, FALSE );

	}

	room = ch->in_room;

	for ( pArea = first_bsort; pArea; pArea = pArea->next_sort )
	{
		if ( room->area == pArea )
		{
			send_to_char( "&R> this area is not installed yet\n\r&w", ch);
			return;
		}
	}

	if ( !IS_SET( room->room_flags , ROOM_EMPTY_HOME ) )
	{
		send_to_char( "&R> this room is not for sale\n\r&w", ch);
		return;
	}

	if ( ch->gold < cost )
	{
		send_to_char( "&R> this room costs 10.000 credits\n\r&w", ch);
		return;
	}

	if ( argument[0] == '\0' )
	{
		send_to_char( "> set the node name - a single-line node description\n\r", ch );
		send_to_char( "> syntax: buyhome <node name>\n\r", ch );
		send_to_char( "> cost: 10,000c if you do have a home already\n\r", ch );
		return;
	}

	STRFREE( room->name );
	room->name = STRALLOC( argument );

	STRFREE( room->description );
	room->description = STRALLOC( "home, sweet home." ); //

	ch->gold -= cost;

	REMOVE_BIT( room->room_flags , ROOM_EMPTY_HOME );
	SET_BIT( room->room_flags , ROOM_PLR_HOME );
	SET_BIT( room->room_flags , ROOM_NOPEDIT );

	fold_area( room->area, room->area->filename, FALSE );

	ch->plr_home = room;
	do_save( ch , "" );

}

void do_ammo( CHAR_DATA *ch, char *argument )
{
	OBJ_DATA *wield;
	OBJ_DATA *obj;
	bool checkammo = FALSE;
	int charge =0;

	obj = NULL;
	wield = get_eq_char( ch, WEAR_WIELD );
	if (wield)
	{
		obj = get_eq_char( ch, WEAR_DUAL_WIELD );
		if (!obj)
			obj = get_eq_char( ch, WEAR_HOLD );
	}
	else
	{
		wield = get_eq_char( ch, WEAR_HOLD );
		obj = NULL;
	}

	if (!wield || wield->item_type != ITEM_WEAPON )
	{
		send_to_char( "&R> you do not seem to be holding a weapon\n\r&w", ch);
		return;
	}

	if ( wield->value[3] == WEAPON_BLASTER )
	{

		if ( obj && obj->item_type != ITEM_AMMO )
		{
			send_to_char( "&R> your hands are too full to reload your blaster\n\r&w", ch);
			return;
		}

		if (obj)
		{
			if ( obj->value[0] > wield->value[5] )
			{
				send_to_char( "> that patch is not suitable for your blaster", ch);
				return;
			}
			unequip_char( ch, obj );
			checkammo = TRUE;
			charge = obj->value[0];
			separate_obj( obj );
			extract_obj( obj );
		}
		else
		{
			for ( obj = ch->last_carrying; obj; obj = obj->prev_content )
			{
				if ( obj->item_type == ITEM_AMMO)
				{
					if ( obj->value[0] > wield->value[5] )
					{
						send_to_char( "> that patch is not suitable for your blaster", ch);
						continue;
					}
					checkammo = TRUE;
					charge = obj->value[0];
					separate_obj( obj );
					extract_obj( obj );
					break;
				}
			}
		}

		if (!checkammo)
		{
			send_to_char( "&R> you do not seem to have any patch to reload your blaster with\n\r&w", ch);
			return;
		}

		ch_printf( ch, "> you patch your blaster\n\r> your blaster is charged with %d shots at high power to %d shots on low\n\r", charge/5, charge );
		act( AT_PLAIN, "> $n patches their $p", ch, wield, NULL, TO_ROOM );

	}
	else
	{

		if ( obj && obj->item_type != ITEM_BATTERY )
		{
			send_to_char( "&R> your hands are too full to patch that\n\r&w", ch);
			return;
		}

		if (obj)
		{
			unequip_char( ch, obj );
			checkammo = TRUE;
			charge = obj->value[0];
			separate_obj( obj );
			extract_obj( obj );
		}
		else
		{
			for ( obj = ch->last_carrying; obj; obj = obj->prev_content )
			{
				if ( obj->item_type == ITEM_BATTERY)
				{
					checkammo = TRUE;
					charge = obj->value[0];
					separate_obj( obj );
					extract_obj( obj );
					break;
				}
			}
		}

		if (!checkammo)
		{
			send_to_char( "&R> you do not seem to have a suitable patch\n\r&w", ch);
			return;
		}

		if (wield->value[3] == WEAPON_LIGHTSABER )
		{
			ch_printf( ch, "> you replace your power cell\n\rYour lightsaber is charged to %d/%d units\n\r", charge, charge );
			act( AT_PLAIN, "> $n replaces the power cell in $p", ch, wield, NULL, TO_ROOM );
			act( AT_PLAIN, "> $p ignites with a bright glow", ch, wield, NULL, TO_ROOM );
		}
		else if (wield->value[3] == WEAPON_VIBRO_BLADE )
		{
			ch_printf( ch, "> you patch your blade\n\r> your blade module is charged to %d/%d units\n\r", charge, charge );
			act( AT_PLAIN, "> $n patches their $p", ch, wield, NULL, TO_ROOM );
		}
		else
		{
			ch_printf( ch, "> you feel very foolish\n\r" );
			act( AT_PLAIN, "> $n tries to patch their $p", ch, wield, NULL, TO_ROOM );
		}
	}

	wield->value[4] = charge;

}

void do_setblaster( CHAR_DATA *ch, char *argument )
{
	OBJ_DATA *wield;
	OBJ_DATA *wield2;

	wield = get_eq_char( ch, WEAR_WIELD );
	if( wield && !( wield->item_type == ITEM_WEAPON && wield->value[3] == WEAPON_BLASTER ) )
		wield = NULL;
	wield2 = get_eq_char( ch, WEAR_DUAL_WIELD );
	if( wield2 && !( wield2->item_type == ITEM_WEAPON && wield2->value[3] == WEAPON_BLASTER ) )
		wield2 = NULL;

	if ( !wield && !wield2 )
	{
		send_to_char( "&R> you do not seem to be wielding a blaster\n\r&w", ch);
		return;
	}

	if ( argument[0] == '\0' )
	{
		send_to_char( "&R> syntax: setblaster <full|high|normal|half|low|stun>\n\r&w", ch);
		return;
	}

	if ( wield )
		act( AT_PLAIN, "> $n adjusts the settings on $p", ch, wield, NULL, TO_ROOM );

	if ( wield2 )
		act( AT_PLAIN, "> $n adjusts the settings on $p", ch, wield2, NULL, TO_ROOM );

	if ( !str_cmp( argument, "full" ) )
	{
		if (wield)
		{
			wield->blaster_setting = BLASTER_FULL;
			send_to_char( "&Y> wielded blaster set to full power\n\r&w", ch);
		}
		if (wield2)
		{
			wield2->blaster_setting = BLASTER_FULL;
			send_to_char( "&Y> dualwielded blaster set to full power\n\r&w", ch);
		}
		return;
	}
	if ( !str_cmp( argument, "high" ) )
	{
		if (wield)
		{
			wield->blaster_setting = BLASTER_HIGH;
			send_to_char( "&Y> wielded blaster set to high power\n\r&w", ch);
		}
		if (wield2)
		{
			wield2->blaster_setting = BLASTER_HIGH;
			send_to_char( "&Y> dualwielded blaster set to high power\n\r&w", ch);
		}
		return;
	}
	if ( !str_cmp( argument, "normal" ) )
	{
		if (wield)
		{
			wield->blaster_setting = BLASTER_NORMAL;
			send_to_char( "&Y> wielded blaster set to normal power\n\r&w", ch);
		}
		if (wield2)
		{
			wield2->blaster_setting = BLASTER_NORMAL;
			send_to_char( "&Y> dualwielded blaster set to normal power\n\r&w", ch);
		}
		return;
	}
	if ( !str_cmp( argument, "half" ) )
	{
		if (wield)
		{
			wield->blaster_setting = BLASTER_HALF;
			send_to_char( "&Y> wielded blaster set to half power\n\r&w", ch);
		}
		if (wield2)
		{
			wield2->blaster_setting = BLASTER_HALF;
			send_to_char( "&Y> dualwielded blaster set to half power\n\r&w", ch);
		}
		return;
	}
	if ( !str_cmp( argument, "low" ) )
	{
		if (wield)
		{
			wield->blaster_setting = BLASTER_LOW;
			send_to_char( "&Y> wielded blaster set to low power\n\r&w", ch);
		}
		if (wield2)
		{
			wield2->blaster_setting = BLASTER_LOW;
			send_to_char( "&Y> dualwielded blaster set to low power\n\r&w", ch);
		}
		return;
	}
	if ( !str_cmp( argument, "stun" ) )
	{
		if (wield)
		{
			wield->blaster_setting = BLASTER_STUN;
			send_to_char( "&Y> wielded blaster set to stun\n\r&w", ch);
		}
		if (wield2)
		{
			wield2->blaster_setting = BLASTER_STUN;
			send_to_char( "&Y> dualwielded blaster set to stun\n\r&w", ch);
		}
		return;
	}
	else
		do_setblaster( ch , "" );

}

void do_use( CHAR_DATA *ch, char *argument )
{
	char arg[MAX_INPUT_LENGTH];
	char argd[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	OBJ_DATA *device;
	OBJ_DATA *obj;
	ch_ret retcode;

	argument = one_argument( argument, argd );
	argument = one_argument( argument, arg );

	if ( !str_cmp( arg , "on" ) )
		argument = one_argument( argument, arg );

	if ( argd[0] == '\0' )
	{
		send_to_char( "> use what\n\r", ch );
		return;
	}

	if ( ( device = get_eq_char( ch, WEAR_HOLD ) ) == NULL ||
			!nifty_is_name(argd, device->name) )
	{
		return;
	}

	if ( device->item_type != ITEM_DEVICE )
	{
		send_to_char( "> you cannot figure out what it is your supposed to do with it\n\r", ch );
		return;
	}

	if ( device->value[2] <= 0 )
	{
		send_to_char( "> it has no more charge left", ch);
		return;
	}

	obj = NULL;
	if ( arg[0] == '\0' )
	{
		if ( ch->fighting )
		{
			victim = who_fighting( ch );
		}
		else
		{
			send_to_char( "> use on whom or what\n\r", ch );
			return;
		}
	}
	else
	{
		if ( ( victim = get_char_room ( ch, arg ) ) == NULL
				&&   ( obj    = get_obj_here  ( ch, arg ) ) == NULL )
		{
			send_to_char( "> you cannot find your target\n\r", ch );
			return;
		}
	}

	WAIT_STATE( ch, 1 * PULSE_VIOLENCE );

	if ( device->value[2] > 0 )
	{
		device->value[2]--;
		if ( victim )
		{
			if ( !oprog_use_trigger( ch, device, victim, NULL, NULL ) )
			{
				act( AT_MAGIC, "> $n uses $p on $N", ch, device, victim, TO_ROOM );
				act( AT_MAGIC, "> you use $p on $N", ch, device, victim, TO_CHAR );
			}
		}
		else
		{
			if ( !oprog_use_trigger( ch, device, NULL, obj, NULL ) )
			{
				act( AT_MAGIC, "> $n uses $p on $P", ch, device, obj, TO_ROOM );
				act( AT_MAGIC, "> you use $p on $P", ch, device, obj, TO_CHAR );
			}
		}

		retcode = obj_cast_spell( device->value[3], device->value[0], ch, victim, obj );
		if ( retcode == rCHAR_DIED || retcode == rBOTH_DIED )
		{
			bug( "do_use: char died", 0 );
			return;
		}
	}


	return;
}

/*
 * Fill a container
 * Many enhancements added by Thoric (ie: filling non-drink containers)
 */
void do_fill( CHAR_DATA *ch, char *argument )
{
	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	OBJ_DATA *obj;
	OBJ_DATA *source;
	sh_int    dest_item, src_item1, src_item2, src_item3, src_item4;
	int       diff;
	bool      all = FALSE;

	argument = one_argument( argument, arg1 );
	argument = one_argument( argument, arg2 );

	/* munch optional words */
	if ( (!str_cmp( arg2, "from" ) || !str_cmp( arg2, "with" ))
			&&    argument[0] != '\0' )
		argument = one_argument( argument, arg2 );

	if ( arg1[0] == '\0' )
	{
		send_to_char( "> fill what\n\r", ch );
		return;
	}

	if ( ms_find_obj(ch) )
		return;

	if ( ( obj = get_obj_carry( ch, arg1 ) ) == NULL )
	{
		send_to_char( "> you do not have that item\n\r", ch );
		return;
	}
	else
		dest_item = obj->item_type;

	src_item1 = src_item2 = src_item3 = src_item4 = -1;
	switch( dest_item )
	{
	default:
		act( AT_ACTION, "> $n tries to fill $p", ch, obj, NULL, TO_ROOM );
		send_to_char( "> you cannot fill that\n\r", ch );
		return;
		/* place all fillable item types here */
	case ITEM_DRINK_CON:
		src_item1 = ITEM_FOUNTAIN;
	case ITEM_CONTAINER:
		src_item1 = ITEM_CONTAINER;	src_item2 = ITEM_CORPSE_NPC;
		src_item3 = ITEM_CORPSE_PC;	src_item4 = ITEM_CORPSE_NPC;    break;
	}

	if ( dest_item == ITEM_CONTAINER )
	{
		if ( IS_SET(obj->value[1], CONT_CLOSED) )
		{
			act( AT_PLAIN, "> the $d is closed", ch, NULL, obj->name, TO_CHAR );
			return;
		}
		if ( get_obj_weight( obj ) / obj->count
				>=   obj->value[0] )
		{
			send_to_char( "> it is already full as it can be\n\r", ch );
			return;
		}
	}
	else
	{
		diff = obj->value[0] - obj->value[1];
		if ( diff < 1 || obj->value[1] >= obj->value[0] )
		{
			send_to_char( "> it is already full as it can be\n\r", ch );
			return;
		}
	}

	if ( arg2[0] != '\0' )
	{
		if ( dest_item == ITEM_CONTAINER
				&& (!str_cmp( arg2, "all" ) || !str_prefix( "all", arg2 )) )
		{
			all = TRUE;
			source = NULL;
		}
		else
		{
			if ( ( source =  get_obj_here( ch, arg2 ) ) == NULL )
			{
				send_to_char( "> you cannot find that item\n\r", ch );
				return;
			}
		}
	}
	else
		source = NULL;

	if ( !source )
	{
		bool      found = FALSE;
		OBJ_DATA *src_next;

		found = FALSE;
		separate_obj( obj );
		for ( source = ch->in_room->first_content;
				source;
				source = src_next )
		{
			src_next = source->next_content;
			if (dest_item == ITEM_CONTAINER)
			{
				if ( !CAN_WEAR(source, ITEM_TAKE)
						||   (IS_OBJ_STAT( source, ITEM_PROTOTYPE) && !can_take_proto(ch))
						||    ch->carry_weight + get_obj_weight(source) > can_carry_w(ch)
						||   (get_obj_weight(source) + get_obj_weight(obj)/obj->count)
						> obj->value[0] )
					continue;
				if ( all && arg2[3] == '.'
						&&  !nifty_is_name( &arg2[4], source->name ) )
					continue;
				obj_from_room(source);
				if ( source->item_type == ITEM_MONEY )
				{
					ch->gold += source->value[0];
					extract_obj( source );
				}
				else
					obj_to_obj(source, obj);
				found = TRUE;
			}
			else
				if (source->item_type == src_item1
						||  source->item_type == src_item2
						||  source->item_type == src_item3
						||  source->item_type == src_item4 )
				{
					found = TRUE;
					break;
				}
		}
		if ( !found )
		{
			switch( src_item1 )
			{
			default:
				send_to_char( "> there is nothing appropriate here\n\r", ch );
				return;
			case ITEM_FOUNTAIN:
				send_to_char( "> there is no fountain or pool here\n\r", ch );
				return;
			}
		}
		if (dest_item == ITEM_CONTAINER)
		{
			act( AT_ACTION, "> you fill $p", ch, obj, NULL, TO_CHAR );
			act( AT_ACTION, "> $n fills $p", ch, obj, NULL, TO_ROOM );
			return;
		}
	}

	if (dest_item == ITEM_CONTAINER)
	{
		OBJ_DATA *otmp, *otmp_next;
		char name[MAX_INPUT_LENGTH];
		CHAR_DATA *gch;
		char *pd;
		bool found = FALSE;

		if ( source == obj )
		{
			send_to_char( "> you cannot fill something with itself\n\r", ch );
			return;
		}

		switch( source->item_type )
		{
		default:	/* put something in container */
			if ( !source->in_room	/* disallow inventory items */
					||   !CAN_WEAR(source, ITEM_TAKE)
					||   (IS_OBJ_STAT( source, ITEM_PROTOTYPE) && !can_take_proto(ch))
					||    ch->carry_weight + get_obj_weight(source) > can_carry_w(ch)
					||   (get_obj_weight(source) + get_obj_weight(obj)/obj->count)
					> obj->value[0] )
			{
				send_to_char( "> invalid command\n\r", ch );
				return;
			}
			separate_obj( obj );
			act( AT_ACTION, "> you take $P and put it inside $p", ch, obj, source, TO_CHAR );
			act( AT_ACTION, "> $n takes $P and puts it inside $p", ch, obj, source, TO_ROOM );
			obj_from_room(source);
			obj_to_obj(source, obj);
			break;
		case ITEM_MONEY:
			send_to_char( "> invalid command\n\r", ch );
			break;
		case ITEM_CORPSE_PC:
			if ( IS_NPC(ch) )
			{
				send_to_char( "> invalid command\n\r", ch );
				return;
			}

			pd = source->short_descr;
			pd = one_argument( pd, name );
			pd = one_argument( pd, name );
			pd = one_argument( pd, name );
			pd = one_argument( pd, name );

			if ( str_cmp( name, ch->name ) && !IS_IMMORTAL(ch) )
			{
				bool fGroup;

				fGroup = FALSE;
				for ( gch = first_char; gch; gch = gch->next )
				{
					if ( !IS_NPC(gch)
							&&   is_same_group( ch, gch )
							&&   !str_cmp( name, gch->name ) )
					{
						fGroup = TRUE;
						break;
					}
				}
				if ( !fGroup )
				{
					send_to_char( "> that is someone else's corpse\n\r", ch );
					return;
				}
			}

		case ITEM_CONTAINER:
			if ( source->item_type == ITEM_CONTAINER  /* do not remove */
					&&   IS_SET(source->value[1], CONT_CLOSED) )
			{
				act( AT_PLAIN, "> the $d is closed", ch, NULL, source->name, TO_CHAR );
				return;
			}
		case ITEM_DROID_CORPSE:
		case ITEM_CORPSE_NPC:
			if ( (otmp=source->first_content) == NULL )
			{
				send_to_char( "> it is empty\n\r", ch );
				return;
			}
			separate_obj( obj );
			for ( ; otmp; otmp = otmp_next )
			{
				otmp_next = otmp->next_content;

				if ( !CAN_WEAR(otmp, ITEM_TAKE)
						||   (IS_OBJ_STAT( otmp, ITEM_PROTOTYPE) && !can_take_proto(ch))
						||    ch->carry_number + otmp->count > can_carry_n(ch)
						||    ch->carry_weight + get_obj_weight(otmp) > can_carry_w(ch)
						||   (get_obj_weight(source) + get_obj_weight(obj)/obj->count)
						> obj->value[0] )
					continue;
				obj_from_obj(otmp);
				obj_to_obj(otmp, obj);
				found = TRUE;
			}
			if ( found )
			{
				act( AT_ACTION, "> you fill $p from $P", ch, obj, source, TO_CHAR );
				act( AT_ACTION, "> $n fills $p from $P", ch, obj, source, TO_ROOM );
			}
			else
				send_to_char( "> there is nothing appropriate in there\n\r", ch );
			break;
		}
		return;
	}

	if ( source->value[1] < 1 )
	{
		send_to_char( "> there's none left\n\r", ch );
		return;
	}
	if ( source->count > 1 && source->item_type != ITEM_FOUNTAIN )
		separate_obj( source );
	separate_obj( obj );

	switch( source->item_type )
	{
	default:
		bug( "do_fill: got bad item type: %d", source->item_type );
		send_to_char( "> something went wrong\n\r", ch );
		return;
	case ITEM_FOUNTAIN:
		if ( obj->value[1] != 0 && obj->value[2] != 0 )
		{
			send_to_char( "> there is already another liquid in it\n\r", ch );
			return;
		}
		obj->value[2] = 0;
		obj->value[1] = obj->value[0];
		act( AT_ACTION, "> you fill $p from $P", ch, obj, source, TO_CHAR );
		act( AT_ACTION, "> $n fills $p from $P", ch, obj, source, TO_ROOM );
		return;
	case ITEM_DRINK_CON:
		if ( obj->value[1] != 0 && obj->value[2] != source->value[2] )
		{
			send_to_char( "> there is already another liquid in it\n\r", ch );
			return;
		}
		obj->value[2] = source->value[2];
		if ( source->value[1] < diff )
			diff = source->value[1];
		obj->value[1] += diff;
		source->value[1] -= diff;
		act( AT_ACTION, "> you fill $p from $P", ch, obj, source, TO_CHAR );
		act( AT_ACTION, "> $n fills $p from $P", ch, obj, source, TO_ROOM );
		return;
	}
}

void do_drink( CHAR_DATA *ch, char *argument )
{
	char arg[MAX_INPUT_LENGTH];
	OBJ_DATA *obj;
	int amount;
	int liquid;

	argument = one_argument( argument, arg );
	/* munch optional words */
	if ( !str_cmp( arg, "from" ) && argument[0] != '\0' )
		argument = one_argument( argument, arg );

	if ( arg[0] == '\0' )
	{
		for ( obj = ch->in_room->first_content; obj; obj = obj->next_content )
			if ( (obj->item_type == ITEM_FOUNTAIN) )
				break;

		if ( !obj )
		{
			send_to_char( "> drink what\n\r", ch );
			return;
		}
	}
	else
	{
		if ( ( obj = get_obj_here( ch, arg ) ) == NULL )
		{
			send_to_char( "> you cannot find it\n\r", ch );
			return;
		}
	}

	if ( obj->count > 1 && obj->item_type != ITEM_FOUNTAIN )
		separate_obj(obj);

	if ( !IS_NPC(ch) && ch->pcdata->condition[COND_DRUNK] > 40 )
	{
		send_to_char( "> you fail to reach your mouth\n\r", ch );
		return;
	}

	switch ( obj->item_type )
	{
	default:
		if ( obj->carried_by == ch )
		{
			act( AT_ACTION, "> $n lifts $p up to $s mouth and tries to drink from it", ch, obj, NULL, TO_ROOM );
			act( AT_ACTION, "> you bring $p up to your mouth and try to drink from it", ch, obj, NULL, TO_CHAR );
		}
		else
		{
			act( AT_ACTION, "> $n gets down and tries to drink from $p... (is $e feeling ok?)", ch, obj, NULL, TO_ROOM );
			act( AT_ACTION, "> you get down on the ground and try to drink from $p", ch, obj, NULL, TO_CHAR );
		}
		break;

	case ITEM_FOUNTAIN:
		if ( !oprog_use_trigger( ch, obj, NULL, NULL, NULL ) )
		{
			act( AT_ACTION, "> $n drinks from the fountain", ch, NULL, NULL, TO_ROOM );
			send_to_char( "> you take a long thirst quenching drink\n\r", ch );
		}

		if ( !IS_NPC(ch) )
			ch->pcdata->condition[COND_THIRST] = 40;
		break;

	case ITEM_DRINK_CON:
		if ( obj->value[1] <= 0 )
		{
			send_to_char( "> it is already empty\n\r", ch );
			return;
		}

		if ( ( liquid = obj->value[2] ) >= LIQ_MAX )
		{
			bug( "Do_drink: bad liquid number %d", liquid );
			liquid = obj->value[2] = 0;
		}

		if ( !oprog_use_trigger( ch, obj, NULL, NULL, NULL ) )
		{
			act( AT_ACTION, "> $n drinks $T from $p",
					ch, obj, liq_table[liquid].liq_name, TO_ROOM );
			act( AT_ACTION, "> you drink $T from $p",
					ch, obj, liq_table[liquid].liq_name, TO_CHAR );
		}

		amount = 1; /* UMIN(amount, obj->value[1]); */
		/* what was this? concentrated drinks?  concentrated water
	   too I suppose... sheesh! */

		gain_condition( ch, COND_DRUNK,
				amount * liq_table[liquid].liq_affect[COND_DRUNK  ] );
		gain_condition( ch, COND_FULL,
				amount * liq_table[liquid].liq_affect[COND_FULL   ] );
		gain_condition( ch, COND_THIRST,
				amount * liq_table[liquid].liq_affect[COND_THIRST ] );

		if ( !IS_NPC(ch) )
		{
			if ( ch->pcdata->condition[COND_DRUNK]  > 24 )
				send_to_char( "> you feel quite sloshed\n\r", ch );
			else
				if ( ch->pcdata->condition[COND_DRUNK]  > 18 )
					send_to_char( "> you feel very drunk\n\r", ch );
				else
					if ( ch->pcdata->condition[COND_DRUNK]  > 12 )
						send_to_char( "> you feel drunk\n\r", ch );
					else
						if ( ch->pcdata->condition[COND_DRUNK]  > 8 )
							send_to_char( "> you feel a little drunk\n\r", ch );
						else
							if ( ch->pcdata->condition[COND_DRUNK]  > 5 )
								send_to_char( "> you feel light headed\n\r", ch );

			if ( ch->pcdata->condition[COND_FULL]   > 40 )
				send_to_char( "> you are full\n\r", ch );

			if ( ch->pcdata->condition[COND_THIRST] > 40 )
				send_to_char( "> you feel bloated\n\r", ch );
			else
				if ( ch->pcdata->condition[COND_THIRST] > 36 )
					send_to_char( "> your stomach is sloshing around\n\r", ch );
				else
					if ( ch->pcdata->condition[COND_THIRST] > 30 )
						send_to_char( "> you do not feel thirsty\n\r", ch );
		}

		if ( obj->value[3] )
		{
			/* The drink was poisoned! */
			AFFECT_DATA af;

			act( AT_POISON, "> $n sputters and gags", ch, NULL, NULL, TO_ROOM );
			act( AT_POISON, "> you sputter and gag", ch, NULL, NULL, TO_CHAR );
			ch->mental_state = URANGE( 20, ch->mental_state + 5, 100 );
			af.type      = gsn_poison;
			af.duration  = 3 * obj->value[3];
			af.location  = APPLY_NONE;
			af.modifier  = 0;
			af.bitvector = AFF_POISON;
			affect_join( ch, &af );
		}

		obj->value[1] -= amount;
		break;
	}
	WAIT_STATE(ch, PULSE_PER_SECOND );
	return;
}

void do_eat( CHAR_DATA *ch, char *argument )
{
	OBJ_DATA *obj;
	int foodcond;

	if ( argument[0] == '\0' )
	{
		send_to_char( "> eat what\n\r", ch );
		return;
	}

	if ( IS_NPC(ch) || ch->pcdata->condition[COND_FULL] > 5 )
		if ( ms_find_obj(ch) )
			return;

	if ( (obj = find_obj(ch, argument, TRUE)) == NULL )
		return;

	if ( !IS_IMMORTAL(ch) )
	{
		if ( obj->item_type != ITEM_FOOD  )
		{
			act( AT_ACTION, "> $n starts to nibble on $p ($e must really be hungry)",  ch, obj, NULL, TO_ROOM );
			act( AT_ACTION, "> you try to nibble on $p", ch, obj, NULL, TO_CHAR );
			return;
		}

		if ( !IS_NPC(ch) && !IS_SET( ch->pcdata->cyber, CYBER_REACTOR ) && ch->pcdata->condition[COND_FULL] > 40 )
		{
			send_to_char( "> you are too full to eat more\n\r", ch );
			return;
		}
	}

	/* required due to object grouping */
	separate_obj( obj );

	WAIT_STATE( ch, PULSE_PER_SECOND/2 );

	if ( obj->in_obj )
	{
		act( AT_PLAIN, "> you take $p from $P", ch, obj, obj->in_obj, TO_CHAR );
		act( AT_PLAIN, "> $n takes $p from $P", ch, obj, obj->in_obj, TO_ROOM );
	}
	if ( !oprog_use_trigger( ch, obj, NULL, NULL, NULL ) )
	{
		if ( !obj->action_desc || obj->action_desc[0]=='\0' )
		{
			act( AT_ACTION, "> $n eats $p",  ch, obj, NULL, TO_ROOM );
			act( AT_ACTION, "> you eat $p", ch, obj, NULL, TO_CHAR );
		}
		else
			actiondesc( ch, obj, NULL );
	}

	switch ( obj->item_type )
	{

	case ITEM_FOOD:
		if ( obj->timer > 0 && obj->value[1] > 0 )
			foodcond = (obj->timer * 10) / obj->value[1];
		else
			foodcond = 10;

		if ( !IS_NPC(ch) )
		{
			int condition;

			condition = ch->pcdata->condition[COND_FULL];
			gain_condition( ch, COND_FULL, (obj->value[0] * foodcond) / 10 );
			if ( condition <= 1 && ch->pcdata->condition[COND_FULL] > 1 )
				send_to_char( "> you are no longer hungry\n\r", ch );
			else if ( ch->pcdata->condition[COND_FULL] > 40 )
				send_to_char( "> you are full\n\r", ch );
		}

		if (  obj->value[3] != 0
				||   (foodcond < 4 && number_range( 0, foodcond + 1 ) == 0) )
		{
			/* The food was poisoned! */
			AFFECT_DATA af;

			if ( obj->value[3] != 0 )
			{
				act( AT_POISON, "> $n chokes and gags", ch, NULL, NULL, TO_ROOM );
				act( AT_POISON, "> you choke and gag", ch, NULL, NULL, TO_CHAR );
				ch->mental_state = URANGE( 20, ch->mental_state + 5, 100 );
			}
			else
			{
				act( AT_POISON, "> $n gags on $p", ch, obj, NULL, TO_ROOM );
				act( AT_POISON, "> you gag on $p", ch, obj, NULL, TO_CHAR );
				ch->mental_state = URANGE( 15, ch->mental_state + 5, 100 );
			}

			af.type      = gsn_poison;
			af.duration  = 2 * obj->value[0]
			                              * (obj->value[3] > 0 ? obj->value[3] : 1);
			af.location  = APPLY_NONE;
			af.modifier  = 0;
			af.bitvector = AFF_POISON;
			affect_join( ch, &af );
		}
		break;

	}

	if ( obj->serial == cur_obj )
		global_objcode = rOBJ_EATEN;
	extract_obj( obj );
	return;
}

void do_empty( CHAR_DATA *ch, char *argument )
{
	OBJ_DATA *obj;
	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];

	argument = one_argument( argument, arg1 );
	argument = one_argument( argument, arg2 );
	if ( !str_cmp( arg2, "into" ) && argument[0] != '\0' )
		argument = one_argument( argument, arg2 );

	if ( arg1[0] == '\0' )
	{
		send_to_char( "> empty what\n\r", ch );
		return;
	}
	if ( ms_find_obj(ch) )
		return;

	if ( (obj = get_obj_carry( ch, arg1 )) == NULL )
	{
		send_to_char( "> you are not carrying that\n\r", ch );
		return;
	}
	if ( obj->count > 1 )
		separate_obj(obj);

	switch( obj->item_type )
	{
	default:
		act( AT_ACTION, "> you shake $p in an attempt to empty it", ch, obj, NULL, TO_CHAR );
		act( AT_ACTION, "> $n begins to shake $p in an attempt to empty it", ch, obj, NULL, TO_ROOM );
		return;
	case ITEM_DRINK_CON:
		if ( obj->value[1] < 1 )
		{
			send_to_char( "> it is already empty\n\r", ch );
			return;
		}
		act( AT_ACTION, "> you empty $p", ch, obj, NULL, TO_CHAR );
		act( AT_ACTION, "> $n empties $p", ch, obj, NULL, TO_ROOM );
		obj->value[1] = 0;
		return;
	case ITEM_CONTAINER:
		if ( IS_SET(obj->value[1], CONT_CLOSED) )
		{
			act( AT_PLAIN, "> the $d is closed", ch, NULL, obj->name, TO_CHAR );
			return;
		}
		if ( !obj->first_content )
		{
			send_to_char( "> it is already empty\n\r", ch );
			return;
		}
		if ( arg2[0] == '\0' )
		{
			if ( !IS_NPC(ch) &&  IS_SET( ch->act, PLR_LITTERBUG ) )
			{
				set_char_color( AT_MAGIC, ch );
				send_to_char( "> some force stops you\n\r", ch );
				set_char_color( AT_MAGIC, ch );
				send_to_char( "> no littering here\n\r", ch );
				return;
			}
			if ( empty_obj( obj, NULL, ch->in_room ) )
			{
				act( AT_ACTION, "> you empty $p", ch, obj, NULL, TO_CHAR );
				act( AT_ACTION, "> $n empties $p", ch, obj, NULL, TO_ROOM );
				if ( IS_SET( sysdata.save_flags, SV_DROP ) )
					save_char_obj( ch );
			}
			else
				send_to_char( "Hmmm... didn't work\n\r", ch );
		}
		else
		{
			OBJ_DATA *dest = get_obj_here( ch, arg2 );

			if ( !dest )
			{
				send_to_char( "> you cannot find it\n\r", ch );
				return;
			}
			if ( dest == obj )
			{
				send_to_char( "> you cannot empty something into itself\n\r", ch );
				return;
			}
			if ( dest->item_type != ITEM_CONTAINER )
			{
				send_to_char( "> that is not a container\n\r", ch );
				return;
			}
			if ( IS_SET(dest->value[1], CONT_CLOSED) )
			{
				act( AT_PLAIN, "> the $d is closed", ch, NULL, dest->name, TO_CHAR );
				return;
			}
			separate_obj( dest );
			if ( empty_obj( obj, dest, NULL ) )
			{
				act( AT_ACTION, "> you empty $p into $P", ch, obj, dest, TO_CHAR );
				act( AT_ACTION, "> $n empties $p into $P", ch, obj, dest, TO_ROOM );
				if ( !dest->carried_by
						&&    IS_SET( sysdata.save_flags, SV_PUT ) )
					save_char_obj( ch );
			}
			else
				act( AT_ACTION, "> $P is too full", ch, obj, dest, TO_CHAR );
		}
		return;
	}
}

void actiondesc( CHAR_DATA *ch, OBJ_DATA *obj, void *vo )
{
	char charbuf[MAX_STRING_LENGTH];
	char roombuf[MAX_STRING_LENGTH];
	char *srcptr = obj->action_desc;
	char *charptr = charbuf;
	char *roomptr = roombuf;
	const char *ichar;
	const char *iroom;

	while ( *srcptr != '\0' )
	{
		if ( *srcptr == '$' )
		{
			srcptr++;
			switch ( *srcptr )
			{
			case 'e':
				ichar = "you";
				iroom = "$e";
				break;

			case 'm':
				ichar = "you";
				iroom = "$m";
				break;

			case 'n':
				ichar = "you";
				iroom = "> $n";
				break;

			case 's':
				ichar = "your";
				iroom = "$s";
				break;

				/*case 'q':
        iroom = "s";
        break;*/

			default:
				srcptr--;
				*charptr++ = *srcptr;
				*roomptr++ = *srcptr;
				break;
			}
		}
		else if ( *srcptr == '%' && *++srcptr == 's' )
		{
			ichar = "> you";
			iroom = IS_NPC( ch ) ? ch->short_descr : ch->name;
		}
		else
		{
			*charptr++ = *srcptr;
			*roomptr++ = *srcptr;
			srcptr++;
			continue;
		}

		while ( ( *charptr = *ichar ) != '\0' )
		{
			charptr++;
			ichar++;
		}

		while ( ( *roomptr = *iroom ) != '\0' )
		{
			roomptr++;
			iroom++;
		}
		srcptr++;
	}

	*charptr = '\0';
	*roomptr = '\0';

	/*
sprintf( buf, "Charbuf: %s", charbuf );
log_string_plus( buf, LOG_HIGH, LEVEL_LESSER ); 
sprintf( buf, "Roombuf: %s", roombuf );
log_string_plus( buf, LOG_HIGH, LEVEL_LESSER ); 
	 */

	switch( obj->item_type )
	{
	case ITEM_FOUNTAIN:
		act( AT_ACTION, charbuf, ch, obj, ch, TO_CHAR );
		act( AT_ACTION, roombuf, ch, obj, ch, TO_ROOM );
		return;

	case ITEM_DRINK_CON:
		act( AT_ACTION, charbuf, ch, obj, liq_table[obj->value[2]].liq_name, TO_CHAR );
		act( AT_ACTION, roombuf, ch, obj, liq_table[obj->value[2]].liq_name, TO_ROOM );
		return;

	case ITEM_ARMOR:
	case ITEM_WEAPON:
	case ITEM_LIGHT:
		act( AT_ACTION, charbuf, ch, obj, ch, TO_CHAR );
		act( AT_ACTION, roombuf, ch, obj, ch, TO_ROOM );
		return;

	case ITEM_FOOD:
		act( AT_ACTION, charbuf, ch, obj, ch, TO_CHAR );
		act( AT_ACTION, roombuf, ch, obj, ch, TO_ROOM );
		return;

	default:
		return;
	}
	return;
}

void do_hail( CHAR_DATA *ch , char *argument )
{
	int vnum;
	ROOM_INDEX_DATA *room;

	if ( !ch->in_room )
		return;

	if ( !ch->in_room->area || !ch->in_room->area->planet )
	{
		send_to_char( "> you cannot use that command here\n\r", ch );
		return;
	}

	if ( ch->position < POS_FIGHTING )
	{
		send_to_char( "> you might want to stop fighting first\n\r", ch );
		return;
	}

	if ( ch->position < POS_STANDING )
	{
		send_to_char( "> you might want to stand up first\n\r", ch );
		return;
	}

	if ( IS_SET( ch->in_room->room_flags , ROOM_INDOORS ) )
	{
		send_to_char( "> you'll have to go outside to do that\n\r", ch );
		return;
	}

	if ( IS_SET( ch->in_room->room_flags , ROOM_SPACECRAFT ) )
	{
		send_to_char( "> you cannot do that on spacecraft\n\r", ch );
		return;
	}

	if ( ch->gold < (ch->top_level-9)  )
	{
		send_to_char( "> you do not have enough credits\n\r", ch );
		return;
	}

	vnum = ch->in_room->vnum;

	for ( room = ch->in_room->area->first_room  ;  room  ;  room = room->next_in_area )
	{
		if ( IS_SET(room->room_flags , ROOM_HOTEL ) )
			break;
	}

	if ( room == NULL || !IS_SET(room->room_flags , ROOM_HOTEL ) )
	{
		send_to_char( "> you cannot use that command here\n\r", ch );
		return;
	}

	ch->gold -= UMAX(ch->top_level-9 , 0);

	act( AT_ACTION, "> $n uses emergency logout", ch, NULL, NULL,  TO_ROOM );

	char_from_room( ch );
	char_to_room( ch, room );

	send_to_char( "> you use the emergency logout\n\r> you pay 20 credits\n\r\n\n" , ch );
	act( AT_ACTION, "> $n $T", ch, NULL, "arrives and logs out",  TO_ROOM );

	do_look( ch, "auto" );

}

void do_suicide( CHAR_DATA *ch, char *argument )
{
	char  logbuf[MAX_STRING_LENGTH];

	if ( IS_NPC(ch) || !ch->pcdata )
	{
		send_to_char( "> invalid command\n\r", ch );
		return;
	}

	if ( argument[0] == '\0' )
	{
		send_to_char( "&R> if you really want to delete this character type suicide and your password\n\r", ch );
		return;
	}

	if ( strcmp( crypt( argument, ch->pcdata->pwd ), ch->pcdata->pwd ) )
	{
		send_to_char( "> sorry wrong password\n\r", ch );
		sprintf( logbuf , "%s attempting to commit suicide... WRONG PASSWORD!" , ch->name );
		log_string( logbuf );
		return;
	}

	act( AT_BLOOD, "> with a sad determination and trembling hands you slit your own throat",  ch, NULL, NULL, TO_CHAR    );
	act( AT_BLOOD, "> cold shivers run down your spine as you watch $n slit $s own throat",  ch, NULL, NULL, TO_ROOM );

	sprintf( logbuf , "> %s has committed suicide" , ch->name );
	log_string( logbuf );

	set_cur_char(ch);
	raw_kill( ch, ch );

}

void do_bank( CHAR_DATA *ch, char *argument )
{
	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	char arg3[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	long amount = 0;

	argument = one_argument( argument , arg1 );
	argument = one_argument( argument , arg2 );
	argument = one_argument( argument , arg3 );

	if ( IS_NPC(ch) || !ch->pcdata )
		return;

	if (!ch->in_room || !IS_SET(ch->in_room->room_flags, ROOM_BANK) )
	{
		send_to_char( "> you must be in a bank to do that\n\r", ch );
		return;
	}

	if ( arg1[0] == '\0' )
	{
		send_to_char( "> syntax: BANK <deposit|withdraw|balance|transfer> [amount] <player>\n\r", ch );
		return;
	}

	if (arg2[0] != '\0' )
		amount = atoi(arg2);

	if ( !str_prefix( arg1 , "deposit" ) )
	{
		if ( amount  <= 0 )
		{
			send_to_char( "> you may only deposit amounts greater than zero\n\r", ch );
			do_bank( ch , "" );
			return;
		}

		if ( ch->gold < amount )
		{
			send_to_char( "> you do not have that many credits on you\n\r", ch );
			return;
		}

		ch->gold -= amount;
		ch->pcdata->bank += amount;

		ch_printf( ch , "> you deposit %ld credits into your account\n\r" ,amount );
		return;
	}
	else if ( !str_prefix( arg1 , "withdraw" ) )
	{
		if ( amount  <= 0 )
		{
			send_to_char( "> you may only withdraw amounts greater than zero\n\r", ch );
			do_bank( ch , "" );
			return;
		}

		if ( ch->pcdata->bank < amount )
		{
			send_to_char( "> you do not have that many credits in your account\n\r", ch );
			return;
		}

		ch->gold += amount;
		ch->pcdata->bank -= amount;

		ch_printf( ch , "> you withdraw %ld credits from your account\n\r" ,amount );
		return;

	}
	else if ( !str_prefix( arg1 , "balance" ) )
	{
		ch_printf( ch , "> you have %ld credits in your account\n\r" , ch->pcdata->bank );
		return;
	}

	else if ( !str_prefix( arg1, "transfer" ) )
	{

		if ( amount <= 0 )
		{
			ch_printf( ch, "> you may only transfer ammounts greater than 0\n\r");
			return;
		}
		if ( ch->pcdata->bank < amount )
		{
			ch_printf( ch , "> you do not have that many credits in your account\n\r");
			return;
		}
		if ( arg3[0] == '\0' )
		{
			ch_printf( ch, "> transfer credits to whom\n\r" );
			return;
		}
		if ( (victim = get_char_world( ch, arg3 )) == NULL )
		{
			ch_printf( ch, "> user not found\n\r");
			return;
		}
		if ( IS_NPC(victim) || !victim->pcdata )
		{
			ch_printf( ch, "> you cannot transfer them any credits\n\r" );
			return;
		}
		else
		{
			ch->pcdata->bank -= amount;
			victim->pcdata->bank += amount;
			ch_printf( victim, "> %s has transfered %ld credits to your account\n\r", ch->name, amount);
			ch_printf( ch, "> credits transfered\n\r" );
		}
	}
	else
	{
		do_bank( ch , "" );
		return;
	}


}

void do_dig( CHAR_DATA *ch, char *argument )
{
	char arg [MAX_INPUT_LENGTH];
	OBJ_DATA *obj;
	OBJ_DATA *startobj;
	bool found, shovel;
	EXIT_DATA *pexit;

	switch( ch->substate )
	{
	default:
		if ( IS_NPC(ch)  && IS_AFFECTED( ch, AFF_CHARM ) )
		{
			send_to_char( "> you cannot concentrate enough for that\n\r", ch );
			return;
		}
		if ( ch->mount )
		{
			send_to_char( "> you cannot do that while mounted\n\r", ch );
			return;
		}
		if ( ship_from_room( ch->in_room ) )
		{
			send_to_char( "> you would not want to do that here\n\r", ch );
			return;
		}
		one_argument( argument, arg );
		if ( arg[0] != '\0' )
		{
			if ( ( pexit = find_door( ch, arg, TRUE ) ) == NULL
					&&     get_dir(arg) == -1 )
			{
				send_to_char( "> what direction is that\n\r", ch );
				return;
			}
			if ( pexit )
			{
				if ( !IS_SET(pexit->exit_info, EX_DIG)
						&&   !IS_SET(pexit->exit_info, EX_CLOSED) )
				{
					send_to_char( "> there is no need to dig out that exit\n\r", ch );
					return;
				}
			}
		}
		else
		{
			switch( ch->in_room->sector_type )
			{
			case SECT_CITY:
			case SECT_INSIDE:
				send_to_char( "> the floor is too hard to dig through\n\r", ch );
				return;
			case SECT_WATER_SWIM:
			case SECT_WATER_NOSWIM:
			case SECT_UNDERWATER:
				send_to_char( "> you cannot dig here\n\r", ch );
				return;
			case SECT_AIR:
				send_to_char( "> not possible in the air\n\r", ch );
				return;
			}
		}
		add_timer( ch, TIMER_DO_FUN, 3, do_dig, 1);
		ch->dest_buf = str_dup( arg );
		send_to_char( "> you begin digging\n\r", ch );
		act( AT_PLAIN, "> $n begins digging", ch, NULL, NULL, TO_ROOM );
		return;

	case 1:
		if ( !ch->dest_buf )
		{
			send_to_char( "> your digging was interrupted\n\r", ch );
			act( AT_PLAIN, "> $n's digging was interrupted", ch, NULL, NULL, TO_ROOM );
			bug( "do_dig: dest_buf NULL", 0 );
			return;
		}
		strcpy( arg, ch->dest_buf );
		DISPOSE( ch->dest_buf );
		break;

	case SUB_TIMER_DO_ABORT:
		DISPOSE( ch->dest_buf );
		ch->substate = SUB_NONE;
		send_to_char( "> you stop digging\n\r", ch );
		act( AT_PLAIN, "> $n stops digging", ch, NULL, NULL, TO_ROOM );
		return;
	}

	ch->substate = SUB_NONE;

	if ( number_percent() == 23 )
	{
		send_to_char( "> you feel a little bit stronger\n\r", ch );
		ch->perm_str++;
		ch->perm_str = UMIN( ch->perm_str , 25 );
	}

	/* not having a shovel makes it harder to succeed */
	shovel = FALSE;
	for ( obj = ch->first_carrying; obj; obj = obj->next_content )
		if ( obj->item_type == ITEM_SHOVEL )
		{
			shovel = TRUE;
			break;
		}

	/* dig out an EX_DIG exit... */
	if ( arg[0] != '\0' )
	{
		if ( ( pexit = find_door( ch, arg, TRUE ) ) != NULL
				&&     IS_SET( pexit->exit_info, EX_DIG )
		&&     IS_SET( pexit->exit_info, EX_CLOSED ) )
		{
			/* 4 times harder to dig open a passage without a shovel */
			if ( (number_percent() * (shovel ? 1 : 4)) < 80 )
			{
				REMOVE_BIT( pexit->exit_info, EX_CLOSED );
				send_to_char( "> you dig open a passageway\n\r", ch );
				act( AT_PLAIN, "> $n digs open a passageway", ch, NULL, NULL, TO_ROOM );
				return;
			}
		}
		send_to_char( "> your dig did not discover any exit\n\r", ch );
		act( AT_PLAIN, "> $n's dig did not discover any exit", ch, NULL, NULL, TO_ROOM );
		return;
	}

	startobj = ch->in_room->first_content;
	found = FALSE;

	for ( obj = startobj; obj; obj = obj->next_content )
	{
		/* twice as hard to find something without a shovel */
		if ( IS_OBJ_STAT( obj, ITEM_BURRIED )
				&&  (number_percent() * (shovel ? 1 : 2)) < 80 )
		{
			found = TRUE;
			break;
		}
	}

	if ( !found )
	{
		send_to_char( "> your dig uncovered nothing\n\r", ch );
		act( AT_PLAIN, "> $n's dig uncovered nothing", ch, NULL, NULL, TO_ROOM );
		return;
	}

	separate_obj(obj);
	REMOVE_BIT( obj->extra_flags, ITEM_BURRIED );
	act( AT_SKILL, "> your dig uncovered $p", ch, obj, NULL, TO_CHAR );
	act( AT_SKILL, "> $n's dig uncovered $p", ch, obj, NULL, TO_ROOM );

	return;
}


void do_search( CHAR_DATA *ch, char *argument )
{
	char arg  [MAX_INPUT_LENGTH];
	OBJ_DATA *obj;
	OBJ_DATA *container;
	OBJ_DATA *startobj;
	int percent, door;
	bool found, room;

	door = -1;
	switch( ch->substate )
	{
	default:
		if ( IS_NPC(ch) && IS_AFFECTED( ch, AFF_CHARM ) )
		{
			send_to_char( "> you cannot concentrate enough for that\n\r", ch );
			return;
		}
		if ( ch->mount )
		{
			send_to_char( "> you cannot do that while mounted\n\r", ch );
			return;
		}
		argument = one_argument( argument, arg );
		if ( arg[0] != '\0' && (door = get_door( arg )) == -1 )
		{
			container = get_obj_here( ch, arg );
			if ( !container )
			{
				send_to_char( "> you cannot find that here\n\r", ch );
				return;
			}
			if ( container->item_type != ITEM_CONTAINER )
			{
				send_to_char( "> you cannot search in that\n\r", ch );
				return;
			}
			if ( IS_SET(container->value[1], CONT_CLOSED) )
			{
				send_to_char( "> it is closed\n\r", ch );
				return;
			}
		}
		add_timer( ch, TIMER_DO_FUN, 3,
				do_search, 1 );
		send_to_char( "> you begin your search\n\r", ch );
		ch->dest_buf = str_dup( arg );
		return;

	case 1:
		if ( !ch->dest_buf )
		{
			send_to_char( "> your search was interrupted\n\r", ch );
			bug( "do_search: dest_buf NULL", 0 );
			return;
		}
		strcpy( arg, ch->dest_buf );
		DISPOSE( ch->dest_buf );
		break;
	case SUB_TIMER_DO_ABORT:
		DISPOSE( ch->dest_buf );
		ch->substate = SUB_NONE;
		send_to_char( "> you stop your search\n\r", ch );
		return;
	}

	if ( number_percent() == 23 )
	{
		send_to_char( "> you feel a little bit wiser\n\r", ch );
		ch->perm_wis++;
		ch->perm_wis = UMIN( ch->perm_wis , 25 );
	}

	ch->substate = SUB_NONE;
	if ( arg[0] == '\0' )
	{
		room = TRUE;
		startobj = ch->in_room->first_content;
	}
	else
	{
		if ( (door = get_door( arg )) != -1 )
			startobj = NULL;
		else
		{
			container = get_obj_here( ch, arg );
			if ( !container )
			{
				send_to_char( "> you cannot find that here\n\r", ch );
				return;
			}
			startobj = container->first_content;
		}
	}

	found = FALSE;

	if ( (!startobj && door == -1) || IS_NPC(ch) )
	{
		send_to_char( "> you find nothing\n\r", ch );
		return;
	}

	percent  = number_percent( );

	if ( door != -1 )
	{
		EXIT_DATA *pexit;

		if ( (pexit = get_exit( ch->in_room, door )) != NULL
				&&   IS_SET( pexit->exit_info, EX_SECRET )
		&&   IS_SET( pexit->exit_info, EX_xSEARCHABLE )
		&&   percent < 80 )
		{
			act( AT_SKILL, "> your search reveals the $d", ch, NULL, pexit->keyword, TO_CHAR );
			act( AT_SKILL, "> $n finds the $d", ch, NULL, pexit->keyword, TO_ROOM );
			REMOVE_BIT( pexit->exit_info, EX_SECRET );
			return;
		}
	}
	else
		for ( obj = startobj; obj; obj = obj->next_content )
		{
			if ( IS_OBJ_STAT( obj, ITEM_HIDDEN )
					&&   percent < 70 )
			{
				found = TRUE;
				break;
			}
		}

	if ( !found )
	{
		send_to_char( "> you find nothing\n\r", ch );
		return;
	}

	separate_obj(obj);
	REMOVE_BIT( obj->extra_flags, ITEM_HIDDEN );
	act( AT_SKILL, "> your search reveals $p", ch, obj, NULL, TO_CHAR );
	act( AT_SKILL, "> $n finds $p", ch, obj, NULL, TO_ROOM );
	return;
}

void do_cyber(CHAR_DATA *ch, char *argument)
{
	CHAR_DATA *mob;
	char arg[MAX_INPUT_LENGTH];
	char buf[MAX_STRING_LENGTH];
	int cost;

	if ( IS_NPC(ch) )
		return;

	/* check for surgeon */
	for ( mob = ch->in_room->first_person; mob; mob = mob->next_in_room )
	{
		if ( IS_NPC(mob) && IS_SET(mob->act, ACT_IS_SURGEON) )
			break;
	}

	/* if there are no surgeon, display the characters enhancements*/
	if ( mob == NULL )
	{
		sprintf(buf, "> cyber implants:\n\r %s\n\r",
				cyber_bit_name(ch->pcdata->cyber));
		send_to_char(buf,ch);
		return;
	}

	one_argument(argument,arg);

	/* if there are a surgeon, give a list*/
	if (arg[0] == '\0')
	{
		/* display price list */
		do_say( mob, "&RI have these parts in stock&r:&G&C" );
		do_say( mob,"&C&W  comm&R    :&G Internal Comm Unit&Y        5000&C Credits&");
		do_say( mob,"&W  eyes&R    :&G Infrared Eyes&Y            10000&C Credits");
		do_say( mob,"&W  legs&R    :&G Cyber Replacements&Y       10000&C Credits");
		do_say( mob,"&W  reflex&R  :&G Augmented Reflexes&Y       13000&C Credits");
		do_say( mob,"&W  mind&R    :&G Internal Computer&Y        14000&C Credits");
		do_say( mob,"&W  muscle&R  :&G Augmented Muscles&Y        14000&C Credits");
		do_say( mob,"&W  chest&R   :&G External Chest Plating&Y   15000&C Credits");
		//        do_say( mob,"&W  reactor&R :&G Internal Reactor&Y         10000&C Credits");
		//        do_say( mob,"  sterile&R :&G cybernetic sterliation&Y   11500&C Credits");
		do_say( mob,"&R Type cyber&C <type>&R to buy one, or help cyber to get more info.&G&C");
		return;
	}

	/* Lets see what the character wants to have */
	if (!str_prefix(arg,"comm"))
	{
		cost  = 5000;
		if (cost > (ch->gold))
		{
			do_say( mob, "> you do not have enough credits for my services" );
			return;
		}
		if (IS_SET(ch->pcdata->cyber,CYBER_COMM))
		{
			do_say(mob,"> you already got that part" );
			return;
		}
		SET_BIT (ch->pcdata->cyber, CYBER_COMM );
	}

	else if (!str_prefix(arg,"eyes"))
	{
		cost  = 10000;
		if (cost > (ch->gold))
		{
			do_say( mob, "> you do not have enough credits for my services" );
			return;
		}
		if (IS_SET(ch->pcdata->cyber,CYBER_EYES))
		{
			do_say(mob,"> you already got that part" );
			return;
		}
		SET_BIT (ch->pcdata->cyber, CYBER_EYES );
		ch->affected_by   = AFF_INFRARED;
	}
	else if (!str_prefix(arg,"legs"))
	{
		cost = 10000;
		if (cost > (ch->gold))
		{
			do_say( mob, "> you do not have enough credits for my services" );
			return;
		}
		if (IS_SET(ch->pcdata->cyber,CYBER_LEGS))
		{
			do_say(mob,"> you already got that part" );
			return;
		}
		SET_BIT (ch->pcdata->cyber, CYBER_LEGS );
		ch->max_move += number_range ( 200 , 500 );
	}
	else if (!str_prefix(arg,"chest"))
	{
		cost  = 15000;
		if (cost > (ch->gold))
		{
			do_say( mob, "> you do not have enough credits for my services" );
			return;
		}
		if (IS_SET(ch->pcdata->cyber,CYBER_CHEST))
		{
			do_say(mob,"> you already got that part" );
			return;
		}
		SET_BIT (ch->pcdata->cyber, CYBER_CHEST );
	}
	else if (!str_prefix(arg,"reflex"))
	{
		cost = 13000;
		if (cost > (ch->gold))
		{
			do_say( mob, "> you do not have enough credits for my services" );
			return;
		}
		if (IS_SET(ch->pcdata->cyber,CYBER_REFLEXES))
		{
			do_say(mob,"> you already got that part" );
			return;
		}
		SET_BIT (ch->pcdata->cyber, CYBER_REFLEXES );
	}
	/*
        else if (!str_prefix(arg,"reactor"))
        {
                cost = 10000;
                if (cost > (ch->gold))
                {
                        do_say( mob, "> you do not have enough credits for my services" );
                        return;
                }
                if (IS_SET(ch->pcdata->cyber, CYBER_REACTOR))
                {
                        do_say(mob,"> you already got that part" );
                        return;
                }
               SET_BIT (ch->pcdata->cyber, CYBER_REACTOR );
                ch->max_move += number_range ( 200 , 500 );
        }
	 */
	else if (!str_prefix(arg,"mind"))
	{
		cost = 14000;
		if (cost > (ch->gold))
		{
			do_say( mob, "> you do not have enough credits for my services" );
			return;
		}
		if (IS_SET(ch->pcdata->cyber,CYBER_MIND))
		{
			do_say(mob,"> you already got that part" );
			return;
		}
		SET_BIT (ch->pcdata->cyber, CYBER_MIND );
	}

	else if (!str_prefix(arg,"muscle"))
	{
		cost = 14000;
		if (cost > (ch->gold))
		{
			do_say( mob, "> you do not have enough credits for my services" );
			return;
		}
		if (IS_SET(ch->pcdata->cyber,CYBER_STRENGTH))
		{
			do_say(mob,"> you already got that part" );
			return;
		}
		SET_BIT (ch->pcdata->cyber, CYBER_STRENGTH );
	}
	else
	{
		do_say( mob, "> type 'cyber' for a list of cybernetics" );
		return;
	}

	WAIT_STATE(ch,PULSE_VIOLENCE);

	ch->gold -= cost;
	do_say( mob,"> there we go better then then the original" );
}

//done for Neuro
