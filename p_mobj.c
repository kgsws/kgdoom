#include "doomdef.h"

#include "i_system.h"
#include "z_zone.h"
#include "m_random.h"

#include "p_local.h"

#include "st_stuff.h"
#include "hu_stuff.h"

#include "s_sound.h"

#include "doomstat.h"

// [kg] LUA support
#include "kg_lua.h"

#ifdef SERVER
#include <netinet/in.h>
#include "network.h"
#include "sv_cmds.h"
#else
#include "cl_cmds.h"
#endif

// [kg] keep track of projectile hits
extern mobj_t *hitmobj;

void G_PlayerReborn (int player);
void P_SpawnMapThing (mapthing_hexen_t*	mthing);

//
// P_SetMobjState
// Returns true if the mobj is still present.
//
int test;

boolean
P_SetMobjState
( mobj_t*	mobj,
  statenum_t	state )
{
    state_t*	st;

    do
    {
	if (state == S_NULL)
	{
	    mobj->state = (state_t *) S_NULL;
#ifdef SERVER
	    P_RemoveMobj (mobj, false);
#else
	    P_RemoveMobj (mobj);
#endif
	    return false;
	}

	// [kg] animation 'alias'
	if(state & STATE_ANIMATION)
		state = L_StateFromAlias(mobj, state);

	st = &states[state];
	mobj->state = st;
	mobj->tics = st->tics;
	mobj->sprite = st->sprite;
	mobj->frame = st->frame;

	// [kg] call LUA function
	L_StateCall(st, mobj);
	// [kg] state might have changed
	state = mobj->state->nextstate;
    } while (!mobj->tics);
				
    return true;
}


//
// P_ExplodeMissile  
//
void P_ExplodeMissile (mobj_t* mo)
{
    mo->momx = mo->momy = mo->momz = 0;

    if(mo->info->deathstate)
    {
	P_SetMobjState (mo, mo->info->deathstate);

	mo->tics -= P_Random()&3;

	if (mo->tics < 1)
	    mo->tics = 1;
    }

    mo->flags &= ~MF_MISSILE;

    if (mo->info->deathsound)
	S_StartSound (mo, mo->info->deathsound, SOUND_BODY);
}


//
// P_XYMovement  
// [kg] check valid position more often
// MAXMOVE_STEP is basicaly smallest radius that won't skip any collision
//

