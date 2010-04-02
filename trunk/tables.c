#include <time.h>
#include <stdio.h>
#include <string.h>
#include "mud.h"

#if defined(KEY)
#undef KEY
#endif

#define KEY( literal, field, value )					\
				if ( !str_cmp( word, literal ) )	\
				{					\
				    field  = value;			\
				    fMatch = TRUE;			\
				    break;				\
				}


/* global variables */
int top_sn;

SKILLTYPE *		skill_table	[MAX_SKILL];

char * const skill_tname[] = { "unknown", "Spell", "Skill", "Weapon" };

SPELL_FUN *spell_function( char *name )
{
    if ( !str_cmp( name, "spell_smaug" ))	     return spell_smaug;
    if ( !str_cmp( name, "spell_blindness" ))	     return spell_blindness;
    if ( !str_cmp( name, "spell_charm_person" ))     return spell_charm_person;
    if ( !str_cmp( name, "spell_fireball" ))	     return spell_fireball;
    if ( !str_cmp( name, "spell_invis" ))	     return spell_invis;
    if ( !str_cmp( name, "spell_identify" ))	     return spell_identify;
    if ( !str_cmp( name, "spell_lightning_bolt" ))   return spell_lightning_bolt;
    if ( !str_cmp( name, "spell_poison" ))	     return spell_poison;
    if ( !str_cmp( name, "spell_possess" ))	     return spell_possess;
    if ( !str_cmp( name, "spell_sleep" ))	     return spell_sleep;
    if ( !str_cmp( name, "reserved" ))		     return NULL;
    if ( !str_cmp( name, "spell_null" ))	     return spell_null;
    return spell_notfound;
}

