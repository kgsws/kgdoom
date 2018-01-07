// [kg] complete rewrite for LUA support

#include "doomdef.h"
#include "info.h"

#include "p_mobj.h"
#include "p_pspr.h"

int numobjtypes;
mobjinfo_t *mobjinfo;

int numstates;
state_t *states;

int numsnames;
sprname_t *sprnames;

int MT_PLAYER;
int MT_TELEPORTMAN;

//
// some engine defaults

const sprname_t info_def_sprnames[NUM_DEF_SPRITES] =
{
	{.u = 0x4e4b4e55} // 'UNKN'
};

const state_t info_def_states[NUM_DEF_STATES] =
{
	{}, // S_NULL
	{.tics = -1, .frame = FF_FULLBRIGHT}, // S_UNKNOWN
	{.nextstate = S_ITEMRESPAWN1}, // S_ITEMRESPAWN0
	{} // S_ITEMRESPAWN1
};

const mobjinfo_t info_def_mobjinfo[NUM_DEF_MOBJTYPES] =
{
	{.spawnstate = S_UNKNOWN, .flags = MF_NOBLOCKMAP | MF_NOGRAVITY} // MT_UNKNOWN
};

