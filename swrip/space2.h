/***************************************************************************
*                           STAR WARS: RISE IN POWER                       *
*--------------------------------------------------------------------------*
* Star Wars: Rise in Power Code Additions to Star Wars Reality 1.0         *
* copyright (c) 1999 by the Coding Team at Star Wars: Rise in Power        *
*--------------------------------------------------------------------------*
* Star Wars Reality Code Additions and changes from the Smaug Code         *
* copyright (c) 1997 by Sean Cooper                                        *
* -------------------------------------------------------------------------*
* Starwars and Starwars Names copyright(c) Lucas Film Ltd.                 *
*--------------------------------------------------------------------------*
* SMAUG 1.0 (C) 1994, 1995, 1996 by Derek Snider                           *
* SMAUG code team: Thoric, Altrag, Blodkai, Narn, Haus,                    *
* Scryn, Rennard, Swordbearer, Gorog, Grishnakh and Tricops                *
* ------------------------------------------------------------------------ *
* Merc 2.1 Diku Mud improvments copyright (C) 1992, 1993 by Michael        *
* Chastain, Michael Quan, and Mitchell Tse.                                *
* Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,          *
* Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.     *
* ------------------------------------------------------------------------ *
*                           Space Module 2 Include                         *
****************************************************************************/

#include <math.h>
#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/dir.h>
#include "mud.h"
#define MAX_TEMPLATETYPE 38