DO_FUN *skill_function( char *name )
{
    switch( name[3] )
    {
    case 'a':
        if ( !str_cmp( name, "do_aaccept" ))            return do_aaccept;
        if ( !str_cmp( name, "do_ahall" ))              return do_ahall;
        if ( !str_cmp( name, "do_arena" ))              return do_arena;
        if ( !str_cmp( name, "do_awho" ))               return do_awho;
        //if ( !str_cmp( name, "do_alia" ))               return do_alia;
        if ( !str_cmp( name, "do_alias" ))               return do_alias;
	if ( !str_cmp( name, "do_adecline" ))		return do_adecline;
	if ( !str_cmp( name, "do_aassign" ))		return do_aassign;
	if ( !str_cmp( name, "do_addpilot" ))  		return do_addpilot;
	if ( !str_cmp( name, "do_accelerate" ))         return do_accelerate;
	if ( !str_cmp( name, "do_advance" ))		return do_advance;
	if ( !str_cmp( name, "do_affected" ))		return do_affected;
	if ( !str_cmp( name, "do_afk" ))		return do_afk;
	if ( !str_cmp( name, "do_aid" ))		return do_aid;
	if ( !str_cmp( name, "do_allow" ))		return do_allow;
	if ( !str_cmp( name, "do_allsave" ))		return do_allsave;
	if ( !str_cmp( name, "do_allships" ))		return do_allships;
	if ( !str_cmp( name, "do_allspeeders" ))	return do_allspeeders;
	if ( !str_cmp( name, "do_ammo" ))		return do_ammo;
	if ( !str_cmp( name, "do_ansi" ))		return do_ansi;
	if ( !str_cmp( name, "do_appraise" ))		return do_appraise;
	if ( !str_cmp( name, "do_appoint" ))		return do_appoint;
	if ( !str_cmp( name, "do_areas" ))		return do_areas;
	if ( !str_cmp( name, "do_arrest" ))		return do_arrest;
	if ( !str_cmp( name, "do_aset" ))		return do_aset;
	if ( !str_cmp( name, "do_astat" ))		return do_astat;
	if ( !str_cmp( name, "do_at" ))			return do_at;
	if ( !str_cmp( name, "do_auction" ))		return do_auction;
	if ( !str_cmp( name, "do_authorize" ))		return do_authorize;
	if ( !str_cmp( name, "do_autorecharge" ))       return do_autorecharge;
	if ( !str_cmp( name, "do_autotrack"))           return do_autotrack;
	if ( !str_cmp( name, "do_autopilot"))           return do_autopilot;
	break;
    case 'b':
    	if ( !str_cmp( name, "do_bet" ))                return do_bet;
	if ( !str_cmp( name, "do_backstab" ))		return do_backstab;
	if ( !str_cmp( name, "do_balzhur" ))		return do_balzhur;
	if ( !str_cmp( name, "do_bamfin" ))		return do_bamfin;
	if ( !str_cmp( name, "do_bamfout" ))		return do_bamfout;
	if ( !str_cmp( name, "do_ban" ))		return do_ban;
	if ( !str_cmp( name, "do_bank" ))		return do_bank;
	if ( !str_cmp( name, "do_bashdoor" ))		return do_bashdoor;
	if ( !str_cmp( name, "do_beep" ))               return do_beep;
	if ( !str_cmp( name, "do_berserk" ))		return do_berserk;
	if ( !str_cmp( name, "do_bestow" ))		return do_bestow;
	if ( !str_cmp( name, "do_bestowarea" ))		return do_bestowarea;
	if ( !str_cmp( name, "do_bio" ))		return do_bio;
	if ( !str_cmp( name, "do_blackjack" ))		return do_blackjack;
	if ( !str_cmp( name, "do_board" ))              return do_board;
	if ( !str_cmp( name, "do_boards" ))		return do_boards;
	if ( !str_cmp( name, "do_bodybag" ))		return do_bodybag;
	if ( !str_cmp( name, "do_bomb" ))		return do_bomb;
	if ( !str_cmp( name, "do_bridge" ))		return do_bridge;
	if ( !str_cmp( name, "do_bset" ))		return do_bset;
	if ( !str_cmp( name, "do_bstat" ))		return do_bstat;
	if ( !str_cmp( name, "do_bug" ))		return do_bug;
	if ( !str_cmp( name, "do_bury" ))		return do_bury;
	if ( !str_cmp( name, "do_buy" ))		return do_buy;
	if ( !str_cmp( name, "do_buyskill" ))		return do_buyskill;
	if ( !str_cmp( name, "do_buyhome" ))            return do_buyhome;
	if ( !str_cmp( name, "do_buyship" ))            return do_buyship;
	break;
    case 'c':
        if ( !str_cmp( name, "do_challenge" ))          return do_challenge;
        if ( !str_cmp( name, "do_chaos" ))              return do_chaos;
	if ( !str_cmp( name, "do_calculate" ))          return do_calculate;
	if ( !str_cmp( name, "do_capture" ))            return do_capture;
	if ( !str_cmp( name, "do_cast" ))		return do_cast;
	if ( !str_cmp( name, "do_cedit" ))		return do_cedit;
	if ( !str_cmp( name, "do_chaff" ))		return do_chaff;
	if ( !str_cmp( name, "do_channels" ))		return do_channels;
	if ( !str_cmp( name, "do_circle" ))		return do_circle;
	if ( !str_cmp( name, "do_clans" ))		return do_clans;
	if ( !str_cmp( name, "do_clan_donate" ))	return do_clan_donate;
	if ( !str_cmp( name, "do_clan_withdraw" ))	return do_clan_withdraw;
	if ( !str_cmp( name, "do_clantalk" ))		return do_clantalk;
	if ( !str_cmp( name, "do_clanbuyship" ))        return do_clanbuyship;
	if ( !str_cmp( name, "do_climb" ))		return do_climb;
	if ( !str_cmp( name, "do_close" ))		return do_close;
	if ( !str_cmp( name, "do_closebay" ))  		return do_closebay;
	if ( !str_cmp( name, "do_closehatch"  ))        return do_closehatch;
	if ( !str_cmp( name, "do_cmdtable" ))		return do_cmdtable;
	if ( !str_cmp( name, "do_codeapp" ))		return do_codeapp;
	if ( !str_cmp( name, "do_codemed" ))		return do_codemed;
	if ( !str_cmp( name, "do_coding" ))		return do_coding;
	if ( !str_cmp( name, "do_commands" ))		return do_commands;
	if ( !str_cmp( name, "do_compare" ))		return do_compare;
	#ifdef MCCP
		if ( !str_cmp( name, "do_compress" ))		return do_compress;
	#endif
	if ( !str_cmp( name, "do_connect" ))		return do_connect;
	if ( !str_cmp( name, "do_config" ))		return do_config;
	if ( !str_cmp( name, "do_consider" ))		return do_consider;
	if ( !str_cmp( name, "do_construction" ))	return do_construction;
	if ( !str_cmp( name, "do_constructportal" ))		return do_constructportal;
	if ( !str_cmp( name, "do_contractpropaganda" ))	return do_contractpropaganda;
	if ( !str_cmp( name, "do_credits" ))		return do_credits;
	if ( !str_cmp( name, "do_cset" ))		return do_cset;
	if ( !str_cmp( name, "do_cyber" ))		return do_cyber;
	if ( !str_cmp( name, "do_cy_rbuy" ))		return do_cy_rbuy;
	if ( !str_cmp( name, "do_cy_rsell" ))		return do_cy_rsell;
	if ( !str_cmp( name, "do_cy_resset" ))		return do_cy_resset;
	break;
    case 'd':
    	if ( !str_cmp( name, "do_decompile" ))		return do_decompile;
    	if ( !str_cmp( name, "do_demote" ))		return do_demote;
	if ( !str_cmp( name, "do_deny" ))		return do_deny;
	if ( !str_cmp( name, "do_description" ))	return do_description;
	if ( !str_cmp( name, "do_designship" ))		return do_designship;
	if ( !str_cmp( name, "do_destro" ))		return do_destro;
	if ( !str_cmp( name, "do_destroy" ))		return do_destroy;
	if ( !str_cmp( name, "do_dig" ))		return do_dig;
	if ( !str_cmp( name, "do_disarm" ))		return do_disarm;
	if ( !str_cmp( name, "do_disconnect" ))		return do_disconnect;
	if ( !str_cmp( name, "do_disguise" ))           return do_disguise;
	if ( !str_cmp( name, "do_dismount" ))		return do_dismount;
	if ( !str_cmp( name, "do_dmesg" ))		return do_dmesg;
	if ( !str_cmp( name, "do_down" ))		return do_down;
	if ( !str_cmp( name, "do_drag" ))		return do_drag;
	if ( !str_cmp( name, "do_drink" ))		return do_drink;
	if ( !str_cmp( name, "do_drive" ))		return do_drive;
	if ( !str_cmp( name, "do_drop" ))		return do_drop;
	break;
    case 'e':
	if ( !str_cmp( name, "do_east" ))		return do_east;
	if ( !str_cmp( name, "do_eat" ))		return do_eat;
	if ( !str_cmp( name, "do_echo" ))		return do_echo;
	if ( !str_cmp( name, "do_email" ))		return do_email;
	if ( !str_cmp( name, "do_emote" ))		return do_emote;
	if ( !str_cmp( name, "do_empty" ))		return do_empty;
	if ( !str_cmp( name, "do_empower" ))		return do_empower;
	if ( !str_cmp( name, "do_enlist" ))		return do_enlist;
	if ( !str_cmp( name, "do_enter" ))		return do_enter;
	if ( !str_cmp( name, "do_equipment" ))		return do_equipment;
	if ( !str_cmp( name, "do_examine" ))		return do_examine;
	if ( !str_cmp( name, "do_examineobject" ))		return do_examineobject;
	if ( !str_cmp( name, "do_exits" ))		return do_exits;
	break;
    case 'f':
	if ( !str_cmp( name, "do_fill" ))		return do_fill;
	if ( !str_cmp( name, "do_fire" ))               return do_fire;
	if ( !str_cmp( name, "do_first_aid" ))          return do_first_aid;
	if ( !str_cmp( name, "do_fixchar" ))		return do_fixchar;
	if ( !str_cmp( name, "do_flee" ))		return do_flee;
	if ( !str_cmp( name, "do_fly" ))		return do_fly;
	if ( !str_cmp( name, "do_foldarea" ))		return do_foldarea;
	if ( !str_cmp( name, "do_follow" ))		return do_follow;
	if ( !str_cmp( name, "do_for" ))		return do_for;
	if ( !str_cmp( name, "do_force" ))		return do_force;
	if ( !str_cmp( name, "do_forget" ))		return do_forget;
	if ( !str_cmp( name, "do_forceclose" ))		return do_forceclose;
	if ( !str_cmp( name, "do_form_password" ))	return do_form_password;
	if ( !str_cmp( name, "do_foundorg" ))	return do_foundorg;
	if ( !str_cmp( name, "do_freeze" ))		return do_freeze;
	break;
    case 'g':

    	if ( !str_cmp( name, "do_gcatchup" ))           return do_gcatchup;
        if ( !str_cmp( name, "do_glist" ))              return do_glist;
        if ( !str_cmp( name, "do_gread" ))              return do_gread;
        if ( !str_cmp( name, "do_gremove" ))            return do_gremove;
        if ( !str_cmp( name, "do_gwrite" ))             return do_gwrite;
	if ( !str_cmp( name, "do_global_boards" ))	return do_global_boards;
	if ( !str_cmp( name, "do_global_note" ))	return do_global_note;

	if ( !str_cmp( name, "do_get" ))		return do_get;
	if ( !str_cmp( name, "do_give" ))		return do_give;
	if ( !str_cmp( name, "do_glance" ))		return do_glance;
        if ( !str_cmp( name, "do_gold" ))               return do_gold;
        if ( !str_cmp( name, "do_gnet" ))               return do_gnet;
	if ( !str_cmp( name, "do_goto" ))		return do_goto;
	if ( !str_cmp( name, "do_gouge" ))		return do_gouge;
	if ( !str_cmp( name, "do_group" ))		return do_group;
	if ( !str_cmp( name, "do_gtell" ))		return do_gtell;
	break;
    case 'h':
	if ( !str_cmp( name, "do_hail" ))		return do_hail;
	if ( !str_cmp( name, "do_hedit" ))		return do_hedit;
	if ( !str_cmp( name, "do_help" ))		return do_help;
	if ( !str_cmp( name, "do_hide" ))		return do_hide;
	if ( !str_cmp( name, "do_hijack" ))		return do_hijack;
	if ( !str_cmp( name, "do_hitall" ))		return do_hitall;
	if ( !str_cmp( name, "do_hlist" ))		return do_hlist;
	if ( !str_cmp( name, "do_holylight" ))		return do_holylight;
	if ( !str_cmp( name, "do_homehall" ))		return do_homehall;
	if ( !str_cmp( name, "do_homerecall" ))		return do_homerecall;
	if ( !str_cmp( name, "do_homestray" ))		return do_homestray;
	if ( !str_cmp( name, "do_homepage" ))		return do_homepage;
	if ( !str_cmp( name, "do_hset" ))		return do_hset;
	if ( !str_cmp( name, "do_hyperspace" ))         return do_hyperspace;
	break;
    case 'i':
	if ( !str_cmp( name, "do_immortalize" ))	return do_immortalize;
	if ( !str_cmp( name, "do_immtalk" ))		return do_immtalk;
	if ( !str_cmp( name, "do_imports"))             return do_imports;
	if ( !str_cmp( name, "do_info" ))               return do_info;
	if ( !str_cmp( name, "do_induct" ))		return do_induct;
	if ( !str_cmp( name, "do_inquire" ))		return do_inquire;
	if ( !str_cmp( name, "do_inventory" ))		return do_inventory;
	if ( !str_cmp( name, "do_invis" ))		return do_invis;
	break;
    case 'j':
        break;
    case 'k':
	if ( !str_cmp( name, "do_kick" ))		return do_kick;
	if ( !str_cmp( name, "do_kill" ))		return do_kill;
	break;
    case 'l':
	if ( !str_cmp( name, "do_last" ))		return do_last;
	if ( !str_cmp( name, "do_land"))                return do_land;
	if ( !str_cmp( name, "do_landscape"))           return do_landscape;
	if ( !str_cmp( name, "do_launch"))              return do_launch;
	if ( !str_cmp( name, "do_layout"))              return do_layout;
	if ( !str_cmp( name, "do_leave" ))		return do_leave;
	if ( !str_cmp( name, "do_leaveship" ))		return do_leaveship;
	if ( !str_cmp( name, "do_level" ))		return do_level;
	if ( !str_cmp( name, "do_list" ))		return do_list;
	if ( !str_cmp( name, "do_load_cargo"))          return do_load_cargo;
	if ( !str_cmp( name, "do_loadup" ))		return do_loadup;
	if ( !str_cmp( name, "do_lock" ))		return do_lock;
	if ( !str_cmp( name, "do_log" ))		return do_log;
	if ( !str_cmp( name, "do_look" ))		return do_look;
	if ( !str_cmp( name, "do_low_purge" ))		return do_low_purge;
	break;
    case 'm':
	if ( !str_cmp( name, "do_mailroom" ))		return do_mailroom;
	if ( !str_cmp( name, "do_make" ))		return do_make;
	if ( !str_cmp( name, "do_makearmor" ))		return do_makearmor;
	if ( !str_cmp( name, "do_makeblade" ))          return do_makeblade;
	if ( !str_cmp( name, "do_makeblaster" ))        return do_makeblaster;
	if ( !str_cmp( name, "do_makelightsaber" ))     return do_makelightsaber;
	if ( !str_cmp( name, "do_makeshield" ))		return do_makeshield;
	if ( !str_cmp( name, "do_makeconstruct" ))	return do_makeconstruct;
	if ( !str_cmp( name, "do_makeconstructupgrade" ))	return do_makeconstructupgrade;
	if ( !str_cmp( name, "do_makecontainer" ))	return do_makecontainer;
	if ( !str_cmp( name, "do_makejewelry" ))	return do_makejewelry;
	if ( !str_cmp( name, "do_makeboard" ))		return do_makeboard;
	if ( !str_cmp( name, "do_makeclan" ))		return do_makeclan;
	if ( !str_cmp( name, "do_makehelpoff" ))		return do_makehelpoff;
	if ( !str_cmp( name, "do_makeship" ))           return do_makeship;
	if ( !str_cmp( name, "do_makerepair" ))		return do_makerepair;
	if ( !str_cmp( name, "do_makeshop" ))		return do_makeshop;
	if ( !str_cmp( name, "do_makeplanet" ))	        return do_makeplanet;
	if ( !str_cmp( name, "do_mapper" ))		return do_mapper;
	if ( !str_cmp( name, "do_mcreate" ))		return do_mcreate;
	if ( !str_cmp( name, "do_memory" ))		return do_memory;
	if ( !str_cmp( name, "do_mfind" ))		return do_mfind;
	if ( !str_cmp( name, "do_minvoke" ))		return do_minvoke;
	if ( !str_cmp( name, "do_mlist" ))		return do_mlist;
	if ( !str_cmp( name, "do_mount" ))		return do_mount;
	if ( !str_cmp( name, "do_mp_close_passage" ))	return do_mp_close_passage;
	if ( !str_cmp( name, "do_mp_damage" ))		return do_mp_damage;
	if ( !str_cmp( name, "do_mp_deposit" ))		return do_mp_deposit;
	if ( !str_cmp( name, "do_mp_offer_agent" ))	return do_mp_offer_agent;
	if ( !str_cmp( name, "do_mp_offer_job" ))	return do_mp_offer_job;
	if ( !str_cmp( name, "do_mp_open_passage" ))	return do_mp_open_passage;
	if ( !str_cmp( name, "do_mp_practice" ))	return do_mp_practice;
	if ( !str_cmp( name, "do_mp_restore" ))		return do_mp_restore;
	if ( !str_cmp( name, "do_mp_slay" ))		return do_mp_slay;
	if ( !str_cmp( name, "do_mp_withdraw" ))	return do_mp_withdraw;
	if ( !str_cmp( name, "do_mpadvance" ))		return do_mpadvance;
	if ( !str_cmp( name, "do_mpapply" ))		return do_mpapply;
	if ( !str_cmp( name, "do_mpapplyb" ))		return do_mpapplyb;
	if ( !str_cmp( name, "do_mpasound" ))		return do_mpasound;
	if ( !str_cmp( name, "do_mpat" ))		return do_mpat;
	if ( !str_cmp( name, "do_mpdream" ))		return do_mpdream;
	if ( !str_cmp( name, "do_mpecho" ))		return do_mpecho;
	if ( !str_cmp( name, "do_mpechoaround" ))	return do_mpechoaround;
	if ( !str_cmp( name, "do_mpechoat" ))		return do_mpechoat;
	if ( !str_cmp( name, "do_mpedit" ))		return do_mpedit;
	if ( !str_cmp( name, "do_mpgain" ))		return do_mpgain;
	if ( !str_cmp( name, "do_mpforce" ))		return do_mpforce;
	if ( !str_cmp( name, "do_mpgoto" ))		return do_mpgoto;
	if ( !str_cmp( name, "do_mpinvis" ))		return do_mpinvis;
	if ( !str_cmp( name, "do_mpjunk" ))		return do_mpjunk;
	if ( !str_cmp( name, "do_mpkill" ))		return do_mpkill;
	if ( !str_cmp( name, "do_mpmload" ))		return do_mpmload;
	if ( !str_cmp( name, "do_mpmset" ))		return do_mpmset;
	if ( !str_cmp( name, "do_mpnothing" ))		return do_mpnothing;
	if ( !str_cmp( name, "do_mpoload" ))		return do_mpoload;
	if ( !str_cmp( name, "do_mppkset" ))		return do_mppkset;
	if ( !str_cmp( name, "do_mppurge" ))		return do_mppurge;
	if ( !str_cmp( name, "do_mpstat" ))		return do_mpstat;
	if ( !str_cmp( name, "do_mptransfer" ))		return do_mptransfer;
	if(  !str_cmp( name, "do_mpquestcomplete" )) return do_mpquestcomplete;
	if(  !str_cmp( name, "do_mpsetquest" )) return do_mpsetquest;
	if ( !str_cmp( name, "do_mset" ))		return do_mset;
	if ( !str_cmp( name, "do_mstat" ))		return do_mstat;
	if ( !str_cmp( name, "do_murde" ))		return do_murde;
	if ( !str_cmp( name, "do_murder" ))		return do_murder;
	if ( !str_cmp( name, "do_mwhere" ))		return do_mwhere;
	break;
    case 'n':
	if ( !str_cmp( name, "do_newbiechat" ))		return do_newbiechat;
	if ( !str_cmp( name, "do_newbieset" ))		return do_newbieset;
	if ( !str_cmp( name, "do_nodeupgrade" ))		return do_nodeupgrade;
	if ( !str_cmp( name, "do_noemote" ))		return do_noemote;
	if ( !str_cmp( name, "do_noresolve" ))		return do_noresolve;
	if ( !str_cmp( name, "do_north" ))		return do_north;
	if ( !str_cmp( name, "do_northeast" ))		return do_northeast;
	if ( !str_cmp( name, "do_northwest" ))		return do_northwest;
	if ( !str_cmp( name, "do_notell" ))		return do_notell;
	if ( !str_cmp( name, "do_notify" ))		return do_notify;
        if ( !str_cmp( name, "do_notitle" ))            return do_notitle;
	if ( !str_cmp( name, "do_noteroom" ))		return do_noteroom;
	break;
    case 'o':
	if ( !str_cmp( name, "do_ocreate" ))		return do_ocreate;
	if ( !str_cmp( name, "do_ofind" ))		return do_ofind;
	if ( !str_cmp( name, "do_oinvoke" ))		return do_oinvoke;
	if ( !str_cmp( name, "do_oldscore" ))		return do_oldscore;
	if ( !str_cmp( name, "do_olist" ))		return do_olist;
	if ( !str_cmp( name, "do_ooc" ))		return do_ooc;
	if ( !str_cmp( name, "do_opedit" ))		return do_opedit;
	if ( !str_cmp( name, "do_open" ))		return do_open;
	if ( !str_cmp( name, "do_openbay" ))  		return do_openbay;
	if ( !str_cmp( name, "do_openhatch" ))          return do_openhatch;
	if ( !str_cmp( name, "do_opstat" ))		return do_opstat;
	if ( !str_cmp( name, "do_order" ))		return do_order;
	if ( !str_cmp( name, "do_oset" ))		return do_oset;
	if ( !str_cmp( name, "do_ostat" ))		return do_ostat;
	if ( !str_cmp( name, "do_outcast" ))		return do_outcast;
	if ( !str_cmp( name, "do_overthrow" ))		return do_overthrow;
	if ( !str_cmp( name, "do_owhere" ))		return do_owhere;
	break;
    case 'p':
	if ( !str_cmp( name, "do_pager" ))		return do_pager;
	if ( !str_cmp( name, "do_password" ))		return do_password;
	if ( !str_cmp( name, "do_peace" ))		return do_peace;
	if ( !str_cmp( name, "do_pick" ))		return do_pick;
	if ( !str_cmp( name, "do_pickshiplock" ))	return do_pickshiplock;
	if ( !str_cmp( name, "do_planets" ))		return do_planets;
	if ( !str_cmp( name, "do_pnet" ))		return do_pnet;
	if ( !str_cmp( name, "do_poison_weapon" ))	return do_poison_weapon;
	if ( !str_cmp( name, "do_postguard" ))		return do_postguard;
	if ( !str_cmp( name, "do_prompt" ))		return do_prompt;
	if ( !str_cmp( name, "do_propaganda" ))		return do_propaganda;
	if ( !str_cmp( name, "do_prototypes" ))		return do_prototypes;
	if ( !str_cmp( name, "do_purge" ))		return do_purge;
	if ( !str_cmp( name, "do_put" ))		return do_put;
	break;
    case 'q':
	if ( !str_cmp( name, "do_quicktalk" ))		return do_quicktalk;
	if ( !str_cmp( name, "do_qui" ))		return do_qui;
	if ( !str_cmp( name, "do_quit" ))		return do_quit;
	break;
    case 'r':
        if ( !str_cmp( name, "do_radar" )) 		return do_radar;
	if ( !str_cmp( name, "do_rank" ))		return do_rank;
	if ( !str_cmp( name, "do_rat" ))		return do_rat;
	if ( !str_cmp( name, "do_reboo" ))		return do_reboo;
	if ( !str_cmp( name, "do_reboot" ))		return do_reboot;
	if ( !str_cmp( name, "do_recharge" ))           return do_recharge;
	if ( !str_cmp( name, "do_recho" ))		return do_recho;
	if ( !str_cmp( name, "do_recall" ))              return do_recall;
        if ( !str_cmp( name, "do_redit" ))              return do_redit;
	if ( !str_cmp( name, "do_reinforcements" ))	return do_reinforcements;
	if ( !str_cmp( name, "do_regoto" ))		return do_regoto;
	if ( !str_cmp( name, "do_remove" ))		return do_remove;
	if ( !str_cmp( name, "do_rempilot" ))           return do_rempilot;
	if ( !str_cmp( name, "do_renamenode" ))           return do_renamenode;
	if ( !str_cmp( name, "do_rentship" ))           return do_rentship;
	if ( !str_cmp( name, "do_repair" ))		return do_repair;
	if ( !str_cmp( name, "do_repairset" ))		return do_repairset;
	if ( !str_cmp( name, "do_repairship" ))         return do_repairship;
	if ( !str_cmp( name, "do_repairshops" ))	return do_repairshops;
	if ( !str_cmp( name, "do_repairstat" ))		return do_repairstat;
	if ( !str_cmp( name, "do_reply" ))		return do_reply;
	if ( !str_cmp( name, "do_report" ))		return do_report;
	if ( !str_cmp( name, "do_rescue" ))		return do_rescue;
	if ( !str_cmp( name, "do_reset" ))		return do_reset;
	if ( !str_cmp( name, "do_resetship" ))		return do_resetship;
	if ( !str_cmp( name, "do_resign" ))		return do_resign;
	if ( !str_cmp( name, "do_resources" ))		return do_resources;
	if ( !str_cmp( name, "do_rest" ))		return do_rest;
	if ( !str_cmp( name, "do_restore" ))		return do_restore;
	if ( !str_cmp( name, "do_restoretime" ))	return do_restoretime;
	if ( !str_cmp( name, "do_restrict" ))		return do_restrict;
	if ( !str_cmp( name, "do_retire" ))		return do_retire;
	if ( !str_cmp( name, "do_retran" ))		return do_retran;
	if ( !str_cmp( name, "do_return" ))		return do_return;
	if ( !str_cmp( name, "do_reverseengineer" ))		return do_reverseengineer;
	if ( !str_cmp( name, "do_revert" ))		return do_revert;
	if ( !str_cmp( name, "do_rlist" ))		return do_rlist;
	if ( !str_cmp( name, "do_roomfix" ))		return do_roomfix;
	if ( !str_cmp( name, "do_rpedit" ))		return do_rpedit;
	if ( !str_cmp( name, "do_rpstat" ))		return do_rpstat;
	if ( !str_cmp( name, "do_rset" ))		return do_rset;
	if ( !str_cmp( name, "do_rstat" ))		return do_rstat;
	break;
    case 's':
	if ( !str_cmp( name, "do_save" ))		return do_save;
	if ( !str_cmp( name, "do_savearea" ))		return do_savearea;
	if ( !str_cmp( name, "do_say" ))		return do_say;
	if ( !str_cmp( name, "do_scan" ))		return do_scan;
	if ( !str_cmp( name, "do_score" ))		return do_score;
	if ( !str_cmp( name, "do_search" ))		return do_search;
	if ( !str_cmp( name, "do_securenode" ))		return do_securenode;
	if ( !str_cmp( name, "do_sedit" ))		return do_sedit;
	if ( !str_cmp( name, "do_sell" ))		return do_sell;
	if ( !str_cmp( name, "do_sellship" ))           return do_sellship;
	if ( !str_cmp( name, "do_setblaster" ))	        return do_setblaster;
	if ( !str_cmp( name, "do_set_boot_time" ))	return do_set_boot_time;
	if ( !str_cmp( name, "do_setclan" ))		return do_setclan;
	if ( !str_cmp( name, "do_setenlist" ))		return do_setenlist;
	if ( !str_cmp( name, "do_setship" ))            return do_setship;
	if ( !str_cmp( name, "do_setstarsystem" ))      return do_setstarsystem;
	if ( !str_cmp( name, "do_setplanet" ))	        return do_setplanet;
	if ( !str_cmp( name, "do_setprototype" ))	return do_setprototype;
	if ( !str_cmp( name, "do_setwages" ))		return do_setwages;
	if ( !str_cmp( name, "do_ships" ))              return do_ships;
	if ( !str_cmp( name, "do_shops" ))		return do_shops;
	if ( !str_cmp( name, "do_shopset" ))		return do_shopset;
	if ( !str_cmp( name, "do_shopstat" ))		return do_shopstat;
	if ( !str_cmp( name, "do_shove" ))		return do_shove;
	if ( !str_cmp( name, "do_showclan" ))		return do_showclan;
	if ( !str_cmp( name, "do_showplanet" ))		return do_showplanet;
	if ( !str_cmp( name, "do_sresources" ))		return do_sresources;
	if ( !str_cmp( name, "do_showprototype" ))	return do_showprototype;
	if ( !str_cmp( name, "do_showship" ))           return do_showship;
	if ( !str_cmp( name, "do_showstarsystem" ))     return do_showstarsystem;
	if ( !str_cmp( name, "do_shutdow" ))		return do_shutdow;
	if ( !str_cmp( name, "do_shutdown" ))		return do_shutdown;
	if ( !str_cmp( name, "do_silence" ))		return do_silence;
	if ( !str_cmp( name, "do_sit" ))		return do_sit;
	if ( !str_cmp( name, "do_skills" ))		return do_skills;
	if ( !str_cmp( name, "do_sla" ))		return do_sla;
	if ( !str_cmp( name, "do_slay" ))		return do_slay;
	if ( !str_cmp( name, "do_sleep" ))		return do_sleep;
	if ( !str_cmp( name, "do_slicebank" ))		return do_slicebank;
	if ( !str_cmp( name, "do_slicefund" ))		return do_slicefund;
	if ( !str_cmp( name, "do_slicesnippets" ))		return do_slicesnippets;
	if ( !str_cmp( name, "do_slice" ))		return do_slice;
	if ( !str_cmp( name, "do_slist" ))		return do_slist;
	if ( !str_cmp( name, "do_slookup" ))		return do_slookup;
	if ( !str_cmp( name, "do_sn_anchor" )) 		return do_sn_anchor;
	if ( !str_cmp( name, "do_sn_audit" )) 		return do_sn_audit;
	if ( !str_cmp( name, "do_sn_checkout" )) 		return do_sn_checkout;
	if ( !str_cmp( name, "do_sn_dropline" )) 		return do_sn_dropline;
	if ( !str_cmp( name, "do_sn_emp" )) 		return do_sn_emp;
	if ( !str_cmp( name, "do_sn_jackhammer" )) 		return do_sn_jackhammer;
	if ( !str_cmp( name, "do_sn_krash" )) 		return do_sn_krash;
	if ( !str_cmp( name, "do_sn_randomizer" )) 		return do_sn_randomizer;
	if ( !str_cmp( name, "do_sn_reconstruct" )) 		return do_sn_reconstruct;
	if ( !str_cmp( name, "do_sn_shortcut" )) 		return do_sn_shortcut;
	if ( !str_cmp( name, "do_sn_spun" )) 		return do_sn_spun;
	if ( !str_cmp( name, "do_sn_uninstall" )) 		return do_sn_uninstall;
	if ( !str_cmp( name, "do_snipe" ))		return do_snipe;
	if ( !str_cmp( name, "do_sneak" ))		return do_sneak;
	if ( !str_cmp( name, "do_snoop" ))		return do_snoop;
	if ( !str_cmp( name, "do_sober" ))		return do_sober;
	if ( !str_cmp( name, "do_socials" ))		return do_socials;
	if ( !str_cmp( name, "do_south" ))		return do_south;
	if ( !str_cmp( name, "do_southeast" ))		return do_southeast;
	if ( !str_cmp( name, "do_southwest" ))		return do_southwest;
	if ( !str_cmp( name, "do_speeders" ))		return do_speeders;
	if ( !str_cmp( name, "do_split" ))		return do_split;
	if ( !str_cmp( name, "do_sset" ))		return do_sset;
	if ( !str_cmp( name, "do_stand" ))		return do_stand;
	if ( !str_cmp( name, "do_starsystems" ))        return do_starsystems;
        if ( !str_cmp( name, "do_status" ))             return do_status;
	if ( !str_cmp( name, "do_steal" ))		return do_steal;
	if ( !str_cmp( name, "do_switch" ))		return do_switch;
	if ( !str_cmp( name, "do_shiptalk" ))		return do_shiptalk;
	if ( !str_cmp( name, "do_sudoku" ))		return do_sudoku;
	if ( !str_cmp( name, "do_suicide" ))		return do_suicide;
	if ( !str_cmp( name, "do_survey" ))		return do_survey;
	if ( !str_cmp( name, "do_systemtalk" ))		return do_systemtalk;
	break;
    case 't':
	if ( !str_cmp( name, "do_target" ))             return do_target;
	if ( !str_cmp( name, "do_teach" ))		return do_teach;
	if ( !str_cmp( name, "do_tell" ))		return do_tell;
	if ( !str_cmp( name, "do_throw" ))		return do_throw;
	if ( !str_cmp( name, "do_time" ))		return do_time;
	if ( !str_cmp( name, "do_timecmd" ))		return do_timecmd;
	if ( !str_cmp( name, "do_title" ))		return do_title;
	if ( !str_cmp( name, "do_torture" ))		return do_torture;
	if ( !str_cmp( name, "do_track" ))		return do_track;
	if ( !str_cmp( name, "do_tractorbeam" ))	return do_tractorbeam;
	if ( !str_cmp( name, "do_tradechat" ))	return do_tradechat;
	if ( !str_cmp( name, "do_trajectory" ))		return do_trajectory;
	if ( !str_cmp( name, "do_transfer" ))		return do_transfer;
	if ( !str_cmp( name, "do_transship" ))		return do_transship;
	if ( !str_cmp( name, "do_trust" ))		return do_trust;
	if ( !str_cmp( name, "do_typo" ))		return do_typo;
	break;
    case 'u':
    	//if ( !str_cmp( name, "do_unalias" ))               return do_unalias;
	if ( !str_cmp( name, "do_unfoldarea" ))		return do_unfoldarea;
	if ( !str_cmp( name, "do_unload_cargo"))        return do_unload_cargo;
	if ( !str_cmp( name, "do_unlock" ))		return do_unlock;
        if ( !str_cmp( name, "do_unsilence" ))          return do_unsilence;
	if ( !str_cmp( name, "do_up" ))			return do_up;
	if ( !str_cmp( name, "do_use" ))		return do_use;
	if ( !str_cmp( name, "do_users" ))		return do_users;
	break;
    case 'v':
	if ( !str_cmp( name, "do_value" ))		return do_value;
	if ( !str_cmp( name, "do_visible" ))		return do_visible;
	if ( !str_cmp( name, "do_vnums" ))		return do_vnums;
	if ( !str_cmp( name, "do_vsearch" ))		return do_vsearch;
	break;
    case 'w':
	if ( !str_cmp( name, "do_war" ))		return do_war;
	if ( !str_cmp( name, "do_wake" ))		return do_wake;
	if ( !str_cmp( name, "do_wchat" ))		return do_wchat;
	if ( !str_cmp( name, "do_wear" ))		return do_wear;
	if ( !str_cmp( name, "do_weather" ))		return do_weather;
	if ( !str_cmp( name, "do_west" ))		return do_west;
	if ( !str_cmp( name, "do_where" ))		return do_where;
	if ( !str_cmp( name, "do_who" ))		return do_who;
	if ( !str_cmp( name, "do_whois" ))		return do_whois;
	if ( !str_cmp( name, "do_wimpy" ))		return do_wimpy;
	if ( !str_cmp( name, "do_wizhelp" ))		return do_wizhelp;
	if ( !str_cmp( name, "do_wizlist" ))		return do_wizlist;
	if ( !str_cmp( name, "do_wizlock" ))		return do_wizlock;
	if ( !str_cmp( name, "do_workmate" ))		return do_workmate;
	break;
    case 'y':
	if ( !str_cmp( name, "do_yell" ))		return do_yell;
	break;
    case 'z':
	if ( !str_cmp( name, "do_zones" ))		return do_zones;
    }
    return skill_notfound;
}

