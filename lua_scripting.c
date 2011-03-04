/****************************************************************************
 * [S]imulated [M]edieval [A]dventure multi[U]ser [G]ame      |   \\._.//   *
 * -----------------------------------------------------------|   (0...0)   *
 * SMAUG 1.4 (C) 1994, 1995, 1996, 1998  by Derek Snider      |    ).:.(    *
 * -----------------------------------------------------------|    {o o}    *
 * SMAUG code team: Thoric, Altrag, Blodkai, Narn, Haus,      |   / ' ' \   *
 * Scryn, Rennard, Swordbearer, Gorog, Grishnakh, Nivek,      |~'~.VxvxV.~'~*
 * Tricops and Fireblade                                      |             *
 * ------------------------------------------------------------------------ *
 * Merc 2.1 Diku Mud improvments copyright (C) 1992, 1993 by Michael        *
 * Chastain, Michael Quan, and Mitchell Tse.                                *
 * Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,          *
 * Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.     *
 * ------------------------------------------------------------------------ *
 *			 Lua Scripting Module     by Nick Gammon                  			    *
 ****************************************************************************/
 
#include <stdio.h>
#include <string.h>

#include "mud.h"

//#include "lualib.h"
//#include "lauxlib.h"

#define CHARACTER_STATE "character.state"
#define MUD_LIBRARY "mud"

//DEBUG LUA//
#define MAX_VNUM 256



/* void RegisterLuaCommands (lua_State *L); */ /* Implemented in lua_commands.c */
LUALIB_API int luaopen_bits(lua_State *L);  /* Implemented in lua_bits.c */

#if 0 /* not used yet */
static int optboolean (lua_State *L, const int narg, const int def) 
  {
  /* that argument not present, take default  */
  if (lua_gettop (L) < narg)
    return def;

  /* nil will default to the default  */
  if (lua_isnil (L, narg))
    return def;

  if (lua_isboolean (L, narg))
    return lua_toboolean (L, narg);

  return luaL_checknumber (L, narg) != 0;
}
#endif

static int check_vnum (lua_State *L)
  {

  int vnum = luaL_checknumber (L, 1);
  if ( vnum < 1 || vnum > MAX_VNUM )
    luaL_error (L, "Vnum %d is out of range 1 to %d", vnum, MAX_VNUM);

  return vnum;
  }  /* end of check_vnum */


/* Given a Lua state, return the character it belongs to */

CHAR_DATA * L_getchar (lua_State *L)
 {
 /* retrieve our character */
 
  CHAR_DATA * ch;
  
  /* retrieve the character */
  lua_pushstring(L, CHARACTER_STATE);  /* push address */
  lua_gettable(L, LUA_ENVIRONINDEX);  /* retrieve value */

  ch = (CHAR_DATA *) lua_touserdata(L, -1);  /* convert to data */
  lua_pop(L, 1);  /* pop result */

  return ch;
} /* end of L_getchar */
  

static CHAR_DATA * L_find_character (lua_State *L)
{
  CHAR_DATA * ch = L_getchar (L);
  const char * name = luaL_optstring (L, 1, "self");
  CHAR_DATA *wch = NULL;
  
  if (strcasecmp (name, "self") == 0)
    return ch;
  
  /*
  * check the room for an exact match 
  */
  for( wch = ch->in_room->first_person; wch; wch = wch->next_in_room )
    if( can_see( ch, wch ) && 
       !IS_NPC( wch ) && 
       ( strcasecmp ( name, wch->name ) == 0 ))
      break;  /* found it! */
  
  /*
  * check the world for an exact match 
  */
  if (!wch)
   {
   for( wch = first_char; wch; wch = wch->next )
      if( can_see( ch, wch ) && 
          !IS_NPC( wch ) &&
          ( strcasecmp( name, wch->name ) == 0 ) )
        break;        
    }  /* end of checking entire world */   
  
  return wch;  /* use target character, not self */
   
}  /* end of L_find_character */

/* For debugging, show traceback information */

static void GetTracebackFunction (lua_State *L)
  {
  lua_pushliteral (L, LUA_DBLIBNAME);     /* "debug"   */
  lua_rawget      (L, LUA_GLOBALSINDEX);    /* get debug library   */

  if (!lua_istable (L, -1))
    {
    lua_pop (L, 2);   /* pop result and debug table  */
    lua_pushnil (L);
    return;
    }

  /* get debug.traceback  */
  lua_pushstring(L, "traceback");  
  lua_rawget    (L, -2);               /* get traceback function  */
  
  if (!lua_isfunction (L, -1))
    {
    lua_pop (L, 2);   /* pop result and debug table  */
    lua_pushnil (L);
    return;
    }

  lua_remove (L, -2);   /* remove debug table, leave traceback function  */
  }  /* end of GetTracebackFunction */

