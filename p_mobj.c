#include "doomdef.h"

#include "i_system.h"
#include "z_zone.h"
#include "m_random.h"

#include "p_local.h"
#include "p_generic.h"

#include "st_stuff.h"
#include "hu_stuff.h"

#include "s_sound.h"

#include "doomstat.h"

#include "p_inventory.h"

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
// [kg] for thing Z collision checking
extern mobj_t *thzcbot;
extern mobj_t *thzctop;

void G_PlayerReborn (int player);
void P_SpawnMapThing (mapthing_hexen_t*	mthing);

//
// P_ForceMobjState
// Returns true if the mobj is still present.
//
int test;

boolean
P_ForceMobjState
( mobj_t*	mobj,
  statenum_t	state )
{
    state_t*	st;

    do
    {
	if (state == S_NULL || state == STATE_NULL_NEXT)
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
	{
		state = L_StateFromAlias(mobj->info, state);
		mobj->animation = state & STATE_AMASK; // new animation ID
	}

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
	P_SetMobjAnimation(mo, ANIM_DEATH, 0);

	mo->tics -= P_Random()&3;

	if (mo->tics < 1)
	    mo->tics = 1;
    }

    mo->flags &= ~MF_MISSILE;
    mo->flags |= MF_NOZCHANGE;

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
    fixed_t floorz;

    int stepdir = 0;

    if (!mo->momx && !mo->momy)
    {
	if (mo->flags & MF_SKULLFLY)
	{
	    // the skull slammed into something
	    mo->flags &= ~MF_SKULLFLY;
	    mo->momx = mo->momy = mo->momz = 0;

	    P_SetMobjAnimation(mo, ANIM_SPAWN, 0);
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
			sec = ceilingline->backsector;
		    else
			sec = ceilingline->frontsector;
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
		if(hitmobj && mo->flags & MF_MOBJBOUNCE)
		{
			mo->angle = ((mo->angle - ANG180) - ANG45) + (P_Random() << 22);
			mo->momx = FixedMul(mo->speed, finecosine[mo->angle>>ANGLETOFINESHIFT]);
			mo->momy = FixedMul(mo->speed, finesine[mo->angle>>ANGLETOFINESHIFT]);
			if(mo->info->bouncesound)
				S_StartSound(mo, mo->info->bouncesound, SOUND_BODY);
		} else
		if((ceilingline || floorline) && mo->flags & MF_WALLBOUNCE)
		{
			line_t *l = ceilingline ? ceilingline : floorline;
			mo->angle = 2 * l->angle - mo->angle;
			mo->momx = FixedMul(mo->speed, finecosine[mo->angle>>ANGLETOFINESHIFT]);
			mo->momy = FixedMul(mo->speed, finesine[mo->angle>>ANGLETOFINESHIFT]);
			if(mo->info->bouncesound)
				S_StartSound(mo, mo->info->bouncesound, SOUND_BODY);
		} else
		P_ExplodeMissile (mo);
#ifdef SERVER
		// [kg] explode clientside projectile
		if(hitmobj)
		    SV_UpdateMobj(mo, SV_MOBJF_POSITION | SV_MOBJF_MOMENTNUM | SV_MOBJF_FLAGS | SV_MOBJF_STATE | SV_MOBJF_SOUND_DEATH);
#endif
		break;
	    }
	    else
	    {
		mo->momx = mo->momy = 0;
		break;
	    }
	}
	// [kg] check line special location change
	if(ptryx != mo->x || ptryy != mo->y)
	    break;
    } while (xmove || ymove);
    
    // slow down
    if (player && player->cheats & CF_NOMOMENTUM)
    {
	// debug option for no sliding at all
	mo->momx = mo->momy = 0;
	return;
    }

    if (mo->flags & (MF_MISSILE | MF_SKULLFLY|MF_NOZCHANGE) )
	return; 	// no friction for missiles ever

    // [kg] detect floorz to walk on things
    floorz = mo->floorz;
    if(mo->z > floorz)
    {
	P_CheckPositionZ(mo);
	if(thzcbot && thzcbot->z + thzcbot->height > mo->floorz)
	    floorz = thzcbot->z + thzcbot->height;
    }

    // [kg] preset ground info
    mo->onground = false;

    if (mo->z > floorz)
	return;		// no friction when airborne

    // [kg] set actual status
    mo->onground = true;

    if (mo->flags & MF_CORPSE)
    {
	// do not stop sliding
	//  if halfway off a step with some momentum
	if (mo->momx > FRACUNIT/4
	    || mo->momx < -FRACUNIT/4
	    || mo->momy > FRACUNIT/4
	    || mo->momy < -FRACUNIT/4)
	{
	    if (floorz != mo->subsector->sector->floorheight)
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
	if(player && player->mo->animation == ANIM_SEE)
	    P_SetMobjAnimation(player->mo, ANIM_SPAWN, 0);

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
    fixed_t floorz, ceilingz;

    floorz = mo->floorz;
    ceilingz = mo->ceilingz;

    // [kg] mobj Z collision
    if(mo->z > floorz && !(mo->flags & (MF_MISSILE | MF_NOZCHANGE | MF_NOCLIP)))
    {
	if(!thzcbot) // use thing calculated in XY movement, if any
	    P_CheckPositionZ(mo);
	if(thzcbot && thzcbot->z + thzcbot->height > mo->floorz)
	{
	    floorz = thzcbot->z + thzcbot->height;
	}
	thzcbot = NULL; // and reset it now
    }

    // [kg] reset ground check
    mo->onground = false;

    // check for smooth step up
#ifdef SERVER
    if(mo->player && mo->z < floorz)
#else
    if(!local_player_predict && mo->player && mo->z < floorz)
#endif
    {
	mo->player->viewheight -= floorz-mo->z;

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
    if (mo->z <= floorz)
    {
	// hit the floor
	mo->onground = true;

	if(mo->momz < 0 && !(mo->player && (mo->player->cheats & CF_SPECTATOR || local_player_predict)))
	{
	    // [kg] call 'floor hit' callback
	    L_CrashThing(mo);
	    // [kg] added bounce effect
	    if(!mo->bounce || mo->momz > -FRACUNIT*3)
		mo->momz = 0;
	}
	mo->z = floorz;

	if ( (mo->flags & MF_MISSILE)
	     && !(mo->flags & MF_NOCLIP) )
	{
	    if(mo->subsector->sector->floorpic == skyflatnum && floorz == mo->subsector->sector->floorheight)
#ifdef SERVER
		P_RemoveMobj (mo, false);
#else
		P_RemoveMobj (mo);
#endif
	    else
	    if(!mo->bounce || !mo->momz)
	    {
		P_ExplodeMissile (mo);
		return;
	    }
	}
	if(mo->bounce)
	{
	    mo->momz = FixedMul(-mo->momz, mo->bounce);
	    if(mo->info->bouncesound)
		S_StartSound(mo, mo->info->bouncesound, SOUND_BODY);
	}
    }
    else if (! (mo->flags & MF_NOGRAVITY) )
    {
	if (mo->momz == 0)
	    mo->momz = -mo->gravity*2;
	else
	    mo->momz -= mo->gravity;
    }
	
    if (mo->z + mo->height > ceilingz)
    {
	// hit the ceiling

	// [kg] added bounce effect
	if(!mo->bounce || mo->momz < FRACUNIT*3)
	    mo->momz = 0;

	mo->z = ceilingz - mo->height;

	if ( (mo->flags & MF_MISSILE)
	     && !(mo->flags & MF_NOCLIP) )
	{
	    if(mo->subsector->sector->ceilingpic == skyflatnum && ceilingz == mo->subsector->sector->ceilingheight)
#ifdef SERVER
		P_RemoveMobj (mo, false);
#else
		P_RemoveMobj (mo);
#endif
	    else
	    if(!mo->bounce || !mo->momz)
	    {
		P_ExplodeMissile (mo);
		return;
	    }
	}
	if(mo->bounce)
	{
	    mo->momz = FixedMul(-mo->momz, mo->bounce);
	    if(mo->info->bouncesound)
		S_StartSound(mo, mo->info->bouncesound, SOUND_BODY);
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
/*    mo = P_SpawnMobj (mobj->x,
		      mobj->y,
		      mobj->subsector->sector->floorheight , MT_TFOG);
    // initiate teleport sound
    if(mo->info->seesound)
	S_StartSound (mo, mo->info->seesound, SOUND_BODY);
#ifdef SERVER
    // tell clients about this
    SV_SpawnMobj(mo, SV_MOBJF_SOUND_SEE);
#endif
*/
    // spawn a teleport fog at the new spot
    ss = R_PointInSubsector (x,y); 

/*    mo = P_SpawnMobj (x, y, ss->sector->floorheight , MT_TFOG);
    // initiate teleport sound
    if(mo->info->seesound)
	S_StartSound (mo, mo->info->seesound, SOUND_BODY);
#ifdef SERVER
    // tell clients about this
    SV_SpawnMobj(mo, SV_MOBJF_SOUND_SEE);
#endif
*/
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
    sector_t *sec = mobj->subsector->sector;

#ifdef SERVER
    if(disable_player_think && mobj->player)
	return;
#endif

#ifndef SERVER
    if(!netgame)
#endif
    // [kg] generalized sector damage
    if(sec->damage && sec->damagetick & 0x7FFF)
    {
	if(	!(leveltime % (sec->damagetick & 0x7FFF)) &&
		(sec->damagetick & 0x8000 || mobj->z <= mobj->subsector->sector->floorheight)
	) {
	    P_DamageMobj(mobj, NULL, NULL, sec->damage, sec->damagetype);
	    // FIXME: decent NOP/NULL/Nil function pointer please.
	    if(mobj->thinker.function.acv == (actionf_v) (-1))
		return;		// mobj was removed
	}
    }

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
    if ( (mobj->z != mobj->floorz) || mobj->momz)
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

    // [kg] run all Lua generic tickers
    if(mobj->generic_ticker)
	L_RunGenericTickers(mobj);

    // [kg] new cheat
    if(mobj->player && mobj->player->cheats & CF_INFHEALTH && mobj->health < 100)
	mobj->health++;

    // cycle through states,
    // calling action functions at transitions
    if (mobj->tics != -1)
    {
	mobj->tics--;
		
	// you can cycle through multiple states in a tic
	if (!mobj->tics)
	    if (!P_ForceMobjState (mobj, mobj->state->nextstate) )
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
    mobj->speed = info->speed;
    mobj->mass = info->mass;
    mobj->gravity = info->gravity;
    mobj->bounce = info->bounce;
    mobj->blocking = info->blocking;
    mobj->canpass = info->canpass;
    mobj->translation = info->translation;
    mobj->renderstyle = info->renderstyle;
    mobj->rendertable = info->rendertable;
    memcpy(mobj->damagescale, mobj->info->damagescale, NUMDAMAGETYPES);

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

    mobj->z = z;

    if(is_setup)
    {
	mobj->floorz = mobj->subsector->sector->floorheight;
	mobj->ceilingz = mobj->subsector->sector->ceilingheight;
    } else
	// [kg] force 3D detection
	P_GetPosition(mobj);

    if(z == ONFLOORZ)
	mobj->z = mobj->floorz;
    else if(z == ONCEILINGZ)
	mobj->z = mobj->ceilingz - mobj->info->height;

    if(mobj->z <= mobj->floorz)
	mobj->onground = true;

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

	// [kg] destroy inventory
	P_RemoveInventory(mobj);

	// [kg] remove all tickers
	P_RemoveMobjTickers(mobj);

	// [kg] remove all references
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
    boolean		isDoll = false;

    playerstate_t oldst;

    x = mthing->x << FRACBITS;
    y = mthing->y << FRACBITS;

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
        {
	    // only update position
	    P_UnsetThingPosition(players[consoleplayer].mo);
	    players[consoleplayer].mo->x = x;
	    players[consoleplayer].mo->y = y;
	    P_SetThingPosition(players[consoleplayer].mo);
	    players[consoleplayer].mo->z = players[consoleplayer].mo->subsector->sector->floorheight;
	    return;
        }
	p = &players[consoleplayer];
    } else
    {
	// not playing?
	if (!playeringame[mthing->type-1])
	    return;
    }
#endif

    oldst = p->playerstate;
    if (p->playerstate == PST_REBORN)
	G_PlayerReborn (mthing->type-1);

    if(isHexen)
	z = R_PointInSubsector(x,y)->sector->floorheight + (mthing->z << FRACBITS);
    else
	z = ONFLOORZ;

    if(p->mo)
    {
	// spawn voodoo doll
	x = p->mo->x;
	y = p->mo->y;
	z = p->mo->z;
	// move player to new location
	P_UnsetThingPosition(p->mo);
	p->mo->x = mthing->x << FRACBITS;
	p->mo->y = mthing->y << FRACBITS;
	p->mo->z = R_PointInSubsector(p->mo->x, p->mo->y)->sector->floorheight;
	if(isHexen)
	    p->mo->z += mthing->z << FRACBITS;
	P_SetThingPosition(p->mo);
	isDoll = true;
    }

    mobj = P_SpawnMobj (x,y,z, MT_PLAYER);

    // set color translations for player sprites; TODO: change
//    if (mthing->type > 1)
//	mobj->flags |= (mthing->type-1)<<MF_TRANSSHIFT;
		
    mobj->angle	= ANG45 * (mthing->angle/45);
    mobj->player = p;
    mobj->reactiontime = 2;

    if(isDoll)
	return;

    p->mo = mobj;
    p->playerstate = PST_LIVE;	
    p->refire = 0;
    p->message = NULL;
    p->damagecount = 0;
    p->bonuscount = 0;
    p->extralight = 0;
    p->viewheight = mobj->info->viewz;

    p->viewmap.lump = 0;
    p->viewmap.idx = 0;
    p->viewmap.data = NULL;

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

    // [kg] call Lua, if spawned
    if(oldst == PST_REBORN)
    {
	if(p == &players[consoleplayer])
	    ST_ClearInventory();
	L_SpawnPlayer(p);
    } else
    // [kg] give original inventory
    {
	p->mo->inventory = p->inventory;
	p->inventory = NULL;
	p->mo->armorpoints = p->armorpoints;
	p->mo->armortype = p->armortype;
	p->mo->health = p->health;
    }

    // setup gun psprite
    P_SetupPsprites (p);

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
	if (!sv_deathmatch)
	    P_SpawnPlayer (mthing, -1);

	return;
    }

#ifndef SERVER
    if(netgame)
	return;
#endif

    // check for apropriate skill level
    if (!isHexen && !netgame && (mthing->flags & 16) )
	return;

    if (gameskill == sk_baby)
	bit = 1;
    else if (gameskill == sk_nightmare)
	bit = 4;
    else
	bit = 1<<(gameskill-1);

    if (!(mthing->flags & bit) )
	return;
	
    if(isHexen)
    {
	if(!netgame && !(mthing->flags & 0x0100))
		return;
	if(sv_deathmatch && !(mthing->flags & 0x0400))
		return;
	if(!sv_deathmatch && !(mthing->flags & 0x0200))
		return;
    } else
    {
	// [kg] spawn network weapons only in deathmatch
	if(netgame && mthing->flags & 16)
	{
	    if(!(mobjinfo[i].flags & MF_ISMONSTER) && !sv_deathmatch)
		// it's not a monster, don't spawn
		return;
	}
    }

    // find which type to spawn
    for (i=0 ; i< numobjtypes ; i++)
	if (mthing->type == mobjinfo[i].doomednum)
	    break;
	 
    if (i == numobjtypes)
    {
	printf("P_SpawnMapThing: Unknown type %i at (%i, %i)\n", mthing->type, mthing->x, mthing->y);
	i = MT_UNKNOWN;
    }

    // don't spawn keycards and players in deathmatch
    if (sv_deathmatch && mobjinfo[i].flags & MF_NOTDMATCH)
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

    mobj->tag = mthing->tid;

    if(z != ONFLOORZ && z != ONCEILINGZ)
	mobj->z = mobj->subsector->sector->floorheight + z;

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
  mobj_t *origin,
  mobj_t *cause )
{
    mobj_t *th;
    sector_t *sec;

    th = P_SpawnMobj (x,y,z, la_pufftype);

    th->target = cause;
    th->source = origin;

    th->angle = R_PointToAngle2(th->x, th->y, cause->x, cause->y);

    sec = th->subsector->sector;
    if(z < sec->floorheight)
	z = sec->floorheight;
    if(z > sec->ceilingheight - th->info->height)
	z = sec->ceilingheight - th->info->height;
    th->z = z;

    P_SetMobjAnimation(th, ANIM_SPAWN, 0);

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
  mobj_t *origin,
  mobj_t *cause )
{
    mobj_t*	th;

    th = P_SpawnMobj (x,y,z, la_pufftype);

    th->target = cause;
    th->source = origin;

    th->angle = R_PointToAngle2(th->x, th->y, origin->x, origin->y);

    if(th->info->painstate)
	P_SetMobjAnimation(th, ANIM_PAIN, 0);
    else
	P_SetMobjAnimation(th, ANIM_SPAWN, 0);

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
    th->momz = FixedMul(th->speed, slope);
    th->momx = FixedMul(th->speed, finecosine[ango>>ANGLETOFINESHIFT]);
    th->momy = FixedMul(th->speed, finesine[ango>>ANGLETOFINESHIFT]);

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

