#include <stdio.h>
#include <stdlib.h>
#include "mud.h"
#include <string.h>

/*
 
Explanation of ship template language:
  Ship Template Language is a way to copy premade rooms into a specified layout. Each copied room
  MUST be from a different premade room.  IE: You cannot use premade room 14 in the same layout twice.
  You can reuse premade rooms in different layouts.  Whitespace is ignored.

  Use by example
  When multiple strings are accepted the more verbose methods will be listed first

  To create a 1 room fighter from premade room 8.
  "1] 8)" or "1] 8"
  The '1]' specifies that this is a single room layout.
  The '8' or '8)' both specify using premade room 8 with no exits.

  To create a 2 room fighter from premade rooms 8 and 9 with a 2-way exit going from room 8 north to room 9.  
  Example 1: "2] 9; 8)20:9"
  The '2]' specifies that this is a 2 room layout
  The '9' specifies that we use premade room 9
  The ';' specifies that we are no longer in room 9
  The '8)' specifies that we use premade room 8 and it will be followed by a list of exits
  The '20:9' specifies a northern 2-way exit between rooms 8 and 9, start at 8 and going to 9
  Example 2: "2] 8)20:9"
  This example does the same thing as Example 1 but uses an implicit creation of room 9.  Room 9 will
  still be created even though it was not explicitly listed as a room to create.

  To create a 3 room fighter from premade rooms 8, 9, and 10.
  8 links north to 9, 9 links north to 10.
  "3] 8; 10; 9)20:10,22:8"
  The ',' specifies that room 9 will have a second exit.
  Alternate method using implicit creation of rooms:
  "3] 9)20:10,22:8"
   
  Use of implicit creation of rooms is encouraged because it actually takes less processing time.  In fact, the
  shorter the string is the faster it will be processed.

  For further help talk to Kre.
*/

int lookupstrtype( int rtype ) // looks up the type of the ship template room - DV 3-20-03
{
  switch ( rtype )
  {
    case 1: return STRTYPE_PILOTSEAT;
    case 2: return STRTYPE_ENTRANCE;
    case 3: return STRTYPE_COSEAT;
    case 4: return STRTYPE_ENGINEROOM;
    case 5: return STRTYPE_NAVSEAT;
    case 6: return STRTYPE_GUNSEAT;
    case 7: return STRTYPE_HANGAR;
    case 8: return STRTYPE_COCKPIT;
    case 12: return STRTYPE_COCKPIT;
    case 61: return STRTYPE_ENTRANCE;
    default: return STRTYPE_DEFAULT; // Hallways, cabins, etc... anything that isn't set to the ship structure - DV 3-20-03
  }
  return STRTYPE_DEFAULT;
}

bool setroomtoship( SHIP_DATA *ship, ROOM_INDEX_DATA *room, int strtype )
{
  switch(strtype)
  {
    case STRTYPE_PILOTSEAT: ship->pilotseat = room->vnum; break;
    case STRTYPE_ENTRANCE: ship->entrance = room->vnum; break;
    case STRTYPE_COSEAT: ship->coseat = room->vnum; break;
    case STRTYPE_ENGINEROOM: ship->engineroom = room->vnum; break;
    case STRTYPE_NAVSEAT: ship->navseat = room->vnum; break;
    case STRTYPE_GUNSEAT: ship->gunseat = room->vnum; break;
    case STRTYPE_HANGAR: ship->hanger = room->vnum; break;
    case STRTYPE_COCKPIT: 
      ship->cockpit = room->vnum;
      ship->coseat = room->vnum;
      ship->navseat = room->vnum;
      ship->gunseat = room->vnum;
      ship->pilotseat = room->vnum;
    case STRTYPE_DEFAULT: break;
    default: return FALSE; break;
  }
  return TRUE;
}

/**********************************************************
Function: make_template_room - Kre
	  Creates a room and sets its attributes
Arguments:
	rtype - the room which will be copied as the original  
	ship  - the ship the room is being created for - DV 3-20-03
	roomnum - what number room this is in the template - Used to create unique vnums - DV 3-20-03
	
**********************************************************/