char *spell_name( SPELL_FUN *spell )
{
    if ( spell == spell_smaug )		    return "spell_smaug";
    if ( spell == spell_blindness )	    return "spell_blindness";
    if ( spell == spell_charm_person )	    return "spell_charm_person";
    if ( spell == spell_fireball )	    return "spell_fireball";
    if ( spell == spell_identify )	    return "spell_identify";
    if ( spell == spell_invis )		    return "spell_invis";
    if ( spell == spell_lightning_bolt )    return "spell_lightning_bolt";
    if ( spell == spell_possess )	    return "spell_possess";
    if ( spell == spell_sleep )		    return "spell_sleep";
    if ( spell == spell_null )		    return "spell_null";
    return "reserved";
}

char *skill_name( DO_FUN *skill )
{

	if ( skill == do_cy_rsell)           return "do_cy_rsell";
	if ( skill == do_cy_rbuy)           return "do_cy_rbuy";
	if ( skill == do_cy_resset)           return "do_cy_resset";

    if ( skill == do_imports)           return "do_imports";
    if ( skill == do_load_cargo)        return "do_load_cargo";
    if ( skill == do_unload_cargo )     return "do_unload_cargo";

	//if ( skill == do_alia )           return "do_alia";
	if ( skill == do_alias )           return "do_alias";
	//if ( skill == do_unalias )           return "do_unalias";
	if ( skill == do_contractpropaganda )           return "do_contractpropaganda";
	if ( skill == do_sn_jackhammer )           return "do_sn_jackhammer";
	if ( skill == do_sn_krash )           return "do_sn_krash";
	if ( skill == do_sn_dropline )           return "do_sn_dropline";
	if ( skill == do_sn_randomizer )           return "do_sn_randomizer";
	if ( skill == do_sn_reconstruct )           return "do_sn_reconstruct";
	if ( skill == do_sn_spun )           return "do_sn_spun";
	if ( skill == do_sn_uninstall )           return "do_sn_uninstall";
	if ( skill == do_sn_anchor )           return "do_sn_anchor";
	if ( skill == do_sn_audit )           return "do_sn_audit";
	if ( skill == do_sn_checkout )           return "do_sn_checkout";
	if ( skill == do_sn_emp )           return "do_sn_emp";
	if ( skill == do_sn_shortcut )           return "do_sn_shortcut";
	if ( skill == do_reverseengineer )           return "do_reverseengineer";
    if ( skill == do_aaccept )           return "do_aaccept";
    if ( skill == do_ahall )            return "do_ahall";
    if ( skill == do_arena )            return "do_arena";
    if ( skill == do_awho )             return "do_awho";
    if ( skill == do_bet )              return "do_bet";
    if ( skill == do_challenge )        return "do_challenge";
    if ( skill == do_chaos )            return "do_chaos";
    if ( skill == do_adecline )          return "do_adecline";
    if ( skill == do_slicebank )     return "do_slicebank";
    if ( skill == do_slicefund )     return "do_slicefund";
    if ( skill == do_slicesnippets )     return "do_slicesnippets";
    if ( skill == do_nodeupgrade )     return "do_nodeupgrade";
    if ( skill == do_inquire )     return "do_inquire";
    if ( skill == do_arrest )     return "do_arrest";
    if ( skill == do_buyskill )     return "do_buyskill";
    if ( skill == do_war )     return "do_war";
    if ( skill == do_setenlist )     return "do_setenlist";
    if ( skill == do_connect )     return "do_connect";
    if ( skill == do_constructportal )     return "do_constructportal";
    if ( skill == do_setwages )     return "do_setwages";
    if ( skill == do_prototypes )     return "do_prototypes";
    if ( skill == do_setprototype )     return "do_setprototype";
    if ( skill == do_showprototype )     return "do_showprototype";
    if ( skill == do_rentship )     return "do_rentship";
    if ( skill == do_designship )     return "do_designship";
    if ( skill == do_disarm )     return "do_disarm";
    if ( skill == do_resign )     return "do_resign";
    if ( skill == do_order )     return "do_order";
    if ( skill == do_rat )     return "do_rat";
    if ( skill == do_gtell )     return "do_gtell";
    if ( skill == do_forget )     return "do_forget";
    if ( skill == do_survey )     return "do_survey";
    if ( skill == do_skills )     return "do_skills";
    if ( skill == do_landscape )     return "do_landscape";
    if ( skill == do_construction )     return "do_construction";
    if ( skill == do_allsave )     return "do_allsave";
    if ( skill == do_teach )     return "do_teach";
    if ( skill == do_quicktalk )     return "do_quicktalk";
    if ( skill == do_bridge )     return "do_bridge";
    if ( skill == do_propaganda )     return "do_propaganda";
    if ( skill == do_autopilot )     return "do_autopilot";
    if ( skill == do_allspeeders )     return "do_allspeeders";
    if ( skill == do_speeders )     return "do_speeders";
    if ( skill == do_suicide )     return "do_suicide";
    if ( skill == do_hijack )     return "do_hijack";
    if ( skill == do_pickshiplock )     return "do_pickshiplock";
    if ( skill == do_makejewelry )     return "do_makejewelry";
    if ( skill == do_shiptalk )     return "do_shiptalk";
    if ( skill == do_systemtalk )     return "do_systemtalk";
    if ( skill == do_planets )     return "do_planets";
    if ( skill == do_makeplanet )     return "do_makeplanet";
    if ( skill == do_setplanet )     return "do_setplanet";
    if ( skill == do_hail )     return "do_hail";
    if ( skill == do_homehall )     return "do_homehall";
    if ( skill == do_homerecall )     return "do_homerecall";
    if ( skill == do_homestray )     return "do_homestray";
    if ( skill == do_foundorg )     return "do_foundorg";
    if ( skill == do_appoint )     return "do_appoint";
    if ( skill == do_demote )     return "do_demote";
    if ( skill == do_empower )     return "do_empower";
    if ( skill == do_capture )     return "do_capture";
    if ( skill == do_chaff )     return "do_chaff";
    if ( skill == do_allships )     return "do_allships";
    if ( skill == do_fly )     return "do_fly";
    if ( skill == do_bomb )     return "do_bomb";
    if ( skill == do_bank )     return "do_bank";
    if ( skill == do_drive )     return "do_drive";
    if ( skill == do_setblaster )     return "do_setblaster";
    if ( skill == do_ammo )     return "do_ammo";
    if ( skill == do_use )     return "do_use";
    if ( skill == do_tractorbeam )     return "do_tractorbeam";
    if ( skill == do_makearmor )     return "do_makearmor";
    if ( skill == do_reinforcements )     return "do_reinforcements";
    if ( skill == do_postguard )     return "do_postguard";
    if ( skill == do_torture )     return "do_torture";
    if ( skill == do_throw )     return "do_throw";
    if ( skill == do_snipe )     return "do_snipe";
    if ( skill == do_disguise )     return "do_disguise";
    if ( skill == do_first_aid )     return "do_first_aid";
    if ( skill == do_autotrack )     return "do_autotrack";
    if ( skill == do_overthrow )     return "do_overthrow";
    if ( skill == do_openbay )     return "do_openbay";
    if ( skill == do_closebay )     return "do_openbay";
    if ( skill == do_radar )     return "do_radar";
    if ( skill == do_recall )    return "do_recall";
    if ( skill == do_buyship )		return "do_buyship";
    if ( skill == do_buyhome )		return "do_buyhome";
    if ( skill == do_clanbuyship )		return "do_clanbuyship";
    if ( skill == do_sellship )         return "do_sellship";
    if ( skill == do_info )            return "do_info";
    if ( skill == do_autorecharge )     return "do_autorecharge";
    if ( skill == do_openhatch )         return "do_openhatch";
    if ( skill == do_closehatch )         return "do_closehatch";
    if ( skill == do_hyperspace )         return "do_hyperspace";
    if ( skill == do_target )         return "do_target";
    if ( skill == do_fire )         return "do_fire";
    if ( skill == do_calculate )         return "do_calculate";
    if ( skill == do_recharge )         return "do_recharge";
    if ( skill == do_repairship )         return "do_repairship";
    if ( skill == do_addpilot )         return "do_addpilot";
    if ( skill == do_rempilot )         return "do_rempilot";
    if ( skill == do_aassign )		return "do_aassign";
    if ( skill == do_accelerate )	return "do_accelerate";
    if ( skill == do_advance )		return "do_advance";
    if ( skill == do_affected )		return "do_affected";
    if ( skill == do_afk )		return "do_afk";
    if ( skill == do_aid )		return "do_aid";
    if ( skill == do_allow )		return "do_allow";
    if ( skill == do_ansi )		return "do_ansi";
    if ( skill == do_appraise )		return "do_appraise";
    if ( skill == do_areas )		return "do_areas";
    if ( skill == do_aset )		return "do_aset";
    if ( skill == do_astat )		return "do_astat";
    if ( skill == do_at )		return "do_at";
    if ( skill == do_auction )		return "do_auction";
    if ( skill == do_authorize )	return "do_authorize";
    if ( skill == do_backstab )		return "do_backstab";
    if ( skill == do_balzhur )		return "do_balzhur";
    if ( skill == do_bamfin )		return "do_bamfin";
    if ( skill == do_bamfout )		return "do_bamfout";
    if ( skill == do_ban )		return "do_ban";
    if ( skill == do_bashdoor )		return "do_bashdoor";
    if ( skill == do_beep )             return "do_beep";
    if ( skill == do_berserk )		return "do_berserk";
    if ( skill == do_bestow )		return "do_bestow";
    if ( skill == do_bestowarea )	return "do_bestowarea";
    if ( skill == do_bio )		return "do_bio";
    if ( skill == do_blackjack )		return "do_blackjack";
    if ( skill == do_board )            return "do_board";
    if ( skill == do_boards )		return "do_boards";
    if ( skill == do_bodybag )		return "do_bodybag";
    if ( skill == do_bset )		return "do_bset";
    if ( skill == do_bstat )		return "do_bstat";
    if ( skill == do_bug )		return "do_bug";
    if ( skill == do_bury )		return "do_bury";
    if ( skill == do_buy )		return "do_buy";
    if ( skill == do_cast )		return "do_cast";
    if ( skill == do_cedit )		return "do_cedit";
    if ( skill == do_channels )		return "do_channels";
    if ( skill == do_pnet )		return "do_pnet";
    if ( skill == do_gnet )		return "do_gnet";
    if ( skill == do_ooc )		return "do_ooc";
    if ( skill == do_circle )		return "do_circle";
    if ( skill == do_clans )		return "do_clans";
    if ( skill == do_clan_donate )	return "do_clan_donate";
    if ( skill == do_clan_withdraw )	return "do_clan_withdraw";
    if ( skill == do_clantalk )		return "do_clantalk";
    if ( skill == do_climb )		return "do_climb";
    if ( skill == do_close )		return "do_close";
    if ( skill == do_cmdtable )		return "do_cmdtable";
    if ( skill == do_codeapp )		return "do_codeapp";
    if ( skill == do_workmate )		return "do_workmate";
    if ( skill == do_codemed )		return "do_codemed";
    if ( skill == do_coding )		return "do_coding";
    if ( skill == do_commands )		return "do_commands";
    if ( skill == do_compare )		return "do_compare";
    #ifdef MCCP
        if ( skill == do_compress )		return "do_compress";
    #endif
    if ( skill == do_config )		return "do_config";
    if ( skill == do_consider )		return "do_consider";
    if ( skill == do_credits )		return "do_credits";
    if ( skill == do_cset )		return "do_cset";
    if ( skill == do_cyber )		return "do_cyber";
    if ( skill == do_decompile )		return "do_decompile";
    if ( skill == do_deny )		return "do_deny";
    if ( skill == do_description )	return "do_description";
    if ( skill == do_destro )		return "do_destro";
    if ( skill == do_destroy )		return "do_destroy";
    if ( skill == do_dig )		return "do_dig";
    if ( skill == do_disconnect )	return "do_disconnect";
    if ( skill == do_dismount )		return "do_dismount";
    if ( skill == do_dmesg )		return "do_dmesg";
    if ( skill == do_down )		return "do_down";
    if ( skill == do_drag )		return "do_drag";
    if ( skill == do_drink )		return "do_drink";
    if ( skill == do_drop )		return "do_drop";
/*    if ( skill == do_diagnose )		return "do_diagnose";
*/    if ( skill == do_east )		return "do_east";
    if ( skill == do_eat )		return "do_eat";
    if ( skill == do_echo )		return "do_echo";
    if ( skill == do_email )		return "do_email";
    if ( skill == do_emote )		return "do_emote";
    if ( skill == do_empty )		return "do_empty";
    if ( skill == do_enlist )		return "do_enlist";
    if ( skill == do_enter )		return "do_enter";
    if ( skill == do_equipment )	return "do_equipment";
    if ( skill == do_examine )		return "do_examine";
    if ( skill == do_exits )		return "do_exits";
    if ( skill == do_fill )		return "do_fill";
    if ( skill == do_fixchar )		return "do_fixchar";
    if ( skill == do_flee )		return "do_flee";
    if ( skill == do_foldarea )		return "do_foldarea";
    if ( skill == do_follow )		return "do_follow";
    if ( skill == do_for )		return "do_for";
    if ( skill == do_force )		return "do_force";
    if ( skill == do_forceclose )	return "do_forceclose";
    if ( skill == do_form_password )	return "do_form_password";
    if ( skill == do_freeze )		return "do_freeze";

    if ( skill == do_gcatchup )         return "do_gcatchup";
    if ( skill == do_glist )            return "do_glist";
    if ( skill == do_gread )            return "do_gread";
    if ( skill == do_gremove )          return "do_gremove";
    if ( skill == do_gwrite )           return "do_gwrite";
    if ( skill == do_global_boards )	return "do_global_boards";
    if ( skill == do_global_note )	return "do_global_note";

    if ( skill == do_get )		return "do_get";
    if ( skill == do_give )		return "do_give";
    if ( skill == do_glance )		return "do_glance";
    if ( skill == do_gold )             return "do_gold";
    if ( skill == do_goto )		return "do_goto";
    if ( skill == do_gouge )		return "do_gouge";
    if ( skill == do_group )		return "do_group";
    if ( skill == do_hedit )		return "do_hedit";
    if ( skill == do_help )		return "do_help";
    if ( skill == do_hide )		return "do_hide";
    if ( skill == do_hitall )		return "do_hitall";
    if ( skill == do_hlist )		return "do_hlist";
    if ( skill == do_holylight )	return "do_holylight";
    if ( skill == do_homepage )		return "do_homepage";
    if ( skill == do_hset )		return "do_hset";
    if ( skill == do_immortalize )	return "do_immortalize";
    if ( skill == do_immtalk )		return "do_immtalk";
    if ( skill == do_induct )		return "do_induct";
    if ( skill == do_inventory )	return "do_inventory";
    if ( skill == do_invis )		return "do_invis";
    if ( skill == do_kick )		return "do_kick";
    if ( skill == do_kill )		return "do_kill";
    if ( skill == do_last )		return "do_last";
    if ( skill == do_land )             return "do_land";
    if ( skill == do_launch )		return "do_launch";
    if ( skill == do_layout )		return "do_layout";
    if ( skill == do_leave )		return "do_leave";
    if ( skill == do_leaveship )	return "do_leaveship";
    if ( skill == do_level )		return "do_level";
    if ( skill == do_list )		return "do_list";
    if ( skill == do_loadarea )		return "do_loadarea";
    if ( skill == do_loadup )		return "do_loadup";
    if ( skill == do_lock )		return "do_lock";
    if ( skill == do_log )		return "do_log";
    if ( skill == do_look )		return "do_look";
    if ( skill == do_low_purge )	return "do_low_purge";
    if ( skill == do_mailroom )		return "do_mailroom";
    if ( skill == do_make )		return "do_make";
    if ( skill == do_makeconstruct )	return "do_makeconstruct";
    if ( skill == do_makeconstructupgrade )	return "do_makeconstructupgrade";
    if ( skill == do_makecontainer )	return "do_makecontainer";
    if ( skill == do_makeshield )	return "do_makeshield";
    if ( skill == do_makeblade )	return "do_makeblade";
    if ( skill == do_makeblaster )	return "do_makeblaster";
    if ( skill == do_makelightsaber )	return "do_makelightsaber";
    if ( skill == do_makeboard )	return "do_makeboard";
    if ( skill == do_makeclan )		return "do_makeclan";
    if ( skill == do_makehelpoff )		return "do_makehelpoff";
    if ( skill == do_makerepair )	return "do_makerepair";
    if ( skill == do_makeshop )		return "do_makeshop";
    if ( skill == do_makeship )         return "do_makeship";
    if ( skill == do_mapper )		return "do_mapper";
    if ( skill == do_mcreate )		return "do_mcreate";
    if ( skill == do_memory )		return "do_memory";
    if ( skill == do_mfind )		return "do_mfind";
    if ( skill == do_minvoke )		return "do_minvoke";
    if ( skill == do_mlist )		return "do_mlist";
    if ( skill == do_mount )		return "do_mount";
    if ( skill == do_mp_offer_agent )	return "do_mp_offer_agent";
    if ( skill == do_mp_offer_job )	return "do_mp_offer_job";
    if ( skill == do_mp_close_passage )	return "do_mp_close_passage";
    if ( skill == do_mp_damage )	return "do_mp_damage";
    if ( skill == do_mp_deposit )	return "do_mp_deposit";
    if ( skill == do_mp_open_passage )	return "do_mp_open_passage";
    if ( skill == do_mp_practice )	return "do_mp_practice";
    if ( skill == do_mp_restore )	return "do_mp_restore";
    if ( skill == do_mp_slay )		return "do_mp_slay";
    if ( skill == do_mp_withdraw )	return "do_mp_withdraw";
    if ( skill == do_mpadvance )	return "do_mpadvance";
    if ( skill == do_mpapply )		return "do_mpapply";
    if ( skill == do_mpapplyb )		return "do_mpapplyb";
    if ( skill == do_mpasound )		return "do_mpasound";
    if ( skill == do_mpat )		return "do_mpat";
    if ( skill == do_mpdream )		return "do_mpdream";
    if ( skill == do_mpecho )		return "do_mpecho";
    if ( skill == do_mpechoaround )	return "do_mpechoaround";
    if ( skill == do_mpechoat )		return "do_mpechoat";
    if ( skill == do_mpedit )		return "do_mpedit";
    if ( skill == do_mpgain )		return "do_mpgain";
    if ( skill == do_mpforce )		return "do_mpforce";
    if ( skill == do_mpgoto )		return "do_mpgoto";
    if ( skill == do_mpinvis )		return "do_mpinvis";
    if ( skill == do_mpjunk )		return "do_mpjunk";
    if ( skill == do_mpkill )		return "do_mpkill";
    if ( skill == do_mpmload )		return "do_mpmload";
    if ( skill == do_mpmset )		return "do_mpmset";
    if ( skill == do_mpnothing )	return "do_mpnothing";
    if ( skill == do_mpoload )		return "do_mpoload";
    if ( skill == do_mppkset )		return "do_mppkset";
    if ( skill == do_mppurge )		return "do_mppurge";
    if ( skill == do_mpstat )		return "do_mpstat";
    if ( skill == do_mptransfer )	return "do_mptransfer";
    if ( skill == do_mpquestcomplete )  return "do_mpquestcomplete";
    if ( skill == do_mpsetquest ) 	return "do_mpsetquest";
    if ( skill == do_mset )		return "do_mset";
    if ( skill == do_mstat )		return "do_mstat";
    if ( skill == do_murde )		return "do_murde";
    if ( skill == do_murder )		return "do_murder";
    if ( skill == do_mwhere )		return "do_mwhere";
    if ( skill == do_newbiechat )	return "do_newbiechat";
    if ( skill == do_newbieset )	return "do_newbieset";
    if ( skill == do_noemote )		return "do_noemote";
    if ( skill == do_noresolve )	return "do_noresolve";
    if ( skill == do_north )		return "do_north";
    if ( skill == do_northeast )	return "do_northeast";
    if ( skill == do_northwest )	return "do_northwest";
    if ( skill == do_notell )		return "do_notell";
    if ( skill == do_notify )		return "do_notify";
    if ( skill == do_notitle )          return "do_notitle";
    if ( skill == do_noteroom )		return "do_noteroom";
    if ( skill == do_ocreate )		return "do_ocreate";
    if ( skill == do_ofind )		return "do_ofind";
    if ( skill == do_oinvoke )		return "do_oinvoke";
    if ( skill == do_oldscore )		return "do_oldscore";
    if ( skill == do_olist )		return "do_olist";
    if ( skill == do_opedit )		return "do_opedit";
    if ( skill == do_open )		return "do_open";
    if ( skill == do_opstat )		return "do_opstat";
    if ( skill == do_oset )		return "do_oset";
    if ( skill == do_ostat )		return "do_ostat";
    if ( skill == do_outcast )		return "do_outcast";
    if ( skill == do_owhere )		return "do_owhere";
    if ( skill == do_pager )		return "do_pager";
    if ( skill == do_password )		return "do_password";
    if ( skill == do_peace )		return "do_peace";
    if ( skill == do_pick )		return "do_pick";
    if ( skill == do_poison_weapon )	return "do_poison_weapon";
    if ( skill == do_prompt )		return "do_prompt";
    if ( skill == do_purge )		return "do_purge";
    if ( skill == do_put )		return "do_put";
    if ( skill == do_qui )		return "do_qui";
    if ( skill == do_quit )		return "do_quit";
    if ( skill == do_rank )		return "do_rank";
    if ( skill == do_reboo )		return "do_reboo";
    if ( skill == do_reboot )		return "do_reboot";
    if ( skill == do_recho )		return "do_recho";
    if ( skill == do_redit )		return "do_redit";
    if ( skill == do_regoto )		return "do_regoto";
    if ( skill == do_remove )		return "do_remove";
    if ( skill == do_renamenode )		return "do_renamenode";
    if ( skill == do_repair )		return "do_repair";
    if ( skill == do_repairset )	return "do_repairset";
    if ( skill == do_repairshops )	return "do_repairshops";
    if ( skill == do_repairstat )	return "do_repairstat";
    if ( skill == do_reply )		return "do_reply";
    if ( skill == do_report )		return "do_report";
    if ( skill == do_rescue )		return "do_rescue";
    if ( skill == do_reset )		return "do_reset";
    if ( skill == do_resetship )	return "do_resetship";
    if ( skill == do_resources )	return "do_resources";
    if ( skill == do_rest )		return "do_rest";
    if ( skill == do_restore )		return "do_restore";
    if ( skill == do_restoretime )	return "do_restoretime";
    if ( skill == do_restrict )		return "do_restrict";
    if ( skill == do_retire )		return "do_retire";
    if ( skill == do_retran )		return "do_retran";
    if ( skill == do_return )		return "do_return";
    if ( skill == do_revert )		return "do_revert";
    if ( skill == do_rlist )		return "do_rlist";
    if ( skill == do_roomfix )		return "do_roomfix";
    if ( skill == do_rpedit )		return "do_rpedit";
    if ( skill == do_rpstat )		return "do_rpstat";
    if ( skill == do_rset )		return "do_rset";
    if ( skill == do_rstat )		return "do_rstat";
    if ( skill == do_save )		return "do_save";
    if ( skill == do_savearea )		return "do_savearea";
    if ( skill == do_say )		return "do_say";
    if ( skill == do_scan )		return "do_scan";
    if ( skill == do_score )		return "do_score";
    if ( skill == do_search )		return "do_search";
    if ( skill == do_securenode )		return "do_securenode";
    if ( skill == do_sudoku )		return "do_sudoku";
    if ( skill == do_examineobject )		return "do_examineobject";
    if ( skill == do_sedit )		return "do_sedit";
    if ( skill == do_sell )		return "do_sell";
    if ( skill == do_set_boot_time )	return "do_set_boot_time";
    if ( skill == do_setclan )		return "do_setclan";
    if ( skill == do_setship )          return "do_setship";
    if ( skill == do_setstarsystem )    return "do_setstarsystem";
    if ( skill == do_ships )            return "do_ships";
    if ( skill == do_shops )		return "do_shops";
    if ( skill == do_shopset )		return "do_shopset";
    if ( skill == do_shopstat )		return "do_shopstat";
    if ( skill == do_shove )		return "do_shove";
    if ( skill == do_showclan )		return "do_showclan";
    if ( skill == do_showplanet )		return "do_showplanet";
    if ( skill == do_sresources )		return "do_sresources";
    if ( skill == do_showship )         return "do_showship";
    if ( skill == do_showstarsystem )   return "do_showstarsystem";
    if ( skill == do_shutdow )		return "do_shutdow";
    if ( skill == do_shutdown )		return "do_shutdown";
    if ( skill == do_silence )		return "do_silence";
    if ( skill == do_sit )		return "do_sit";
    if ( skill == do_sla )		return "do_sla";
    if ( skill == do_slay )		return "do_slay";
    if ( skill == do_sleep )		return "do_sleep";
    if ( skill == do_slice )		return "do_slice";
    if ( skill == do_slist )		return "do_slist";
    if ( skill == do_slookup )		return "do_slookup";
    if ( skill == do_sneak )		return "do_sneak";
    if ( skill == do_snoop )		return "do_snoop";
    if ( skill == do_sober )		return "do_sober";
    if ( skill == do_socials )		return "do_socials";
    if ( skill == do_south )		return "do_south";
    if ( skill == do_southeast )	return "do_southeast";
    if ( skill == do_southwest )	return "do_southwest";
    if ( skill == do_split )		return "do_split";
    if ( skill == do_sset )		return "do_sset";
    if ( skill == do_stand )		return "do_stand";
    if ( skill == do_status )		return "do_status";
    if ( skill == do_starsystems )      return "do_starsystems";
    if ( skill == do_steal )		return "do_steal";
    if ( skill == do_switch )		return "do_switch";
    if ( skill == do_tell )		return "do_tell";
    if ( skill == do_time )		return "do_time";
    if ( skill == do_timecmd )		return "do_timecmd";
    if ( skill == do_title )		return "do_title";
    if ( skill == do_track )		return "do_track";
    if ( skill == do_tradechat )		return "do_tradechat";
    if ( skill == do_wchat )		return "do_wchat";
    if ( skill == do_trajectory )	return "do_trajectory";
    if ( skill == do_transfer )		return "do_transfer";
    if ( skill == do_transship )		return "do_transship";
    if ( skill == do_trust )		return "do_trust";
    if ( skill == do_typo )		return "do_typo";
    if ( skill == do_unfoldarea )	return "do_unfoldarea";
    if ( skill == do_unlock )		return "do_unlock";
    if ( skill == do_unsilence )        return "do_unsilence";
    if ( skill == do_up )		return "do_up";
    if ( skill == do_users )		return "do_users";
    if ( skill == do_value )		return "do_value";
    if ( skill == do_visible )		return "do_visible";
    if ( skill == do_vnums )		return "do_vnums";
    if ( skill == do_vsearch )		return "do_vsearch";
    if ( skill == do_wake )		return "do_wake";
    if ( skill == do_wear )		return "do_wear";
    if ( skill == do_weather )		return "do_weather";
    if ( skill == do_west )		return "do_west";
    if ( skill == do_where )		return "do_where";
    if ( skill == do_who )		return "do_who";
    if ( skill == do_whois )		return "do_whois";
    if ( skill == do_wimpy )		return "do_wimpy";
    if ( skill == do_wizhelp )		return "do_wizhelp";
    if ( skill == do_wizlist )		return "do_wizlist";
    if ( skill == do_wizlock )		return "do_wizlock";
    if ( skill == do_yell )		return "do_yell";
    if ( skill == do_zones )		return "do_zones";
    return "reserved";
}

