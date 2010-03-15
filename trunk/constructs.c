#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include "mud.h"

extern int top_r_vnum;

// wilderspace port node to access the randomly generated construct

void do_makeconstruct(CHAR_DATA *ch, char *argument) {

	ROOM_INDEX_DATA * constnode;
	ROOM_INDEX_DATA * location;
	ROOM_INDEX_DATA * troom;
	AREA_DATA * pArea;
	AREA_DATA * tarea;
	EXIT_DATA * xit;
	int rnum, exitposs;
	sh_int edir;

	location = ch->in_room;

	if (IS_NPC(ch) || !ch->pcdata) {
		send_to_char("> invalid command\n\r", ch);
		return;
	}

	if (!ch->in_room || !IS_SET( ch->in_room->room_flags, ROOM_SHIPYARD )) {
		send_to_char("> go to the straylight io node\n\r", ch);
		return;
	}

	if (ch->pcdata->roomconstruct) {
		send_to_char("> &Rsorry, but you already have a construct\n\r", ch);
		return;
	}

	if ( ch->gold < 5000 )
	{
		send_to_char( "> &Rfirst level construct costs 5,000 credits&w\n\r", ch);
		return;
	}

	ch->gold -= 5000;

	for (rnum = 1; rnum <= 4; rnum++) {

		constnode = make_room(++top_r_vnum);

		if (!constnode) {
			bug("makep: make_room failed", 0);
			return;
		}

		for (tarea = first_area; tarea; tarea = tarea->next) {
			if (!str_cmp(tarea->filename, "construct")) {
				pArea = tarea;
			}
		}

		constnode->area = pArea;

		constnode->owner = STRALLOC( "wilderspace" );
		constnode->description = STRALLOC("a wilderspace node.");
		constnode->name = STRALLOC( "node" );
		constnode->sector_type = SECT_RAINFOREST;
		SET_BIT( constnode->room_flags , ROOM_NOPEDIT );

		LINK( constnode , pArea->first_room , pArea->last_room , next_in_area , prev_in_area );

		if (rnum == 1) {
			SET_BIT( constnode->room_flags , ROOM_NO_MOB );
			SET_BIT( constnode->room_flags , ROOM_SAFE );
			constnode->sector_type = SECT_INSIDE;
			ch->pcdata->roomconstruct = top_r_vnum;
			ch->pcdata->constructlevel = 1;

			STRFREE( constnode->name );
			STRFREE( constnode->description );
			constnode->description = STRALLOC("a safehouse node.\n\r");
			constnode->name = STRALLOC( "safehouse" );

		}

		if (rnum > 1) {

			troom = get_room_index(top_r_vnum - 1);

			for (exitposs = 1; exitposs <= 4; exitposs++) {
				edir = number_range(0, 3);
				xit = get_exit(troom, rev_dir[edir]);
				if (xit) {

					if (edir == 0)
						edir = 1;
					else if (edir == 3)
						edir = 2;
					else {
						if (number_range(1, 2) == 1)
							edir = edir - 1;
						else
							edir = edir + 1;
					}

					xit = make_exit(constnode, troom, edir);
					xit->keyword = STRALLOC( "" );
					xit->description = STRALLOC( "" );
					xit->key = -1;
					xit->exit_info = 0;
					int doorchance = number_range(1, 100);
					if (doorchance > 50 || rnum == 2) {
						SET_BIT( xit->exit_info , EX_ISDOOR );
						SET_BIT( xit->exit_info , EX_CLOSED );
					}
					xit = make_exit(troom, constnode, rev_dir[edir]);
					xit->keyword = STRALLOC( "" );
					xit->description = STRALLOC( "" );
					xit->key = -1;
					xit->exit_info = 0;
					if (doorchance > 50 || rnum == 2) {
						SET_BIT( xit->exit_info , EX_ISDOOR );
						SET_BIT( xit->exit_info , EX_CLOSED );
					}
					exitposs = 5;

				} else {
					xit = make_exit(constnode, troom, edir);
					xit->keyword = STRALLOC( "" );
					xit->description = STRALLOC( "" );
					xit->key = -1;
					xit->exit_info = 0;
					int doorchance = number_range(1, 100);
					if (doorchance > 50 || rnum == 2) {
						SET_BIT( xit->exit_info , EX_ISDOOR );
						SET_BIT( xit->exit_info , EX_CLOSED );
					}
					xit = make_exit(troom, constnode, rev_dir[edir]);
					xit->keyword = STRALLOC( "" );
					xit->description = STRALLOC( "" );
					xit->key = -1;
					xit->exit_info = 0;
					if (doorchance > 50 || rnum == 2) {
						SET_BIT( xit->exit_info , EX_ISDOOR );
						SET_BIT( xit->exit_info , EX_CLOSED );
					}
					exitposs = 5;
				}
			}
		}

	}

	fold_area(pArea, pArea->filename, FALSE );
	reset_all();
	save_char_obj(ch);
	send_to_char("> you connect to your construct\n\r", ch);
	act(AT_GREEN, "> $n connects to their construct", ch, NULL, NULL, TO_ROOM );
	char_from_room(ch);
	char_to_room(ch, get_room_index(ch->pcdata->roomconstruct));
	do_look(ch, "auto");
	return;

}

