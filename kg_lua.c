// LUA game scripting
// by kgsws
#include "doomdef.h"
#include "doomstat.h"
#include "i_system.h"
#include "p_local.h"
#include "p_inter.h"
#include "p_pickup.h"
#include "p_generic.h"

#include "w_wad.h"
#include "z_zone.h"

#include "s_sound.h"

#include "m_random.h"

#ifdef LINUX
#include <lua5.3/lua.h>
#include <lua5.3/lauxlib.h>
#include <lua5.3/lualib.h>
#else
#include "lua5.3.4/lua.h"
#include "lua5.3.4/lauxlib.h"
#include "lua5.3.4/lualib.h"
#endif

#include "kg_lua.h"

#ifdef SERVER
#include <netinet/in.h>
#include "network.h"
#include "sv_cmds.h"
#endif

// used to export C functions
typedef struct
{
	char *name;
	int (*func)(lua_State *L);
} luafunc_t;

// used to make 'model' of a C structure
typedef struct lua_table_model_s
{
	const char *name;
	int offset;
	int ltype;
	int (*set)(lua_State*, void*, void*);
	int (*get)(lua_State*, void*, void*);
} lua_table_model_t;

// model for mobj flags
typedef struct
{
	const char *name;
	uint64_t value; // flag combinations are used
} lua_mobjflag_t;

// model for exported state actions
typedef struct
{
	const char *name;
	void (*func)(mobj_t*);
} lua_mobjaction_t;

static lua_State *luaS_game;

static lua_State *luaS_block_iter;
static int block_lua_func;
static int block_lua_arg;
static int block_lua_x0;
static int block_lua_x1;
static int block_lua_y0;
static int block_lua_y1;
boolean PIT_LuaCheckThing(mobj_t* thing);

//
// tables

static int LUA_ThinkerIndex(lua_State *L);

static int LUA_print(lua_State *L);
static int LUA_createMobjType(lua_State *L);
static int LUA_setPlayerType(lua_State *L);
static int LUA_setDoomTeleportType(lua_State *L);
static int LUA_doomRandom(lua_State *L);
static int LUA_spawnMobj(lua_State *L);
static int LUA_blockThingsIterator(lua_State *L);
static int LUA_globalThingsIterator(lua_State *L);
static int LUA_sectorTagIterator(lua_State *L);

static int LUA_removeMobjFromMobj(lua_State *L);
static int LUA_flagCheckMobj(lua_State *L);
static int LUA_teleportMobj(lua_State *L);
static int LUA_checkMobjPos(lua_State *L);
static int LUA_spawnMissile(lua_State *L);
static int LUA_attackAim(lua_State *L);
static int LUA_meleeRange(lua_State *L);
static int LUA_mobjDistance(lua_State *L);
static int LUA_damageFromMobj(lua_State *L);
static int LUA_lineAttack(lua_State *L);
static int LUA_thrustFromMobj(lua_State *L);
static int LUA_sightCheckFromMobj(lua_State *L);
static int LUA_radiusDamageFromMobj(lua_State *L);

static int LUA_soundFromMobj(lua_State *L);

static int LUA_setPlayerWeapon(lua_State *L);

static int LUA_lowFloorFromSector(lua_State *L);
static int LUA_highFloorFromSector(lua_State *L);
static int LUA_lowCeilingFromSector(lua_State *L);
static int LUA_highCeilingFromSector(lua_State *L);
static int LUA_shortTexFromSector(lua_State *L);
static int LUA_genericPlaneFromSector(lua_State *L);

static int LUA_animationFromMobj(lua_State *L);

static int func_set_states(lua_State *L, void *dst, void *o);
static int func_set_fixedt(lua_State *L, void *dst, void *o);
static int func_get_fixedt(lua_State *L, void *dst, void *o);
static int func_set_short(lua_State *L, void *dst, void *o);
static int func_get_short(lua_State *L, void *dst, void *o);

static int func_set_readonly(lua_State *L, void *dst, void *o);
static int func_set_mobjhealth(lua_State *L, void *dst, void *o);
static int func_set_mobjangle(lua_State *L, void *dst, void *o);
static int func_get_mobjangle(lua_State *L, void *dst, void *o);
static int func_set_mobjpitch(lua_State *L, void *dst, void *o);
static int func_set_radius(lua_State *L, void *dst, void *o);
static int func_set_mobj(lua_State *L, void *dst, void *o);
static int func_get_mobj(lua_State *L, void *dst, void *o);
static int func_set_flags(lua_State *L, void *dst, void *o);
static int func_get_sector(lua_State *L, void *dst, void *o);
static int func_get_info(lua_State *L, void *dst, void *o);

static int func_get_removemobj(lua_State *L, void *dst, void *o);
static int func_get_checkflagmobj(lua_State *L, void *dst, void *o);
static int func_get_teleportmobj(lua_State *L, void *dst, void *o);
static int func_get_checkmobjpos(lua_State *L, void *dst, void *o);
static int func_get_spawnmissile(lua_State *L, void *dst, void *o);
static int func_get_attackaim(lua_State *L, void *dst, void *o);
static int func_get_meleerange(lua_State *L, void *dst, void *o);
static int func_get_distance(lua_State *L, void *dst, void *o);
static int func_get_damage(lua_State *L, void *dst, void *o);
static int func_get_lineattack(lua_State *L, void *dst, void *o);
static int func_get_thrustmobj(lua_State *L, void *dst, void *o);
static int func_get_checkmobjsight(lua_State *L, void *dst, void *o);
static int func_get_mobjradiusdmg(lua_State *L, void *dst, void *o);

static int func_get_mobjsound_body(lua_State *L, void *dst, void *o);
static int func_get_mobjsound_weapon(lua_State *L, void *dst, void *o);
static int func_get_mobjsound_pickup(lua_State *L, void *dst, void *o);

static int func_setplayerweapon(lua_State *L, void *dst, void *o);

static int func_get_lowefloorsector(lua_State *L, void *dst, void *o);
static int func_get_highfloorsector(lua_State *L, void *dst, void *o);
static int func_get_loweceilingsector(lua_State *L, void *dst, void *o);
static int func_get_highceilingsector(lua_State *L, void *dst, void *o);
static int func_get_shortexfloorsector(lua_State *L, void *dst, void *o);
static int func_get_genericfloorsector(lua_State *L, void *dst, void *o);
static int func_get_genericceilingsector(lua_State *L, void *dst, void *o);

static int func_set_sectorheight(lua_State *L, void *dst, void *o);
static int func_set_flattexture(lua_State *L, void *dst, void *o);
static int func_get_lumpname(lua_State *L, void *dst, void *o);
static int func_set_lumpname_optional(lua_State *L, void *dst, void *o);

// all exported LUA functions; TODO: split into stages
static const luafunc_t lua_functions[] =
{
	{"print", LUA_print},
	// loading stage
	{"createMobjType", LUA_createMobjType},
	{"setPlayerType", LUA_setPlayerType},
	{"setDoomTeleportType", LUA_setDoomTeleportType},
	{"doomRandom", LUA_doomRandom},
	// map stage
	{"spawnMobj", LUA_spawnMobj},
	{"blockThingsIterator", LUA_blockThingsIterator},
	{"globalThingsIterator", LUA_globalThingsIterator},
	{"sectorTagIterator", LUA_sectorTagIterator},
};

// all mobj flags
static const lua_mobjflag_t lua_mobjflags[] =
{
	{"special", MF_SPECIAL},
	{"solid", MF_SOLID},
	{"shootable", MF_SHOOTABLE},
	{"noSector", MF_NOSECTOR},
	{"noBlockmap", MF_NOBLOCKMAP},
	{"ambush", MF_AMBUSH},
	{"spawnCeiling", MF_SPAWNCEILING},
	{"noGravity", MF_NOGRAVITY},
	{"dropOff", MF_DROPOFF},
	{"pickup", MF_PICKUP},
	{"noClip", MF_NOCLIP},
	{"slide", MF_SLIDE},
	{"float", MF_FLOAT},
	{"missile", MF_MISSILE},
	{"dropped", MF_DROPPED},
	{"shadow", MF_SHADOW},
	{"noblood", MF_NOBLOOD},
	{"corpse", MF_CORPSE},
	{"countKill", MF_COUNTKILL},
	{"countItem", MF_COUNTITEM},
	{"notInDeathmatch", MF_NOTDMATCH},
	{"isMonster", MF_ISMONSTER},
	{"noRadiusDamage", MF_NORADIUSDMG},
	{"holey", MF_HOLEY},
	{"skullFly", MF_SKULLFLY},
	// flag combinations
	{"Monster", MF_ISMONSTER | MF_COUNTKILL | MF_SOLID | MF_SHOOTABLE},
	{"Projectile", MF_MISSILE | MF_NOBLOCKMAP | MF_NOGRAVITY | MF_DROPOFF},
};

// all exported state actions
static const lua_mobjaction_t lua_mobjactions[] =
{
	{"Look", A_Look},
	{"Chase", A_Chase},
	{"Fall", A_Fall},
	{"FaceTarget", A_FaceTarget},

	{"SoundSee", A_SoundSee},
	{"SoundAttack", A_SoundAttack},
	{"SoundPain", A_SoundPain},
	{"SoundActive", A_SoundActive},
	{"SoundDeath", A_SoundDeath},
	{"SoundXDeath", A_SoundXDeath},

	{"WeaponRaise", A_WeaponRaise},
	{"WeaponReady", A_WeaponReady},
	{"WeaponLower", A_WeaponLower},

	{"NoiseAlert", A_NoiseAlert},
};

