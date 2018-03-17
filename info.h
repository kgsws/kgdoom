#ifndef __INFO__
#define __INFO__

// Needed for action function pointer handling.
#include "doomdef.h"
#include "d_think.h"

typedef int spritenum_t;

typedef union
{
	char t[4];
	uint32_t u;
} sprname_t;

#define INFO_SPRITE_ALLOC	128
int numsnames;
extern sprname_t *sprnames;

#define INFO_STATE_ALLOC	512
extern int numstates;
typedef uint32_t statenum_t;

typedef struct
{
	spritenum_t sprite;
	uint16_t frame;
	short tics;
	int func; // [kg] LUA registry index
	statenum_t nextstate;
//	long misc1, misc2;
} state_t;

extern state_t *states;

// [kg] animation index
enum
{
	ANIM_SPAWN, // must be first
	ANIM_SEE,
	ANIM_PAIN,
	ANIM_MELEE,
	ANIM_MISSILE,
	ANIM_DEATH,
	ANIM_XDEATH,
	ANIM_RAISE,
	ANIM_CRUSH,
	ANIM_HEAL,
	ANIM_WPN_RAISE,
	ANIM_WPN_READY,
	ANIM_WPN_LOWER,
	ANIM_WPN_FIRE_MAIN,
	ANIM_WPN_FIRE_ALT,
	ANIM_WPN_FLASH_MAIN,
	ANIM_WPN_FLASH_ALT
};

typedef struct
{
	// [kg] must be first
	degenthinker_t dthink;

	// [kg] keep same order as in 'lua_mobjtype' and animation index enum
	int	spawnstate; // must be first
	int	seestate;
	int	painstate;
	int	meleestate;
	int	missilestate;
	int	deathstate;
	int	xdeathstate;
	int	raisestate;
	int	crushstate;
	int	healstate;
	int	weaponraise;
	int	weaponready;
	int	weaponlower;
	int	weaponfiremain;
	int	weaponfirealt;
	int	weaponflashmain;
	int	weaponflashalt;

	// [kg] keep same order as in 'lua_mobjtype'
	int	seesound; // must be first
	int	attacksound;
	int	painsound;
	int	activesound; // also pickup sound
	int	deathsound;
	int	xdeathsound;

	int	doomednum;
	int	spawnhealth;
	int	reactiontime;
	int	painchance;
	int	speed;
	int	radius;
	int	height;
	int	mass;
	int	damage;
	int	stepheight;
	int	gravity;
	uint64_t	flags;

	// [kg] inventory stuff
	int maxcount;

	// [kg] some new stuff
	int species;
	int blocking;
	int canpass;
	int viewz;
	int shootz;
	int bobz;
	int icon;

	// [kg] renderer
	int renderstyle;
	uint8_t *rendertable;

	// [kg] damage resistance
	int damagetype;
	uint8_t	damagescale[NUMDAMAGETYPES];

	// [kg] some Lua stuff
	int lua_action;
	int lua_arg;
	// [kg] Lua; actor hit floor
	int crash_action;
} mobjinfo_t;

extern int numobjtypes;
extern mobjinfo_t *mobjinfo;

typedef int mobjtype_t;

// some to-be-set types
extern int MT_PLAYER;

// TODO: fix these
#define MT_IFOG 0
#define MT_PUFF 0
#define MT_BLOOD 0

//
// few always present types

// sprites
enum
{
	SPR_UNKN,
	NUM_DEF_SPRITES
};

// states
enum
{
	S_NULL,
	S_UNKNOWN,
	S_ITEMRESPAWN0,
	S_ITEMRESPAWN1,
	NUM_DEF_STATES
};

// mobj types
enum
{
	MT_UNKNOWN,
	NUM_DEF_MOBJTYPES
};

extern const sprname_t info_def_sprnames[NUM_DEF_SPRITES];
extern const state_t info_def_states[NUM_DEF_STATES];
extern const mobjinfo_t info_def_mobjinfo[NUM_DEF_MOBJTYPES];


#endif

