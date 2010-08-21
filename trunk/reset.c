#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "mud.h"

ROOM_INDEX_DATA * room_index_hash[MAX_KEY_HASH];

SHIP_DATA * make_mob_ship(PLANET_DATA *planet, int model);
void    resetship args( ( SHIP_DATA *ship ) );

void do_reset(CHAR_DATA * ch, char * argument) {
	reset_all();
}

/*
 * Reset everything.
 */
void reset_all() {

	ROOM_INDEX_DATA *pRoomIndex;
	int iHash;
	CHAR_DATA * mob = NULL;
	MOB_INDEX_DATA *pMobIndex = NULL;
	OBJ_INDEX_DATA *pObjIndex = NULL;
	OBJ_DATA * obj = NULL;
	EXIT_DATA *xit;
	bool found = FALSE;
	int vnum, anumber, onum, nodelevel, mobvnum, mobgold, numguards;
	int baselevel, entertainmax, multimediamax, financemax, productmax;
	char buf[MAX_STRING_LENGTH];
	char buf1[MAX_STRING_LENGTH];

	/* natural disasters */
	{
		PLANET_DATA * dPlanet = NULL;
		int pCount = 0;
		int rCount;
		CLAN_DATA * dClan = NULL;
		DESCRIPTOR_DATA *d = NULL;

		for (dPlanet = first_planet; dPlanet; dPlanet = dPlanet->next)
			pCount++;

		rCount = number_range(1, pCount);

		pCount = 0;

		for (dPlanet = first_planet; dPlanet; dPlanet = dPlanet->next)
			if (++pCount == rCount)
				break;

		if (dPlanet && dPlanet->area && dPlanet->governed_by)
			dClan = dPlanet->governed_by;

		if (dClan)
			for (d = first_descriptor; d; d = d->next)
				if (d->connected == CON_PLAYING && !d->original && d->character
						&& d->character->pcdata && d->character->pcdata->clan
						&& d->character->pcdata->clan == dPlanet->governed_by)
					break;

		if (d) {
			switch (number_bits(2)) {
			case 0:

				for (pRoomIndex = dPlanet->area->first_room; pRoomIndex; pRoomIndex
						= pRoomIndex->next_in_area)
					if (pRoomIndex->sector_type == SECT_DESERT
							&& !IS_SET( pRoomIndex->room_flags , ROOM_NO_MOB ))
						break;
				if (pRoomIndex) {
					int mCount;
					char dBuf[MAX_STRING_LENGTH];

					if ((pMobIndex = get_mob_index(MOB_VNUM_ALIEN))) {
						sprintf(
								dBuf,
								"(GCHAT) user: %s is being infected by AI viruses",
								dPlanet->name);
						echo_to_all(AT_LBLUE, dBuf, ECHOTAR_ALL );
						for (mCount = 0; mCount < 3; mCount++) {
							mob = create_mobile(pMobIndex);
							char_to_room(mob, pRoomIndex);
							mob->hit = 100;
							mob->max_hit = 100;
								SET_BIT(mob->affected_by, AFF_TRUESIGHT);
							if ((pObjIndex = get_obj_index(OBJ_VNUM_BLASTER))
									!= NULL) {
								obj = create_object(pObjIndex, mob->top_level);
								obj_to_char(obj, mob);
								equip_char(mob, obj, WEAR_WIELD);
							}
							do_setblaster(mob, "full");
						}
					}

				}
				break;

			default:
				break;
			}

		}

	}

	for (iHash = 0; iHash < MAX_KEY_HASH; iHash++) {
		for (pRoomIndex = room_index_hash[iHash]; pRoomIndex; pRoomIndex
				= pRoomIndex->next) {
			vnum = 0;
			onum = 0;

			for (xit = pRoomIndex->first_exit; xit; xit = xit->next)
				if (IS_SET(xit->exit_info , EX_ISDOOR )) {
					SET_BIT( xit->exit_info , EX_CLOSED );
					if (xit->key >= 0)
						SET_BIT( xit->exit_info , EX_LOCKED );
				}

			if (IS_SET(pRoomIndex->room_flags, ROOM_TRADE ))
				vnum = MOB_VNUM_TRADER;
			if (IS_SET(pRoomIndex->room_flags, ROOM_SUPPLY ))
				vnum = MOB_VNUM_SUPPLIER;
			if (IS_SET(pRoomIndex->room_flags, ROOM_PAWN ))
				vnum = MOB_VNUM_PAWNER;
			if (IS_SET(pRoomIndex->room_flags, ROOM_HOTEL ))
				vnum = MOB_VNUM_WAITER;
			if (IS_SET(pRoomIndex->room_flags, ROOM_GARAGE ))
				vnum = MOB_VNUM_MECHANIC;
			//if (IS_SET(pRoomIndex->room_flags, ROOM_EMPLOYMENT ))
			//	vnum = MOB_VNUM_JOB_OFFICER;

			if (vnum > 0) {
				found = FALSE;

				for (mob = pRoomIndex->first_person; mob; mob
						= mob->next_in_room) {
					if (IS_NPC( mob ) && mob->pIndexData
							&& mob->pIndexData->vnum == vnum)
						found = TRUE;
				}

				if (!found) {

					if (!(pMobIndex = get_mob_index(vnum))) {
						bug("Reset_all: Missing mob (%d)", vnum);
						return;
					}

					mob = create_mobile(pMobIndex);
					SET_BIT ( mob->act , ACT_CITIZEN );
					//if (room_is_dark(pRoomIndex))
						SET_BIT(mob->affected_by, AFF_INFRARED);
					char_to_room(mob, pRoomIndex);
					if (pRoomIndex->area && pRoomIndex->area->planet)
						pRoomIndex->area->planet->population++;
					if ((IS_SET(pRoomIndex->room_flags, ROOM_NOPEDIT) && vnum
							== MOB_VNUM_TRADER) || vnum == MOB_VNUM_SUPPLIER) {

						if (vnum != MOB_VNUM_SUPPLIER || number_bits(1) == 0) {

							if (!(pObjIndex = get_obj_index(OBJ_VNUM_COMPILER))) {
								bug("Reset_all: Missing default compiler (%d)",
										OBJ_VNUM_COMPILER );
								return;
							}

							obj = create_object(pObjIndex, 1);
							SET_BIT(obj->extra_flags, ITEM_INVENTORY);
							obj = obj_to_char(obj, mob);
						}

						if (vnum != MOB_VNUM_SUPPLIER || number_bits(1) == 0) {

							if (!(pObjIndex = get_obj_index(OBJ_VNUM_DEVKIT))) {
								bug("Reset_all: Missing default devkit (%d)",
										OBJ_VNUM_DEVKIT );
								return;
							}

							obj = create_object(pObjIndex, 1);
							SET_BIT(obj->extra_flags, ITEM_INVENTORY);
							obj = obj_to_char(obj, mob);
						}

						if (vnum != MOB_VNUM_SUPPLIER || number_bits(1) == 0) {
							if (!(pObjIndex = get_obj_index(OBJ_VNUM_COMLINK))) {
								bug("Reset_all: Missing default comlink (%d)",
										OBJ_VNUM_COMLINK );
								return;
							}
							obj = create_object(pObjIndex, 1);
							SET_BIT(obj->extra_flags, ITEM_INVENTORY);
							obj = obj_to_char(obj, mob);
						}

						if (vnum != MOB_VNUM_SUPPLIER || number_bits(1) == 0) {
							if (!(pObjIndex = get_obj_index(OBJ_VNUM_SEWKIT))) {
								bug("Reset_all: Missing default parser (%d)",
										OBJ_VNUM_SEWKIT );
								return;
							}
							obj = create_object(pObjIndex, 1);
							SET_BIT(obj->extra_flags, ITEM_INVENTORY);
							obj = obj_to_char(obj, mob);
						}

						if (vnum != MOB_VNUM_SUPPLIER || number_bits(1) == 0) {
							if (!(pObjIndex = get_obj_index(OBJ_VNUM_SHOVEL))) {
								bug("Reset_all: Missing default shovel (%d)",
										OBJ_VNUM_SHOVEL );
								return;
							}
							obj = create_object(pObjIndex, 1);
							SET_BIT(obj->extra_flags, ITEM_INVENTORY);
							obj = obj_to_char(obj, mob);
						}
					}

					if (vnum == MOB_VNUM_SUPPLIER) {
						if (number_bits(1) == 0) {
							if ((pObjIndex = get_obj_index(OBJ_VNUM_BATTERY))) {
								obj = create_object(pObjIndex, 1);
								SET_BIT(obj->extra_flags, ITEM_INVENTORY);
								obj = obj_to_char(obj, mob);
							}
						}
						if (number_bits(1) == 0) {
							if (!(pObjIndex = get_obj_index(OBJ_VNUM_BACKPACK))) {
								obj = create_object(pObjIndex, 1);
								SET_BIT(obj->extra_flags, ITEM_INVENTORY);
								obj = obj_to_char(obj, mob);
							}
						}
						if (number_bits(1) == 0) {
							if ((pObjIndex = get_obj_index(OBJ_VNUM_AMMO))) {
								obj = create_object(pObjIndex, 1);
								SET_BIT(obj->extra_flags, ITEM_INVENTORY);
								obj = obj_to_char(obj, mob);
							}
						}
						if (number_bits(1) == 0) {
							if ((pObjIndex = get_obj_index(
									OBJ_VNUM_SCHOOL_DAGGER))) {
								obj = create_object(pObjIndex, 1);
								SET_BIT(obj->extra_flags, ITEM_INVENTORY);
								obj = obj_to_char(obj, mob);
							}
						}
						if (number_bits(1) == 0) {
							if ((pObjIndex = get_obj_index(OBJ_VNUM_BLASTER))) {
								obj = create_object(pObjIndex, 1);
								SET_BIT(obj->extra_flags, ITEM_INVENTORY);
								obj = obj_to_char(obj, mob);
							}
						}

						onum = number_range(OBJ_VNUM_FIRST_PART,
								OBJ_VNUM_LAST_PART );
						if ((pObjIndex = get_obj_index(onum))) {
							obj = create_object(pObjIndex, 1);
							obj = obj_to_char(obj, mob);
							SET_BIT(obj->extra_flags, ITEM_INVENTORY);
						}
					}

				}

			}

			// firewalls

			if (IS_SET( pRoomIndex->room_flags, ROOM_BARRACKS )
					&& pRoomIndex->area && pRoomIndex->area->planet) {
				int guard_count = 0;
				int guardlevel = pRoomIndex->level + 1;
				OBJ_DATA * blaster;
				GUARD_DATA * guard;
				char tmpbuf[MAX_STRING_LENGTH];

				if (!(pMobIndex = get_mob_index(MOB_VNUM_PATROL))) {
					bug("Reset_all: Missing default patrol (%d)", vnum);
					return;
				}

				for (guard = pRoomIndex->area->planet->first_guard; guard; guard
						= guard->next_on_planet)
					guard_count++;

				if (pRoomIndex->area->planet->barracks * 5 <= guard_count)
					continue;

				mob = create_mobile(pMobIndex);
				char_to_room(mob, pRoomIndex);
				mob->top_level = 20 * guardlevel;
				mob->hit = 50 * guardlevel;
				mob->max_hit = 50 * guardlevel;
				mob->armor = 50;
				mob->damroll = 5 * guardlevel;
				mob->hitroll = 5 * guardlevel;
				if ((pObjIndex = get_obj_index(OBJ_VNUM_BLASTER)) != NULL) {
					blaster = create_object(pObjIndex, mob->top_level);
					obj_to_char(blaster, mob);
					equip_char(mob, blaster, WEAR_WIELD);
				}
				do_setblaster(mob, "full");

				CREATE( guard , GUARD_DATA , 1 );

				guard->planet = pRoomIndex->area->planet;
				LINK( guard , guard->planet->first_guard, guard->planet->last_guard, next_on_planet, prev_on_planet );
				LINK( guard , first_guard, last_guard, next, prev );
				guard->mob = mob;
				guard->reset_loc = pRoomIndex;
				mob->guard_data = guard;
				//if (room_is_dark(pRoomIndex))
					SET_BIT(mob->affected_by, AFF_INFRARED);
				if (pRoomIndex->area->planet->governed_by) {
					sprintf(tmpbuf, "ICE [%s] patrols the area\n\r",
							pRoomIndex->area->planet->governed_by->name);
					STRFREE( mob->long_descr );
					mob->long_descr = STRALLOC( tmpbuf );
					mob->mob_clan = pRoomIndex->area->planet->governed_by;
				}

				continue;
			}

			if (IS_SET( pRoomIndex->room_flags, ROOM_CAN_LAND )
					&& pRoomIndex->area && pRoomIndex->area->planet) {

				 if (!IS_SET( pRoomIndex->room_flags2, ROOM_HOMESYSIO ) )
				    {

				char tmpbuf[MAX_STRING_LENGTH];
				CHAR_DATA * rch;
				numguards = 0;

				if (!(pMobIndex = get_mob_index(MOB_VNUM_GUARD))) {
					bug("Reset_all: Missing default guard (%d)", vnum);
					return;
				}

				for (rch = pRoomIndex->first_person; rch; rch
						= rch->next_in_room)
					if (IS_NPC(rch) && rch->pIndexData && rch->pIndexData->vnum
							== MOB_VNUM_GUARD)
						numguards++;

				if (numguards >= 2)
					continue;

				mob = create_mobile(pMobIndex);
				char_to_room(mob, pRoomIndex);
				mob->top_level = 100;
				mob->hit = 500;
				mob->max_hit = 500;
				mob->armor = 0;
				mob->damroll = 0;
				mob->hitroll = 20;

				//if (room_is_dark(pRoomIndex))
					SET_BIT(mob->affected_by, AFF_INFRARED);
				if (pRoomIndex->area->planet->governed_by) {
					sprintf(tmpbuf, "guard ICE [%s]\n\r",
							pRoomIndex->area->planet->governed_by->name);
					STRFREE( mob->long_descr );
					mob->long_descr = STRALLOC( tmpbuf );
					mob->mob_clan = pRoomIndex->area->planet->governed_by;

					STRFREE( mob->description );
					mob->description	=STRALLOC( "You see a Guard ICE.\n Looking akward and built like a tank, he almost seems polygonal.\nResponsible of the security of his affected system, he watches around and he's ready to arrest any spoilsport : you better don't mess with Major Tom.\r");

				}

			}

			}

			// repos

			if (pRoomIndex->sector_type == SECT_FIELD
					|| pRoomIndex->sector_type == SECT_FOREST
					|| pRoomIndex->sector_type == SECT_HILLS
					|| pRoomIndex->sector_type == SECT_SCRUB ) {

			baselevel = pRoomIndex->level;

			if ( baselevel == 0 )
			nodelevel = 1;
			else if ( baselevel == 1 )
			nodelevel = 2;
			else if ( baselevel == 2 )
			nodelevel = 4;
			else if ( baselevel == 3 )
			nodelevel = 8;
			else if ( baselevel == 4 )
			nodelevel = 16;
			else if ( baselevel == 5 )
			nodelevel = 32;

			if (baselevel < 1)
				baselevel = 1;

			entertainmax = pRoomIndex->area->planet->entertain_plus - pRoomIndex->area->planet->entertain_minus;
			multimediamax = pRoomIndex->area->planet->multimedia_plus - pRoomIndex->area->planet->multimedia_minus;
			financemax = pRoomIndex->area->planet->finance_plus - pRoomIndex->area->planet->finance_minus;
			productmax = pRoomIndex->area->planet->product_plus - pRoomIndex->area->planet->product_minus;

			numguards = 0;
			CHAR_DATA * rch;

			switch (pRoomIndex->sector_type) {

			default:
				continue;
				break;

			case SECT_FIELD:

				if ( pRoomIndex->area->planet->entertain_count + baselevel > entertainmax )
					continue;

				vnum = 56;

				for (rch = pRoomIndex->first_person; rch; rch
						= rch->next_in_room)
					if (IS_NPC(rch) && rch->pIndexData && rch->pIndexData->vnum
							== vnum)
						numguards++;

				if (numguards >= 1)
					continue;

				pRoomIndex->area->planet->entertain_count += baselevel;
			break;

			case SECT_FOREST:

				if ( pRoomIndex->area->planet->multimedia_count + baselevel > multimediamax )
					continue;

				vnum = 57;

				for (rch = pRoomIndex->first_person; rch; rch
						= rch->next_in_room)
					if (IS_NPC(rch) && rch->pIndexData && rch->pIndexData->vnum
							== vnum)
						numguards++;

				if (numguards >= 1)
					continue;

				pRoomIndex->area->planet->multimedia_count += baselevel;
			break;
			case SECT_HILLS:

				if ( pRoomIndex->area->planet->finance_count + baselevel > financemax )
					continue;

				vnum = 58;

				for (rch = pRoomIndex->first_person; rch; rch
						= rch->next_in_room)
					if (IS_NPC(rch) && rch->pIndexData && rch->pIndexData->vnum
							== vnum)
						numguards++;

				if (numguards >= 1)
					continue;

				pRoomIndex->area->planet->finance_count += baselevel;
			break;
			case SECT_SCRUB:

				if ( pRoomIndex->area->planet->product_count + baselevel > productmax )
					continue;

				vnum = 59;

				for (rch = pRoomIndex->first_person; rch; rch
						= rch->next_in_room)
					if (IS_NPC(rch) && rch->pIndexData && rch->pIndexData->vnum
							== vnum)
						numguards++;

				if (numguards >= 1)
					continue;
				pRoomIndex->area->planet->product_count += baselevel;
			break;
			}

			if (!(pMobIndex = get_mob_index(vnum))) {
				bug("Reset_all: Missing mob (%d)", vnum);
				return;
			}

				mob = create_mobile(pMobIndex);
				//if (room_is_dark(pRoomIndex))
					SET_BIT(mob->affected_by, AFF_INFRARED);
				char_to_room(mob, pRoomIndex);
				mob->top_level = 20 * baselevel;
				mob->hit = 20 * baselevel;
				mob->max_hit = 20 * baselevel;
				mob->armor = -20 * baselevel;
				mob->damroll = 2 * baselevel;
				mob->hitroll = 4 * baselevel;
				continue;

		}

			/* hidden food & resources */

			if (!pRoomIndex->area || !pRoomIndex->area->planet)
				continue;

			anumber = number_bits(9);

			if ( ( pRoomIndex->sector_type == SECT_FARMLAND && !pRoomIndex->last_content && number_bits(3) == 0 )
			|| ( pRoomIndex->sector_type == SECT_RAINFOREST && !pRoomIndex->last_content && number_bits(3) == 0 )
			|| ( pRoomIndex->sector_type == SECT_GLACIAL && !pRoomIndex->last_content && number_bits(3) == 0 )
			|| ( pRoomIndex->sector_type == SECT_TUNNEL && !pRoomIndex->last_content && number_bits(3) == 0 )
			|| ( pRoomIndex->sector_type == SECT_CORRIDOR && !pRoomIndex->last_content && number_bits(3) == 0 )
			|| ( pRoomIndex->sector_type == SECT_FARM && !pRoomIndex->last_content && number_bits(3) == 0 )
			|| ( pRoomIndex->sector_type == SECT_FACTORY && !pRoomIndex->last_content && number_bits(3) == 0 ))
			{

				switch (pRoomIndex->sector_type) {

				default:
					continue;
					break;

				case SECT_RAINFOREST:
				case SECT_GLACIAL:
				case SECT_TUNNEL:
				case SECT_CORRIDOR:


					anumber = number_range(0, 8);
					if (anumber == 0)
						vnum = 31;
					else if (anumber == 1)
						vnum = 34;
					else if (anumber == 2)
						vnum = 35;
					else if (anumber == 3)
						vnum = 57;
					else if (anumber == 4)
						vnum = 59;
					else
						vnum = number_range(75, 99);


					if (!(pObjIndex = get_obj_index(vnum))) {
						bug("Reset_all: Missing obj (%d)", vnum);
						return;
					}
					obj = create_object(pObjIndex, 1);

					if (vnum > 90 && vnum < 98) {
						if (number_range(1, 2) == 1)
							SET_BIT ( obj->extra_flags , ITEM_BURRIED );
						else
							SET_BIT ( obj->extra_flags , ITEM_HIDDEN );
					}

					if (vnum == OBJ_VNUM_GOLD) {
						obj->value[0] = number_range(1, 10);
						obj->value[1] = obj->value[0];
						obj->cost = obj->value[0] * 10;
					}

					obj_to_room(obj, pRoomIndex);

					break;

				case SECT_FARMLAND:
				case SECT_FARM:
				case SECT_FACTORY:

					nodelevel = pRoomIndex->level;

					anumber = number_range(1, 10);
					if (anumber <= 4)
						vnum = 110;
					else if (anumber <= 7)
						vnum = 111;
					else if (anumber <= 9)
						vnum = 112;
					else if (anumber == 10)
						vnum = 113;
					else
						vnum = 110;

					if (nodelevel == 0)
					{

						if (!(pObjIndex = get_obj_index(vnum))) {
							bug("Reset_all: Missing obj (%d)", vnum);
							return;
						}

						obj = create_object(pObjIndex, 1);
						obj->cost = obj->cost * 2;
					    STRFREE( obj->short_descr );
					    sprintf( buf , "%s [alpha]" , obj->name );
					    obj->short_descr = STRALLOC( buf );
					    STRFREE( obj->description );
					    sprintf( buf1 , "%s [alpha]" , obj->name );
					    obj->description = STRALLOC( buf1 );

					}
					else if (nodelevel == 1)
					{

							if (!(pObjIndex = get_obj_index(vnum))) {
								bug("Reset_all: Missing obj (%d)", vnum);
								return;
							}

							obj = create_object(pObjIndex, 1);
							obj->cost = obj->cost * 4;
						    STRFREE( obj->short_descr );
						    sprintf( buf , "%s [beta]" , obj->name );
						    obj->short_descr = STRALLOC( buf );
						    STRFREE( obj->description );
						    sprintf( buf1 , "%s [beta]" , obj->name );
						    obj->description = STRALLOC( buf1 );
						}
					else if (nodelevel == 2)
					{

							if (!(pObjIndex = get_obj_index(vnum))) {
								bug("Reset_all: Missing obj (%d)", vnum);
								return;
							}

							obj = create_object(pObjIndex, 1);
							obj->cost = obj->cost * 8;
						    STRFREE( obj->short_descr );
						    sprintf( buf , "%s [candidate]" , obj->name );
						    obj->short_descr = STRALLOC( buf );
						    STRFREE( obj->description );
						    sprintf( buf1 , "%s [candidate]" , obj->name );
						    obj->description = STRALLOC( buf1 );
						}
					else if (nodelevel == 3)
					{

							if (!(pObjIndex = get_obj_index(vnum))) {
								bug("Reset_all: Missing obj (%d)", vnum);
								return;
							}

							obj = create_object(pObjIndex, 1);
							obj->cost = obj->cost * 16;
						    STRFREE( obj->short_descr );
						    sprintf( buf , "%s [release]" , obj->name );
						    obj->short_descr = STRALLOC( buf );
						    STRFREE( obj->description );
						    sprintf( buf1 , "%s [release]" , obj->name );
						    obj->description = STRALLOC( buf1 );
						}
					else if (nodelevel == 4)
					{

							if (!(pObjIndex = get_obj_index(vnum))) {
								bug("Reset_all: Missing obj (%d)", vnum);
								return;
							}

							obj = create_object(pObjIndex, 1);
							obj->cost = obj->cost * 32;
						    STRFREE( obj->short_descr );
						    sprintf( buf , "%s [prototype]" , obj->name );
						    obj->short_descr = STRALLOC( buf );
						    STRFREE( obj->description );
						    sprintf( buf1 , "%s [prototype]" , obj->name );
						    obj->description = STRALLOC( buf1 );
						}
					else if (nodelevel == 5)
					{

							if (!(pObjIndex = get_obj_index(vnum))) {
								bug("Reset_all: Missing obj (%d)", vnum);
								return;
							}

							obj = create_object(pObjIndex, 1);
							obj->cost = obj->cost * 64;
						    STRFREE( obj->short_descr );
						    sprintf( buf , "%s [wilderspace]" , obj->name );
						    obj->short_descr = STRALLOC( buf );
						    STRFREE( obj->description );
						    sprintf( buf1 , "%s [wilderspace]" , obj->name );
						    obj->description = STRALLOC( buf1 );
						}
					else
					{

							if (!(pObjIndex = get_obj_index(vnum))) {
								bug("Reset_all: Missing obj (%d)", vnum);
								return;
							}

							obj = create_object(pObjIndex, 1);
							obj->cost = obj->cost * 2;
						    STRFREE( obj->short_descr );
						    sprintf( buf , "%s [buggy]" , obj->name );
						    obj->short_descr = STRALLOC( buf );
						    STRFREE( obj->description );
						    sprintf( buf1 , "%s [buggy]" , obj->name );
						    obj->description = STRALLOC( buf1 );
						}

					obj_to_room(obj, pRoomIndex);

					break;

			}
			}

			/* random mobs start here */

			if (IS_SET( pRoomIndex->room_flags, ROOM_NO_MOB ))
				continue;

			if (number_bits(1) != 0)
				continue;

			if (pRoomIndex->sector_type == SECT_DESERT) {

				if (pRoomIndex->area->planet->population >= max_population(
						pRoomIndex->area->planet))
					continue;

				if (number_bits(5) == 0) {

					if ((pMobIndex = get_mob_index(MOB_VNUM_VENDOR))) {
						int rep;

						mob = create_mobile(pMobIndex);
						SET_BIT ( mob->act , ACT_CITIZEN );
						char_to_room(mob, pRoomIndex);
						pRoomIndex->area->planet->population++;
						for (rep = 0; rep < 3; rep++)
							if ((pObjIndex = get_obj_index(
									number_range(OBJ_VNUM_FIRST_FABRIC,
											OBJ_VNUM_LAST_FABRIC )))) {
								obj = create_object(pObjIndex, 1);
								SET_BIT(obj->extra_flags, ITEM_INVENTORY);
								obj = obj_to_char(obj, mob);
							}
						if ((pObjIndex = get_obj_index(OBJ_VNUM_SEWKIT))) {
							obj = create_object(pObjIndex, 1);
							SET_BIT(obj->extra_flags, ITEM_INVENTORY);
							obj = obj_to_char(obj, mob);
						}
						continue;
					}
				}

				if (number_bits(6) == 0) {
					if ((pMobIndex = get_mob_index(MOB_VNUM_DEALER))) {
						mob = create_mobile(pMobIndex);
						SET_BIT ( mob->act , ACT_CITIZEN );
						char_to_room(mob, pRoomIndex);
						pRoomIndex->area->planet->population++;
						if ((pObjIndex = get_obj_index(OBJ_VNUM_BLACK_POWDER))) {
							obj = create_object(pObjIndex, 1);
							SET_BIT(obj->extra_flags, ITEM_INVENTORY);
							obj = obj_to_char(obj, mob);
						}
						continue;
					}
				}

				if (number_bits(6) == 0) {
					int mnum;

					switch (number_bits(2)) {
					default:
						mnum = MOB_VNUM_BUM;
						break;
					case 0:
						mnum = MOB_VNUM_THUG;
						break;
					case 1:
						mnum = MOB_VNUM_THIEF;
						break;
					}

					if ((pMobIndex = get_mob_index(mnum))) {
						mob = create_mobile(pMobIndex);
						SET_BIT ( mob->act , ACT_CITIZEN );
						char_to_room(mob, pRoomIndex);
						pRoomIndex->area->planet->population++;
						continue;
					}
				}

				switch ( pRoomIndex->level ) {
					default:
						mobvnum = 60;
						mobgold = number_range(1, 2);
						break;

					case 0:
						mobvnum = 60;
						mobgold = number_range(1, 2);
						break;

					case 1:
						mobvnum = 61;
						mobgold = number_range(2, 4);
						break;

					case 2:
						mobvnum = 62;
						mobgold = number_range(4, 8);
						break;

					case 3:
						mobvnum = 63;
						mobgold = number_range(8, 16);
						break;

					case 4:
						mobvnum = 64;
						mobgold = number_range(16, 32);
						break;

					case 5:
						mobvnum = 65;
						mobgold = number_range(32, 64);
						break;
					}

				if (!(pMobIndex = get_mob_index(mobvnum))) {
					bug("Reset_all: Missing default user (%d)", mobvnum);
					return;
				}

				mob = create_mobile(pMobIndex);
				SET_BIT ( mob->act , ACT_CITIZEN );
				mob->sex = number_bits(1) + 1;
				mob->gold = mobgold;

				char_to_room(mob, pRoomIndex);
				pRoomIndex->area->planet->population++;
				continue;
			}

			// wildlife

			if (pRoomIndex->area->planet->wildlife
					> pRoomIndex->area->planet->wilderness && !IS_SET( pRoomIndex->area->planet->flags, PLANET_HIDDEN ) )
				continue;

			anumber = number_bits(3);

			switch (pRoomIndex->sector_type) {

			default:
				continue;
				break;

			case SECT_BREEDING:
			case SECT_HALL:
			case SECT_TUNNEL:
			case SECT_CORRIDOR:
			case SECT_RUINS:
			case SECT_SETTLEMENT:
			case SECT_FACTORY:
			case SECT_PRISON:
			case SECT_FARM:

			{
				CHAR_DATA * rch;
				numguards = 0;
				for (rch = pRoomIndex->first_person; rch; rch
						= rch->next_in_room)
					if (IS_NPC(rch) && rch->pIndexData && rch->pIndexData->vnum
							== 50)
						numguards++;

				if (numguards >= 2)
					continue;

				CHAR_DATA * sch;
				int numhench = 0;
				for (sch = pRoomIndex->first_person; sch; sch
						= sch->next_in_room)
					if (IS_NPC(sch) && sch->pIndexData && sch->pIndexData->vnum
							== 51)
						numhench++;

				if (numhench >= 1)
					continue;

				vnum = 50;

				int randmob = number_range(1, 5);

				if ( randmob == 1)
				{

				vnum = 51;

				if (!(pMobIndex = get_mob_index(vnum))) {
					bug("Reset_all: Missing mob (%d)", vnum);
					return;
				}
				mob = create_mobile(pMobIndex);
				SET_BIT(mob->affected_by, AFF_INFRARED);
				char_to_room(mob, pRoomIndex);
				mob->top_level = 15 * (pRoomIndex->level + 1);
				mob->hit = 50 * (pRoomIndex->level + 1);
				mob->max_hit = 50 * (pRoomIndex->level + 1);
				mob->armor = 0 - ((pRoomIndex->level + 1) * 20);
				mob->damroll = 5 * (pRoomIndex->level + 1);
				mob->hitroll = 5 * (pRoomIndex->level + 1);

				if ((pObjIndex = get_obj_index(OBJ_VNUM_SCHOOL_DAGGER))
						!= NULL) {
					obj = create_object(pObjIndex, mob->top_level);
					obj_to_char(obj, mob);
					equip_char(mob, obj, WEAR_WIELD);
				}
				//do_setblaster(mob, "full");

				STRFREE( mob->long_descr );
				mob->long_descr	= STRALLOC( "rogue hacker\n\r" );
				STRFREE( mob->name );
				mob->name	= STRALLOC( "rogue hacker" );
				STRFREE( mob->short_descr );
				mob->short_descr	= STRALLOC( "rogue hacker" );
				mob->gold = ( number_range(100, 200) * (pRoomIndex->level + 1) );

				STRFREE( mob->description );
				mob->description	=STRALLOC( "You see a Rogue Hacker. \nSurvivor of a gone era, he was certainly some kind of traitor, conspiring in the shadows against NetWatch Org. They were taken down in its fall.\n Even if he now looks like an outdated poor thing, never underestimate him : a feral spark still glows in his eyes, watching around for something to pounce with his blade.\n\n\r");

				continue;
				}
				else if ( randmob == 2)
				{
					if (!(pMobIndex = get_mob_index(vnum))) {
						bug("Reset_all: Missing mob (%d)", vnum);
						return;
					}
					mob = create_mobile(pMobIndex);
					//if (room_is_dark(pRoomIndex))
					SET_BIT(mob->affected_by, AFF_INFRARED);
					char_to_room(mob, pRoomIndex);
					mob->top_level = 15 * (pRoomIndex->level + 1);
					mob->hit = 50 * (pRoomIndex->level + 1);
					mob->max_hit = 50 * (pRoomIndex->level + 1);
					mob->armor = 0 - ((pRoomIndex->level + 1) * 20);
					mob->damroll = 5 * (pRoomIndex->level + 1);
					mob->hitroll = 5 * (pRoomIndex->level + 1);

					if ((pObjIndex = get_obj_index(OBJ_VNUM_SCHOOL_DAGGER))
							!= NULL) {
						obj = create_object(pObjIndex, mob->top_level);
						obj_to_char(obj, mob);
						equip_char(mob, obj, WEAR_WIELD);
					}
					//do_setblaster(mob, "full");

					STRFREE( mob->name );
					mob->name	= STRALLOC( "netWatch scout" );
					STRFREE( mob->short_descr );
					mob->short_descr	= STRALLOC( "netWatch scout" );
					STRFREE( mob->long_descr );
					mob->long_descr	= STRALLOC( "netWatch scout\n\r" );
					mob->gold = ( number_range(50, 100) * (pRoomIndex->level + 1) );

					STRFREE( mob->description );
					mob->description	=STRALLOC( "You see a NetWatch Scout. \nBulky and mainly composed of its comlink module and scanner array, it moves systematically through the node scanning for intruders. \nA NetWatch Emblem glows dimly on its side, memory of the past greatness of systems and organization now forgotten.\n\r");

					continue;
				}
				else if ( randmob == 3)
				{
					if (!(pMobIndex = get_mob_index(vnum))) {
						bug("Reset_all: Missing mob (%d)", vnum);
						return;
					}
					mob = create_mobile(pMobIndex);
					//if (room_is_dark(pRoomIndex))
						SET_BIT(mob->affected_by, AFF_INFRARED);
					char_to_room(mob, pRoomIndex);
					mob->top_level = 15 * (pRoomIndex->level + 1);
					mob->hit = 50 * (pRoomIndex->level + 1);
					mob->max_hit = 50 * (pRoomIndex->level + 1);
					mob->armor = 0 - ((pRoomIndex->level + 1) * 20);
					mob->damroll = 5 * (pRoomIndex->level + 1);
					mob->hitroll = 5 * (pRoomIndex->level + 1);

					STRFREE( mob->name );
					mob->name	= STRALLOC( "metropolis virus" );
					STRFREE( mob->short_descr );
					mob->short_descr	= STRALLOC( "metropolis virus" );
					STRFREE( mob->long_descr );
					mob->long_descr	= STRALLOC( "metropolis virus\n\r" );
					mob->gold = ( number_range(25, 50) * (pRoomIndex->level + 1) );

					STRFREE( mob->description );
					mob->description	=STRALLOC( "you see a Metro Virus. \nLarge and rough, the Metro Virus glares around the node, daring anyone to challenge it.\n With its experience of Metro, it's a dreadful opponent and it'll not hesitate to take some distance and shoot you if you're too mighty for direct assault.\n\r" );


					if ((pObjIndex = get_obj_index(OBJ_VNUM_BLASTER))
							!= NULL) {
						obj = create_object(pObjIndex, mob->top_level);
						obj_to_char(obj, mob);
						equip_char(mob, obj, WEAR_WIELD);
					}
					do_setblaster(mob, "full");

					if ((pObjIndex = get_obj_index(OBJ_VNUM_TOKEN))
							!= NULL) {
						obj = create_object(pObjIndex, mob->top_level);
						obj_to_char(obj, mob);
					}

					continue;
				}
				else
				{
					if (!(pMobIndex = get_mob_index(vnum))) {
						bug("Reset_all: Missing mob (%d)", vnum);
						return;
					}
					mob = create_mobile(pMobIndex);
					SET_BIT(mob->affected_by, AFF_INFRARED);
					char_to_room(mob, pRoomIndex);
					mob->top_level = 5 * (pRoomIndex->level + 1);
					mob->hit = 20 * (pRoomIndex->level + 1);
					mob->max_hit = 20 * (pRoomIndex->level + 1);
					mob->damroll = (pRoomIndex->level + 1);
					mob->hitroll = (pRoomIndex->level + 1);

					STRFREE( mob->name );
					mob->name	= STRALLOC( "metropolis bot" );
					STRFREE( mob->short_descr );
					mob->short_descr	= STRALLOC( "metropolis bot" );
					STRFREE( mob->long_descr );
					mob->long_descr	= STRALLOC( "metropolis bot\n\r" );
					mob->gold = ( number_range(5, 10) * (pRoomIndex->level + 1) );

					STRFREE( mob->description );
					mob->description	=STRALLOC( "You see a Metro bot, a large Cube with a seemingly unbroken surface.\nA small scanner pops out on one of the surfaces and sweeps the node, checking for some ennemy to wipe out.\n Then, it returns and leaves the surface uniform once again, treacherously harmless-looking.\n\r");

					if ((pObjIndex = get_obj_index(OBJ_VNUM_TOKEN))
							!= NULL) {
						obj = create_object(pObjIndex, mob->top_level);
						obj_to_char(obj, mob);
					}

					continue;
				}
			}

				break;

			case SECT_GLACIAL:
				anumber = number_range(0, 5);
				if (anumber == 0)
					vnum = MOB_VNUM_SMALL_ANIMAL;
				else if (anumber == 1)
					vnum = MOB_VNUM_BIRD;
				else if (anumber == 2)
					vnum = MOB_VNUM_SCAVENGER;
				else if (anumber == 3)
					vnum = MOB_VNUM_PREDITOR;
				else if (anumber == 4)
					vnum = MOB_VNUM_DATAMINER;
				else
					vnum = MOB_VNUM_INSECT;
				break;

			case SECT_RAINFOREST:

				if (pRoomIndex->level == 0)
				{
					CHAR_DATA * rch;
					numguards = 0;
					for (rch = pRoomIndex->first_person; rch; rch
							= rch->next_in_room)
						if (IS_NPC(rch) && rch->pIndexData && rch->pIndexData->vnum
								== 50)
							numguards++;

					if (numguards >= 2)
						continue;

					CHAR_DATA * sch;
					int numhench = 0;
					for (sch = pRoomIndex->first_person; sch; sch
							= sch->next_in_room)
						if (IS_NPC(sch) && sch->pIndexData && sch->pIndexData->vnum
								== 51)
							numhench++;

					if (numhench >= 1)
						continue;

					vnum = 50;
					int randmob = number_range(1, 16);
					if ( randmob == 1)
					{

					vnum = 51;

					if (!(pMobIndex = get_mob_index(vnum))) {
						bug("Reset_all: Missing mob (%d)", vnum);
						return;
					}
					mob = create_mobile(pMobIndex);
					//if (room_is_dark(pRoomIndex))
						SET_BIT(mob->affected_by, AFF_INFRARED);
					char_to_room(mob, pRoomIndex);
					mob->top_level = 20;
					mob->hit = 100;
					mob->max_hit = 100;
					mob->armor = -50;
					mob->damroll = 10;
					mob->hitroll = 20;

					if ((pObjIndex = get_obj_index(OBJ_VNUM_SCHOOL_DAGGER))
							!= NULL) {
						obj = create_object(pObjIndex, mob->top_level);
						obj_to_char(obj, mob);
						equip_char(mob, obj, WEAR_WIELD);
					}
					//do_setblaster(mob, "full");

					STRFREE( mob->long_descr );
					mob->long_descr	= STRALLOC( "gravedigger [worm]\n\r" );
					STRFREE( mob->name );
					mob->name	= STRALLOC( "gravedigger worm" );
					STRFREE( mob->short_descr );
					mob->short_descr	= STRALLOC( "gravedigger worm" );
					mob->gold = number_range(20, 40);

					continue;
					}
					else if ( randmob == 2)
					{
						if (!(pMobIndex = get_mob_index(vnum))) {
							bug("Reset_all: Missing mob (%d)", vnum);
							return;
						}
						mob = create_mobile(pMobIndex);
						//if (room_is_dark(pRoomIndex))
							SET_BIT(mob->affected_by, AFF_INFRARED);
						char_to_room(mob, pRoomIndex);
						mob->top_level = 10;
						mob->hit = 50;
						mob->max_hit = 50;
						mob->armor = 0;
						mob->damroll = 0;
						mob->hitroll = 5;

						if ((pObjIndex = get_obj_index(OBJ_VNUM_SCHOOL_DAGGER))
								!= NULL) {
							obj = create_object(pObjIndex, mob->top_level);
							obj_to_char(obj, mob);
							equip_char(mob, obj, WEAR_WIELD);
						}
						//do_setblaster(mob, "full");

						STRFREE( mob->name );
						mob->name	= STRALLOC( "graverobber virus" );
						STRFREE( mob->short_descr );
						mob->short_descr	= STRALLOC( "graverobber virus" );
						STRFREE( mob->long_descr );
						mob->long_descr	= STRALLOC( "graverobber [virus]\n\r" );
						mob->gold = number_range(10, 20);

						continue;
					}
					else if ( randmob == 3)
					{
						if (!(pMobIndex = get_mob_index(vnum))) {
							bug("Reset_all: Missing mob (%d)", vnum);
							return;
						}
						mob = create_mobile(pMobIndex);
						//if (room_is_dark(pRoomIndex))
							SET_BIT(mob->affected_by, AFF_INFRARED);
						char_to_room(mob, pRoomIndex);
						mob->top_level = 10;
						mob->hit = 50;
						mob->max_hit = 50;
						mob->armor = 0;
						mob->damroll = 0;
						mob->hitroll = 5;

						STRFREE( mob->name );
						mob->name	= STRALLOC( "ghoul virus" );
						STRFREE( mob->short_descr );
						mob->short_descr	= STRALLOC( "ghoul virus" );
						STRFREE( mob->long_descr );
						mob->long_descr	= STRALLOC( "ghoul [virus]\n\r" );
						mob->gold = number_range(10, 20);

						if ((pObjIndex = get_obj_index(OBJ_VNUM_BLASTER))
								!= NULL) {
							obj = create_object(pObjIndex, mob->top_level);
							obj_to_char(obj, mob);
							equip_char(mob, obj, WEAR_WIELD);
						}
						do_setblaster(mob, "normal");

						continue;
					}
					else if ( randmob >= 4)
					{
						if (!(pMobIndex = get_mob_index(vnum))) {
							bug("Reset_all: Missing mob (%d)", vnum);
							return;
						}
						mob = create_mobile(pMobIndex);
						//if (room_is_dark(pRoomIndex))
							SET_BIT(mob->affected_by, AFF_INFRARED);
						char_to_room(mob, pRoomIndex);
						mob->top_level = 10;
						mob->hit = 50;
						mob->max_hit = 50;
						mob->armor = 0;
						mob->damroll = 0;
						mob->hitroll = 5;

						STRFREE( mob->name );
						mob->name	= STRALLOC( "skeleton virus" );
						STRFREE( mob->short_descr );
						mob->short_descr	= STRALLOC( "skeleton virus" );
						STRFREE( mob->long_descr );
						mob->long_descr	= STRALLOC( "skeleton [virus]\n\r" );
						mob->gold = number_range(10, 20);


						continue;
					}
					else if ( randmob >= 10)
					{
						if (!(pMobIndex = get_mob_index(vnum))) {
							bug("Reset_all: Missing mob (%d)", vnum);
							return;
						}
						mob = create_mobile(pMobIndex);
						//if (room_is_dark(pRoomIndex))
							SET_BIT(mob->affected_by, AFF_INFRARED);
						char_to_room(mob, pRoomIndex);
						mob->top_level = 10;
						mob->hit = 50;
						mob->max_hit = 50;
						mob->armor = 0;
						mob->damroll = 0;
						mob->hitroll = 5;

						STRFREE( mob->name );
						mob->name	= STRALLOC( "zombie virus" );
						STRFREE( mob->short_descr );
						mob->short_descr	= STRALLOC( "zombie virus" );
						STRFREE( mob->long_descr );
						mob->long_descr	= STRALLOC( "zombie [virus]\n\r" );
						mob->gold = number_range(10, 20);

						continue;
					}

				}
				else if (pRoomIndex->level == 1)
				{
					CHAR_DATA * rch;
					numguards = 0;
					for (rch = pRoomIndex->first_person; rch; rch
							= rch->next_in_room)
						if (IS_NPC(rch) && rch->pIndexData && rch->pIndexData->vnum
								== 50)
							numguards++;

					if (numguards >= 4)
						continue;

					CHAR_DATA * sch;
					int numhench = 0;
					for (sch = pRoomIndex->first_person; sch; sch
							= sch->next_in_room)
						if (IS_NPC(sch) && sch->pIndexData && sch->pIndexData->vnum
								== 51)
							numhench++;

					if (numhench >= 2)
						continue;

					CHAR_DATA * ach;
					int numaivatar = 0;
					for (ach = pRoomIndex->first_person; ach; ach
							= ach->next_in_room)
						if (IS_NPC(ach) && ach->pIndexData && ach->pIndexData->vnum
								== 52)
							numaivatar++;

					if (numaivatar >= 1)
						continue;

					int randmob = number_range(1, 16);
					if ( randmob == 1)
					{

					vnum = 52;

					if (!(pMobIndex = get_mob_index(vnum))) {
						bug("Reset_all: Missing mob (%d)", vnum);
						return;
					}
					mob = create_mobile(pMobIndex);
					//if (room_is_dark(pRoomIndex))
						SET_BIT(mob->affected_by, AFF_INFRARED);
					char_to_room(mob, pRoomIndex);
					mob->top_level = 40;
					mob->hit = 200;
					mob->max_hit = 200;
					mob->armor = -100;
					mob->damroll = 20;
					mob->hitroll = 30;

					if ((pObjIndex = get_obj_index(OBJ_VNUM_SCHOOL_DAGGER))
							!= NULL) {
						obj = create_object(pObjIndex, mob->top_level);
						obj_to_char(obj, mob);
						equip_char(mob, obj, WEAR_WIELD);
					}
					//do_setblaster(mob, "full");

					STRFREE( mob->long_descr );
					mob->long_descr	= STRALLOC( "necromancer [aivatar]\n\r" );
					STRFREE( mob->name );
					mob->name	= STRALLOC( "necromancer aivatar" );
					STRFREE( mob->short_descr );
					mob->short_descr	= STRALLOC( "necromancer aivatar" );
					mob->gold = number_range(40, 80);

					continue;
					}
					else if ( randmob == 2)
					{
						vnum = 51;

						if (!(pMobIndex = get_mob_index(vnum))) {
							bug("Reset_all: Missing mob (%d)", vnum);
							return;
						}
						mob = create_mobile(pMobIndex);
						//if (room_is_dark(pRoomIndex))
							SET_BIT(mob->affected_by, AFF_INFRARED);
						char_to_room(mob, pRoomIndex);
						mob->top_level = 20;
						mob->hit = 100;
						mob->max_hit = 100;
						mob->armor = -50;
						mob->damroll = 10;
						mob->hitroll = 20;

						if ((pObjIndex = get_obj_index(OBJ_VNUM_SCHOOL_DAGGER))
								!= NULL) {
							obj = create_object(pObjIndex, mob->top_level);
							obj_to_char(obj, mob);
							equip_char(mob, obj, WEAR_WIELD);
						}
						//do_setblaster(mob, "full");

						STRFREE( mob->name );
						mob->name	= STRALLOC( "gravedigger worm" );
						STRFREE( mob->short_descr );
						mob->short_descr	= STRALLOC( "gravedigger worm" );
						STRFREE( mob->long_descr );
						mob->long_descr	= STRALLOC( "gravedigger [worm]\n\r" );
						mob->gold = number_range(20, 40);
						continue;
					}
					else if ( randmob == 3)
					{
						vnum = 51;

						if (!(pMobIndex = get_mob_index(vnum))) {
							bug("Reset_all: Missing mob (%d)", vnum);
							return;
						}
						mob = create_mobile(pMobIndex);
						//if (room_is_dark(pRoomIndex))
							SET_BIT(mob->affected_by, AFF_INFRARED);
						char_to_room(mob, pRoomIndex);
						mob->top_level = 20;
						mob->hit = 100;
						mob->max_hit = 100;
						mob->armor = -50;
						mob->damroll = 10;
						mob->hitroll = 20;

						if ((pObjIndex = get_obj_index(OBJ_VNUM_BLASTER))
								!= NULL) {
							obj = create_object(pObjIndex, mob->top_level);
							obj_to_char(obj, mob);
							equip_char(mob, obj, WEAR_WIELD);
						}
						do_setblaster(mob, "normal");

						STRFREE( mob->name );
						mob->name	= STRALLOC( "banshee worm" );
						STRFREE( mob->short_descr );
						mob->short_descr	= STRALLOC( "banshee worm" );
						STRFREE( mob->long_descr );
						mob->long_descr	= STRALLOC( "banshee [worm]\n\r" );
						mob->gold = number_range(20, 40);
						continue;
					}
					else if ( randmob >= 4)
					{
						vnum = 51;

						if (!(pMobIndex = get_mob_index(vnum))) {
							bug("Reset_all: Missing mob (%d)", vnum);
							return;
						}
						mob = create_mobile(pMobIndex);
						//if (room_is_dark(pRoomIndex))
							SET_BIT(mob->affected_by, AFF_INFRARED);
						char_to_room(mob, pRoomIndex);
						mob->top_level = 20;
						mob->hit = 100;
						mob->max_hit = 100;
						mob->armor = -50;
						mob->damroll = 10;
						mob->hitroll = 20;

						STRFREE( mob->name );
						mob->name	= STRALLOC( "gargoyle worm" );
						STRFREE( mob->short_descr );
						mob->short_descr	= STRALLOC( "gargoyle worm" );
						STRFREE( mob->long_descr );
						mob->long_descr	= STRALLOC( "gargoyle [worm]\n\r" );
						mob->gold = number_range(20, 40);
						continue;
					}
					else if ( randmob >= 8)
					{
						vnum = 51;

						if (!(pMobIndex = get_mob_index(vnum))) {
							bug("Reset_all: Missing mob (%d)", vnum);
							return;
						}
						mob = create_mobile(pMobIndex);
						//if (room_is_dark(pRoomIndex))
							SET_BIT(mob->affected_by, AFF_INFRARED);
						char_to_room(mob, pRoomIndex);
						mob->top_level = 20;
						mob->hit = 100;
						mob->max_hit = 100;
						mob->armor = -50;
						mob->damroll = 10;
						mob->hitroll = 20;

						STRFREE( mob->name );
						mob->name	= STRALLOC( "spirit worm" );
						STRFREE( mob->short_descr );
						mob->short_descr	= STRALLOC( "spirit worm" );
						STRFREE( mob->long_descr );
						mob->long_descr	= STRALLOC( "spirit [worm]\n\r" );
						mob->gold = number_range(20, 40);
						continue;
					}
					else if ( randmob == 13)
					{
						vnum = 50;

						if (!(pMobIndex = get_mob_index(vnum))) {
							bug("Reset_all: Missing mob (%d)", vnum);
							return;
						}
						mob = create_mobile(pMobIndex);
						//if (room_is_dark(pRoomIndex))
							SET_BIT(mob->affected_by, AFF_INFRARED);
						char_to_room(mob, pRoomIndex);
						mob->top_level = 10;
						mob->hit = 50;
						mob->max_hit = 50;
						mob->armor = 0;
						mob->damroll = 0;
						mob->hitroll = 5;

						if ((pObjIndex = get_obj_index(OBJ_VNUM_SCHOOL_DAGGER))
								!= NULL) {
							obj = create_object(pObjIndex, mob->top_level);
							obj_to_char(obj, mob);
							equip_char(mob, obj, WEAR_WIELD);
						}
						//do_setblaster(mob, "full");

						STRFREE( mob->name );
						mob->name	= STRALLOC( "graverobber virus" );
						STRFREE( mob->short_descr );
						mob->short_descr	= STRALLOC( "graverobber virus" );
						STRFREE( mob->long_descr );
						mob->long_descr	= STRALLOC( "graverobber [virus]\n\r" );
						mob->gold = number_range(10, 20);

						continue;
					}
					else if ( randmob == 14)
					{
						vnum = 50;

						if (!(pMobIndex = get_mob_index(vnum))) {
							bug("Reset_all: Missing mob (%d)", vnum);
							return;
						}
						mob = create_mobile(pMobIndex);
						//if (room_is_dark(pRoomIndex))
							SET_BIT(mob->affected_by, AFF_INFRARED);
						char_to_room(mob, pRoomIndex);
						mob->top_level = 10;
						mob->hit = 50;
						mob->max_hit = 50;
						mob->armor = 0;
						mob->damroll = 0;
						mob->hitroll = 5;

						if ((pObjIndex = get_obj_index(OBJ_VNUM_BLASTER))
								!= NULL) {
							obj = create_object(pObjIndex, mob->top_level);
							obj_to_char(obj, mob);
							equip_char(mob, obj, WEAR_WIELD);
						}
						do_setblaster(mob, "normal");

						STRFREE( mob->name );
						mob->name	= STRALLOC( "ghoul virus" );
						STRFREE( mob->short_descr );
						mob->short_descr	= STRALLOC( "ghoul virus" );
						STRFREE( mob->long_descr );
						mob->long_descr	= STRALLOC( "ghoul [virus]\n\r" );
						mob->gold = number_range(10, 20);
						continue;
					}
					else if ( randmob == 15)
					{
						vnum = 50;

						if (!(pMobIndex = get_mob_index(vnum))) {
							bug("Reset_all: Missing mob (%d)", vnum);
							return;
						}
						mob = create_mobile(pMobIndex);
						//if (room_is_dark(pRoomIndex))
							SET_BIT(mob->affected_by, AFF_INFRARED);
						char_to_room(mob, pRoomIndex);
						mob->top_level = 10;
						mob->hit = 50;
						mob->max_hit = 50;
						mob->armor = 0;
						mob->damroll = 0;
						mob->hitroll = 5;

						STRFREE( mob->name );
						mob->name	= STRALLOC( "zombie virus" );
						STRFREE( mob->short_descr );
						mob->short_descr	= STRALLOC( "zombie virus" );
						STRFREE( mob->long_descr );
						mob->long_descr	= STRALLOC( "zombie [virus]\n\r" );
						mob->gold = number_range(10, 20);
						continue;
					}
					else if ( randmob == 16)
					{
						vnum = 50;

						if (!(pMobIndex = get_mob_index(vnum))) {
							bug("Reset_all: Missing mob (%d)", vnum);
							return;
						}
						mob = create_mobile(pMobIndex);
						//if (room_is_dark(pRoomIndex))
							SET_BIT(mob->affected_by, AFF_INFRARED);
						char_to_room(mob, pRoomIndex);
						mob->top_level = 10;
						mob->hit = 50;
						mob->max_hit = 50;
						mob->armor = 0;
						mob->damroll = 0;
						mob->hitroll = 5;

						STRFREE( mob->name );
						mob->name	= STRALLOC( "skeleton virus" );
						STRFREE( mob->short_descr );
						mob->short_descr	= STRALLOC( "skeleton virus" );
						STRFREE( mob->long_descr );
						mob->long_descr	= STRALLOC( "skeleton [virus]\n\r" );
						mob->gold = number_range(10, 20);
						continue;
					}

				}
				else if (pRoomIndex->level == 2)
				{
					CHAR_DATA * rch;
					numguards = 0;
					for (rch = pRoomIndex->first_person; rch; rch
							= rch->next_in_room)
						if (IS_NPC(rch) && rch->pIndexData && rch->pIndexData->vnum
								== 51)
							numguards++;

					if (numguards >= 6)
						continue;

					CHAR_DATA * sch;
					int numhench = 0;
					for (sch = pRoomIndex->first_person; sch; sch
							= sch->next_in_room)
						if (IS_NPC(sch) && sch->pIndexData && sch->pIndexData->vnum
								== 52)
							numhench++;

					if (numhench >= 4)
						continue;

					CHAR_DATA * ach;
					int numaivatar = 0;
					for (ach = pRoomIndex->first_person; ach; ach
							= ach->next_in_room)
						if (IS_NPC(ach) && ach->pIndexData && ach->pIndexData->vnum
								== 53)
							numaivatar++;

					if (numaivatar >= 2)
						continue;

					int randmob = number_range(1, 16);
					if ( randmob == 1)
					{

					vnum = 53;

					if (!(pMobIndex = get_mob_index(vnum))) {
						bug("Reset_all: Missing mob (%d)", vnum);
						return;
					}
					mob = create_mobile(pMobIndex);
					//if (room_is_dark(pRoomIndex))
						SET_BIT(mob->affected_by, AFF_INFRARED);
					char_to_room(mob, pRoomIndex);
					mob->top_level = 80;
					mob->hit = 500;
					mob->max_hit = 500;
					mob->armor = -250;
					mob->damroll = 40;
					mob->hitroll = 50;

					if ((pObjIndex = get_obj_index(OBJ_VNUM_SCHOOL_DAGGER))
							!= NULL) {
						obj = create_object(pObjIndex, mob->top_level);
						obj_to_char(obj, mob);
						equip_char(mob, obj, WEAR_WIELD);
					}
					//do_setblaster(mob, "full");

					STRFREE( mob->long_descr );
					mob->long_descr	= STRALLOC( "soulflayer [rom]\n\r" );
					STRFREE( mob->name );
					mob->name	= STRALLOC( "soulflayer rom" );
					STRFREE( mob->short_descr );
					mob->short_descr	= STRALLOC( "soulflayer rom" );
					mob->gold = number_range(100, 200);

					continue;
					}
					else if ( randmob == 2)
					{
						vnum = 52;

						if (!(pMobIndex = get_mob_index(vnum))) {
							bug("Reset_all: Missing mob (%d)", vnum);
							return;
						}
						mob = create_mobile(pMobIndex);
						//if (room_is_dark(pRoomIndex))
							SET_BIT(mob->affected_by, AFF_INFRARED);
						char_to_room(mob, pRoomIndex);
						mob->top_level = 40;
						mob->hit = 200;
						mob->max_hit = 200;
						mob->armor = -100;
						mob->damroll = 20;
						mob->hitroll = 30;

						if ((pObjIndex = get_obj_index(OBJ_VNUM_SCHOOL_DAGGER))
								!= NULL) {
							obj = create_object(pObjIndex, mob->top_level);
							obj_to_char(obj, mob);
							equip_char(mob, obj, WEAR_WIELD);
						}
						//do_setblaster(mob, "full");

						STRFREE( mob->long_descr );
						mob->long_descr	= STRALLOC( "necromancer [aivatar]\n\r" );
						STRFREE( mob->name );
						mob->name	= STRALLOC( "necromancer aivatar" );
						STRFREE( mob->short_descr );
						mob->short_descr	= STRALLOC( "necromancer aivatar" );
						mob->gold = number_range(40, 80);
						continue;
					}
					else if ( randmob == 3)
					{
						vnum = 52;

						if (!(pMobIndex = get_mob_index(vnum))) {
							bug("Reset_all: Missing mob (%d)", vnum);
							return;
						}
						mob = create_mobile(pMobIndex);
						//if (room_is_dark(pRoomIndex))
							SET_BIT(mob->affected_by, AFF_INFRARED);
						char_to_room(mob, pRoomIndex);
						mob->top_level = 40;
						mob->hit = 200;
						mob->max_hit = 200;
						mob->armor = -100;
						mob->damroll = 20;
						mob->hitroll = 30;

						if ((pObjIndex = get_obj_index(OBJ_VNUM_BLASTER))
								!= NULL) {
							obj = create_object(pObjIndex, mob->top_level);
							obj_to_char(obj, mob);
							equip_char(mob, obj, WEAR_WIELD);
						}
						do_setblaster(mob, "normal");

						STRFREE( mob->long_descr );
						mob->long_descr	= STRALLOC( "vampire [aivatar]\n\r" );
						STRFREE( mob->name );
						mob->name	= STRALLOC( "vampire aivatar" );
						STRFREE( mob->short_descr );
						mob->short_descr	= STRALLOC( "vampire aivatar" );
						mob->gold = number_range(40, 80);
						continue;
					}
					else if ( randmob >= 4)
					{
						vnum = 52;

						if (!(pMobIndex = get_mob_index(vnum))) {
							bug("Reset_all: Missing mob (%d)", vnum);
							return;
						}
						mob = create_mobile(pMobIndex);
						//if (room_is_dark(pRoomIndex))
							SET_BIT(mob->affected_by, AFF_INFRARED);
						char_to_room(mob, pRoomIndex);
						mob->top_level = 40;
						mob->hit = 200;
						mob->max_hit = 200;
						mob->armor = -100;
						mob->damroll = 20;
						mob->hitroll = 30;

						STRFREE( mob->long_descr );
						mob->long_descr	= STRALLOC( "wraith [aivatar]\n\r" );
						STRFREE( mob->name );
						mob->name	= STRALLOC( "wraith aivatar" );
						STRFREE( mob->short_descr );
						mob->short_descr	= STRALLOC( "wraith aivatar" );
						mob->gold = number_range(40, 80);
						continue;
					}
					else if ( randmob >= 8)
					{
						vnum = 52;

						if (!(pMobIndex = get_mob_index(vnum))) {
							bug("Reset_all: Missing mob (%d)", vnum);
							return;
						}
						mob = create_mobile(pMobIndex);
						//if (room_is_dark(pRoomIndex))
							SET_BIT(mob->affected_by, AFF_INFRARED);
						char_to_room(mob, pRoomIndex);
						mob->top_level = 40;
						mob->hit = 200;
						mob->max_hit = 200;
						mob->armor = -100;
						mob->damroll = 20;
						mob->hitroll = 30;

						STRFREE( mob->long_descr );
						mob->long_descr	= STRALLOC( "lich [aivatar]\n\r" );
						STRFREE( mob->name );
						mob->name	= STRALLOC( "lich aivatar" );
						STRFREE( mob->short_descr );
						mob->short_descr	= STRALLOC( "lich aivatar" );
						mob->gold = number_range(10, 20);
						continue;
					}
					else if ( randmob == 13)
					{
						vnum = 51;

						if (!(pMobIndex = get_mob_index(vnum))) {
							bug("Reset_all: Missing mob (%d)", vnum);
							return;
						}
						mob = create_mobile(pMobIndex);
						//if (room_is_dark(pRoomIndex))
							SET_BIT(mob->affected_by, AFF_INFRARED);
						char_to_room(mob, pRoomIndex);
						mob->top_level = 20;
						mob->hit = 100;
						mob->max_hit = 100;
						mob->armor = -50;
						mob->damroll = 10;
						mob->hitroll = 20;

						if ((pObjIndex = get_obj_index(OBJ_VNUM_SCHOOL_DAGGER))
								!= NULL) {
							obj = create_object(pObjIndex, mob->top_level);
							obj_to_char(obj, mob);
							equip_char(mob, obj, WEAR_WIELD);
						}
						//do_setblaster(mob, "full");

						STRFREE( mob->name );
						mob->name	= STRALLOC( "gravedigger worm" );
						STRFREE( mob->short_descr );
						mob->short_descr	= STRALLOC( "gravedigger worm" );
						STRFREE( mob->long_descr );
						mob->long_descr	= STRALLOC( "gravedigger [worm]\n\r" );
						mob->gold = number_range(20, 40);
						continue;					}
					else if ( randmob == 14)
					{
						vnum = 51;

						if (!(pMobIndex = get_mob_index(vnum))) {
							bug("Reset_all: Missing mob (%d)", vnum);
							return;
						}
						mob = create_mobile(pMobIndex);
						//if (room_is_dark(pRoomIndex))
							SET_BIT(mob->affected_by, AFF_INFRARED);
						char_to_room(mob, pRoomIndex);
						mob->top_level = 20;
						mob->hit = 100;
						mob->max_hit = 100;
						mob->armor = -50;
						mob->damroll = 10;
						mob->hitroll = 20;

						if ((pObjIndex = get_obj_index(OBJ_VNUM_BLASTER))
								!= NULL) {
							obj = create_object(pObjIndex, mob->top_level);
							obj_to_char(obj, mob);
							equip_char(mob, obj, WEAR_WIELD);
						}
						do_setblaster(mob, "normal");

						STRFREE( mob->name );
						mob->name	= STRALLOC( "banshee worm" );
						STRFREE( mob->short_descr );
						mob->short_descr	= STRALLOC( "banshee worm" );
						STRFREE( mob->long_descr );
						mob->long_descr	= STRALLOC( "banshee [worm]\n\r" );
						mob->gold = number_range(20, 40);
						continue;						}
					else if ( randmob == 15)
					{
						vnum = 51;

						if (!(pMobIndex = get_mob_index(vnum))) {
							bug("Reset_all: Missing mob (%d)", vnum);
							return;
						}
						mob = create_mobile(pMobIndex);
						//if (room_is_dark(pRoomIndex))
							SET_BIT(mob->affected_by, AFF_INFRARED);
						char_to_room(mob, pRoomIndex);
						mob->top_level = 20;
						mob->hit = 100;
						mob->max_hit = 100;
						mob->armor = -50;
						mob->damroll = 10;
						mob->hitroll = 20;

						STRFREE( mob->name );
						mob->name	= STRALLOC( "gargoyle worm" );
						STRFREE( mob->short_descr );
						mob->short_descr	= STRALLOC( "gargoyle worm" );
						STRFREE( mob->long_descr );
						mob->long_descr	= STRALLOC( "gargoyle [worm]\n\r" );
						mob->gold = number_range(20, 40);
						continue;						}
					else if ( randmob == 16)
					{
						vnum = 51;

						if (!(pMobIndex = get_mob_index(vnum))) {
							bug("Reset_all: Missing mob (%d)", vnum);
							return;
						}
						mob = create_mobile(pMobIndex);
						//if (room_is_dark(pRoomIndex))
							SET_BIT(mob->affected_by, AFF_INFRARED);
						char_to_room(mob, pRoomIndex);
						mob->top_level = 20;
						mob->hit = 100;
						mob->max_hit = 100;
						mob->armor = -50;
						mob->damroll = 10;
						mob->hitroll = 20;

						STRFREE( mob->name );
						mob->name	= STRALLOC( "spirit worm" );
						STRFREE( mob->short_descr );
						mob->short_descr	= STRALLOC( "spirit worm" );
						STRFREE( mob->long_descr );
						mob->long_descr	= STRALLOC( "spirit [worm]\n\r" );
						mob->gold = number_range(20, 40);
						continue;
					}
			}
				break;

			}

			if (!(pMobIndex = get_mob_index(vnum))) {
				bug("Reset_all: Missing mob (%d)", vnum);
				return;
			}

			mob = create_mobile(pMobIndex);
			REMOVE_BIT ( mob->act , ACT_CITIZEN );
			//if (room_is_dark(pRoomIndex))
				SET_BIT(mob->affected_by, AFF_INFRARED);
			char_to_room(mob, pRoomIndex);
			pRoomIndex->area->planet->wildlife++;

			}
			}
}

