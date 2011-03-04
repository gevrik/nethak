/*
 * WildMatrix.C
 *
 *  Created on: 24 august 2010
 *      Author: aiseant
 *
 * Mostly based on abyss.c
 * but surf command and free
 */

#include <math.h>
#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "mud.h"

extern int top_r_vnum;

void followsurf( CHAR_DATA *ch , char *argument );

void do_surf ( CHAR_DATA *ch , char *argument )
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

	if ( str_cmp(ch->in_room->area->planet->name, "WildMatrix") )
	{
		send_to_char( "> &RYou can not do that in this system&w\n\r", ch );
		return;
	}

	if( !IS_IMMORTAL(ch) )
		if ( ch->in_room->area->planet->size >= 9999 )
		{
			send_to_char( "> WildMatrix is fully revealed\n\r", ch );
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
		send_to_char( "> &RYou cannot surf at this address, try:\n\r&w", ch);
		send_to_char( "north, east, south, west\n\r", ch);
		return;
	}

	if( !IS_IMMORTAL(ch) )
	{
		if ( IS_SET( ch->in_room->room_flags2 , ROOM_DEADEND ) )
		{
			send_to_char( "> &RThis site is a dead-end&w\n\r", ch );
			return;
		}

	}

	if ( argument[0] == '\0' )
	{
		send_to_char( "> &YSpecify address for surf&w\n\r", ch );
		return;
	}

	edir = get_dir(argument);
	xit = get_exit(ch->in_room, edir);
	if ( xit )
	{
		send_to_char( "> &RThere is already a site at this address&w\n\r", ch );
		return;
	}

	chance = (int) (ch->pcdata->learned[gsn_spacecraft]);
	if ( number_percent( ) > chance )
	{
		send_to_char( "> &RYou fail to surf - try again&w\n\r", ch );
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
		roomtype = 1;
		break;


	case SECT_STACK:

		// Stack 20
		// 404 35
		// WebBrowser 5
		// Cookie 5
		// ChatRoom 5
		// Index.php 10
		// Host 10
		// Proxy 10

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

	case SECT_LINK:

		// Link 25
		// Wiki 15
		// Torrent 10
		// 404 5
		// Index.php 20
		// Server 15
		// Forum 5
		// Nexus 5

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

	case SECT_WIKI:

		// Link 10
		// Wiki 25
		// Torrent 5
		// 404 10
		// WebBrowser 5
		// ChatRoom 5
		// Index.php 5
		// Server 5
		// Host 5
		// Forum 15
		// BrokenPort 5
		// Proxy 5

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

	case SECT_404:
		// Stack 5
		// Link 10
		// 404 25
		// WebBrowser 5
		// Cookie 5
		// ChatRoom 5
		// Index.php 5
		// Server 5
		// Hyperlink 5
		// Host 5
		// Forum 5
		// BrokenPort 5
		// Nexus 5
		// Proxy 10

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
			roomtype = 20;

		}
		else if (newareatype >= 30) {
			roomtype = 7;

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
			roomtype = 2;
		}
		else if (newareatype >= 5) {
			roomtype = 19;
		}
		else {
			roomtype = 20;
		}

		break;

	case SECT_WEBBROWSER:
		// Stack 5
		// Wiki 10
		// 404 5
		// WebBrowser 15
		// ChatRoom 5
		// Index.php 10
		// Server 10
		// Hyperlink 5
		// Host 10
		// Forum 5
		// BrokenPort 5
		// Link 5
		// Nexus 5
		// Proxy 5

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
			roomtype = 7;
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
			roomtype = 2;
		}
		else if (newareatype >= 5) {
			roomtype = 19;
		}
		else {
			roomtype = 20;
		}

		break;


		case SECT_COOKIE:
		// 404 50
		// Cookie 50

		if (newareatype >= 50) {
			roomtype = 5;
		}
		else {
			roomtype = 8;
		}

		break;

	case SECT_CHATROOM:
		// Stack 5
		// Wiki 5
		// 404 15
		// WebBrowser 15
		// ChatRoom 20
		// Forum 10
		// BrokenPort 15
		// Nexus 10
		// Proxy 5

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

	case SECT_INDEX:
		// Stack 5
		// Link 20
		// Wiki 10
		// 404 10
		// WebBrowser 5
		// Index.php 10
		// Server 10
		// Host 10
		// Forum 10
		// BrokenPort 10

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
			roomtype = 2;
		}

		break;

	case SECT_SERVER:
		// Link 10
		// Wiki 5
		// 404 5
		// WebBrowser 10
		// Index.php 10
		// Server 15
		// Hyperlink 5
		// Host 10
		// Forum 5
		// BrokenPort 10
		// Nexus 5
		// Proxy 10

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
			roomtype = 7;

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


	case SECT_HOST:
		// Stack 5
		// Wiki 5
		// 404 5
		// WebBrowser 15
		// Index.php 15
		// Server 10
		// Host 15
		// Forum 5
		// BrokenPort 5
		// Link 5
		// Nexus 5
		// Proxy 5

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
			roomtype = 2;
		}
		else if (newareatype >= 5) {
			roomtype = 19;
		}
		else {
			roomtype = 20;
		}


		break;

	case SECT_FORUM:
		// Link 5
		// Wiki 5
		// 404 5
		// WebBrowser 5
		// ChatRoom 20
		// Index.php 10
		// Server 10
		// Host 5
		// Forum 20
		// BrokenPort 10
		// Proxy 5

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

	case SECT_BROKENPORT:
		// Wiki 10
		// 404 10
		// WebBrowser 10
		// ChatRoom 20
		// Index.php 10
		// Server 10
		// Host 10
		// Forum 10
		// Proxy 10

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


	case SECT_NEXUS:
		// Link 10
		// 404 5
		// WebBrowser 5
		// ChatRoom 5
		// Server 5
		// Nexus 65

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
			roomtype = 2;
		}
		else {
			roomtype = 19;
		}

		break;

	case SECT_PROXY:
		// Stack 10
		// Wiki 10
		// 404 10
		// WebBrowser 10
		// ChatRoom 10
		// Server 10
		// Host 10
		// Forum 10
		// BrokenPort 10
		// Proxy 10

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
		strcat( bufrdesc, "site written in very basic html.\n");
	}
	else if (roommaterial == 2)
	{
		strcat( bufrdesc, "site written in basic html.\n");
	}
	else if (roommaterial == 3)
	{
		strcat( bufrdesc, "site written in html with some css.\n");
	}
	else if (roommaterial == 4)
	{
		strcat( bufrdesc, "site written in html and a strange derivated of php.\n");
	}
	else if (roommaterial == 5)
	{
		strcat( bufrdesc, "site written in a strange derivated of php.\n");
	}
	else if (roommaterial == 6)
	{
		strcat( bufrdesc, "site written in php with traces of java.\n");
	}
	else if (roommaterial == 7)
	{
		strcat( bufrdesc, "site fully written with javascript.\n");
	}
	else if (roommaterial == 8)
	{
		strcat( bufrdesc, "site made of poor java and flash animations.\n");
	}
	else if (roommaterial == 9)
	{
		strcat( bufrdesc, "site made of very nice flash layout.\n");
	}
	else
	{
		strcat( bufrdesc, "site written with a mix of xhtml and java.\n");
	}

	// rest

	switch (roomtype) {

	default:
		break;

		case 1: // Stack


		roomnoise = number_range(1, 3);

		// height

		roomheight = number_range(1, 2);

		if (roomheight == 1)
			strcat( bufrdesc, " The ceiling is low.\n");
		else
			strcat( bufrdesc, " The ceiling is high.\n");

		// name

		if ( number_range(1, 2) == 1 ){
			strcpy( bufrname, "Stack" );
			strcat( bufrdesc, "You are on top of a pyramid made of grey cubes.\n");
		}
		else {
			strcpy( bufrname, "Stack" );
			strcat( bufrdesc, "Numerous grey cubes lies on the floor.\n");
		}

		if (roomnoise == 1)
			strcat( bufrdesc, "You hear an irregular ticking noise.\n");
		else if (roomnoise == 2)
			strcat( bufrdesc, "There is a static sound in there.\n");
		else
			strcat( bufrdesc, "You hear a loud noise as another cube falls on the floor out of nowhere.\n");

		strcat( bufrdesc, " LIFO is written on the walls everywhere.\n");



		// random detail

		randomdescription = number_range(1, 5);

		if ( randomdescription == 1 ){
			strcat( bufrdesc, "Engraved on a cube you can read: Erno was here.\n"); }
		else if ( randomdescription == 2 ){
			strcat( bufrdesc, "Small bugs crawl over useless bits.\n"); }
		else if ( randomdescription == 3 ){
			strcat( bufrdesc, "One of the cube is cracked open, written on its side is: Do not open, E.S.\n"); }
		else if ( randomdescription == 4 ){
			strcat( bufrdesc, "This smells rust and dust.\n"); }
		else {
			strcat( bufrdesc, "One cube is pink and not grey.\n"); }

		// walls

		roomwalls = number_range(1, 2);

		if (roomwalls == 1){
			strcat( bufrdesc, "The walls are smooth.\n");
		}
		else {
			strcat( bufrdesc, "On one wall, you find a lever, but it is broken.\n");
		}

		SET_BIT( nRoom->room_flags , ROOM_NOPEDIT );
		nRoom->sector_type = SECT_STACK;

		break;

	case 2: // Link

        // height

        roomheight = number_range(1, 3);

        if (roomheight == 1)
            strcat( bufrdesc, "You can not see a ceiling above you.\n");
        else if (roomheight == 2)
            strcat( bufrdesc, "There is a ceiling just a few levels above this one.\n");
        else
            strcat( bufrdesc, "There is a ceiling many levels above this one.\n");

        // name

        strcpy( bufrname, "Link" );

        strcat( bufrdesc, "You are on a bridge built out of chains, some of which are broken.\n");
        // noise

        roomnoise = number_range(1, 2);

        if (roomnoise == 1){
            strcat( bufrdesc, "The only sound here is the one the chains make in the cold wind.\n");
        }
        else {
            strcat( bufrdesc, "You hear bugs flying around here.\n");
        }

        // random detail

        randomdescription = number_range(1, 7);

        if ( randomdescription == 1 ){
            strcat( bufrdesc, "You see a picture of three golden triangles on a wall.\n"); }
        else if ( randomdescription == 2 ){
            strcat( bufrdesc, "There are padlocks in some chains.\n"); }
        else if ( randomdescription == 3 ){
            strcat( bufrdesc, "One of the chain is blue, another is violet.\n"); }
        else if ( randomdescription == 4 ){
            strcat( bufrdesc, "Broken shackles are tied to the bridge.\n"); }
        else if ( randomdescription == 5 ){
            strcat( bufrdesc, "There are rust spoys everywhere.\n"); }
        else if ( randomdescription == 6 ){
            strcat( bufrdesc, "Three human hands with pointing fingers lay there.\n"); }
        else {
            strcat( bufrdesc, "On one chain is marked : dead"); }

        // walls

        roomwalls = number_range(1, 3);

        if (roomwalls == 1){
            strcat( bufrdesc, "On the bottom of a wall is written: The cake is a lie.\n");
        }
	if (roomwalls == 2){
            strcat( bufrdesc, "Almost unreadable, there's a message on the wall : \nSomething Wicked Is Coming This Way.\n");
        }
        else {
            strcat( bufrdesc, "On a wall is written: I am hidden.\n");
        }

        SET_BIT( nRoom->room_flags , ROOM_NOPEDIT );
        nRoom->sector_type = SECT_LINK;
        break;

	case 3: // Wiki

		// height

		roomheight = number_range(1, 3);

		if (roomheight == 1)
		    strcat( bufrdesc, "The ceiling is high.\n");
		else
		    strcat( bufrdesc, "If there's a ceiling, you can't see it.\n");

		strcpy( bufrname, "Wikisite" );

		strcat( bufrdesc, "There is a crumbling gargantuan octopus made of wires here.\n");

		// noise

		roomnoise = number_range(1, 2);

		if (roomnoise == 1){
		    strcat( bufrdesc, "You hear something like many books being flipped simultaneously.\n");
		}
		else {
		    strcat( bufrdesc, "It is very quiet.\n");
		}

		// random detail

		randomdescription = number_range(1, 7);

		if ( randomdescription == 1 ){
		    strcat( bufrdesc, "Burned papers are scattered all over the floor.\n"); }
		else if ( randomdescription == 2 ){
		    strcat( bufrdesc, "An worn out sign says: reference needed.\n"); }
		else if ( randomdescription == 3 ){
		    strcat( bufrdesc, "The eyes of the octopus are made of puzzle pieces.\n"); }
		else if ( randomdescription == 4 ){
		    strcat( bufrdesc, "There is a desk here, absolutly clean.\n"); }
		else if ( randomdescription == 5 ){
		    strcat( bufrdesc, "Two barrel of paint are here: one blue and one red.\n"); }
		else if ( randomdescription == 6 ){
		    strcat( bufrdesc, "Banana peals and peanuts are scattered around a pile of anthropoids bones.\n"); }
		else {
		    strcat( bufrdesc, "A piece of paper is pinned on a tentacle, you can read: I am Edith.\n"); }

		// walls

		roomwalls = number_range(1, 2);

		if (roomwalls == 1){
		    strcat( bufrdesc, "On a wall is a big board saying: DELETE EVERYTHING.\n");
		}
		else {
		    strcat( bufrdesc, "Old surveillance cameras are on the walls.\n");
		    SET_BIT( nRoom->room_flags2 , ROOM_FOUNTAIN );
		}

		//SET_BIT( nRoom->room_flags , ROOM_NO_MOB );

		SET_BIT( nRoom->room_flags , ROOM_NOPEDIT );
		nRoom->sector_type = SECT_WIKI;
		break;

	case 4: // Torrent

		strcpy( bufrname, "Torrent" );

		randomdescription = number_range(1, 3);

		if ( randomdescription == 1 ){
			strcpy( bufrdesc, "A seemingly endless Torrent runs until out-of-sight.\n"); }
		else if ( randomdescription == 2 ){
			strcpy( bufrdesc, "A seemingly endless and wide Torrent that stretches out into all directions around you.\n"); }
		else if ( randomdescription == 3 ){
			strcpy( bufrdesc, "You see a seemingly endless Torrent that is filled with some kind of dark mist.\n"); }

		//SET_BIT( nRoom->room_flags , ROOM_NO_MOB );
		SET_BIT( nRoom->room_flags , ROOM_NOPEDIT );

		nRoom->sector_type = SECT_TORRENT;
		break;

	case 5: // 404

		// height

		roomheight = number_range(1, 3);

		if (roomheight == 1)
			strcat( bufrdesc, "The ceiling is so high that you can just imagine it.\n");
		else if (roomheight == 2)
			strcat( bufrdesc, "There is a ceiling just a few levels above this one.\n");
		else
			strcat( bufrdesc, "There is a ceiling many levels above this one.\n");

		// name

		strcpy( bufrname, "404" );

		strcat( bufrdesc, "You are in a 404 site of WildMatrix. It is desert.\n");

		// noise

		roomnoise = number_range(1, 3);

		if (roomnoise == 1){
			strcat( bufrdesc, "The only sound you can hear is your footstep.\n");
		}
		else if (roomnoise == 2){
			strcat( bufrdesc, "You hear the sound of the CyberSpace wind, blowing through this empty site.\n");
		}
		else {
			strcat( bufrdesc, "The whole structure is scratching and grinding.\n");
		}

		// walls

		roomwalls = number_range(1, 10);

		if (roomwalls <= 3){
			strcat( bufrdesc, "Walls are made of rusty metal plates, spotted with strange scratches.\n");
		}
		else if (roomwalls <= 6){
			strcat( bufrdesc, "All data flows seem to end here and never go out.\n");
		}
		else if (roomwalls == 7){
			strcat( bufrdesc, "You would say that something uncivilized happened here.\n");
		}
		else {
			strcat( bufrdesc, "Walls are made of smooth metal plates, \nspeckled with rust and maladjusted.\n");
		}

		// random detail

		randomdescription = number_range(1, 7);

		if ( randomdescription <= 6 ){
			strcat( bufrdesc, "\"404 error - PAGE NOT FOUND\" is written everywhere here"); }
		else {
			strcat( bufrdesc, "For some strange reason, somebody painted a \"200 OK\" message on the wall.\n"); }



		//SET_BIT( nRoom->room_flags , ROOM_NO_MOB );
		SET_BIT( nRoom->room_flags , ROOM_NOPEDIT );
		nRoom->sector_type = SECT_404;
		break;

	case 6: // WebBrowser

		// height

		roomheight = number_range(1, 3);

		if (roomheight == 1)
			strcat( bufrdesc, "The ceiling is low.\n");
		else if (roomheight == 2)
			strcat( bufrdesc, "The ceiling is high.\n");
		else
			strcat( bufrdesc, "The ceiling is massively high.\n");

		// name

		strcpy( bufrname, "WebBrowser" );

		strcat( bufrdesc, "One of the thousands of WebBrowsers that allow you to surf between sites of WildMatrix.\n");

		// noise

		roomnoise = number_range(1, 3);

		if (roomnoise == 1){
			strcat( bufrdesc, "You think you hear some footsteps in the distance.\n");
		}
		else if (roomnoise == 2){
			strcat( bufrdesc, "You can hear water drip from the ceiling.\n");
			SET_BIT( nRoom->room_flags2 , ROOM_FOUNTAIN );
		}
		else {
			strcat( bufrdesc, "The whistle of the CyberSpace wind fills this empty room.\n");
		}

		// random detail

		randomdescription = number_range(1, 10);

		if ( randomdescription == 1 ){
			strcat( bufrdesc, "There is a metal grate in one of the walls.\n"); }
		else if ( randomdescription == 2 ){
			strcat( bufrdesc, "A large fan rotor has been grafted into one of the walls.\n"); }
		else if ( randomdescription == 3 ){
			strcat( bufrdesc, "Thin and thick trunks of wires run across the walls and floor.\n"); }
		else if ( randomdescription == 4 ){
			strcat( bufrdesc, "Big and small pipes run across the middle of the walls.\n");
		}
		else if ( randomdescription == 5 ){
			strcat( bufrdesc, "A rusty, broken ladder leans against one of the walls.\n");}
		else if ( randomdescription == 6 ){
			strcat( bufrdesc, "A large crossbeam dominates this room.\n"); }
		else if ( randomdescription == 7 ){
			strcat( bufrdesc, "The walls are riddled with bullet holes.\n"); }
		else if ( randomdescription == 8 ){
			strcat( bufrdesc, "A circuit box sits on one of the walls here.\n"); }
		else if ( randomdescription == 9 ){
			strcat( bufrdesc, "A lot of rubble is strewn across the floor.\n"); }
		else {
			strcat( bufrdesc, "Small, square lights are embedded in the walls of the room.\n"); }

		SET_BIT( nRoom->room_flags , ROOM_NOPEDIT );
		nRoom->sector_type = SECT_WEBBROWSER;
		break;

	case 7: // Hyperlink

		strcpy( bufrname, "Hyperlink" );

		strcpy( bufrdesc, "You can follow this Hyperlink until the next level.\n");

		islevelchange = TRUE;
		SET_BIT( nRoom->room_flags , ROOM_NOPEDIT );

		nRoom->sector_type = SECT_HYPERLINK;

		break;

	case 8: // Cookie

		// height

		roomheight = number_range(1, 3);

		if (roomheight == 1)
			strcat( bufrdesc, "The ceiling is low.\n");
		else if (roomheight == 2)
			strcat( bufrdesc, "The ceiling is high.\n");
		else
			strcat( bufrdesc, "The ceiling is massively high.\n");

		// name

		strcpy( bufrname, "Cookie" );

		strcat( bufrdesc, "This is an ancient Cookie.\n");

		// noise

		roomnoise = number_range(1, 3);

		if (roomnoise == 1){
			strcat( bufrdesc, "You can hear a grinding noise.\n");
		}
		else if (roomnoise == 2){
			strcat( bufrdesc, "A constant whirring noise sounds through the room.\n");
		}
		else {
			strcat( bufrdesc, "You can hear the sound of metal rubbing against metal.\n");
		}

		// random detail

		randomdescription = number_range(1, 10);

		if ( randomdescription <= 2 ){
			strcat( bufrdesc, "The cookie is full of lines and lines of encrypted connexions.\n"); }
		else if ( randomdescription <= 4 ){
			strcat( bufrdesc, "You would say that there's at least an hundred of identifiers contained here.\n"); }
		else if ( randomdescription == 3 ){
			strcat( bufrdesc, "Wait ... you can read this code ... but it wasn't a real cookie, it was a spyware ! "); }
		else {
			strcat( bufrdesc, "Walls are covered with lines of code almost unreadable, rub out by the time.\n"); }

		SET_BIT( nRoom->room_flags , ROOM_NOPEDIT );
		nRoom->sector_type = SECT_COOKIE;

		break;

	case 9: // ChatRoom

		roomnoise = number_range(1, 3);

		// height

		roomheight = number_range(1, 2);

		if (roomheight == 1)
			strcat( bufrdesc, "The ceiling is low.\n");
		else
			strcat( bufrdesc, "The ceiling is high.\n");

		// name

		if ( number_range(1, 2) == 1 ){
			strcpy( bufrname, "ChatRoom" );
			strcat( bufrdesc, "Obviously, this place was a very popular ChatRoom.\n");

		}
		else {
			strcpy( bufrname, "ChatRoom" );
			strcat( bufrdesc, "You cannot imagine what kind of people would use this ChatRoom.\n");

		}

		// noise

		if (roomnoise == 1)
			strcat( bufrdesc, "You can still hear old echos of conversations which stand here long time ago.\n");
		else if (roomnoise == 2){
			strcat( bufrdesc, "Chattering shut up a long long time ago");
			SET_BIT( nRoom->room_flags2 , ROOM_FOUNTAIN );
		}
		else
			strcat( bufrdesc, "It is very quiet. For the moment.\n");

		// walls

		roomwalls = number_range(1, 2);

		if (roomwalls == 1){
			strcat( bufrdesc, "The walls are covered with billions of old conversation lines.\n");
		}
		else {
			strcat( bufrdesc, "The walls are covered with very odds caracters that you cannot read.\n");
		}

		// random detail

		randomdescription = number_range(1, 10);

		if ( randomdescription == 1 ){
			strcat( bufrdesc, "On a wall is written : \"WELCOME NEWFAG !\" "); }
		else if ( randomdescription == 2 ){
			strcat( bufrdesc, "Scribbled on the bottom of one of the walls, you can read \"I was here\""); }
		else if ( randomdescription == 3 ){
			strcat( bufrdesc, "Written is almost unreadable font, you think decipher the sentence \"He comes\".\n"); }
		else if ( randomdescription == 4 ){
			strcat( bufrdesc, "You realize that, apparently, all the users of this chat was nammed John or Dave.\n"); }
		else {
			strcat( bufrdesc, "Some barrels have been left in this room.\n"); }


		SET_BIT( nRoom->room_flags2 , ROOM_FOOD );
		SET_BIT( nRoom->room_flags , ROOM_NOPEDIT );
		nRoom->sector_type = SECT_CHATROOM;

		break;

	case 10: // Index.php

		// height

		roomheight = number_range(1, 3);

		if (roomheight == 1)
			strcat( bufrdesc, "The ceiling is high.\n");
		else if (roomheight == 2)
			strcat( bufrdesc, "The ceiling is very high.\n");
		else
			strcat( bufrdesc, "The ceiling is massively high.\n");

		// name

		strcpy( bufrname, "Index.php" );

		strcat( bufrdesc, "An Index.php site, a kind of site that invaded \nthe place during its golden years.\n");

		// noise
			strcat( bufrdesc, "The ceiling is massively high.\n");

		// random detail

		// wires, signs, circuit box, debries, rubble, spires, grill, archway, pillar, decorations, bullet holes

		randomdescription = number_range(1, 11);

		if ( randomdescription == 1 ){
			strcat( bufrdesc, "Thick and thin wire trunks run across the floor and walls.\n"); }
		else if ( randomdescription == 2 ){
			strcat( bufrdesc, "A sign with strange writings has been attached to one of the walls.\n"); }
		else if ( randomdescription == 3 ){
			strcat( bufrdesc, "A circuit box sits in one of the walls.\n"); }
		else if ( randomdescription == 4 ){
			strcat( bufrdesc, "Lots a debry that fell from above has gathered in this room.\n");
		}
		else if ( randomdescription == 5 ){
			strcat( bufrdesc, "Big heaps of rubble block block some parts of this room.\n");}
		else if ( randomdescription == 6 ){
			strcat( bufrdesc, "Strange spires grow out of the ground and build macabre archways.\n"); }
		else if ( randomdescription == 7 ){
			strcat( bufrdesc, "A rusty metal grate has been grafted into one of the walls.\n"); }
		else if ( randomdescription == 8 ){
			strcat( bufrdesc, "A beautiful archway decorates this Index.php.\n"); }
		else if ( randomdescription == 9 ){
			strcat( bufrdesc, "Large, round pillars rise into the sky in this room.\n"); }
		else if ( randomdescription == 10 ){
			strcat( bufrdesc, "intricate decorations have been added to some of the walls.\n"); }
		else {
			strcat( bufrdesc, "The walls are full of bullet holes.\n"); }

		SET_BIT( nRoom->room_flags , ROOM_NOPEDIT );
		nRoom->sector_type = SECT_INDEX;

		break;

	case 12: // Server

		// height

		roomheight = number_range(1, 3);

		if (roomheight == 1)
			strcat( bufrdesc, "The open sky is far, far above you.\n");
		else if (roomheight == 2)
			strcat( bufrdesc, "There is a ceiling just a few levels above this one.\n");
		else
			strcat( bufrdesc, "There is a ceiling many levels above this one.\n");

		// name

		strcpy( bufrname, "Server" );

		strcat( bufrdesc, "This room was a Server, a wide and accessible storage for everything and everybody.\n");

		// noise

		// noise

		roomnoise = number_range(1, 3);

		if (roomnoise == 1){
			strcat( bufrdesc, "A pool of slime is blubbering in the center of the site.\n");
		}
		else {
			strcat( bufrdesc, "You can hear a weird buzzing noise but cannot localize the source.\n");
		}

		// random detail

		// wires, compartment, metal grate, ventilation pipe, remains

		randomdescription = number_range(1, 5);

		if ( randomdescription == 1 ){
			strcat( bufrdesc, "Thick and thin wire trunks run across the floor and walls.\n"); }
		else if ( randomdescription == 2 ){
			strcat( bufrdesc, "A small compartment has been built into one of the walls.\n"); }
		else if ( randomdescription == 3 ){
			strcat( bufrdesc, "A metal grate has been grafted into one of the walls.\n"); }
		else if ( randomdescription == 4 ){
			strcat( bufrdesc, "The floor is marked with deep scratches.\n");
		}
		else {
			strcat( bufrdesc, "A large ventilation pipe is in one of the walls.\n"); }

		SET_BIT( nRoom->room_flags , ROOM_NOPEDIT );
		nRoom->sector_type = SECT_SERVER;

		break;


	case 15: // Host

		// height

		roomheight = number_range(1, 3);

		if (roomheight == 1)
			strcat( bufrdesc, "The open sky is far, far above you.\n");
		else if (roomheight == 2)
			strcat( bufrdesc, "There is a ceiling just a few levels above this one.\n");
		else
			strcat( bufrdesc, "There is a ceiling many levels above this one.\n");

		// name

		strcpy( bufrname, "Host" );

		strcat( bufrdesc, "This area either has not withstood the trials of time or some catastrophe has happened here. Whichever it is, this section of WildMatrix seems abandonned.\n");

		// noise

		strcat( bufrdesc, "The wind blows noisily through here.\n");

		// random detail

		// wires, signs, circuit box, debries, rubble, spires, grill, archway, pillar, decorations, bullet holes

		randomdescription = number_range(1, 11);

		if ( randomdescription == 1 ){
			strcat( bufrdesc, "Rusted and cut wire trunks run across the floor and walls.\n"); }
		else if ( randomdescription == 2 ){
			strcat( bufrdesc, "A battered sign with faded symbols dangles from the wall.\n"); }
		else if ( randomdescription == 3 ){
			strcat( bufrdesc, "A destroyed circuit box sits in one of the walls.\n"); }
		else if ( randomdescription == 4 ){
			strcat( bufrdesc, "Lots a debrie that fell from above has gathered in this room.\n");
		}
		else if ( randomdescription == 5 ){
			strcat( bufrdesc, "Big heaps of rubble block block some parts of this room.\n");}
		else if ( randomdescription == 6 ){
			strcat( bufrdesc, "Wire frames pop out of the Host of the walls.\n"); }
		else if ( randomdescription == 7 ){
			strcat( bufrdesc, "A rusty metal grate hangs from one of the walls in this room.\n"); }
		else if ( randomdescription == 8 ){
			strcat( bufrdesc, "A big rock has caused one of the walls to collapse.\n"); }
		else if ( randomdescription == 9 ){
			strcat( bufrdesc, "Broken pillars dot the room.\n"); }
		else if ( randomdescription == 10 ){
			strcat( bufrdesc, "Scraped and broken decorations remain on some of the cracked walls.\n"); }
		else {
			strcat( bufrdesc, "The ancient remains of some creatures litter the floor.\n"); }

		SET_BIT( nRoom->room_flags , ROOM_NOPEDIT );
		nRoom->sector_type = SECT_HOST;

		break;

	case 16: // Forum

		// height

		roomheight = number_range(1, 3);

		if (roomheight == 1)
			strcat( bufrdesc, "The ceiling is so high that you can just imagine it.\n");
		else if (roomheight == 2)
			strcat( bufrdesc, "There is a ceiling just a few levels above this one.\n");
		else
			strcat( bufrdesc, "There is a ceiling many levels above this one.\n");

		// name

		strcpy( bufrname, "Forum" );

		strcat( bufrdesc, "This open area was a Forum.\n");

		// noise

		roomnoise = number_range(1, 3);

		if (roomnoise == 1){
			strcat( bufrdesc, "You can hear automatic readers readings old posts about a sort of game.\n");
		}
		else if (roomnoise == 2){
			strcat( bufrdesc, "You can hear automatic readers readings old venimous posts about womans.\n");
		}
		else {
			strcat( bufrdesc, "You can heard automatic readers readings old excited and incoherent posts.\n");
		}

		// random detail

		randomdescription = number_range(1, 5);

		if ( randomdescription == 1 ){
			strcat( bufrdesc, "You can see some dried blood on the floor.\n"); }
		else if ( randomdescription == 2 ){
			strcat( bufrdesc, "A small water hole has been dug in the middle of the room.\n");
			SET_BIT( nRoom->room_flags2 , ROOM_FOUNTAIN );
		}
		else if ( randomdescription == 3 ){
			strcat( bufrdesc, "A bannier is flashing various add.\n");
			SET_BIT( nRoom->room_flags2 , ROOM_SHOPPING ); }
		else if ( randomdescription == 4 ){
			strcat( bufrdesc, "Someone has discarded some broken equipment here.\n"); }
		else {
			strcat( bufrdesc, "Boxes and crates are piled up in one corner of this room.\n"); }

		// walls

		roomwalls = number_range(1, 4);

		if (roomwalls == 1){
			strcat( bufrdesc, "A balcony has been built into one of the walls.\n");
		}
		else if (roomwalls == 2){
			strcat( bufrdesc, "Nests of strange creatures are attached to the walls.\n");
		}
		else if (roomwalls == 3){
			strcat( bufrdesc, "It seems that admins locked this post.\n");
		}
		else {
			strcat( bufrdesc, "The walls are smooth.\n");
		}

		//SET_BIT( nRoom->room_flags , ROOM_NO_MOB );

		SET_BIT( nRoom->room_flags , ROOM_NOPEDIT );
		nRoom->sector_type = SECT_FORUM;
		break;

	case 17: // BrokenPort

		strcpy( bufrname, "BrokenPort" );

		randomdescription = number_range(1, 3);

		if ( randomdescription == 1 ){
			strcat( bufrdesc, "A improvised bench leans against one of the walls.\n"); }
		else if ( randomdescription == 2 ){
			strcat( bufrdesc, "Some decaying remains are on the grounds.\n"); }
		else if ( randomdescription == 3 ){
			strcat( bufrdesc, "A bunch of connecting spot are embedded in the wall.\nNone seems functional anymore\n"); }

		//SET_BIT( nRoom->room_flags , ROOM_NO_MOB );
		//SET_BIT( nRoom->room_flags , ROOM_SAFE );

		SET_BIT( nRoom->room_flags , ROOM_NOPEDIT );
		nRoom->sector_type = SECT_BROKENPORT;
		break;


	case 19: // Nexus

		strcpy( bufrname, "Nexus" );

		randomdescription = number_range(1, 3);

		if ( randomdescription == 1 ){
			strcat( bufrdesc, "Sark and brooding, this Nexus leads to a different part of WildMatrix.\n"); }
		else if ( randomdescription == 2 ){
			strcat( bufrdesc, "The lightly coloured material that this Nexus was made is now darkened, tainted with dried blood.\n"); }
		else if ( randomdescription == 3 ){
			strcat( bufrdesc, "Winding its way through the megastructure, this Nexus heralds the change to a new level of WildMatrix.\n"); }

		islevelchange = TRUE;

		SET_BIT( nRoom->room_flags , ROOM_NOPEDIT );
		nRoom->sector_type = SECT_NEXUS;
		break;

	case 20: // Proxy

		strcpy( bufrname, "Proxy" );

		randomdescription = number_range(1, 3);

		if ( randomdescription == 1 ){
			strcat( bufrdesc, "This Proxy was obviously overloaded when it crashed.\n"); }
		else if ( randomdescription == 2 ){
			strcat( bufrdesc, "This Proxy was apparently not used very often.\n"); }
		else if ( randomdescription == 3 ){
			strcat( bufrdesc, "This Proxy is full of wires and rubble.\n"); }

		SET_BIT( nRoom->room_flags , ROOM_NOPEDIT );
		nRoom->sector_type = SECT_PROXY;
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
			if (number_range(1, 10)  == 1 && nRoom->sector_type != SECT_404)
			{
				SET_BIT( nRoom->room_flags2 , ROOM_DEADEND );
				strcat( bufrdesc, "it seems that this site is a dead-end.\n");
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

	// stairway, Hyperlink or Nexus

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

		strcpy( bufrname, "BrokenPort" );

		randomdescription = number_range(1, 3);

		if ( randomdescription == 1 ){
			strcpy( bufrdesc, "This area seems to be safe to rest in. A bench leans against one of the walls.\n"); }
		else if ( randomdescription == 2 ){
			strcpy( bufrdesc, "Colonists have left old rags here that one can rest on.\n"); }
		else if ( randomdescription == 3 ){
			strcpy( bufrdesc, "This area must have once been a park. The plots are empty and the earth is like stone. Some benches line the walls.\n"); }

		//SET_BIT( lRoom->room_flags , ROOM_NO_MOB );
		//SET_BIT( lRoom->room_flags , ROOM_SAFE );

		lRoom->sector_type = SECT_BROKENPORT;

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

	sprintf( buf , "> a new site is uncovered at this address: %s" , dir_name[edir] );
	echo_to_room( AT_WHITE, ch->in_room, buf );

	send_to_char( "> you move to the new site.\n\r", ch );
	act(AT_GREEN, "> $n moves to a new site.", ch, NULL, NULL, TO_ROOM );

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
			followsurf(ch, "north");
		else if (edir == 1)
			followsurf(ch, "east");
		else if (edir == 2)
			followsurf(ch, "South");
		else if (edir == 3)
			followsurf(ch, "West");


	}

	do_look( ch, "auto" );

	return;

}

