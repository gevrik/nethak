/*
 * combine.c
 *
 *  Created on: 24 août 2010
 *      Author: aiseant
 */

#include <math.h>
#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "mud.h"


void do_nawak(CHAR_DATA * ch ,OBJ_DATA * source1, OBJ_DATA* source2 )
{

	//verify source1 and source2 existing in inv of ch;
	
	int ch_snippet = 0;
	int destroy = FALSE;
	int vnum=0;
	char *name, *description;
	OBJ_DATA *obj;

///////////////////////////////////////////////////////
    for (obj = ch->last_carrying; obj; obj = obj->prev_content) {
            if (!strcmp(obj->name, source1->name) && ch_snippet ==0 ) ch_snippet += 1;
	}

    for (obj = ch->last_carrying; obj; obj = obj->prev_content) {
            if (!strcmp(obj->name, source2->name) && ch_snippet == 1 ) ch_snippet += 1;
    	}

    if (ch_snippet<2) {
            send_to_char("> &RSource items needed&w\n\r", ch);
            return;
    	}
		
//////////////////////////////////////////
	
	
	//message of beginning;

 send_to_char("> &RLet's see what you can do with those stuffs ...&w\n\r", ch);



	switch(source1->pIndexData->vnum)
	{

		case 115 ://115
			switch(source2->pIndexData->vnum)
				{
				case  115://115
					destroy=TRUE;
					//create FRAGMENT;/116
					vnum=116;
				break;

				case 116://116
					destroy=TRUE;
					//create FILES;//120
					vnum=120;
				break;
				default : break;
				}
		break;


		case 116 ://116
			switch(source2->pIndexData->vnum)
				{
				case 115:
					destroy=TRUE;					
					//create FILE;//117
					vnum=117;
				break;

				case 116://116
					destroy=TRUE;
					//create FILES;//117
					vnum=117;
				break;
				default:break;
				}
		break;

		case 118 ://118
			switch(source2->pIndexData->vnum)
				{
				case 119://119
					destroy=TRUE;
					//create IDENTIFICATION;//120
					vnum=120;
				break;
				default: break;
				}
		break;

		case 119 : //119
			switch(source2->pIndexData->vnum)
				{
				case 118: //118
					destroy=TRUE;
					//create IDENTIFICATION;//120
					vnum=120;
				break;
				default: break;
				}
		break;

		case 121: //121
			destroy=TRUE;
			send_to_char("Geez ! He just eat all and run away !", ch);
		break;


		default : // items non combinable
			 send_to_char("> &R... nothing ! &w\n\r", ch); 
		break;

	}

//////////// DESTROYING THE SOURCE ITEMS ///////////////
	if(destroy)
	{
		ch_snippet=FALSE;

		for (obj = ch->last_carrying; obj; obj = obj->prev_content) {
		                if (!strcmp(obj->name,source1->name) && ch_snippet == FALSE) 
				{
		                        ch_snippet = TRUE;
		                        separate_obj(obj);
		                        obj_from_char(obj);
		                        extract_obj( obj );
		                }
		        }

		ch_snippet=FALSE;

		for (obj = ch->last_carrying; obj; obj = obj->prev_content) {
		                if (!strcmp(obj->name,source2->name) && ch_snippet == FALSE) 
				{
		                        ch_snippet = TRUE;
		                        separate_obj(obj);
		                        obj_from_char(obj);
		                        extract_obj( obj );
		                }
		        }

		if(vnum!=0)
		{
			obj= create_object(get_obj_index(vnum),0);//create item	
			obj_to_char(obj,ch);//put it in inv of char
			//put description etc
		}

	}

}