// all mobj type values
static const lua_table_model_t lua_mobjtype[] =
{
	// numbers
	{"ednum", offsetof(mobjinfo_t, doomednum), LUA_TNUMBER},
	{"health", offsetof(mobjinfo_t, spawnhealth), LUA_TNUMBER},
	{"reactionTime", offsetof(mobjinfo_t, reactiontime), LUA_TNUMBER},
	{"painChance", offsetof(mobjinfo_t, painchance), LUA_TNUMBER},
	{"speed", offsetof(mobjinfo_t, speed), LUA_TNUMBER, func_set_fixedt, func_get_fixedt},
	{"radius", offsetof(mobjinfo_t, radius), LUA_TNUMBER, func_set_fixedt, func_get_fixedt},
	{"height", offsetof(mobjinfo_t, height), LUA_TNUMBER, func_set_fixedt, func_get_fixedt},
	{"mass", offsetof(mobjinfo_t, mass), LUA_TNUMBER},
	{"damage", offsetof(mobjinfo_t, damage), LUA_TNUMBER},
	{"viewz", offsetof(mobjinfo_t, viewz), LUA_TNUMBER, func_set_fixedt, func_get_fixedt},
	{"shootz", offsetof(mobjinfo_t, shootz), LUA_TNUMBER, func_set_fixedt, func_get_fixedt},
	{"bobz", offsetof(mobjinfo_t, bobz), LUA_TNUMBER, func_set_fixedt, func_get_fixedt},
	{"species", offsetof(mobjinfo_t, species), LUA_TNUMBER, func_set_fixedt, func_get_fixedt},
	{"flags", offsetof(mobjinfo_t, flags), LUA_TNUMBER},
	// states; keep same order as in 'mobjinfo_t'
	{"_spawn", offsetof(mobjinfo_t, spawnstate), LUA_TTABLE, func_set_states},
	{"_see", offsetof(mobjinfo_t, seestate), LUA_TTABLE, func_set_states},
	{"_pain", offsetof(mobjinfo_t, painstate), LUA_TTABLE, func_set_states},
	{"_melee", offsetof(mobjinfo_t, meleestate), LUA_TTABLE, func_set_states},
	{"_missile", offsetof(mobjinfo_t, missilestate), LUA_TTABLE, func_set_states},
	{"_death", offsetof(mobjinfo_t, deathstate), LUA_TTABLE, func_set_states},
	{"_xdeath", offsetof(mobjinfo_t, xdeathstate), LUA_TTABLE, func_set_states},
	{"_raise", offsetof(mobjinfo_t, raisestate), LUA_TTABLE, func_set_states},
	{"_crush", offsetof(mobjinfo_t, crushstate), LUA_TTABLE, func_set_states},
	{"_heal", offsetof(mobjinfo_t, healstate), LUA_TTABLE, func_set_states},
	{"_wRaise", offsetof(mobjinfo_t, weaponraise), LUA_TTABLE, func_set_states},
	{"_wReady", offsetof(mobjinfo_t, weaponready), LUA_TTABLE, func_set_states},
	{"_wLower", offsetof(mobjinfo_t, weaponlower), LUA_TTABLE, func_set_states},
	{"_wFireMain", offsetof(mobjinfo_t, weaponfiremain), LUA_TTABLE, func_set_states},
	{"_wFireAlt", offsetof(mobjinfo_t, weaponfirealt), LUA_TTABLE, func_set_states},
	{"_wFlashMain", offsetof(mobjinfo_t, weaponflashmain), LUA_TTABLE, func_set_states},
	{"_wFlashAlt", offsetof(mobjinfo_t, weaponflashalt), LUA_TTABLE, func_set_states},
	// sounds
	{"seeSound", offsetof(mobjinfo_t, seesound), LUA_TSTRING, func_set_lumpname_optional, func_get_lumpname},
	{"attackSound", offsetof(mobjinfo_t, attacksound), LUA_TSTRING, func_set_lumpname_optional, func_get_lumpname},
	{"painSound", offsetof(mobjinfo_t, painsound), LUA_TSTRING, func_set_lumpname_optional, func_get_lumpname},
	{"activeSound", offsetof(mobjinfo_t, activesound), LUA_TSTRING, func_set_lumpname_optional, func_get_lumpname},
	{"deathSound", offsetof(mobjinfo_t, deathsound), LUA_TSTRING, func_set_lumpname_optional, func_get_lumpname},
	{"xdeathSound", offsetof(mobjinfo_t, xdeathsound), LUA_TSTRING, func_set_lumpname_optional, func_get_lumpname},
};

// all mobj values
static const lua_table_model_t lua_mobj[] =
{
	{"health", offsetof(mobj_t, health), LUA_TNUMBER, func_set_mobjhealth},
	{"angle", offsetof(mobj_t, angle), LUA_TNUMBER, func_set_mobjangle, func_get_mobjangle},
	{"pitch", offsetof(mobj_t, pitch), LUA_TNUMBER, func_set_mobjpitch, func_get_fixedt},
	{"floorz", offsetof(mobj_t, floorz), LUA_TNUMBER, func_set_fixedt, func_get_fixedt},
	{"ceilingz", offsetof(mobj_t, ceilingz), LUA_TNUMBER, func_set_fixedt, func_get_fixedt},
	{"radius", offsetof(mobj_t, radius), LUA_TNUMBER, func_set_radius, func_get_fixedt},
	{"height", offsetof(mobj_t, height), LUA_TNUMBER, func_set_fixedt, func_get_fixedt},
	{"momx", offsetof(mobj_t, momx), LUA_TNUMBER, func_set_fixedt, func_get_fixedt},
	{"momy", offsetof(mobj_t, momy), LUA_TNUMBER, func_set_fixedt, func_get_fixedt},
	{"momz", offsetof(mobj_t, momz), LUA_TNUMBER, func_set_fixedt, func_get_fixedt},
	{"flags", offsetof(mobj_t, flags), LUA_TNUMBER, func_set_flags},
	{"movedir", offsetof(mobj_t, movedir), LUA_TNUMBER},
	{"movecount", offsetof(mobj_t, movecount), LUA_TNUMBER},
	{"target", offsetof(mobj_t, target), LUA_TLIGHTUSERDATA, func_set_mobj, func_get_mobj},
	{"source", offsetof(mobj_t, source), LUA_TLIGHTUSERDATA, func_set_mobj, func_get_mobj},
	{"attacker", offsetof(mobj_t, attacker), LUA_TLIGHTUSERDATA, func_set_mobj, func_get_mobj},
	{"mobj", offsetof(mobj_t, mobj), LUA_TLIGHTUSERDATA, func_set_mobj, func_get_mobj},
	{"reactiontime", offsetof(mobj_t, reactiontime), LUA_TNUMBER},
	{"threshold", offsetof(mobj_t, threshold), LUA_TNUMBER},
	{"tics", offsetof(mobj_t, tics), LUA_TNUMBER},
	// read only
	{"x", offsetof(mobj_t, x), LUA_TNUMBER, func_set_readonly, func_get_fixedt},
	{"y", offsetof(mobj_t, y), LUA_TNUMBER, func_set_readonly, func_get_fixedt},
	{"z", offsetof(mobj_t, z), LUA_TNUMBER, func_set_readonly, func_get_fixedt},
	// special stuff
	// TODO: player
	{"sector", 0, LUA_TLIGHTUSERDATA, func_set_readonly, func_get_sector},
	{"info", 0, LUA_TLIGHTUSERDATA, func_set_readonly, func_get_info},
	// functions
	{"Remove", 0, LUA_TFUNCTION, func_set_readonly, func_get_removemobj},
	{"Flag", 0, LUA_TFUNCTION, func_set_readonly, func_get_checkflagmobj},
	{"Teleport", 0, LUA_TFUNCTION, func_set_readonly, func_get_teleportmobj},
	{"CheckPosition", 0, LUA_TFUNCTION, func_set_readonly, func_get_checkmobjpos},
	{"SpawnMissile", 0, LUA_TFUNCTION, func_set_readonly, func_get_spawnmissile},
	{"AttackAim", 0, LUA_TFUNCTION, func_set_readonly, func_get_attackaim},
	{"MeleeRange", 0, LUA_TFUNCTION, func_set_readonly, func_get_meleerange},
	{"Distance", 0, LUA_TFUNCTION, func_set_readonly, func_get_distance},
	{"Damage", 0, LUA_TFUNCTION, func_set_readonly, func_get_damage},
	{"LineAttack", 0, LUA_TFUNCTION, func_set_readonly, func_get_lineattack},
	{"Thrust", 0, LUA_TFUNCTION, func_set_readonly, func_get_thrustmobj},
	{"CheckSight", 0, LUA_TFUNCTION, func_set_readonly, func_get_checkmobjsight},
	{"RadiusDamage", 0, LUA_TFUNCTION, func_set_readonly, func_get_mobjradiusdmg},
	{"SoundBody", 0, LUA_TFUNCTION, func_set_readonly, func_get_mobjsound_body},
	{"SoundWeapon", 0, LUA_TFUNCTION, func_set_readonly, func_get_mobjsound_weapon},
	{"SoundPickup", 0, LUA_TFUNCTION, func_set_readonly, func_get_mobjsound_pickup},
};

// all player values
static const lua_table_model_t lua_player[] =
{
	{"mo", offsetof(player_t, mo), LUA_TLIGHTUSERDATA, func_set_mobj, func_get_mobj},
	// functions
	{"setWeapon", 0, LUA_TLIGHTUSERDATA, func_set_readonly, func_setplayerweapon},
};

// all sector values
static const lua_table_model_t lua_sector[] =
{
	// soundorg?
	{"floorheight", offsetof(sector_t, floorheight), LUA_TNUMBER, func_set_sectorheight, func_get_fixedt},
	{"ceilingheight", offsetof(sector_t, ceilingheight), LUA_TNUMBER, func_set_sectorheight, func_get_fixedt},
	{"floorpic", offsetof(sector_t, floorpic), LUA_TSTRING, func_set_flattexture, func_get_lumpname},
	{"ceilingpic", offsetof(sector_t, ceilingpic), LUA_TSTRING, func_set_flattexture, func_get_lumpname},
	{"lightlevel", offsetof(sector_t, lightlevel), LUA_TNUMBER, func_set_short, func_get_short},
	{"special", offsetof(sector_t, special), LUA_TNUMBER, func_set_short, func_get_short},
	{"tag", offsetof(sector_t, tag), LUA_TNUMBER, func_set_short, func_get_short},
	// functions
	{"FindLowestFloor", 0, LUA_TFUNCTION, func_set_readonly, func_get_lowefloorsector},
	{"FindHighestFloor", 0, LUA_TFUNCTION, func_set_readonly, func_get_highfloorsector},
	{"FindLowestCeiling", 0, LUA_TFUNCTION, func_set_readonly, func_get_loweceilingsector},
	{"FindHighestCeiling", 0, LUA_TFUNCTION, func_set_readonly, func_get_highceilingsector},
	{"GetShortestTexture", 0, LUA_TFUNCTION, func_set_readonly, func_get_shortexfloorsector},
	{"GenericFloor", 0, LUA_TFUNCTION, func_set_readonly, func_get_genericfloorsector},
	{"GenericCeiling", 0, LUA_TFUNCTION, func_set_readonly, func_get_genericceilingsector},
};

//
// debug

static void LUA_DumpTable(lua_State *L, int depth)
{
	char temp[depth*2+1];

	if(depth)
		memset(temp, ' ', depth*2);
	temp[depth*2] = 0;
	// assuming table is on top of the stack
	lua_pushnil(L);
	while(lua_next(L, -2) != 0)
	{
		// key
		switch(lua_type(L, -2))
		{
			case LUA_TSTRING:
				printf("%s%s:", temp, lua_tostring(L, -2));
			break;
			case LUA_TNUMBER:
				printf("%s[%i]:", temp, (int)lua_tonumber(L, -2));
			break;
			default:
				printf("%s%s:", temp, lua_typename(L, lua_type(L, -2)));
			break;
		}
		// value
		switch(lua_type(L, -1))
		{
			case LUA_TSTRING:
				printf(" %s\n", lua_tostring(L, -1));
			break;
			case LUA_TNUMBER:
				printf(" %i\n", (int)lua_tonumber(L, -1));
			break;
			default:
				printf("\n");
				LUA_DumpTable(L, depth+1);
			break;
		}
		lua_pop(L, 1);
	}
}

//
// mobj states

static int state_mobjt; // for final fixup
static int state_state; // for 'loop'

