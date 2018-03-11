#include <stdlib.h>

#include "doomdef.h"

#include "m_random.h"
#include "i_system.h"

#include "doomdef.h"
#include "p_local.h"

#include "s_sound.h"

#include "g_game.h"

// State.
#include "doomstat.h"
#include "r_state.h"

#ifdef SERVER
#include <netinet/in.h>
#include "network.h"
#include "sv_cmds.h"
#endif

typedef enum
{
    DI_EAST,
    DI_NORTHEAST,
    DI_NORTH,
    DI_NORTHWEST,
    DI_WEST,
    DI_SOUTHWEST,
    DI_SOUTH,
    DI_SOUTHEAST,
    DI_NODIR,
    NUMDIRS
    
} dirtype_t;


//
// P_NewChaseDir related LUT.
//
dirtype_t opposite[] =
{
  DI_WEST, DI_SOUTHWEST, DI_SOUTH, DI_SOUTHEAST,
  DI_EAST, DI_NORTHEAST, DI_NORTH, DI_NORTHWEST, DI_NODIR
};

dirtype_t diags[] =
{
    DI_NORTHWEST, DI_NORTHEAST, DI_SOUTHWEST, DI_SOUTHEAST
};





void A_Fall (mobj_t *actor);


//
// ENEMY THINKING
// Enemies are allways spawned
// with targetplayer = -1, threshold = 0
// Most monsters are spawned unaware of all players,
// but some can be made preaware
//


//
// Called by P_NoiseAlert.
// Recursively traverse adjacent sectors,
// sound blocking lines cut off traversal.
//

mobj_t*		soundtarget;

void
P_RecursiveSound
( sector_t*	sec,
  int		soundblocks )
{
    int		i;
    line_t*	check;
    sector_t*	other;
	
    // wake up all monsters in this sector
    if (sec->validcount == validcount
	&& sec->soundtraversed <= soundblocks+1)
    {
	return;		// already flooded
    }
    
    sec->validcount = validcount;
    sec->soundtraversed = soundblocks+1;
    sec->soundtarget = soundtarget;
	
    for (i=0 ;i<sec->linecount ; i++)
    {
	check = sec->lines[i];
	if (! (check->flags & LF_TWOSIDED) )
	    continue;
	
	P_LineOpening (check);

	if (openrange <= 0)
	    continue;	// closed door
	
	if ( sides[ check->sidenum[0] ].sector == sec)
	    other = sides[ check->sidenum[1] ] .sector;
	else
	    other = sides[ check->sidenum[0] ].sector;
	
	if (check->flags & LF_SOUNDBLOCK)
	{
	    if (!soundblocks)
		P_RecursiveSound (other, 1);
	}
	else
	    P_RecursiveSound (other, soundblocks);
    }
}



//
// P_NoiseAlert
// If a monster yells at a player,
// it will alert other monsters to the player.
//
void
P_NoiseAlert
( mobj_t*	target,
  mobj_t*	emmiter )
{
    soundtarget = target;
    validcount++;
    P_RecursiveSound (emmiter->subsector->sector, 0);
}

// [kg] action code
void A_NoiseAlert(mobj_t *target)
{
    soundtarget = target;
    validcount++;
    P_RecursiveSound(target->subsector->sector, 0);
}

//
// P_CheckMeleeRange
//
boolean P_CheckMeleeRange (mobj_t *actor, mobj_t *target, boolean zCheck)
{
    fixed_t	dist;

    if (!target)
	return false;

    dist = P_AproxDistance (target->x-actor->x, target->y-actor->y);

    if (dist >= MELEERANGE-20*FRACUNIT+target->radius)
	return false;

    if(zCheck && actor->z >= target->z + target->height)
	return false;

    if(zCheck && target->z > actor->z + actor->height)
	return false;
	
    if (! P_CheckSight (actor, actor->target) )
	return false;
							
    return true;		
}

