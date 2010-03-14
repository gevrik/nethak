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

	//	if (!IS_SET( location->room_flags , ROOM_SAFE )) {
	//		send_to_char("> you do not seem to be in a wilderspace portal\n\r", ch);
	//		return;
	//	}

//	if (ch->pcdata->roomconstruct) {
//		send_to_char("> &Rsorry, but you already have a construct\n\r", ch);
//		return;
//	}

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
		constnode->sector_type = SECT_GLACIAL;

		LINK( constnode , pArea->first_room , pArea->last_room , next_in_area , prev_in_area );

//		if (rnum == 1)
//			ch->pcdata->roomconstruct = top_r_vnum;

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

	}

	//save_char_obj(ch);
	return;

}
