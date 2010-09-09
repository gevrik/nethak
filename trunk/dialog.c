/*
 * Dialog.c
 *
 *  Created on: september 9th 2010
 *      Author: aiseant
 */
 
 #include <sys/types.h>
#include <sys/time.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include "mud.h"


void dialog(CHAR_DATA* ch)
{
	
	if(ch->pIndexData->vnum==42)
	{
	
		switch(ch->in_room->vnum)
		{
			case 31911 : //newtuto connexion
			do_say( ch,"Welcome human.\nYou're now connected in Newtutorial,\na system dedicated to help people starting.\nYou're free to leave as soon as you want,\nbut you'll certainly need to know everything\nwe can teach you here. And it's free.");
			//(social) Byte chortles.
			do_say( ch,"If you want to leave, you can use the\ncommand SYSTEM (or SYS) and see\n-almost- the whole CyberSpace : it displays\nthe list of systems you can connect, the\nregion of the CyberSpace where they are,\nhe owner of the system and its CPU load.\nAt the moment, you only needs to know\nthe name of the system you want, and type\nCONNECT (or CONN) followed by the choosen\none. This will display the list of accessible\nlobbies, with their dedicated node number.\nYou generally would connect IO lobby.\nTo proceed, type CONNECT name_system number_lobby.");
			do_say( ch,"You will learn everything you need to know\nabout owners of system and CPU load if you\nchoose to follow the tutorial. To do that, please\nfollow the path and go north (NORTH, or N).");
			do_say( ch,"Have a good uptime in the CyberSpace, human.");
			//(social) Byte goodbyes
			break;
			
			case 32095 :	//newtuto fw node
			do_say( ch,"Welcome human.\nI'm glad you choose to follow our teaching.\nAs you can see, here are few ICE\n(Intrusion Countermeasures Electronics).\nThose entities are the police of the system.\nWe strongly advice you not to mess with them,\nthey generally don't appreciate spoilsport.\nThey also hunt unallowed people intruding\nthe system (and the sentence is often the flatline).\nWe are in a firewall node, which explains why\nthere are ICE here. You can see it in the name\nof the node, but also by using ANALYZE (or ANA)\ncommand. It's a skill, but you normally already know it.");
			do_say( ch,"The first line is the level of the node. There are 6\nlevels of node : blue, green, orange, red, ultra-violet\nand black. Blue node spawns fewer and weaker\nitems or entities than black, but upgrading level\nof node costs a lot of credits, you'll learn that with\nmy brother, Byte, and construct lessons.");
			do_say( ch,"Index isn't important at the moment,\nSystem is the system where the node was build\nand Owner the person who did it.");
			do_say( ch,"Sector is the type of the node. Here it's firewall (or fw),\nand fw nodes spawn ICE entities (and no item).\nIn systems, you can also find :\nDatabase node, spawning class and var files, but also naughty virii\nTerminal node, spawning User, Trader and Program entities (and no item)\nSubserver node, spawning resource files (and no entity)\nand Supply node, spawning Trader entity (and no item).\nBy the way, you can go back west and see a supply node,\nwith more informations.");
			do_say( ch,"Have a good uptime in CyberSpace, human.");
			//(social)Byte goodbyes
			break;
			
			case 31942 : //newtuto path1
			do_say( ch,"Welcome human.\nI'm glad you choose to follow our teaching.\nYou can do a lot of things in the CyberSpace,\nwe will see that now.");
			do_say( ch,"o have the list of possible actions, please\ntype COMMAND.  You can type any command\nby itself for instructions about needed argument.\nYou can already use some of those commands,\nbut sometimes you'll need to learn or buy \nspecific skill.\nType SKILL to see which skill you already have.\nGenerally, if you try a command and have \"invalid\ninput\" as result, it means that you need the\ncorresponding skill.");
			do_say( ch,"You'll mostly learn the use of the commands\nby using it, or continuing the tutorial.\nYou also can try HELP Name_Command to get\nmore informations about it, but our General\nKnowledge Database recently crashed and some\nfiles are still missing. We apologize about that.");
			do_say( ch,"The most important command for you right now\nis LOOK. My brother, Byte, should have teaching\nyou this one. It displays the name of the node you\nare and its description, the connections (or exits),\nthe objects lying on the floor and other entities\nwho are present.\nYou see that you can go north to keep following\nthe path, but also go EAST (or e) to learn more\nabout the different node types or go WEST (or w)\nto go in suply node.");
			do_say( ch,"I advice you to start with Nodes and you'll visit Supply right after. ");
			do_say( ch,"Have a good uptime in CyberSpace, human. ");
			//(social)Byte goodbyes
			break;
			
			case 32096 ://newtuto supply 1
			do_say( ch,"Welcome human. \nI'm glad you choose to follow our teaching.\nYou're in a supply node, one of the four \nbasics with database, terminal and subserver.\nThose kind of nodes are populated with \nTrader Program. \nThey usually sell a lot of \nvery usefull items and are a very easy way \nto find necessary stuff.");
			do_say( ch,"To see what they can sell you, use LIST.\nYou can use the Ref index to buy the item : \nBuy #1 will buy the first item on the list \nBuy 10 #1 will buy ten of them at once\nWhen you need something you cannot\nfind in systems, think at Supply Traders !\nYou'll find patch, modules, and various \nresource files, depending of the trader and \nthe level of the node.  ");
			do_say( ch,"By the way, you should explore a little more \ntoward west, you'll see that Tutors constructed \nyou few Supply node and upgraded it, in order\nto show you all the colors. \nAnd my brothers will teach you more about \nstuff you can find for sale.");
			do_say( ch,"Have a good uptime in CyberSpace, human.");
			//(social)Byte goodbyes
			break;
			
			case 32104 ://newtuto supply 2
			do_say( ch,"Welcome human. \nI'm glad you choose to follow our teaching.\nLet's talk a little about modules you can buy ... ");
			do_say( ch,"Compilers are mostly for Codeapp and Decking\nAlso needed for a number of other codeskills\nDevkits are for slicing skills and codemed\nParsers are used with most codeskills ");
			do_say( ch,"Comlinks are required for some chat channels\nComlinks only work when held or in memory\nDataminers help when digging up items\nItems are only found after they are buried ");
			do_say( ch,"The two last types of module, Blade and Blaster,\nwill be explained in detail in the room at west. ");
			do_say( ch,"Have a good uptime in CyberSpace, human.");
			//(social)Byte goodbyes
			break;
			
			case 32105 ://newtuto supply 3
			do_say( ch,"Welcome human. \nI\'m glad you choose to follow our teaching.\n Let\'s talk a little about weapon modules !");
			do_say( ch,"Blaster and Blade modules are basic weapons\nTheir damage is low, but their ammo is high\nTo use a weapon, you must HOLD it (HOLD weapon_name).\nBasically, you can only hold one thing at once, but soon you'll \nlearn how to be able to hold two.");
			do_say( ch,"You can see what you're holding (and wearing) by typing \nEQUIPMENT (or EQ). Use REMOVE equipment_name to \nget your hand free (the equipment will return in your inventory).\nIf you use EQ (or EXAMINE weapon_name) , you'll also see \n how many ammo you've on your weapon.");
			do_say( ch,"Blaster patches are used to reload blasters and utility patches\nto reload blades.\nA Blaster shot takes various amount of ammo, depending on \nthe level it's set. Use SETBLASTER to change the power of \nyour blaster.\nBlade consumes its charge when you're holding it, either if \nyou use it or not.\nTo reload a weapon, hold it, make sure you've the right type \nof patch in your inventory and use PATCH, or AMMO command.");
			do_say( ch,"For the last items, please go west.\nHave a good uptime in CyberSpace, human.");
			//(social)Byte goodbyes
			break;
						
			case 32106 ://newtuto supply 4
			do_say( ch,"Welcome human. \nI'm glad you choose to follow our teaching.\nThis is the last step about Supply Nodes \nand what you can find in there.");
			do_say( ch,"Classes (function, utility, blade, blaster, def) \nare materials required for the codeskills.\nMost of them can be found at a supply node \nbut Def classes are sold by traders in terminal nodes.\r");
			do_say( ch,"Wilderspace subroutines are strange things : \nthey are for making medmods to heal people\nIts all you need except a devkit and codemed.");
			do_say( ch,"Please go back (hit east few time) to the Path to follow your quest.\nHave a good uptime in CyberSpace, human.");
			//(social)Byte goodbyes
			break;

			
			default :  do_say( ch,"urm");
				break;
			
		}
	
	}
}