ROOM_INDEX_DATA *make_template_room(int rtype, SHIP_DATA *ship, int roomnum) 
{
    ROOM_INDEX_DATA *room, *templateroom;
    int strtype, strvnum, roomvnum;
    
    // STR_AREAVNUM is set in mud.h.  This is the first room of the area in which the template rooms reside - DV 3-20-03    
    strvnum = STR_AREAVNUM+rtype; 
    
    //strtype is the type of room it is... engineroom, etc - DV 4-15-03
    strtype = lookupstrtype( rtype );

    // Hopefully this produces a unique vnum for every virtual room.. if not, we may have to play with it some more - DV 3-20-03    
    // Alright, so that formula didn't work... so I'm going for a straight sequencial system.
    roomvnum    = (ship->shipID*100) + roomnum;
    
    room = make_room( roomvnum );
    
    templateroom = get_room_index( strvnum );
    
    if ( !templateroom )
    {
      bug( "make_template_room: no template room exists.\n\r", 0 );
      return NULL;
    }
    
    
    room->area    	= templateroom->area;
    room->sector_type 	= templateroom->sector_type;
    room->room_flags  	= templateroom->room_flags;
    room->name 		= STRALLOC( templateroom->name );
    room->description 	= STRALLOC( templateroom->description );
    room->tunnel 	= templateroom->tunnel;
    if ( !setroomtoship( ship, room, strtype ) )
    {
      bug( "make_template_room: setroomtoship failed.\n\r", 0 );
      return NULL;
    }

    return room;
}

int create_exit( int location, int evnum, int edir )
{
  EXIT_DATA *xit = NULL;
  EXIT_DATA *texit;
  ROOM_INDEX_DATA *tmp;
  
    if ( edir < 0 )
      return 1;
      
//  if ( (xit = get_exit_num(get_room_index(location), edir)) != NULL )
//    edir = xit->vdir;

	if ( (tmp = get_room_index( evnum )) == NULL )
	{

	    return 1;
	}
	if ( !xit )
	{
	    if ( xit && get_exit_to(get_room_index(location), edir, tmp->vnum) )
	    {
		return 1;
	    }
	    xit = make_exit( get_room_index(location), tmp, edir );
	    xit->keyword		= STRALLOC( "" );
	    xit->description		= STRALLOC( "" );
	    xit->key			= -1;
	    xit->exit_info		= 0;
	}
	if ( xit->to_room != tmp )
	{
	    xit->to_room = tmp;
	    xit->vnum = evnum;
	    texit = get_exit_to( xit->to_room, rev_dir[edir], get_room_index(location)->vnum );
	    if ( texit )
	    {
		texit->rexit = xit;
		xit->rexit = texit;
	    }
	}
	return 0;
}

int reversedir( int dir )
{
  switch( dir )
  {
    case DIR_NORTH: return DIR_SOUTH;
    case DIR_EAST: return DIR_WEST;
    case DIR_SOUTH: return DIR_NORTH;
    case DIR_WEST: return DIR_EAST;
    case DIR_UP: return DIR_DOWN;
    case DIR_DOWN: return DIR_UP;
    case DIR_NORTHEAST: return DIR_SOUTHWEST;
    case DIR_NORTHWEST: return DIR_SOUTHEAST;
    case DIR_SOUTHEAST: return DIR_NORTHWEST;
    case DIR_SOUTHWEST: return DIR_NORTHEAST;
    default: return -1;
  }
  return -1;
}

int join_template_rooms(int room1, ROOM_INDEX_DATA *room2, int exitType)
{
    int success = 0, dir;
    bool twoway;
//  char buf[MAX_INPUT_LENGTH];
    
    twoway = FALSE;
    
    switch(exitType)
    {
	case 10: dir = DIR_NORTH; 	twoway = FALSE; break; // 1-way north
	case 11: dir = DIR_EAST; 	twoway = FALSE; break; //1-way east 
	case 12: dir = DIR_SOUTH; 	twoway = FALSE; break; //1-way south
	case 13: dir = DIR_WEST; 	twoway = FALSE; break; //1-way west
	case 14: dir = DIR_NORTHEAST; 	twoway = FALSE; break; //1-way northeast
	case 15: dir = DIR_NORTHWEST; 	twoway = FALSE; break; //1-way northwest
	case 16: dir = DIR_SOUTHEAST; 	twoway = FALSE; break; //1-way southeast
	case 17: dir = DIR_SOUTHWEST; 	twoway = FALSE; break; //1-way southwest
	case 18: dir = DIR_UP; 		twoway = FALSE; break; //1-way up 
	case 19: dir = DIR_DOWN; 	twoway = FALSE; break; //1-way down 
	case 20: dir = DIR_NORTH; 	twoway = TRUE; break; //2-way north 
	case 21: dir = DIR_EAST; 	twoway = TRUE; break; //2-way east 
	case 22: dir = DIR_SOUTH; 	twoway = TRUE; break; //2-way south 
	case 23: dir = DIR_WEST; 	twoway = TRUE; break; //2-way west 
	case 24: dir = DIR_NORTHEAST; 	twoway = TRUE; break; //2-way northeast 
	case 25: dir = DIR_NORTHWEST; 	twoway = TRUE; break; //2-way northwest 
	case 26: dir = DIR_SOUTHEAST; 	twoway = TRUE; break; //2-way southeast 
	case 27: dir = DIR_SOUTHWEST; 	twoway = TRUE; break; //2-way southwest 
	case 28: dir = DIR_UP; 		twoway = TRUE; break; //2-way up 
	case 29: dir = DIR_DOWN; 	twoway = TRUE; break; //2-way down 
	default: return 1;
    }
    
//sprintf( buf, "Room1: %d Room2: %d exitType: %d\n\r", room1, room2->vnum, exitType );
//bug ( buf, 0 );
    
    if( twoway )
    {
      success += create_exit( room1, (room2->vnum), dir );
      success += create_exit( (room2->vnum), room1, reversedir(dir) );
    }
    else
      success += create_exit( room1, (room2->vnum), dir );
    
    if ( success > 0 )
      return 1;
      
    return 0;
}