/*
 * Function used by qsort to sort skills
 */
int skill_comp( SKILLTYPE **sk1, SKILLTYPE **sk2 )
{
    SKILLTYPE *skill1 = (*sk1);
    SKILLTYPE *skill2 = (*sk2);

    if ( !skill1 && skill2 )
	return 1;
    if ( skill1 && !skill2 )
	return -1;
    if ( !skill1 && !skill2 )
	return 0;
    if ( skill1->type < skill2->type )
	return -1;
    if ( skill1->type > skill2->type )
	return 1;
    return strcmp( skill1->name, skill2->name );
}

/*
 * Sort the skill table with qsort
 */
void sort_skill_table()
{
    log_string( "Sorting skill table..." );
    qsort( &skill_table[1], top_sn-1, sizeof( SKILLTYPE * ),
		(int(*)(const void *, const void *)) skill_comp );
}


/*
 * Write skill data to a file
 */
void fwrite_skill( FILE *fpout, SKILLTYPE *skill )
{
	SMAUG_AFF *aff;

	fprintf( fpout, "Name         %s~\n",	skill->name	);
	fprintf( fpout, "Type         %s\n",	skill_tname[skill->type]);
	fprintf( fpout, "Flags        %d\n",	skill->flags	);
	if ( skill->target )
	  fprintf( fpout, "Target       %d\n",	skill->target	);
	if ( skill->minimum_position )
	  fprintf( fpout, "Minpos       %d\n",	skill->minimum_position );
	if ( skill->saves )
	  fprintf( fpout, "Saves        %d\n",	skill->saves	);
	if ( skill->slot )
	  fprintf( fpout, "Slot         %d\n",	skill->slot	);
	if ( skill->min_mana )
	  fprintf( fpout, "Mana         %d\n",	skill->min_mana );
	if ( skill->beats )
	  fprintf( fpout, "Rounds       %d\n",	skill->beats	);
	if ( skill->skill_fun )
	  fprintf( fpout, "Code         %s\n",	skill_name(skill->skill_fun) );
	else
	if ( skill->spell_fun )
	  fprintf( fpout, "Code         %s\n",	spell_name(skill->spell_fun) );
	fprintf( fpout, "Dammsg       %s~\n",	skill->noun_damage );
	if ( skill->msg_off && skill->msg_off[0] != '\0' )
	  fprintf( fpout, "Wearoff      %s~\n",	skill->msg_off	);

	if ( skill->hit_char && skill->hit_char[0] != '\0' )
	  fprintf( fpout, "Hitchar      %s~\n",	skill->hit_char );
	if ( skill->hit_vict && skill->hit_vict[0] != '\0' )
	  fprintf( fpout, "Hitvict      %s~\n",	skill->hit_vict );
	if ( skill->hit_room && skill->hit_room[0] != '\0' )
	  fprintf( fpout, "Hitroom      %s~\n",	skill->hit_room );

	if ( skill->miss_char && skill->miss_char[0] != '\0' )
	  fprintf( fpout, "Misschar     %s~\n",	skill->miss_char );
	if ( skill->miss_vict && skill->miss_vict[0] != '\0' )
	  fprintf( fpout, "Missvict     %s~\n",	skill->miss_vict );
	if ( skill->miss_room && skill->miss_room[0] != '\0' )
	  fprintf( fpout, "Missroom     %s~\n",	skill->miss_room );

	if ( skill->die_char && skill->die_char[0] != '\0' )
	  fprintf( fpout, "Diechar      %s~\n",	skill->die_char );
	if ( skill->die_vict && skill->die_vict[0] != '\0' )
	  fprintf( fpout, "Dievict      %s~\n",	skill->die_vict );
	if ( skill->die_room && skill->die_room[0] != '\0' )
	  fprintf( fpout, "Dieroom      %s~\n",	skill->die_room );

	if ( skill->imm_char && skill->imm_char[0] != '\0' )
	  fprintf( fpout, "Immchar      %s~\n",	skill->imm_char );
	if ( skill->imm_vict && skill->imm_vict[0] != '\0' )
	  fprintf( fpout, "Immvict      %s~\n",	skill->imm_vict );
	if ( skill->imm_room && skill->imm_room[0] != '\0' )
	  fprintf( fpout, "Immroom      %s~\n",	skill->imm_room );

	if ( skill->dice && skill->dice[0] != '\0' )
	  fprintf( fpout, "Dice         %s~\n",	skill->dice );
	if ( skill->value )
	  fprintf( fpout, "Value        %d\n",	skill->value );
	if ( skill->difficulty )
	  fprintf( fpout, "Difficulty   %d\n",	skill->difficulty );
	if ( skill->participants )
	  fprintf( fpout, "Participants %d\n",	skill->participants );
	if ( skill->components && skill->components[0] != '\0' )
	  fprintf( fpout, "Components   %s~\n",	skill->components );
	for ( aff = skill->affects; aff; aff = aff->next )
	  fprintf( fpout, "Affect       '%s' %d '%s' %d\n",
	  	aff->duration, aff->location, aff->modifier, aff->bitvector );
	if ( skill->alignment )
	  fprintf( fpout, "Alignment   %d\n",	skill->alignment );

	fprintf( fpout, "End\n\n" );
}