static int state_getSprite(const char *spr)
{
	int i;

	// search for this name
	for(i = 0; i < numsnames; i++)
	{
		if(sprnames[i].u == *(const uint32_t*)spr)
			return i;
	}
	// add this name
	if((numsnames+1) / INFO_SPRITE_ALLOC > numsnames / INFO_SPRITE_ALLOC)
	{
		int asize = (numsnames + INFO_SPRITE_ALLOC) * sizeof(sprname_t);
		// realloc buffer
		sprname_t *temp = realloc(sprnames, asize);
		if(!temp)
			I_Error("out of memory\n");
		sprnames = temp;
	}
	sprnames[numsnames].u = *(const uint32_t*)spr;
	numsnames++;
	// done
	return numsnames - 1;
}

static int state_add(lua_State *L)
{
	int len;
	state_t st;
	const char *sf;
	int num;

	len = lua_rawlen(L, -1);
	if(len <= 0)
		return 0;

	memset(&st, 0, sizeof(state_t));
	st.func = LUA_REFNIL;

	//
	// sprite + frame

	lua_pushinteger(L, 1);
	lua_gettable(L, -2);
	if(lua_type(L, -1) != LUA_TSTRING)
		return luaL_error(L, "invalid sprite / frame");
	sf = lua_tostring(L, -1);
	lua_pop(L, 1);
	// fullbright?
	if(sf[0] == '*')
	{
		st.frame = FF_FULLBRIGHT;
		sf++;
	}
	// check
	if(strlen(sf) < 5)
		return luaL_error(L, "invalid sprite / frame");
	// parse sprite
	st.sprite = state_getSprite(sf);
	// parse frame
	if(sf[4] < 'A' || sf[4] > '_')
		return luaL_error(L, "invalid sprite frame");
	st.frame |= sf[4] - 'A';

	//
	// tics

	if(len < 2)
		goto finish;

	lua_pushinteger(L, 2);
	lua_gettable(L, -2);
	if(lua_type(L, -1) != LUA_TNUMBER)
		return luaL_error(L, "invalid state duration");
	st.tics = lua_tointeger(L, -1);
	lua_pop(L, 1);

	//
	// function

	if(len < 3)
		goto finish;

	lua_pushinteger(L, 3);
	lua_gettable(L, -2);
	switch(lua_type(L, -1))
	{
		case LUA_TNIL:
			lua_pop(L, 1);
		break;
		case LUA_TFUNCTION:
			st.func = luaL_ref(L, LUA_REGISTRYINDEX);
		break;
		default:
			return luaL_error(L, "invalid state function");
		break;
	}

	//
	// done
finish:
	// set next frame
	st.nextstate = numstates + 1;
	// add new state
	if((numstates+1) / INFO_STATE_ALLOC > numstates / INFO_STATE_ALLOC)
	{
		int asize = (numstates + INFO_STATE_ALLOC) * sizeof(state_t);
		// realloc buffer
		state_t *temp = realloc(states, asize);
		if(!temp)
			I_Error("out of memory\n");
		states = temp;
	}
	memcpy(states + numstates, &st, sizeof(state_t));
	numstates++;

	return 0;
}

void L_StateCall(state_t *st, mobj_t *mo)
{
	if(st->func == LUA_REFNIL)
		return;
	// function to call
	lua_rawgeti(luaS_game, LUA_REGISTRYINDEX, st->func);
	// mobj to pass
	lua_pushlightuserdata(luaS_game, mo);
	// do the call
	if(lua_pcall(luaS_game, 1, 0, 0))
		// script error
		I_Error("L_StateCall: %s", lua_tostring(luaS_game, -1));
}

statenum_t L_StateFromAlias(mobjinfo_t *info, statenum_t state)
{
	int anim;
	int offs;

	// animation ID
	anim = state & STATE_AMASK;
	offs = (state & STATE_OMASK) >> STATE_OFFSHIFT;
	// set this animation
	return *(&info->spawnstate + anim) + offs;
}

//
// mobj types

static int func_set_fixedt(lua_State *L, void *dst, void *o)
{
	lua_Number num;

	num = lua_tonumber(L, -1);
	*(int*)dst = (int)(num * (lua_Number)FRACUNIT);
	return 0;
}

static int func_get_fixedt(lua_State *L, void *dst, void *o)
{
	lua_pushnumber(L, *(fixed_t*)dst / (lua_Number)FRACUNIT);
	return 1;
}

static int func_set_short(lua_State *L, void *dst, void *o)
{
	*(short*)dst = lua_tointeger(L, -1);
	return 0;
}

static int func_get_short(lua_State *L, void *dst, void *o)
{
	lua_pushinteger(L, *(short*)dst);
	return 1;
}

static int func_set_states(lua_State *L, void *dst, void *o)
{
	size_t i, len;
	const char *jump;
	int finished = 0;

	state_state = numstates;
	len = lua_rawlen(L, -1);

	for(i = 1; i <= len && !finished; i++)
	{
		lua_pushinteger(L, i);
		lua_gettable(L, -2);
		switch(lua_type(L, -1))
		{
			case LUA_TTABLE:
				state_add(L);
			break;
			case LUA_TSTRING:
				jump = lua_tostring(L, -1);
				if(jump[0] == '_')
				{
					int j;
					int k = 0; // k is animation ID
					int offs = 0;
					char temp[16];
					char *ptr = temp;

					// check for offset
					strncpy(temp, jump, sizeof(temp)-1);
					temp[sizeof(temp)-1] = 0;

					ptr = temp;
					while(*ptr)
					{
						if(*ptr == '+')
						{
							*ptr = 0;
							if(sscanf(ptr + 1, "%d", &offs) != 1)
								offs = 0;
							break;
						}
						ptr++;
					}

					// find this animation
					for(j = 0; j < sizeof(lua_mobjtype)/sizeof(lua_table_model_t); j++)
					{
						if(lua_mobjtype[j].name[0] == '_')
						{
							if(!strcmp(temp, lua_mobjtype[j].name))
							{
								// set next state 'alias' with optional offset
								finished = 1;
								states[numstates-1].nextstate = STATE_ANIMATION | k | (offs << STATE_OFFSHIFT);
								break;
							}
							k++;
						}
					}
					if(finished)
						break;
				}
				if(!strcmp(jump, "loop") && state_state != numstates)
				{
					finished = 1;
					states[numstates-1].nextstate = state_state;
					break;
				}
				luaL_error(L, "invalid type for state");
			break;
			default:
				luaL_error(L, "invalid type for state");
			break;
		}
		lua_pop(L, 1);
	}

	if(state_state == numstates)
		return 0;

	if(!finished)
		states[numstates-1].nextstate = 0;

	*(int*)dst = state_state;

	return 0;
}

//
// mobj set/get functions

// read-only value
static int func_set_readonly(lua_State *L, void *dst, void *o)
{
	return luaL_error(L, "tried to modify read-only value");
}

// health has to be synchronized with player
static int func_set_mobjhealth(lua_State *L, void *dst, void *o)
{
	mobj_t *mo = o;

	*(int*)dst = lua_tointeger(L, -1);
	if(mo->player)
		mo->player->health = mo->health;
	return 0;
}

// special decimal handling for angles
static int func_set_mobjangle(lua_State *L, void *dst, void *o)
{
	*(angle_t*)dst = lua_tonumber(L, -1) * (lua_Number)(1 << ANGLETOFINESHIFT);
	return 0;
}

static int func_get_mobjangle(lua_State *L, void *dst, void *o)
{
	lua_pushnumber(L, *(angle_t*)dst / (lua_Number)(1 << ANGLETOFINESHIFT));
	return 1;
}

// pitch values have to be in certain range and in fixed_t format
static int func_set_mobjpitch(lua_State *L, void *dst, void *o)
{
	fixed_t pitch = lua_tonumber(L, -1) * (lua_Number)FRACUNIT;

	if(pitch > 50000)
	    pitch = 50000;
	if(pitch < -50000)
	    pitch = -50000;

	*(int*)dst = pitch;
	return 0;
}

// radius changes have to update blockmap links
static int func_set_radius(lua_State *L, void *dst, void *o)
{
	mobj_t *mo = o;
	int radius = lua_tonumber(L, -1) * (lua_Number)FRACUNIT;

	P_UnsetThingPosition(mo);
	*(fixed_t*)dst = radius;
	P_SetThingPosition(mo);

	return 0;
}

// special mobj type
static int func_set_mobj(lua_State *L, void *dst, void *o)
{
	thinker_t *th;

	th = lua_touserdata(L, -1);
	if(th && th->lua_type != TT_MOBJ)
		return luaL_error(L, "invalid thinker type, mobj expected");

	*(void**)dst = th;
	return 0;
}

static int func_get_mobj(lua_State *L, void *dst, void *o)
{
	if(*(void**)dst)
		lua_pushlightuserdata(L, *(void**)dst);
	else
		lua_pushnil(L);
	return 1;
}

// few flags have to be checked when changed
static int func_set_flags(lua_State *L, void *dst, void *o)
{
	uint64_t nf, change;
	mobj_t *mo = o;

	nf = lua_tointeger(L, -1);

	change = mo->flags ^ nf;
	if(change & (MF_NOBLOCKMAP | MF_NOSECTOR))
	{
		P_UnsetThingPosition(mo);
		mo->flags = nf;
		P_SetThingPosition(mo);
	} else
		mo->flags = nf;
	return 0;
}

// return sector from mobj
static int func_get_sector(lua_State *L, void *dst, void *o)
{
	mobj_t *mo = o;

	if(mo->subsector && mo->subsector->sector)
		lua_pushlightuserdata(L, mo->subsector->sector);
	else
		lua_pushnil(L);
	return 1;
}

// return mobj info from mobj
static int func_get_info(lua_State *L, void *dst, void *o)
{
	mobj_t *mo = o;
	lua_pushlightuserdata(L, mo->info);
	return 1;
}

// return mobj removal function
static int func_get_removemobj(lua_State *L, void *dst, void *o)
{
	lua_pushlightuserdata(L, o);
	lua_pushcclosure(L, LUA_removeMobjFromMobj, 1);
	return 1;
}

// return flag check function
static int func_get_checkflagmobj(lua_State *L, void *dst, void *o)
{
	lua_pushlightuserdata(L, o);
	lua_pushcclosure(L, LUA_flagCheckMobj, 1);
	return 1;
}

// return fuction for teleporting mobj
static int func_get_teleportmobj(lua_State *L, void *dst, void *o)
{
	lua_pushlightuserdata(L, o);
	lua_pushcclosure(L, LUA_teleportMobj, 1);
	return 1;
}

// return function for mobj position check
static int func_get_checkmobjpos(lua_State *L, void *dst, void *o)
{
	lua_pushlightuserdata(L, o);
	lua_pushcclosure(L, LUA_checkMobjPos, 1);
	return 1;
}

// return fuction for missile shooting
static int func_get_spawnmissile(lua_State *L, void *dst, void *o)
{
	lua_pushlightuserdata(L, o);
	lua_pushcclosure(L, LUA_spawnMissile, 1);
	return 1;
}