struct templatetype templatetypes[MAX_TEMPLATETYPE] =
{
  { 0, 0, "JV-7 Shuttle", "3] 8)22:2; 2)22:10", "Cygunus Spaceworks JV-7-Three room shuttlecraft with passenger lounge.", 40, 15, 800 },
  { 1, 0, "T-Wing Starfighter", "1] 8;", "Hoersh-Kessel T-wing Starfighter - Cheap and fast.", 30, 10, 500 },
  { 2, 0, "R-41 Starchaser", "1] 8;", "Hoersh-Kessel R-41 Starchaser - Room enough for two.", 35, 10, 550 },
  { 3, 0, "Z-95 Headhunter", "1] 8;", "Z-95 - Highly adaptable, cheap and fast.", 30, 20, 550 },
  { 4, 0, "Z-100 Centurion", "1] 8;", "Z-100 - Two-passenger, high adaptable.", 35, 20, 600 },
  { 5, 0, "HLAF-500", "1] 8;", "CEC HLAF-500 - Fast single-seater.", 30, 10, 500 },
  { 6, 0, "KSE Cloakshape", "2] 8)22:11", "Cloakshape w/stabilizer conversion set - Slow and sluggish, but highly adaptable.", 48, 15, 1000 },
  { 7, 0, "VFS Lightning Bomber", "1] 8;", "VFS Lightning Bomber - Two-seater Heavy Bomber.", 35, 30, 650 },
  { 8, 0, "VFS Road Runner", "1] 8;", "VFS Road Runner - Fast and light.", 30, 5, 250 },
  { 9, 0, "A-9 Vigiliance Interceptor", "1] 8;", "A-9 - Fast and agile.", 32, 5, 300 },
  {10, 0, "VFS Gauntlet", "1] 8;", "Gauntlet-Designed to get out and fast.", 35, 3, 250 },
  {11, 0, "Shobquix Tocsan 8-Q", "1] 8;", "Toscan 8-Q - Heavy, but durable.", 40, 15, 1000 },
  {12, 0, "Pinook Starfighter", "1] 8;", "Pinook - Small and versatile.", 30, 10, 500 },
  {13, 0, "Sunhui Spaceworks Razor", "1] 8;", "Razor - Adaptable fighter.", 35, 15, 600 },
  {14, 10, "T-65c A2 Starfighter", "1] 8;", "X-Wing - Powerful and versatile.", 40, 15, 500 },
  {15, 10, "Koensayr BTL-S3 Longprobe", "1] 8;", "Y-Wing - Two-seater, sturdy, but slow and sluggish.", 45, 30, 1000 },
  {16, 10, "Dodonna/Blissez RZ-1 Starfighter", "1] 8;", "A-Wing - Fast and agile.", 35, 20, 300 },
  {17, 20, "SFS Tie/ln StarFighter", "1] 8;", "TIE Fighter - Small and agile, but weak.", 25, 5, 150 },
  {18, 20, "SFS Tie/Int Starfighter", "1] 8;", "TIE Interceptor - Small and agile, can be more heavily armed then the Tie/ln.", 30, 10, 200 },
  {19, 20, "SFS Tie/B Starfighter", "1] 8;", "TIE Bomber - Heavy and slow, but can be better equipped.", 35, 15, 800 },
  {20, 20, "SFS Tie/R Starfighter", "1] 8;", "Tie Recon - Expanded Tie/ln to include more internal space.", 35, 30, 800 },
  {21, 20, "Cygnus Spaceworks XG-1", "1]8;", "Assault Gunboat - Adaptable and durable.", 40, 20, 700 },
  {22, 1, "KSE Firespray-31", "5] 9)22:13,23:2,28:12,29:4;", "Firespray - Powerful and relatively light.", 60, 20, 2000 },
  {23, 1, "Pursuer Enforcement Ship", "6] 9)20:8,29:2,22:14;14)22:13,29:4;", "Pursuer - Versatile.", 60, 20, 2500 },
  {24, 21, "Fast Attack Patrol Ship", "8] 9)20:8,23:2,21:16;14)20:9,23:17,21:10,22:4;", "FAPS - Strong and light.", 65, 15, 2000 },
  {25, 21, "Gamma-class Assault Shuttle", "3] 8)22:2; 2)22:18", "Imperial Telgorn GAS - Versatile.", 50, 20, 2500 },
  {26, 11, "Gamma-class Assault Shuttle", "3] 8)22:2; 2)22:18", "Rebel Telgorn GAS - Versatile.", 50, 20, 2500 },
  {27, 1, "CEC Action IV", "10] 22)20:8,22:9;9)22:4,29:14;14)22:20,21:13,23:19,20:21;2)22:21;", "Act IV - BIG ship.", 75, 20, 5500 },
  {28, 1, "CEC YT-1300", "8] 9)20:8,28:23,23:2,25:17,27:4;20)22:13,24:17,26:4;", "YT-1300 - Highly adaptable, light.", 65, 20, 3500 },
  {29, 1, "CEC Modified YT-1300", "9] 9)20:8,28:23,29:24,23:2,25:17,27:4;20)22:13,24:17,26:4;", "YT-1300 - Modified has add'l turret mount.", 65, 20, 3500 },
  {30, 1, "CEC Barloz", "6] 8)22:2;9)20:2,23:20,22:4;20)22:13;", "Barloz - Versatile, mid-weight.", 65, 20, 3500 },
  {31, 0, "SFS Gat-12 Starfighter", "3] 8)22:2;2)22:4", "Skipray Blastboat - Light for its mods.", 65, 15, 3000 },
  {32, 21, "MT/191 Drop-Ship", "9] 9)20:8,23:23,21:24,22:14;14)23:18,21:25,22:4,29:2;", "Imperial Dropship - Heavy.", 65, 15, 5500 },
  {33, 11, "MT/191 Drop-Ship", "9] 9)20:8,23:23,21:24,22:14;14)23:18,21:25,22:4,29:2;", "Rebel Dropship - Heavy.", 65, 15, 3500 },
  {34, 1, "Mobquet Medium Cargo Hauler", "8] 9)20:8,23:23,21:24,22:2;2)23:17,21:26,22:4;", "Mobquest - BIG ship, high mods.", 80, 20, 6500 },
  {35, 2, "Corellian CR-90 Blockade Runner", "19] 12)24:3,25:1,23:5,21:6,22:22;3)23:1,22:6;1)22:5;51)28:22,21:16,23:62,22:21;21)21:20,23:17,22:27;20)22:13;27)23:26,22:52;17)22:26;52)21:7,22:4,23:61;", "Corellian Corvette - Capital class - CR-90 Blockade Runner", 400, 20, 10000 },
  {36, 2, "Corellian Gunship", "20]12)25:01,24:03,23:05,21:06,29:22;01)22:05,21:03;03)22:06;22)22:27,29:15;27)23:13,21:16,22:28;28)21:11,23:20,22:29;29)21:62,23:19,22:30;20)20:13;07)28:30,29:04,21:61", "Corellian Gunship - Capital class - Slightly heavier than the runner.", 400, 20, 12000 },
  {37, 2, "Marauder Corvette", "22]12)20:03;03)20:01,21:06,23:05;01)27:05,26:06;61)20:12,22:51;51)21:27,23:28,22:31,29:07;29)23:27,21:20;30)21:28,23:10;20)26:13;10)27:19;32)20:31,22:04,29:26;26)20:15;12)29:16;", "Marauder Corvette - Capital class - Slightly heavier than the runner.", 400, 20, 12000 }
};

#define MAX_CARGO_NAMES 10

char *  const   cargo_names [MAX_CARGO_NAMES] = 
{ "ore", "food", "electronics", "weapons", "medical", "clothing", "luxuries", 
  "spice", "water", "special"
};

//Price defaults for each cargo type.
struct cargo_data cargodefaults[CARGOTYPE_DEFAULT] =
{
  { CARGOTYPE_ORE, 		2000	 },
  { CARGOTYPE_FOOD, 		500 	 },
  { CARGOTYPE_ELECTRONICS, 	5000	 },
  { CARGOTYPE_WEAPONS, 		10000 	 },
  { CARGOTYPE_MEDICAL, 		8000 	 },
  { CARGOTYPE_CLOTHING, 	1000	 },
  { CARGOTYPE_LUXURIES, 	20000 	 },
  { CARGOTYPE_SPICE, 		15000 	 },
  { CARGOTYPE_WATER, 		200 	 },
  { CARGOTYPE_SPECIAL, 		5000 	 }
};