/*
 * Save the skill table to disk
 */
void save_skill_table()
{
    int x;
    FILE *fpout;

    if ( (fpout=fopen( SKILL_FILE, "w" )) == NULL )
    {
	bug( "Cannot open skills.dat for writting", 0 );
	perror( SKILL_FILE );
	return;
    }

    for ( x = 0; x < top_sn; x++ )
    {
	if ( !skill_table[x]->name || skill_table[x]->name[0] == '\0' )
	   break;
	fprintf( fpout, "#SKILL\n" );
	fwrite_skill( fpout, skill_table[x] );
    }
    fprintf( fpout, "#END\n" );
    fclose( fpout );
}


/*
 * Save the socials to disk
 */
void save_socials()
{
    FILE *fpout;
    SOCIALTYPE *social;
    int x;

    if ( (fpout=fopen( SOCIAL_FILE, "w" )) == NULL )
    {
	bug( "Cannot open socials.dat for writting", 0 );
	perror( SOCIAL_FILE );
	return;
    }

    for ( x = 0; x < 27; x++ )
    {
	for ( social = social_index[x]; social; social = social->next )
	{
	    if ( !social->name || social->name[0] == '\0' )
	    {
		bug( "Save_socials: blank social in hash bucket %d", x );
		continue;
	    }
	    fprintf( fpout, "#SOCIAL\n" );
	    fprintf( fpout, "Name        %s~\n",	social->name );
	    if ( social->char_no_arg )
		fprintf( fpout, "CharNoArg   %s~\n",	social->char_no_arg );
	    else
	        bug( "Save_socials: NULL char_no_arg in hash bucket %d", x );
	    if ( social->others_no_arg )
		fprintf( fpout, "OthersNoArg %s~\n",	social->others_no_arg );
	    if ( social->char_found )
		fprintf( fpout, "CharFound   %s~\n",	social->char_found );
	    if ( social->others_found )
		fprintf( fpout, "OthersFound %s~\n",	social->others_found );
	    if ( social->vict_found )
		fprintf( fpout, "VictFound   %s~\n",	social->vict_found );
	    if ( social->char_auto )
		fprintf( fpout, "CharAuto    %s~\n",	social->char_auto );
	    if ( social->others_auto )
		fprintf( fpout, "OthersAuto  %s~\n",	social->others_auto );
	    fprintf( fpout, "End\n\n" );
	}
    }
    fprintf( fpout, "#END\n" );
    fclose( fpout );
}