// return monster aiming function
static int func_get_attackaim(lua_State *L, void *dst, void *o)
{
	lua_pushlightuserdata(L, o);
	lua_pushcclosure(L, LUA_attackAim, 1);
	return 1;
}

// return melee range check function
static int func_get_meleerange(lua_State *L, void *dst, void *o)
{
	lua_pushlightuserdata(L, o);
	lua_pushcclosure(L, LUA_meleeRange, 1);
	return 1;
}

// return distance calculation function
static int func_get_distance(lua_State *L, void *dst, void *o)
{
	lua_pushlightuserdata(L, o);
	lua_pushcclosure(L, LUA_mobjDistance, 1);
	return 1;
}

// return damage function
static int func_get_damage(lua_State *L, void *dst, void *o)
{
	lua_pushlightuserdata(L, o);
	lua_pushcclosure(L, LUA_damageFromMobj, 1);
	return 1;
}

// return hitscan function
static int func_get_lineattack(lua_State *L, void *dst, void *o)
{
	lua_pushlightuserdata(L, o);
	lua_pushcclosure(L, LUA_lineAttack, 1);
	return 1;
}

// return mobj thrust function
static int func_get_thrustmobj(lua_State *L, void *dst, void *o)
{
	lua_pushlightuserdata(L, o);
	lua_pushcclosure(L, LUA_thrustFromMobj, 1);
	return 1;
}

// return mobj sight check function
static int func_get_checkmobjsight(lua_State *L, void *dst, void *o)
{
	lua_pushlightuserdata(L, o);
	lua_pushcclosure(L, LUA_sightCheckFromMobj, 1);
	return 1;
}

// return radius damage function
static int func_get_mobjradiusdmg(lua_State *L, void *dst, void *o)
{
	lua_pushlightuserdata(L, o);
	lua_pushcclosure(L, LUA_radiusDamageFromMobj, 1);
	return 1;
}

// return sound playback function
static int func_get_mobjsound_body(lua_State *L, void *dst, void *o)
{
	lua_pushlightuserdata(L, o);
	lua_pushinteger(L, SOUND_BODY);
	lua_pushcclosure(L, LUA_soundFromMobj, 2);
	return 1;
}

// return sound playback function
static int func_get_mobjsound_weapon(lua_State *L, void *dst, void *o)
{
	lua_pushlightuserdata(L, o);
	lua_pushinteger(L, SOUND_WEAPON);
	lua_pushcclosure(L, LUA_soundFromMobj, 2);
	return 1;
}

// return sound playback function
static int func_get_mobjsound_pickup(lua_State *L, void *dst, void *o)
{
	lua_pushlightuserdata(L, o);
	lua_pushinteger(L, SOUND_PICKUP);
	lua_pushcclosure(L, LUA_soundFromMobj, 2);
	return 1;
}

// return weapon set function
static int func_setplayerweapon(lua_State *L, void *dst, void *o)
{
	lua_pushlightuserdata(L, o);
	lua_pushcclosure(L, LUA_setPlayerWeapon, 1);
	return 1;
}

// return floor search function
static int func_get_lowefloorsector(lua_State *L, void *dst, void *o)
{
	lua_pushlightuserdata(L, o);
	lua_pushcclosure(L, LUA_lowFloorFromSector, 1);
	return 1;
}

// return floor search function
static int func_get_highfloorsector(lua_State *L, void *dst, void *o)
{
	lua_pushlightuserdata(L, o);
	lua_pushcclosure(L, LUA_highFloorFromSector, 1);
	return 1;
}

// return ceiling search function
static int func_get_loweceilingsector(lua_State *L, void *dst, void *o)
{
	lua_pushlightuserdata(L, o);
	lua_pushcclosure(L, LUA_lowCeilingFromSector, 1);
	return 1;
}

// return ceiling search function
static int func_get_highceilingsector(lua_State *L, void *dst, void *o)
{
	lua_pushlightuserdata(L, o);
	lua_pushcclosure(L, LUA_highCeilingFromSector, 1);
	return 1;
}

// return texture height search function
static int func_get_shortexfloorsector(lua_State *L, void *dst, void *o)
{
	lua_pushlightuserdata(L, o);
	lua_pushcclosure(L, LUA_shortTexFromSector, 1);
	return 1;
}

// return generic floor move function
static int func_get_genericfloorsector(lua_State *L, void *dst, void *o)
{
	lua_pushlightuserdata(L, o);
	lua_pushboolean(L, true);
	lua_pushcclosure(L, LUA_genericPlaneFromSector, 2);
	return 1;
}

// return generic ceiling move function
static int func_get_genericceilingsector(lua_State *L, void *dst, void *o)
{
	lua_pushlightuserdata(L, o);
	lua_pushboolean(L, false);
	lua_pushcclosure(L, LUA_genericPlaneFromSector, 2);
	return 1;
}

// change sector heights
static int func_set_sectorheight(lua_State *L, void *dst, void *o)
{
	lua_Number num;

	num = lua_tonumber(L, -1);
	*(int*)dst = (int)(num * (lua_Number)FRACUNIT);

	P_ChangeSector(o, false);

	return 0;
}

// 'flat' textures
static int func_set_flattexture(lua_State *L, void *dst, void *o)
{
	const char *tex;
	char temp[8];
	int lump;

	tex = lua_tostring(L, -1);
	strncpy(temp, tex, sizeof(temp));
	lump = R_FlatNumForName(temp);

	*(int*)dst = lump;

	return 0;
}

static int func_set_lumpname_optional(lua_State *L, void *dst, void *o)
{
	*(int*)dst = W_GetNumForNameLua(lua_tostring(L, -1), true);
	return 0;
}

static int func_get_lumpname(lua_State *L, void *dst, void *o)
{
	const char *tex;

	tex = W_LumpNumName(*(int*)dst);
	if(!tex)
		lua_pushstring(L, "-");
	else
	{
		char temp[9];
		memcpy(temp, tex, 8);
		temp[8] = 0;
		lua_pushstring(L, temp);
	}

	return 1;
}

//
// common functions

const lua_table_model_t *LUA_FieldByName(const char *name, const lua_table_model_t *model, int modelsize)
{
	int i;

	for(i = 0; i < modelsize; i++, model++)
	{
		if(!strcmp(model->name, name))
			return model;
	}
	return NULL;
}

int LUA_StructFromTable(lua_State *L, const lua_table_model_t *model, int modelsize, void *dst)
{
	int i;
	// assuming table is on top of the stack
	lua_pushnil(L);
	while(lua_next(L, -2) != 0)
	{
		if(lua_type(L, -2) == LUA_TSTRING)
		{
			const char *key = lua_tostring(L, -2);
			int type = lua_type(L, -1);
			// only string can be a key
			for(i = 0; i < modelsize; i++)
			{
				if(type == model[i].ltype && !strcmp(key, model[i].name))
				{
					if(model[i].set)
						model[i].set(L, ((void*)dst) + model[i].offset, dst);
					else
					{
						// right now only numbers are supported
						int num = lua_tointeger(L, -1);
						*(int*)(dst + model[i].offset) = num;
					}
					break;
				}
			}
			if(i == modelsize)
				return luaL_error(L, "key '%s' is not expected", key);
		} else
			return luaL_error(L, "invalid key for structure");
		lua_pop(L, 1);
	}
	return 0;
}

mobj_t *LUA_GetMobjParam(lua_State *L, int idx, boolean allownull)
{
	mobj_t *dest;

	if(allownull && lua_type(L, idx) == LUA_TNIL)
		return NULL;

	luaL_checktype(L, idx, LUA_TLIGHTUSERDATA);

	if(!lua_getmetatable(L, idx))
		return (void*)luaL_error(L, "mobj expected");

	lua_pushstring(L, "__index");
	lua_rawget(L, -2);

	if(lua_tocfunction(L, -1) != LUA_ThinkerIndex)
		return (void*)luaL_error(L, "mobj expected");

	lua_pop(L, 2);

	dest = lua_touserdata(L, idx);

	if(dest->thinker.lua_type != TT_MOBJ)
		return (void*)luaL_error(L, "mobj expected");

	return dest;
}

int LUA_GetMobjTypeParam(lua_State *L, int idx)
{
	mobjinfo_t *dest;

	luaL_checktype(L, idx, LUA_TLIGHTUSERDATA);

	if(!lua_getmetatable(L, idx))
		return luaL_error(L, "mobj type expected");

	lua_pushstring(L, "__index");
	lua_rawget(L, -2);

	if(lua_tocfunction(L, -1) != LUA_ThinkerIndex)
		return luaL_error(L, "mobj type expected");

	lua_pop(L, 2);

	dest = lua_touserdata(L, idx);

	if(dest->dthink.lua_type != TT_MOBJINFO)
		return luaL_error(L, "mobj type expected");

	return dest - mobjinfo;
}

//
// LUA functions

static int LUA_print(lua_State *L)
{
	// arg0 - debug text
	if(lua_gettop(L) == 1)
		printf("LUA: %s\n", lua_tostring(L, -1));
	return 0;
}

static int LUA_createMobjType(lua_State *L)
{
	if(lua_gettop(L) == 1 && lua_type(L, -1) == LUA_TTABLE)
	{
		int i, j;
		int k = 0;
		mobjinfo_t temp;

		// set type defaults
		memset(&temp, 0, sizeof(mobjinfo_t));
		state_mobjt = numstates;
		LUA_StructFromTable(L, lua_mobjtype, sizeof(lua_mobjtype)/sizeof(lua_table_model_t), &temp);
		// finish mobj type
		if((numobjtypes+1) / INFO_MOBJTYPE_ALLOC > numobjtypes / INFO_MOBJTYPE_ALLOC)
		{
			int asize = (numobjtypes + INFO_MOBJTYPE_ALLOC) * sizeof(mobjinfo_t);
			// realloc buffer
			mobjinfo_t *temp = realloc(mobjinfo, asize);
			if(!temp)
				I_Error("out of memory\n");
			mobjinfo = temp;
		}
		temp.dthink.lua_type = TT_MOBJINFO;
		memcpy(mobjinfo + numobjtypes, &temp, sizeof(mobjinfo_t));
		lua_pushlightuserdata(L, mobjinfo + numobjtypes);
		numobjtypes++;
	} else
		return luaL_error(L, "createMobjType: single table required");
	return 1;
}

static int LUA_setPlayerType(lua_State *L)
{
	MT_PLAYER = LUA_GetMobjTypeParam(L, 1);
	return 0;
}

static int LUA_setDoomTeleportType(lua_State *L)
{
	MT_TELEPORTMAN = LUA_GetMobjTypeParam(L, 1);
	return 0;
}

