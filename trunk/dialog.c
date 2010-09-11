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
	CHAR_DATA *locut;

	for ( locut = ch->in_room->first_person; locut; locut = locut->next_in_room )
	{
		if (locut->pIndexData->vnum==42)
			break;
	}
	
	if(!locut)
	{
		//pb : Byte missing
	}
	else
	{
		switch(ch->in_room->vnum)
		{
			case 31911 : //newtuto connexion
			if( (ch->lesson&BV00) !=BV00)
			{
				//(social) Byte chortles.
				send_to_char( ">The Byte flies in front of you and says :\n\r", ch);
				WAIT_STATE(ch, 2);
				send_to_char( "&BByte : Welcome human.\nYou're now connected in Newtutorial,\na system dedicated to help people starting.\nYou're free to leave as soon as you want,\nbut you'll certainly need to know everything\nwe can teach you here. And it's free.\n\n\r", ch);
				WAIT_STATE(ch, 2);
				send_to_char( "&BByte : If you want to leave, you can use the\ncommand SYSTEM (or SYS) and see\n-almost- the whole CyberSpace : it displays\nthe list of systems you can connect, the\nregion of the CyberSpace where they are,\nhe owner of the system and its CPU load.\n\n\r", ch);
				send_to_char( "&BByte : At the moment, you only needs to know\nthe name of the system you want, and type\nCONNECT (or CONN) followed by the choosen\none. This will display the list of accessible\nlobbies, with their dedicated node number.\nYou generally would connect IO lobby.\nTo proceed, type CONNECT name_system number_lobby.\n\n\r", ch);
				send_to_char( "&BByte : You will learn everything you need to know\nabout owners of system and CPU load if you\nchoose to follow the tutorial. To do that, please\nfollow the path and go north (NORTH, or N).\n\n\r", ch);
				send_to_char( "&BByte : Have a good uptime in the CyberSpace, human.\n\n\r", ch);
				//(social) Byte goodbyes
				
				ch->lesson= ch->lesson|BV00;
			}
			break;

			case 32095 :	//newtuto fw node
			if( (ch->lesson&BV01) !=BV01)
			{
				send_to_char( ">The Byte flies in front of you and says :\n\r", ch);
				send_to_char( "&BByte : Welcome human.\nI'm glad you choose to follow our teaching.\nAs you can see, here are few ICE\n(Intrusion Countermeasures Electronics).\nThose entities are the police of the system.\nWe strongly advice you not to mess with them,\nthey generally don't appreciate spoilsport.\nThey also hunt unallowed people intruding\nthe system (and the sentence is often the flatline).\nWe are in a firewall node, which explains why\nthere are ICE here. You can see it in the name\nof the node, but also by using ANALYZE (or ANA)\ncommand. It's a skill, but you normally already know it.\n\n\r", ch);
				WAIT_STATE(ch, 2);
				send_to_char( "&BByte : The first line is the level of the node. \nBasically, blue is a low level node.\nIndex isn't important at the moment,\nSystem is the system where the node was build\nand Owner the person who did it.\n\n\r", ch);
				send_to_char( "&BByte : Sector is the type of the node. \nHere it's firewall (or fw),\nand fw nodes spawn ICE entities (and no item).\nIn systems, you can also find :\nDatabase node, spawning class and var files, \nbut also naughty virii\nTerminal node, spawning User, Trader and \nProgram entities (but no item)\nSubserver node, spawning resource files (and no entity)\nand Supply node, spawning Trader entity (but no item).\nBy the way, you can go back west and see a supply node,\nwith more informations.\n\n\r", ch);
				send_to_char( "&BByte : Have a good uptime in CyberSpace, human.\n\n\r", ch);
				//(social)Byte goodbyes
				ch->lesson= ch->lesson|BV01;
			}
			break;

			case 31942 : //newtuto path1
			if( (ch->lesson&BV02) !=BV02)
			{
				send_to_char( ">The Byte flies in front of you and says :\n\r", ch);
				send_to_char( "&BByte : Welcome human.\nI'm glad you choose to follow our teaching.\nYou can do a lot of things in the CyberSpace,\nwe will see that now.\n\n\r", ch);
				WAIT_STATE(ch, 2);
				send_to_char( "&BByte : To have the list of possible actions, please\ntype COMMAND.  You can type any command\nby itself for instructions about needed argument.\nYou can already use some of those commands,\nbut sometimes you'll need to learn or buy \nspecific skill.\nGenerally, if you try a command and have \"invalid\ninput\" as result, it means that you need the\ncorresponding skill.\n\n\r", ch);
				send_to_char( "&BByte : You'll mostly learn the use of the commands\nby using it, or continuing the tutorial.\nYou also can try HELP Name_Command to get\nmore informations about it, but our General\nKnowledge Database recently crashed and some\nfiles are still missing. We apologize about that.\n\n\r", ch);
				send_to_char( "&BByte : The most important command for you right now\nis LOOK. My brother, Byte, should have teaching\nyou this one. It displays the name of the node you\nare and its description, the connections (or exits),\nthe objects lying on the floor and other entities\nwho are present.\nYou see that you can go north to keep following\nthe path, but also go EAST (or e) to learn more\nabout the different node types or go WEST (or w)\nto go in suply node.\n\n\r", ch);
				send_to_char( "&BByte : I advice you to start with Nodes and \nyou'll visit Supply right after. \n\n\r", ch);
				send_to_char( "&BByte : Have a good uptime in CyberSpace, human. \n\n\r", ch);
				//(social)Byte goodbyes
				ch->lesson= ch->lesson|BV02;
			}
			break;

			case 32096 ://newtuto supply 1
			if( (ch->lesson&BV03) !=BV03)
			{
				send_to_char( ">The Byte flies in front of you and says :\n\r", ch);
				send_to_char( "&BByte : Welcome human. \nI'm glad you choose to follow our teaching.\nYou're in a supply node, one of the four \nbasics with database, terminal and subserver.\nThose kind of nodes are populated with \nTrader Program. \nThey usually sell a lot of \nvery usefull items and are a very easy way \nto find necessary stuff.\n\n\r", ch);
				WAIT_STATE(ch, 2);
				send_to_char( "&BByte : To see what they can sell you, use LIST.\nYou can use the Ref index to buy the item : \nBuy #1 will buy the first item on the list \nBuy 10 #1 will buy ten of them at once\nWhen you need something you cannot\nfind in systems, think at Supply Traders !\nYou'll find patch, modules, and various \nresource files, depending of the trader and \nthe level of the node.  \n\n\r", ch);				
				send_to_char( "&BByte : By the way, you should explore a little more \ntoward west, my brothers will teach you more about \nstuff you can find for sale.\n\n\r", ch);
				send_to_char( "&BByte : Have a good uptime in CyberSpace, human.\n\n\r", ch);
				//(social)Byte goodbyes
				ch->lesson= ch->lesson|BV03;
			}
			break;

			case 32104 ://newtuto supply 2
			if( (ch->lesson&BV04) !=BV04)
			{
				send_to_char( ">The Byte flies in front of you and says :\n\r", ch);
				send_to_char( "&BByte : Welcome human. \nI'm glad you choose to follow our teaching.\nLet's talk a little about modules you can buy ... \n\n\r", ch);
				WAIT_STATE(ch, 2);
				send_to_char( "&BByte : Compilers are mostly for Codeapp and Decking\nAlso needed for a number of other codeskills\nDevkits are for slicing skills and codemed\nParsers are used with most codeskills \n\n\r", ch);
				send_to_char( "&BByte : Comlinks are required for some chat channels\nComlinks only work when held or in memory\nDataminers help when digging up items\nItems are only found after they are buried \n\n\r", ch);
				send_to_char( "&BByte : The two last types of module, Blade and Blaster,\nwill be explained in detail in the room at west. \n\n\r", ch);
				send_to_char( "&BByte : Have a good uptime in CyberSpace, human.\n\n\r", ch);
				//(social)Byte goodbyes
				ch->lesson= ch->lesson|BV04;
			}
			break;

			case 32105 ://newtuto supply 3
			if( (ch->lesson&BV05) !=BV05)
			{
				send_to_char( ">The Byte flies in front of you and says :\n\r", ch);
				send_to_char( "&BByte : Welcome human. \nI\'m glad you choose to follow our teaching.\nLet\'s talk a little about .. weapons !\n\n\r", ch);
				WAIT_STATE(ch, 2);
				send_to_char( "&BByte : Blaster and Blade modules are basic weapons\nTheir damage is low, but their ammo is high\nTo use a weapon, you must HOLD it (HOLD weapon_name).\nBasically, you can only hold one thing at once, but soon you'll \nlearn how to be able to hold two.\n\n\r", ch);
				send_to_char( "&BByte : You can see what you're holding (and wearing) by typing \nEQUIPMENT (or EQ). Use REMOVE equipment_name to \nget your hand free (the equipment will return in your inventory).\nIf you use EQ (or EXAMINE weapon_name) , you'll also see \n how many ammo you've on your weapon.\n\n\r", ch);
				send_to_char( "&BByte : Blaster patches are used to reload blasters \nand utility patchesto reload blades.\nA Blaster shot takes various amount of ammo, depending on \nthe level it's set. Use SETBLASTER to change the power of \nyour blaster.\nBlade consumes its charge when you're holding it, either if \nyou use it or not.\nTo reload a weapon, hold it, make sure you've the right type \nof patch in your inventory and use PATCH, or AMMO command.\n\n\r", ch);
				send_to_char( "&BByte : For the last items, please go west.\nHave a good uptime in CyberSpace, human.\n\n\r", ch);
				//(social)Byte goodbyes
				ch->lesson= ch->lesson|BV05;
			}
			break;

			case 32106 ://newtuto supply 4
			if( (ch->lesson&BV06) !=BV06)
			{
				send_to_char( ">The Byte flies in front of you and says :\n\r", ch);
				send_to_char( "&BByte : Welcome human. \nI'm glad you choose to follow our teaching.\nThis is the last step about Supply Nodes \nand what you can find in there.\n\n\r", ch);
				WAIT_STATE(ch, 2);
				send_to_char( "&BByte : Classes (function, utility, blade, blaster, def) \nare materials required for the codeskills.\nMost of them can be found at a supply node \nbut Def classes are sold by traders in terminal nodes.\r\n\n\r", ch);
				send_to_char( "&BByte : Wilderspace subroutines are strange things : \nthey are for making medmods to heal people\nIts all you need except a devkit and codemed.\n\n\r", ch);
				send_to_char( "&BByte : Please go back (hit east few time) to the Path \nto continue your quest.\n\n\r", ch);
				send_to_char( "&BByte : Have a good uptime in CyberSpace, human.\n\n\r", ch);
				//(social)Byte goodbyes
				ch->lesson= ch->lesson|BV06;
			}
			break;
			
			case 32099 ://newtuto subserevr1
			if( (ch->lesson&BV07) !=BV07)
			{
				send_to_char( ">The Byte flies in front of you and says :\n\r", ch);
				send_to_char( "&BByte : Welcome human. \nI'm glad you choose to follow our teaching.\nWe will talk a little bit further about level of node :\nanalyze this node and you will see it is blue.\nAll nodes are blue when they are constructed.\nYou'll see that, in this system, Tutors constructed\nyou many nodesof each type and upgraded it.\n\n\r",ch);
				send_to_char( "&BByte : You can also see that you're in a Subserver.\nThey generate resource files, useful when\nyou need something to decompile.\n\n\r",ch);
				send_to_char( "&BByte : Resource file are :\nMacro [alpha], value 4 snippets\nRoutine [alpha], value 8 snippets\nHeader [alpha], value 16 snippets\nFunction [alpha], value 32 snippets\n\n\r",ch);
				send_to_char( "&BByte : Snippets are required for the codeapp skill\nbut you'll get into that later.\n[alpha] is the quality of the file, and generally,\nfiles from blue subservers have little value.\n\n\r",ch);
				send_to_char( "&BByte : Please go west to see other levels.\n\n\r", ch);
				send_to_char( "&BByte : Have a good uptime in CyberSpace, human.\n\n\r", ch);
				ch->lesson= ch->lesson|BV07;
			}
			break;
			
			case 32109 ://newtuto subserevr2
			if( (ch->lesson&BV08) !=BV08)
			{
				send_to_char( ">The Byte flies in front of you and says :\n\r", ch);
				send_to_char( "&BByte : Analyze the node and you will see it is green\nThe NODEUP command upgrades your nodes\nThere are 5 upgrades available for any node\nEach upgrade costs twice as much as the last\nUpgrading nodes increases their potential\nThey will spawn better entities and objects\nAnd even make a codegate harder to pick\n\n\r", ch);
				send_to_char( "&BByte : You can see that files generated here\nare better :\nMacro [beta], value 8 snippets\nRoutine [beta], value 16 snippets\nHeader [beta], value 32 snippets\nFunction [beta], value 64 snippets\n\n\r", ch);
				send_to_char( "&BByte : Please go west to see other levels.\n\n\r", ch);
				send_to_char( "&BByte : Have a good uptime in CyberSpace, human.\n\n\r", ch);
				ch->lesson= ch->lesson|BV08;
			}
			break;
			
			
			case 32110 ://newtuto subserevr3
			if( (ch->lesson&BV09) !=BV09)
			{			
				send_to_char( ">The Byte flies in front of you and says :\n\r", ch);
				send_to_char( "&BByte : Continue analyzing these subservers as you go\nYou want to find the black subserver\nYou may see some different files as you go\nThere are four different kinds found here\nMacro [candidate], value 16 snippets\nRoutine [candidate], value 32 snippets\nHeader [candidate] , value 64 snippets\nFunction [candidate] , value 128 snippets\n\n\r", ch);
				send_to_char( "&BByte : Please go west to see other levels.\n\n\r", ch);
				send_to_char( "&BByte : Have a good uptime in CyberSpace, human.\n\n\r", ch);
				ch->lesson= ch->lesson|BV09;
			}
			break;
			
			case 32111://newtuto subserevr4
			if( (ch->lesson&BV10) !=BV10)
			{			
				send_to_char( ">The Byte flies in front of you and says :\n\r", ch);
				send_to_char( "&BByte : Continue analyzing these subservers as you go\nYou want to find the black subserver \nMacros have the lowest value of the four\nRoutines are twice as valuable as a Macro\nHeaders are twice as valuable as Routines\nFunctions are twice as valuable as Headers\nMacro [release], value 32 snippets \nRoutine [release] value 64 snippets \nHeader [release], value 128 snippets \nFunction [release], value 256 snippets \n\n\r", ch);
				send_to_char( "&BByte : Please go west to see other levels.\n\n\r", ch);
				send_to_char( "&BByte : Have a good uptime in CyberSpace, human.\n\n\r", ch);
				ch->lesson= ch->lesson|BV10;
			}
			break;

			case 32112://newtuto subserevr5
			if( (ch->lesson&BV11) !=BV11)
			{			
				send_to_char( ">The Byte flies in front of you and says :\n\r", ch);
				send_to_char( "&BByte : Continue analyzing these subservers as you go\nYou have almost reached the black subserver ! \nWhen you'll learn about decompiling,\nremember to choose wisely what to decompile into what\nMacro [prototype], value 64 snippets \nRoutine [prototype], value 128 snippets\nHeader [prototype], value 256 snippets\nFunction [prototype], value 512 snippets\n\n\r", ch);
				send_to_char( "&BByte : Please go west to see other levels.\n\n\r", ch);
				send_to_char( "&BByte : Have a good uptime in CyberSpace, human.\n\n\r", ch);
				ch->lesson= ch->lesson|BV11;
			}
			break;

			case 32113://newtuto subserevr6
			if( (ch->lesson&BV12) !=BV12)
			{			
				send_to_char( ">The Byte flies in front of you and says :\n\r", ch);
				send_to_char( "&BByte : A black node is fully upgraded\nYou want as many black nodes as you can get\nFiles from black subservers have high value\nThey are great for decompiling snippets\nRemember, there are 6 levels of node : \nblue, green, orange, red, ultra-violet and black.Macro [wilderspace], value 128 snippets\nRoutine [wilderspace], value 256 snippets\nHeader [wilderspace], value 512 snippets\nFunction [wilderspace], value 1024 snippets\n\n\r", ch);
				send_to_char( "&BByte : Please go west to see other levels.\n\n\r", ch);
				send_to_char( "&BByte : Have a good uptime in CyberSpace, human.\n\n\r", ch);
				ch->lesson= ch->lesson|BV12;
			}
			break;

			case 32097 ://newtuto path2
			if( (ch->lesson&BV13) !=BV13)
			{
				send_to_char( ">The Byte flies in front of you and says :\n\r", ch);
				send_to_char( "&BByte : We will learn some basic commands.\nFirst, about the environment around you :\nAnalyze a node to see its information\nLook to see the whole node and its content\nEvaluate an item to see its information\nExamine an item to see its description\nExamine a container to see what is in it\n\n\r", ch);
				send_to_char( "&BByte : Then, about communicate with other players:\nUse SAY to speak to everybody in the node\nUse TELL <name_of_player> to speak to him\nUse NEWBIE to speak on the newbie channel\nUse GCHAT to speak about general things\nUse OOC to speak Out Of Character\n\n\r", ch);
				send_to_char( "&BByte : Please use the right channel for the right thing.\nYou should start by saying \"Hi\" on the NEWBIE\nchannel, this way everybody on the CyberSpace\nwould welcome you.\nType WHO to see who is on.\n\n\r", ch);
				send_to_char( "&BByte : Have a good uptime in CyberSpace, human.\n\n\r", ch);
				ch->lesson= ch->lesson|BV13;
			}
			break;
			
			case 32098 ://newtuto database 1
				send_to_char( ">The Byte flies in front of you and says :\n\r", ch);
				send_to_char( "&BByte : You should be sure to be well prepared before\ngoing further. Databases are dangerous places :\nnaughty virii spawn in there.\nI advice you to learn some fighting skills (go back\nto the path and follow it north) and you'll return\nwhen you'll need the files you can find here.\n\n\r", ch);
				send_to_char( "&BByte : If you already learn it, type SNEAK\nThis will make you unnoticed and you'll\n be able to enter databases without\n being aggressed by virii.\nIf you have to fight, type KICK\n to make a more powerful attack\n If you want to fight, type KILL virus.\n\n\r",ch);
				send_to_char( "&BByte : Have a good uptime in CyberSpace, human.\n\n\r", ch);
			break;
			
			case 32100://newtuto path 3
			if( (ch->lesson&BV14) !=BV14)
			{
				send_to_char( ">The Byte flies in front of you and says :\n\r", ch);
				send_to_char( "&BByte : Well, human, what do you think about learning\nnew abilities ?\nType SKILL to see what you already know.\nYou can see that you know some basics, with a\nlevel around 50. When you use a skill, the level\nwill increase, until 100 and then you'll be an adept.\nTo see what you can learn, type BUYSKILL.\nThis displays a long list, but don't worry, it's easy.\n\n\r", ch);
				send_to_char( "&BByte : As you're a newbie, I advice you to learn KICK and\nSNEAK.\nKick will be very usefull in combat, you'll see that\nsoon, and sneak is very useful to avoid combat.\nWith those, you'll be able to go in databases without\nrisks and you'll develop other capabilities while using it.\nBuying skill cost you money, type MONEY to see\nhow much you've got now.\nThere're numerous skills, you'll learn the use of each\nwhen you'll need it.\n\n\r", ch);
				send_to_char( "&BByte : If you follow the path east, you'll learn more about coding and will need other skills./n/n/r",ch);
				send_to_char( "&BByte : Have a good uptime in CyberSpace, human.\n\n\r", ch);
				ch->lesson= ch->lesson|BV14;
			}
			break;
			
			case 32102://newtuto job
			if( (ch->lesson&BV15) !=BV15)
			{
				send_to_char( ">The Byte flies in front of you and says :\n\r", ch);
				send_to_char( "&BByte : Here you are in an employment node.\nYou can get job (so gain credits) by typing JOB.\nThere are different kind of job. When you've got\none, type JOB again to see details about it.\n\r", ch);
				send_to_char( "&BByte : You can have to deliver a package at a target :\nyou have to connect to the target system, find\nthe employment node and type COMPLETE.\n\r", ch);
				send_to_char( "&BByte : You can have to find a resource and bring it\nto a target system : find the right item, connect\nto the target system, find the employment node and\ntype COMPLETE.\n\r", ch);
				send_to_char( "&BByte : You can have an agent job, requiring you to\ninfect a city : you have to connect to the target\nsystem, find a dataminer program and type\nUPLOAD VIRUS DATAMINER. Those job\ncould be dangerous, because you'll need to\nintrude into protected systems.\n\r", ch);
				send_to_char( "&BByte : Anyway, if you give up, use JOB CLEAR.\nBut you'll have to pay some credits for that.\n\r", ch);
				send_to_char( "&BByte : Have a good uptime in CyberSpace, human.\n\n\r", ch);
				ch->lesson= ch->lesson|BV15;
			}
			break;
			
			case 32103://newtuto job
			if( (ch->lesson&BV16) !=BV16)
			{
				send_to_char( ">The Byte flies in front of you and says :\n\r", ch);
				send_to_char( "&BByte : This is a trader. You can sell it stuffs\nbut it's only interested by good items\nType LIST to see if it has something\nto sell.\nYou can find different type of Traders\n[virii] sell you viral code, what is\nuseful for poisonned weapons\n[def] sell def classes\n[patches] sell patches and tools\nbut also other various items\n\r", ch);
				send_to_char( "&BByte : Have a good uptime in CyberSpace, human.\n\n\r", ch);
				ch->lesson= ch->lesson|BV16;
			}
			break;
			
			
			case 32114 :// newtuto database
			if( (ch->lesson&BV17) !=BV17)
			{
				send_to_char( ">The Byte flies in front of you and says :\n\r", ch);
				send_to_char( "&BByte : If you get a job from an Agent node\nYou will have to find a Dataminer\nUpload or give the virus to the dataminer:\n\r", ch);
				send_to_char( "&BByte : Murphy virus are easy to kill, use KICK.\nA Predator Virus is a bit more dangerous\nIf you cant defeat one, you should flee\nThey will chase you even if you're sneaking:\n\r", ch);
				ch->lesson= ch->lesson|BV17;
			}
			break;
			
			case 32115:// newtuto database
			if( (ch->lesson&BV18) !=BV18)
			{
				send_to_char( ">The Byte flies in front of you and says :\n\r", ch);
				send_to_char( "&BByte : Clusters, carriers, and collectors are basic,\nfriendly programs. They are weak and run\naway if threatened.:\n\r", ch);
				send_to_char( "&BByte : You may find different classes\nsuch as a def class or blade class.\nThese are used for various codeskills but\nmost can be purchased at a supply node\nor decompiled from a resource file:\n\r", ch);
				ch->lesson= ch->lesson|BV18;
			}
			break;
			
			case 32116:// newtuto database
			if( (ch->lesson&BV19) !=BV19)
			{
				send_to_char( ">The Byte flies in front of you and says :\n\r", ch);
				send_to_char( "&BByte : One particular class you should look for\nis the Application class, used with codeutil.\nYou can purchase materials for armor\nbut you have to find Applications for utils.\nIt's the only way to reach -50 damage mod.\nType SCORE now to see your damage mod\nA full set of armor and a shield gives -44.\n\r", ch);
				ch->lesson= ch->lesson|BV19;
			}
			break;
			
			case 32117:// newtuto database
			if( (ch->lesson&BV20) !=BV20)
			{
				send_to_char( ">The Byte flies in front of you and says :\n\r", ch);
				send_to_char( "&BByte : Evaluate Application classes as you find them\nCost is the important number here but  to have\na perfect util with 30 defense, you need an\nApplication with a cost of 100:\n\r", ch);
				send_to_char( "&BByte : You can get an Application with decompile app\nHowever, these have a cost of 25 and always result\nin a broken util. However, they can be good to practice :\n\r", ch);
				ch->lesson= ch->lesson|BV20;
			}
			break;
			
			default :  send_to_char( "&BByte : urm\n\n\r", ch);
				break;

		}
	}
	
}