//
// P_CheckMissileRange
//
boolean P_CheckMissileRange (mobj_t* actor)
{
    fixed_t	dist;
	
    if (! P_CheckSight (actor, actor->target) )
	return false;
	
    if ( actor->flags & MF_JUSTHIT )
    {
	// the target just hit the enemy,
	// so fight back!
	actor->flags &= ~MF_JUSTHIT;
	return true;
    }
	
    if (actor->reactiontime)
	return false;	// do not attack yet
		
    // OPTIMIZE: get this from a global checksight
    dist = P_AproxDistance ( actor->x-actor->target->x,
			     actor->y-actor->target->y) - 64*FRACUNIT;
    
    if (!actor->info->meleestate)
	dist -= 128*FRACUNIT;	// no melee attack, so fire more

    dist >>= 16;

// TODO: mobj type config
/*    if (actor->type == MT_VILE)
    {
	if (dist > 14*64)	
	    return false;	// too far away
    }
	

    if (actor->type == MT_UNDEAD)
    {
	if (dist < 196)	
	    return false;	// close for fist attack
	dist >>= 1;
    }
	

    if (actor->type == MT_CYBORG
	|| actor->type == MT_SPIDER
	|| actor->type == MT_SKULL)
    {
	dist >>= 1;
    }
*/    
    if (dist > 200)
	dist = 200;
		
//    if (actor->type == MT_CYBORG && dist > 160)
//	dist = 160;

    if (P_Random () < dist)
	return false;
		
    return true;
}


//
// P_Move
// Move in the current direction,
// returns false if the move is blocked.
//
fixed_t	xspeed[8] = {FRACUNIT,47000,0,-47000,-FRACUNIT,-47000,0,47000};
fixed_t yspeed[8] = {0,47000,FRACUNIT,47000,0,-47000,-FRACUNIT,-47000};

#define MAXSPECIALCROSS	8

extern	line_t*	spechit[MAXSPECIALCROSS];
extern	int	numspechit;

boolean P_Move (mobj_t*	actor)
{
    fixed_t	tryx;
    fixed_t	tryy;
    
    line_t*	ld;
    
    // warning: 'catch', 'throw', and 'try'
    // are all C++ reserved words
    boolean	try_ok;
    boolean	good;
		
    if (actor->movedir >= DI_NODIR)
	return false;

    tryx = actor->x + FixedMul(actor->info->speed, xspeed[actor->movedir]);
    tryy = actor->y + FixedMul(actor->info->speed, yspeed[actor->movedir]);

    try_ok = P_TryMove (actor, tryx, tryy);

    if (!try_ok)
    {
	// open any specials
	if (actor->flags & MF_FLOAT && floatok)
	{
	    // must adjust height
	    if (actor->z < tmfloorz)
		actor->z += FLOATSPEED;
	    else
		actor->z -= FLOATSPEED;

	    P_ZMovement(actor);

	    actor->flags |= MF_INFLOAT;
	    return true;
	}
		
	if (!numspechit)
	    return false;
			
	actor->movedir = DI_NODIR;
	good = false;
	while (numspechit--)
	{
	    ld = spechit[numspechit];
	    // if the special is not a door
	    // that can be opened,
	    // return false
	    if(P_ExtraLineSpecial(actor, ld, !P_PointOnLineSide(tryx, tryy, ld), EXTRA_USE))
		good = true;
	}
	return good;
    }
    else
    {
	actor->flags &= ~MF_INFLOAT;
    }
	
	
    if (! (actor->flags & MF_FLOAT) )	
	actor->z = actor->floorz;
    return true; 
}


//
// TryWalk
// Attempts to move actor on
// in its current (ob->moveangle) direction.
// If blocked by either a wall or an actor
// returns FALSE
// If move is either clear or blocked only by a door,
// returns TRUE and sets...
// If a door is in the way,
// an OpenDoor call is made to start it opening.
//
boolean P_TryWalk (mobj_t* actor)
{	
    if (!P_Move (actor))
    {
	return false;
    }

    actor->movecount = P_Random()&15;
    return true;
}