static int LUA_doomRandom(lua_State *L)
{
	int top = lua_gettop(L);
	int from, to;

	if(!top)
	{
		lua_pushinteger(L, P_Random());
		return 1;
	}

	luaL_checktype(L, 1, LUA_TNUMBER);

	if(top < 2)
	{
		from = 0;
		to = lua_tointeger(L, 1);
	} else
	{
		luaL_checktype(L, 2, LUA_TNUMBER);
		from = lua_tointeger(L, 1);
		to = lua_tointeger(L, 2);
	}

	if(to < from)
	{
		top = to;
		to = from;
		from = top;
	}

	lua_pushinteger(L, from + (rand() % ((to - from)+1)) );
	return 1;
}

static int LUA_spawnMobj(lua_State *L)
{
	mobj_t *mo;
	fixed_t x, y, z;
	angle_t ang;
	int type;
	int top = lua_gettop(L);

	luaL_checktype(L, 2, LUA_TNUMBER); // x
	luaL_checktype(L, 3, LUA_TNUMBER); // y

	// type; required
	type = LUA_GetMobjTypeParam(L, 1);

	// x and y; required
	x = lua_tonumber(L, 2) * (lua_Number)FRACUNIT;
	y = lua_tonumber(L, 3) * (lua_Number)FRACUNIT;

	// z; optional
	if(top > 3)
	{
		luaL_checktype(L, 4, LUA_TNUMBER);
		z = lua_tonumber(L, 4) * (lua_Number)FRACUNIT;
	} else
		z = ONFLOORZ;

	// angle; optional
	if(top > 4)
		ang = lua_tonumber(L, 5) * (lua_Number)(1 << ANGLETOFINESHIFT);
	else
		ang = 0;

	// spawn
	mo = P_SpawnMobj(x, y, z, type);

	// play first action
	P_SetMobjState(mo, mo->info->spawnstate);

	lua_pushlightuserdata(L, mo);

	return 1;
}

static int LUA_blockThingsIterator(lua_State *L)
{
	fixed_t x, y, xe, ye, t;
	int bx, by;
	int top = lua_gettop(L);
	int ftype = lua_type(L, 1);

	if(top == 2 || (top == 3 && ftype == LUA_TLIGHTUSERDATA))
	{
		// mobj, func, [arg]
		mobj_t *mo;

		luaL_checktype(L, 2, LUA_TFUNCTION);

		mo = LUA_GetMobjParam(L, 1, false);

		if(top > 2)
			block_lua_arg = luaL_ref(L, LUA_REGISTRYINDEX);
		else
			block_lua_arg = LUA_REFNIL;

		block_lua_func = luaL_ref(L, LUA_REGISTRYINDEX);

		x = mo->x;
		xe = x + mo->radius;
		x -= mo->radius;
		y = mo->y;
		ye = y + mo->radius;
		y -= mo->radius;
	} else
	if(top == 3 || top == 4)
	{
		// x, y, func, [arg]
		luaL_checktype(L, 1, LUA_TNUMBER);
		x = (fixed_t)(lua_tonumber(L, 1) * (lua_Number)FRACUNIT);
		luaL_checktype(L, 2, LUA_TNUMBER);
		y = (fixed_t)(lua_tonumber(L, 2) * (lua_Number)FRACUNIT);

		xe = x;
		ye = y;

		luaL_checktype(L, 3, LUA_TFUNCTION);

		if(top > 3)
			block_lua_arg = luaL_ref(L, LUA_REGISTRYINDEX);
		else
			block_lua_arg = LUA_REFNIL;

		block_lua_func = luaL_ref(L, LUA_REGISTRYINDEX);

	} else
	if(top == 5 || top == 6)
	{
		// x1, y1, x2, y2, func, [arg]
		luaL_checktype(L, 1, LUA_TNUMBER);
		x = (fixed_t)(lua_tonumber(L, 1) * (lua_Number)FRACUNIT);
		luaL_checktype(L, 2, LUA_TNUMBER);
		y = (fixed_t)(lua_tonumber(L, 2) * (lua_Number)FRACUNIT);

		luaL_checktype(L, 3, LUA_TNUMBER);
		xe = (fixed_t)(lua_tonumber(L, 3) * (lua_Number)FRACUNIT);
		luaL_checktype(L, 4, LUA_TNUMBER);
		ye = (fixed_t)(lua_tonumber(L, 4) * (lua_Number)FRACUNIT);

		luaL_checktype(L, 5, LUA_TFUNCTION);

		if(top > 5)
			block_lua_arg = luaL_ref(L, LUA_REGISTRYINDEX);
		else
			block_lua_arg = LUA_REFNIL;

		block_lua_func = luaL_ref(L, LUA_REGISTRYINDEX);
	} else
		return luaL_error(L, "blockThingsIterator: incorrect number of arguments");

	lua_settop(L, 0);
	luaS_block_iter = L;

	if(x > xe)
	{
		t = x;
		x = xe;
		xe = t;
	}

	if(y > ye)
	{
		t = y;
		y = ye;
		ye = t;
	}

	block_lua_x0 = x;
	block_lua_x1 = xe;
	block_lua_y0 = y;
	block_lua_y1 = ye;

	x = (x - bmaporgx)>>MAPBLOCKSHIFT;
	xe = (xe - bmaporgx)>>MAPBLOCKSHIFT;
	y = (y - bmaporgy)>>MAPBLOCKSHIFT;
	ye = (ye - bmaporgy)>>MAPBLOCKSHIFT;

	for(bx = x; bx <= xe; bx++)
		for(by = y; by <= ye; by++)
			if(!P_BlockThingsIterator(bx, by, PIT_LuaCheckThing))
				break;

	luaL_unref(L, LUA_REGISTRYINDEX, block_lua_func);
	luaL_unref(L, LUA_REGISTRYINDEX, block_lua_arg);

	return lua_gettop(L);
}

static int LUA_globalThingsIterator(lua_State *L)
{
	int arg, func;
	thinker_t *think;
	int top = lua_gettop(L);

	if(top > 2)
		return luaL_error(L, "globalThingsIterator: incorrect number of arguments");

	luaL_checktype(L, 1, LUA_TFUNCTION);

	if(top > 1)
		arg = luaL_ref(L, LUA_REGISTRYINDEX);
	else
		arg = LUA_REFNIL;

	func = luaL_ref(L, LUA_REGISTRYINDEX);

	lua_settop(L, 0);

	for(think = thinkercap.next; think != &thinkercap; think = think->next)
	{
		if(think->lua_type != TT_MOBJ)
			// not a mobj thinker
			continue;
		// function to call
		lua_rawgeti(L, LUA_REGISTRYINDEX, func);
		// mobj to pass
		lua_pushlightuserdata(L, think);
		// parameter to pass
		lua_rawgeti(L, LUA_REGISTRYINDEX, arg);
		// do the call
		if(lua_pcall(L, 2, LUA_MULTRET, 0))
			// script error
			return luaL_error(L, "ThingsIterator: %s", lua_tostring(L, -1));

		if(lua_gettop(L) == 0)
			// no return means continue
			continue;

		// first return has to be continue flag
		luaL_checktype(L, 1, LUA_TBOOLEAN);

		if(lua_toboolean(L, 1))
			// iteration will continue, discard all returns
			lua_settop(L, 0);
		else
		{
			// remove only this flag, rest will get returned
			lua_remove(L, 1);
			break;
		}
	}
	luaL_unref(L, LUA_REGISTRYINDEX, func);
	luaL_unref(L, LUA_REGISTRYINDEX, arg);
	return lua_gettop(L);
}

static int LUA_sectorTagIterator(lua_State *L)
{
	int i, tag;
	int arg, func;
	int top = lua_gettop(L);

	if(top > 3)
		return luaL_error(L, "sectorTagIterator: incorrect number of arguments");

	luaL_checktype(L, 1, LUA_TNUMBER);
	luaL_checktype(L, 2, LUA_TFUNCTION);

	tag = lua_tointeger(L, 1);

	if(top > 2)
		arg = luaL_ref(L, LUA_REGISTRYINDEX);
	else
		arg = LUA_REFNIL;

	func = luaL_ref(L, LUA_REGISTRYINDEX);

	lua_settop(L, 0);

	for(i = 0; i < numsectors; i++)
	{
		if(sectors[i].tag != tag)
			continue;
		// function to call
		lua_rawgeti(L, LUA_REGISTRYINDEX, func);
		// mobj to pass
		lua_pushlightuserdata(L, &sectors[i]);
		// parameter to pass
		lua_rawgeti(L, LUA_REGISTRYINDEX, arg);
		// do the call
		if(lua_pcall(L, 2, LUA_MULTRET, 0))
			// script error
			return luaL_error(L, "SectorIterator: %s", lua_tostring(L, -1));

		if(lua_gettop(L) == 0)
			// no return means continue
			continue;

		// first return has to be continue flag
		luaL_checktype(L, 1, LUA_TBOOLEAN);

		if(lua_toboolean(L, 1))
			// iteration will continue, discard all returns
			lua_settop(L, 0);
		else
		{
			// remove only this flag, rest will get returned
			lua_remove(L, 1);
			break;
		}
	}

	luaL_unref(L, LUA_REGISTRYINDEX, func);
	luaL_unref(L, LUA_REGISTRYINDEX, arg);
	return lua_gettop(L);
}

//
// indirect LUA functions

static int LUA_TableReadOnly(lua_State *L)
{
	return luaL_error(L, "tried to modify read-only table");
}

static int LUA_ThinkerIndex(lua_State *L)
{
	int i;
	const char *idx;
	const lua_table_model_t *field;
	thinker_t *th;
	int top = lua_gettop(L);

	// get index
	luaL_checktype(L, 2, LUA_TSTRING);
	idx = lua_tostring(L, 2);

	// get thinker type
	th = lua_touserdata(L, 1);
	switch(th->lua_type)
	{
		case TT_MOBJ:
			// special check for animation changes
			if(idx[0] == '_')
			{
				if(top != 2)
					return func_set_readonly(L, th, th);
				int k = 0;
				for(i = 0; i < sizeof(lua_mobjtype) / sizeof(lua_table_model_t); i++)
				{
					if(lua_mobjtype[i].name[0] == '_')
					{
						if(!strcmp(idx, lua_mobjtype[i].name))
						{
							lua_pushlightuserdata(L, th);
							lua_pushinteger(L, k);
							lua_pushcclosure(L, LUA_animationFromMobj, 2);
							return 1;
						}
						k++;
					}
				}
				field = NULL;
			} else
			// normal check
			field = LUA_FieldByName(idx, lua_mobj, sizeof(lua_mobj) / sizeof(lua_table_model_t));
		break;
		case TT_MOBJINFO:
			field = LUA_FieldByName(idx, lua_mobjtype, sizeof(lua_mobjtype) / sizeof(lua_table_model_t));
		break;
		case TT_PLAYER:
			field = LUA_FieldByName(idx, lua_player, sizeof(lua_player) / sizeof(lua_table_model_t));
		break;
		case TT_SECTOR:
			field = LUA_FieldByName(idx, lua_sector, sizeof(lua_sector) / sizeof(lua_table_model_t));
		break;
		default:
			return luaL_error(L, "unknown thinker type");
	}

	// check field
	if(!field)
		return luaL_error(L, "thinker does not contain '%s'", idx);

	if(top == 2)
	{
		// index
		if(field->get)
			return field->get(L, ((void*)th) + field->offset, th);
		lua_pushinteger(L, *(int*)(((void*)th) + field->offset));
		return 1;
	}

	// newindex
	if(field->ltype != LUA_TLIGHTUSERDATA)
		luaL_checktype(L, -1, field->ltype);

	if(field->set)
		return field->set(L, ((void*)th) + field->offset, th);
	*(int*)(((void*)th) + field->offset) = lua_tointeger(L, -1);

	return 0;
}