int get_skill( char *skilltype )
{
    if ( !str_cmp( skilltype, "Spell" ) )
      return SKILL_SPELL;
    if ( !str_cmp( skilltype, "Skill" ) )
      return SKILL_SKILL;
    if ( !str_cmp( skilltype, "Weapon" ) )
      return SKILL_WEAPON;
    return SKILL_UNKNOWN;
}

/*
 * Save the commands to disk
 */
void save_commands()
{
    FILE *fpout;
    CMDTYPE *command;
    int x;

    if ( (fpout=fopen( COMMAND_FILE, "w" )) == NULL )
    {
	bug( "Cannot open commands.dat for writing", 0 );
	perror( COMMAND_FILE );
	return;
    }

    for ( x = 0; x < 126; x++ )
    {
	for ( command = command_hash[x]; command; command = command->next )
	{
	    if ( !command->name || command->name[0] == '\0' )
	    {
		bug( "Save_commands: blank command in hash bucket %d", x );
		continue;
	    }
	    fprintf( fpout, "#COMMAND\n" );
	    fprintf( fpout, "Name        %s~\n", command->name		);
	    fprintf( fpout, "Code        %s\n",	 skill_name(command->do_fun) );
	    fprintf( fpout, "Position    %d\n",	 command->position	);
	    fprintf( fpout, "Level       %d\n",	 command->level		);
	    fprintf( fpout, "Log         %d\n",	 command->log		);
	    fprintf( fpout, "End\n\n" );
	}
    }
    fprintf( fpout, "#END\n" );
    fclose( fpout );
}

