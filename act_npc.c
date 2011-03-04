/*
 * ACT_NPC.c
 *
 *  Created on: september 29th 2010
 *      Author: aiseant
 */
 
 #include <sys/types.h>
#include <sys/time.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include "mud.h"


void npc_sleep(CHAR_DATA* mob, int time)
{
  MPSLEEP_DATA *mpsleep = NULL;

	CREATE(mpsleep, MPSLEEP_DATA, 1);
	
	 /* State variables */
	
	/* mpsleep->ignorelevel = ignorelevel;
	 mpsleep->iflevel = iflevel;
	 for (count = 0; count < MAX_IFS; count++)
	 {
	 	for (count2 = 0; count2 < DO_ELSE; count2++)
		 {
	 		mpsleep->ifstate[count][count2] =
	 		ifstate[count][count2];
	 	}
	 }
 	}*/
	 /* Driver arguments */
	// mpsleep->com_list = STRALLOC(command_list);
	 mpsleep->mob = mob;
	 //mpsleep->actor = actor;
	// mpsleep->obj = obj;
	// mpsleep->vo = vo;
	// mpsleep->single_step = single_step;

	 /* Time to sleep */
	// cmnd = one_argument( cmnd, arg);
	// if (cmnd[0] == '\0')
	 mpsleep->timer = time;
	// else
	// mpsleep->timer = atoi(cmnd);;

	 /*if (mpsleep->timer < 1)
	 {
	 progbug("mpsleep - bad arg, using default", mob);
	 mpsleep->timer = 4;
	 }*/

	 /* Save type of prog, room, object or mob */

	/* if (mpsleep->mob->pIndexData->vnum == 3)
	 {
	 	if (!str_prefix("Room", mpsleep->mob->description))
	 	{
	 	mpsleep->type = MP_ROOM;
	 	mpsleep->room = mpsleep->mob->in_room;
	 	}
	 	else if (!str_prefix("Object", mpsleep->mob->description))
		mpsleep->type = MP_OBJ;
	 }
	 else*/
		 mpsleep->type = MP_MOB;

	 LINK(mpsleep, first_mpsleep, last_mpsleep, next, prev);
	
}