void P_NewChaseDir (mobj_t*	actor)
{
    fixed_t	deltax;
    fixed_t	deltay;
    
    dirtype_t	d[3];
    
    int		tdir;
    dirtype_t	olddir;
    
    dirtype_t	turnaround;

    if (!actor->target)
	I_Error ("P_NewChaseDir: called with no target");
		
    olddir = actor->movedir;
    turnaround=opposite[olddir];

    deltax = actor->target->x - actor->x;
    deltay = actor->target->y - actor->y;

    if (deltax>10*FRACUNIT)
	d[1]= DI_EAST;
    else if (deltax<-10*FRACUNIT)
	d[1]= DI_WEST;
    else
	d[1]=DI_NODIR;

    if (deltay<-10*FRACUNIT)
	d[2]= DI_SOUTH;
    else if (deltay>10*FRACUNIT)
	d[2]= DI_NORTH;
    else
	d[2]=DI_NODIR;

    // try direct route
    if (d[1] != DI_NODIR
	&& d[2] != DI_NODIR)
    {
	actor->movedir = diags[((deltay<0)<<1)+(deltax>0)];
	if (actor->movedir != turnaround && P_TryWalk(actor))
	    return;
    }

    // try other directions
    if (P_Random() > 200
	||  abs(deltay)>abs(deltax))
    {
	tdir=d[1];
	d[1]=d[2];
	d[2]=tdir;
    }

    if (d[1]==turnaround)
	d[1]=DI_NODIR;
    if (d[2]==turnaround)
	d[2]=DI_NODIR;
	
    if (d[1]!=DI_NODIR)
    {
	actor->movedir = d[1];
	if (P_TryWalk(actor))
	{
	    // either moved forward or attacked
	    return;
	}
    }

    if (d[2]!=DI_NODIR)
    {
	actor->movedir =d[2];

	if (P_TryWalk(actor))
	    return;
    }

    // there is no direct path to the player,
    // so pick another direction.
    if (olddir!=DI_NODIR)
    {
	actor->movedir =olddir;

	if (P_TryWalk(actor))
	    return;
    }

    // randomly determine direction of search
    if (P_Random()&1) 	
    {
	for ( tdir=DI_EAST;
	      tdir<=DI_SOUTHEAST;
	      tdir++ )
	{
	    if (tdir!=turnaround)
	    {
		actor->movedir =tdir;
		
		if ( P_TryWalk(actor) )
		    return;
	    }
	}
    }
    else
    {
	for ( tdir=DI_SOUTHEAST;
	      tdir != (DI_EAST-1);
	      tdir-- )
	{
	    if (tdir!=turnaround)
	    {
		actor->movedir =tdir;
		
		if ( P_TryWalk(actor) )
		    return;
	    }
	}
    }

    if (turnaround !=  DI_NODIR)
    {
	actor->movedir =turnaround;
	if ( P_TryWalk(actor) )
	    return;
    }

    actor->movedir = DI_NODIR;	// can not move
}



//
// P_LookForPlayers
// If allaround is false, only look 180 degrees in front.
// Returns true if a player is targeted.
//
boolean
P_LookForPlayers
( mobj_t*	actor,
  boolean	allaround )
{
    int		c;
    int		stop;
    player_t*	player;
    sector_t*	sector;
    angle_t	an;
    fixed_t	dist;

    if(!playercount)
	return false;

    sector = actor->subsector->sector;
	
    c = 0;
    stop = (actor->lastlook-1)&3;
	
    for ( ; ; actor->lastlook = (actor->lastlook+1)&3 )
    {
	if (!playeringame[actor->lastlook])
	    continue;
			
	if (c++ == 2
	    || actor->lastlook == stop)
	{
	    // done looking
	    return false;	
	}
	
	player = &players[actor->lastlook];

	if (!player->mo || player->mo->health <= 0)
	    continue;		// dead

	if (!P_CheckSight (actor, player->mo))
	    continue;		// out of sight
			
	if (!allaround)
	{
	    an = R_PointToAngle2 (actor->x,
				  actor->y, 
				  player->mo->x,
				  player->mo->y)
		- actor->angle;
	    
	    if (an > ANG90 && an < ANG270)
	    {
		dist = P_AproxDistance (player->mo->x - actor->x,
					player->mo->y - actor->y);
		// if real close, react anyway
		if (dist > MELEERANGE)
		    continue;	// behind back
	    }
	}
		
	actor->target = player->mo;
	return true;
    }

    return false;
}