static int LUA_removeMobjFromMobj(lua_State *L)
{
	mobj_t *mo;

	mo = lua_touserdata(L, lua_upvalueindex(1));

	P_RemoveMobj(mo);

	return 0;
}

static int LUA_flagCheckMobj(lua_State *L)
{
	mobj_t *mo;
	int flag;

	luaL_checktype(L, 1, LUA_TNUMBER);
	flag = lua_tointeger(L, 1);

	mo = lua_touserdata(L, lua_upvalueindex(1));

	lua_pushboolean(L, !!(mo->flags & flag));

	return 1;
}

static int LUA_teleportMobj(lua_State *L)
{
	mobj_t *mo;
	fixed_t x, y, z;
	boolean stomp;

	luaL_checktype(L, 1, LUA_TNUMBER);
	luaL_checktype(L, 2, LUA_TNUMBER);
	luaL_checktype(L, 3, LUA_TNUMBER);
	luaL_checktype(L, 4, LUA_TBOOLEAN);

	x = lua_tonumber(L, 1) * (lua_Number)FRACUNIT;
	y = lua_tonumber(L, 2) * (lua_Number)FRACUNIT;
	z = lua_tonumber(L, 3) * (lua_Number)FRACUNIT;
	stomp = lua_toboolean(L, 4);

	mo = lua_touserdata(L, lua_upvalueindex(1));

	if(stomp)
	{
		if(!P_TeleportMove(mo, x, y)) // TODO: proper Z
		{
			lua_pushboolean(L, 0);
			return 1;
		}
		mo->z = z;
	} else
	{
		P_UnsetThingPosition(mo);
		mo->x = x;
		mo->y = y;
		mo->z = z;
		P_SetThingPosition(mo);
	}

	if(mo->player)
		mo->player->viewz = mo->z + mo->player->viewheight;

	lua_pushboolean(L, 1);

	return 1;
}

static int LUA_checkMobjPos(lua_State *L)
{
	mobj_t *mo;
	fixed_t x, y, z;

	mo = lua_touserdata(L, lua_upvalueindex(1));

	if(lua_gettop(L) == 0)
	{
		x = mo->x;
		y = mo->y;
		z = mo->z;
	} else
	{
		luaL_checktype(L, 1, LUA_TNUMBER);
		luaL_checktype(L, 2, LUA_TNUMBER);
		luaL_checktype(L, 3, LUA_TNUMBER);
		x = lua_tonumber(L, 1) * (lua_Number)FRACUNIT;
		y = lua_tonumber(L, 2) * (lua_Number)FRACUNIT;
		z = lua_tonumber(L, 3) * (lua_Number)FRACUNIT;
	}

	lua_pushboolean(L, P_CheckPosition(mo, x, y)); // TODO: add Z
	return 1;
}

static int LUA_spawnMissile(lua_State *L)
{
	mobj_t *mo;
	mobj_t *mi;
	int type;
	angle_t a = 0;
	fixed_t x = 0;
	fixed_t y = 0;
	fixed_t z = 0;
	fixed_t s = 0;
	int top = lua_gettop(L);

	// type, must be present
	type = LUA_GetMobjTypeParam(L, 1);

	// absolute angle
	if(top > 1)
	{
		luaL_checktype(L, 2, LUA_TNUMBER);
		a = lua_tonumber(L, 2) * (lua_Number)(1 << ANGLETOFINESHIFT);
	}

	// absolute pitch
	if(top > 2)
	{
		luaL_checktype(L, 3, LUA_TNUMBER);
		s = (fixed_t)(lua_tonumber(L, 3) * (lua_Number)FRACUNIT);
	}

	// z offset
	if(top > 3)
	{
		luaL_checktype(L, 4, LUA_TNUMBER);
		z = (fixed_t)(lua_tonumber(L, 4) * (lua_Number)FRACUNIT);
	}

	// x offset
	if(top > 4)
	{
		luaL_checktype(L, 5, LUA_TNUMBER);
		x = (fixed_t)(lua_tonumber(L, 5) * (lua_Number)FRACUNIT);
	}

	// y offset
	if(top > 5)
	{
		luaL_checktype(L, 6, LUA_TNUMBER);
		y = (fixed_t)(lua_tonumber(L, 6) * (lua_Number)FRACUNIT);
	}

	// spawn
	mo = lua_touserdata(L, lua_upvalueindex(1));
	mi = P_SpawnMissile(mo, type, a, s, z, x, y);

	lua_pushlightuserdata(L, mi);

	return 1;
}

static int LUA_attackAim(lua_State *L)
{
	// this will need better math
	mobj_t *source;
	mobj_t *dest;
	angle_t angle;
	fixed_t slope;
	fixed_t z;
	boolean hitscan = 0;
	int top = lua_gettop(L);

	source = lua_touserdata(L, lua_upvalueindex(1));

	// optional type
	if(top > 0)
	{
		luaL_checktype(L, 1, LUA_TBOOLEAN);
		hitscan = lua_toboolean(L, 1);
	}

	if(top > 1)
	{
		// optional target
		dest = LUA_GetMobjParam(L, 2, false);
	} else
		// use mobj target
		dest = source->target;

	if(source->player && !dest)
	{
		// player aim
		angle = source->angle;
		if(sv_freeaim)
		{
			slope = source->pitch;
		} else
		{
			slope = P_AimLineAttack (source, angle, 16*64*FRACUNIT, NULL);

			if (!linetarget)
			{
				angle += 1<<26;
				slope = P_AimLineAttack (source, angle, 16*64*FRACUNIT, NULL);
				if (!linetarget)
				{
					angle -= 2<<26;
					slope = P_AimLineAttack (source, angle, 16*64*FRACUNIT, NULL);
				}
			}
		}
	} else
	{
		// other aim

		if(!dest)
		{
			// nothing to aim at
			lua_pushinteger(L, source->angle / (lua_Number)(1 << ANGLETOFINESHIFT));
			lua_pushinteger(L, source->pitch / (lua_Number)FRACUNIT);
			return 2;
		}

		angle = R_PointToAngle2(source->x, source->y, dest->x, dest->y);

		if(hitscan)
		{
			slope = P_AimLineAttack(source, angle, MISSILERANGE, NULL);
			if(!linetarget)
				hitscan = false;
		}

		if(!hitscan)
		{
			if(dest->z < source->z)
				z = dest->z + dest->info->height / 4;
			else
				z = dest->z;

			slope = P_AproxDistance(dest->x - source->x, dest->y - source->y);
			slope = P_AproxDistance(slope, z - source->z) / FRACUNIT;

			if(slope < 1)
				slope = 1;

			slope = (z - source->z) / slope;
		}
		// invisiblity random
		if(!source->player && dest->flags & MF_SHADOW)
			angle += (P_Random()-P_Random())<<21;
	}

	lua_pushnumber(L, angle / (lua_Number)(1 << ANGLETOFINESHIFT));
	lua_pushnumber(L, slope / (lua_Number)FRACUNIT);

	return 2;
}

static int LUA_meleeRange(lua_State *L)
{
	mobj_t *source;
	mobj_t *dest;
	boolean zCheck;
	int top = lua_gettop(L);

	source = lua_touserdata(L, lua_upvalueindex(1));

	if(top > 0)
		// optional target
		dest = LUA_GetMobjParam(L, 1, false);
	else
		// use mobj target
		dest = source->target;

	if(top > 1)
	{
		luaL_checktype(L, 2, LUA_TBOOLEAN);
		zCheck = lua_toboolean(L, 2);
	} else
		zCheck = true;

	if(P_CheckMeleeRange(source, dest, zCheck))
		lua_pushboolean(L, true);
	else
		lua_pushboolean(L, false);

	return 1;
}

static int LUA_mobjDistance(lua_State *L)
{
	mobj_t *source;
	mobj_t *dest;
	boolean zCheck;
	fixed_t dist;
	int top = lua_gettop(L);

	source = lua_touserdata(L, lua_upvalueindex(1));

	if(top > 0)
		// optional target
		dest = LUA_GetMobjParam(L, 1, false);
	else
		// use mobj target
		dest = source->target;

	if(top > 1)
	{
		luaL_checktype(L, 2, LUA_TBOOLEAN);
		zCheck = lua_toboolean(L, 2);
	} else
		zCheck = true;

	dist = P_AproxDistance(dest->x - source->x, dest->y - source->y);
	if(zCheck)
		dist = P_AproxDistance(dest->z - source->z, dist);

	lua_pushnumber(L, dist / (lua_Number)FRACUNIT);

	return 1;
}

static int LUA_damageFromMobj(lua_State *L)
{
	int damage;
	mobj_t *dest;
	mobj_t *inflictor = NULL;
	mobj_t *source = NULL;
	int top = lua_gettop(L);

	// damage
	if(lua_type(L, 1) == LUA_TBOOLEAN)
	{
		if(lua_toboolean(L, 1))
			damage = 1000000;
		else
			damage = INSTANTKILL;
	} else
	{
		luaL_checktype(L, 1, LUA_TNUMBER);
		damage = lua_tointeger(L, 1);
		if(damage < 0) // Doom random
			damage = ((P_Random()%(-damage))+1)*3;
	}

	// optional source
	if(top > 1)
		source = LUA_GetMobjParam(L, 2, true);

	// optional inflictor
	if(top > 2)
		inflictor = LUA_GetMobjParam(L, 3, true);

	dest = lua_touserdata(L, lua_upvalueindex(1));

	P_DamageMobj(dest, inflictor, source, damage);

	if(dest->health > 0)
		lua_pushboolean(L, false);
	else
		lua_pushboolean(L, true);

	return 1;
}

static int LUA_lineAttack(lua_State *L)
{
	mobj_t *mo;
	int type, damage;
	angle_t a = 0;
	fixed_t x = 0;
	fixed_t z = 0;
	fixed_t s = 0;
	int top = lua_gettop(L);

	// type, must be present
	type = LUA_GetMobjTypeParam(L, 1);

	// damage, must be present
	luaL_checktype(L, 2, LUA_TNUMBER);
	damage = lua_tointeger(L, 2);
	if(damage < 0) // Doom random
		damage = ((P_Random()%(-damage))+1)*3;

	// absolute angle
	if(top > 2)
	{
		luaL_checktype(L, 3, LUA_TNUMBER);
		a = lua_tonumber(L, 3) * (lua_Number)(1 << ANGLETOFINESHIFT);
	}

	// absolute pitch
	if(top > 3)
	{
		luaL_checktype(L, 4, LUA_TNUMBER);
		s = (fixed_t)(lua_tonumber(L, 4) * (lua_Number)FRACUNIT);
	}

	// z offset
	if(top > 4)
	{
		luaL_checktype(L, 5, LUA_TNUMBER);
		z = (fixed_t)(lua_tonumber(L, 5) * (lua_Number)FRACUNIT);
	}

	// x offset
	if(top > 4)
	{
		luaL_checktype(L, 5, LUA_TNUMBER);
		x = (fixed_t)(lua_tonumber(L, 5) * (lua_Number)FRACUNIT);
	}

	// spawn
	mo = lua_touserdata(L, lua_upvalueindex(1));

	la_pufftype = type;
	P_LineAttack(mo, a, MISSILERANGE, s, damage, z, x);

	if(la_puffmobj)
		lua_pushlightuserdata(L, la_puffmobj);
	else
		lua_pushnil(L);

	if(linetarget)
		lua_pushlightuserdata(L, linetarget);
	else
		lua_pushnil(L);

	return 2;
}