SHIP_DATA * make_mob_ship(PLANET_DATA *planet, int model) {
	SHIP_DATA *ship;
	int shipreg = 0;
	int dIndex = 0;
	char filename[10];
	char shipname[MAX_STRING_LENGTH];

	if (!planet || !planet->governed_by || !planet->starsystem)
		return NULL;

	/* mobships are given filenames < 0 and are not saved */

	for (ship = first_ship; ship; ship = ship->next)
		if (shipreg > atoi(ship->filename))
			shipreg = atoi(ship->filename);

	shipreg--;
	sprintf(filename, "%d", shipreg);

	CREATE( ship, SHIP_DATA, 1 );
	LINK( ship, first_ship, last_ship, next, prev );

	ship->filename = str_dup(filename);

	ship->next_in_starsystem = NULL;
	ship->prev_in_starsystem = NULL;
	ship->next_in_room = NULL;
	ship->prev_in_room = NULL;
	ship->first_turret = NULL;
	ship->last_turret = NULL;
	ship->first_hanger = NULL;
	ship->last_hanger = NULL;
	ship->in_room = NULL;
	ship->starsystem = NULL;
	ship->home = STRALLOC( planet->starsystem->name );
	for (dIndex = 0; dIndex < MAX_SHIP_ROOMS; dIndex++)
		ship->description[dIndex] = NULL;
	ship->owner = STRALLOC( planet->governed_by->name );
	ship->pilot = STRALLOC("");
	ship->copilot = STRALLOC("");
	ship->dest = NULL;
	ship->type = MOB_SHIP;
	ship->class = 0;
	ship->model = model;
	ship->hyperspeed = 0;
	ship->laserstate = LASER_READY;
	ship->missilestate = MISSILE_READY;
	ship->tractorbeam = 2;
	ship->hatchopen = FALSE;
	ship->autotrack = FALSE;
	ship->autospeed = FALSE;
	ship->location = 0;
	ship->lastdoc = 0;
	ship->shipyard = 0;
	ship->collision = 0;
	ship->target = NULL;
	ship->currjump = NULL;
	ship->chaff = 0;
	ship->maxchaff = 0;
	ship->chaff_released = FALSE;

	switch (ship->model) {
	case MOB_BATTLESHIP:
		ship->realspeed = 25;
		ship->maxmissiles = 50;
		ship->lasers = 10;
		ship->maxenergy = 30000;
		ship->maxshield = 1000;
		ship->maxhull = 30000;
		ship->manuever = 25;
		sprintf(shipname, "Battlecruiser m%d (%s)", 0 - shipreg,
				planet->governed_by->name);
		break;

	case MOB_CRUISER:
		ship->realspeed = 50;
		ship->maxmissiles = 30;
		ship->lasers = 8;
		ship->maxenergy = 15000;
		ship->maxshield = 350;
		ship->maxhull = 10000;
		ship->manuever = 50;
		sprintf(shipname, "Cruiser m%d (%s)", 0 - shipreg,
				planet->governed_by->name);
		break;

	case MOB_DESTROYER:
		ship->realspeed = 100;
		ship->maxmissiles = 20;
		ship->lasers = 6;
		ship->maxenergy = 7500;
		ship->maxshield = 200;
		ship->maxhull = 2000;
		ship->manuever = 100;
		ship->hyperspeed = 100;
		sprintf(shipname, "Corvette m%d (%s)", 0 - shipreg,
				planet->governed_by->name);
		break;

	default:
		ship->realspeed = 255;
		ship->maxmissiles = 0;
		ship->lasers = 2;
		ship->maxenergy = 2500;
		ship->maxshield = 0;
		ship->maxhull = 100;
		ship->manuever = 100;
		sprintf(shipname, "Patrol Starfighter m%d (%s)", 0 - shipreg,
				planet->governed_by->name);
		break;
	}

	ship->name = STRALLOC( shipname );
	ship->hull = ship->maxhull;
	ship->missiles = ship->maxmissiles;
	ship->energy = ship->maxenergy;
	ship->shield = 0;

	ship_to_starsystem(ship, starsystem_from_name(ship->home));
	ship->vx = planet->x + number_range(-2000, 2000);
	ship->vy = planet->y + number_range(-2000, 2000);
	ship->vz = planet->z + number_range(-2000, 2000);
	ship->shipstate = SHIP_READY;
	ship->autopilot = TRUE;
	ship->autorecharge = TRUE;
	ship->shield = ship->maxshield;

	return ship;
}

//done for Neuro
