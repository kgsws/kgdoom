#ifndef __P_LOCAL__
#define __P_LOCAL__

#ifndef __R_LOCAL__
#include "r_local.h"
#endif

#define FLOATSPEED		(FRACUNIT*4)


#define MAXHEALTH		100

// mapblocks are used to check movement
// against lines and things
#define MAPBLOCKUNITS	128
#define MAPBLOCKSIZE	(MAPBLOCKUNITS*FRACUNIT)
#define MAPBLOCKSHIFT	(FRACBITS+7)
#define MAPBMASK		(MAPBLOCKSIZE-1)
#define MAPBTOFRAC		(MAPBLOCKSHIFT-FRACBITS)


// player radius for movement checking
#define PLAYERRADIUS	16*FRACUNIT

#define GRAVITY		FRACUNIT
#define MAXMOVE		(32*FRACUNIT)

// [kg] check valid position more often
// basicaly smallest radius that won't skip any collision
#define MAXMOVE_STEP	(8*FRACUNIT)

#define USERANGE		(64*FRACUNIT)
#define MELEERANGE		(64*FRACUNIT)
#define MISSILERANGE	(8192*FRACUNIT)

// follow a player exlusively for 3 seconds
#define	BASETHRESHOLD	 	100

// [kg] instat kill damage values, no thrust
#define INSTANTKILL	12345678
#define INSTANTGIB	12345679

// [kg] some levelinfo used for server -> client stuff
extern int level_lump;
// and this one for Lua
extern char level_name[9];

// [kg] moved here
#define STOPSPEED		0x1000
#define FRICTION		0xe800

//
// P_TICK
//

// both the head and tail of the thinker list
extern	thinker_t	thinkercap;	


void P_InitThinkers (void);
void P_AddThinker (thinker_t* thinker, luathinker_t type);
void P_RemoveThinker (thinker_t* thinker);


//
// P_PSPR
//
void P_SetupPsprites (player_t* curplayer);
void P_MovePsprites (player_t* curplayer);
void P_DropWeapon (player_t* player);


//
// P_USER
//
void	P_PlayerThink (player_t* player);


//
// P_MOBJ
//
#define ONFLOORZ		MININT
#define ONCEILINGZ		MAXINT

mobj_t*
P_SpawnMobj
( fixed_t	x,
  fixed_t	y,
  fixed_t	z,
  mobjtype_t	type );

mobj_t *P_MobjByNetId(int netid);

#ifdef SERVER
void 	P_RemoveMobj (mobj_t* th, boolean clientside);
#else
void 	P_RemoveMobj (mobj_t* th);
#endif
boolean	P_SetMobjState (mobj_t* mobj, statenum_t state);
boolean P_SetMobjAnimation(mobj_t *mobj, int anim, int skip);
void 	P_MobjThinker (mobj_t* mobj);

void	P_SpawnPuff (fixed_t x, fixed_t y, fixed_t z, mobj_t *origin, mobj_t *cause);
void 	P_SpawnBlood (fixed_t x, fixed_t y, fixed_t z, mobj_t *origin, mobj_t *cause);

// [kg] it's different now
mobj_t *P_SpawnMissile(mobj_t *source, mobjtype_t type, angle_t ango, fixed_t pio, fixed_t zo, fixed_t xo, fixed_t yo);

void P_SpawnPlayer (mapthing_hexen_t* mthing, int netplayer);

//
// P_ENEMY
//
void P_NoiseAlert (mobj_t* target, mobj_t* emmiter);


//
// P_MAPUTL
//
typedef struct
{
    fixed_t	x;
    fixed_t	y;
    fixed_t	dx;
    fixed_t	dy;
    
} divline_t;

typedef struct
{
    fixed_t	frac;		// along trace line
    boolean	isaline;
    union {
	mobj_t*	thing;
	line_t*	line;
    }			d;
} intercept_t;

#define MAXINTERCEPTS	256

extern intercept_t	intercepts[MAXINTERCEPTS];
extern intercept_t*	intercept_p;

typedef boolean (*traverser_t) (intercept_t *in);

fixed_t P_AproxDistance (fixed_t dx, fixed_t dy);
int 	P_PointOnLineSide (fixed_t x, fixed_t y, line_t* line);
int 	P_PointOnDivlineSide (fixed_t x, fixed_t y, divline_t* line);
void 	P_MakeDivline (line_t* li, divline_t* dl);
fixed_t P_InterceptVector (divline_t* v2, divline_t* v1);
int 	P_BoxOnLineSide (fixed_t* tmbox, line_t* ld);