static int LUA_thrustFromMobj(lua_State *L)
{
	mobj_t *mo;
	fixed_t speed;
	angle_t a = 0;
	int top = lua_gettop(L);

	// speed, must be present
	luaL_checktype(L, 1, LUA_TNUMBER);
	speed = (fixed_t)(lua_tonumber(L, 1) * (lua_Number)FRACUNIT);

	// angle, optional
	if(top > 1)
	{
		luaL_checktype(L, 2, LUA_TNUMBER);
		a = lua_tonumber(L, 2) * (lua_Number)(1 << ANGLETOFINESHIFT);
	}

	mo = lua_touserdata(L, lua_upvalueindex(1));
	a += mo->angle;

	mo->momx = FixedMul(speed, finecosine[a>>ANGLETOFINESHIFT]);
	mo->momy = FixedMul(speed, finesine[a>>ANGLETOFINESHIFT]);

	return 0;
}

static int LUA_sightCheckFromMobj(lua_State *L)
{
	mobj_t *source;
	mobj_t *dest;

	source = lua_touserdata(L, lua_upvalueindex(1));

	if(lua_gettop(L) > 0)
		// optional target
		dest = LUA_GetMobjParam(L, -1, false);
	else
		// use mobj target
		dest = source->target;

	if(dest && P_CheckSight(source, dest))
		lua_pushboolean(L, true);
	else
		lua_pushboolean(L, false);

	return 1;
}

static int LUA_radiusDamageFromMobj(lua_State *L)
{
	mobj_t *source;
	int damage;
	fixed_t range;
	mobj_t *from = NULL;
	boolean hurt = true;
	int top = lua_gettop(L);

	// range; required
	luaL_checktype(L, 1, LUA_TNUMBER);
	range = (fixed_t)(lua_tonumber(L, 1) * (lua_Number)FRACUNIT);

	// damage; required
	luaL_checktype(L, 2, LUA_TNUMBER);
	damage = lua_tointeger(L, 2);

	// attacker; optional
	if(top > 2)
		from = LUA_GetMobjParam(L, 3, true);

	// hurt flag; optional
	if(top > 3)
	{
		luaL_checktype(L, 4, LUA_TBOOLEAN);
		hurt = lua_toboolean(L, 4);
	}

	source = lua_touserdata(L, lua_upvalueindex(1));

	P_RadiusAttack(source, from, range, damage, hurt);

	return 0;
}

static int LUA_soundFromMobj(lua_State *L)
{
	const char *tex;
	char temp[8];
	int lump;
	int chan, idx;
	mobj_t *source;
	int top = lua_gettop(L);

	// at least one sound
	luaL_checktype(L, 1, LUA_TSTRING);

	// sound(s) to play
	// if multiple sounds are specified, pick random one
	if(top > 1)
		idx = 1 + (P_Random() % top);
	else
		idx = 1;

	tex = lua_tostring(L, idx);

	if(tex[0] == '-' && tex[1] == 0)
	{
		// support for 'no sound'
		return 0;
	} else
	{
		strncpy(temp, tex, sizeof(temp));
		lump = W_GetNumForName(temp);
	}

	source = lua_touserdata(L, lua_upvalueindex(1));
	chan = lua_tointeger(L, lua_upvalueindex(2));

	S_StartSound(source, lump, chan);

	return 0;
}

static int LUA_setPlayerWeapon(lua_State *L)
{
	player_t *pl;

	pl = lua_touserdata(L, lua_upvalueindex(1));
	pl->pendingweapon = LUA_GetMobjTypeParam(L, 1);
	P_BringUpWeapon(pl);

	return 0;
}

static int LUA_genericPlaneFromSector(lua_State *L)
{
	generic_info_t gen;
	int top = lua_gettop(L);
	boolean floor;

	gen.sector = lua_touserdata(L, lua_upvalueindex(1));
	floor = lua_toboolean(L, lua_upvalueindex(2));

	// stopz; required
	luaL_checktype(L, 1, LUA_TNUMBER);
	gen.stopz = (fixed_t)lua_tonumber(L, 1) * (lua_Number)FRACUNIT;

	// speed; required
	luaL_checktype(L, 2, LUA_TNUMBER);
	gen.speed = (fixed_t)lua_tonumber(L, 2) * (lua_Number)FRACUNIT;
	if(gen.speed < 0)
		gen.speed = -gen.speed;

	// crush speed; optional
	if(top > 2)
	{
		luaL_checktype(L, 3, LUA_TNUMBER);
		gen.crushspeed = (fixed_t)lua_tonumber(L, 3) * (lua_Number)FRACUNIT;
		if(gen.crushspeed < 0)
			gen.crushspeed = -gen.crushspeed;
	}

	// sounds; optional
	if(top > 3)
	{
		luaL_checktype(L, 4, LUA_TSTRING);
		gen.startsound = W_GetNumForNameLua(lua_tostring(L, 4), true);
	} else
		gen.startsound = 0;

	if(top > 4)
	{
		luaL_checktype(L, 5, LUA_TSTRING);
		gen.stopsound = W_GetNumForNameLua(lua_tostring(L, 5), true);
	} else
		gen.stopsound = 0;

	if(top > 5)
	{
		luaL_checktype(L, 6, LUA_TSTRING);
		gen.movesound = W_GetNumForNameLua(lua_tostring(L, 6), true);
	} else
		gen.movesound = 0;

	// texture; optional
	if(top > 6)
	{
		luaL_checktype(L, 7, LUA_TSTRING);
		char temp[8];
		strncpy(temp, lua_tostring(L, 7), sizeof(temp));
		gen.stoppic = R_FlatNumForName(temp);
	} else
	if(floor)
		gen.stoppic = gen.sector->floorpic;
	else
		gen.stoppic = gen.sector->ceilingpic;

	if(floor)
	{
		gen.startpic = gen.sector->floorpic;
		gen.startz = gen.sector->floorheight;
		P_GenericSectorFloor(&gen);
	} else
	{
		gen.startpic = gen.sector->ceilingpic;
		gen.startz = gen.sector->ceilingheight;
		P_GenericSectorCeiling(&gen);
	}

	return 0;
}

static int LUA_lowFloorFromSector(lua_State *L)
{
	int i;
	sector_t *sec, *next;
	fixed_t height;

	sec = lua_touserdata(L, lua_upvalueindex(1));

	if(!sec->linecount)
	{
		lua_pushnumber(L, sec->floorheight / (lua_Number)FRACUNIT);
		return 1;
	}

	height = ONCEILINGZ;

	for(i = 0; i < sec->linecount; i++)
	{
		next = getNextSector(sec->lines[i], sec);
		if(next && next->floorheight < height)
			height = next->floorheight;
	}

	lua_pushnumber(L, height / (lua_Number)FRACUNIT);

	return 1;
}

static int LUA_highFloorFromSector(lua_State *L)
{
	int i;
	sector_t *sec, *next;
	fixed_t height;

	sec = lua_touserdata(L, lua_upvalueindex(1));

	if(!sec->linecount)
	{
		lua_pushnumber(L, sec->floorheight / (lua_Number)FRACUNIT);
		return 1;
	}

	height = ONFLOORZ;

	for(i = 0; i < sec->linecount; i++)
	{
		next = getNextSector(sec->lines[i], sec);
		if(next && next->floorheight > height)
			height = next->floorheight;
	}

	lua_pushnumber(L, height / (lua_Number)FRACUNIT);

	return 1;
}

static int LUA_lowCeilingFromSector(lua_State *L)
{
	int i;
	sector_t *sec, *next;
	fixed_t height;

	sec = lua_touserdata(L, lua_upvalueindex(1));

	if(!sec->linecount)
	{
		lua_pushnumber(L, sec->ceilingheight / (lua_Number)FRACUNIT);
		return 1;
	}

	height = ONCEILINGZ;

	for(i = 0; i < sec->linecount; i++)
	{
		next = getNextSector(sec->lines[i], sec);
		if(next && next->ceilingheight < height)
			height = next->ceilingheight;
	}

	lua_pushnumber(L, height / (lua_Number)FRACUNIT);

	return 1;
}

static int LUA_highCeilingFromSector(lua_State *L)
{
	int i;
	sector_t *sec, *next;
	fixed_t height;

	sec = lua_touserdata(L, lua_upvalueindex(1));

	if(!sec->linecount)
	{
		lua_pushnumber(L, sec->ceilingheight / (lua_Number)FRACUNIT);
		return 1;
	}

	height = ONFLOORZ;

	for(i = 0; i < sec->linecount; i++)
	{
		next = getNextSector(sec->lines[i], sec);
		if(next && next->ceilingheight > height)
			height = next->ceilingheight;
	}

	lua_pushnumber(L, height / (lua_Number)FRACUNIT);

	return 1;
}

static int LUA_shortTexFromSector(lua_State *L)
{
	int i;
	sector_t *sec, *next;
	side_t *side;
	fixed_t height = ONCEILINGZ;

	luaL_checktype(L, 1, LUA_TBOOLEAN);

	sec = lua_touserdata(L, lua_upvalueindex(1));

	if(!sec->linecount)
	{
		lua_pushnumber(L, 0);
		return 1;
	}

	if(lua_toboolean(L, 1))
	{
		for(i = 0; i < sec->linecount; i++)
		{
			if(sec->lines[i]->flags & ML_TWOSIDED)
			{
				side = &sides[sec->lines[i]->sidenum[0]];
				if(side->toptexture >= 0)
					if(textureheight[side->toptexture] < height)
						height = textureheight[side->toptexture];
				side = &sides[sec->lines[i]->sidenum[1]];
				if(side->toptexture >= 0)
					if(textureheight[side->toptexture] < height)
						height = textureheight[side->toptexture];
			}
		}
	} else
	{
		for(i = 0; i < sec->linecount; i++)
		{
			if(sec->lines[i]->flags & ML_TWOSIDED)
			{
				side = &sides[sec->lines[i]->sidenum[0]];
				if(side->bottomtexture >= 0)
					if(textureheight[side->bottomtexture] < height)
						height = textureheight[side->bottomtexture];
				side = &sides[sec->lines[i]->sidenum[1]];
				if(side->bottomtexture >= 0)
					if(textureheight[side->bottomtexture] < height)
						height = textureheight[side->bottomtexture];
			}
		}
	}

	if(height == ONCEILINGZ)
		height = 0;

	lua_pushnumber(L, height / (lua_Number)FRACUNIT);

	return 1;
}