//
// ACTION ROUTINES
//

//
// A_Look
// Stay in state until a player is sighted.
//
void A_Look (mobj_t* actor)
{
#ifndef SERVER
    if(netgame)
	return;
#endif
    mobj_t*	targ;
	
    actor->threshold = 0;	// any shot will wake up
    targ = actor->subsector->sector->soundtarget;

    if (targ
	&& (targ->flags & MF_SHOOTABLE) )
    {
	actor->target = targ;

	if ( actor->flags & MF_AMBUSH )
	{
	    if (P_CheckSight (actor, actor->target))
		goto seeyou;
	}
	else
	    goto seeyou;
    }
	
	
    if (!P_LookForPlayers (actor, false) )
	return;
		
    // go into chase state
  seeyou:
    if (actor->info->seesound)
	S_StartSound (actor, actor->info->seesound, SOUND_BODY);

    P_SetMobjAnimation(actor, ANIM_SEE, 0);
#ifdef SERVER
    // tell clients about this
    SV_UpdateMobj(actor, SV_MOBJF_STATE | SV_MOBJF_TARGET | SV_MOBJF_SOUND_SEE);
#endif
}


//
// A_Chase
// Actor has a melee attack,
// so it tries to close as fast as possible
//
void A_Chase (mobj_t*	actor)
{
    int		delta;
    int movedir = actor->movedir;

#ifndef SERVER
    if(netgame)
    {
	// turn towards movement direction if not there yet
	if (actor->movedir < 8)
	{
	    actor->angle &= (7<<29);
	    delta = actor->angle - (actor->movedir << 29);
	    if (delta > 0)
		actor->angle -= ANG90/2;
	    else if (delta < 0)
		actor->angle += ANG90/2;
	}
	// chase towards player, use last known direction, do nothing else
	P_Move(actor);
	return;
    }
#endif

    if (actor->reactiontime)
	actor->reactiontime--;
				
    // modify target threshold
    if  (actor->threshold)
    {
	if (!actor->target
	    || actor->target->health <= 0)
	{
	    actor->threshold = 0;
	}
	else
	    actor->threshold--;
    }
    
    // turn towards movement direction if not there yet
    if (actor->movedir < 8)
    {
	actor->angle &= (7<<29);
	delta = actor->angle - (actor->movedir << 29);
	
	if (delta > 0)
	    actor->angle -= ANG90/2;
	else if (delta < 0)
	    actor->angle += ANG90/2;
    }

    if (!actor->target
	|| !(actor->target->flags&MF_SHOOTABLE))
    {
	// look for a new target
	if (P_LookForPlayers(actor,true))
	    return; 	// got a new target
	
	P_SetMobjAnimation(actor, ANIM_SPAWN, 0);
#ifdef SERVER
	// tell clients about this
	SV_UpdateMobj(actor, SV_MOBJF_POSITION | SV_MOBJF_FLOORZ | SV_MOBJF_CEILZ | SV_MOBJF_ANGLE | SV_MOBJF_STATE | SV_MOBJF_TARGET);
#endif
	return;
    }
    
    // do not attack twice in a row
    if (actor->flags & MF_JUSTATTACKED)
    {
	actor->flags &= ~MF_JUSTATTACKED;
	if (gameskill != sk_nightmare && !fastparm)
	{
	    P_NewChaseDir (actor);
#ifdef SERVER
	    if(movedir != actor->movedir)
		// tell clients about this
		SV_UpdateMobj(actor, SV_MOBJF_POSITION | SV_MOBJF_FLOORZ | SV_MOBJF_CEILZ | SV_MOBJF_MDIR);
#endif
	}
	return;
    }
    
    // check for melee attack
    if (actor->info->meleestate
	&& P_CheckMeleeRange (actor, actor->target, true))
    {
	P_SetMobjAnimation(actor, ANIM_MELEE, 0);
#ifdef SERVER
	// tell clients about this
	SV_UpdateMobj(actor, SV_MOBJF_POSITION | SV_MOBJF_FLOORZ | SV_MOBJF_CEILZ | SV_MOBJF_ANGLE | SV_MOBJF_STATE | SV_MOBJF_TARGET | SV_MOBJF_SOUND_ATTACK);
#endif
	return;
    }
    
    // check for missile attack
    if (actor->info->missilestate)
    {
	if (gameskill < sk_nightmare
	    && !fastparm && actor->movecount)
	{
	    goto nomissile;
	}
	
	if (!P_CheckMissileRange (actor))
	    goto nomissile;
	
	P_SetMobjAnimation(actor, ANIM_MISSILE, 0);
	actor->flags |= MF_JUSTATTACKED;
#ifdef SERVER
	// tell clients about this
	SV_UpdateMobj(actor, SV_MOBJF_POSITION | SV_MOBJF_FLOORZ | SV_MOBJF_CEILZ | SV_MOBJF_ANGLE | SV_MOBJF_STATE | SV_MOBJF_TARGET);
#endif
	return;
    }

    // ?
  nomissile:
    // possibly choose another target
    if (netgame
	&& !actor->threshold
	&& !P_CheckSight (actor, actor->target) )
    {
	if (P_LookForPlayers(actor,true))
	    return;	// got a new target
    }
    
    // chase towards player
    if (--actor->movecount<0
	|| !P_Move (actor))
    {
	P_NewChaseDir (actor);
#ifdef SERVER
	if(movedir != actor->movedir)
	    // tell clients about this
	    SV_UpdateMobj(actor, SV_MOBJF_POSITION | SV_MOBJF_FLOORZ | SV_MOBJF_CEILZ |  SV_MOBJF_MDIR);
#endif
    }
    
    // make active sound
    if (actor->info->activesound
	&& P_Random () < 3)
    {
	S_StartSound (actor, actor->info->activesound, SOUND_BODY);
    }
}