void P_XYMovement (mobj_t* mo) 
{ 	
    fixed_t 	ptryx;
    fixed_t	ptryy;
    player_t*	player;
    fixed_t	xmove;
    fixed_t	ymove;

    int stepdir = 0;

    if (!mo->momx && !mo->momy)
    {
	if (mo->flags & MF_SKULLFLY)
	{
	    // the skull slammed into something
	    mo->flags &= ~MF_SKULLFLY;
	    mo->momx = mo->momy = mo->momz = 0;

	    P_SetMobjState (mo, mo->info->spawnstate);
	}
	return;
    }
	
    player = mo->player;
		
    if (mo->momx > MAXMOVE)
	mo->momx = MAXMOVE;
    else if (mo->momx < -MAXMOVE)
	mo->momx = -MAXMOVE;

    if (mo->momy > MAXMOVE)
	mo->momy = MAXMOVE;
    else if (mo->momy < -MAXMOVE)
	mo->momy = -MAXMOVE;
		
    xmove = mo->momx;
    ymove = mo->momy;

    if(xmove < 0)
    {
	xmove = -xmove;
	stepdir = 1;
    }
    if(ymove < 0)
    {
	ymove = -ymove;
	stepdir |= 2;
    }

    do
    {
	fixed_t stepx, stepy;

	stepx = xmove > MAXMOVE_STEP ? MAXMOVE_STEP : xmove;
	stepy = ymove > MAXMOVE_STEP ? MAXMOVE_STEP : ymove;

	if(stepdir & 1)
	    ptryx = mo->x - stepx;
	else
	    ptryx = mo->x + stepx;

	if(stepdir & 2)
	    ptryy = mo->y - stepy;
	else
	    ptryy = mo->y + stepy;

	xmove -= stepx;
	ymove -= stepy;

	hitmobj = NULL;
	if (!P_TryMove (mo, ptryx, ptryy))
	{
	    // blocked move
	    if (mo->flags & MF_SLIDE)
	    {	// try to slide along it
		P_SlideMove (mo);
		// [kg] done moving
		break;
	    }
	    else if (mo->flags & MF_MISSILE)
	    {
		if(ceilingline)
		{
		    sector_t *sec;
		    if(P_PointOnLineSide(mo->x, mo->y, ceilingline))
			sec = ceilingline->frontsector;
		    else
			sec = ceilingline->backsector;
		    // explode a missile
		    if(sec->ceilingpic == skyflatnum && mo->z + mo->height >= sec->ceilingheight)
		    {
			// Hack to prevent missiles exploding
			// against the sky.
			// Does not handle sky floors.
#ifdef SERVER
			P_RemoveMobj (mo, false);
#else
			P_RemoveMobj (mo);
#endif
			return;
		    }
		}
		P_ExplodeMissile (mo);
#ifdef SERVER
		// [kg] explode clientside projectile
		if(hitmobj)
		    SV_UpdateMobj(mo, SV_MOBJF_POSITION | SV_MOBJF_MOMENTNUM | SV_MOBJF_FLAGS | SV_MOBJF_STATE | SV_MOBJF_SOUND_DEATH);
#endif
	    }
	    else
		mo->momx = mo->momy = 0;
	}
    } while (xmove || ymove);
    
    // slow down
    if (player && player->cheats & CF_NOMOMENTUM)
    {
	// debug option for no sliding at all
	mo->momx = mo->momy = 0;
	return;
    }

    if (mo->flags & (MF_MISSILE | MF_SKULLFLY) )
	return; 	// no friction for missiles ever
		
    if (mo->z > mo->floorz)
	return;		// no friction when airborne

    if (mo->flags & MF_CORPSE)
    {
	// do not stop sliding
	//  if halfway off a step with some momentum
	if (mo->momx > FRACUNIT/4
	    || mo->momx < -FRACUNIT/4
	    || mo->momy > FRACUNIT/4
	    || mo->momy < -FRACUNIT/4)
	{
	    if (mo->floorz != mo->subsector->sector->floorheight)
		return;
	}
    }

    if (mo->momx > -STOPSPEED
	&& mo->momx < STOPSPEED
	&& mo->momy > -STOPSPEED
	&& mo->momy < STOPSPEED
	&& (!player
	    || (player->cmd.forwardmove== 0
		&& player->cmd.sidemove == 0 ) ) )
    {
#ifndef SERVER
	if(!netgame)
#endif
	// if in a walking frame, stop moving
//	if ( player&&(unsigned)((player->mo->state - states)- S_PLAY_RUN1) < 4)
//	    P_SetMobjState (player->mo, S_PLAY);
	// TODO: handle player animation
	
	mo->momx = 0;
	mo->momy = 0;
    }
    else
    {
	mo->momx = FixedMul (mo->momx, FRICTION);
	mo->momy = FixedMul (mo->momy, FRICTION);
    }
}