static int LUA_animationFromMobj(lua_State *L)
{
	mobj_t *mo;
	int *st;
	int an;
	int skip = 0;

	mo = lua_touserdata(L, lua_upvalueindex(1));
	an = lua_tointeger(L, lua_upvalueindex(2));

	st = &mo->info->spawnstate;

	if(lua_gettop(L) > 0)
	{
		luaL_checktype(L, 1, LUA_TNUMBER);
		skip = lua_tointeger(L, 1);
	}

	if(st[an])
	{
		P_SetMobjAnimation(mo, an, skip);
		lua_pushboolean(L, true);
	} else
		lua_pushboolean(L, false);

	return 1;
}

static int LUA_doMobjAction(lua_State *L)
{
	void (*func)(mobj_t*);
	mobj_t *mo;

	luaL_checktype(L, -1, LUA_TLIGHTUSERDATA);

	if(!lua_getmetatable(L, -1))
		return luaL_error(L, "doMobjAction: mobj expected");

	lua_pushstring(L, "__index");
	lua_rawget(L, -2);

	if(lua_tocfunction(L, -1) != LUA_ThinkerIndex)
		return luaL_error(L, "doMobjAction: mobj expected");

	mo = lua_touserdata(L, -3);

	if(mo->thinker.lua_type != TT_MOBJ)
		return luaL_error(L, "doMobjAction: mobj expected");

	func = lua_touserdata(L, lua_upvalueindex(1));
	func(mo);

	return 0;
}

static int LUA_SinIndex(lua_State *L)
{
	angle_t ang;

	luaL_checktype(L, -1, LUA_TNUMBER);
	ang = (int)lua_tonumber(L, -1) & FINEMASK;
	lua_pushnumber(L, finesine[ang] / (lua_Number)FRACUNIT);
	return 1;
}

static int LUA_CosIndex(lua_State *L)
{
	angle_t ang;

	luaL_checktype(L, -1, LUA_TNUMBER);
	ang = (int)lua_tonumber(L, -1) & FINEMASK;
	lua_pushnumber(L, finecosine[ang] / (lua_Number)FRACUNIT);
	return 1;
}

//
// LUA state

void L_ExportFlags(lua_State *L)
{
	int i;

	lua_createtable(L, 0, sizeof(lua_mobjflags) / sizeof(lua_mobjflag_t)); // 'mf' table
	for(i = 0; i < sizeof(lua_mobjflags) / sizeof(lua_mobjflag_t); i++)
	{
		lua_pushinteger(L, lua_mobjflags[i].value);
		lua_setfield(L, -2, lua_mobjflags[i].name);
	}
	lua_setglobal(L, "mf");
}

void L_ExportMobjActions(lua_State *L)
{
	int i;

	lua_createtable(L, 0, sizeof(lua_mobjactions) / sizeof(lua_mobjaction_t)); // 'a' table
	for(i = 0; i < sizeof(lua_mobjactions) / sizeof(lua_mobjaction_t); i++)
	{
		lua_pushlightuserdata(L, lua_mobjactions[i].func);
		lua_pushcclosure(L, LUA_doMobjAction, 1);
		lua_setfield(L, -2, lua_mobjactions[i].name);
	}
	lua_setglobal(L, "a");
}

void L_ExportThinkerMetatable(lua_State *L)
{
	// userdata
	lua_pushlightuserdata(L, NULL);
	// metatable; all userdata share single metatable
	lua_createtable(L, 0, 2);
	// newindex
	lua_pushcfunction(L, LUA_ThinkerIndex);
	lua_setfield(L, -2, "__newindex");
	// index
	lua_pushcfunction(L, LUA_ThinkerIndex);
	lua_setfield(L, -2, "__index");
	// set metatable
	lua_setmetatable(L, -2);
	// pop userdata
	lua_pop(L, 1);
}

void L_ExportSinTable(lua_State *L)
{
	// variable 'finesine'
	lua_createtable(L, 0, 0);
	// metatable
	lua_createtable(L, 0, 2);
	// newindex
	lua_pushcfunction(L, LUA_TableReadOnly);
	lua_setfield(L, -2, "__newindex");
	// index
	lua_pushcfunction(L, LUA_SinIndex);
	lua_setfield(L, -2, "__index");
	// add metatable
	lua_setmetatable(L, -2);
	// finish
	lua_setglobal(L, "finesine");
}

void L_ExportCosTable(lua_State *L)
{
	// variable 'finesine'
	lua_createtable(L, 0, 0);
	// metatable
	lua_createtable(L, 0, 2);
	// newindex
	lua_pushcfunction(L, LUA_TableReadOnly);
	lua_setfield(L, -2, "__newindex");
	// index
	lua_pushcfunction(L, LUA_CosIndex);
	lua_setfield(L, -2, "__index");
	// add metatable
	lua_setmetatable(L, -2);
	// finish
	lua_setglobal(L, "finecosine");
}

void L_LoadScript(int lump)
{
	int i;

	// new lua state
	if(!luaS_game)
	{
		luaS_game = luaL_newstate();
		// export some Lua libraries
		luaL_requiref(luaS_game, "table", luaopen_table, true);
		luaL_requiref(luaS_game, "math", luaopen_math, true);
		luaL_requiref(luaS_game, "string", luaopen_string, true);
		// export all functions
		for(i = 0; i < sizeof(lua_functions) / sizeof(luafunc_t); i++)
			lua_register(luaS_game, lua_functions[i].name, lua_functions[i].func);
		// export all mobj flags
		L_ExportFlags(luaS_game);
		// export all state functions
		L_ExportMobjActions(luaS_game);
		// export thinker table (for any lightuserdata)
		L_ExportThinkerMetatable(luaS_game);
		// export Doom sin / cos tables
		L_ExportSinTable(luaS_game);
		L_ExportCosTable(luaS_game);
	}
	// load lua script
	luaL_loadbuffer(luaS_game, W_CacheLumpNum(lump), W_LumpLength(lump), W_LumpNumName(lump));
	// call (initialize) script
	if(lua_pcall(luaS_game, 0, 0, 0))
	{
		I_Error("L_LoadScript: %s", lua_tostring(luaS_game, -1));
		lua_close(luaS_game);
		return;
	}
}

static boolean cb_LuaLoad(int lump)
{
	L_LoadScript(lump);
	return false;
}

void L_Init()
{
	int i;

	// create sprite names
	sprnames = malloc(INFO_SPRITE_ALLOC * sizeof(sprname_t));
	if(!sprnames)
		I_Error("out of memory\n");
	// add default names
	memcpy(sprnames, info_def_sprnames, sizeof(info_def_sprnames));
	numsnames = NUM_DEF_SPRITES;

	// create mobj states
	states = malloc(INFO_STATE_ALLOC * sizeof(state_t));
	if(!states)
		I_Error("out of memory\n");
	// add default states
	memcpy(states, info_def_states, sizeof(info_def_states));
	numstates = NUM_DEF_STATES;
	// also clear functions of these states
	for(i = 0; i < NUM_DEF_STATES; i++)
		states[i].func = LUA_REFNIL;

	// create mobj types
	mobjinfo = malloc(INFO_MOBJTYPE_ALLOC * sizeof(mobjinfo_t));
	if(!mobjinfo)
		I_Error("out of memory\n");
	// add default types
	memcpy(mobjinfo, info_def_mobjinfo, sizeof(info_def_mobjinfo));
	numobjtypes = NUM_DEF_MOBJTYPES;

	// load script from all WADs
	W_ForEachName("GAMELUA", cb_LuaLoad);
}

//
// game callbacks

void L_SetupMap()
{
	const char *maplump = W_LumpNumName(level_lump);

	lua_pushstring(luaS_game, maplump);
	lua_setglobal(luaS_game, "levelLump");

	// TODO: start map load scripts
	lua_settop(luaS_game, 0);
}

void L_SpawnPlayer(player_t *pl)
{
	lua_getglobal(luaS_game, "playerSpawn");
	if(lua_type(luaS_game, -1) == LUA_TFUNCTION)
	{
		lua_pushlightuserdata(luaS_game, pl);
		if(lua_pcall(luaS_game, 1, 0, 0))
			// script error
			I_Error("L_SpawnPlayer: %s", lua_tostring(luaS_game, -1));
	}
	lua_settop(luaS_game, 0);
}

//
// other functions

boolean P_SetMobjAnimation(mobj_t *mo, int anim, int skip)
{
	int state;
	int *st;

	if(anim < 0)
	{
		// only skip some states
		state = mo->state - states;
	} else
	{
		// change animation
		st = &mo->info->spawnstate;
		state = st[anim];
	}

	while(1)
	{
		if(state == S_NULL)
		{
			mo->state = NULL;
#ifdef SERVER
			P_RemoveMobj(mo, false);
#else
			P_RemoveMobj(mo);
#endif
			return false;
		}

		if(!skip)
			return P_SetMobjState(mo, state);
		skip--;

		state = states[state].nextstate;
		if(state & STATE_ANIMATION)
		{
			state = L_StateFromAlias(mo->info, state);
			return P_SetMobjState(mo, state);
		}
	}
	return true;
}

boolean PIT_LuaCheckThing(mobj_t *thing)
{
	boolean ret;
	fixed_t tbx0 = thing->x + thing->radius;
	fixed_t tbx1 = thing->x - thing->radius;
	fixed_t tby0 = thing->y + thing->radius;
	fixed_t tby1 = thing->y - thing->radius;

	// range check
	if(tbx1 > block_lua_x1)
		return true;
	if(tbx0 < block_lua_x0)
		return true;
	if(tby1 > block_lua_y1)
		return true;
	if(tby0 < block_lua_y0)
		return true;

	// function to call
	lua_rawgeti(luaS_block_iter, LUA_REGISTRYINDEX, block_lua_func);
	// mobj to pass
	lua_pushlightuserdata(luaS_block_iter, thing);
	// parameter to pass
	lua_rawgeti(luaS_block_iter, LUA_REGISTRYINDEX, block_lua_arg);
	// do the call
	if(lua_pcall(luaS_block_iter, 2, LUA_MULTRET, 0))
		// script error
		return luaL_error(luaS_block_iter, "ThingsIterator: %s", lua_tostring(luaS_block_iter, -1));

	if(lua_gettop(luaS_block_iter) == 0)
		// no return means continue
		return true;

	// first return has to be continue flag
	luaL_checktype(luaS_block_iter, 1, LUA_TBOOLEAN);
	ret = lua_toboolean(luaS_block_iter, 1);

	if(ret)
		// iteration will continue, discard all returns
		lua_settop(luaS_block_iter, 0);
	else
		// remove only this flag, rest will get returned
		lua_remove(luaS_block_iter, 1);

	return ret;
}