extern fixed_t		opentop;
extern fixed_t 		openbottom;
extern fixed_t		openrange;
extern fixed_t		lowfloor;

void 	P_LineOpening (line_t* linedef);

boolean P_BlockLinesIterator (int x, int y, boolean(*func)(line_t*) );
boolean P_BlockThingsIterator (int x, int y, boolean(*func)(mobj_t*) );

#define PT_ADDLINES		1
#define PT_ADDTHINGS	2
#define PT_EARLYOUT		4

extern divline_t	trace;

boolean
P_PathTraverse
( fixed_t	x1,
  fixed_t	y1,
  fixed_t	x2,
  fixed_t	y2,
  int		flags,
  boolean	(*trav) (intercept_t *));

void P_UnsetThingPosition (mobj_t* thing);
void P_SetThingPosition (mobj_t* thing);


//
// P_MAP
//

// If "floatok" true, move would be ok
// if within "tmfloorz - tmceilingz".
extern boolean		floatok;
extern fixed_t		tmfloorz;
extern fixed_t		tmceilingz;


extern	line_t*		ceilingline;

boolean P_CheckPosition (mobj_t *thing, fixed_t x, fixed_t y);
boolean P_TryMove (mobj_t* thing, fixed_t x, fixed_t y);
boolean P_TeleportMove (mobj_t* thing, fixed_t x, fixed_t y);
void	P_SlideMove (mobj_t* mo);
boolean P_CheckSight (mobj_t* t1, mobj_t* t2);
void 	P_UseLines (player_t* player);

boolean P_ChangeSector(sector_t *sector, int lua_func, int lua_arg);

// [kg] thing Z clipping
void P_CheckPositionZ(mobj_t *thing);
// [kg] only to find foorz and ceilingz
void P_CheckPositionLines(mobj_t *thing);

extern mobj_t*	linetarget;	// who got hit (or NULL)

fixed_t
P_AimLineAttack
( mobj_t*	t1,
  angle_t	angle,
  fixed_t	distance,
  mobj_t *target );

void
P_LineAttack
( mobj_t*	t1,
  angle_t	angle,
  fixed_t	distance,
  fixed_t	slope,
  int		damage,
  fixed_t	zo,
  fixed_t	xo );

void
P_RadiusAttack
( mobj_t*	spot,
  mobj_t*	source,
  fixed_t	range,
  int		damage,
  boolean	hurtsource,
  int		damagetype );



//
// P_SETUP
//
extern byte*		rejectmatrix;	// for fast sight rejection
extern uint16_t*	blockmaplump;	// offsets in blockmap are from here
extern uint16_t*	blockmap;
extern int		bmapwidth;
extern int		bmapheight;	// in mapblocks
extern fixed_t		bmaporgx;
extern fixed_t		bmaporgy;	// origin of block map
extern blocklink_t**	blocklinks;	// for thing chains



//
// P_INTER
//

void
P_DamageMobj
( mobj_t*	target,
  mobj_t*	inflictor,
  mobj_t*	source,
  int		damage,
  int		damagetype );


//
// P_SPEC
//
#include "p_spec.h"

//
// P_ENEMY

boolean P_CheckMeleeRange (mobj_t *actor, mobj_t *target, boolean zCheck);

//
// P_MAP

mobjtype_t la_pufftype;

// [kg] weapons
void P_BringUpWeapon(player_t* player);
void P_WeaponRefire(player_t *player, int offset);
void P_WeaponFlash(player_t *player, int offset);

// [kg] all provided state functions
void A_SoundSee(mobj_t* actor);
void A_SoundAttack(mobj_t* actor);
void A_SoundPain(mobj_t* actor);
void A_SoundActive(mobj_t* actor);
void A_SoundDeath(mobj_t* actor);
void A_SoundXDeath(mobj_t* actor);

void A_Fall(mobj_t *actor);
void A_FaceTarget(mobj_t *actor);
void A_Look(mobj_t *actor);
void A_Chase(mobj_t *actor);
void A_VileChase(mobj_t *actor);

void A_WeaponRaise(mobj_t *actor);
void A_WeaponReady(mobj_t *actor);
void A_WeaponLower(mobj_t *actor);
void A_WeaponFlash(mobj_t *mo);
void A_WeaponRefire(mobj_t *mo);

void A_NoiseAlert(mobj_t *actor);

sector_t*
getNextSector
( line_t*	line,
  sector_t*	sec );

#endif	// __P_LOCAL__