void followsurf ( CHAR_DATA *ch , char *argument )
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
			&& str_cmp( argument, "South" )
			&& str_cmp( argument, "West" )
			&& str_cmp( argument, "east" )
			&& str_cmp( argument, "n")
			&& str_cmp( argument, "e")
			&& str_cmp( argument, "s")
			&& str_cmp( argument, "w"))
	{
		send_to_char( "&RYou cannot explore at this address, try:\n\r&w", ch);
		send_to_char( "north, east, south, west.\n\r", ch);
		return;
	}

	if( !IS_IMMORTAL(ch) )
	{
		/*if ( IS_SET( ch->in_room->room_flags , ROOM_NOPEDIT ) )
		{
			send_to_char( "&RYou may not explore from this site!&w\n\r", ch );
			return;
		}*/

		if ( IS_SET( ch->in_room->room_flags2 , ROOM_DEADEND ) )
		{
			send_to_char( "&RThis site is a dead-end!&w\n\r", ch );
			return;
		}

	}

	if ( argument[0] == '\0' )
	{
		send_to_char( "&YSurf at which address?&w\n\r", ch );
		return;
	}

	edir = get_dir(argument);
	xit = get_exit(ch->in_room, edir);
	if ( xit )
	{
		send_to_char( "&RThere is already a site at this address!&w\n\r", ch );
		return;
	}

	chance = (int) (ch->pcdata->learned[gsn_spacecraft]);
	if ( number_percent( ) > chance )
	{
		send_to_char( "&RYou fail to surf - try again!&w\n\r", ch );
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

	case SECT_STACK:

		// Stack 20
		// 404 35
		// WebBrowser 5
		// Cookie 5
		// ChatRoom 5
		// Index.php 10
		// Host 10
		// Proxy 10

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

	case SECT_LINK:

		// Link 25
		// Wiki 15
		// Torrent 10
		// 404 5
		// Index.php 20
		// Server 15
		// Forum 5
		// Nexus 5

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

	case SECT_WIKI:

		// Link 10
		// Wiki 25
		// Torrent 5
		// 404 10
		// WebBrowser 5
		// ChatRoom 5
		// Index.php 5
		// Server 5
		// Host 5
		// Forum 15
		// BrokenPort 5
		// Proxy 5

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

	case SECT_404:
		// Stack 5
		// Link 10
		// 404 25
		// WebBrowser 5
		// Cookie 5
		// ChatRoom 5
		// Index.php 5
		// Server 5
		// Hyperlink 5
		// Host 5
		// Forum 5
		// BrokenPort 5
		// Nexus 5
		// Proxy 10

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
			roomtype = 20;

		}
		else if (newareatype >= 30) {
			roomtype = 7;

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
			roomtype = 2;
		}
		else if (newareatype >= 5) {
			roomtype = 19;
		}
		else {
			roomtype = 20;
		}

		break;

	case SECT_WEBBROWSER:
		// Stack 5
		// Wiki 10
		// 404 5
		// WebBrowser 15
		// ChatRoom 5
		// Index.php 10
		// Server 10
		// Hyperlink 5
		// Host 10
		// Forum 5
		// BrokenPort 5
		// Link 5
		// Nexus 5
		// Proxy 5

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
			roomtype = 7;
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
			roomtype = 2;
		}
		else if (newareatype >= 5) {
			roomtype = 19;
		}
		else {
			roomtype = 20;
		}

		break;

	case SECT_COOKIE:
		// 404 50
		// Cookie 50

		if (newareatype >= 50) {
			roomtype = 5;
		}
		else {
			roomtype = 8;
		}

		break;

	case SECT_CHATROOM:
		// Stack 5
		// Wiki 5
		// 404 15
		// WebBrowser 15
		// ChatRoom 20
		// Forum 10
		// BrokenPort 15
		// Nexus 10
		// Proxy 5

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

	case SECT_INDEX:
		// Stack 5
		// Link 10
		// Wiki 10
		// 404 10
		// WebBrowser 5
		// Index.php 10
		// Server 10
		// Host 10
		// Forum 10
		// BrokenPort 10
		// Link10

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
			roomtype = 2;
		}

		break;

	case SECT_SERVER:
		// Link 10
		// Wiki 5
		// 404 5
		// WebBrowser 10
		// Index.php 10
		// Server 15
		// Hyperlink 5
		// Host 10
		// Forum 5
		// BrokenPort 10
		// Nexus 5
		// Proxy 10

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
			roomtype = 7;

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



	case SECT_HOST:
		// Stack 5
		// Wiki 5
		// 404 5
		// WebBrowser 15
		// Index.php 15
		// Server 10
		// Host 15
		// Forum 5
		// BrokenPort 5
		// Link 5
		// Nexus 5
		// Proxy 5

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
			roomtype = 2;
		}
		else if (newareatype >= 5) {
			roomtype = 19;
		}
		else {
			roomtype = 20;
		}


		break;

	case SECT_FORUM:
		// Link 5
		// Wiki 5
		// 404 5
		// WebBrowser 5
		// ChatRoom 20
		// Index.php 10
		// Server 10
		// Host 5
		// Forum 20
		// BrokenPort 10
		// Proxy 5

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

	case SECT_BROKENPORT:
		// Wiki 10
		// 404 10
		// WebBrowser 10
		// ChatRoom 20
		// Index.php 10
		// Server 10
		// Host 10
		// Forum 10
		// Proxy 10

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


	case SECT_NEXUS:
		// Link 10
		// 404 5
		// WebBrowser 5
		// ChatRoom 5
		// Server 5
		// Host 5
		// Nexus 65

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
			roomtype = 2;
		}
		else {
			roomtype = 19;
		}

		break;

	case SECT_PROXY:
		// Stack 10
		// Wiki 10
		// 404 10
		// WebBrowser 10
		// ChatRoom 10
		// Server 10
		// Host 10
		// Forum 10
		// BrokenPort 10
		// Proxy 10

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
		strcat( bufrdesc, "site written in very basic html.\n");
	}
	else if (roommaterial == 2)
	{
		strcat( bufrdesc, "site written in basic html.\n");
	}
	else if (roommaterial == 3)
	{
		strcat( bufrdesc, "site written in html with some css.\n");
	}
	else if (roommaterial == 4)
	{
		strcat( bufrdesc, "site written in html and a strange derivated of php.\n");
	}
	else if (roommaterial == 5)
	{
		strcat( bufrdesc, "site written in very net a strange derivated of php.\n");
	}
	else if (roommaterial == 6)
	{
		strcat( bufrdesc, "site written in php with traces of java.\n");
	}
	else if (roommaterial == 7)
	{
		strcat( bufrdesc, "site fully written with javascript.\n");
	}
	else if (roommaterial == 8)
	{
		strcat( bufrdesc, "site made of poor java and flash animations.\n");
	}
	else if (roommaterial == 9)
	{
		strcat( bufrdesc, "site made of very nice flash layout.\n");
	}
	else
	{
		strcat( bufrdesc, "site written with a mix of xhtml and java.\n");
	}

	// rest

	switch (roomtype) {

	default:
		break;

		case 1: // Stack


		roomnoise = number_range(1, 3);

		// height

		roomheight = number_range(1, 2);

		if (roomheight == 1)
			strcat( bufrdesc, " The ceiling is low.\n");
		else
			strcat( bufrdesc, " The ceiling is high.\n");

		// name

		if ( number_range(1, 2) == 1 ){
			strcpy( bufrname, "Stack" );
			strcat( bufrdesc, "You are on top of a pyramid made of grey cubes.\n");
		}
		else {
			strcpy( bufrname, "Stack" );
			strcat( bufrdesc, "Numerous grey cubes lies on the floor.\n");
		}

		if (roomnoise == 1)
			strcat( bufrdesc, "You hear an irregular ticking noise.\n");
		else if (roomnoise == 2)
			strcat( bufrdesc, "There is a static sound in there.\n");
		else
			strcat( bufrdesc, "You hear a loud noise as another cube falls on the floor out of nowhere.\n");

		strcat( bufrdesc, " LIFO is written on the walls everywhere.\n");



		// random detail

		randomdescription = number_range(1, 5);

		if ( randomdescription == 1 ){
			strcat( bufrdesc, "Engraved on a cube you can read: Erno was here.\n"); }
		else if ( randomdescription == 2 ){
			strcat( bufrdesc, "Small bugs crawl over useless bits.\n"); }
		else if ( randomdescription == 3 ){
			strcat( bufrdesc, "One of the cube is cracked open, written on its side is: Do not open, E.S.\n"); }
		else if ( randomdescription == 4 ){
			strcat( bufrdesc, "This smells rust and dust.\n"); }
		else {
			strcat( bufrdesc, "One cube is pink and not grey.\n"); }

		// walls

		roomwalls = number_range(1, 2);

		if (roomwalls == 1){
			strcat( bufrdesc, "The walls are smooth.\n");
		}
		else {
			strcat( bufrdesc, "On one wall, you find a lever, but it is broken.\n");
		}

		SET_BIT( nRoom->room_flags , ROOM_NOPEDIT );
		nRoom->sector_type = SECT_STACK;

		break;

	case 2: // Link

        // height

        roomheight = number_range(1, 3);

        if (roomheight == 1)
            strcat( bufrdesc, "You can not see a ceiling above you.\n");
        else if (roomheight == 2)
            strcat( bufrdesc, "There is a ceiling just a few levels above this one.\n");
        else
            strcat( bufrdesc, "There is a ceiling many levels above this one.\n");

        // name

        strcpy( bufrname, "Link" );

        strcat( bufrdesc, "You are on a bridge built out of chains, some of which are broken.\n");
        // noise

        roomnoise = number_range(1, 2);

        if (roomnoise == 1){
            strcat( bufrdesc, "The only sound here is the one the chains make in the cold wind.\n");
        }
        else {
            strcat( bufrdesc, "You hear bugs flying around here.\n");
        }

        // random detail

        randomdescription = number_range(1, 7);

        if ( randomdescription == 1 ){
            strcat( bufrdesc, "You see a picture of three golden triangles on a wall.\n"); }
        else if ( randomdescription == 2 ){
            strcat( bufrdesc, "There are padlocks in some chains.\n"); }
        else if ( randomdescription == 3 ){
            strcat( bufrdesc, "One of the chain is blue, another is violet.\n"); }
        else if ( randomdescription == 4 ){
            strcat( bufrdesc, "Broken shackles are tied to the bridge.\n"); }
        else if ( randomdescription == 5 ){
            strcat( bufrdesc, "There are rust spoys everywhere.\n"); }
        else if ( randomdescription == 6 ){
            strcat( bufrdesc, "Three human hands with pointing fingers lay there.\n"); }
        else {
            strcat( bufrdesc, "On one chain is marked : dead"); }

        // walls

        roomwalls = number_range(1, 3);

        if (roomwalls == 1){
            strcat( bufrdesc, "On the bottom of a wall is written: The cake is a lie.\n");
        }
	if (roomwalls == 2){
            strcat( bufrdesc, "Almost unreadable, there's a message on the wall : \nSomething Wicked Is Coming This Way.\n");
        }
        else {
            strcat( bufrdesc, "On a wall is written: I am hidden.\n");
        }

        SET_BIT( nRoom->room_flags , ROOM_NOPEDIT );
        nRoom->sector_type = SECT_LINK;
        break;

	case 3: // Wiki

		// height

		roomheight = number_range(1, 3);

		if (roomheight == 1)
		    strcat( bufrdesc, "The ceiling is high.\n");
		else
		    strcat( bufrdesc, "If there's a ceiling, you can't see it.\n");

		strcpy( bufrname, "Wikisite" );

		strcat( bufrdesc, "There is a crumbling gargantuan octopus made of wires here.\n");

		// noise

		roomnoise = number_range(1, 2);

		if (roomnoise == 1){
		    strcat( bufrdesc, "You hear something like many books being flipped simultaneously.\n");
		}
		else {
		    strcat( bufrdesc, "It is very quiet.\n");
		}

		// random detail

		randomdescription = number_range(1, 7);

		if ( randomdescription == 1 ){
		    strcat( bufrdesc, "Burned papers are scattered all over the floor.\n"); }
		else if ( randomdescription == 2 ){
		    strcat( bufrdesc, "An worn out sign says: reference needed.\n"); }
		else if ( randomdescription == 3 ){
		    strcat( bufrdesc, "The eyes of the octopus are made of puzzle pieces.\n"); }
		else if ( randomdescription == 4 ){
		    strcat( bufrdesc, "There is a desk here, absolutly clean.\n"); }
		else if ( randomdescription == 5 ){
		    strcat( bufrdesc, "Two barrel of paint are here: one blue and one red.\n"); }
		else if ( randomdescription == 6 ){
		    strcat( bufrdesc, "Banana peals and peanuts are scattered around a pile of anthropoids bones.\n"); }
		else {
		    strcat( bufrdesc, "A piece of paper is pinned on a tentacle, you can read: I am Edith.\n"); }

		// walls

		roomwalls = number_range(1, 2);

		if (roomwalls == 1){
		    strcat( bufrdesc, "On a wall is a big board saying: DELETE EVERYTHING.\n");
		}
		else {
		    strcat( bufrdesc, "Old surveillance cameras are on the walls.\n");
		    SET_BIT( nRoom->room_flags2 , ROOM_FOUNTAIN );
		}

		//SET_BIT( nRoom->room_flags , ROOM_NO_MOB );

		SET_BIT( nRoom->room_flags , ROOM_NOPEDIT );
		nRoom->sector_type = SECT_WIKI;
		break;

	case 4: // Torrent

		strcpy( bufrname, "Torrent" );

		randomdescription = number_range(1, 3);

		if ( randomdescription == 1 ){
			strcpy( bufrdesc, "A seemingly endless Torrent runs until out-of-sight.\n"); }
		else if ( randomdescription == 2 ){
			strcpy( bufrdesc, "A seemingly endless and wide Torrent that stretches out into all directions around you.\n"); }
		else if ( randomdescription == 3 ){
			strcpy( bufrdesc, "You see a seemingly endless Torrent that is filled with some kind of dark mist.\n"); }

		//SET_BIT( nRoom->room_flags , ROOM_NO_MOB );
		SET_BIT( nRoom->room_flags , ROOM_NOPEDIT );

		nRoom->sector_type = SECT_TORRENT;
		break;

	case 5: // 404

		// height

		roomheight = number_range(1, 3);

		if (roomheight == 1)
			strcat( bufrdesc, "The ceiling is so high that you can just imagine it.\n");
		else if (roomheight == 2)
			strcat( bufrdesc, "There is a ceiling just a few levels above this one.\n");
		else
			strcat( bufrdesc, "There is a ceiling many levels above this one.\n");

		// name

		strcpy( bufrname, "404" );

		strcat( bufrdesc, "You are in a 404 site of WildMatrix. It is desert.\n");

		// noise

		roomnoise = number_range(1, 3);

		if (roomnoise == 1){
			strcat( bufrdesc, "The only sound you can hear is your footstep.\n");
		}
		else if (roomnoise == 2){
			strcat( bufrdesc, "You hear the sound of the CyberSpace wind, blowing through this empty site.\n");
		}
		else {
			strcat( bufrdesc, "The whole structure is scratching and grinding.\n");
		}

		// walls

		roomwalls = number_range(1, 10);

		if (roomwalls <= 3){
			strcat( bufrdesc, "Walls are made of rusty metal plates, spotted with strange scratches.\n");
		}
		else if (roomwalls <= 6){
			strcat( bufrdesc, "All data flows seem to end here and never go out.\n");
		}
		else if (roomwalls == 7){
			strcat( bufrdesc, "You would say that something uncivilized happened here.\n");
		}
		else {
			strcat( bufrdesc, "Walls are made of smooth metal plates, \nspeckled with rust and maladjusted.\n");
		}

		// random detail

		randomdescription = number_range(1, 7);

		if ( randomdescription <= 6 ){
			strcat( bufrdesc, "\"404 error - PAGE NOT FOUND\" is written everywhere here"); }
		else {
			strcat( bufrdesc, "For some strange reason, somebody painted a \"200 OK\" message on the wall.\n"); }



		//SET_BIT( nRoom->room_flags , ROOM_NO_MOB );
		SET_BIT( nRoom->room_flags , ROOM_NOPEDIT );
		nRoom->sector_type = SECT_404;
		break;

	case 6: // WebBrowser

		// height

		roomheight = number_range(1, 3);

		if (roomheight == 1)
			strcat( bufrdesc, "The ceiling is low.\n");
		else if (roomheight == 2)
			strcat( bufrdesc, "The ceiling is high.\n");
		else
			strcat( bufrdesc, "The ceiling is massively high.\n");

		// name

		strcpy( bufrname, "WebBrowser" );

		strcat( bufrdesc, "One of the thousands of WebBrowsers that allow you to surf between sites of WildMatrix.\n");

		// noise

		roomnoise = number_range(1, 3);

		if (roomnoise == 1){
			strcat( bufrdesc, "You think you hear some footsteps in the distance.\n");
		}
		else if (roomnoise == 2){
			strcat( bufrdesc, "You can hear water drip from the ceiling.\n");
			SET_BIT( nRoom->room_flags2 , ROOM_FOUNTAIN );
		}
		else {
			strcat( bufrdesc, "The whistle of the CyberSpace wind fills this empty room.\n");
		}

		// random detail

		randomdescription = number_range(1, 10);

		if ( randomdescription == 1 ){
			strcat( bufrdesc, "There is a metal grate in one of the walls.\n"); }
		else if ( randomdescription == 2 ){
			strcat( bufrdesc, "A large fan rotor has been grafted into one of the walls.\n"); }
		else if ( randomdescription == 3 ){
			strcat( bufrdesc, "Thin and thick trunks of wires run across the walls and floor.\n"); }
		else if ( randomdescription == 4 ){
			strcat( bufrdesc, "Big and small pipes run across the middle of the walls.\n");
		}
		else if ( randomdescription == 5 ){
			strcat( bufrdesc, "A rusty, broken ladder leans against one of the walls.\n");}
		else if ( randomdescription == 6 ){
			strcat( bufrdesc, "A large crossbeam dominates this room.\n"); }
		else if ( randomdescription == 7 ){
			strcat( bufrdesc, "The walls are riddled with bullet holes.\n"); }
		else if ( randomdescription == 8 ){
			strcat( bufrdesc, "A circuit box sits on one of the walls here.\n"); }
		else if ( randomdescription == 9 ){
			strcat( bufrdesc, "A lot of rubble is strewn across the floor.\n"); }
		else {
			strcat( bufrdesc, "Small, square lights are embedded in the walls of the room.\n"); }

		SET_BIT( nRoom->room_flags , ROOM_NOPEDIT );
		nRoom->sector_type = SECT_WEBBROWSER;
		break;

	case 7: // Hyperlink

		strcpy( bufrname, "Hyperlink" );

		strcpy( bufrdesc, "You can follow this Hyperlink until the next level.\n");

		islevelchange = TRUE;
		SET_BIT( nRoom->room_flags , ROOM_NOPEDIT );

		nRoom->sector_type = SECT_HYPERLINK;

		break;

	case 8: // Cookie

		// height

		roomheight = number_range(1, 3);

		if (roomheight == 1)
			strcat( bufrdesc, "The ceiling is low.\n");
		else if (roomheight == 2)
			strcat( bufrdesc, "The ceiling is high.\n");
		else
			strcat( bufrdesc, "The ceiling is massively high.\n");

		// name

		strcpy( bufrname, "Cookie" );

		strcat( bufrdesc, "This is an ancient Cookie.\n");

		// noise

		roomnoise = number_range(1, 3);

		if (roomnoise == 1){
			strcat( bufrdesc, "You can hear a grinding noise.\n");
		}
		else if (roomnoise == 2){
			strcat( bufrdesc, "A constant whirring noise sounds through the room.\n");
		}
		else {
			strcat( bufrdesc, "You can hear the sound of metal rubbing against metal.\n");
		}

		// random detail

		randomdescription = number_range(1, 10);

		if ( randomdescription <= 2 ){
			strcat( bufrdesc, "The cookie is full of lines and lines of encrypted connexions.\n"); }
		else if ( randomdescription <= 4 ){
			strcat( bufrdesc, "You would say that there's at least an hundred of identifiers contained here.\n"); }
		else if ( randomdescription == 3 ){
			strcat( bufrdesc, "Wait ... you can read this code ... but it wasn't a real cookie, it was a spyware ! "); }
		else {
			strcat( bufrdesc, "Walls are covered with lines of code almost unreadable, rub out by the time.\n"); }

		SET_BIT( nRoom->room_flags , ROOM_NOPEDIT );
		nRoom->sector_type = SECT_COOKIE;

		break;

	case 9: // ChatRoom

		roomnoise = number_range(1, 3);

		// height

		roomheight = number_range(1, 2);

		if (roomheight == 1)
			strcat( bufrdesc, "The ceiling is low.\n");
		else
			strcat( bufrdesc, "The ceiling is high.\n");

		// name

		if ( number_range(1, 2) == 1 ){
			strcpy( bufrname, "ChatRoom" );
			strcat( bufrdesc, "Obviously, this place was a very popular ChatRoom.\n");

		}
		else {
			strcpy( bufrname, "ChatRoom" );
			strcat( bufrdesc, "You cannot imagine what kind of people would use this ChatRoom.\n");

		}

		// noise

		if (roomnoise == 1)
			strcat( bufrdesc, "You can still hear old echos of conversations which stand here long time ago.\n");
		else if (roomnoise == 2){
			strcat( bufrdesc, "Chattering shut up a long long time ago");
			SET_BIT( nRoom->room_flags2 , ROOM_FOUNTAIN );
		}
		else
			strcat( bufrdesc, "It is very quiet. For the moment.\n");

		// walls

		roomwalls = number_range(1, 2);

		if (roomwalls == 1){
			strcat( bufrdesc, "The walls are covered with billions of old conversation lines.\n");
		}
		else {
			strcat( bufrdesc, "The walls are covered with very odds caracters that you cannot read.\n");
		}

		// random detail

		randomdescription = number_range(1, 10);

		if ( randomdescription == 1 ){
			strcat( bufrdesc, "On a wall is written : \"WELCOME NEWFAG !\" "); }
		else if ( randomdescription == 2 ){
			strcat( bufrdesc, "Scribbled on the bottom of one of the walls, you can read \"I was here\""); }
		else if ( randomdescription == 3 ){
			strcat( bufrdesc, "Written is almost unreadable font, you think decipher the sentence \"He comes\".\n"); }
		else if ( randomdescription == 4 ){
			strcat( bufrdesc, "You realize that, apparently, all the users of this chat was nammed John or Dave.\n"); }
		else {
			strcat( bufrdesc, "Some barrels have been left in this room.\n"); }


		SET_BIT( nRoom->room_flags2 , ROOM_FOOD );
		SET_BIT( nRoom->room_flags , ROOM_NOPEDIT );
		nRoom->sector_type = SECT_CHATROOM;

		break;

	case 10: // Index.php

		// height

		roomheight = number_range(1, 3);

		if (roomheight == 1)
			strcat( bufrdesc, "The ceiling is high.\n");
		else if (roomheight == 2)
			strcat( bufrdesc, "The ceiling is very high.\n");
		else
			strcat( bufrdesc, "The ceiling is massively high.\n");

		// name

		strcpy( bufrname, "Index.php" );

		strcat( bufrdesc, "An Index.php site, a kind of site that invaded \nthe place during its golden years.\n");

		// noise
			strcat( bufrdesc, "The ceiling is massively high.\n");

		// random detail

		// wires, signs, circuit box, debries, rubble, spires, grill, archway, pillar, decorations, bullet holes

		randomdescription = number_range(1, 11);

		if ( randomdescription == 1 ){
			strcat( bufrdesc, "Thick and thin wire trunks run across the floor and walls.\n"); }
		else if ( randomdescription == 2 ){
			strcat( bufrdesc, "A sign with strange writings has been attached to one of the walls.\n"); }
		else if ( randomdescription == 3 ){
			strcat( bufrdesc, "A circuit box sits in one of the walls.\n"); }
		else if ( randomdescription == 4 ){
			strcat( bufrdesc, "lots a debry that fell from above has gathered in this room.\n");
		}
		else if ( randomdescription == 5 ){
			strcat( bufrdesc, "Big heaps of rubble block block some parts of this room.\n");}
		else if ( randomdescription == 6 ){
			strcat( bufrdesc, "Strange spires grow out of the ground and build macabre archways.\n"); }
		else if ( randomdescription == 7 ){
			strcat( bufrdesc, "A rusty metal grate has been grafted into one of the walls.\n"); }
		else if ( randomdescription == 8 ){
			strcat( bufrdesc, "A beautiful archway decorates this Index.php.\n"); }
		else if ( randomdescription == 9 ){
			strcat( bufrdesc, "large, round pillars rise into the sky in this room.\n"); }
		else if ( randomdescription == 10 ){
			strcat( bufrdesc, "intricate decorations have been added to some of the walls.\n"); }
		else {
			strcat( bufrdesc, "The walls are full of bullet holes.\n"); }

		SET_BIT( nRoom->room_flags , ROOM_NOPEDIT );
		nRoom->sector_type = SECT_INDEX;

		break;

	case 12: // Server

		// height

		roomheight = number_range(1, 3);

		if (roomheight == 1)
			strcat( bufrdesc, "The open sky is far, far above you.\n");
		else if (roomheight == 2)
			strcat( bufrdesc, "There is a ceiling just a few levels above this one.\n");
		else
			strcat( bufrdesc, "There is a ceiling many levels above this one.\n");

		// name

		strcpy( bufrname, "Server" );

		strcat( bufrdesc, "This room was a Server, a wide and accessible storage for everything and everybody.\n");

		// noise

		// noise

		roomnoise = number_range(1, 3);

		if (roomnoise == 1){
			strcat( bufrdesc, "A pool of slime is blubbering in the center of the site.\n");
		}
		else {
			strcat( bufrdesc, "You can hear a weird buzzing noise but cannot localize the source.\n");
		}

		// random detail

		// wires, compartment, metal grate, ventilation pipe, remains

		randomdescription = number_range(1, 5);

		if ( randomdescription == 1 ){
			strcat( bufrdesc, "Thick and thin wire trunks run across the floor and walls.\n"); }
		else if ( randomdescription == 2 ){
			strcat( bufrdesc, "A small compartment has been built into one of the walls.\n"); }
		else if ( randomdescription == 3 ){
			strcat( bufrdesc, "A metal grate has been grafted into one of the walls.\n"); }
		else if ( randomdescription == 4 ){
			strcat( bufrdesc, "The floor is marked with deep scratches.\n");
		}
		else {
			strcat( bufrdesc, "A large ventilation pipe is in one of the walls.\n"); }

		SET_BIT( nRoom->room_flags , ROOM_NOPEDIT );
		nRoom->sector_type = SECT_SERVER;

		break;


	case 15: // Host

		// height

		roomheight = number_range(1, 3);

		if (roomheight == 1)
			strcat( bufrdesc, "The open sky is far, far above you.\n");
		else if (roomheight == 2)
			strcat( bufrdesc, "There is a ceiling just a few levels above this one.\n");
		else
			strcat( bufrdesc, "There is a ceiling many levels above this one.\n");

		// name

		strcpy( bufrname, "Host" );

		strcat( bufrdesc, "This area either has not withstood the trials of time or some catastrophe has happened here. Whichever it is, this section of WildMatrix seems abandonned.\n");

		// noise

		strcat( bufrdesc, "The wind blows noisily through here.\n");

		// random detail

		// wires, signs, circuit box, debries, rubble, spires, grill, archway, pillar, decorations, bullet holes

		randomdescription = number_range(1, 11);

		if ( randomdescription == 1 ){
			strcat( bufrdesc, "Rusted and cut wire trunks run across the floor and walls.\n"); }
		else if ( randomdescription == 2 ){
			strcat( bufrdesc, "A battered sign with faded symbols dangles from the wall.\n"); }
		else if ( randomdescription == 3 ){
			strcat( bufrdesc, "A destroyed circuit box sits in one of the walls.\n"); }
		else if ( randomdescription == 4 ){
			strcat( bufrdesc, "lots a debrie that fell from above has gathered in this room.\n");
		}
		else if ( randomdescription == 5 ){
			strcat( bufrdesc, "Big heaps of rubble block block some parts of this room.\n");}
		else if ( randomdescription == 6 ){
			strcat( bufrdesc, "Wire frames pop out of the Host of the walls.\n"); }
		else if ( randomdescription == 7 ){
			strcat( bufrdesc, "A rusty metal grate hangs from one of the walls in this room.\n"); }
		else if ( randomdescription == 8 ){
			strcat( bufrdesc, "A big rock has caused one of the walls to collapse.\n"); }
		else if ( randomdescription == 9 ){
			strcat( bufrdesc, "Broken pillars dot the room.\n"); }
		else if ( randomdescription == 10 ){
			strcat( bufrdesc, "Scraped and broken decorations remain on some of the cracked walls.\n"); }
		else {
			strcat( bufrdesc, "The ancient remains of some creatures litter the floor.\n"); }

		SET_BIT( nRoom->room_flags , ROOM_NOPEDIT );
		nRoom->sector_type = SECT_HOST;

		break;

	case 16: // Forum

		// height

		roomheight = number_range(1, 3);

		if (roomheight == 1)
			strcat( bufrdesc, "The ceiling is so high that you can just imagine it.\n");
		else if (roomheight == 2)
			strcat( bufrdesc, "There is a ceiling just a few levels above this one.\n");
		else
			strcat( bufrdesc, "There is a ceiling many levels above this one.\n");

		// name

		strcpy( bufrname, "Forum" );

		strcat( bufrdesc, "This open area was a Forum.\n");

		// noise

		roomnoise = number_range(1, 3);

		if (roomnoise == 1){
			strcat( bufrdesc, "You can hear automatic readers readings old posts about a sort of game.\n");
		}
		else if (roomnoise == 2){
			strcat( bufrdesc, "You can hear automatic readers readings old venimous posts about womans.\n");
		}
		else {
			strcat( bufrdesc, "You can heard automatic readers readings old excited and incoherent posts.\n");
		}

		// random detail

		randomdescription = number_range(1, 5);

		if ( randomdescription == 1 ){
			strcat( bufrdesc, "You can see some dried blood on the floor.\n"); }
		else if ( randomdescription == 2 ){
			strcat( bufrdesc, "A small water hole has been dug in the middle of the room.\n");
			SET_BIT( nRoom->room_flags2 , ROOM_FOUNTAIN );
		}
		else if ( randomdescription == 3 ){
			strcat( bufrdesc, "A bannier is flashing various add.\n");
			SET_BIT( nRoom->room_flags2 , ROOM_SHOPPING ); }
		else if ( randomdescription == 4 ){
			strcat( bufrdesc, "Someone has discarded some broken equipment here.\n"); }
		else {
			strcat( bufrdesc, "Boxes and crates are piled up in one corner of this room.\n"); }

		// walls

		roomwalls = number_range(1, 4);

		if (roomwalls == 1){
			strcat( bufrdesc, "A balcony has been built into one of the walls.\n");
		}
		else if (roomwalls == 2){
			strcat( bufrdesc, "Nests of strange creatures are attached to the walls.\n");
		}
		else if (roomwalls == 3){
			strcat( bufrdesc, "It seems that admins locked this post.\n");
		}
		else {
			strcat( bufrdesc, "The walls are smooth.\n");
		}

		//SET_BIT( nRoom->room_flags , ROOM_NO_MOB );

		SET_BIT( nRoom->room_flags , ROOM_NOPEDIT );
		nRoom->sector_type = SECT_FORUM;
		break;

	case 17: // BrokenPort

		strcpy( bufrname, "BrokenPort" );

		randomdescription = number_range(1, 3);

		if ( randomdescription == 1 ){
			strcat( bufrdesc, "A improvised bench leans against one of the walls.\n"); }
		else if ( randomdescription == 2 ){
			strcat( bufrdesc, "Some decaying remains are on the grounds.\n"); }
		else if ( randomdescription == 3 ){
			strcat( bufrdesc, "A bunch of connecting spot are embedded in the wall.\nNone seems functional anymore\n"); }

		//SET_BIT( nRoom->room_flags , ROOM_NO_MOB );
		//SET_BIT( nRoom->room_flags , ROOM_SAFE );

		SET_BIT( nRoom->room_flags , ROOM_NOPEDIT );
		nRoom->sector_type = SECT_BROKENPORT;
		break;


	case 19: // Nexus

		strcpy( bufrname, "Nexus" );

		randomdescription = number_range(1, 3);

		if ( randomdescription == 1 ){
			strcat( bufrdesc, "Sark and brooding, this Nexus leads to a different part of WildMatrix.\n"); }
		else if ( randomdescription == 2 ){
			strcat( bufrdesc, "The lightly coloured material that this Nexus was made is now darkened, tainted with dried blood.\n"); }
		else if ( randomdescription == 3 ){
			strcat( bufrdesc, "Winding its way through the megastructure, this Nexus heralds the change to a new level of WildMatrix.\n"); }

		islevelchange = TRUE;

		SET_BIT( nRoom->room_flags , ROOM_NOPEDIT );
		nRoom->sector_type = SECT_NEXUS;
		break;

	case 20: // Proxy

		strcpy( bufrname, "Proxy" );

		randomdescription = number_range(1, 3);

		if ( randomdescription == 1 ){
			strcat( bufrdesc, "This Proxy was obviously overloaded when it crashed.\n"); }
		else if ( randomdescription == 2 ){
			strcat( bufrdesc, "This Proxy was apparently not used very often.\n"); }
		else if ( randomdescription == 3 ){
			strcat( bufrdesc, "This Proxy is full of wires and rubble.\n"); }

		SET_BIT( nRoom->room_flags , ROOM_NOPEDIT );
		nRoom->sector_type = SECT_PROXY;
		break;

	}

	// 1 out of 10 rooms are dead-ends

	if (!islevelchange)
	{
		if (number_range(1, 10)  == 1 && nRoom->sector_type != SECT_404)
		{
			SET_BIT( nRoom->room_flags2 , ROOM_DEADEND );
			strcat( bufrdesc, "It seems that this site is a dead-end.");
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

	// stairway, Hyperlink or Nexus

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

		strcpy( bufrname, "BrokenPort" );

		randomdescription = number_range(1, 3);

		if ( randomdescription == 1 ){
			strcat( bufrdesc, "A improvised bench leans against one of the walls.\n"); }
		else if ( randomdescription == 2 ){
			strcat( bufrdesc, "Some decaying remains are on the grounds.\n"); }
		else if ( randomdescription == 3 ){
			strcat( bufrdesc, "A bunch of connecting spot are embedded in the wall.\nNone seems functional anymore\n"); }

		//SET_BIT( lRoom->room_flags , ROOM_NO_MOB );
		//SET_BIT( lRoom->room_flags , ROOM_SAFE );

		lRoom->sector_type = SECT_BROKENPORT;

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

