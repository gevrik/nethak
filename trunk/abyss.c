#include <math.h>
#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "mud.h"

extern int top_r_vnum;

void followconst( CHAR_DATA *ch , char *argument );

void do_probe ( CHAR_DATA *ch , char *argument )
{
	CLAN_DATA * clan;
	int chance;
	EXIT_DATA * xit;
	EXIT_DATA * xit2;
	sh_int roomdistance;
	int edir, roomtype, newareatype, roomsize, roomlevel, exitposs;
	int roommaterial, randomdescription, roomwidth, roomlength, roomheight, roomnoise, roomwalls;
	ROOM_INDEX_DATA *nRoom;
	ROOM_INDEX_DATA *lRoom;
	char buf[MAX_STRING_LENGTH];
	char bufrname[MAX_STRING_LENGTH];
	char bufrdesc[MAX_STRING_LENGTH];
	//PLANET_DATA * dPlanet = NULL;
	PLANET_DATA *planet;
	bool islevelchange = FALSE;
	bool hasexit = FALSE;
	int cost;

	if ( !IS_NPC(ch) && !ch->pcdata->learned[gsn_spacecraft] )
	{
		send_to_char("> you do not know how to probe\n\r", ch );
		return;
	}

	if ( str_cmp(ch->in_room->area->planet->name, "metropolis") )
	{
		send_to_char( "> &Ryou can not probe in this system&w\n\r", ch );
		return;
	}

	cost = ( ch->in_room->level + 1 ) * 10;

	if (ch->gold < cost)
	{
		ch_printf( ch, "> &Ryou need %d credits to probe here&w\n\r", cost );
		return;
	}

	if ( IS_NPC(ch) || !ch->pcdata || !ch->in_room )
		return;

	clan = ch->pcdata->clan;
	planet = ch->in_room->area->planet;

	if ( str_cmp( argument, "north" )
			&& str_cmp( argument, "south" )
			&& str_cmp( argument, "west" )
			&& str_cmp( argument, "east" )
			&& str_cmp( argument, "n")
			&& str_cmp( argument, "e")
			&& str_cmp( argument, "s")
			&& str_cmp( argument, "w"))
	{
		send_to_char( "> &Ryou cannot probe into that direction, try:\n\r&w", ch);
		send_to_char( "north, east, south, west\n\r", ch);
		return;
	}

	if( !IS_IMMORTAL(ch) )
	{
		if ( IS_SET( ch->in_room->room_flags , ROOM_NOPEDIT ) )
		{
			send_to_char( "> &Ryou may not probe in this node&w\n\r", ch );
			return;
		}

		if ( IS_SET( ch->in_room->room_flags2 , ROOM_DEADEND ) )
		{
			send_to_char( "> &Rthis node is a dead-end&w\n\r", ch );
			return;
		}

	}

	if ( argument[0] == '\0' )
	{
		send_to_char( "> &Yspecify direction for probe&w\n\r", ch );
		return;
	}

	edir = get_dir(argument);
	xit = get_exit(ch->in_room, edir);
	if ( xit )
	{
		send_to_char( "> &Rthere is already a node in that direction&w\n\r", ch );
		return;
	}

	chance = (int) (ch->pcdata->learned[gsn_spacecraft]);
	if ( number_percent( ) > chance )
	{
		send_to_char( "> &Ryou fail to probe - try again&w\n\r", ch );
		return;
	}

	ch->gold -= cost;
	ch_printf( ch, "> &Wyou spent %d credits probing into a new node&w\n\r", cost );

	roomlevel = ch->in_room->level;

	// determine room type

	nRoom = make_room( ++top_r_vnum );
	nRoom->area = ch->in_room->area;
	LINK( nRoom , ch->in_room->area->first_room , ch->in_room->area->last_room , next_in_area , prev_in_area );
	STRFREE( nRoom->name );
	STRFREE( nRoom->description );
	nRoom->level = roomlevel;

	newareatype = number_range(1, 100);

	switch (ch->in_room->sector_type) {

	default:

		break;

	case SECT_BREEDING:

		// breeding 20
		// city 35
		// corridor 5
		// factory 5
		// farm 5
		// hall 10
		// ruins 10
		// tunnel 10

		if (newareatype >= 80){
			roomtype = 1;
		}
		else if (newareatype >= 45) {
			roomtype = 5;
		}
		else if (newareatype >= 40) {
			roomtype = 6;
		}
		else if (newareatype >= 35) {
			roomtype = 8;
		}
		else if (newareatype >= 30) {
			roomtype = 9;
		}
		else if (newareatype >= 20) {
			roomtype = 10;
		}
		else if (newareatype >= 10) {
			roomtype = 15;
		}
		else {
			roomtype = 20;
		}

		break;

	case SECT_BRIDGE:

		// bridge 25
		// camp 15
		// chasm 10
		// city 5
		// hall 20
		// platform 15
		// settlement 5
		// tower 5

		if (newareatype >= 75){
			roomtype = 2;

		}
		else if (newareatype >= 60) {
			roomtype = 3;

		}
		else if (newareatype >= 50) {
			roomtype = 4;

		}
		else if (newareatype >= 45) {
			roomtype = 5;

		}
		else if (newareatype >= 25) {
			roomtype = 10;
		}
		else if (newareatype >= 10) {
			roomtype = 12;

		}
		else if (newareatype >= 5) {
			roomtype = 16;
		}
		else {
			roomtype = 19;
		}

		break;

	case SECT_CAMP:

		// bridge 10
		// camp 25
		// chasm 5
		// city 10
		// corridor 5
		// farm 5
		// hall 5
		// platform 5
		// ruins 5
		// settlement 15
		// shelter 5
		// tunnel 5

		if (newareatype >= 90){
			roomtype = 2;

		}
		else if (newareatype >= 65) {
			roomtype = 3;

		}
		else if (newareatype >= 60) {
			roomtype = 4;

		}
		else if (newareatype >= 50) {
			roomtype = 5;

		}
		else if (newareatype >= 45) {
			roomtype = 6;

		}
		else if (newareatype >= 40) {
			roomtype = 9;

		}
		else if (newareatype >= 35) {
			roomtype = 10;
		}
		else if (newareatype >= 30) {
			roomtype = 12;

		}
		else if (newareatype >= 25) {
			roomtype = 15;

		}
		else if (newareatype >= 10) {
			roomtype = 16;
		}
		else if (newareatype >= 5) {
			roomtype = 17;
		}
		else {
			roomtype = 20;
		}

		break;

	case SECT_CITY:
		// breeding 5
		// bridge 5
		// city 25
		// corridor 5
		// factory 5
		// farm 5
		// hall 5
		// platform 5
		// prison 5
		// ramp 5
		// ruins 5
		// settlement 5
		// shelter 5
		// stairway 5
		// tower 5
		// tunnel 5

		if (newareatype >= 95){
			roomtype = 1;

		}

		else if (newareatype >= 90){
			roomtype = 2;

		}
		else if (newareatype >= 65) {
			roomtype = 5;

		}
		else if (newareatype >= 60) {
			roomtype = 6;

		}
		else if (newareatype >= 55) {
			roomtype = 8;

		}
		else if (newareatype >= 50) {
			roomtype = 9;

		}
		else if (newareatype >= 45) {
			roomtype = 10;
		}
		else if (newareatype >= 40) {
			roomtype = 12;

		}
		else if (newareatype >= 35) {
			roomtype = 13;

		}
		else if (newareatype >= 30) {
			roomtype = 14;

		}
		else if (newareatype >= 25) {
			roomtype = 15;

		}
		else if (newareatype >= 20) {
			roomtype = 16;
		}
		else if (newareatype >= 15) {
			roomtype = 17;
		}
		else if (newareatype >= 10) {
			roomtype = 18;
		}
		else if (newareatype >= 5) {
			roomtype = 19;
		}
		else {
			roomtype = 20;
		}

		break;

	case SECT_CORRIDOR:
		// breeding 5
		// camp 10
		// city 5
		// corridor 15
		// farm 5
		// hall 10
		// platform 10
		// ramp 5
		// ruins 10
		// settlement 5
		// shelter 5
		// stairway 5
		// tower 5
		// tunnel 5

		if (newareatype >= 95){
			roomtype = 1;

		}

		else if (newareatype >= 85){
			roomtype = 3;
		}
		else if (newareatype >= 80) {
			roomtype = 5;
		}
		else if (newareatype >= 65) {
			roomtype = 6;
		}
		else if (newareatype >= 60) {
			roomtype = 9;
		}
		else if (newareatype >= 50) {
			roomtype = 10;
		}
		else if (newareatype >= 40) {
			roomtype = 12;
		}
		else if (newareatype >= 35) {
			roomtype = 14;
		}
		else if (newareatype >= 25) {
			roomtype = 15;
		}
		else if (newareatype >= 20) {
			roomtype = 16;
		}
		else if (newareatype >= 15) {
			roomtype = 17;
		}
		else if (newareatype >= 10) {
			roomtype = 18;
		}
		else if (newareatype >= 5) {
			roomtype = 19;
		}
		else {
			roomtype = 20;
		}

		break;

	case SECT_FACTORY:
		// city 50
		// factory 50

		if (newareatype >= 50) {
			roomtype = 5;
		}
		else {
			roomtype = 8;
		}

		break;

	case SECT_FARM:
		// breeding 5
		// camp 5
		// city 15
		// corridor 15
		// farm 20
		// settlement 10
		// shelter 15
		// tower 10
		// tunnel 5

		if (newareatype >= 95){
			roomtype = 1;

		}

		else if (newareatype >= 90){
			roomtype = 3;
		}
		else if (newareatype >= 75) {
			roomtype = 5;
		}
		else if (newareatype >= 60) {
			roomtype = 6;
		}
		else if (newareatype >= 40) {
			roomtype = 9;
		}
		else if (newareatype >= 30) {
			roomtype = 16;
		}
		else if (newareatype >= 15) {
			roomtype = 17;
		}
		else if (newareatype >= 5) {
			roomtype = 19;
		}
		else {
			roomtype = 20;
		}

		break;

	case SECT_HALL:
		// breeding 5
		// bridge 10
		// camp 10
		// city 10
		// corridor 5
		// hall 10
		// platform 10
		// ruins 10
		// settlement 10
		// shelter 10
		// stairway 10

		if (newareatype >= 95){
			roomtype = 1;

		}
		else if (newareatype >= 85){
			roomtype = 2;
		}
		else if (newareatype >= 75){
			roomtype = 3;
		}
		else if (newareatype >= 65) {
			roomtype = 5;
		}
		else if (newareatype >= 60) {
			roomtype = 6;
		}
		else if (newareatype >= 50) {
			roomtype = 10;
		}
		else if (newareatype >= 40) {
			roomtype = 12;
		}
		else if (newareatype >= 30) {
			roomtype = 15;
		}
		else if (newareatype >= 20) {
			roomtype = 16;
		}
		else if (newareatype >= 10) {
			roomtype = 17;
		}
		else  {
			roomtype = 18;
		}

		break;

	case SECT_PLATFORM:
		// bridge 10
		// camp 5
		// city 5
		// corridor 10
		// hall 10
		// platform 15
		// ramp 5
		// ruins 10
		// settlement 5
		// shelter 10
		// tower 5
		// tunnel 10

		if (newareatype >= 90){
			roomtype = 2;

		}

		else if (newareatype >= 85){
			roomtype = 3;

		}
		else if (newareatype >= 80) {
			roomtype = 5;
		}
		else if (newareatype >= 70) {
			roomtype = 6;
		}
		else if (newareatype >= 60) {
			roomtype = 10;
		}
		else if (newareatype >= 45) {
			roomtype = 12;

		}
		else if (newareatype >= 40) {
			roomtype = 14;

		}
		else if (newareatype >= 30) {
			roomtype = 15;

		}
		else if (newareatype >= 25) {
			roomtype = 16;
		}
		else if (newareatype >= 15) {
			roomtype = 17;
		}
		else if (newareatype >= 10) {
			roomtype = 19;
		}
		else {
			roomtype = 20;
		}


		break;

	case SECT_PRISON:
		// city 20
		// prison 70
		// settlement 10

		if (newareatype >= 80) {
			roomtype = 5;
		}
		else if (newareatype >= 10) {
			roomtype = 13;
		}
		else {
			roomtype = 16;
		}

		break;

	case SECT_RAMP:
		// ramp 30
		// elevator 10
		// city 10
		// corridor 25
		// platform 25

		if (newareatype >= 70){
			roomtype = 14;

		}

		else if (newareatype >= 60){
			roomtype = 7;

		}
		else if (newareatype >= 50) {
			roomtype = 5;

		}
		else if (newareatype >= 25) {
			roomtype = 6;
		}
		else {
			roomtype = 12;

		}

		break;

	case SECT_RUINS:
		// breeding 5
		// camp 5
		// city 5
		// corridor 15
		// hall 15
		// platform 10
		// ruins 15
		// settlement 5
		// shelter 5
		// stairway 5
		// tower 5
		// tunnel 5

		if (newareatype >= 95){
			roomtype = 1;

		}

		else if (newareatype >= 90){
			roomtype = 3;
		}
		else if (newareatype >= 85) {
			roomtype = 5;
		}
		else if (newareatype >= 70) {
			roomtype = 6;
		}
		else if (newareatype >= 55) {
			roomtype = 10;
		}
		else if (newareatype >= 45) {
			roomtype = 12;
		}
		else if (newareatype >= 30) {
			roomtype = 15;
		}
		else if (newareatype >= 25) {
			roomtype = 16;
		}
		else if (newareatype >= 15) {
			roomtype = 17;
		}
		else if (newareatype >= 10) {
			roomtype = 18;
		}
		else if (newareatype >= 5) {
			roomtype = 19;
		}
		else {
			roomtype = 20;
		}


		break;

	case SECT_SETTLEMENT:
		// bridge 5
		// camp 5
		// city 5
		// corridor 5
		// farm 20
		// hall 10
		// platform 10
		// ruins 5
		// settlement 20
		// shelter 10
		// tunnel 5

		if (newareatype >= 95){
			roomtype = 2;

		}

		else if (newareatype >= 90){
			roomtype = 3;

		}
		else if (newareatype >= 85) {
			roomtype = 5;
		}
		else if (newareatype >= 80) {
			roomtype = 6;

		}
		else if (newareatype >= 60) {
			roomtype = 9;

		}
		else if (newareatype >= 50) {
			roomtype = 10;
		}
		else if (newareatype >= 40) {
			roomtype = 12;
		}
		else if (newareatype >= 35) {
			roomtype = 15;
		}
		else if (newareatype >= 15) {
			roomtype = 16;
		}
		else if (newareatype >= 5) {
			roomtype = 17;
		}
		else {
			roomtype = 20;
		}

		break;

	case SECT_SHELTER:
		// camp 10
		// city 10
		// corridor 10
		// farm 20
		// hall 10
		// platform 10
		// ruins 10
		// settlement 10
		// tunnel 10

		if (newareatype >= 90){
			roomtype = 3;

		}
		else if (newareatype >= 80) {
			roomtype = 5;

		}
		else if (newareatype >= 70) {
			roomtype = 6;
		}
		else if (newareatype >= 50) {
			roomtype = 9;

		}
		else if (newareatype >= 40) {
			roomtype = 10;
		}
		else if (newareatype >= 30) {
			roomtype = 12;

		}
		else if (newareatype >= 20) {
			roomtype = 15;

		}
		else if (newareatype >= 10) {
			roomtype = 16;
		}
		else {
			roomtype = 20;
		}

		break;

	case SECT_STAIRWAY:
		// chasm 10
		// city 10
		// corridor 15
		// hall 15
		// ruins 10
		// stairway 20
		// tower 20

		if (newareatype >= 90){
			roomtype = 4;

		}
		else if (newareatype >= 80) {
			roomtype = 5;

		}
		else if (newareatype >= 65) {
			roomtype = 6;

		}
		else if (newareatype >= 50) {
			roomtype = 10;
		}
		else if (newareatype >= 40) {
			roomtype = 15;

		}
		else if (newareatype >= 20) {
			roomtype = 18;
		}
		else {
			roomtype = 19;
		}

		break;

	case SECT_TOWER:
		// bridge 5
		// city 5
		// corridor 5
		// farm 5
		// platform 5
		// ruins 5
		// stairway 5
		// tower 65

		if (newareatype >= 95){
			roomtype = 2;

		}
		else if (newareatype >= 90) {
			roomtype = 5;

		}
		else if (newareatype >= 85) {
			roomtype = 6;
		}
		else if (newareatype >= 80) {
			roomtype = 9;
		}
		else if (newareatype >= 75) {
			roomtype = 12;
		}
		else if (newareatype >= 70) {
			roomtype = 15;
		}
		else if (newareatype >= 65) {
			roomtype = 18;
		}
		else {
			roomtype = 19;
		}

		break;

	case SECT_TUNNEL:
		// breeding 10
		// camp 10
		// city 10
		// corridor 10
		// farm 10
		// platform 10
		// ruins 10
		// settlement 10
		// shelter 10
		// tunnel 10

		if (newareatype >= 90){
			roomtype = 1;

		}

		else if (newareatype >= 80){
			roomtype = 3;

		}
		else if (newareatype >= 70) {
			roomtype = 5;
		}
		else if (newareatype >= 60) {
			roomtype = 6;
		}
		else if (newareatype >= 50) {
			roomtype = 9;
		}
		else if (newareatype >= 40) {
			roomtype = 12;

		}
		else if (newareatype >= 30) {
			roomtype = 15;

		}
		else if (newareatype >= 20) {
			roomtype = 16;
		}
		else if (newareatype >= 10) {
			roomtype = 17;
		}
		else {
			roomtype = 20;
		}

		break;

	}

	// actual rooms now

	roomsize = number_range(1, 5);
	roomwidth = number_range(1, 4);
	roomlength = number_range(1, 3);
	roommaterial = number_range(1, 10);

	// width

	if (roomwidth == 1)
	{
		strcpy( bufrdesc, "a narrow, ");
	}
	else if (roomwidth == 2)
	{
		strcpy( bufrdesc, "a thin, ");
	}
	else if (roomwidth == 3)
	{
		strcpy( bufrdesc, "a ");
	}
	else
	{
		strcpy( bufrdesc, "a wide, ");
	}

	// length

	if (roomlength == 1)
	{
		strcat( bufrdesc, "short ");
		roomdistance = 1;
	}
	else if (roomlength == 2)
	{
		strcat( bufrdesc, "medium-sized ");
		roomdistance = 2;
	}
	else
	{
		strcat( bufrdesc, "long ");
		roomdistance = 4;
	}

	// material

	if (roommaterial == 1)
	{
		strcat( bufrdesc, "area made out of cracked stone.");
	}
	else if (roommaterial == 2)
	{
		strcat( bufrdesc, "area made out of smooth stone.");
	}
	else if (roommaterial == 3)
	{
		strcat( bufrdesc, "area made out of rough stone.");
	}
	else if (roommaterial == 4)
	{
		strcat( bufrdesc, "area made out of padded stone.");
	}
	else if (roommaterial == 5)
	{
		strcat( bufrdesc, "area made out of weathered stone.");
	}
	else if (roommaterial == 6)
	{
		strcat( bufrdesc, "area made out of rusted metal.");
	}
	else if (roommaterial == 7)
	{
		strcat( bufrdesc, "area made out of smooth metal.");
	}
	else if (roommaterial == 8)
	{
		strcat( bufrdesc, "area made out of cracked metal.");
	}
	else if (roommaterial == 9)
	{
		strcat( bufrdesc, "area made out of rough metal.");
	}
	else
	{
		strcat( bufrdesc, "area made out of shiny metal.");
	}

	// rest

	switch (roomtype) {

	default:
		break;

	case 1: // breeding


		roomnoise = number_range(1, 3);

		// height

		roomheight = number_range(1, 2);

		if (roomheight == 1)
			strcat( bufrdesc, " the ceiling is low. ");
		else
			strcat( bufrdesc, " the ceiling is high. ");

		// name

		if ( number_range(1, 2) == 1 ){
			strcpy( bufrname, "hive" );
			strcat( bufrdesc, " lines upon lines of cyborg cloning tanks inhabit this area. ");

			if (roomnoise == 1)
				strcat( bufrdesc, " a blubbering noise is coming from the tanks. ");
			else if (roomnoise == 2)
				strcat( bufrdesc, " you can hear a whistling noise. ");
			else
				strcat( bufrdesc, " you hear the sound of flowing liquids. ");

			strcat( bufrdesc, " the whole area smells very sterile. ");

		}
		else {
			strcpy( bufrname, "hive" );
			strcat( bufrdesc, " a hideous nest of mutated beings is attached to one of the walls. ");

			roomnoise = number_range(1, 3);

			if (roomnoise == 1)
				strcat( bufrdesc, " a blubbering noise is coming from the nest. ");
			else if (roomnoise == 2)
				strcat( bufrdesc, " you can hear a whistling noise. ");
			else
				strcat( bufrdesc, " you hear the sound of flowing liquids. ");

			strcat( bufrdesc, " the whole area smells rotten. ");

		}

		// random detail

		randomdescription = number_range(1, 5);

		if ( randomdescription == 1 ){
			strcat( bufrdesc, "lots of wires wind themselves like snakes around the floor and walls. "); }
		else if ( randomdescription == 2 ){
			strcat( bufrdesc, "small and large pipes run along the walls of the room. "); }
		else if ( randomdescription == 3 ){
			strcat( bufrdesc, "some broken devices are lying on the floor. "); }
		else if ( randomdescription == 4 ){
			strcat( bufrdesc, "several boxes are stacked in one corner of the room. "); }
		else {
			strcat( bufrdesc, "some barrels have been left in this room. "); }

		// walls

		roomwalls = number_range(1, 2);

		if (roomwalls == 1){
			strcat( bufrdesc, "the walls are smooth. ");
		}
		else {
			strcat( bufrdesc, "there is a balcony in one of the walls. ");
		}

		nRoom->sector_type = SECT_BREEDING;

		break;

	case 2: // bridge

		// height

		roomheight = number_range(1, 3);

		if (roomheight == 1)
			strcat( bufrdesc, " you can not see a ceiling above you. ");
		else if (roomheight == 2)
			strcat( bufrdesc, " there is a ceiling just a few levels above this one. ");
		else
			strcat( bufrdesc, " there is a ceiling many levels above this one. ");

		// name

		strcpy( bufrname, "bridge" );

		strcat( bufrdesc, " you are on a bridge. ");

		// noise

		roomnoise = number_range(1, 2);

		if (roomnoise == 1){
			strcat( bufrdesc, "a humming noise fills the air. ");
		}
		else {
			strcat( bufrdesc, "a constant boom attacks your ears. ");
		}

		// random detail

		randomdescription = number_range(1, 7);

		if ( randomdescription == 1 ){
			strcat( bufrdesc, "a small compartment is in one of the walls. "); }
		else if ( randomdescription == 2 ){
			strcat( bufrdesc, "some human remains are here. "); }
		else if ( randomdescription == 3 ){
			strcat( bufrdesc, "you can see some dried blood on the floor. "); }
		else if ( randomdescription == 4 ){
			strcat( bufrdesc, "a cadaver is here. "); }
		else if ( randomdescription == 5 ){
			strcat( bufrdesc, "small and large pipes run across this bridge. "); }
		else if ( randomdescription == 6 ){
			strcat( bufrdesc, "someone has discarded some old machinery here. "); }
		else {
			strcat( bufrdesc, "a large skeleton lies here. "); }

		// walls

		roomwalls = number_range(1, 2);

		if (roomwalls == 1){
			strcat( bufrdesc, "the bridge traverses a dark and seemingly endless chasm. ");
		}
		else {
			strcat( bufrdesc, "there is a huge billboard with strange markings on one of the walls. ");
		}

		nRoom->sector_type = SECT_BRIDGE;
		break;

	case 3: // camp

		// height

		roomheight = number_range(1, 3);

		if (roomheight == 1)
			strcat( bufrdesc, " the ceiling is low. ");
		else
			strcat( bufrdesc, " the ceiling is high. ");

		strcpy( bufrname, "campsite" );

		strcat( bufrdesc, " someone have created a camp site here. ");

		// noise

		roomnoise = number_range(1, 2);

		if (roomnoise == 1){
			strcat( bufrdesc, "you can hear drops splashing on the floor from above. ");
		}
		else {
			strcat( bufrdesc, "it is very quiet. ");
		}

		// random detail

		randomdescription = number_range(1, 7);

		if ( randomdescription == 1 ){
			strcat( bufrdesc, "a ruined bedroll has been discarded here. "); }
		else if ( randomdescription == 2 ){
			strcat( bufrdesc, "remains of a small fire are here. "); }
		else if ( randomdescription == 3 ){
			strcat( bufrdesc, "some mutated animal bones are scattered around here. "); }
		else if ( randomdescription == 4 ){
			strcat( bufrdesc, "bullet shells litter the floor. "); }
		else if ( randomdescription == 5 ){
			strcat( bufrdesc, "empty boxes are scattered around the room. "); }
		else if ( randomdescription == 6 ){
			strcat( bufrdesc, "some barrels have been knocked over. "); }
		else {
			strcat( bufrdesc, "a transporter has crashed here, only its wreck remains. "); }

		// walls

		roomwalls = number_range(1, 2);

		if (roomwalls == 1){
			strcat( bufrdesc, "the walls are smooth and stained. ");
		}
		else {
			strcat( bufrdesc, "there is a large water pipe in one wall. ");
			SET_BIT( nRoom->room_flags2 , ROOM_FOUNTAIN );
		}

		//SET_BIT( nRoom->room_flags , ROOM_NO_MOB );

		nRoom->sector_type = SECT_CAMP;
		break;

	case 4: // chasm

		strcpy( bufrname, "chasm" );

		randomdescription = number_range(1, 3);

		if ( randomdescription == 1 ){
			strcpy( bufrdesc, "a seemingly endless chasm, surrounded by the walls of other areas in the Metropolis. "); }
		else if ( randomdescription == 2 ){
			strcpy( bufrdesc, "a seemingly endless and wide chasm that stretches out into all directions around you."); }
		else if ( randomdescription == 3 ){
			strcpy( bufrdesc, "a seemingly endless chasm that is filled with some kind of dark mist."); }

		SET_BIT( nRoom->room_flags , ROOM_NO_MOB );
		SET_BIT( nRoom->room_flags , ROOM_NOPEDIT );

		nRoom->sector_type = SECT_CHASM;
		break;

	case 5: // city

		// height

		roomheight = number_range(1, 3);

		if (roomheight == 1)
			strcat( bufrdesc, " the open sky is far, far above you. ");
		else if (roomheight == 2)
			strcat( bufrdesc, " there is a ceiling just a few levels above this one. ");
		else
			strcat( bufrdesc, " there is a ceiling many levels above this one. ");

		// name

		strcpy( bufrname, "city" );

		strcat( bufrdesc, " you are in a bustling city in the Metropolis. ");

		// noise

		roomnoise = number_range(1, 4);

		if (roomnoise == 1){
			strcat( bufrdesc, "chatter from all of the people around you fills the street. ");
		}
		else if (roomnoise == 2){
			strcat( bufrdesc, "a dog is barking loudly. ");
		}
		else if (roomnoise == 3){
			strcat( bufrdesc, "you hear the creaking wheels of a large cart. ");
		}
		else {
			strcat( bufrdesc, "a doom-sayer is heralding the coming of a darker age. ");
		}

		// random detail

		randomdescription = number_range(1, 7);

		if ( randomdescription == 1 ){
			strcat( bufrdesc, "a make-shift cart stands here, loaded with empty boxes and barrels. "); }
		else if ( randomdescription == 2 ){
			strcat( bufrdesc, "small, slightly mutated pets roam the streets. "); }
		else if ( randomdescription == 3 ){
			strcat( bufrdesc, "you can see some dried blood on the floor. "); }
		else if ( randomdescription == 4 ){
			strcat( bufrdesc, "a fountain stands in the middle of this area. ");
			SET_BIT( nRoom->room_flags2 , ROOM_FOUNTAIN );
		}
		else if ( randomdescription == 5 ){
			strcat( bufrdesc, "someone is advertising their crafted goods loudly. ");
			SET_BIT( nRoom->room_flags2 , ROOM_SHOPPING ); }
		else if ( randomdescription == 6 ){
			strcat( bufrdesc, "someone has discarded some old machinery here. "); }
		else {
			strcat( bufrdesc, "some criminals have been hanged here. "); }

		// walls

		roomwalls = number_range(1, 4);

		if (roomwalls == 1){
			strcat( bufrdesc, "a balcony has been built into one of the walls. ");
		}
		else if (roomwalls == 2){
			strcat( bufrdesc, "make-shift living compartments have been attached to or built into the walls. ");
		}
		else if (roomwalls == 3){
			strcat( bufrdesc, "metal railings keep citizens from falling down from this platform. ");
		}
		else {
			strcat( bufrdesc, "the walls are smooth. ");
		}

		//SET_BIT( nRoom->room_flags , ROOM_NO_MOB );

		nRoom->sector_type = SECT_CITY;
		break;

	case 6: // corridor

		// height

		roomheight = number_range(1, 3);

		if (roomheight == 1)
			strcat( bufrdesc, " the ceiling is low. ");
		else if (roomheight == 2)
			strcat( bufrdesc, " the ceiling is high. ");
		else
			strcat( bufrdesc, " the ceiling is massively high. ");

		// name

		strcpy( bufrname, "corridor" );

		strcat( bufrdesc, " one of the thousands of corridors that connect areas in the Metropolis. ");

		// noise

		roomnoise = number_range(1, 3);

		if (roomnoise == 1){
			strcat( bufrdesc, "you think you hear some footsteps in the distance. ");
		}
		else if (roomnoise == 2){
			strcat( bufrdesc, "you can hear water drip from the ceiling. ");
			SET_BIT( nRoom->room_flags2 , ROOM_FOUNTAIN );
		}
		else {
			strcat( bufrdesc, "a monotonous hum fills this room. ");
		}

		// random detail

		randomdescription = number_range(1, 10);

		if ( randomdescription == 1 ){
			strcat( bufrdesc, "there is a metal grate in one of the walls. "); }
		else if ( randomdescription == 2 ){
			strcat( bufrdesc, "a large fan rotor has been grafted into one of the walls. "); }
		else if ( randomdescription == 3 ){
			strcat( bufrdesc, "thin and thick trunks of wires run across the walls and floor. "); }
		else if ( randomdescription == 4 ){
			strcat( bufrdesc, "big and small pipes run across the middle of the walls. ");
		}
		else if ( randomdescription == 5 ){
			strcat( bufrdesc, "a rusty, broken ladder leans against one of the walls. ");}
		else if ( randomdescription == 6 ){
			strcat( bufrdesc, "a large crossbeam dominates this room. "); }
		else if ( randomdescription == 7 ){
			strcat( bufrdesc, "the walls are riddled with bullet holes. "); }
		else if ( randomdescription == 8 ){
			strcat( bufrdesc, "a circuit box sits on one of the walls here. "); }
		else if ( randomdescription == 9 ){
			strcat( bufrdesc, "a lot of rubble is strewn across the floor. "); }
		else {
			strcat( bufrdesc, "small, square lights are embedded in the walls of the room. "); }

		nRoom->sector_type = SECT_CORRIDOR;
		break;

	case 7: // elevator

		strcpy( bufrname, "elevator" );

		strcpy( bufrdesc, "a large service elevator that is attached and runs on the rails in the wall behind it. ");

		islevelchange = TRUE;
		SET_BIT( nRoom->room_flags , ROOM_NOPEDIT );

		nRoom->sector_type = SECT_ELEVATOR;

		break;

	case 8: // factory

		// height

		roomheight = number_range(1, 3);

		if (roomheight == 1)
			strcat( bufrdesc, " the ceiling is low. ");
		else if (roomheight == 2)
			strcat( bufrdesc, " the ceiling is high. ");
		else
			strcat( bufrdesc, " the ceiling is massively high. ");

		// name

		strcpy( bufrname, "factory" );

		strcat( bufrdesc, " this is an ancient factory building with lots of useful devices and machinery. ");

		// noise

		roomnoise = number_range(1, 3);

		if (roomnoise == 1){
			strcat( bufrdesc, "you can hear a grinding noise. ");
		}
		else if (roomnoise == 2){
			strcat( bufrdesc, "a constant whirring noise sounds through the room. ");
		}
		else {
			strcat( bufrdesc, "you can hear the sound of metal banging against metal. ");
		}

		// random detail

		randomdescription = number_range(1, 10);

		if ( randomdescription == 1 ){
			strcat( bufrdesc, "a semi-intact workbench is here. "); }
		else if ( randomdescription == 2 ){
			strcat( bufrdesc, "a large fan has been grafted into one of the walls. "); }
		else if ( randomdescription == 3 ){
			strcat( bufrdesc, "long, colourful trunks of wires run along the walls of this rooms. "); }
		else if ( randomdescription == 4 ){
			strcat( bufrdesc, "big and small pipes run across the middle of the walls. ");
		}
		else if ( randomdescription == 5 ){
			strcat( bufrdesc, "a massive metal press is in this room. ");}
		else if ( randomdescription == 6 ){
			strcat( bufrdesc, "sewing and cloth presses occupy most of the space in this room. "); }
		else if ( randomdescription == 7 ){
			strcat( bufrdesc, "scientific devices sit on top large tables. "); }
		else if ( randomdescription == 8 ){
			strcat( bufrdesc, "a furnace is in this room. "); }
		else if ( randomdescription == 9 ){
			strcat( bufrdesc, "lots of boxes are stacked in the corners of this room. "); }
		else {
			strcat( bufrdesc, "large crates are stacked in this storage area of the factory. "); }

		nRoom->sector_type = SECT_FACTORY;

		break;

	case 9: // farm

		roomnoise = number_range(1, 3);

		// height

		roomheight = number_range(1, 2);

		if (roomheight == 1)
			strcat( bufrdesc, " the ceiling is low. ");
		else
			strcat( bufrdesc, " the ceiling is high. ");

		// name

		if ( number_range(1, 2) == 1 ){
			strcpy( bufrname, "storage" );
			strcat( bufrdesc, " this storage area is filled with shelves that hold food containers. ");

			strcat( bufrdesc, " you can smell the dried food in the containers. ");

		}
		else {
			strcpy( bufrname, "spores" );
			strcat( bufrdesc, " someone is cultivating edible spores on one wall of this room. ");

			strcat( bufrdesc, " you can smell the delicious growth on the wall. ");

		}

		// noise

		if (roomnoise == 1)
			strcat( bufrdesc, " you can hear scattering noises coming from the walls. ");
		else if (roomnoise == 2){
			strcat( bufrdesc, " water is dropping from a grate in one of the walls. ");
			SET_BIT( nRoom->room_flags2 , ROOM_FOUNTAIN );
		}
		else
			strcat( bufrdesc, " it is very quiet. ");

		// random detail

		randomdescription = number_range(1, 5);

		if ( randomdescription == 1 ){
			strcat( bufrdesc, "the whole area is well preserved. "); }
		else if ( randomdescription == 2 ){
			strcat( bufrdesc, "small and large pipes run along the walls of the room. "); }
		else if ( randomdescription == 3 ){
			strcat( bufrdesc, "some broken devices are lying on the floor. "); }
		else if ( randomdescription == 4 ){
			strcat( bufrdesc, "several boxes are stacked in one corner of the room. "); }
		else {
			strcat( bufrdesc, "some barrels have been left in this room. "); }

		// walls

		roomwalls = number_range(1, 2);

		if (roomwalls == 1){
			strcat( bufrdesc, "the walls are smooth and dry. ");
		}
		else {
			strcat( bufrdesc, "the walls are rough and wet. ");
		}

		SET_BIT( nRoom->room_flags2 , ROOM_FOOD );
		nRoom->sector_type = SECT_FARM;

		break;

	case 10: // hall

		// height

		roomheight = number_range(1, 3);

		if (roomheight == 1)
			strcat( bufrdesc, " the ceiling is high. ");
		else if (roomheight == 2)
			strcat( bufrdesc, " the ceiling is very high. ");
		else
			strcat( bufrdesc, " the ceiling is massively high. ");

		// name

		strcpy( bufrname, "hallway" );

		strcat( bufrdesc, " this is one of the many magnificent hallways in the Metropolis. ");

		// noise

		strcat( bufrdesc, "all noises echo through the whole area. ");

		// random detail

		// wires, signs, circuit box, debries, rubble, spires, grill, archway, pillar, decorations, bullet holes

		randomdescription = number_range(1, 11);

		if ( randomdescription == 1 ){
			strcat( bufrdesc, "thick and thin wire trunks run across the floor and walls. "); }
		else if ( randomdescription == 2 ){
			strcat( bufrdesc, "a sign with strange writings has been attached to one of the walls. "); }
		else if ( randomdescription == 3 ){
			strcat( bufrdesc, "a circuit box sits in one of the walls. "); }
		else if ( randomdescription == 4 ){
			strcat( bufrdesc, "lots a debry that fell from above has gathered in this room. ");
		}
		else if ( randomdescription == 5 ){
			strcat( bufrdesc, "big heaps of rubble block block some parts of this room. ");}
		else if ( randomdescription == 6 ){
			strcat( bufrdesc, "strange spires grow out of the ground and build macabre archways. "); }
		else if ( randomdescription == 7 ){
			strcat( bufrdesc, "a rusty metal grate has been grafted into one of the walls. "); }
		else if ( randomdescription == 8 ){
			strcat( bufrdesc, "a beautiful archway decorates this hallway. "); }
		else if ( randomdescription == 9 ){
			strcat( bufrdesc, "large, round pillars rise into the sky in this room. "); }
		else if ( randomdescription == 10 ){
			strcat( bufrdesc, "intricate decorations have been added to some of the walls. "); }
		else {
			strcat( bufrdesc, "the walls are full of bullet holes. "); }

		nRoom->sector_type = SECT_HALL;

		break;

	case 12: // platform

		// height

		roomheight = number_range(1, 3);

		if (roomheight == 1)
			strcat( bufrdesc, " the open sky is far, far above you. ");
		else if (roomheight == 2)
			strcat( bufrdesc, " there is a ceiling just a few levels above this one. ");
		else
			strcat( bufrdesc, " there is a ceiling many levels above this one. ");

		// name

		strcpy( bufrname, "platform" );

		strcat( bufrdesc, " this room is a platform, open to some of its sides. ");

		// noise

		// noise

		roomnoise = number_range(1, 3);

		if (roomnoise == 1){
			strcat( bufrdesc, "you hear the rushing of water nearby. ");
			SET_BIT( nRoom->room_flags2 , ROOM_FOUNTAIN );
		}
		else if (roomnoise == 2){
			strcat( bufrdesc, "a pool of slime is blubbering. ");
		}
		else {
			strcat( bufrdesc, "you can hear a weird buzzing noise. ");
		}

		// random detail

		// wires, compartment, metal grate, ventilation pipe, remains

		randomdescription = number_range(1, 5);

		if ( randomdescription == 1 ){
			strcat( bufrdesc, "thick and thin wire trunks run across the floor and walls. "); }
		else if ( randomdescription == 2 ){
			strcat( bufrdesc, "a small compartment has been built into one of the walls. "); }
		else if ( randomdescription == 3 ){
			strcat( bufrdesc, "a metal grate has been grafted into one of the walls. "); }
		else if ( randomdescription == 4 ){
			strcat( bufrdesc, "the scattered remains of creatures litter the floor. ");
		}
		else {
			strcat( bufrdesc, "a large ventilation pipe is in one of the walls. "); }

		nRoom->sector_type = SECT_PLATFORM;

		break;

	case 13: // prison

		strcpy( bufrname, "Prison" );
		strcat( bufrdesc, "a holding area. It is barren except for some old, rusty metal instruments that might have been used for torture.");
		nRoom->sector_type = SECT_PRISON;
		SET_BIT( nRoom->room_flags , ROOM_NOPEDIT );

		break;

	case 14: // ramp

		strcpy( bufrname, "ramp" );

		randomdescription = number_range(1, 3);

		if ( randomdescription == 1 ){
			strcat( bufrdesc, "lots of wires and tubes run along the walls of this ramp. "); }
		else if ( randomdescription == 2 ){
			strcat( bufrdesc, "there are circuit boxes mounted on the walls of this ramp. "); }
		else if ( randomdescription == 3 ){
			strcat( bufrdesc, "the rails in the walls of this ramp could mean that an elevator is close. "); }

		nRoom->sector_type = SECT_RAMP;
		SET_BIT( nRoom->room_flags , ROOM_NOPEDIT );

		break;

	case 15: // ruins

		// height

		roomheight = number_range(1, 3);

		if (roomheight == 1)
			strcat( bufrdesc, " the open sky is far, far above you. ");
		else if (roomheight == 2)
			strcat( bufrdesc, " there is a ceiling just a few levels above this one. ");
		else
			strcat( bufrdesc, " there is a ceiling many levels above this one. ");

		// name

		strcpy( bufrname, "ruins" );

		strcat( bufrdesc, " this area either has not withstood the trials of time or some catastrophe has happened here. Whichever it is, the builder seem to have abandoned this section of the Metropolis. ");

		// noise

		strcat( bufrdesc, "the wind blows noisily through here. ");

		// random detail

		// wires, signs, circuit box, debries, rubble, spires, grill, archway, pillar, decorations, bullet holes

		randomdescription = number_range(1, 11);

		if ( randomdescription == 1 ){
			strcat( bufrdesc, "rusted and cut wire trunks run across the floor and walls. "); }
		else if ( randomdescription == 2 ){
			strcat( bufrdesc, "a battered sign with faded symbols dangles from the wall. "); }
		else if ( randomdescription == 3 ){
			strcat( bufrdesc, "a destroyed circuit box sits in one of the walls. "); }
		else if ( randomdescription == 4 ){
			strcat( bufrdesc, "lots a debrie that fell from above has gathered in this room. ");
		}
		else if ( randomdescription == 5 ){
			strcat( bufrdesc, "big heaps of rubble block block some parts of this room. ");}
		else if ( randomdescription == 6 ){
			strcat( bufrdesc, "wire frames pop out of the ruins of the walls. "); }
		else if ( randomdescription == 7 ){
			strcat( bufrdesc, "a rusty metal grate hangs from one of the walls in this room. "); }
		else if ( randomdescription == 8 ){
			strcat( bufrdesc, "a big rock has caused one of the walls to collapse. "); }
		else if ( randomdescription == 9 ){
			strcat( bufrdesc, "broken pillars dot the room. "); }
		else if ( randomdescription == 10 ){
			strcat( bufrdesc, "scraped and broken decorations remain on some of the cracked walls. "); }
		else {
			strcat( bufrdesc, "the ancient remains of some creatures litter the floor. "); }

		nRoom->sector_type = SECT_RUINS;

		break;

	case 16: // settlement

		// height

		roomheight = number_range(1, 3);

		if (roomheight == 1)
			strcat( bufrdesc, " the open sky is far, far above you. ");
		else if (roomheight == 2)
			strcat( bufrdesc, " there is a ceiling just a few levels above this one. ");
		else
			strcat( bufrdesc, " there is a ceiling many levels above this one. ");

		// name

		strcpy( bufrname, "settlement" );

		strcat( bufrdesc, " humans have started to convert this open area into a small settlement. ");

		// noise

		roomnoise = number_range(1, 3);

		if (roomnoise == 1){
			strcat( bufrdesc, "chatter from a group of colonists can be heard in this room. ");
		}
		else if (roomnoise == 2){
			strcat( bufrdesc, "a dog is barking loudly. ");
		}
		else {
			strcat( bufrdesc, "an injured colonist is moaning in one corner of the room. ");
		}

		// random detail

		randomdescription = number_range(1, 5);

		if ( randomdescription == 1 ){
			strcat( bufrdesc, "you can see some dried blood on the floor. "); }
		else if ( randomdescription == 2 ){
			strcat( bufrdesc, "a small water hole has been dug in the middle of the room. ");
			SET_BIT( nRoom->room_flags2 , ROOM_FOUNTAIN );
		}
		else if ( randomdescription == 3 ){
			strcat( bufrdesc, "someone is advertising their crafted goods loudly. ");
			SET_BIT( nRoom->room_flags2 , ROOM_SHOPPING ); }
		else if ( randomdescription == 4 ){
			strcat( bufrdesc, "someone has discarded some broken equipment here. "); }
		else {
			strcat( bufrdesc, "boxes and crates are piled up in one corner of this room. "); }

		// walls

		roomwalls = number_range(1, 4);

		if (roomwalls == 1){
			strcat( bufrdesc, "a balcony has been built into one of the walls. ");
		}
		else if (roomwalls == 2){
			strcat( bufrdesc, "make-shift living compartments have been attached to or built into the walls. ");
		}
		else if (roomwalls == 3){
			strcat( bufrdesc, "metal railings keep citizens from falling down from this platform. ");
		}
		else {
			strcat( bufrdesc, "the walls are smooth. ");
		}

		//SET_BIT( nRoom->room_flags , ROOM_NO_MOB );

		nRoom->sector_type = SECT_SETTLEMENT;
		break;

	case 17: // shelter

		strcpy( bufrname, "shelter" );

		randomdescription = number_range(1, 3);

		if ( randomdescription == 1 ){
			strcat( bufrdesc, "this area seems to be safe to rest in. A bench leans against one of the walls. "); }
		else if ( randomdescription == 2 ){
			strcat( bufrdesc, "colonists have left old rags here that one can rest on. "); }
		else if ( randomdescription == 3 ){
			strcat( bufrdesc, "this area must have once been a park. The plots are empty and the earth is like stone. Some benches line the walls. "); }

		SET_BIT( nRoom->room_flags , ROOM_NO_MOB );
		SET_BIT( nRoom->room_flags , ROOM_SAFE );

		nRoom->sector_type = SECT_SHELTER;
		break;

	case 18: // stairway

		strcpy( bufrname, "stairway" );

		randomdescription = number_range(1, 3);

		if ( randomdescription == 1 ){
			strcat( bufrdesc, "this stairway is cracked and weathered. "); }
		else if ( randomdescription == 2 ){
			strcat( bufrdesc, "exquisite craftmanship made sure that this stairway stands tall when faced with time. "); }
		else if ( randomdescription == 3 ){
			strcat( bufrdesc, "the Metropolis stretches around you as far as you can see from this stairway. "); }

		SET_BIT( nRoom->room_flags , ROOM_NOPEDIT );

		islevelchange = TRUE;

		nRoom->sector_type = SECT_STAIRWAY;

		break;

	case 19: // tower

		strcpy( bufrname, "tower" );

		randomdescription = number_range(1, 3);

		if ( randomdescription == 1 ){
			strcat( bufrdesc, "sark and brooding, this tower leads to a different level of the Metropolis. "); }
		else if ( randomdescription == 2 ){
			strcat( bufrdesc, "the lightly coloured material that this tower was made from distracts from all of the dried blood that decorates the walls. "); }
		else if ( randomdescription == 3 ){
			strcat( bufrdesc, "winding its way through the megastructure, this tower heralds the change to a new level of the Metropolis. "); }

		SET_BIT( nRoom->room_flags , ROOM_NOPEDIT );

		islevelchange = TRUE;

		nRoom->sector_type = SECT_TOWER;
		break;

	case 20: // tunnel

		strcpy( bufrname, "tunnel" );

		randomdescription = number_range(1, 3);

		if ( randomdescription == 1 ){
			strcat( bufrdesc, "this tunnel was carved into the walls of the megastructure. "); }
		else if ( randomdescription == 2 ){
			strcat( bufrdesc, "a natural tunnel was burried here. "); }
		else if ( randomdescription == 3 ){
			strcat( bufrdesc, "this section of tunnel is full of wires and rubble. "); }

		nRoom->sector_type = SECT_TUNNEL;
		break;

	}

	// 1 out of 3 rooms will have another room attached to them

	if (number_range(1, 3) < 3 && !islevelchange)
	{
		hasexit = TRUE;
	}

	// 1 out of 10 rooms are dead-ends

	if (!islevelchange)
	{
		if (!hasexit){
			if (number_range(1, 10)  == 1 && nRoom->sector_type != SECT_CITY)
			{
				SET_BIT( nRoom->room_flags2 , ROOM_DEADEND );
				strcat( bufrdesc, "it seems that this area is a dead-end.");
			}
		}
	}

	// assign name and description

	strcat( bufrdesc, "\n\r");

	nRoom->name = STRALLOC( bufrname );
	nRoom->description = STRALLOC ( bufrdesc );

	// make room

	nRoom->owner = STRALLOC( "unknown" );

	xit = make_exit( ch->in_room, nRoom, edir );
	xit->keyword		= STRALLOC( "" );
	xit->description	= STRALLOC( "" );
	xit->key		= -1;
	xit->exit_info	= 0;
	xit->distance	= roomdistance;

	xit2 = make_exit( nRoom , ch->in_room  , rev_dir[edir] );
	xit2->keyword		= STRALLOC( "" );
	xit2->description	= STRALLOC( "" );
	xit2->key		= -1;
	xit2->exit_info	= 0;
	xit2->distance	= roomdistance;

	ch->in_room->area->planet->size++;

	learn_from_success( ch , gsn_spacecraft );

	if ( number_percent() == 23 )
	{
		send_to_char( "> you feel more intelligent than before.\n\r", ch );
		ch->perm_int++;
		ch->perm_int = UMIN( ch->perm_int , 25 );
	}

	// stairway, elevator or tower

	if (islevelchange){

		if ( roomlevel == 0 )
		{
			roomlevel = roomlevel + 1;
			edir = 4;
		}
		else if ( roomlevel == 5)
		{
			roomlevel = roomlevel - 1;
			edir = 5;
		}
		else {
			if (number_range(1, 2) == 1){
				roomlevel = roomlevel + 1;
				edir = 4; }
			else
			{
				roomlevel = roomlevel - 1;
				edir = 5;
			}

		}

		lRoom = make_room( ++top_r_vnum );
		lRoom->area = ch->in_room->area;
		LINK( lRoom , ch->in_room->area->first_room , ch->in_room->area->last_room , next_in_area , prev_in_area );
		STRFREE( lRoom->name );
		STRFREE( lRoom->description );
		lRoom->level = roomlevel;

		strcpy( bufrname, "shelter" );

		randomdescription = number_range(1, 3);

		if ( randomdescription == 1 ){
			strcpy( bufrdesc, "this area seems to be safe to rest in. A bench leans against one of the walls. "); }
		else if ( randomdescription == 2 ){
			strcpy( bufrdesc, "colonists have left old rags here that one can rest on. "); }
		else if ( randomdescription == 3 ){
			strcpy( bufrdesc, "this area must have once been a park. The plots are empty and the earth is like stone. Some benches line the walls. "); }

		SET_BIT( lRoom->room_flags , ROOM_NO_MOB );
		SET_BIT( lRoom->room_flags , ROOM_SAFE );

		lRoom->sector_type = SECT_SHELTER;

		strcat( bufrdesc, "\n\r");

		lRoom->name = STRALLOC( bufrname );
		lRoom->description = STRALLOC ( bufrdesc );

		// make room

		lRoom->owner = STRALLOC( "unknown" );

		xit = make_exit( nRoom, lRoom, edir );
		xit->keyword		= STRALLOC( "" );
		xit->description	= STRALLOC( "" );
		xit->key		= -1;
		xit->exit_info	= 0;
		xit->distance	= roomdistance;

		xit2 = make_exit( lRoom , nRoom  , rev_dir[edir] );
		xit2->keyword		= STRALLOC( "" );
		xit2->description	= STRALLOC( "" );
		xit2->key		= -1;
		xit2->exit_info	= 0;
		xit2->distance	= roomdistance;

		ch->in_room->area->planet->size++;

	}

	// has another exit

	SET_BIT( ch->in_room->area->flags , AFLAG_MODIFIED );

	sprintf( buf , "> a new area is uncovered in this direction: %s" , dir_name[edir] );
	echo_to_room( AT_WHITE, ch->in_room, buf );

	send_to_char( "> you move to the new area.\n\r", ch );
	act(AT_GREEN, "> $n moves to a new area.", ch, NULL, NULL, TO_ROOM );

	if (number_range(1, 10) == 10)
	reset_all();

	char_from_room( ch );
	char_to_room( ch, nRoom );

	if (hasexit){

		for (exitposs = 1; exitposs <= 4; exitposs++) {
			edir = number_range(0, 3);
			xit = get_exit(nRoom, edir);
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

					exitposs = 5;
				}

			} else {

				exitposs = 5;
			}
		}

		if (edir == 0)
			followconst(ch, "north");
		else if (edir == 1)
			followconst(ch, "east");
		else if (edir == 2)
			followconst(ch, "south");
		else if (edir == 3)
			followconst(ch, "west");



	}

	ch->pcdata->qexplored += (cost / 10);
	do_look( ch, "auto" );

	return;

}