static int CallLuaWithTraceBack (lua_State *L, const int iArguments, const int iReturn)
  {

  int error;
  int base = lua_gettop (L) - iArguments;  /* function index */
  GetTracebackFunction (L);
  if (lua_isnil (L, -1))
    {
    lua_pop (L, 1);   /* pop non-existent function  */
    error = lua_pcall (L, iArguments, iReturn, 0);
    }  
  else
    {
    lua_insert (L, base);  /* put it under chunk and args */
    error = lua_pcall (L, iArguments, iReturn, base);
    lua_remove (L, base);  /* remove traceback function */
    }

  return error;
  }  /* end of CallLuaWithTraceBack  */

  
/* let scripters find our directories and file names */

#define INFO_ITEM(arg) \
  lua_pushstring (L, arg);  \
  lua_setfield (L, -2, #arg)
  
static int L_system_info (lua_State *L)
{
  lua_newtable(L);  /* table for the info */
  //DEBUG LUA
  /*INFO_ITEM (PLAYER_DIR);
  INFO_ITEM (BACKUP_DIR);
  INFO_ITEM (GOD_DIR);
  INFO_ITEM (BOARD_DIR);
  INFO_ITEM (CLAN_DIR);
  INFO_ITEM (COUNCIL_DIR);
  INFO_ITEM (DEITY_DIR);
  INFO_ITEM (BUILD_DIR);
  INFO_ITEM (SYSTEM_DIR);
  INFO_ITEM (PROG_DIR);
  INFO_ITEM (CORPSE_DIR);
  INFO_ITEM (CLASS_DIR);
  INFO_ITEM (RACE_DIR);
  INFO_ITEM (WATCH_DIR);
  INFO_ITEM (LUA_DIR);
  INFO_ITEM (AREA_LIST);
  INFO_ITEM (WATCH_LIST);
  INFO_ITEM (BAN_LIST);
  INFO_ITEM (RESERVED_LIST);
  INFO_ITEM (CLAN_LIST);
  INFO_ITEM (COUNCIL_LIST);
  INFO_ITEM (GUILD_LIST);
  INFO_ITEM (GOD_LIST);
  INFO_ITEM (DEITY_LIST);
  INFO_ITEM (CLASS_LIST);
  INFO_ITEM (RACE_LIST);
  INFO_ITEM (MORPH_FILE);
  INFO_ITEM (BOARD_FILE);
  INFO_ITEM (SHUTDOWN_FILE);
  INFO_ITEM (IMM_HOST_FILE);
  INFO_ITEM (RIPSCREEN_FILE);
  INFO_ITEM (RIPTITLE_FILE);
  INFO_ITEM (ANSITITLE_FILE);
  INFO_ITEM (ASCTITLE_FILE);
  INFO_ITEM (BOOTLOG_FILE);
  INFO_ITEM (PBUG_FILE);
  INFO_ITEM (IDEA_FILE);
  INFO_ITEM (TYPO_FILE);
  INFO_ITEM (FIXED_FILE);
  INFO_ITEM (LOG_FILE);
  INFO_ITEM (MOBLOG_FILE);
  INFO_ITEM (WIZLIST_FILE);
  INFO_ITEM (WHO_FILE);
  INFO_ITEM (WEBWHO_FILE);
  INFO_ITEM (REQUEST_PIPE);
  INFO_ITEM (SKILL_FILE);
  INFO_ITEM (HERB_FILE);
  INFO_ITEM (TONGUE_FILE);
  INFO_ITEM (SOCIAL_FILE);
  INFO_ITEM (COMMAND_FILE);
  INFO_ITEM (PROJECTS_FILE);
  INFO_ITEM (PLANE_FILE);*/
  INFO_ITEM (LUA_STARTUP);
  return 1;  /* the table itself */
}  /* end of L_system_info */

#define CH_STR_ITEM(arg) \
  if (ch->arg)  \
  {   \
  lua_pushstring (L, ch->arg);  \
  lua_setfield (L, -2, #arg); \
  }
  
#define CH_NUM_ITEM(arg) \
  lua_pushnumber (L, ch->arg);  \
  lua_setfield (L, -2, #arg)
    
#define PC_STR_ITEM(arg) \
if (pc->arg)  \
  {   \
  lua_pushstring (L, pc->arg);  \
  lua_setfield (L, -2, #arg); \
  }
  
#define PC_NUM_ITEM(arg) \
  lua_pushnumber (L, pc->arg);  \
  lua_setfield (L, -2, #arg)
    
static int L_character_info (lua_State *L)
{
  CHAR_DATA * ch = L_find_character (L);
     
  if (!ch)
    return 0;
 
  PC_DATA *pc = ch->pcdata;
  
  lua_newtable(L);  /* table for the info */
  
  /* strings */
  
  CH_STR_ITEM (name);
  CH_STR_ITEM (short_descr);
  CH_STR_ITEM (long_descr);
  CH_STR_ITEM (description);
  
  /* numbers */
  
  CH_NUM_ITEM (num_fighting);
  CH_NUM_ITEM (substate);
  CH_NUM_ITEM (sex);
  //CH_NUM_ITEM (Class);
  //CH_NUM_ITEM (race);
  //CH_NUM_ITEM (level);
  CH_NUM_ITEM (trust);
  CH_NUM_ITEM (played);
  CH_NUM_ITEM (logon);
  CH_NUM_ITEM (save_time);
  CH_NUM_ITEM (timer);
  CH_NUM_ITEM (wait);
  CH_NUM_ITEM (hit);
  CH_NUM_ITEM (max_hit);
  CH_NUM_ITEM (mana);
  CH_NUM_ITEM (max_mana);
  CH_NUM_ITEM (move);
  CH_NUM_ITEM (max_move);
  //CH_NUM_ITEM (practice);
  CH_NUM_ITEM (numattacks);
  CH_NUM_ITEM (gold);
  //CH_NUM_ITEM (exp);
  CH_NUM_ITEM (carry_weight);
  CH_NUM_ITEM (carry_number);
  CH_NUM_ITEM (xflags);
  //CH_NUM_ITEM (no_immune);
  //CH_NUM_ITEM (no_resistant);
  //CH_NUM_ITEM (no_susceptible);
  CH_NUM_ITEM (immune);
  CH_NUM_ITEM (resistant);
  CH_NUM_ITEM (susceptible);
  //CH_NUM_ITEM (speaks);
  //CH_NUM_ITEM (speaking);
  CH_NUM_ITEM (saving_poison_death);
  CH_NUM_ITEM (saving_wand);
  CH_NUM_ITEM (saving_para_petri);
  CH_NUM_ITEM (saving_breath);
  CH_NUM_ITEM (saving_spell_staff);
  CH_NUM_ITEM (alignment);
  CH_NUM_ITEM (barenumdie);
  CH_NUM_ITEM (baresizedie);
  CH_NUM_ITEM (mobthac0);
  CH_NUM_ITEM (hitroll);
  CH_NUM_ITEM (damroll);
  CH_NUM_ITEM (hitplus);
  CH_NUM_ITEM (damplus);
  CH_NUM_ITEM (position);
  CH_NUM_ITEM (defposition);
  //CH_NUM_ITEM (style);
  CH_NUM_ITEM (height);
  CH_NUM_ITEM (weight);
  CH_NUM_ITEM (armor);
  CH_NUM_ITEM (wimpy);
  CH_NUM_ITEM (deaf);
  CH_NUM_ITEM (perm_str);
  CH_NUM_ITEM (perm_int);
  CH_NUM_ITEM (perm_wis);
  CH_NUM_ITEM (perm_dex);
  CH_NUM_ITEM (perm_con);
  CH_NUM_ITEM (perm_cha);
  CH_NUM_ITEM (perm_lck);
  CH_NUM_ITEM (mod_str);
  CH_NUM_ITEM (mod_int);
  CH_NUM_ITEM (mod_wis);
  CH_NUM_ITEM (mod_dex);
  CH_NUM_ITEM (mod_con);
  CH_NUM_ITEM (mod_cha);
  CH_NUM_ITEM (mod_lck);
  CH_NUM_ITEM (mental_state);  
  CH_NUM_ITEM (emotional_state);  
     
  /* player characters have extra stuff (in "pc" sub table)  */
  if (pc)
  {
   lua_newtable(L);  /* table for the info */
      
   PC_STR_ITEM (homepage);
   PC_STR_ITEM (clan_name);
   //PC_STR_ITEM (council_name);
   //PC_STR_ITEM (deity_name);
   PC_STR_ITEM (bamfin);
   PC_STR_ITEM (bamfout);
   //PC_STR_ITEM (filename);   
   PC_STR_ITEM (rank);
   PC_STR_ITEM (title);
   PC_STR_ITEM (bestowments);     
   PC_STR_ITEM (helled_by);
   PC_STR_ITEM (bio);  
   PC_STR_ITEM (authed_by); 
   PC_STR_ITEM (prompt);    
   //PC_STR_ITEM (fprompt);   
   PC_STR_ITEM (subprompt); 
   
   PC_NUM_ITEM (flags);     
   PC_NUM_ITEM (pkills);    
   PC_NUM_ITEM (pdeaths);   
   //PC_NUM_ITEM (mkills);    
   //PC_NUM_ITEM (mdeaths);   
   //PC_NUM_ITEM (illegal_pk);   
   PC_NUM_ITEM (outcast_time); 
   PC_NUM_ITEM (restore_time); 
   //PC_NUM_ITEM (r_range_lo); 
   //PC_NUM_ITEM (r_range_hi);
   //PC_NUM_ITEM (m_range_lo); 
   //PC_NUM_ITEM (m_range_hi);
   //PC_NUM_ITEM (o_range_lo); 
   //PC_NUM_ITEM (o_range_hi);
   PC_NUM_ITEM (wizinvis);   
   PC_NUM_ITEM (min_snoop);  
   PC_NUM_ITEM (quest_number); 
   PC_NUM_ITEM (quest_curr);   
   PC_NUM_ITEM (quest_accum);  
   //PC_NUM_ITEM (favor);        
   //PC_NUM_ITEM (charmies);     
   PC_NUM_ITEM (auth_state);
   PC_NUM_ITEM (release_date); 
   PC_NUM_ITEM (pagerlen);     
   lua_setfield (L, -2, "pc");  
  }
     
  return 1;  /* the table itself */
}  /* end of L_character_info */
 
#define OBJ_STR_ITEM(arg) \
if (obj->arg)  \
  {   \
  lua_pushstring (L, obj->arg);  \
  lua_setfield (L, -2, #arg); \
  }
  
#define OBJ_NUM_ITEM(arg) \
  lua_pushnumber (L, obj->arg);  \
  lua_setfield (L, -2, #arg)
  
  
static void build_inventory (lua_State *L, OBJ_DATA * obj );

static void add_object_item (lua_State *L, OBJ_DATA * obj, const int item)
  {
int i;
  lua_newtable(L);  /* table for the info */
          
  OBJ_STR_ITEM (name);
  OBJ_STR_ITEM (short_descr);
  OBJ_STR_ITEM (description);
  OBJ_STR_ITEM (action_desc);
   
  OBJ_NUM_ITEM (item_type);
  OBJ_NUM_ITEM (magic_flags);  
  OBJ_NUM_ITEM (wear_flags);
  OBJ_NUM_ITEM (wear_loc);
  OBJ_NUM_ITEM (weight);
  OBJ_NUM_ITEM (cost);
  OBJ_NUM_ITEM (level);
  OBJ_NUM_ITEM (timer);
  OBJ_NUM_ITEM (count);    
  OBJ_NUM_ITEM (serial);   
  //OBJ_NUM_ITEM (room_vnum);
  
  if (obj->pIndexData)
    {
    lua_pushnumber (L, obj->pIndexData->vnum); 
    lua_setfield (L, -2, "vnum");
    }
    
  lua_newtable(L);  /* table for the values */
  
  /* do 6 values */
  for (i = 0; i < 6; i++)
    {
    lua_pushnumber (L, obj->value [i]); 
    lua_rawseti (L, -2, i + 1);       
    }
  lua_setfield (L, -2, "value");    
  
  if( obj->first_content )   /* node has a child? */
    {
    lua_newtable(L);  /* table for the inventory */
    build_inventory(L,  obj->first_content );
    lua_setfield (L, -2, "contents");
    }
  lua_rawseti (L, -2, item);         
  
    
  } /* end of add_object_item */
 
/* do one inventory level */
  
static void build_inventory (lua_State *L, OBJ_DATA * obj )
{
  int item = 1;
   
  for ( ; obj; obj = obj->next_content)
    {
      
    /* if carrying it, add to table */
    
    if( obj->wear_loc == WEAR_NONE)
      add_object_item (L, obj, item++);
      
  }  /* end of for loop */
  
}  /* end of build_inventory */

static int L_inventory (lua_State *L)
{
  CHAR_DATA * ch = L_find_character (L);
   
  if (!ch)
    return 0;
    
  lua_newtable(L);  /* table for the inventory */
    
  /* recursively build inventory */
  
  build_inventory (L, ch->first_carrying);
  
  return 1;  /* the table itself */
}  /* end of L_inventory */

static int L_equipped (lua_State *L)
  {
  int item = 1;
  CHAR_DATA * ch = L_find_character (L);
  OBJ_DATA * obj;
  
  if (!ch)
    return 0;
    
  lua_newtable(L);  /* table for the inventory */
    
  for (obj = ch->first_carrying ; obj; obj = obj->next_content)
    {
    if (ch == obj->carried_by && 
        obj->wear_loc > WEAR_NONE)
      add_object_item (L, obj, item++);
     }  /* end of for loop */

  return 1;  /* the table itself */
}  /* end of L_equipped */

static bool check_inventory (CHAR_DATA * ch, OBJ_DATA * obj, const int vnum )
{
  /* check all siblings */   
  for ( ; obj; obj = obj->next_content)
    {
    if( obj->wear_loc == WEAR_NONE)
      {
      if (obj->pIndexData && obj->pIndexData->vnum == vnum)
        return TRUE;

      if( obj->first_content )   /* node has a child? */
        {
        if (check_inventory(ch, obj->first_content, vnum ))
          return TRUE;
        }
      }  /* end of carrying it */
      
  }  /* end of for loop */
  
  return FALSE;
}  /* end of check_inventory */

/* argument: vnum of item */

static int L_carryingvnum (lua_State *L)
{
  CHAR_DATA * ch = L_getchar (L);
   
  int vnum = check_vnum (L);
     
  /* recursively check inventory */
  
  lua_pushboolean (L, check_inventory (ch, ch->first_carrying, vnum));
  
  return 1;  /* the result */
}  /* end of L_carryingvnum */

/* argument: location of item (eg. finger, legs, see item_w_flags) */
 
static int L_wearing (lua_State *L)
{
// CHAR_DATA * ch = L_getchar (L);
// OBJ_DATA * obj;
// const char * sloc = luaL_checkstring (L, 1);  /* location, eg. finger */
// int loc;

  // /* translate location into a number */
  // for (loc = 0; loc < MAX_WEAR; loc++)
    // if (strcasecmp (sloc, item_w_flags [loc]) == 0)
       // break;
     
   // if (loc >= MAX_WEAR)
     // luaL_error (L, "Bad wear location '%s' to 'wearing'", sloc);
    
   // for (obj = ch->first_carrying ; obj; obj = obj->next_content)
    // {
    // if (ch == obj->carried_by && loc == obj->wear_loc)
       // {
       // lua_pushnumber (L, obj->pIndexData->vnum);
       // return 1;
       // }  /* end of if worn at desired location */
    // }  /* end of for loop */

  // lua_pushboolean (L, FALSE);
  // return 1;  /* the result */
  return 0;
}  /* end of L_wearing */
    
/* argument: vnum of item */
 
static int L_wearingvnum (lua_State *L)
{
CHAR_DATA * ch = L_getchar (L);
OBJ_DATA * obj;
bool found = FALSE;

  int vnum = check_vnum (L);
      
  for (obj = ch->first_carrying ; obj; obj = obj->next_content)
    {
    if (ch == obj->carried_by && 
        obj->wear_loc > WEAR_NONE && 
        obj->pIndexData && obj->pIndexData->vnum == vnum)
       {
       found = TRUE;
       break;
       }  /* end of if carried this vnum */
    }  /* end of for loop */

  lua_pushboolean (L, found);
     
  return 1;  /* the result */
}  /* end of L_wearingvnum */

#define MOB_STR_ITEM(arg) \
if (mob->arg)  \
  {   \
  lua_pushstring (L, mob->arg);  \
  lua_setfield (L, -2, #arg); \
  }
  
#define MOB_NUM_ITEM(arg) \
  lua_pushnumber (L, mob->arg);  \
  lua_setfield (L, -2, #arg)
  
static int L_mob_info (lua_State *L)
  {
  MOB_INDEX_DATA *mob = get_mob_index( check_vnum (L) );
  if (!mob)
    return 0;
    
 lua_newtable(L);  /* table for the info */
  
  /* strings */
     
  MOB_STR_ITEM (player_name);
  MOB_STR_ITEM (short_descr);
  MOB_STR_ITEM (long_descr);
  MOB_STR_ITEM (description);
  //MOB_STR_ITEM (spec_funname);
  
  MOB_NUM_ITEM (vnum);
  MOB_NUM_ITEM (count);
  MOB_NUM_ITEM (killed);
  MOB_NUM_ITEM (sex);
  MOB_NUM_ITEM (level);
  MOB_NUM_ITEM (alignment);
  MOB_NUM_ITEM (mobthac0);
  MOB_NUM_ITEM (ac);
  MOB_NUM_ITEM (hitnodice);
  MOB_NUM_ITEM (hitsizedice);
  MOB_NUM_ITEM (hitplus);
  MOB_NUM_ITEM (damnodice);
  MOB_NUM_ITEM (damsizedice);
  MOB_NUM_ITEM (damplus);
  MOB_NUM_ITEM (numattacks);
  MOB_NUM_ITEM (gold);
  MOB_NUM_ITEM (exp);
  MOB_NUM_ITEM (xflags);
  MOB_NUM_ITEM (immune);
  MOB_NUM_ITEM (resistant);
  MOB_NUM_ITEM (susceptible);
  // MOB_NUM_ITEM (speaks);
  // MOB_NUM_ITEM (speaking);
  MOB_NUM_ITEM (position);
  MOB_NUM_ITEM (defposition);
  MOB_NUM_ITEM (height);
  MOB_NUM_ITEM (weight);
  // MOB_NUM_ITEM (race);
  // MOB_NUM_ITEM (Class);
  MOB_NUM_ITEM (hitroll);
  MOB_NUM_ITEM (damroll);
  MOB_NUM_ITEM (perm_str);
  MOB_NUM_ITEM (perm_int);
  MOB_NUM_ITEM (perm_wis);
  MOB_NUM_ITEM (perm_dex);
  MOB_NUM_ITEM (perm_con);
  MOB_NUM_ITEM (perm_cha);
  MOB_NUM_ITEM (perm_lck);
  MOB_NUM_ITEM (saving_poison_death);
  MOB_NUM_ITEM (saving_wand);
  MOB_NUM_ITEM (saving_para_petri);
  MOB_NUM_ITEM (saving_breath);
  MOB_NUM_ITEM (saving_spell_staff);

  return 1;  /* the table itself */ 
  }  /* end of L_mob_info */

  
static int L_object_info (lua_State *L)
  {
OBJ_INDEX_DATA *obj = get_obj_index ( check_vnum (L) );
int i;

  if (!obj)
    return 0;
    
 lua_newtable(L);  /* table for the info */
  
  /* strings */
     
   OBJ_STR_ITEM (name);
   OBJ_STR_ITEM (short_descr);
   OBJ_STR_ITEM (description);
   OBJ_STR_ITEM (action_desc);

   OBJ_NUM_ITEM (vnum);
   OBJ_NUM_ITEM (level);
   OBJ_NUM_ITEM (item_type);
   OBJ_NUM_ITEM (magic_flags);  
   OBJ_NUM_ITEM (wear_flags);
   OBJ_NUM_ITEM (count);
   OBJ_NUM_ITEM (weight);
   OBJ_NUM_ITEM (cost);
   OBJ_NUM_ITEM (serial);
   OBJ_NUM_ITEM (layers);

  lua_newtable(L);  /* table for the values */
  
  /* do 6 values */
  for (i = 0; i < 6; i++)
    {
    lua_pushnumber (L, obj->value [i]); 
    lua_rawseti (L, -2, i + 1);       
    }
  lua_setfield (L, -2, "value");    

   
  return 1;  /* the table itself */ 
  }  /* end of L_object_info */
  
static int L_object_name (lua_State *L)
  {
OBJ_INDEX_DATA *obj = get_obj_index ( check_vnum (L) );

  if (!obj)
    return 0;  

  lua_pushstring (L, obj->short_descr);
  lua_pushstring (L, obj->description);
  
  return 2;
} /* end of L_object_name */

#define ROOM_STR_ITEM(arg) \
if (room->arg)  \
  {   \
  lua_pushstring (L, room->arg);  \
  lua_setfield (L, -2, #arg); \
  }
  
#define ROOM_NUM_ITEM(arg) \
  lua_pushnumber (L, room->arg);  \
  lua_setfield (L, -2, #arg)
    
#define EXIT_STR_ITEM(arg) \
if (pexit->arg)  \
  {   \
  lua_pushstring (L, pexit->arg);  \
  lua_setfield (L, -2, #arg); \
  }
  
#define  EXIT_NUM_ITEM(arg) \
  lua_pushnumber (L, pexit->arg);  \
  lua_setfield (L, -2, #arg)
        
static int L_room_info (lua_State *L)
{
  CHAR_DATA * ch = L_getchar (L); /* get character pointer */
  ROOM_INDEX_DATA * room = ch->in_room;  /* which room s/he is in */
  EXIT_DATA * pexit;
  
  if (lua_isnumber (L, 1))
    room = get_room_index( check_vnum (L) );
 
  if (room == NULL) 
    return 0;  /* oops - not in room or specified room does not exist */
    
  
  lua_newtable(L);  /* table for the info */
  
  /* strings */
  
  ROOM_STR_ITEM (name);
  ROOM_STR_ITEM (description);
  
  /* numbers */
  
  ROOM_NUM_ITEM (vnum);
  ROOM_NUM_ITEM (room_flags);
  ROOM_NUM_ITEM (light);   
  ROOM_NUM_ITEM (sector_type);
  ROOM_NUM_ITEM (tele_vnum);
  ROOM_NUM_ITEM (tele_delay);
  ROOM_NUM_ITEM (tunnel);  
       
  /* now do the exits */
  
  lua_newtable(L);  /* table for all exits */
  
  for( pexit = room->first_exit; pexit; pexit = pexit->next )
    {
    lua_newtable(L);  /* table for the info */
  
    EXIT_STR_ITEM (keyword);    
    EXIT_STR_ITEM (description);
        
    EXIT_NUM_ITEM (vnum);       
    EXIT_NUM_ITEM (rvnum);      
    EXIT_NUM_ITEM (exit_info);  
    EXIT_NUM_ITEM (key);        
    EXIT_NUM_ITEM (distance);   
    // EXIT_NUM_ITEM (pull);       
    // EXIT_NUM_ITEM (pulltype);   
    
    lua_rawseti (L, -2, pexit->vdir);  /* key is direction */
    }
  lua_setfield (L, -2, "exits");
  
  return 1;  /* the table itself */
}  /* end of L_room_info */

static int L_send_to_char (lua_State *L)
{
  send_to_char(luaL_checkstring (L, 1), L_getchar (L));
  return 0;
}  /* end of L_send_to_char */

static int L_interpret (lua_State *L)
{
  char command [MAX_INPUT_LENGTH];  
  strncpy (command, luaL_checkstring (L, 1), sizeof (command));  
  command [sizeof (command) - 1] = 0;  
  
  interpret (L_getchar (L), command);
  return 0;
}  /* end of L_interpret */

static int L_gain_exp (lua_State *L)
{
  //gain_exp (L_getchar (L), luaL_checknumber (L, 1));
  return 0;
}  /* end of L_gain_exp */

static int L_oinvoke (lua_State *L)
{
CHAR_DATA * ch = L_getchar (L); /* get character pointer */
OBJ_INDEX_DATA *pObjIndex;
OBJ_DATA * obj;
bool taken = 0;

  int vnum = check_vnum (L);
  int level = luaL_optnumber (L, 2, 1);
  if ( level < 1 || level > MAX_LEVEL )
    luaL_error (L, "Level %d is out of range 1 to %d", level, MAX_LEVEL);

  pObjIndex = get_obj_index ( vnum );

  if (!pObjIndex)
   luaL_error (L, "Cannot invoke object vnum %d - it does not exist", vnum);
   
   obj = create_object( pObjIndex, level );
   if( CAN_WEAR( obj, ITEM_TAKE ) )
     {
     obj = obj_to_char( obj, ch );
     taken = TRUE;
   }
   else
      obj = obj_to_room( obj, ch->in_room );
      
  lua_pushboolean (L, taken);
  
  return 1;  /* true if we took the item */
} /* end of L_oinvoke */
   
static int L_mobinworld (lua_State *L)
{
MOB_INDEX_DATA *m_index;
int world_count = 0;
  
  m_index = get_mob_index ( check_vnum (L) );

  if (m_index)
    world_count = m_index->count;
  
  if (world_count)
    lua_pushnumber (L, world_count);
  else
    lua_pushboolean (L, FALSE);
  return 1;
}  /* end of L_mobinworld */

static int L_mobinarea (lua_State *L)
{
CHAR_DATA * ch = L_getchar (L);
CHAR_DATA *tmob;
MOB_INDEX_DATA *m_index;
int world_count = 0;
int found_count = 0;
int result = 0;
int vnum = check_vnum (L);

  m_index = get_mob_index ( vnum );

  if (m_index )
    world_count = m_index->count;
  
  for( tmob = first_char; tmob && found_count != world_count; tmob = tmob->next )
   {
   if( IS_NPC( tmob ) && tmob->pIndexData->vnum == vnum )
     {
      found_count++;
      if( tmob->in_room->area == ch->in_room->area )
         result++;
     }
   }
   
  if (result)
    lua_pushnumber (L, result);
  else
    lua_pushboolean (L, FALSE);
  return 1;
}  /* end of L_mobinarea */

static int L_mobinroom (lua_State *L)
{
CHAR_DATA * ch = L_getchar (L);
CHAR_DATA *tmob;
int result = 0;
  
  int vnum = check_vnum (L);

  for( tmob = ch->in_room->first_person; tmob; tmob = tmob->next_in_room )
   {
   if ( IS_NPC( tmob ) && tmob->pIndexData->vnum == vnum )
         result++;
   }
   
  if (result)
    lua_pushnumber (L, result);
  else
    lua_pushboolean (L, FALSE);
  return 1;
}  /* end of L_mobinroom */

static int L_players_in_room (lua_State *L)
{
  
CHAR_DATA * ch = L_getchar (L); /* get character pointer */
ROOM_INDEX_DATA * room = ch->in_room;  /* which room s/he is in */
 
CHAR_DATA * wch;
int count = 1;
  
 if (lua_isnumber (L, 1))
    room = get_room_index( check_vnum (L) );
 
  if (room == NULL) 
    return 0;  /* oops - not in room or specified room does not exist */
  
  lua_newtable(L);  /* table for the info */
    
  for( wch = room->first_person; wch; wch = wch->next_in_room )
    if( can_see( ch, wch ) && 
       !IS_NPC( wch ))
       {
       lua_pushstring (L, wch->name); 
       lua_rawseti (L, -2, count++);  /* key is count */
       }
   
  return 1;
}  /* end of L_players_in_room */

static int L_players_in_game (lua_State *L)
{
  
CHAR_DATA * ch = L_getchar (L); /* get character pointer */
 
CHAR_DATA * wch;
int count = 1;
 
  lua_newtable(L);  /* table for the info */
    
   for( wch = first_char; wch; wch = wch->next )
      if( can_see( ch, wch ) && 
          !IS_NPC( wch ))
       {
       lua_pushstring (L, wch->name); 
       lua_rawseti (L, -2, count++);  /* key is count */
       }
   
  return 1;
}  /* end of L_players_in_game */

static int L_mobs_in_room (lua_State *L)
{
  
CHAR_DATA * ch = L_getchar (L); /* get character pointer */
ROOM_INDEX_DATA * room = ch->in_room;  /* which room s/he is in */
 
CHAR_DATA * wch;
int count = 1;
  
 if (lua_isnumber (L, 1))
    room = get_room_index( check_vnum (L) );
 
  if (room == NULL) 
    return 0;  /* oops - not in room or specified room does not exist */
  
  lua_newtable(L);  /* table for the info */
    
  for( wch = room->first_person; wch; wch = wch->next_in_room )
    if( can_see( ch, wch ) && 
       IS_NPC( wch ))
       {
       lua_pushnumber (L, wch->pIndexData->vnum); 
       lua_rawseti (L, -2, count++);  /* key is count */
       }
   
  return 1;
}  /* end of L_mobs_in_room */

static const struct luaL_reg mudlib [] = 
  {
  {"system_info", L_system_info},
  {"character_info", L_character_info},
  {"mob_info", L_mob_info},
  {"room_info", L_room_info},
  {"object_info", L_object_info},  /* object prototype */
  {"inventory", L_inventory},      /* invoked objects */
  {"equipped", L_equipped},        /* invoked objects */
  {"object_name", L_object_name},   /* short, long object name */
  {"players_in_room", L_players_in_room},  /* table of players in the room */
  {"players_in_game", L_players_in_game},  /* table of players in the game */
  {"mobs_in_room", L_mobs_in_room},  /* table of mobs in the room */

  {"send_to_char", L_send_to_char},  /* send text to character */
  {"interpret", L_interpret},        /* interpret command, as if we typed it */
    
  /* alter character state */
  
  {"gain_exp", L_gain_exp},
  {"oinvoke", L_oinvoke},
  
  /* if checks */
  
  {"mobinworld", L_mobinworld},
  {"mobinarea", L_mobinarea},
  {"mobinroom", L_mobinroom},
  {"carryingvnum", L_carryingvnum},
  {"wearing", L_wearing},
  {"wearingvnum", L_wearingvnum},
  
   {NULL, NULL}
  };  /* end of mudlib */
     

static int RegisterLuaRoutines (lua_State *L)
  {

  lua_newtable (L);  /* environment */
  lua_replace (L, LUA_ENVIRONINDEX);

  /* this makes environment variable "character.state" by the pointer to our character */
  lua_settable(L, LUA_ENVIRONINDEX);

  /* register all mud.xxx routines */
  luaL_register (L, MUD_LIBRARY, mudlib);
  
  /* using interpret now
  RegisterLuaCommands (L);
  */

  luaopen_bits (L);     /* bit manipulation */

  return 0;
  
}  /* end of RegisterLuaRoutines */

void open_lua  ( CHAR_DATA * ch)
  {
  lua_State *L = luaL_newstate ();   /* opens Lua */
  ch->L = L;
  
  if (ch->L == NULL)
    {
    fprintf( stderr, "Cannot open Lua state\n");
    return;  /* catastrophic failure */
    }

  luaL_openlibs (L);    /* open all standard libraries */

  /* call as Lua function because we need the environment  */
  lua_pushcfunction(L, RegisterLuaRoutines);
  lua_pushstring(L, CHARACTER_STATE);  /* push address */
  lua_pushlightuserdata(L, (void *)ch);    /* push value */
  lua_call(L, 2, 0);
 
  /* run initialiation script */
  if (luaL_loadfile (L, LUA_STARTUP) ||
      CallLuaWithTraceBack (L, 0, 0))
      {
      const char * sError = lua_tostring(L, -1);
      
      fprintf( stderr, "Error loading Lua startup file:\n %s\n", 
              sError);
      
      if (IS_IMMORTAL(ch))
        {
        set_char_color( AT_YELLOW, ch );
        ch_printf (ch, "Error loading Lua startup file:\n %s\n", 
                  sError); 
        }  /* end of immortal */

      }

  lua_settop (L, 0);    /* get rid of stuff lying around */
        
  }  /* end of open_lua */

void close_lua ( CHAR_DATA * ch)
  {
 
  if (ch->L == NULL)
   return;  /* never opened */

  lua_close (ch->L);
  ch->L = NULL;
   
  }/* end of close_lua */

static lua_State * find_lua_function (CHAR_DATA * ch, const char * fname)
  {
  lua_State *L = ch->L;
  
  if (!L)
    return NULL;  /* can't do it */
    
  /* find requested function */
  
  lua_getglobal (L, fname);  
  if (!lua_isfunction (L, -1))
    {
    lua_pop (L, 1);
    fprintf( stderr, "Warning: Lua script function '%s' does not exist\n", fname);
    return NULL;  /* not there */
    }
      
  return L;  
  }
  
static void call_lua_function (CHAR_DATA * ch, 
                                lua_State *L, 
                                const char * fname, 
                                const int nArgs)

{
  
  if (CallLuaWithTraceBack (L, nArgs, 0))
    {
    const char * sError = lua_tostring(L, -1);
     fprintf( stderr, 
       "Error executing Lua function %s:\n %s\n", 
       fname, 
       sError);
  
    if (IS_IMMORTAL(ch))
      {
      set_char_color( AT_YELLOW, ch );
      ch_printf (ch, "Error executing Lua function '%s':\n %s\n", 
         fname, 
         sError); 
    }  /* end of immortal */
    
    lua_pop(L, 1);  /* pop error */
    
    }  /* end of error */
    
}  /* end of call_lua_function */


void call_lua (CHAR_DATA * ch, const char * fname, const char * argument)
  {
    
  int nArgs = 0;
  lua_State *L = find_lua_function (ch, fname);
   
  if (!L)
    return;
  
  /* if they want to send an argument, push it now */  
  if (argument)
    {
    lua_pushstring(L, argument);  /* push argument, if any */
    nArgs++;
    }
  
  call_lua_function (ch, L, fname, nArgs);
    
  }  /* end of call_lua */

void call_lua_num (CHAR_DATA * ch, const char * fname, const int argument)
  {
  lua_State *L = find_lua_function (ch, fname);
 
  if (!L)
    return;
       
  lua_pushnumber (L, argument);  
  
  call_lua_function (ch, L, fname, 1);
    
  }  /* end of call_lua_num */
