#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include "mud.h"

void do_sn_jackhammer(CHAR_DATA *ch, char *argument) {
	OBJ_DATA *obj;
	int chance;
	EXIT_DATA *xit, *texit;
	int edir;
	ROOM_INDEX_DATA *location;
	char buf[MAX_STRING_LENGTH];
	PLANET_DATA *planet;
	bool ch_snippet;

	planet = ch->in_room->area->planet;

	if (IS_NPC(ch) || !ch->pcdata || !ch->in_room)
		return;

	if (IS_SET( ch->in_room->room_flags , ROOM_NOPEDIT )) {
		send_to_char("> you cannot use this app here\n\r", ch);
		return;
	}

	if (argument[0] == '\0') {
		send_to_char("> syntax: jackhammer <direction>\n\r", ch);
		return;
	}

	location = ch->in_room;
	edir = get_dir(argument);
	xit = get_exit(ch->in_room, edir);

	if (!xit) {
		send_to_char("> &Rno connection in that direction&w\n\r", ch);
		return;
	}

	if (!IS_SET( xit->exit_info , EX_ISDOOR )) {
		send_to_char("> &Rno codegate in that direction&w\n\r", ch);
		return;
	} else {

		ch_snippet = FALSE;

		for (obj = ch->last_carrying; obj; obj = obj->prev_content) {
			if (obj->item_type == ITEM_SNIPPET && !strcmp(obj->name,
					"jackhammer")) {
				ch_snippet = TRUE;
				separate_obj(obj);
				obj_from_char(obj);
			}
		}

		if (!ch_snippet) {
			send_to_char("> &Rjackhammer application needed&w\n\r", ch);
			return;
		}

		chance = (int) (ch->pcdata->learned[gsn_spacecraft]);
		if (number_percent() > chance) {
			send_to_char("> &Ryou fail to execute the application&w\n\r", ch);
			return;
		}

		sprintf(buf, "> %s's jackhammer breaks down the gate to the: %s",
				ch->name, dir_name[edir]);
		echo_to_room(AT_RED, ch->in_room, buf);
		REMOVE_BIT( xit->exit_info , EX_ISDOOR );
		REMOVE_BIT( xit->exit_info , EX_LOCKED );
		REMOVE_BIT( xit->exit_info , EX_CLOSED );
		xit->key = -1;
		texit = get_exit_to(xit->to_room, rev_dir[edir], ch->in_room->vnum);
		if (texit) {
			REMOVE_BIT( texit->exit_info , EX_ISDOOR );
			REMOVE_BIT( texit->exit_info , EX_LOCKED );
			REMOVE_BIT( texit->exit_info , EX_CLOSED );
			texit->key = -1;
		}

	}

	learn_from_success(ch, gsn_spacecraft);

	SET_BIT( ch->in_room->area->flags , AFLAG_MODIFIED );

}

//done for Neuro