SKILLTYPE *fread_skill( FILE *fp )
{
    char buf[MAX_STRING_LENGTH];
    char *word;
    bool fMatch;
    SKILLTYPE *skill;

    CREATE( skill, SKILLTYPE, 1 );

    for ( ; ; )
    {
	word   = feof( fp ) ? "End" : fread_word( fp );
	fMatch = FALSE;

	switch ( UPPER(word[0]) )
	{
	case '*':
	    fMatch = TRUE;
	    fread_to_eol( fp );
	    break;

	case 'A':
	    KEY( "Alignment",	skill->alignment,	fread_number( fp ) );
	    if ( !str_cmp( word, "Affect" ) )
	    {
		SMAUG_AFF *aff;

		CREATE( aff, SMAUG_AFF, 1 );
		aff->duration = str_dup( fread_word( fp ) );
		aff->location = fread_number( fp );
		aff->modifier = str_dup( fread_word( fp ) );
		aff->bitvector = fread_number( fp );
		aff->next = skill->affects;
		skill->affects = aff;
		fMatch = TRUE;
		break;
	    }
	    break;

	case 'C':
	    if ( !str_cmp( word, "Code" ) )
	    {
		SPELL_FUN *spellfun;
		DO_FUN	  *dofun;
		char	  *w = fread_word(fp);

		fMatch = TRUE;
		if ( (spellfun=spell_function(w)) != spell_notfound )
		   skill->spell_fun = spellfun;
		else
		if ( (dofun=skill_function(w)) != skill_notfound )
		   skill->skill_fun = dofun;
		else
		{
		   sprintf( buf, "fread_skill: unknown skill/spell %s", w );
		   bug( buf, 0 );
		   skill->spell_fun = spell_null;
		}
		break;
	    }
	    KEY( "Code",	skill->spell_fun, spell_function(fread_word(fp)) );
	    KEY( "Components",	skill->components,	fread_string_nohash( fp ) );
	    break;

	case 'D':
            KEY( "Dammsg",	skill->noun_damage,	fread_string_nohash( fp ) );
	    KEY( "Dice",	skill->dice,		fread_string_nohash( fp ) );
	    KEY( "Diechar",	skill->die_char,	fread_string_nohash( fp ) );
	    KEY( "Dieroom",	skill->die_room,	fread_string_nohash( fp ) );
	    KEY( "Dievict",	skill->die_vict,	fread_string_nohash( fp ) );
	    KEY( "Difficulty",	skill->difficulty,	fread_number( fp ) );
	    break;

	case 'E':
	    if ( !str_cmp( word, "End" ) )
		return skill;
	    break;

	case 'F':
	    KEY( "Flags",	skill->flags,		fread_number( fp ) );
	    break;

	case 'H':
	    KEY( "Hitchar",	skill->hit_char,	fread_string_nohash( fp ) );
	    KEY( "Hitroom",	skill->hit_room,	fread_string_nohash( fp ) );
	    KEY( "Hitvict",	skill->hit_vict,	fread_string_nohash( fp ) );
	    break;

	case 'I':
	    KEY( "Immchar",	skill->imm_char,	fread_string_nohash( fp ) );
	    KEY( "Immroom",	skill->imm_room,	fread_string_nohash( fp ) );
	    KEY( "Immvict",	skill->imm_vict,	fread_string_nohash( fp ) );
	    break;

	case 'M':
	    KEY( "Mana",	skill->min_mana,	fread_number( fp ) );
	    KEY( "Minpos",	skill->minimum_position, fread_number( fp ) );
	    KEY( "Misschar",	skill->miss_char,	fread_string_nohash( fp ) );
	    KEY( "Missroom",	skill->miss_room,	fread_string_nohash( fp ) );
	    KEY( "Missvict",	skill->miss_vict,	fread_string_nohash( fp ) );
	    break;

	case 'N':
            KEY( "Name",	skill->name,		fread_string_nohash( fp ) );
	    break;

	case 'P':
	    KEY( "Participants",skill->participants,	fread_number( fp ) );
	    break;

	case 'R':
	    KEY( "Rounds",	skill->beats,		fread_number( fp ) );
	    break;

	case 'S':
	    KEY( "Slot",	skill->slot,		fread_number( fp ) );
	    KEY( "Saves",	skill->saves,		fread_number( fp ) );
	    break;

	case 'T':
	    KEY( "Target",	skill->target,		fread_number( fp ) );
	    KEY( "Type",	skill->type,  get_skill(fread_word( fp ))  );
	    break;

	case 'V':
	    KEY( "Value",	skill->value,		fread_number( fp ) );
	    break;

	case 'W':
	    KEY( "Wearoff",	skill->msg_off,		fread_string_nohash( fp ) );
	    break;
	}

	if ( !fMatch )
	{
            sprintf( buf, "Fread_skill: no match: %s", word );
	    bug( buf, 0 );
	}
    }
}