//
// P_ZMovement
//
void P_ZMovement (mobj_t* mo)
{
    fixed_t	dist;
    fixed_t	delta;
    
    // check for smooth step up
#ifdef SERVER
    if(mo->player && mo->z < mo->floorz)
#else
    if(!local_player_predict && mo->player && mo->z < mo->floorz)
#endif
    {
	mo->player->viewheight -= mo->floorz-mo->z;

	mo->player->deltaviewheight = (mo->info->viewz - mo->player->viewheight)>>3;
    }

    // adjust height
    mo->z += mo->momz;
	
    if ( mo->flags & MF_FLOAT
	 && mo->target)
    {
	// float down towards target if too close
	if ( !(mo->flags & MF_SKULLFLY)
	     && !(mo->flags & MF_INFLOAT) )
	{
	    dist = P_AproxDistance (mo->x - mo->target->x,
				    mo->y - mo->target->y);
	    
	    delta =(mo->target->z + (mo->height>>1)) - mo->z;

	    if (delta<0 && dist < -(delta*3) )
		mo->z -= FLOATSPEED;
	    else if (delta>0 && dist < (delta*3) )
		mo->z += FLOATSPEED;			
	}
	
    }
    
    // clip movement
    if (mo->z <= mo->floorz)
    {
	// hit the floor

	if (mo->flags & MF_SKULLFLY)
	{
	    // the skull slammed into something
	    mo->momz = -mo->momz;
	}

	if (mo->momz < 0)
	{
#ifdef SERVER
	    if (mo->player && mo->health > 0
#else
	    if (!local_player_predict && mo->player && mo->health > 0
#endif
		&& mo->momz < -GRAVITY*8 &&
		!(mo->player->cheats & CF_SPECTATOR)
	    ){
		// Squat down.
		// Decrease viewheight for a moment
		// after hitting the ground (hard),
		// and utter appropriate sound.
		mo->player->deltaviewheight = mo->momz>>3;
//		S_StartSound (mo, sfx_oof, SOUND_BODY);
	    }
	    if(!(mo->flags & MF_NOGRAVITY)) // [kg] added no gravity check
		mo->momz = 0;
	}
	mo->z = mo->floorz;

	if ( (mo->flags & MF_MISSILE)
	     && !(mo->flags & MF_NOCLIP) )
	{
	    if(mo->subsector->sector->floorpic == skyflatnum)
#ifdef SERVER
		P_RemoveMobj (mo, false);
#else
		P_RemoveMobj (mo);
#endif
	    else
		P_ExplodeMissile (mo);
	    return;
	}
    }
    else if (! (mo->flags & MF_NOGRAVITY) )
    {
	if (mo->momz == 0)
	    mo->momz = -GRAVITY*2;
	else
	    mo->momz -= GRAVITY;
    }
	
    if (mo->z + mo->height > mo->ceilingz)
    {
	// hit the ceiling
	if (mo->momz > 0 && !(mo->flags & MF_NOGRAVITY))  // [kg] added no gravity check
	    mo->momz = 0;

	mo->z = mo->ceilingz - mo->height;

	if (mo->flags & MF_SKULLFLY)
	{	// the skull slammed into something
	    mo->momz = -mo->momz;
	}
	
	if ( (mo->flags & MF_MISSILE)
	     && !(mo->flags & MF_NOCLIP) )
	{
	    if(mo->subsector->sector->ceilingpic == skyflatnum)
#ifdef SERVER
		P_RemoveMobj (mo, false);
#else
		P_RemoveMobj (mo);
#endif
	    else
		P_ExplodeMissile (mo);
	    return;
	}
    }
} 



//
// P_NightmareRespawn
//
void
P_NightmareRespawn (mobj_t* mobj)
{
    fixed_t		x;
    fixed_t		y;
    fixed_t		z; 
    subsector_t*	ss; 
    mobj_t*		mo;
    mapthing_hexen_t*		mthing;
		
    x = mobj->spawnpoint.x << FRACBITS; 
    y = mobj->spawnpoint.y << FRACBITS; 

    // somthing is occupying it's position?
    if (!P_CheckPosition (mobj, x, y) ) 
	return;	// no respwan

    // spawn a teleport fog at old spot
    // because of removal of the body?
    mo = P_SpawnMobj (mobj->x,
		      mobj->y,
		      mobj->subsector->sector->floorheight , MT_TFOG);
    // initiate teleport sound
    if(mo->info->seesound)
	S_StartSound (mo, mo->info->seesound, SOUND_BODY);
#ifdef SERVER
    // tell clients about this
    SV_SpawnMobj(mo, SV_MOBJF_SOUND_SEE);
#endif

    // spawn a teleport fog at the new spot
    ss = R_PointInSubsector (x,y); 

    mo = P_SpawnMobj (x, y, ss->sector->floorheight , MT_TFOG);
    // initiate teleport sound
    if(mo->info->seesound)
	S_StartSound (mo, mo->info->seesound, SOUND_BODY);
#ifdef SERVER
    // tell clients about this
    SV_SpawnMobj(mo, SV_MOBJF_SOUND_SEE);
#endif

    // spawn the new monster
    mthing = &mobj->spawnpoint;
	
    // spawn it
    if (mobj->info->flags & MF_SPAWNCEILING)
	z = ONCEILINGZ;
    else
	z = ONFLOORZ;

    // inherit attributes from deceased one
    mo = P_SpawnMobj (x,y,z, mobj->type);
    mo->spawnpoint = mobj->spawnpoint;	
    mo->angle = ANG45 * (mthing->angle/45);

    if (mthing->flags & MTF_AMBUSH)
	mo->flags |= MF_AMBUSH;

    mo->reactiontime = 18;
#ifdef SERVER
    // tell clients about this
    SV_SpawnMobj(mo, 0);
#endif
    // remove the old monster,
#ifdef SERVER
    P_RemoveMobj (mobj, true);
#else
    P_RemoveMobj (mobj);
#endif
}

//
// P_MobjThinker
//
void P_MobjThinker (mobj_t* mobj)
{
#ifdef SERVER
    if(disable_player_think && mobj->player)
	return;
#endif
    // momentum movement
    if (mobj->momx
	|| mobj->momy
	|| (mobj->flags&MF_SKULLFLY) )
    {
	P_XYMovement (mobj);

	// FIXME: decent NOP/NULL/Nil function pointer please.
	if (mobj->thinker.function.acv == (actionf_v) (-1))
	    return;		// mobj was removed
    }
    if ( (mobj->z != mobj->floorz)
	 || mobj->momz )
    {
	P_ZMovement (mobj);
	
	// FIXME: decent NOP/NULL/Nil function pointer please.
	if (mobj->thinker.function.acv == (actionf_v) (-1))
	    return;		// mobj was removed
    }

#ifndef SERVER
    if(local_player_predict)
	return;
#endif

	// [kg] new cheat
	if(mobj->player && mobj->player->cheats & CF_INFHEALTH && mobj->health < 100)
	{
		mobj->health++;
		mobj->player->health = mobj->health;
	}

    // cycle through states,
    // calling action functions at transitions
    if (mobj->tics != -1)
    {
	mobj->tics--;
		
	// you can cycle through multiple states in a tic
	if (!mobj->tics)
	    if (!P_SetMobjState (mobj, mobj->state->nextstate) )
		return;		// freed itself
    }
    else
    {
	// check for nightmare respawn
	if (! (mobj->flags & MF_COUNTKILL) )
	    return;

	if (!respawnmonsters)
	    return;

	mobj->movecount++;

	if (mobj->movecount < 12*35)
	    return;

	if ( leveltime&31 )
	    return;

	if (P_Random () > 4)
	    return;

	P_NightmareRespawn (mobj);
    }
}


//
// P_SpawnMobj
//
mobj_t*
P_SpawnMobj
( fixed_t	x,
  fixed_t	y,
  fixed_t	z,
  mobjtype_t	type )
{
    mobj_t*	mobj;
    state_t*	st;
    mobjinfo_t*	info;
	
    mobj = Z_Malloc (sizeof(*mobj), PU_LEVEL, NULL);
    memset (mobj, 0, sizeof (*mobj));
    info = &mobjinfo[type];
	
    mobj->type = type;
    mobj->info = info;
    mobj->x = x;
    mobj->y = y;
    mobj->radius = info->radius;
    mobj->height = info->height;
    mobj->flags = info->flags;
    mobj->health = info->spawnhealth;

    // [kg] unique ID
#ifdef SERVER
    mobj->netid = net_mobjid++;
#else
    if(netgame)
	mobj->netid = -1;
    else
	mobj->netid = net_mobjid++;
#endif

    if (gameskill != sk_nightmare)
	mobj->reactiontime = info->reactiontime;
    
    mobj->lastlook = P_Random () % MAXPLAYERS;
    // do not set the state with P_SetMobjState,
    // because action routines can not be called yet
    st = &states[info->spawnstate];

    mobj->state = st;
    mobj->tics = st->tics;
    mobj->sprite = st->sprite;
    mobj->frame = st->frame;

    // set subsector and/or block links
    P_SetThingPosition (mobj);
	
    mobj->floorz = mobj->subsector->sector->floorheight;
    mobj->ceilingz = mobj->subsector->sector->ceilingheight;

    if (z == ONFLOORZ)
	mobj->z = mobj->floorz;
    else if (z == ONCEILINGZ)
	mobj->z = mobj->ceilingz - mobj->info->height;
    else 
	mobj->z = z;

    mobj->thinker.function.acp1 = (actionf_p1)P_MobjThinker;
	
    P_AddThinker (&mobj->thinker, TT_MOBJ);

    return mobj;
}

//
// [kg] find MOBJ by netid
//
mobj_t *P_MobjByNetId(int netid)
{
	thinker_t *think;
	mobj_t *mo;

	if(!thinkercap.next)
		return NULL;

	for(think = thinkercap.next; think != &thinkercap; think = think->next)
	{
		if(think->function.acv != P_MobjThinker)
		// Not a mobj thinker
			continue;
		mo = (mobj_t *)think;
		if(mo->netid == netid)
			return mo;
	}

	return NULL;
}

//
// P_RemoveMobj
//
#ifdef SERVER
void P_RemoveMobj (mobj_t* mobj, boolean clientside)
{
	// remove from clients
	if(clientside)
		SV_RemoveMobj(mobj);
#else
void P_RemoveMobj (mobj_t* mobj)
{
#endif

	// [kg] cancel type
	mobj->thinker.lua_type = TT_INVALID;

	// unlink from sector and block lists
	P_UnsetThingPosition (mobj);

	// stop any playing sound
	S_StopSound (mobj, SOUND_STOP_ALL);

	// remove all references
	mobj_t *mo;
	thinker_t *think;

	for(think = thinkercap.next; think != &thinkercap; think = think->next)
	{
		if(think->function.acv != P_MobjThinker)
			continue;
		mo = (mobj_t *)think;
		if(mo->source == mobj)
			mo->source = NULL;
		if(mo->target == mobj)
			mo->target = NULL;
		if(mo->attacker == mobj)
			mo->attacker = NULL;
		if(mo->mobj == mobj)
			mo->mobj = NULL;
	}	

	// free block
	P_RemoveThinker ((thinker_t*)mobj);
}

//
// P_SpawnPlayer
// Called when a player is spawned on the level.
// Most of the player structure stays unchanged
//  between levels.
//
void P_SpawnPlayer (mapthing_hexen_t* mthing, int netplayer)
{
    player_t*		p;
    fixed_t		x;
    fixed_t		y;
    fixed_t		z;

    mobj_t*		mobj;

    int			i;

    p = &players[mthing->type-1];

#ifdef SERVER
    if(netplayer < 0)
	return;
    p = &players[netplayer];
#else
    if(netgame)
    {
	// start spectator player
	if(players[consoleplayer].mo)
	    return;
	p = &players[consoleplayer];
    } else
    {
	// not playing?
	if (!playeringame[mthing->type-1])
	    return;
    }
#endif

    if (p->playerstate == PST_REBORN)
	G_PlayerReborn (mthing->type-1);

    x 		= mthing->x << FRACBITS;
    y 		= mthing->y << FRACBITS;
    z		= ONFLOORZ;
    mobj	= P_SpawnMobj (x,y,z, MT_PLAYER);

    // set color translations for player sprites; TODO: change
//    if (mthing->type > 1)
//	mobj->flags |= (mthing->type-1)<<MF_TRANSSHIFT;
		
    mobj->angle	= ANG45 * (mthing->angle/45);
    mobj->player = p;
    mobj->health = p->health;
#ifdef SERVER
    mobj->reactiontime = 2;
#endif

    p->mo = mobj;
    p->playerstate = PST_LIVE;	
    p->refire = 0;
    p->message = NULL;
    p->damagecount = 0;
    p->bonuscount = 0;
    p->extralight = 0;
    p->fixedcolormap = 0;
    p->viewheight = mobj->info->viewz;

#ifndef SERVER
    if(netgame)
    {
	// start as a spectator
	P_UnsetThingPosition(mobj);
	p->cheats = CF_SPECTATOR | CF_NOCLIP | CF_GODMODE;
	mobj->flags &= ~(MF_SOLID | MF_SHOOTABLE);
	mobj->flags |= MF_NOSECTOR | MF_NOBLOCKMAP;
    }
#endif

    // setup gun psprite
    P_SetupPsprites (p);

    // give all cards in death match mode
    if (deathmatch)
	for (i=0 ; i<NUMCARDS ; i++)
	    p->cards[i] = true;

#ifdef SERVER
    // [kg] tell clients about this
    SV_SpawnPlayer(netplayer);
#else
    if (netgame || mthing->type-1 == consoleplayer)
    {
	// wake up the heads up text
	HU_Start ();
    }
#endif
}


//
// P_SpawnMapThing
// The fields of the mapthing should
// already be in host byte order.
//
void P_SpawnMapThing (mapthing_hexen_t* mthing)
{
    int			i;
    int			bit;
    mobj_t*		mobj;
    fixed_t		x;
    fixed_t		y;
    fixed_t		z;
		
    // count deathmatch start positions
    if (mthing->type == 11)
    {
	if (deathmatch_p < &deathmatchstarts[10])
	{
	    memcpy (deathmatch_p, mthing, sizeof(*mthing));
	    deathmatch_p++;
	}
	return;
    }

    // check for players specially
    if (mthing->type <= 4)
    {
	// save spots for respawning in network games
	playerstarts[mthing->type-1] = *mthing;
	if (!deathmatch)
	    P_SpawnPlayer (mthing, -1);

	return;
    }

#ifndef SERVER
    if(netgame)
	return;
#endif

    // check for apropriate skill level
    if (!netgame && (mthing->flags & 16) )
	return;

    if (gameskill == sk_baby)
	bit = 1;
    else if (gameskill == sk_nightmare)
	bit = 4;
    else
	bit = 1<<(gameskill-1);

    if (!(mthing->flags & bit) )
	return;
	
    // find which type to spawn
    for (i=0 ; i< numobjtypes ; i++)
	if (mthing->type == mobjinfo[i].doomednum)
	    break;
	 
    if (i == numobjtypes)
    {
	printf("P_SpawnMapThing: Unknown type %i at (%i, %i)\n", mthing->type, mthing->x, mthing->y);
	i = MT_UNKNOWN;
    }
#ifdef SERVER
    // [kg] spawn network weapons only in deathmatch
    if(netgame && mthing->flags & 16)
    {
	if(!(mobjinfo[i].flags & MF_ISMONSTER) && !deathmatch)
		// it's not a monster, don't spawn
		return;
    }
#endif
    // don't spawn keycards and players in deathmatch
    if (deathmatch && mobjinfo[i].flags & MF_NOTDMATCH)
	return;
		
    // don't spawn any monsters if -nomonsters
    if (nomonsters
	&& (mobjinfo[i].flags & MF_ISMONSTER) )
    {
	return;
    }
    
    // spawn it
    x = mthing->x << FRACBITS;
    y = mthing->y << FRACBITS;
    if(isHexen)
	z = mthing->z << FRACBITS;
    else
	z = ONFLOORZ;

    if (mobjinfo[i].flags & MF_SPAWNCEILING)
	z = ONCEILINGZ;

    mobj = P_SpawnMobj (x,y,z, i);
    mobj->spawnpoint = *mthing;

	if(z != ONFLOORZ && z != ONCEILINGZ)
		mobj->z = mobj->subsector->sector->floorheight;

    if (mobj->tics > 0)
	mobj->tics = 1 + (P_Random () % mobj->tics);
    if (mobj->flags & MF_COUNTKILL)
	totalkills++;
    if (mobj->flags & MF_COUNTITEM)
	totalitems++;
		
    mobj->angle = ANG45 * (mthing->angle/45);
    if (mthing->flags & MTF_AMBUSH)
	mobj->flags |= MF_AMBUSH;
}



//
// GAME SPAWN FUNCTIONS
//


//
// P_SpawnPuff
//
extern fixed_t attackrange;

void
P_SpawnPuff
( fixed_t	x,
  fixed_t	y,
  fixed_t	z,
  mobj_t *origin )
{
    mobj_t *th;
    sector_t *sec;

    th = P_SpawnMobj (x,y,z, la_pufftype);
    th->angle = R_PointToAngle2(th->x, th->y, origin->x, origin->y);

    sec = th->subsector->sector;
    if(z < sec->floorheight)
	z = sec->floorheight;
    if(z > sec->ceilingheight - th->info->height)
	z = sec->ceilingheight - th->info->height;
    th->z = z;

    if(th->info->meleestate && attackrange <= MELEERANGE)
	P_SetMobjState(th, th->info->meleestate);
    else
	P_SetMobjState(th, th->info->spawnstate);

    la_puffmobj = th;

#ifdef SERVER
    // tell clients about this
    SV_SpawnMobj(th, SV_MOBJF_MOMZ);
#endif
}



//
// P_SpawnBlood
// 
void
P_SpawnBlood
( fixed_t	x,
  fixed_t	y,
  fixed_t	z,
  mobj_t *origin )
{
    mobj_t*	th;

    th = P_SpawnMobj (x,y,z, la_pufftype);
    th->angle = R_PointToAngle2(th->x, th->y, origin->x, origin->y);

    if(th->info->painstate)
	P_SetMobjState(th, th->info->painstate);
    else
	P_SetMobjState(th, th->info->spawnstate);

    la_puffmobj = th;

#ifdef SERVER
    // tell clients about this
    SV_SpawnMobj(th, SV_MOBJF_MOMZ | SV_MOBJF_FLAGS);
#endif
}



//
// P_CheckMissileSpawn
// Moves the missile forward a bit
//  and possibly explodes it right there.
//
void P_CheckMissileSpawn (mobj_t* th)
{
    // move a little forward so an angle can
    // be computed if it immediately explodes
    th->x += (th->momx>>1);
    th->y += (th->momy>>1);
    th->z += (th->momz>>1);

    if (!P_TryMove (th, th->x, th->y))
	P_ExplodeMissile (th);
}


//
// P_SpawnMissile
// [kg] it's generic now
//
mobj_t*
P_SpawnMissile
( mobj_t *source,
  mobjtype_t type,
  angle_t ango,
  fixed_t slope,
  fixed_t zo,
  fixed_t xo,
  fixed_t yo )
{
    mobj_t*	th;

    fixed_t	x;
    fixed_t	y;
    fixed_t	z;

    x = source->x;
    y = source->y;
    z = source->z + source->info->shootz + zo;

    if(xo)
    {
	x += FixedMul(xo, finecosine[(ango+ANG90)>>ANGLETOFINESHIFT]);
	y += FixedMul(xo, finesine[(ango+ANG90)>>ANGLETOFINESHIFT]);
    }

    if(yo)
    {
	x += FixedMul(yo, finecosine[ango>>ANGLETOFINESHIFT]);
	y += FixedMul(yo, finesine[ango>>ANGLETOFINESHIFT]);
    }
	
    th = P_SpawnMobj (x,y,z, type);

    if (th->info->seesound)
	S_StartSound (th, th->info->seesound, SOUND_BODY);

    th->source = source;
    th->angle = ango;
    th->momz = FixedMul( th->info->speed, slope);
    if(slope < 0)
	slope = -slope;
    if(slope > FRACUNIT)
	slope = FRACUNIT;
    th->momx = FixedMul( FixedMul(th->info->speed, finecosine[ango>>ANGLETOFINESHIFT]), FRACUNIT - slope);
    th->momy = FixedMul( FixedMul(th->info->speed, finesine[ango>>ANGLETOFINESHIFT]), FRACUNIT - slope);

    P_CheckMissileSpawn (th);
#ifdef SERVER
    // tell clients about this
    if(th->flags & MF_MISSILE)
	SV_SpawnMobj(th, SV_MOBJF_AUTO | SV_MOBJF_STATE | SV_MOBJF_TARGET | SV_MOBJF_SOUND_SEE);
    else
	SV_SpawnMobj(th, SV_MOBJF_AUTO | SV_MOBJF_STATE | SV_MOBJF_TARGET | SV_MOBJF_SOUND_DEATH);
#endif
    return th;
}