//
// A_FaceTarget
//
void A_FaceTarget (mobj_t* actor)
{	
    if (!actor->target)
	return;

    actor->flags &= ~MF_AMBUSH;
	
    actor->angle = R_PointToAngle2 (actor->x,
				    actor->y,
				    actor->target->x,
				    actor->target->y);

#ifndef SERVER
	// [kg] new cheats
	if(actor->target->player)
	{
		mobj_t *fog;
		switch(actor->target->player->cheats & CF_AURAMASK)
		{
			case CF_DEATHAURA:
				P_DamageMobj(actor, actor->target, actor->target, INSTANTKILL, NUMDAMAGETYPES);
			break;
			case CF_SAFEAURA:
				if(actor->flags & MF_COUNTKILL)
					actor->target->player->killcount++;	
//				fog = P_SpawnMobj(actor->x, actor->y, actor->z, MT_TFOG);
				P_RemoveMobj(actor);
//				S_StartSound(fog, sfx_telept, SOUND_BODY);
			break;
		}
	}
#endif
}

void A_SoundSee(mobj_t* actor)
{
	if(actor->info->seesound)
		S_StartSound(actor, actor->info->seesound, SOUND_BODY);
}

void A_SoundAttack(mobj_t* actor)
{
	if(actor->info->attacksound)
		S_StartSound(actor, actor->info->attacksound, SOUND_WEAPON);
}

void A_SoundPain(mobj_t* actor)
{
	if(actor->info->painsound)
		S_StartSound(actor, actor->info->painsound, SOUND_BODY);
}

void A_SoundActive(mobj_t* actor)
{
	if(actor->info->activesound)
		S_StartSound(actor, actor->info->activesound, SOUND_BODY);
}

void A_SoundDeath(mobj_t* actor)
{
	if(actor->info->deathsound)
		S_StartSound(actor, actor->info->deathsound, SOUND_BODY);
}

void A_SoundXDeath(mobj_t* actor)
{
	if(actor->info->xdeathsound)
		S_StartSound(actor, actor->info->xdeathsound, SOUND_BODY);
}

void A_Fall (mobj_t *actor)
{
    // actor is on ground, it can be walked over
    actor->flags &= ~MF_SOLID;

    // So change this if corpse objects
    // are meant to be obstacles.
}