int destroy_vnums(ROOM_INDEX_DATA *rooms[], int size)
{
    int i;
    for(i = 0; i < size; i++) 
    {
    	if ( rooms[i] )
	delete_room( rooms[i] );
	free(rooms[i]);
    }
    return 1;
}
int parse_ship_template(char *string, SHIP_DATA *ship)
{
    #define PST_BUF_SIZE 15
    char buf[PST_BUF_SIZE];
//  char debug[MAX_STRING_LENGTH];
//  char buf2[PST_BUF_SIZE];
    int masterIndex;
    int index;
    int bindex;
    int roomCount;
    int *roomArray;
    ROOM_INDEX_DATA *vnumArray[100];
    int i, j;
    short start;
    int room;
    int room2;
    
//  sprintf( buf, "Parser activated: String: %s, Ship: %s", string, ship->personalname );
//  bug( buf, 0 );
    
    masterIndex=0;
    //Parse out the number of rooms
    bindex=0;
    while(string[masterIndex] != ']' && masterIndex < (int) strlen(string) && bindex < PST_BUF_SIZE)
    {
	buf[bindex] = string[masterIndex];
	masterIndex++;
	bindex++;
    }
    buf[bindex] = '\0';
    if (masterIndex == (int) strlen(string) || bindex == PST_BUF_SIZE) {
	return 1;
    }
    masterIndex++;
    roomCount = atoi(buf);
    roomArray = (int*)calloc(roomCount,sizeof(int));
//    vnumArray = (ROOM_INDEX_DATA*)calloc(roomCount,sizeof(ROOM_INDEX_DATA));

    //Find all the room numbers
    index = masterIndex-1;
    i = 0;
    start = 1;
    bindex = 0;
    while(i < roomCount && index < (int) strlen(string)) {
	index++;
	if (string[index] == ':') {
	    start = 1;
	    continue;
	}
	if ((string[index] == ',' || string[index] == '\0' || string[index] == ')' || string[index] == ';') && start) {
	    buf[bindex]='\0';
	    roomArray[i] = atoi(buf);
	    //Check for duplicates
	    for(j = 0; j < i; j++) {
		if (roomArray[j] == roomArray[i]) {
		    break;
		}
	    }
	    start = 0;
	    bindex = 0;
	    if (j >= i) {
		i++;          //Only increment if we want to keep this number
	    }
	    if (string[index] == ';') {
		start = 1;
	    }
	    continue;
	}
        if (string[index] < '0' && string[index] > '9')
	    continue;  //haven't hit a number yet
	if (start == 1) {
	    buf[bindex] = string[index];
	    bindex++;
	}
    }
    if (i != roomCount) {
	free(roomArray);
//	clearrooms(vnumArray);
	return 2;
    }
    for(i = 0; i < roomCount; i++) {
	vnumArray[i] = make_template_room(roomArray[i], ship, i);

        if( !(vnumArray[i]) )
          continue;


        if( i == 0 )
        {
          ship->entrance      = ((vnumArray[i])->vnum);
          ship->pilotseat     = ((vnumArray[i])->vnum);
          ship->coseat        = ((vnumArray[i])->vnum);
          ship->cockpit       = ((vnumArray[i])->vnum);
          ship->engineroom    = ((vnumArray[i])->vnum);
          ship->navseat       = ((vnumArray[i])->vnum);
          ship->gunseat       = ((vnumArray[i])->vnum);
          ship->lastroom      = roomCount;
//        ship->cockpitroom   = get_room_index((vnumArray[i])->vnum);
        }

//	printf("Make room type: %d, Vnum: %d\n",roomArray[i],vnumArray[i]->vnum);
    }

  ship->firstroom = ship->shipID*100;
  ship->lastroom += ship->firstroom-1;

/*  for( i = 0; i < roomCount; i++ )
    {
    	sprintf(debug, "Rooms: Tmp#: %d Vnum: %d\n\r", roomArray[i], vnumArray[i]->vnum );
    	bug( debug, 0 );
    }
*/

    //Create Exits
    index = masterIndex-1;
    i = 0;
    start = 0;
    bindex = 0;
    room = -1;
    while(/*i < roomCount && */index < (int) strlen(string)) {
	index++;
	if (string[index] == ')' && !start) {
	    room = atoi(buf);
	    for(j = 0; j < roomCount; j++) {
		if ( ( roomArray[j] == room ) && vnumArray[j] ) 
		{
//		    sprintf( debug, "Tmp#: %d Vnum: %d\n\r", room, vnumArray[j]->vnum );
//		    bug( debug, 0 );
		    room = vnumArray[j]->vnum;
		    bindex = 0;
		    break;
		}
	    }
	    if (j == roomCount) {
	    	bug( "No room found for exit.\n\r", 0 );
		destroy_vnums(vnumArray,roomCount);
//		free(vnumArray);
		free(roomArray);
		return 5;
	    } 
	}
	
	//If a ',' or ')' appears in the string, start capturing the exitType - DV 4-20-03
	if ((string[index] == ',' || string[index] == ')') && start == 0 && room > -1) {
	    start = 1;
	    continue;
	}
	//Once start = 1, it is capturing the exitType... when next a separate appears,
	//capture the room template number.
	if (start && ((string[index] == ',' || string[index] == '\0') || string[index] == ';')) 
	{
	    buf[bindex]='\0';
	    
	    //Brings buf to capture the room template number DV- 4-20-03
	    j = 0;
	    while(buf[j]!=':') {
		if (j >= bindex) {
		    //Some error- really have no idea what :) DV - 4-20-03
		    destroy_vnums(vnumArray,roomCount);
//		    free(vnumArray);
		    free(roomArray);
		    return 3;
		}
		j++;
	    }
	    buf[j] = '\0';
	    room2 = atoi((char*)buf+j+1);
//	    sprintf( debug, "Buf: %s Room2: %d\n\r", buf, room2 );
//	    bug( debug, 0 );
	    for(j = 0; j < roomCount; j++) {
		if (roomArray[j] == room2) {
		    if (join_template_rooms(room,vnumArray[j],atoi(buf))) {
			destroy_vnums(vnumArray,roomCount);
//			free(vnumArray);
			free(roomArray);
			return 6;
		    }
		    break;
		}
	    }
	    if (j == roomCount)
	    {
		printf("searching for: %d\n",room2);
		destroy_vnums(vnumArray,roomCount);
//		free(vnumArray);
		free(roomArray);
		return 4;
	    }
	    start = 0;	    
	    bindex = 0;
	    i++;  
	    if (string[index] == ',')
	    	index--;
	    continue;
	}
	if (string[index] == ';') {
	    room = -1;
	    start = 0;
	}
        if (string[index] < '0' && string[index] > '9' && string[index] != ':')
	    continue;  //haven't hit a number yet
	buf[bindex] = string[index];
	bindex++;
	buf[bindex] = '\0';
    }
//  free(vnumArray);
    free(roomArray);
    return 0;
}


/*

int main()
{

    printf("Exit Types\n");
    printf("1-Way\n");
    printf("10=north\n11=east\n12=south\n13=west\n14=northeast\n15=southeast\n16=southwest\n17=northwest\n18=up\n19=down\n");
    printf("2-Way\n");
    printf("20=north\n21=east\n22=south\n23=west\n24=northeast\n25=southeast\n26=southwest\n27=northwest\n28=up\n29=down\n");

    printf("------\nShip 1\n");
    printf("Return: %d\n------\n",parse_ship_template("4] 100)12:101,13:102;102)10:103"));
    printf("------\nShip 2\n");
    printf("Return: %d\n------\n",parse_ship_template("1] 100"));
    printf("------\nShip 3\n");
    printf("Return: %d\n------\n",parse_ship_template("10] 8)20:100,22:13;201)20:3,21:13,22:15,23:12;115)21:3,23:110;1)20:15")); 
  return 0;
}

*/