void do_makeconstructupgrade(CHAR_DATA *ch, char *argument) {

	ROOM_INDEX_DATA * constnode;
	ROOM_INDEX_DATA * location;
	ROOM_INDEX_DATA * troom;
	AREA_DATA * pArea;
	AREA_DATA * tarea;
	EXIT_DATA * xit;
	int rnum, exitposs;
	sh_int edir;

	location = ch->in_room;

	if (IS_NPC(ch) || !ch->pcdata) {
		send_to_char("> invalid command\n\r", ch);
		return;
	}

	if (ch->pcdata->constructlevel == 0) {
		send_to_char("> &Rsorry, but you do not have a construct yet\n\r", ch);
		send_to_char("> &Ruse makeconstruct to get one\n\r", ch);
		return;
	} else if (ch->pcdata->constructlevel == 1) {

		if ( ch->gold < 10000 )
		{
			send_to_char( "> &Rsecond level construct costs 10,000 credits&w\n\r", ch);
			return;
		}

		ch->gold -= 10000;

		ch->pcdata->constructlevel = 2;

		for (rnum = 1; rnum <= 8; rnum++) {

			constnode = make_room(++top_r_vnum);

			if (!constnode) {
				bug("makep: make_room failed", 0);
				return;
			}

			for (tarea = first_area; tarea; tarea = tarea->next) {
				if (!str_cmp(tarea->filename, "construct")) {
					pArea = tarea;
				}
			}

			constnode->area = pArea;

			constnode->owner = STRALLOC( "wilderspace" );
			constnode->description = STRALLOC("a wilderspace node.");
			constnode->name = STRALLOC( "node" );
			constnode->sector_type = SECT_RAINFOREST;
			constnode->level = 1;
			SET_BIT( constnode->room_flags , ROOM_NOPEDIT );

			LINK( constnode , pArea->first_room , pArea->last_room , next_in_area , prev_in_area );

			if (rnum == 1) {
				SET_BIT( constnode->room_flags , ROOM_NO_MOB );
				SET_BIT( constnode->room_flags , ROOM_SAFE );
				constnode->sector_type = SECT_INSIDE;

				STRFREE( constnode->name );
				STRFREE( constnode->description );
				constnode->description = STRALLOC("a safehouse node.");
				constnode->name = STRALLOC( "safehouse" );

				troom = get_room_index(ch->pcdata->roomconstruct + 3);

				for (exitposs = 1; exitposs <= 4; exitposs++) {
					edir = number_range(0, 3);
					xit = get_exit(troom, rev_dir[edir]);
					if (xit) {

						if (edir == 0)
							edir = 1;
						else if (edir == 3)
							edir = 2;
						else {
							if (number_range(1, 2) == 1)
								edir = edir - 1;
							else
								edir = edir + 1;
						}

						xit = make_exit(constnode, troom, edir);
						xit->keyword = STRALLOC( "" );
						xit->description = STRALLOC( "" );
						xit->key = -1;
						xit->exit_info = 0;
						SET_BIT( xit->exit_info , EX_ISDOOR );
						SET_BIT( xit->exit_info , EX_CLOSED );
						xit = make_exit(troom, constnode, rev_dir[edir]);
						xit->keyword = STRALLOC( "" );
						xit->description = STRALLOC( "" );
						xit->key = -1;
						xit->exit_info = 0;
						SET_BIT( xit->exit_info , EX_ISDOOR );
						SET_BIT( xit->exit_info , EX_CLOSED );
						exitposs = 5;

					} else {
						xit = make_exit(constnode, troom, edir);
						xit->keyword = STRALLOC( "" );
						xit->description = STRALLOC( "" );
						xit->key = -1;
						xit->exit_info = 0;
						SET_BIT( xit->exit_info , EX_ISDOOR );
						SET_BIT( xit->exit_info , EX_CLOSED );
						xit = make_exit(troom, constnode, rev_dir[edir]);
						xit->keyword = STRALLOC( "" );
						xit->description = STRALLOC( "" );
						xit->key = -1;
						xit->exit_info = 0;
						SET_BIT( xit->exit_info , EX_ISDOOR );
						SET_BIT( xit->exit_info , EX_CLOSED );
						exitposs = 5;
					}
				}

			}

			if (rnum > 1) {

				troom = get_room_index(top_r_vnum - 1);

				for (exitposs = 1; exitposs <= 4; exitposs++) {
					edir = number_range(0, 3);
					xit = get_exit(troom, rev_dir[edir]);
					if (xit) {

						if (edir == 0)
							edir = 1;
						else if (edir == 3)
							edir = 2;
						else {
							if (number_range(1, 2) == 1)
								edir = edir - 1;
							else
								edir = edir + 1;
						}

						xit = make_exit(constnode, troom, edir);
						xit->keyword = STRALLOC( "" );
						xit->description = STRALLOC( "" );
						xit->key = -1;
						xit->exit_info = 0;
						int doorchance = number_range(1, 100);
						if (doorchance > 50 || rnum == 2) {
							SET_BIT( xit->exit_info , EX_ISDOOR );
							SET_BIT( xit->exit_info , EX_CLOSED );
						}
						xit = make_exit(troom, constnode, rev_dir[edir]);
						xit->keyword = STRALLOC( "" );
						xit->description = STRALLOC( "" );
						xit->key = -1;
						xit->exit_info = 0;
						if (doorchance > 50 || rnum == 2) {
							SET_BIT( xit->exit_info , EX_ISDOOR );
							SET_BIT( xit->exit_info , EX_CLOSED );
						}
						exitposs = 5;

					} else {
						xit = make_exit(constnode, troom, edir);
						xit->keyword = STRALLOC( "" );
						xit->description = STRALLOC( "" );
						xit->key = -1;
						xit->exit_info = 0;
						int doorchance = number_range(1, 100);
						if (doorchance > 50 || rnum == 2) {
							SET_BIT( xit->exit_info , EX_ISDOOR );
							SET_BIT( xit->exit_info , EX_CLOSED );
						}
						xit = make_exit(troom, constnode, rev_dir[edir]);
						xit->keyword = STRALLOC( "" );
						xit->description = STRALLOC( "" );
						xit->key = -1;
						xit->exit_info = 0;
						if (doorchance > 50 || rnum == 2) {
							SET_BIT( xit->exit_info , EX_ISDOOR );
							SET_BIT( xit->exit_info , EX_CLOSED );
						}
						exitposs = 5;
					}
				}
			}

		}
	} else if (ch->pcdata->constructlevel == 2) {

		if ( ch->gold < 20000 )
		{
			send_to_char( "> &Rsecond level construct costs 20,000 credits&w\n\r", ch);
			return;
		}

		ch->gold -= 20000;

		ch->pcdata->constructlevel = 3;

		for (rnum = 1; rnum <= 16; rnum++) {

			constnode = make_room(++top_r_vnum);

			if (!constnode) {
				bug("makep: make_room failed", 0);
				return;
			}

			for (tarea = first_area; tarea; tarea = tarea->next) {
				if (!str_cmp(tarea->filename, "construct")) {
					pArea = tarea;
				}
			}

			constnode->area = pArea;

			constnode->owner = STRALLOC( "wilderspace" );
			constnode->description = STRALLOC("a wilderspace node.");
			constnode->name = STRALLOC( "node" );
			constnode->sector_type = SECT_RAINFOREST;
			constnode->level = 2;
			SET_BIT( constnode->room_flags , ROOM_NOPEDIT );

			LINK( constnode , pArea->first_room , pArea->last_room , next_in_area , prev_in_area );

			if (rnum == 1) {
				SET_BIT( constnode->room_flags , ROOM_NO_MOB );
				SET_BIT( constnode->room_flags , ROOM_SAFE );
				constnode->sector_type = SECT_INSIDE;
				//			ch->pcdata->roomconstruct = top_r_vnum;

				STRFREE( constnode->name );
				STRFREE( constnode->description );
				constnode->description = STRALLOC("a safehouse node.");
				constnode->name = STRALLOC( "safehouse" );

				troom = get_room_index(ch->pcdata->roomconstruct + 11);

				for (exitposs = 1; exitposs <= 4; exitposs++) {
					edir = number_range(0, 3);
					xit = get_exit(troom, rev_dir[edir]);
					if (xit) {

						if (edir == 0)
							edir = 1;
						else if (edir == 3)
							edir = 2;
						else {
							if (number_range(1, 2) == 1)
								edir = edir - 1;
							else
								edir = edir + 1;
						}

						xit = make_exit(constnode, troom, edir);
						xit->keyword = STRALLOC( "" );
						xit->description = STRALLOC( "" );
						xit->key = -1;
						xit->exit_info = 0;
						SET_BIT( xit->exit_info , EX_ISDOOR );
						SET_BIT( xit->exit_info , EX_CLOSED );
						xit = make_exit(troom, constnode, rev_dir[edir]);
						xit->keyword = STRALLOC( "" );
						xit->description = STRALLOC( "" );
						xit->key = -1;
						xit->exit_info = 0;
						SET_BIT( xit->exit_info , EX_ISDOOR );
						SET_BIT( xit->exit_info , EX_CLOSED );
						exitposs = 5;

					} else {
						xit = make_exit(constnode, troom, edir);
						xit->keyword = STRALLOC( "" );
						xit->description = STRALLOC( "" );
						xit->key = -1;
						xit->exit_info = 0;
						SET_BIT( xit->exit_info , EX_ISDOOR );
						SET_BIT( xit->exit_info , EX_CLOSED );
						xit = make_exit(troom, constnode, rev_dir[edir]);
						xit->keyword = STRALLOC( "" );
						xit->description = STRALLOC( "" );
						xit->key = -1;
						xit->exit_info = 0;
						SET_BIT( xit->exit_info , EX_ISDOOR );
						SET_BIT( xit->exit_info , EX_CLOSED );
						exitposs = 5;
					}
				}

			}

			if (rnum > 1) {

				troom = get_room_index(top_r_vnum - 1);

				for (exitposs = 1; exitposs <= 4; exitposs++) {
					edir = number_range(0, 3);
					xit = get_exit(troom, rev_dir[edir]);
					if (xit) {

						if (edir == 0)
							edir = 1;
						else if (edir == 3)
							edir = 2;
						else {
							if (number_range(1, 2) == 1)
								edir = edir - 1;
							else
								edir = edir + 1;
						}

						xit = make_exit(constnode, troom, edir);
						xit->keyword = STRALLOC( "" );
						xit->description = STRALLOC( "" );
						xit->key = -1;
						xit->exit_info = 0;
						int doorchance = number_range(1, 100);
						if (doorchance > 50 || rnum == 2) {
							SET_BIT( xit->exit_info , EX_ISDOOR );
							SET_BIT( xit->exit_info , EX_CLOSED );
						}
						xit = make_exit(troom, constnode, rev_dir[edir]);
						xit->keyword = STRALLOC( "" );
						xit->description = STRALLOC( "" );
						xit->key = -1;
						xit->exit_info = 0;
						if (doorchance > 50 || rnum == 2) {
							SET_BIT( xit->exit_info , EX_ISDOOR );
							SET_BIT( xit->exit_info , EX_CLOSED );
						}
						exitposs = 5;

					} else {
						xit = make_exit(constnode, troom, edir);
						xit->keyword = STRALLOC( "" );
						xit->description = STRALLOC( "" );
						xit->key = -1;
						xit->exit_info = 0;
						int doorchance = number_range(1, 100);
						if (doorchance > 50 || rnum == 2) {
							SET_BIT( xit->exit_info , EX_ISDOOR );
							SET_BIT( xit->exit_info , EX_CLOSED );
						}
						xit = make_exit(troom, constnode, rev_dir[edir]);
						xit->keyword = STRALLOC( "" );
						xit->description = STRALLOC( "" );
						xit->key = -1;
						xit->exit_info = 0;
						if (doorchance > 50 || rnum == 2) {
							SET_BIT( xit->exit_info , EX_ISDOOR );
							SET_BIT( xit->exit_info , EX_CLOSED );
						}
						exitposs = 5;
					}
				}
			}

		}
	} else {
		send_to_char("> &Rthe next construct level is not available yet&w\n\r",
				ch);
		return;
	}

	fold_area(pArea, pArea->filename, FALSE );
	reset_all();
	send_to_char("> construct upgraded\n\r", ch);
	save_char_obj(ch);
	return;

}
