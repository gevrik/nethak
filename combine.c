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


void do_combine(CHAR_DATA * ch ,char *sources )
{

	//verify source1 and source2 existing in inv of ch;
	char arg1[128];
	char arg2[128];
	int ch_snippet = 0;
	int destroy = FALSE;
	int vnum=0;
	int i =0;
	int count_1=0;
	int count_2=0;
	char *name, *description;
	OBJ_DATA *obj;
	OBJ_DATA* source1;
	OBJ_DATA* source2;

	sources = one_argument( sources, arg1 );
	sources = one_argument( sources, arg2 );

	if ( arg1[0] == '\0' || arg2[0] == '\0' )
	    {
		send_to_char( "> specify two objects to combine\n\r", ch );
		return;
	    }

	    if  ( (( source1 = get_obj_world( ch, arg1 )) == NULL)  ||  ((source2 = get_obj_world( ch, arg2 ))  == NULL ) )
	    {
		send_to_char( "> invalid objects\n\r", ch );
		return;
	    }


///////////////////////////////////////////////////////
    for (obj = ch->last_carrying; obj; obj = obj->prev_content) {
            if (!strcmp(obj->name, source1->name))
				if(ch_snippet ==0 ) 
					ch_snippet += 1;
	}

    for (obj = ch->last_carrying; obj; obj = obj->prev_content) {
            if (!strcmp(obj->name, source2->name))
            	if (ch_snippet == 1 ) 
            		ch_snippet += 1;
    	}

    if (ch_snippet<2) {
            send_to_char("> &RYou don't have those items&w\n\n\r", ch);
            return;
    	}
		
//////////////////////////////////////////
	
	
	//message of beginning;

 	send_to_char("> &RLet's see what you can do with those stuffs ...\n\n\n\r", ch);
	WAIT_STATE( ch, skill_table[gsn_propaganda]->beats );


	switch(source1->pIndexData->vnum)
	{

		case OBJ_VNUM_SLAG :
			switch(source2->pIndexData->vnum)
				{
				case  OBJ_VNUM_SLAG:
					destroy=TRUE;
					vnum=OBJ_VNUM_FRAGMENT;
					if(strstr(source1->description,"video") && strstr(source2->description,"video"))
					{
						send_to_char("&GYou've created a fragment of video\n\n\r", ch);
					}
					else if (strstr(source1->description,"picture") && strstr(source2->description,"picture"))
					{
						send_to_char("&GYou've created a fragment of picture\n\n\r", ch);						
					}
					else 
					{	
						destroy=FALSE;
						send_to_char("&R... dammit ! those aren't compatible\n\n\r", ch);
					}
										
				break;

				case OBJ_VNUM_FRAGMENT:
					destroy=TRUE;
					vnum=OBJ_VNUM_FILE;
					if(strstr(source1->description,"video") && strstr(source2->description,"video"))
					{
						send_to_char("&GYou've created a video\n\n\r", ch);
					}
					else if (strstr(source1->description,"picture") && strstr(source2->description,"picture"))
					{
						send_to_char("&GYou've created a picture\n\n\r", ch);						
					}
					else 
					{	
						destroy=FALSE;
						send_to_char("&R... dammit ! those aren't compatible\n\n\r", ch);
					}

				break;
				
				default : break;
				}
		break;


		case OBJ_VNUM_FRAGMENT :
			switch(source2->pIndexData->vnum)
				{
				case OBJ_VNUM_SLAG:
					destroy=TRUE;	
					vnum=OBJ_VNUM_FILE;
					if(strstr(source1->description,"video") && strstr(source2->description,"video"))
					{
						send_to_char("&GYou've created a  video\n\n\r", ch);
					}
					else if (strstr(source1->description,"picture") && strstr(source2->description,"picture"))
					{
						send_to_char("&GYou've created a picture\n\n\r", ch);						
					}
					else 
					{	
						destroy=FALSE;
						send_to_char("&R... dammit ! those aren't compatible\n\n\r", ch);
					}		
				break;

				case OBJ_VNUM_FRAGMENT:
					destroy=TRUE;
					vnum=OBJ_VNUM_FILE;
					if(strstr(source1->description,"video") && strstr(source2->description,"video"))
					{
						send_to_char("&GYou've created a  video\n\n\r", ch);
					}
					else if (strstr(source1->description,"picture") && strstr(source2->description,"picture"))
					{
						send_to_char("&GYou've created a picture\n\n\r", ch);						
					}
					else 
					{	
						destroy=FALSE;
						send_to_char("&R... dammit ! those aren't compatible\n\n\r", ch);
					}
				break;
				default:break;
				}
		break;

		case OBJ_VNUM_PWD :
			switch(source2->pIndexData->vnum)
				{
				case OBJ_VNUM_LOGIN:
					destroy=TRUE;
					vnum=OBJ_VNUM_ID;
					
					for(i=0; &source1->description[i]=="/0"; i++)
						{
						count_1+=atoi(description[i]);
						}
					for(i=0; &source2->description[i]=="/0"; i++)
						{
						count_2+=atoi(description[i]);
						}
					
					if((count_2%2)==(count_1%2))
						send_to_char("&GYou've created an identification\n\n\r", ch);
					else
					{	
						destroy=FALSE;
						send_to_char("&R... dammit ! those aren't compatible\n\n\r", ch);
					}
					
				break;
				default: break;
				}
		break;

		case OBJ_VNUM_LOGIN : 
			switch(source2->pIndexData->vnum)
				{
				case OBJ_VNUM_PWD: 
					destroy=TRUE;
					vnum=OBJ_VNUM_ID;
					send_to_char("&GYou've created an identification\n\n\r", ch);
				break;
				default: break;
				}
		break;
		
		/*case OBJ_VNUM_TOKEN : 
			switch(source2->pIndexData->vnum)
				{
				case OBJ_VNUM_TOKEN: 
					destroy=TRUE;
					vnum=OBJ_VNUM_TOKEN;
					send_to_char("&GYou've created a super token\n\n\r", ch);
				break;
				default: break;
				}
		break;*/

		case OBJ_VNUM_BLOB: 
			destroy=TRUE;
			send_to_char("&GGeez ! It just eat all and run away !\n\n\r", ch);
		break;


		default : // items non combinable

			 send_to_char("> &R... nothing ! &w\n\n\r", ch); 
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
			//put description etc
			obj_to_char(obj,ch);//put it in inv of char
		}

	}

}