void followconst ( CHAR_DATA *ch , char *argument )
{
	CLAN_DATA * clan;
	int chance;
	EXIT_DATA * xit;
	EXIT_DATA * xit2;
	sh_int roomdistance;
	int edir, roomtype, newareatype, roomsize, roomlevel;
	int roommaterial, randomdescription, roomwidth, roomlength, roomheight, roomnoise, roomwalls;
	ROOM_INDEX_DATA *nRoom;
	ROOM_INDEX_DATA *lRoom;
	char bufrname[MAX_STRING_LENGTH];
	char bufrdesc[MAX_STRING_LENGTH];
	//PLANET_DATA * dPlanet = NULL;
	PLANET_DATA *planet;
	bool islevelchange = FALSE;
	bool hasexit = FALSE;

	if ( !IS_NPC(ch) && !ch->pcdata->learned[gsn_spacecraft] )
	{
		return;
	}

	if ( IS_NPC(ch) || !ch->pcdata || !ch->in_room )
		return;

	clan = ch->pcdata->clan;
	planet = ch->in_room->area->planet;

	if ( str_cmp( argument, "north" )
			&& str_cmp( argument, "south" )
			&& str_cmp( argument, "west" )
			&& str_cmp( argument, "east" )
			&& str_cmp( argument, "n")
			&& str_cmp( argument, "e")
			&& str_cmp( argument, "s")
			&& str_cmp( argument, "w"))
	{
		send_to_char( "&RYou cannot explore in that direction, try:\n\r&w", ch);
		send_to_char( "north, east, south, west.\n\r", ch);
		return;
	}

	if( !IS_IMMORTAL(ch) )
	{
		if ( IS_SET( ch->in_room->room_flags , ROOM_NOPEDIT ) )
		{
			send_to_char( "&RYou may not explore from this area!&w\n\r", ch );
			return;
		}

		if ( IS_SET( ch->in_room->room_flags2 , ROOM_DEADEND ) )
		{
			send_to_char( "&RThis area is a dead-end!&w\n\r", ch );
			return;
		}

	}

	if ( argument[0] == '\0' )
	{
		send_to_char( "&YExplore into which direction?&w\n\r", ch );
		return;
	}

	edir = get_dir(argument);
	xit = get_exit(ch->in_room, edir);
	if ( xit )
	{
		send_to_char( "&RThere is already an area in that direction!&w\n\r", ch );
		return;
	}

	chance = (int) (ch->pcdata->learned[gsn_spacecraft]);
	if ( number_percent( ) > chance )
	{
		send_to_char( "&RYou fail to explore - try again!&w\n\r", ch );
		return;
	}

	roomlevel = ch->in_room->level;

	// determine room type

	nRoom = make_room( ++top_r_vnum );
	nRoom->area = ch->in_room->area;
	LINK( nRoom , ch->in_room->area->first_room , ch->in_room->area->last_room , next_in_area , prev_in_area );
	STRFREE( nRoom->name );
	STRFREE( nRoom->description );
	nRoom->level = roomlevel;

	newareatype = number_range(1, 100);

	switch (ch->in_room->sector_type) {

	default:

		break;

	case SECT_BREEDING:

		// breeding 20
		// city 35
		// corridor 5
		// factory 5
		// farm 5
		// hall 10
		// ruins 10
		// tunnel 10

		if (newareatype >= 80){
			roomtype = 1;
		}
		else if (newareatype >= 45) {
			roomtype = 5;
		}
		else if (newareatype >= 40) {
			roomtype = 6;
		}
		else if (newareatype >= 35) {
			roomtype = 8;
		}
		else if (newareatype >= 30) {
			roomtype = 9;
		}
		else if (newareatype >= 20) {
			roomtype = 10;
		}
		else if (newareatype >= 10) {
			roomtype = 15;
		}
		else {
			roomtype = 20;
		}

		break;

	case SECT_BRIDGE:

		// bridge 25
		// camp 15
		// chasm 10
		// city 5
		// hall 20
		// platform 15
		// settlement 5
		// tower 5

		if (newareatype >= 75){
			roomtype = 2;

		}
		else if (newareatype >= 60) {
			roomtype = 3;

		}
		else if (newareatype >= 50) {
			roomtype = 4;

		}
		else if (newareatype >= 45) {
			roomtype = 5;

		}
		else if (newareatype >= 25) {
			roomtype = 10;
		}
		else if (newareatype >= 10) {
			roomtype = 12;

		}
		else if (newareatype >= 5) {
			roomtype = 16;
		}
		else {
			roomtype = 19;
		}

		break;

	case SECT_CAMP:

		// bridge 10
		// camp 25
		// chasm 5
		// city 10
		// corridor 5
		// farm 5
		// hall 5
		// platform 5
		// ruins 5
		// settlement 15
		// shelter 5
		// tunnel 5

		if (newareatype >= 90){
			roomtype = 2;

		}
		else if (newareatype >= 65) {
			roomtype = 3;

		}
		else if (newareatype >= 60) {
			roomtype = 4;

		}
		else if (newareatype >= 50) {
			roomtype = 5;

		}
		else if (newareatype >= 45) {
			roomtype = 6;

		}
		else if (newareatype >= 40) {
			roomtype = 9;

		}
		else if (newareatype >= 35) {
			roomtype = 10;
		}
		else if (newareatype >= 30) {
			roomtype = 12;

		}
		else if (newareatype >= 25) {
			roomtype = 15;

		}
		else if (newareatype >= 10) {
			roomtype = 16;
		}
		else if (newareatype >= 5) {
			roomtype = 17;
		}
		else {
			roomtype = 20;
		}

		break;

	case SECT_CITY:
		// breeding 5
		// bridge 5
		// city 25
		// corridor 5
		// factory 5
		// farm 5
		// hall 5
		// platform 5
		// prison 5
		// ramp 5
		// ruins 5
		// settlement 5
		// shelter 5
		// stairway 5
		// tower 5
		// tunnel 5

		if (newareatype >= 95){
			roomtype = 1;

		}

		else if (newareatype >= 90){
			roomtype = 2;

		}
		else if (newareatype >= 65) {
			roomtype = 5;

		}
		else if (newareatype >= 60) {
			roomtype = 6;

		}
		else if (newareatype >= 55) {
			roomtype = 8;

		}
		else if (newareatype >= 50) {
			roomtype = 9;

		}
		else if (newareatype >= 45) {
			roomtype = 10;
		}
		else if (newareatype >= 40) {
			roomtype = 12;

		}
		else if (newareatype >= 35) {
			roomtype = 13;

		}
		else if (newareatype >= 30) {
			roomtype = 14;

		}
		else if (newareatype >= 25) {
			roomtype = 15;

		}
		else if (newareatype >= 20) {
			roomtype = 16;
		}
		else if (newareatype >= 15) {
			roomtype = 17;
		}
		else if (newareatype >= 10) {
			roomtype = 18;
		}
		else if (newareatype >= 5) {
			roomtype = 19;
		}
		else {
			roomtype = 20;
		}

		break;

	case SECT_CORRIDOR:
		// breeding 5
		// camp 10
		// city 5
		// corridor 15
		// farm 5
		// hall 10
		// platform 10
		// ramp 5
		// ruins 10
		// settlement 5
		// shelter 5
		// stairway 5
		// tower 5
		// tunnel 5

		if (newareatype >= 95){
			roomtype = 1;

		}

		else if (newareatype >= 85){
			roomtype = 3;
		}
		else if (newareatype >= 80) {
			roomtype = 5;
		}
		else if (newareatype >= 65) {
			roomtype = 6;
		}
		else if (newareatype >= 60) {
			roomtype = 9;
		}
		else if (newareatype >= 50) {
			roomtype = 10;
		}
		else if (newareatype >= 40) {
			roomtype = 12;
		}
		else if (newareatype >= 35) {
			roomtype = 14;
		}
		else if (newareatype >= 25) {
			roomtype = 15;
		}
		else if (newareatype >= 20) {
			roomtype = 16;
		}
		else if (newareatype >= 15) {
			roomtype = 17;
		}
		else if (newareatype >= 10) {
			roomtype = 18;
		}
		else if (newareatype >= 5) {
			roomtype = 19;
		}
		else {
			roomtype = 20;
		}

		break;

	case SECT_FACTORY:
		// city 50
		// factory 50

		if (newareatype >= 50) {
			roomtype = 5;
		}
		else {
			roomtype = 8;
		}

		break;

	case SECT_FARM:
		// breeding 5
		// camp 5
		// city 15
		// corridor 15
		// farm 20
		// settlement 10
		// shelter 15
		// tower 10
		// tunnel 5

		if (newareatype >= 95){
			roomtype = 1;

		}

		else if (newareatype >= 90){
			roomtype = 3;
		}
		else if (newareatype >= 75) {
			roomtype = 5;
		}
		else if (newareatype >= 60) {
			roomtype = 6;
		}
		else if (newareatype >= 40) {
			roomtype = 9;
		}
		else if (newareatype >= 30) {
			roomtype = 16;
		}
		else if (newareatype >= 15) {
			roomtype = 17;
		}
		else if (newareatype >= 5) {
			roomtype = 19;
		}
		else {
			roomtype = 20;
		}

		break;

	case SECT_HALL:
		// breeding 5
		// bridge 10
		// camp 10
		// city 10
		// corridor 5
		// hall 10
		// platform 10
		// ruins 10
		// settlement 10
		// shelter 10
		// stairway 10

		if (newareatype >= 95){
			roomtype = 1;

		}
		else if (newareatype >= 85){
			roomtype = 2;
		}
		else if (newareatype >= 75){
			roomtype = 3;
		}
		else if (newareatype >= 65) {
			roomtype = 5;
		}
		else if (newareatype >= 60) {
			roomtype = 6;
		}
		else if (newareatype >= 50) {
			roomtype = 10;
		}
		else if (newareatype >= 40) {
			roomtype = 12;
		}
		else if (newareatype >= 30) {
			roomtype = 15;
		}
		else if (newareatype >= 20) {
			roomtype = 16;
		}
		else if (newareatype >= 10) {
			roomtype = 17;
		}
		else  {
			roomtype = 18;
		}

		break;

	case SECT_PLATFORM:
		// bridge 10
		// camp 5
		// city 5
		// corridor 10
		// hall 10
		// platform 15
		// ramp 5
		// ruins 10
		// settlement 5
		// shelter 10
		// tower 5
		// tunnel 10

		if (newareatype >= 90){
			roomtype = 2;

		}

		else if (newareatype >= 85){
			roomtype = 3;

		}
		else if (newareatype >= 80) {
			roomtype = 5;
		}
		else if (newareatype >= 70) {
			roomtype = 6;
		}
		else if (newareatype >= 60) {
			roomtype = 10;
		}
		else if (newareatype >= 45) {
			roomtype = 12;

		}
		else if (newareatype >= 40) {
			roomtype = 14;

		}
		else if (newareatype >= 30) {
			roomtype = 15;

		}
		else if (newareatype >= 25) {
			roomtype = 16;
		}
		else if (newareatype >= 15) {
			roomtype = 17;
		}
		else if (newareatype >= 10) {
			roomtype = 19;
		}
		else {
			roomtype = 20;
		}


		break;

	case SECT_PRISON:
		// city 20
		// prison 70
		// settlement 10

		if (newareatype >= 80) {
			roomtype = 5;
		}
		else if (newareatype >= 10) {
			roomtype = 13;
		}
		else {
			roomtype = 16;
		}

		break;

	case SECT_RAMP:
		// ramp 30
		// elevator 10
		// city 10
		// corridor 25
		// platform 25

		if (newareatype >= 70){
			roomtype = 14;

		}

		else if (newareatype >= 60){
			roomtype = 7;

		}
		else if (newareatype >= 50) {
			roomtype = 5;

		}
		else if (newareatype >= 25) {
			roomtype = 6;
		}
		else {
			roomtype = 12;

		}

		break;

	case SECT_RUINS:
		// breeding 5
		// camp 5
		// city 5
		// corridor 15
		// hall 15
		// platform 10
		// ruins 15
		// settlement 5
		// shelter 5
		// stairway 5
		// tower 5
		// tunnel 5

		if (newareatype >= 95){
			roomtype = 1;

		}

		else if (newareatype >= 90){
			roomtype = 3;
		}
		else if (newareatype >= 85) {
			roomtype = 5;
		}
		else if (newareatype >= 70) {
			roomtype = 6;
		}
		else if (newareatype >= 55) {
			roomtype = 10;
		}
		else if (newareatype >= 45) {
			roomtype = 12;
		}
		else if (newareatype >= 30) {
			roomtype = 15;
		}
		else if (newareatype >= 25) {
			roomtype = 16;
		}
		else if (newareatype >= 15) {
			roomtype = 17;
		}
		else if (newareatype >= 10) {
			roomtype = 18;
		}
		else if (newareatype >= 5) {
			roomtype = 19;
		}
		else {
			roomtype = 20;
		}


		break;

	case SECT_SETTLEMENT:
		// bridge 5
		// camp 5
		// city 5
		// corridor 5
		// farm 20
		// hall 10
		// platform 10
		// ruins 5
		// settlement 20
		// shelter 10
		// tunnel 5

		if (newareatype >= 95){
			roomtype = 2;

		}

		else if (newareatype >= 90){
			roomtype = 3;

		}
		else if (newareatype >= 85) {
			roomtype = 5;
		}
		else if (newareatype >= 80) {
			roomtype = 6;

		}
		else if (newareatype >= 60) {
			roomtype = 9;

		}
		else if (newareatype >= 50) {
			roomtype = 10;
		}
		else if (newareatype >= 40) {
			roomtype = 12;
		}
		else if (newareatype >= 35) {
			roomtype = 15;
		}
		else if (newareatype >= 15) {
			roomtype = 16;
		}
		else if (newareatype >= 5) {
			roomtype = 17;
		}
		else {
			roomtype = 20;
		}

		break;

	case SECT_SHELTER:
		// camp 10
		// city 10
		// corridor 10
		// farm 20
		// hall 10
		// platform 10
		// ruins 10
		// settlement 10
		// tunnel 10

		if (newareatype >= 90){
			roomtype = 3;

		}
		else if (newareatype >= 80) {
			roomtype = 5;

		}
		else if (newareatype >= 70) {
			roomtype = 6;
		}
		else if (newareatype >= 50) {
			roomtype = 9;

		}
		else if (newareatype >= 40) {
			roomtype = 10;
		}
		else if (newareatype >= 30) {
			roomtype = 12;

		}
		else if (newareatype >= 20) {
			roomtype = 15;

		}
		else if (newareatype >= 10) {
			roomtype = 16;
		}
		else {
			roomtype = 20;
		}

		break;

	case SECT_STAIRWAY:
		// chasm 10
		// city 10
		// corridor 15
		// hall 15
		// ruins 10
		// stairway 20
		// tower 20

		if (newareatype >= 90){
			roomtype = 4;

		}
		else if (newareatype >= 80) {
			roomtype = 5;

		}
		else if (newareatype >= 65) {
			roomtype = 6;

		}
		else if (newareatype >= 50) {
			roomtype = 10;
		}
		else if (newareatype >= 40) {
			roomtype = 15;

		}
		else if (newareatype >= 20) {
			roomtype = 18;
		}
		else {
			roomtype = 19;
		}

		break;

	case SECT_TOWER:
		// bridge 5
		// city 5
		// corridor 5
		// farm 5
		// platform 5
		// ruins 5
		// stairway 5
		// tower 65

		if (newareatype >= 95){
			roomtype = 2;

		}
		else if (newareatype >= 90) {
			roomtype = 5;

		}
		else if (newareatype >= 85) {
			roomtype = 6;
		}
		else if (newareatype >= 80) {
			roomtype = 9;
		}
		else if (newareatype >= 75) {
			roomtype = 12;
		}
		else if (newareatype >= 70) {
			roomtype = 15;
		}
		else if (newareatype >= 65) {
			roomtype = 18;
		}
		else {
			roomtype = 19;
		}

		break;

	case SECT_TUNNEL:
		// breeding 10
		// camp 10
		// city 10
		// corridor 10
		// farm 10
		// platform 10
		// ruins 10
		// settlement 10
		// shelter 10
		// tunnel 10

		if (newareatype >= 90){
			roomtype = 1;

		}

		else if (newareatype >= 80){
			roomtype = 3;

		}
		else if (newareatype >= 70) {
			roomtype = 5;
		}
		else if (newareatype >= 60) {
			roomtype = 6;
		}
		else if (newareatype >= 50) {
			roomtype = 9;
		}
		else if (newareatype >= 40) {
			roomtype = 12;

		}
		else if (newareatype >= 30) {
			roomtype = 15;

		}
		else if (newareatype >= 20) {
			roomtype = 16;
		}
		else if (newareatype >= 10) {
			roomtype = 17;
		}
		else {
			roomtype = 20;
		}

		break;

	}

	// actual rooms now

	roomsize = number_range(1, 5);
	roomwidth = number_range(1, 4);
	roomlength = number_range(1, 3);
	roommaterial = number_range(1, 10);

	// width

	if (roomwidth == 1)
	{
		strcpy( bufrdesc, "A narrow, ");
	}
	else if (roomwidth == 2)
	{
		strcpy( bufrdesc, "A thin, ");
	}
	else if (roomwidth == 3)
	{
		strcpy( bufrdesc, "A ");
	}
	else
	{
		strcpy( bufrdesc, "A wide, ");
	}

	// length

	if (roomlength == 1)
	{
		strcat( bufrdesc, "short ");
		roomdistance = 1;
	}
	else if (roomlength == 2)
	{
		strcat( bufrdesc, "medium-sized ");
		roomdistance = 2;
	}
	else
	{
		strcat( bufrdesc, "long ");
		roomdistance = 4;
	}

	// material

	if (roommaterial == 1)
	{
		strcat( bufrdesc, "area made out of cracked stone.");
	}
	else if (roommaterial == 2)
	{
		strcat( bufrdesc, "area made out of smooth stone.");
	}
	else if (roommaterial == 3)
	{
		strcat( bufrdesc, "area made out of rough stone.");
	}
	else if (roommaterial == 4)
	{
		strcat( bufrdesc, "area made out of padded stone.");
	}
	else if (roommaterial == 5)
	{
		strcat( bufrdesc, "area made out of weathered stone.");
	}
	else if (roommaterial == 6)
	{
		strcat( bufrdesc, "area made out of rusted metal.");
	}
	else if (roommaterial == 7)
	{
		strcat( bufrdesc, "area made out of smooth metal.");
	}
	else if (roommaterial == 8)
	{
		strcat( bufrdesc, "area made out of cracked metal.");
	}
	else if (roommaterial == 9)
	{
		strcat( bufrdesc, "area made out of rough metal.");
	}
	else
	{
		strcat( bufrdesc, "area made out of shiny metal.");
	}

	// rest

	switch (roomtype) {

	default:
		break;

	case 1: // breeding


		roomnoise = number_range(1, 3);

		// height

		roomheight = number_range(1, 2);

		if (roomheight == 1)
			strcat( bufrdesc, " the ceiling is low. ");
		else
			strcat( bufrdesc, " the ceiling is high. ");

		// name

		if ( number_range(1, 2) == 1 ){
			strcpy( bufrname, "Clone Chamber" );
			strcat( bufrdesc, " Lines upon lines of cyborg cloning tanks inhabit this area. ");

			if (roomnoise == 1)
				strcat( bufrdesc, " A blubbering noise is coming from the tanks. ");
			else if (roomnoise == 2)
				strcat( bufrdesc, " You can hear a whistling noise. ");
			else
				strcat( bufrdesc, " You hear the sound of flowing liquids. ");

			strcat( bufrdesc, " The whole area smells very sterile. ");

		}
		else {
			strcpy( bufrname, "Breeding Chamber" );
			strcat( bufrdesc, " A hideous nest of mutated beings is attached to one of the walls. ");

			roomnoise = number_range(1, 3);

			if (roomnoise == 1)
				strcat( bufrdesc, " A blubbering noise is coming from the nest. ");
			else if (roomnoise == 2)
				strcat( bufrdesc, " You can hear a whistling noise. ");
			else
				strcat( bufrdesc, " You hear the sound of flowing liquids. ");

			strcat( bufrdesc, " The whole area smells rotten. ");

		}

		// random detail

		randomdescription = number_range(1, 5);

		if ( randomdescription == 1 ){
			strcat( bufrdesc, "Lots of wires wind themselves like snakes around the floor and walls. "); }
		else if ( randomdescription == 2 ){
			strcat( bufrdesc, "Small and large pipes run along the walls of the room. "); }
		else if ( randomdescription == 3 ){
			strcat( bufrdesc, "Some broken devices are lying on the floor. "); }
		else if ( randomdescription == 4 ){
			strcat( bufrdesc, "Several boxes are stacked in one corner of the room. "); }
		else {
			strcat( bufrdesc, "Some barrels have been left in this room. "); }

		// walls

		roomwalls = number_range(1, 2);

		if (roomwalls == 1){
			strcat( bufrdesc, "The walls are smooth. ");
		}
		else {
			strcat( bufrdesc, "There is a balcony in one of the walls. ");
		}

		nRoom->sector_type = SECT_BREEDING;

		break;

	case 2: // bridge

		// height

		roomheight = number_range(1, 3);

		if (roomheight == 1)
			strcat( bufrdesc, " the open sky is far, far above you. ");
		else if (roomheight == 2)
			strcat( bufrdesc, " there is a ceiling just a few levels above this one. ");
		else
			strcat( bufrdesc, " there is a ceiling many levels above this one. ");

		// name

		strcpy( bufrname, "Bridge" );

		strcat( bufrdesc, " You are on a bridge. ");

		// noise

		roomnoise = number_range(1, 2);

		if (roomnoise == 1){
			strcat( bufrdesc, "A humming noise fills the air. ");
		}
		else {
			strcat( bufrdesc, "A constant boom attacks your ears. ");
		}

		// random detail

		randomdescription = number_range(1, 7);

		if ( randomdescription == 1 ){
			strcat( bufrdesc, "A small compartment is in one of the walls. "); }
		else if ( randomdescription == 2 ){
			strcat( bufrdesc, "Some human remains are here. "); }
		else if ( randomdescription == 3 ){
			strcat( bufrdesc, "You can see some dried blood on the floor. "); }
		else if ( randomdescription == 4 ){
			strcat( bufrdesc, "A cadaver is here. "); }
		else if ( randomdescription == 5 ){
			strcat( bufrdesc, "Small and large pipes run across this bridge. "); }
		else if ( randomdescription == 6 ){
			strcat( bufrdesc, "Someone has discarded some old machinery here. "); }
		else {
			strcat( bufrdesc, "A large skeleton lies here. "); }

		// walls

		roomwalls = number_range(1, 2);

		if (roomwalls == 1){
			strcat( bufrdesc, "The bridge traverses a dark and seemingly endless chasm. ");
		}
		else {
			strcat( bufrdesc, "There is a huge billboard with strange markings on one of the walls. ");
		}

		nRoom->sector_type = SECT_BRIDGE;
		break;

	case 3: // camp

		// height

		roomheight = number_range(1, 3);

		if (roomheight == 1)
			strcat( bufrdesc, " the ceiling is low. ");
		else
			strcat( bufrdesc, " the ceiling is high. ");

		strcpy( bufrname, "Campsite" );

		strcat( bufrdesc, " Humans have created a camp site here. ");

		// noise

		roomnoise = number_range(1, 2);

		if (roomnoise == 1){
			strcat( bufrdesc, "You can hear drops splashing on the floor from above. ");
		}
		else {
			strcat( bufrdesc, "It is very quiet. ");
		}

		// random detail

		randomdescription = number_range(1, 7);

		if ( randomdescription == 1 ){
			strcat( bufrdesc, "A ruined bedroll has been discarded here. "); }
		else if ( randomdescription == 2 ){
			strcat( bufrdesc, "Remains of a small fire are here. "); }
		else if ( randomdescription == 3 ){
			strcat( bufrdesc, "Some mutated animal bones are scattered around here. "); }
		else if ( randomdescription == 4 ){
			strcat( bufrdesc, "Bullet shells litter the floor. "); }
		else if ( randomdescription == 5 ){
			strcat( bufrdesc, "Empty boxes are scattered around the room. "); }
		else if ( randomdescription == 6 ){
			strcat( bufrdesc, "Some barrels have been knocked over. "); }
		else {
			strcat( bufrdesc, "A transporter has crashed here, only its wreck remains. "); }

		// walls

		roomwalls = number_range(1, 2);

		if (roomwalls == 1){
			strcat( bufrdesc, "The walls are smooth and stained. ");
		}
		else {
			strcat( bufrdesc, "There is a large water pipe in one wall. ");
			SET_BIT( nRoom->room_flags2 , ROOM_FOUNTAIN );
		}

		//SET_BIT( nRoom->room_flags , ROOM_NO_MOB );

		nRoom->sector_type = SECT_CAMP;
		break;

	case 4: // chasm

		strcpy( bufrname, "Chasm" );

		randomdescription = number_range(1, 3);

		if ( randomdescription == 1 ){
			strcpy( bufrdesc, "A seemingly endless chasm, surrounded by the walls of other areas in the Metropolis. "); }
		else if ( randomdescription == 2 ){
			strcpy( bufrdesc, "A seemingly endless and wide chasm that stretches out into all directions around you."); }
		else if ( randomdescription == 3 ){
			strcpy( bufrdesc, "A seemingly endless chasm that is filled with some kind of dark mist."); }

		SET_BIT( nRoom->room_flags , ROOM_NO_MOB );
		SET_BIT( nRoom->room_flags , ROOM_NOPEDIT );

		nRoom->sector_type = SECT_CHASM;
		break;

	case 5: // city

		// height

		roomheight = number_range(1, 3);

		if (roomheight == 1)
			strcat( bufrdesc, " the open sky is far, far above you. ");
		else if (roomheight == 2)
			strcat( bufrdesc, " there is a ceiling just a few levels above this one. ");
		else
			strcat( bufrdesc, " there is a ceiling many levels above this one. ");

		// name

		strcpy( bufrname, "City Area" );

		strcat( bufrdesc, " You are in a bustling city in the Metropolis. ");

		// noise

		roomnoise = number_range(1, 4);

		if (roomnoise == 1){
			strcat( bufrdesc, "Chatter from all of the people around you fills the street. ");
		}
		else if (roomnoise == 2){
			strcat( bufrdesc, "A dog is barking loudly. ");
		}
		else if (roomnoise == 3){
			strcat( bufrdesc, "You hear the creaking wheels of a large cart. ");
		}
		else {
			strcat( bufrdesc, "A doom-sayer is heralding the coming of a darker age. ");
		}

		// random detail

		randomdescription = number_range(1, 7);

		if ( randomdescription == 1 ){
			strcat( bufrdesc, "A make-shift cart stands here, loaded with empty boxes and barrels. "); }
		else if ( randomdescription == 2 ){
			strcat( bufrdesc, "Small, slightly mutated pets roam the streets. "); }
		else if ( randomdescription == 3 ){
			strcat( bufrdesc, "You can see some dried blood on the floor. "); }
		else if ( randomdescription == 4 ){
			strcat( bufrdesc, "A fountain stands in the middle of this area. ");
			SET_BIT( nRoom->room_flags2 , ROOM_FOUNTAIN );
		}
		else if ( randomdescription == 5 ){
			strcat( bufrdesc, "Someone is advertising their crafted goods loudly. ");
			SET_BIT( nRoom->room_flags2 , ROOM_SHOPPING ); }
		else if ( randomdescription == 6 ){
			strcat( bufrdesc, "Someone has discarded some old machinery here. "); }
		else {
			strcat( bufrdesc, "Some criminals have been hanged here. "); }

		// walls

		roomwalls = number_range(1, 4);

		if (roomwalls == 1){
			strcat( bufrdesc, "A balcony has been built into one of the walls. ");
		}
		else if (roomwalls == 2){
			strcat( bufrdesc, "Make-shift living compartments have been attached to or built into the walls. ");
		}
		else if (roomwalls == 3){
			strcat( bufrdesc, "Metal railings keep citizens from falling down from this platform. ");
		}
		else {
			strcat( bufrdesc, "The walls are smooth. ");
		}

		//SET_BIT( nRoom->room_flags , ROOM_NO_MOB );

		nRoom->sector_type = SECT_CITY;
		break;

	case 6: // corridor

		// height

		roomheight = number_range(1, 3);

		if (roomheight == 1)
			strcat( bufrdesc, " the ceiling is low. ");
		else if (roomheight == 2)
			strcat( bufrdesc, " the ceiling is high. ");
		else
			strcat( bufrdesc, " the ceiling is massively high. ");

		// name

		strcpy( bufrname, "Corridor" );

		strcat( bufrdesc, " One of the thousands of corridors that connect areas in the Metropolis. ");

		// noise

		roomnoise = number_range(1, 3);

		if (roomnoise == 1){
			strcat( bufrdesc, "You think you hear some footsteps in the distance. ");
		}
		else if (roomnoise == 2){
			strcat( bufrdesc, "You can hear water drip from the ceiling. ");
			SET_BIT( nRoom->room_flags2 , ROOM_FOUNTAIN );
		}
		else {
			strcat( bufrdesc, "A monotonous hum fills this room. ");
		}

		// random detail

		randomdescription = number_range(1, 10);

		if ( randomdescription == 1 ){
			strcat( bufrdesc, "There is a metal grate in one of the walls. "); }
		else if ( randomdescription == 2 ){
			strcat( bufrdesc, "A large fan rotor has been grafted into one of the walls. "); }
		else if ( randomdescription == 3 ){
			strcat( bufrdesc, "Thin and thick trunks of wires run across the walls and floor. "); }
		else if ( randomdescription == 4 ){
			strcat( bufrdesc, "Big and small pipes run across the middle of the walls. ");
		}
		else if ( randomdescription == 5 ){
			strcat( bufrdesc, "A rusty, broken ladder leans against one of the walls. ");}
		else if ( randomdescription == 6 ){
			strcat( bufrdesc, "A large crossbeam dominates this room. "); }
		else if ( randomdescription == 7 ){
			strcat( bufrdesc, "The walls are riddled with bullet holes. "); }
		else if ( randomdescription == 8 ){
			strcat( bufrdesc, "A circuit box sits on one of the walls here. "); }
		else if ( randomdescription == 9 ){
			strcat( bufrdesc, "A lot of rubble is strewn across the floor. "); }
		else {
			strcat( bufrdesc, "Small, square lights are embedded in the walls of the room. "); }

		nRoom->sector_type = SECT_CORRIDOR;
		break;

	case 7: // elevator

		strcpy( bufrname, "Elevator" );

		strcpy( bufrdesc, "A large service elevator that is attached and runs on the rails in the wall behind it. ");

		islevelchange = TRUE;
		SET_BIT( nRoom->room_flags , ROOM_NOPEDIT );

		nRoom->sector_type = SECT_ELEVATOR;

		break;

	case 8: // factory

		// height

		roomheight = number_range(1, 3);

		if (roomheight == 1)
			strcat( bufrdesc, " the ceiling is low. ");
		else if (roomheight == 2)
			strcat( bufrdesc, " the ceiling is high. ");
		else
			strcat( bufrdesc, " the ceiling is massively high. ");

		// name

		strcpy( bufrname, "Factory Building" );

		strcat( bufrdesc, " This is an ancient factory building with lots of useful devices and machinery. ");

		// noise

		roomnoise = number_range(1, 3);

		if (roomnoise == 1){
			strcat( bufrdesc, "You can hear a grinding noise. ");
		}
		else if (roomnoise == 2){
			strcat( bufrdesc, "A constant whirring noise sounds through the room. ");
		}
		else {
			strcat( bufrdesc, "You can hear the sound of metal banging against metal. ");
		}

		// random detail

		randomdescription = number_range(1, 10);

		if ( randomdescription == 1 ){
			strcat( bufrdesc, "A semi-intact workbench is here. "); }
		else if ( randomdescription == 2 ){
			strcat( bufrdesc, "A large fan has been grafted into one of the walls. "); }
		else if ( randomdescription == 3 ){
			strcat( bufrdesc, "Long, colourful trunks of wires run along the walls of this rooms. "); }
		else if ( randomdescription == 4 ){
			strcat( bufrdesc, "Big and small pipes run across the middle of the walls. ");
		}
		else if ( randomdescription == 5 ){
			strcat( bufrdesc, "A massive metal press is in this room. ");}
		else if ( randomdescription == 6 ){
			strcat( bufrdesc, "Sewing and cloth presses occupy most of the space in this room. "); }
		else if ( randomdescription == 7 ){
			strcat( bufrdesc, "Scientific devices sit on top large tables. "); }
		else if ( randomdescription == 8 ){
			strcat( bufrdesc, "A furnace is in this room. "); }
		else if ( randomdescription == 9 ){
			strcat( bufrdesc, "Lots of boxes are stacked in the corners of this room. "); }
		else {
			strcat( bufrdesc, "Large crates are stacked in this storage area of the factory. "); }

		nRoom->sector_type = SECT_FACTORY;

		break;

	case 9: // farm

		roomnoise = number_range(1, 3);

		// height

		roomheight = number_range(1, 2);

		if (roomheight == 1)
			strcat( bufrdesc, " the ceiling is low. ");
		else
			strcat( bufrdesc, " the ceiling is high. ");

		// name

		if ( number_range(1, 2) == 1 ){
			strcpy( bufrname, "Food Storage" );
			strcat( bufrdesc, " This storage area is filled with shelves that hold food containers. ");

			strcat( bufrdesc, " You can smell the dried food in the containers. ");

		}
		else {
			strcpy( bufrname, "Spore Growth" );
			strcat( bufrdesc, " Someone is cultivating edible spores on one wall of this room. ");

			strcat( bufrdesc, " You can smell the delicious growth on the wall. ");

		}

		// noise

		if (roomnoise == 1)
			strcat( bufrdesc, " You can hear scattering noises coming from the walls. ");
		else if (roomnoise == 2){
			strcat( bufrdesc, " Water is dropping from a grate in one of the walls. ");
			SET_BIT( nRoom->room_flags2 , ROOM_FOUNTAIN );
		}
		else
			strcat( bufrdesc, " It is very quiet. ");

		// random detail

		randomdescription = number_range(1, 5);

		if ( randomdescription == 1 ){
			strcat( bufrdesc, "The whole area is well preserved. "); }
		else if ( randomdescription == 2 ){
			strcat( bufrdesc, "Small and large pipes run along the walls of the room. "); }
		else if ( randomdescription == 3 ){
			strcat( bufrdesc, "Some broken devices are lying on the floor. "); }
		else if ( randomdescription == 4 ){
			strcat( bufrdesc, "Several boxes are stacked in one corner of the room. "); }
		else {
			strcat( bufrdesc, "Some barrels have been left in this room. "); }

		// walls

		roomwalls = number_range(1, 2);

		if (roomwalls == 1){
			strcat( bufrdesc, "The walls are smooth and dry. ");
		}
		else {
			strcat( bufrdesc, "The walls are rough and wet. ");
		}

		SET_BIT( nRoom->room_flags2 , ROOM_FOOD );
		nRoom->sector_type = SECT_FARM;

		break;

	case 10: // hall

		// height

		roomheight = number_range(1, 3);

		if (roomheight == 1)
			strcat( bufrdesc, " the ceiling is high. ");
		else if (roomheight == 2)
			strcat( bufrdesc, " the ceiling is very high. ");
		else
			strcat( bufrdesc, " the ceiling is massively high. ");

		// name

		strcpy( bufrname, "Hallway" );

		strcat( bufrdesc, " This is one of the many magnificent hallways in the Metropolis. ");

		// noise

		strcat( bufrdesc, "All noises echo through the whole area. ");

		// random detail

		// wires, signs, circuit box, debries, rubble, spires, grill, archway, pillar, decorations, bullet holes

		randomdescription = number_range(1, 11);

		if ( randomdescription == 1 ){
			strcat( bufrdesc, "Thick and thin wire trunks run across the floor and walls. "); }
		else if ( randomdescription == 2 ){
			strcat( bufrdesc, "A sign with strange writings has been attached to one of the walls. "); }
		else if ( randomdescription == 3 ){
			strcat( bufrdesc, "A circuit box sits in one of the walls. "); }
		else if ( randomdescription == 4 ){
			strcat( bufrdesc, "Lots a debry that fell from above has gathered in this room. ");
		}
		else if ( randomdescription == 5 ){
			strcat( bufrdesc, "Big heaps of rubble block block some parts of this room. ");}
		else if ( randomdescription == 6 ){
			strcat( bufrdesc, "Strange spires grow out of the ground and build macabre archways. "); }
		else if ( randomdescription == 7 ){
			strcat( bufrdesc, "A rusty metal grate has been grafted into one of the walls. "); }
		else if ( randomdescription == 8 ){
			strcat( bufrdesc, "A beautiful archway decorates this hallway. "); }
		else if ( randomdescription == 9 ){
			strcat( bufrdesc, "Large, round pillars rise into the sky in this room. "); }
		else if ( randomdescription == 10 ){
			strcat( bufrdesc, "Intricate decorations have been added to some of the walls. "); }
		else {
			strcat( bufrdesc, "The walls are full of bullet holes. "); }

		nRoom->sector_type = SECT_HALL;

		break;

	case 12: // platform

		// height

		roomheight = number_range(1, 3);

		if (roomheight == 1)
			strcat( bufrdesc, " the open sky is far, far above you. ");
		else if (roomheight == 2)
			strcat( bufrdesc, " there is a ceiling just a few levels above this one. ");
		else
			strcat( bufrdesc, " there is a ceiling many levels above this one. ");

		// name

		strcpy( bufrname, "Open Platform" );

		strcat( bufrdesc, " This room is a platform, open to some of its sides. ");

		// noise

		// noise

		roomnoise = number_range(1, 3);

		if (roomnoise == 1){
			strcat( bufrdesc, "You hear the rushing of water nearby. ");
			SET_BIT( nRoom->room_flags2 , ROOM_FOUNTAIN );
		}
		else if (roomnoise == 2){
			strcat( bufrdesc, "A pool of slime is blubbering. ");
		}
		else {
			strcat( bufrdesc, "You can hear a weird buzzing noise. ");
		}

		// random detail

		// wires, compartment, metal grate, ventilation pipe, remains

		randomdescription = number_range(1, 5);

		if ( randomdescription == 1 ){
			strcat( bufrdesc, "Thick and thin wire trunks run across the floor and walls. "); }
		else if ( randomdescription == 2 ){
			strcat( bufrdesc, "A small compartment has been built into one of the walls. "); }
		else if ( randomdescription == 3 ){
			strcat( bufrdesc, "A metal grate has been grafted into one of the walls. "); }
		else if ( randomdescription == 4 ){
			strcat( bufrdesc, "The scattered remains of creatures litter the floor. ");
		}
		else {
			strcat( bufrdesc, "A large ventilation pipe is in one of the walls. "); }

		nRoom->sector_type = SECT_PLATFORM;

		break;

	case 13: // prison

		strcpy( bufrname, "Prison" );
		strcat( bufrdesc, "A holding area. It is barren except for some old, rusty metal instruments that might have been used for torture.");
		nRoom->sector_type = SECT_PRISON;
		SET_BIT( nRoom->room_flags , ROOM_NOPEDIT );

		break;

	case 14: // ramp

		strcpy( bufrname, "Ramp" );

		randomdescription = number_range(1, 3);

		if ( randomdescription == 1 ){
			strcat( bufrdesc, "Lots of wires and tubes run along the walls of this ramp. "); }
		else if ( randomdescription == 2 ){
			strcat( bufrdesc, "There are circuit boxes mounted on the walls of this ramp. "); }
		else if ( randomdescription == 3 ){
			strcat( bufrdesc, "The rails in the walls of this ramp could mean that an elevator is close. "); }

		nRoom->sector_type = SECT_RAMP;
		SET_BIT( nRoom->room_flags , ROOM_NOPEDIT );

		break;

	case 15: // ruins

		// height

		roomheight = number_range(1, 3);

		if (roomheight == 1)
			strcat( bufrdesc, " the open sky is far, far above you. ");
		else if (roomheight == 2)
			strcat( bufrdesc, " there is a ceiling just a few levels above this one. ");
		else
			strcat( bufrdesc, " there is a ceiling many levels above this one. ");

		// name

		strcpy( bufrname, "Ruins" );

		strcat( bufrdesc, " This area either has not withstood the trials of time or some catastrophe has happened here. Whichever it is, the builder seem to have abandoned this section of the Metropolis. ");

		// noise

		strcat( bufrdesc, "The wind blows noisily through here. ");

		// random detail

		// wires, signs, circuit box, debries, rubble, spires, grill, archway, pillar, decorations, bullet holes

		randomdescription = number_range(1, 11);

		if ( randomdescription == 1 ){
			strcat( bufrdesc, "Rusted and cut wire trunks run across the floor and walls. "); }
		else if ( randomdescription == 2 ){
			strcat( bufrdesc, "A battered sign with faded symbols dangles from the wall. "); }
		else if ( randomdescription == 3 ){
			strcat( bufrdesc, "A destroyed circuit box sits in one of the walls. "); }
		else if ( randomdescription == 4 ){
			strcat( bufrdesc, "Lots a debrie that fell from above has gathered in this room. ");
		}
		else if ( randomdescription == 5 ){
			strcat( bufrdesc, "Big heaps of rubble block block some parts of this room. ");}
		else if ( randomdescription == 6 ){
			strcat( bufrdesc, "Wire frames pop out of the ruins of the walls. "); }
		else if ( randomdescription == 7 ){
			strcat( bufrdesc, "A rusty metal grate hangs from one of the walls in this room. "); }
		else if ( randomdescription == 8 ){
			strcat( bufrdesc, "A big rock has caused one of the walls to collapse. "); }
		else if ( randomdescription == 9 ){
			strcat( bufrdesc, "Broken pillars dot the room. "); }
		else if ( randomdescription == 10 ){
			strcat( bufrdesc, "Scraped and broken decorations remain on some of the cracked walls. "); }
		else {
			strcat( bufrdesc, "The ancient remains of some creatures litter the floor. "); }

		nRoom->sector_type = SECT_RUINS;

		break;

	case 16: // settlement

		// height

		roomheight = number_range(1, 3);

		if (roomheight == 1)
			strcat( bufrdesc, " the open sky is far, far above you. ");
		else if (roomheight == 2)
			strcat( bufrdesc, " there is a ceiling just a few levels above this one. ");
		else
			strcat( bufrdesc, " there is a ceiling many levels above this one. ");

		// name

		strcpy( bufrname, "Human Settlement" );

		strcat( bufrdesc, " Humans have started to convert this open area into a small settlement. ");

		// noise

		roomnoise = number_range(1, 3);

		if (roomnoise == 1){
			strcat( bufrdesc, "Chatter from a group of colonists can be heard in this room. ");
		}
		else if (roomnoise == 2){
			strcat( bufrdesc, "A dog is barking loudly. ");
		}
		else {
			strcat( bufrdesc, "An injured colonist is moaning in one corner of the room. ");
		}

		// random detail

		randomdescription = number_range(1, 5);

		if ( randomdescription == 1 ){
			strcat( bufrdesc, "You can see some dried blood on the floor. "); }
		else if ( randomdescription == 2 ){
			strcat( bufrdesc, "A small water hole has been dug in the middle of the room. ");
			SET_BIT( nRoom->room_flags2 , ROOM_FOUNTAIN );
		}
		else if ( randomdescription == 3 ){
			strcat( bufrdesc, "Someone is advertising their crafted goods loudly. ");
			SET_BIT( nRoom->room_flags2 , ROOM_SHOPPING ); }
		else if ( randomdescription == 4 ){
			strcat( bufrdesc, "Someone has discarded some broken equipment here. "); }
		else {
			strcat( bufrdesc, "Boxes and crates are piled up in one corner of this room. "); }

		// walls

		roomwalls = number_range(1, 4);

		if (roomwalls == 1){
			strcat( bufrdesc, "A balcony has been built into one of the walls. ");
		}
		else if (roomwalls == 2){
			strcat( bufrdesc, "Make-shift living compartments have been attached to or built into the walls. ");
		}
		else if (roomwalls == 3){
			strcat( bufrdesc, "Metal railings keep citizens from falling down from this platform. ");
		}
		else {
			strcat( bufrdesc, "The walls are smooth. ");
		}

		//SET_BIT( nRoom->room_flags , ROOM_NO_MOB );

		nRoom->sector_type = SECT_SETTLEMENT;
		break;

	case 17: // shelter

		strcpy( bufrname, "Shelter" );

		randomdescription = number_range(1, 3);

		if ( randomdescription == 1 ){
			strcat( bufrdesc, "This area seems to be safe to rest in. A bench leans against one of the walls. "); }
		else if ( randomdescription == 2 ){
			strcat( bufrdesc, "Colonists have left old rags here that one can rest on. "); }
		else if ( randomdescription == 3 ){
			strcat( bufrdesc, "This area must have once been a park. The plots are empty and the earth is like stone. Some benches line the walls. "); }

		SET_BIT( nRoom->room_flags , ROOM_NO_MOB );
		SET_BIT( nRoom->room_flags , ROOM_SAFE );

		nRoom->sector_type = SECT_SHELTER;
		break;

	case 18: // stairway

		strcpy( bufrname, "Stairway" );

		randomdescription = number_range(1, 3);

		if ( randomdescription == 1 ){
			strcat( bufrdesc, "This stairway is cracked and weathered. "); }
		else if ( randomdescription == 2 ){
			strcat( bufrdesc, "Exquisite craftmanship made sure that this stairway stands tall when faced with time. "); }
		else if ( randomdescription == 3 ){
			strcat( bufrdesc, "The Metropolis stretches around you as far as you can see from this stairway. "); }

		SET_BIT( nRoom->room_flags , ROOM_NOPEDIT );

		islevelchange = TRUE;

		nRoom->sector_type = SECT_STAIRWAY;

		break;

	case 19: // tower

		strcpy( bufrname, "Tower" );

		randomdescription = number_range(1, 3);

		if ( randomdescription == 1 ){
			strcat( bufrdesc, "Dark and brooding, this tower leads to a different level of the Metropolis. "); }
		else if ( randomdescription == 2 ){
			strcat( bufrdesc, "The lightly coloured material that this tower was made from distracts from all of the dried blood that decorates the walls. "); }
		else if ( randomdescription == 3 ){
			strcat( bufrdesc, "Winding its way through the megastructure, this tower heralds the change to a new level of the Metropolis. "); }

		SET_BIT( nRoom->room_flags , ROOM_NOPEDIT );

		islevelchange = TRUE;

		nRoom->sector_type = SECT_TOWER;
		break;

	case 20: // tunnel

		strcpy( bufrname, "Tunnel" );

		randomdescription = number_range(1, 3);

		if ( randomdescription == 1 ){
			strcat( bufrdesc, "This tunnel was carved into the walls of the megastructure. "); }
		else if ( randomdescription == 2 ){
			strcat( bufrdesc, "A natural tunnel was burried here. "); }
		else if ( randomdescription == 3 ){
			strcat( bufrdesc, "This section of tunnel is full of wires and rubble. "); }

		nRoom->sector_type = SECT_TUNNEL;
		break;

	}

	// 1 out of 10 rooms are dead-ends

	if (!islevelchange)
	{
		if (number_range(1, 10)  == 1 && nRoom->sector_type != SECT_CITY)
		{
			SET_BIT( nRoom->room_flags2 , ROOM_DEADEND );
			strcat( bufrdesc, "It seems that this area is a dead-end.");
		}
	}

	// 1 out of 3 rooms will have another room attached to them

	if (number_range(1, 3)  == 1 && !islevelchange)
	{
		hasexit = TRUE;
	}

	// assign name and description

	strcat( bufrdesc, "\n\r");

	nRoom->name = STRALLOC( bufrname );
	nRoom->description = STRALLOC ( bufrdesc );

	// make room

	nRoom->owner = STRALLOC( "unknown" );

	xit = make_exit( ch->in_room, nRoom, edir );
	xit->keyword		= STRALLOC( "" );
	xit->description	= STRALLOC( "" );
	xit->key		= -1;
	xit->exit_info	= 0;
	xit->distance	= roomdistance;

	xit2 = make_exit( nRoom , ch->in_room  , rev_dir[edir] );
	xit2->keyword		= STRALLOC( "" );
	xit2->description	= STRALLOC( "" );
	xit2->key		= -1;
	xit2->exit_info	= 0;
	xit2->distance	= roomdistance;

	ch->in_room->area->planet->size++;

	learn_from_success( ch , gsn_spacecraft );

	if ( number_percent() == 23 )
	{
		send_to_char( "You feel more intelligent than before.\n\r", ch );
		ch->perm_int++;
		ch->perm_int = UMIN( ch->perm_int , 25 );
	}

	// stairway, elevator or tower

	if (islevelchange){

		if ( roomlevel == 0 )
		{
			roomlevel = roomlevel + 1;
			edir = 4;
		}
		if ( roomlevel == 5 )
		{
			roomlevel = roomlevel - 1;
			edir = 5;
		}
		else {
			if (number_range(1, 2) == 1){
				roomlevel = roomlevel + 1;
				edir = 4; }
			else
			{
				roomlevel = roomlevel - 1;
				edir = 5;
			}

		}

		lRoom = make_room( ++top_r_vnum );
		lRoom->area = ch->in_room->area;
		LINK( lRoom , ch->in_room->area->first_room , ch->in_room->area->last_room , next_in_area , prev_in_area );
		STRFREE( lRoom->name );
		STRFREE( lRoom->description );
		lRoom->level = roomlevel;

		strcpy( bufrname, "Shelter" );

		randomdescription = number_range(1, 3);

		if ( randomdescription == 1 ){
			strcpy( bufrdesc, "This area seems to be safe to rest in. A bench leans against one of the walls. "); }
		else if ( randomdescription == 2 ){
			strcpy( bufrdesc, "Colonists have left old rags here that one can rest on. "); }
		else if ( randomdescription == 3 ){
			strcpy( bufrdesc, "This area must have once been a park. The plots are empty and the earth is like stone. Some benches line the walls. "); }

		SET_BIT( lRoom->room_flags , ROOM_NO_MOB );
		SET_BIT( lRoom->room_flags , ROOM_SAFE );

		lRoom->sector_type = SECT_SHELTER;

		strcat( bufrdesc, "\n\r");

		lRoom->name = STRALLOC( bufrname );
		lRoom->description = STRALLOC ( bufrdesc );

		// make room

		lRoom->owner = STRALLOC( "unknown" );

		xit = make_exit( nRoom, lRoom, edir );
		xit->keyword		= STRALLOC( "" );
		xit->description	= STRALLOC( "" );
		xit->key		= -1;
		xit->exit_info	= 0;
		xit->distance	= roomdistance;

		xit2 = make_exit( lRoom , nRoom  , rev_dir[edir] );
		xit2->keyword		= STRALLOC( "" );
		xit2->description	= STRALLOC( "" );
		xit2->key		= -1;
		xit2->exit_info	= 0;
		xit2->distance	= roomdistance;

		ch->in_room->area->planet->size++;

	}

	// has another exit
	if (number_range(1, 10) == 10)
	reset_all();

	SET_BIT( ch->in_room->area->flags , AFLAG_MODIFIED );

	return;

}