void load_skill_table()
{
    FILE *fp;

    if ( ( fp = fopen( SKILL_FILE, "r" ) ) != NULL )
    {
	top_sn = 0;
	for ( ;; )
	{
	    char letter;
	    char *word;

	    letter = fread_letter( fp );
	    if ( letter == '*' )
	    {
		fread_to_eol( fp );
		continue;
	    }

	    if ( letter != '#' )
	    {
                bug( "Load_skill_table: # not found.", 0 );
		break;
	    }

	    word = fread_word( fp );
            if ( !str_cmp( word, "SKILL"      ) )
	    {
		if ( top_sn >= MAX_SKILL )
		{
		    bug( "load_skill_table: more skills than MAX_SKILL %d", MAX_SKILL );
		    fclose( fp );
		    return;
		}
		skill_table[top_sn++] = fread_skill( fp );
		continue;
	    }
	    else
	    if ( !str_cmp( word, "END"	) )
	        break;
	    else
	    {
                bug( "Load_skill_table: bad section.", 0 );
		continue;
	    }
	}
	fclose( fp );
    }
    else
    {
	bug( "Cannot open skills.dat", 0 );
 	exit(0);
    }
}


void fread_social( FILE *fp )
{
    char buf[MAX_STRING_LENGTH];
    char *word;
    bool fMatch;
    SOCIALTYPE *social;

    CREATE( social, SOCIALTYPE, 1 );

    for ( ;; )
    {
	word   = feof( fp ) ? "End" : fread_word( fp );
	fMatch = FALSE;

	switch ( UPPER(word[0]) )
	{
	case '*':
	    fMatch = TRUE;
	    fread_to_eol( fp );
	    break;

	case 'C':
	    KEY( "CharNoArg",	social->char_no_arg,	fread_string_nohash(fp) );
	    KEY( "CharFound",	social->char_found,	fread_string_nohash(fp) );
	    KEY( "CharAuto",	social->char_auto,	fread_string_nohash(fp) );
	    break;

	case 'E':
	    if ( !str_cmp( word, "End" ) )
	    {
		if ( !social->name )
		{
		    bug( "Fread_social: Name not found", 0 );
		    free_social( social );
		    return;
		}
		if ( !social->char_no_arg )
		{
		    bug( "Fread_social: CharNoArg not found", 0 );
		    free_social( social );
		    return;
		}
		add_social( social );
		return;
	    }
	    break;

	case 'N':
	    KEY( "Name",	social->name,		fread_string_nohash(fp) );
	    break;

	case 'O':
	    KEY( "OthersNoArg",	social->others_no_arg,	fread_string_nohash(fp) );
	    KEY( "OthersFound",	social->others_found,	fread_string_nohash(fp) );
	    KEY( "OthersAuto",	social->others_auto,	fread_string_nohash(fp) );
	    break;

	case 'V':
	    KEY( "VictFound",	social->vict_found,	fread_string_nohash(fp) );
	    break;
	}

	if ( !fMatch )
	{
            sprintf( buf, "Fread_social: no match: %s", word );
	    bug( buf, 0 );
	}
    }
}

void load_socials()
{
    FILE *fp;

    if ( ( fp = fopen( SOCIAL_FILE, "r" ) ) != NULL )
    {
	top_sn = 0;
	for ( ;; )
	{
	    char letter;
	    char *word;

	    letter = fread_letter( fp );
	    if ( letter == '*' )
	    {
		fread_to_eol( fp );
		continue;
	    }

	    if ( letter != '#' )
	    {
                bug( "Load_socials: # not found.", 0 );
		break;
	    }

	    word = fread_word( fp );
            if ( !str_cmp( word, "SOCIAL"      ) )
	    {
                fread_social( fp );
	    	continue;
	    }
	    else
	    if ( !str_cmp( word, "END"	) )
	        break;
	    else
	    {
                bug( "Load_socials: bad section.", 0 );
		continue;
	    }
	}
	fclose( fp );
    }
    else
    {
	bug( "Cannot open socials.dat", 0 );
 	exit(0);
    }
}

void fread_command( FILE *fp )
{
    char buf[MAX_STRING_LENGTH];
    char *word;
    bool fMatch;
    CMDTYPE *command;

    CREATE( command, CMDTYPE, 1 );

    for ( ;; )
    {
	word   = feof( fp ) ? "End" : fread_word( fp );
	fMatch = FALSE;

	switch ( UPPER(word[0]) )
	{
	case '*':
	    fMatch = TRUE;
	    fread_to_eol( fp );
	    break;

	case 'C':
	    KEY( "Code",	command->do_fun,	skill_function(fread_word(fp)) );
	    break;

	case 'E':
	    if ( !str_cmp( word, "End" ) )
	    {
		if ( !command->name )
		{
		    bug( "Fread_command: Name not found", 0 );
		    free_command( command );
		    return;
		}
		if ( !command->do_fun )
		{
		    bug( "Fread_command: Function not found", 0 );
		    free_command( command );
		    return;
		}
		add_command( command );
		return;
	    }
	    break;

	case 'L':
	    KEY( "Level",	command->level,		fread_number(fp) );
	    KEY( "Log",		command->log,		fread_number(fp) );
	    break;

	case 'N':
	    KEY( "Name",	command->name,		fread_string_nohash(fp) );
	    break;

	case 'P':
	    KEY( "Position",	command->position,	fread_number(fp) );
	    break;
	}

	if ( !fMatch )
	{
            sprintf( buf, "Fread_command: no match: %s", word );
	    bug( buf, 0 );
	}
    }
}

void load_commands()
{
    FILE *fp;

    if ( ( fp = fopen( COMMAND_FILE, "r" ) ) != NULL )
    {
	top_sn = 0;
	for ( ;; )
	{
	    char letter;
	    char *word;

	    letter = fread_letter( fp );
	    if ( letter == '*' )
	    {
		fread_to_eol( fp );
		continue;
	    }

	    if ( letter != '#' )
	    {
                bug( "Load_commands: # not found.", 0 );
		break;
	    }

	    word = fread_word( fp );
            if ( !str_cmp( word, "COMMAND"      ) )
	    {
                fread_command( fp );
	    	continue;
	    }
	    else
	    if ( !str_cmp( word, "END"	) )
	        break;
	    else
	    {
                bug( "Load_commands: bad section.", 0 );
		continue;
	    }
	}
	fclose( fp );
    }
    else
    {
	bug( "Cannot open commands.dat", 0 );
 	exit(0);
    }

}

//no need for Neuro
