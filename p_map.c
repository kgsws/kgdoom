#include <stdlib.h>

#include "doomdef.h"

#include "m_bbox.h"
#include "m_random.h"
#include "i_system.h"

#include "doomdef.h"
#include "p_local.h"

#include "s_sound.h"

// State.
#include "doomstat.h"
#include "r_state.h"

#include "p_pickup.h"

#include "p_generic.h"
#include "kg_3dfloor.h"
#include "kg_lua.h"

#ifdef SERVER
#include <netinet/in.h>
#include "network.h"
#include "sv_cmds.h"
#endif


fixed_t		tmbbox[4];
mobj_t*		tmthing;
int		tmflags;
fixed_t		tmx;
fixed_t		tmy;

// [kg] keep track of projectile hits
mobj_t *hitmobj;
// [kg] for thing Z collision checking
mobj_t *thzcbot;
mobj_t *thzctop;
static boolean tmblocked;
static fixed_t tmthingtop;
static fixed_t tmthingtopnew;
static fixed_t tmthingnewz;

// If "floatok" true, move would be ok
// if within "tmfloorz - tmceilingz".
boolean		floatok;

fixed_t		tmfloorz;
fixed_t		tmceilingz;
fixed_t		tmdropoffz;

// keep track of the line that lowers the ceiling,
// so missiles don't explode against sky hack walls
line_t*		ceilingline;
// [kg] for bump special
line_t*		floorline;

// keep track of special lines as they are hit,
// but don't process them until the move is proven valid
#define MAXSPECIALCROSS		8

line_t*		spechit[MAXSPECIALCROSS];
int		numspechit;


//
// MOVEMENT ITERATOR FUNCTIONS
//

//
// PIT_CheckLine
// Adjusts tmfloorz and tmceilingz as lines are contacted
//
boolean PIT_CheckLine (line_t* ld)
{
    extraplane_t *pl;
    boolean is_blocking = false;

    if (tmbbox[BOXRIGHT] <= ld->bbox[BOXLEFT]
	|| tmbbox[BOXLEFT] >= ld->bbox[BOXRIGHT]
	|| tmbbox[BOXTOP] <= ld->bbox[BOXBOTTOM]
	|| tmbbox[BOXBOTTOM] >= ld->bbox[BOXTOP] )
	return true;

    if (P_BoxOnLineSide (tmbbox, ld) != -1)
	return true;
		
    // A line has been hit
    
    // The moving thing's destination position will cross
    // the given line.
    // If this should not be allowed, return false.
    // If the line is special, keep track of it
    // to process later if the move is proven ok.
    // NOTE: specials are NOT sorted by order,
    // so two special lines that are only 8 pixels apart
    // could be crossed in either order.
    
    if (!ld->backsector)
	goto nocross;		// one sided line

    // [kg] new blocking style
    if(~tmthing->canpass & ld->blocking)
	is_blocking = true;

    // [kg] 3D midtex check
    if(is_blocking && (!isHexen || (!sides[ld->sidenum[0]].midtexture && !sides[ld->sidenum[1]].midtexture)))
	// blocking with no textures - use full height
	// ... or in Doom map format
	goto nocross;

    // set openrange, opentop, openbottom
    P_LineOpening (ld);

    // [kg] 3D midtex check
    if(is_blocking)
    {
	fixed_t z0, z1;
	side_t *side;

	// front side
	side = &sides[ld->sidenum[0]];
	if(side->midtexture)
	{
	    if(ld->flags & LF_DONTPEGBOTTOM)
	    {
		z1 = ld->frontsector->floorheight + side->rowoffset;
		if(ld->backsector->floorheight > z1)
			z1 = ld->backsector->floorheight;
		z0 = z1 + textureheight[side->midtexture];
	    } else
	    {
		z0 = ld->frontsector->ceilingheight + side->rowoffset;
		if(ld->backsector->ceilingheight < z0)
			z0 = ld->backsector->ceilingheight;
		z1 = z0 - textureheight[side->midtexture];
	    }

	    if(	z0 > tmthing->z + tmthing->info->stepheight &&
		z1 < tmthing->z + tmthing->height
	    )
		goto nocross;

	    // floor
	    if(z0 > openbottom && z0 <= tmthing->z + tmthing->info->stepheight)
		openbottom = z0;
	    // ceiling
	    if(z1 < opentop && z1 > tmthing->z)
		opentop = z1;
	}

	// back side
	side = &sides[ld->sidenum[1]];
	if(side->midtexture)
	{
	    if(ld->flags & LF_DONTPEGBOTTOM)
	    {
		z1 = ld->frontsector->floorheight + side->rowoffset;
		z0 = z1 + textureheight[side->midtexture];
	    } else
	    {
		z0 = ld->frontsector->ceilingheight + side->rowoffset;
		z1 = z0 - textureheight[side->midtexture];
	    }

	    if(	z0 > tmthing->z + tmthing->info->stepheight &&
		z1 < tmthing->z + tmthing->height
	    )
		goto nocross;

	    // floor
	    if(z0 > openbottom && z0 <= tmthing->z + tmthing->info->stepheight)
		openbottom = z0;
	    // ceiling
	    if(z1 < opentop && z1 > tmthing->z)
		opentop = z1;
	}
    }

    // [kg] 3D floors check
    if(P_PointOnLineSide(tmthing->x, tmthing->y, ld))
    {
	pl = ld->frontsector->exfloor;
	while(pl)
	{
	    if( ~tmthing->canpass & pl->blocking &&
		pl->source->ceilingheight > tmthing->z + tmthing->info->stepheight &&
		pl->source->floorheight < tmthing->z + tmthing->height
	    )
		goto nocross;
	    if( ~tmthing->canpass & pl->blocking &&
		*pl->height > openbottom && *pl->height <= tmthing->z + tmthing->info->stepheight)
		openbottom = *pl->height;
	    pl = pl->next;
	}
	// other side
	pl = ld->backsector->exfloor;
	while(pl)
	{
	    if( ~tmthing->canpass & pl->blocking &&
		*pl->height > openbottom && *pl->height <= tmthing->z + tmthing->info->stepheight)
		openbottom = *pl->height;
	    pl = pl->next;
	}
    } else
    {
	pl = ld->backsector->exfloor;
	while(pl)
	{
	    if( ~tmthing->canpass & pl->blocking &&
		pl->source->ceilingheight > tmthing->z + tmthing->info->stepheight &&
		pl->source->floorheight < tmthing->z + tmthing->height
	    )
		goto nocross;
	    if( ~tmthing->canpass & pl->blocking &&
		*pl->height > openbottom && *pl->height <= tmthing->z + tmthing->info->stepheight)
		openbottom = *pl->height;
	    pl = pl->next;
	}
	// other side
	pl = ld->frontsector->exfloor;
	while(pl)
	{
	    if( ~tmthing->canpass & pl->blocking &&
		*pl->height > openbottom && *pl->height <= tmthing->z + tmthing->info->stepheight)
		openbottom = *pl->height;
	    pl = pl->next;
	}
    }

    // [kg] 3D ceilings check
    pl = ld->frontsector->exceiling;
    while(pl)
    {
	if(*pl->height <= tmthing->z)
	    break;
	if( ~tmthing->canpass & pl->blocking && *pl->height < opentop)
	    opentop = *pl->height;
	pl = pl->next;
    }
    pl = ld->backsector->exceiling;
    while(pl)
    {
	if(*pl->height <= tmthing->z)
	    break;
	if( ~tmthing->canpass & pl->blocking && *pl->height < opentop)
	    opentop = *pl->height;
	pl = pl->next;
    }

    // adjust floor / ceiling heights
    if (opentop < tmceilingz)
    {
	tmceilingz = opentop;
	ceilingline = ld;
    }

    if (openbottom > tmfloorz)
    {
	tmfloorz = openbottom;	
	floorline = ld;
    }

    if (lowfloor < tmdropoffz)
	tmdropoffz = lowfloor;

    // if contacted a special line, add it to the list; [kg] fix the overflow
    if(ld->special && numspechit < MAXSPECIALCROSS)
    {
	spechit[numspechit] = ld;
	numspechit++;
    }

    return true;

nocross:
    floorline = ld;

    return false;
}

//
// PIT_CheckThing
//
boolean PIT_CheckThing (mobj_t* thing)
{
    fixed_t		blockdist;
    boolean		solid;
    int			damage;
		
    if (!(thing->flags & (MF_SOLID|MF_SPECIAL|MF_SHOOTABLE) ))
	return true;
    
    blockdist = thing->radius + tmthing->radius;

    if ( abs(thing->x - tmx) >= blockdist
	 || abs(thing->y - tmy) >= blockdist )
    {
	// didn't hit it
	return true;	
    }
    
    // don't clip against self
    if (thing == tmthing)
	return true;

    // [kg] new blocking
    if(!(~tmthing->canpass & thing->blocking))
	return true;

    // [kg] thing Z collision checking
    if(tmthing->z + tmthing->height <= thing->z)
	return true;
    if(tmthing->z >= thing->z + thing->height)
	return true;

    // check for skulls slamming into things
    if (tmthing->flags & MF_SKULLFLY && thing->flags & MF_SOLID)
    {
	if(tmthing->info->damage < 0)
		damage = ((P_Random()%8)+1)*-tmthing->info->damage;
	else
		damage = tmthing->info->damage;

	tmthing->flags &= ~MF_SKULLFLY;
	tmthing->momx = tmthing->momy = tmthing->momz = 0;
	
	P_SetMobjAnimation(tmthing, ANIM_SPAWN, 0);
#ifdef SERVER
	// tell clients about this
	SV_UpdateMobj(thing, SV_MOBJF_POSITION | SV_MOBJF_MOMENTNUM | SV_MOBJF_STATE);
#else
	if(!netgame)
#endif
	P_DamageMobj (thing, tmthing, tmthing, damage, tmthing->info->damagetype);

	return false;		// stop moving
    }

    
    // missiles can hit other things
    if (tmthing->flags & MF_MISSILE)
    {
#ifndef SERVER
	if(netgame)
	    // [kg] no client side prediction for projectiles
	    return true;
#endif

	// [kg] do not hit self
	if(thing == tmthing->source)
	    return true;

	// [kg] new species handling
	if(thing->info->species && tmthing->source && thing->info->species == tmthing->source->info->species)
	{
	    // hit, but no damage
	    hitmobj = thing;
	    return false;
	}
	
	if (! (thing->flags & MF_SHOOTABLE) )
	{
	    // didn't do any damage
	    if(thing->flags & MF_SOLID)
	    {
		// save hit thing
		hitmobj = thing;
		return false;
	    } else
		return true;
	}
	
	// damage / explode
	damage = tmthing->info->damage;
	if(damage)
	{
	    if(damage < 0) // Doom random
		damage = ((P_Random()%8)+1)*-damage;
	    P_DamageMobj (thing, tmthing, tmthing->source, damage, tmthing->info->damagetype);
	}
	// save hit thing
	hitmobj = thing;

	// don't traverse any more
	return false;				
    }
    
    // check for special pickup
#ifdef SERVER
    if (thing->flags & MF_SPECIAL)
#else
    if (!netgame && thing->flags & MF_SPECIAL)
#endif
    {
	solid = thing->flags&MF_SOLID;
	if (tmflags&MF_PICKUP)
	{
	    // can remove thing
	    P_TouchSpecialThing (thing, tmthing);
	}
	return !solid;
    }
	
    return !(thing->flags & MF_SOLID);
}


//
// MOVEMENT CLIPPING
//

//
// P_CheckPosition
// This is purely informative, nothing is modified
// (except things picked up).
// 
// in:
//  a mobj_t (can be valid or invalid)
//  a position to be checked
//   (doesn't need to be related to the mobj_t->x,y)
//
// during:
//  special things are touched if MF_PICKUP
//  early out on solid lines?
//
// out:
//  newsubsec
//  floorz
//  ceilingz
//  tmdropoffz
//   the lowest point contacted
//   (monsters won't move to a dropoff)
//  speciallines[]
//  numspeciallines
//
boolean
P_CheckPosition
( mobj_t*	thing,
  fixed_t	x,
  fixed_t	y )
{
    int			xl;
    int			xh;
    int			yl;
    int			yh;
    int			bx;
    int			by;
    subsector_t*	newsubsec;
    extraplane_t	*pl;

    tmthing = thing;
    tmflags = thing->flags;
	
    tmx = x;
    tmy = y;
	
    tmbbox[BOXTOP] = y + tmthing->radius;
    tmbbox[BOXBOTTOM] = y - tmthing->radius;
    tmbbox[BOXRIGHT] = x + tmthing->radius;
    tmbbox[BOXLEFT] = x - tmthing->radius;

    newsubsec = R_PointInSubsector (x,y);
    ceilingline = NULL;
    floorline = NULL;
    
    // The base floor / ceiling is from the subsector
    // that contains the point.
    // Any contacted lines the step closer together
    // will adjust them.
    tmfloorz = tmdropoffz = newsubsec->sector->floorheight;
    tmceilingz = newsubsec->sector->ceilingheight;

    // [kg] 3D floors check
    pl = newsubsec->sector->exfloor;
    while(pl)
    {
	if(*pl->height > tmthing->z)
	    break;
	if(*pl->height > tmfloorz)
	    tmfloorz = *pl->height;
	pl = pl->next;
    }
    // [kg] 3D ceilings check
    pl = newsubsec->sector->exceiling;
    while(pl)
    {
	if(*pl->height <= tmthing->z)
	    break;
	if(*pl->height < tmfloorz)
	    tmfloorz = *pl->height;
	pl = pl->next;
    }

    validcount++;
    numspechit = 0;

    if ( tmflags & MF_NOCLIP )
	return true;

    // [kg] new blockmap handling does not need MAXRADIUS
    xl = (tmbbox[BOXLEFT] - bmaporgx)>>MAPBLOCKSHIFT;
    xh = (tmbbox[BOXRIGHT] - bmaporgx)>>MAPBLOCKSHIFT;
    yl = (tmbbox[BOXBOTTOM] - bmaporgy)>>MAPBLOCKSHIFT;
    yh = (tmbbox[BOXTOP] - bmaporgy)>>MAPBLOCKSHIFT;

    if(!(thing->flags & MF_TROUGHMOBJ))
    // Check things first, possibly picking things up.
    for (bx=xl ; bx<=xh ; bx++)
	for (by=yl ; by<=yh ; by++)
	    if (!P_BlockThingsIterator(bx,by,PIT_CheckThing))
		return false;
    
    // check lines
    for (bx=xl ; bx<=xh ; bx++)
	for (by=yl ; by<=yh ; by++)
	    if (!P_BlockLinesIterator (bx,by,PIT_CheckLine))
		return false;

    return true;
}


//
// P_TryMove
// Attempt to move to a new position,
// crossing special lines unless MF_TELEPORT is set.
//
boolean
P_TryMove
( mobj_t*	thing,
  fixed_t	x,
  fixed_t	y )
{
    fixed_t	oldx;
    fixed_t	oldy;
    int		side;
    int		oldside;
    line_t*	ld;

    floatok = false;
    if (!P_CheckPosition (thing, x, y))
	goto nocross;		// solid wall or thing
    
    if ( !(thing->flags & MF_NOCLIP) )
    {
	if (tmceilingz - tmfloorz < thing->height)
	    goto nocross;	// doesn't fit

	floatok = true;
	
	if ( !(thing->flags&MF_TELEPORT) 
	     &&tmceilingz - thing->z < thing->height)
	    goto nocross;	// mobj must lower itself to fit

	if ( !(thing->flags&MF_TELEPORT)
	     && tmfloorz - thing->z > thing->info->stepheight )
	    goto nocross;	// too big a step up

	if ( !(thing->flags&(MF_DROPOFF|MF_FLOAT))
	     && tmfloorz - tmdropoffz > thing->info->stepheight )
	    goto nocross;	// don't stand over a dropoff
    }
    
    // the move is ok,
    // so link the thing into its new position
    P_UnsetThingPosition (thing);

    oldx = thing->x;
    oldy = thing->y;
    thing->floorz = tmfloorz;
    thing->ceilingz = tmceilingz;	
    thing->x = x;
    thing->y = y;

    P_SetThingPosition (thing);
    
    // if any special lines were hit, do the effect
#ifndef SERVER
    if(!netgame)
#endif
    if (! (thing->flags&(MF_TELEPORT|MF_NOCLIP)) )
    {
	while (numspechit--)
	{
	    // see if the line was crossed
	    ld = spechit[numspechit];
	    side = P_PointOnLineSide (thing->x, thing->y, ld);
	    oldside = P_PointOnLineSide (oldx, oldy, ld);
	    if (side != oldside)
	    {
		if(ld->special)
		{
		    fixed_t bx = thing->x;
		    fixed_t by = thing->y;
		    P_ExtraLineSpecial(thing, ld, oldside, EXTRA_CROSS);
		    // [kg] check for teleport
		    if(thing->x != bx || thing->y != by)
			break;
		}
	    }
	}
    }

    return true;

nocross:
#ifndef SERVER
    if(!netgame)
#endif
    // [kg] bump special for player
    if(thing->player)
    {
	if(floorline && floorline->special)
		P_ExtraLineSpecial(thing, floorline, P_PointOnLineSide(thing->x, thing->y, floorline), EXTRA_BUMP);
	if(ceilingline && ceilingline != floorline && ceilingline->special)
		P_ExtraLineSpecial(thing, ceilingline, P_PointOnLineSide(thing->x, thing->y, ceilingline), EXTRA_BUMP);
    }

    return false;
}


//
// P_ThingHeightClip
// Takes a valid thing and adjusts the thing->floorz,
// thing->ceilingz, and possibly thing->z.
// This is called for all nearby monsters
// whenever a sector changes height.
// If the thing doesn't fit,
// the z will be set to the lowest value
// and false will be returned.
//
// [kg] updates for thing Z collisions
boolean P_UpdateThingZ(mobj_t *thing, fixed_t newz);
//
boolean P_ThingHeightClip (mobj_t* thing)
{
    boolean		onfloor;
    fixed_t oldfz;

    onfloor = (thing->z <= thing->floorz);

    P_CheckPositionLines(thing);
    // what about stranding a monster partially off an edge?

    oldfz = thing->floorz;

    thing->floorz = tmfloorz;
    thing->ceilingz = tmceilingz;

    if(onfloor || thing->z <= thing->floorz)
    {
	if(oldfz != thing->floorz)
	{
	    tmblocked = false;
	    // walking monsters rise and fall with the floor
	    if(!P_UpdateThingZ(thing, thing->floorz))
		return false;
	}
    }
    else
    {
	// don't adjust a floating monster unless forced to
	if (thing->z+thing->height > thing->ceilingz)
	    thing->z = thing->ceilingz - thing->height;
    }
	
    if (thing->ceilingz - thing->floorz < thing->height)
	return false;
		
    return true;
}



//
// SLIDE MOVE
// Allows the player to slide along any angled walls.
//
fixed_t		bestslidefrac;
fixed_t		secondslidefrac;

line_t*		bestslideline;
line_t*		secondslideline;

mobj_t*		slidemo;

fixed_t		tmxmove;
fixed_t		tmymove;



//
// P_HitSlideLine
// Adjusts the xmove / ymove
// so that the next move will slide along the wall.
//
void P_HitSlideLine (line_t* ld)
{
    int			side;

    angle_t		lineangle;
    angle_t		moveangle;
    angle_t		deltaangle;
    
    fixed_t		movelen;
    fixed_t		newlen;
	
	
    if (ld->slopetype == ST_HORIZONTAL)
    {
	tmymove = 0;
	return;
    }
    
    if (ld->slopetype == ST_VERTICAL)
    {
	tmxmove = 0;
	return;
    }
	
    side = P_PointOnLineSide (slidemo->x, slidemo->y, ld);
	
    lineangle = R_PointToAngle2 (0,0, ld->dx, ld->dy);

    if (side == 1)
	lineangle += ANG180;

    moveangle = R_PointToAngle2 (0,0, tmxmove, tmymove);
    deltaangle = moveangle-lineangle;

    if (deltaangle > ANG180)
	deltaangle += ANG180;
    //	I_Error ("SlideLine: ang>ANG180");

    lineangle >>= ANGLETOFINESHIFT;
    deltaangle >>= ANGLETOFINESHIFT;
	
    movelen = P_AproxDistance (tmxmove, tmymove);
    newlen = FixedMul (movelen, finecosine[deltaangle]);

    tmxmove = FixedMul (newlen, finecosine[lineangle]);	
    tmymove = FixedMul (newlen, finesine[lineangle]);	
}


//
// PTR_SlideTraverse
//
boolean PTR_SlideTraverse (intercept_t* in)
{
    line_t*	li;
    extraplane_t *pl;
	
    if (!in->isaline)
	I_Error ("PTR_SlideTraverse: not a line?");
		
    li = in->d.line;

    if( ~slidemo->canpass & li->blocking )
    {
	if (P_PointOnLineSide (slidemo->x, slidemo->y, li))
	{
	    // don't hit the back side
	    return true;		
	}
	goto isblocking;
    } else
    if(~slidemo->canpass & li->blocking)
        goto isblocking;

    // set openrange, opentop, openbottom
    P_LineOpening (li);
    
    if (openrange < slidemo->height)
	goto isblocking;		// doesn't fit
		
    if (opentop - slidemo->z < slidemo->height)
	goto isblocking;		// mobj is too high

    if (openbottom - slidemo->z > slidemo->info->stepheight )
	goto isblocking;		// too big a step up

    // [kg] 3D floors check
    if(P_PointOnLineSide(slidemo->x, slidemo->y, li))
    {
	pl = li->frontsector->exfloor;
	while(pl)
	{
	    if(	~slidemo->canpass & pl->blocking &&
		pl->source->ceilingheight > slidemo->z + slidemo->info->stepheight &&
		pl->source->floorheight < slidemo->z + slidemo->height
	    )
		goto isblocking;
	    pl = pl->next;
	}
    } else
    {
	pl = li->backsector->exfloor;
	while(pl)
	{
	    if(	~slidemo->canpass & pl->blocking &&
		pl->source->ceilingheight > slidemo->z + slidemo->info->stepheight &&
		pl->source->floorheight < slidemo->z + slidemo->height
	    )
		goto isblocking;
	    pl = pl->next;
	}
    }

    // this line doesn't block movement
    return true;		
	
    // the line does block movement,
    // see if it is closer than best so far
  isblocking:		
    if (in->frac < bestslidefrac)
    {
	secondslidefrac = bestslidefrac;
	secondslideline = bestslideline;
	bestslidefrac = in->frac;
	bestslideline = li;
    }
	
    return false;	// stop
}



//
// P_SlideMove
// The momx / momy move is bad, so try to slide
// along a wall.
// Find the first line hit, move flush to it,
// and slide along it
//
// This is a kludgy mess.
//
void P_SlideMove (mobj_t* mo)
{
    fixed_t		leadx;
    fixed_t		leady;
    fixed_t		trailx;
    fixed_t		traily;
    fixed_t		newx;
    fixed_t		newy;
    int			hitcount;
		
    slidemo = mo;
    hitcount = 0;
    
  retry:
    if (++hitcount == 3)
	goto stairstep;		// don't loop forever

    
    // trace along the three leading corners
    if (mo->momx > 0)
    {
	leadx = mo->x + mo->radius;
	trailx = mo->x - mo->radius;
    }
    else
    {
	leadx = mo->x - mo->radius;
	trailx = mo->x + mo->radius;
    }
	
    if (mo->momy > 0)
    {
	leady = mo->y + mo->radius;
	traily = mo->y - mo->radius;
    }
    else
    {
	leady = mo->y - mo->radius;
	traily = mo->y + mo->radius;
    }
		
    bestslidefrac = FRACUNIT+1;
	
    P_PathTraverse ( leadx, leady, leadx+mo->momx, leady+mo->momy,
		     PT_ADDLINES, PTR_SlideTraverse );
    P_PathTraverse ( trailx, leady, trailx+mo->momx, leady+mo->momy,
		     PT_ADDLINES, PTR_SlideTraverse );
    P_PathTraverse ( leadx, traily, leadx+mo->momx, traily+mo->momy,
		     PT_ADDLINES, PTR_SlideTraverse );
    
    // move up to the wall
    if (bestslidefrac == FRACUNIT+1)
    {
	// the move most have hit the middle, so stairstep
      stairstep:
	if (!P_TryMove (mo, mo->x, mo->y + mo->momy))
	    P_TryMove (mo, mo->x + mo->momx, mo->y);
	return;
    }

    // fudge a bit to make sure it doesn't hit
    bestslidefrac -= 0x800;	
    if (bestslidefrac > 0)
    {
	newx = FixedMul (mo->momx, bestslidefrac);
	newy = FixedMul (mo->momy, bestslidefrac);
	
	if (!P_TryMove (mo, mo->x+newx, mo->y+newy))
	    goto stairstep;
    }
    
    // Now continue along the wall.
    // First calculate remainder.
    bestslidefrac = FRACUNIT-(bestslidefrac+0x800);
    
    if (bestslidefrac > FRACUNIT)
	bestslidefrac = FRACUNIT;
    
    if (bestslidefrac <= 0)
	return;
    
    tmxmove = FixedMul (mo->momx, bestslidefrac);
    tmymove = FixedMul (mo->momy, bestslidefrac);

    P_HitSlideLine (bestslideline);	// clip the moves

    mo->momx = tmxmove;
    mo->momy = tmymove;
		
    if (!P_TryMove (mo, mo->x+tmxmove, mo->y+tmymove))
    {
	goto retry;
    }
}


//
// P_LineAttack
//
mobj_t*		linetarget;	// who got hit (or NULL)
mobj_t*		shootthing;

// Height if not aiming up or down
// ???: use slope for monsters?
fixed_t		shootz;	

int		la_damage;
fixed_t		attackrange;

// [kg] custom puffs
mobjtype_t	la_pufftype;

fixed_t		aimslope;

// slopes to top and bottom of target
extern fixed_t	topslope;
extern fixed_t	bottomslope;	


//
// PTR_AimTraverse
// Sets linetaget and aimslope when a target is aimed at.
//
boolean
PTR_AimTraverse (intercept_t* in)
{
    line_t*		li;
    mobj_t*		th;
    fixed_t		slope;
    fixed_t		thingtopslope;
    fixed_t		thingbottomslope;
    fixed_t		dist;
		
    if (in->isaline)
    {
	li = in->d.line;
	
	if ( !(li->flags & LF_TWOSIDED) )
	    return false;		// stop
	
	// Crosses a two sided line.
	// A two sided line will restrict
	// the possible target ranges.
	P_LineOpening (li);
	
	if (openbottom >= opentop)
	    return false;		// stop
	
	dist = FixedMul (attackrange, in->frac);

	if (li->frontsector->floorheight != li->backsector->floorheight)
	{
	    slope = FixedDiv (openbottom - shootz , dist);
	    if (slope > bottomslope)
		bottomslope = slope;
	}
		
	if (li->frontsector->ceilingheight != li->backsector->ceilingheight)
	{
	    slope = FixedDiv (opentop - shootz , dist);
	    if (slope < topslope)
		topslope = slope;
	}
		
	if (topslope <= bottomslope)
	    return false;		// stop

	return true;			// shot continues
    }

    // shoot a thing
    th = in->d.thing;

    // [kg] maybe target is already chosen
    if(linetarget)
    {
	if(linetarget != th)
	    return true; // it is not a chosen one
    } else
    {
	// [kg] choosing a target skips these checks
	if (th == shootthing)
	    return true;			// can't shoot self

	if (!(th->flags&MF_SHOOTABLE))
	    return true;			// corpse or something
    }

    // check angles to see if the thing can be aimed at
    dist = FixedMul (attackrange, in->frac);
    thingtopslope = FixedDiv (th->z+th->height - shootz , dist);

    if (thingtopslope < bottomslope)
	return true;			// shot over the thing

    thingbottomslope = FixedDiv (th->z - shootz, dist);

    if (thingbottomslope > topslope)
	return true;			// shot under the thing
    
    // this thing can be hit!
    if (thingtopslope > topslope)
	thingtopslope = topslope;
    
    if (thingbottomslope < bottomslope)
	thingbottomslope = bottomslope;

    aimslope = (thingtopslope+thingbottomslope)/2;
    linetarget = th;

    return false;			// don't go any farther
}


//
// PTR_ShootTraverse
//
boolean PTR_ShootTraverse (intercept_t* in)
{
    fixed_t		x;
    fixed_t		y;
    fixed_t		z;
    fixed_t		frac;
    fixed_t		dz;
    
    line_t*		li;
    
    mobj_t*		th;

    fixed_t		slope;
    fixed_t		dist;
    fixed_t		thingtopslope;
    fixed_t		thingbottomslope;

    uint16_t canopass = ~mobjinfo[la_pufftype].canpass;

    sector_t *frontsector = NULL;
    sector_t *backsector = NULL;

    extraplane_t *pl;

    if (in->isaline)
    {
	boolean is_hit = true;

	li = in->d.line;
	
	if (li->special)
	    P_ExtraLineSpecial(shootthing, li, P_PointOnLineSide(shootthing->x, shootthing->y, li), EXTRA_HITSCAN);

	if( canopass & li->blocking )
	{
	    frontsector = li->frontsector;
	    goto hitline_check3d;
	}

	// crosses a two sided line
	P_LineOpening (li);

	// [kg] position check
	if(P_PointOnLineSide(trace.x, trace.y, li))
	{
		backsector = li->frontsector;
		frontsector = li->backsector;
	} else
	{
		frontsector = li->frontsector;
		backsector = li->backsector;
	}

	dist = FixedMul (attackrange, in->frac);

	slope = FixedDiv (openbottom - shootz , dist);
	if (slope > aimslope)
	    goto hitline_check3d;

	slope = FixedDiv (opentop - shootz , dist);
	if (slope < aimslope)
	    goto hitline_check3d;

	is_hit = false;

	// [kg] 3D sides check
	pl = backsector->exfloor;
	if(pl)
	{
		dz = FixedMul(aimslope, FixedMul(in->frac, attackrange));
		z = shootz + dz;
		while(pl)
		{
			if(canopass & pl->blocking && z >= pl->source->floorheight && z <= pl->source->ceilingheight && pl->source->floorheight < backsector->ceilingheight && pl->source->ceilingheight > backsector->floorheight)
				goto hitline;
			pl->hitover = z >= *pl->height;
			pl = pl->next;
		}
		pl = backsector->exceiling;
		while(pl)
		{
			pl->hitover = z >= *pl->height;
			pl = pl->next;
		}
	}

      hitline_check3d:

	// [kg] 3D floor planes check
	if(aimslope < 0)
	{
		pl = frontsector->exfloor;
		if(pl)
		{
			dz = FixedMul(aimslope, FixedMul(in->frac, attackrange));
			z = shootz + dz;
			if(z < frontsector->floorheight)
				z = frontsector->floorheight;
			while(pl)
			{
				if(canopass & pl->blocking && *pl->height > frontsector->floorheight && *pl->height < frontsector->ceilingheight && ((pl->hitover && z < *pl->height) || (!pl->hitover && z >= *pl->height)))
				{
					// position a bit closer
					frac = in->frac - FixedDiv (4*FRACUNIT,attackrange);
					dz = FixedMul (aimslope, FixedMul(frac, attackrange));
					z = *pl->height;
					frac = -FixedDiv( FixedMul(frac, shootz - *pl->height), dz);
					goto hit3dplane;
				}
				pl = pl->next;
			}
		}
	}
	// [kg] 3D ceiling planes check
	if(aimslope > 0)
	{
		pl = frontsector->exceiling;
		if(pl)
		{
			dz = FixedMul(aimslope, FixedMul(in->frac, attackrange));
			z = shootz + dz;
			if(z > frontsector->ceilingheight)
				z = frontsector->ceilingheight;
			while(pl)
			{
				if(canopass & pl->blocking && *pl->height > frontsector->floorheight && *pl->height < frontsector->ceilingheight && ((pl->hitover && z < *pl->height) || (!pl->hitover && z >= *pl->height)))
				{
					// position a bit closer
					frac = in->frac - FixedDiv (4*FRACUNIT,attackrange);
					dz = FixedMul (aimslope, FixedMul(frac, attackrange));
					z = *pl->height-1;
					frac = -FixedDiv( FixedMul(frac, shootz - *pl->height), dz);
					goto hit3dplane;
				}
				pl = pl->next;
			}
		}
	}

	if(!is_hit)
	    // shot continues
	    return true;

	// hit line
      hitline:

#ifndef SERVER
	if(netgame)
	    return false;
#endif

	// [kg] position check
	if(!frontsector)
	{
		if(P_PointOnLineSide(trace.x, trace.y, li))
		{
			backsector = li->frontsector;
			frontsector = li->backsector;
		} else
		{
			frontsector = li->frontsector;
			backsector = li->backsector;
		}
	}

	// position a bit closer
	frac = in->frac - FixedDiv (4*FRACUNIT,attackrange);
	dz = FixedMul (aimslope, FixedMul(frac, attackrange));
	z = shootz + dz;

	if(frontsector)
	{
		if(aimslope < 0)
		{
			if(z < frontsector->floorheight)
				frac = -FixedDiv( FixedMul(frac, shootz - frontsector->floorheight), dz);
		}
		if(aimslope > 0)
		{
			if(z > frontsector->ceilingheight)
				frac = FixedDiv( FixedMul(frac, frontsector->ceilingheight - shootz), dz);
		}

		if (frontsector->ceilingpic == skyflatnum)
		{
		    // don't shoot the sky!
		    if (z > frontsector->ceilingheight)
			return false;
		    
		    // it's a sky hack wall
		    if	(backsector && backsector->ceilingpic == skyflatnum && backsector->ceilingheight <= z)
			return false;		
		}
	}

      hit3dplane:

	x = trace.x + FixedMul (trace.dx, frac);
	y = trace.y + FixedMul (trace.dy, frac);

	// Spawn bullet puffs.
	P_SpawnPuff (x,y,z, NULL, shootthing);
	
	// don't go any farther
	return false;	
    }
    
    // shoot a thing
    th = in->d.thing;
    if (th == shootthing)
	return true;		// can't shoot self
    
    if (!(th->flags&MF_SHOOTABLE))
	return true;		// corpse or something

    // [kg] blocking
    if(!(canopass & th->blocking))
	return true;

    // check angles to see if the thing can be aimed at
    dist = FixedMul (attackrange, in->frac);
    thingtopslope = FixedDiv (th->z+th->height - shootz , dist);

    if (thingtopslope < aimslope)
	return true;		// shot over the thing

    thingbottomslope = FixedDiv (th->z - shootz, dist);

    if (thingbottomslope > aimslope)
	return true;		// shot under the thing

    
    // hit thing
    // position a bit closer
    frac = in->frac - FixedDiv (10*FRACUNIT,attackrange);

    x = trace.x + FixedMul (trace.dx, frac);
    y = trace.y + FixedMul (trace.dy, frac);
    z = shootz + FixedMul (aimslope, FixedMul(frac, attackrange));

    linetarget = th;

#ifndef SERVER
    if(netgame)
	return false;
#endif

    // Spawn bullet puffs or blod spots,
    // depending on target type.
    if (th->flags & MF_NOBLOOD)
	P_SpawnPuff (x,y,z, th, shootthing);
    else
	P_SpawnBlood (x,y,z, th, shootthing);

    if (la_damage)
	P_DamageMobj (th, shootthing, shootthing, la_damage, mobjinfo[la_pufftype].damagetype);

    // don't go any farther
    return false;
}


//
// P_AimLineAttack
//
fixed_t
P_AimLineAttack
( mobj_t*	t1,
  angle_t	angle,
  fixed_t	distance,
  mobj_t*	target )
{
    fixed_t	x2;
    fixed_t	y2;
	
    angle >>= ANGLETOFINESHIFT;
    shootthing = t1;

    x2 = t1->x + (distance>>FRACBITS)*finecosine[angle];
    y2 = t1->y + (distance>>FRACBITS)*finesine[angle];
    shootz = t1->z + t1->info->shootz;

    // can't shoot outside view angles
    topslope = 100*FRACUNIT/160;	
    bottomslope = -100*FRACUNIT/160;
    
    attackrange = distance;

    if(target)
	linetarget = target;
    else
	linetarget = NULL;
	
    P_PathTraverse ( t1->x, t1->y,
		     x2, y2,
		     PT_ADDLINES|PT_ADDTHINGS,
		     PTR_AimTraverse );
		
    if (linetarget)
	return aimslope;

    return 0;
}
 

//
// P_LineAttack
// If damage == 0, it is just a test trace
// that will leave linetarget set.
//
void
P_LineAttack
( mobj_t*	t1,
  angle_t	angle,
  fixed_t	distance,
  fixed_t	slope,
  int		damage,
  fixed_t	zo,
  fixed_t	xo )
{
	fixed_t	x1 = t1->x;
	fixed_t	y1 = t1->y;
	fixed_t	x2;
	fixed_t	y2;
	extraplane_t *pl;

	if(xo)
	{
		x1 += FixedMul(xo, finecosine[(angle+ANG90)>>ANGLETOFINESHIFT]);
		y1 += FixedMul(xo, finesine[(angle+ANG90)>>ANGLETOFINESHIFT]);
	}

	angle >>= ANGLETOFINESHIFT;
	shootthing = t1;
	la_damage = damage;
	x2 = x1 + (distance>>FRACBITS)*finecosine[angle];
	y2 = y1 + (distance>>FRACBITS)*finesine[angle];
	shootz = t1->z + t1->info->shootz + zo;
	attackrange = distance;
	aimslope = slope;

	linetarget = NULL;

	// 3D planes check
	pl = t1->subsector->sector->exfloor;
	if(pl)
	{
		while(pl)
		{
			pl->hitover = shootz >= *pl->height;
			pl = pl->next;
		}
		pl = t1->subsector->sector->exceiling;
		while(pl)
		{
			pl->hitover = shootz >= *pl->height;
			pl = pl->next;
		}
	}

	P_PathTraverse ( x1, y1, x2, y2, PT_ADDLINES|PT_ADDTHINGS, PTR_ShootTraverse );
}
 


//
// USE LINES
//
mobj_t*		usething;

boolean	PTR_UseTraverse (intercept_t* in)
{
    if (!in->d.line->special)
    {
	P_LineOpening (in->d.line);
	if (openrange <= 0 && usething->info->activesound)
	{
	    S_StartSound (usething, usething->info->activesound, SOUND_BODY);
	    
	    // can't use through a wall
	    return false;	
	}
	// not a special line, but keep checking
	return true ;		
    }

#ifndef SERVER
    if(!netgame)
#endif
    P_ExtraLineSpecial(usething, in->d.line, P_PointOnLineSide(usething->x, usething->y, in->d.line), EXTRA_USE);

    // can't use for than one special line in a row
    return false;
}


//
// P_UseLines
// Looks for special lines in front of the player to activate.
//
void P_UseLines (player_t*	player) 
{
    int		angle;
    fixed_t	x1;
    fixed_t	y1;
    fixed_t	x2;
    fixed_t	y2;
	
    usething = player->mo;
		
    angle = player->mo->angle >> ANGLETOFINESHIFT;

    x1 = player->mo->x;
    y1 = player->mo->y;
    x2 = x1 + (USERANGE>>FRACBITS)*finecosine[angle];
    y2 = y1 + (USERANGE>>FRACBITS)*finesine[angle];
	
    P_PathTraverse ( x1, y1, x2, y2, PT_ADDLINES, PTR_UseTraverse );
}


//
// RADIUS ATTACK
//
mobj_t*		bombsource;
mobj_t*		bombspot;
int		bombdamage;
fixed_t		bombdist;
boolean		bombhurt;
int		bombtype;

//
// PIT_RadiusAttack
// "bombsource" is the creature
// that caused the explosion at "bombspot".
//
boolean PIT_RadiusAttack (mobj_t* thing)
{
    fixed_t	dx;
    fixed_t	dy;
    fixed_t	dz;
    fixed_t	dist;
    int		damage;
	
    if (!(thing->flags & MF_SHOOTABLE) )
	return true;

    // Boss spider and cyborg
    // take no damage from concussion.
    if (thing->flags & MF_NORADIUSDMG)
	return true;

    // [kg] don't hurt origin
    if(!bombhurt && thing == bombsource)
	return true;

    dx = abs(thing->x - bombspot->x);
    dy = abs(thing->y - bombspot->y);
    
    dist = dx > dy ? dx : dy;
    dx = dist - thing->radius;
    if (dx < 0)
	dx = 0;

    // [kg] check z distance
    if(!(thing->flags & MF_NORADIUSZ))
    {
	dz = abs(thing->z - bombspot->z);
	if(thing->z < bombspot->z)
	    dz = dz - thing->height;
	if(dz < 0)
	    dz = 0;
	dist = dx > dz ? dx : dz;
    }

    if (dist >= bombdist)
	return true;	// out of range

    damage = FixedMul(bombdamage, FixedDiv(bombdist - dist, bombdist)) / FRACUNIT;

    if (damage == 0)
	return true;	// out of range

    if ( P_CheckSight (thing, bombspot) )
    {
	// must be in direct path
	P_DamageMobj (thing, bombspot, bombsource, damage, bombtype);
    }
    
    return true;
}


//
// P_RadiusAttack
// Source is the creature that caused the explosion at spot.
//
void
P_RadiusAttack
( mobj_t*	spot,
  mobj_t*	source,
  fixed_t	range,
  int		damage,
  boolean	hurtsource,
  int		damagetype )
{
    int		x;
    int		y;
    
    int		xl;
    int		xh;
    int		yl;
    int		yh;
    
    bombdist = range;
    yh = (spot->y + range - bmaporgy)>>MAPBLOCKSHIFT;
    yl = (spot->y - range - bmaporgy)>>MAPBLOCKSHIFT;
    xh = (spot->x + range - bmaporgx)>>MAPBLOCKSHIFT;
    xl = (spot->x - range - bmaporgx)>>MAPBLOCKSHIFT;
    bombspot = spot;
    bombsource = source;
    bombdamage = damage * FRACUNIT;
    bombhurt = hurtsource;
    bombtype = damagetype;
	
    for (y=yl ; y<=yh ; y++)
	for (x=xl ; x<=xh ; x++)
	    P_BlockThingsIterator (x, y, PIT_RadiusAttack );
}



//
// SECTOR HEIGHT CHANGING
// After modifying a sectors floor or ceiling height,
// call this routine to adjust the positions
// of all things that touch the sector.
//
// If anything doesn't fit anymore, true will be returned.
// If crunch is true, they will take damage
//  as they are being crushed.
// If Crunch is false, you should set the sector height back
//  the way it was and call P_ChangeSector again
//  to undo the changes.
//
boolean		nofit;
// [kg] Lua callback
static boolean docallback;
static sector_t *cs_sector;
static int	cs_lua_func;
static int	cs_lua_arg;

//
// PIT_ChangeSector
//
boolean PIT_ChangeSector (mobj_t*	thing)
{
    mobj_t*	mo;
	
    if (P_ThingHeightClip (thing))
    {
	// keep checking
	return true;
    }

    // crunch bodies to giblets
    if (thing->health <= 0 && thing->flags & MF_CORPSE && thing->info->crushstate)
    {
#ifndef SERVER
	if(!netgame)
	{
#endif

	P_SetMobjAnimation(thing, ANIM_CRUSH, 0);

#ifdef SERVER
	// tell clients about this
	SV_UpdateMobj(thing, SV_MOBJF_AUTO | SV_MOBJF_STATE);
#else
	}
#endif
	// keep checking
	return true;		
    }

    // crunch dropped items
    if (thing->flags & MF_DROPPED)
    {
#ifdef SERVER
	P_RemoveMobj (thing, true);
#else
	if(!netgame)
	    P_RemoveMobj (thing);
#endif
	// keep checking
	return true;		
    }

    if (! (thing->flags & MF_SHOOTABLE) )
    {
	// assume it is bloody gibs or something
	return true;			
    }
    
    nofit = true;

    // [kg] Lua callback
    if(docallback)
    {
	if(!L_CrushThing(thing, cs_sector, cs_lua_func, cs_lua_arg))
		docallback = false;
    }

    // keep checking (crush other things)	
    return true;	
}



//
// P_ChangeSector
//
boolean
P_ChangeSector
( sector_t*	sector,
  int lua_func,
  int lua_arg )
{
    int		x;
    int		y;

    if(lua_func == L_NoRef())
	docallback = false;
    else
	docallback = true;

    nofit = false;
    cs_sector = sector;

    // [kg] Lua callback
    cs_lua_func = lua_func;
    cs_lua_arg = lua_arg;
	
    // re-check heights for all things near the moving sector
    for (x=sector->blockbox[BOXLEFT] ; x<= sector->blockbox[BOXRIGHT] ; x++)
	for (y=sector->blockbox[BOXBOTTOM];y<= sector->blockbox[BOXTOP] ; y++)
	    P_BlockThingsIterator (x, y, PIT_ChangeSector);
	
	
    return nofit;
}

//
// [kg] thing Z collision search
// this will find things under and over this one

boolean PIT_CheckThingZ(mobj_t* thing)
{
	fixed_t blockdist;

	if(thing == tmthing)
		return true;

	if(!(thing->flags & MF_SOLID) || thing->flags & MF_NOCLIP)
		return true;

	if(!(~tmthing->canpass & thing->blocking))
		return true;

	if(tmthing->flags & MF_MISSILE && tmthing->source == thing)
		return true;

	blockdist = thing->radius + tmthing->radius;

	if( abs(thing->x - tmx) >= blockdist || abs(thing->y - tmy) >= blockdist )
		return true;	

	if(tmthing->z <= thing->z)
	{
		if(thzctop)
		{
			if(thing->z < thzctop->z)
				thzctop = thing;
		} else
			thzctop = thing;
	}
	if(tmthing->z >= thing->z + thing->height)
	{
		if(thzcbot)
		{
			if(thing->z + thing->height > thzcbot->z + thzcbot->height)
				thzcbot = thing;
		} else
			thzcbot = thing;
	}
	return true;
}

// this will find things under and over this one
void P_CheckPositionZ(mobj_t *thing)
{
	int xl;
	int xh;
	int yl;
	int yh;
	int bx;
	int by;

	if(thing->flags & MF_NOCLIP)
		return;

	thzcbot = NULL;
	thzctop = NULL;

	if(thing->flags & (MF_NOCLIP | MF_TROUGHMOBJ))
		return;

	tmthing = thing;
	tmx = thing->x;
	tmy = thing->y;

	tmbbox[BOXTOP] = tmy + tmthing->radius;
	tmbbox[BOXBOTTOM] = tmy - tmthing->radius;
	tmbbox[BOXRIGHT] = tmx + tmthing->radius;
	tmbbox[BOXLEFT] = tmx - tmthing->radius;

	xl = (tmbbox[BOXLEFT] - bmaporgx)>>MAPBLOCKSHIFT;
	xh = (tmbbox[BOXRIGHT] - bmaporgx)>>MAPBLOCKSHIFT;
	yl = (tmbbox[BOXBOTTOM] - bmaporgy)>>MAPBLOCKSHIFT;
	yh = (tmbbox[BOXTOP] - bmaporgy)>>MAPBLOCKSHIFT;

	for (bx=xl ; bx<=xh ; bx++)
		for (by=yl ; by<=yh ; by++)
			P_BlockThingsIterator(bx,by,PIT_CheckThingZ);

	return;
}

// this will setup correct floorz/ceilingz values for this thing
void P_CheckPositionLines(mobj_t *thing)
{
	int xl;
	int xh;
	int yl;
	int yh;
	int bx;
	int by;
	sector_t *sec = thing->subsector->sector;

	if(thing->flags & MF_NOCLIP)
	{
		tmfloorz = tmdropoffz = sec->floorheight;
		tmceilingz = sec->ceilingheight;
		return;
	}

	tmthing = thing;
	tmflags = thing->flags;

	tmx = thing->x;
	tmy = thing->y;

	tmbbox[BOXTOP] = tmy + tmthing->radius;
	tmbbox[BOXBOTTOM] = tmy - tmthing->radius;
	tmbbox[BOXRIGHT] = tmx + tmthing->radius;
	tmbbox[BOXLEFT] = tmx - tmthing->radius;

	xl = (tmbbox[BOXLEFT] - bmaporgx)>>MAPBLOCKSHIFT;
	xh = (tmbbox[BOXRIGHT] - bmaporgx)>>MAPBLOCKSHIFT;
	yl = (tmbbox[BOXBOTTOM] - bmaporgy)>>MAPBLOCKSHIFT;
	yh = (tmbbox[BOXTOP] - bmaporgy)>>MAPBLOCKSHIFT;

	// The base floor / ceiling is from the subsector
	// that contains the point.
	// Any contacted lines the step closer together
	// will adjust them.
	tmfloorz = tmdropoffz = sec->floorheight;
	tmceilingz = sec->ceilingheight;

	validcount++;
	numspechit = 0;

	// check lines
	for (bx=xl ; bx<=xh ; bx++)
		for (by=yl ; by<=yh ; by++)
			P_BlockLinesIterator (bx,by,PIT_CheckLine);
}

boolean PIT_UpdateThingZ(mobj_t* thing)
{
	fixed_t blockdist;
	fixed_t top;
	fixed_t newz = thing->z;

	if(thing == tmthing)
		return true;

	if(thing->flags & MF_NOCLIP)
		return true;

	if(!(~tmthing->canpass & thing->blocking))
		return true;

	if(tmthing->flags & MF_MISSILE)
		return true;

	blockdist = thing->radius + tmthing->radius;

	if( abs(thing->x - tmx) >= blockdist || abs(thing->y - tmy) >= blockdist )
		return true;

	top = thing->z + thing->height;

	if(tmthingtopnew > tmthingtop)
	{
		// raising
		if(thing->z >= tmthingtop && thing->z < tmthingtopnew)
		{
			// raise this one too
			newz = tmthingtopnew;
		}
	} else
	{
		// lowering
		if(tmthing->z >= top && tmthingnewz < top)
		{
			// blocked by this thing
			tmthingnewz = top;
			// fix positions
			tmthingtopnew = tmthingnewz + tmthing->height;
		}
	}

	if(newz != thing->z)
	{
		mobj_t *bkthing = tmthing;
		fixed_t bktop = tmthingtop;
		fixed_t bktopnew = tmthingtopnew;
		fixed_t bknewz = tmthingnewz;
		fixed_t bkx = tmx;
		fixed_t bky = tmy;

		P_UpdateThingZ(thing, newz);

		tmthing = bkthing;
		tmthingtop = bktop;
		tmthingtopnew = bktopnew;
		tmthingnewz = bknewz;
		tmx = bkx;
		tmy = bky;
	}

	return true;
}

// this will update thing Z based on sector movement
// might be called recursively for stacked things
boolean P_UpdateThingZ(mobj_t *thing, fixed_t newz)
{
	int xl;
	int xh;
	int yl;
	int yh;
	int bx;
	int by;

	if(thing->flags & (MF_NOCLIP | MF_TROUGHMOBJ))
	{
		thing->z = newz;
		return true;
	}

	tmthingtop = thing->z + thing->height;
	tmthingtopnew = newz + thing->height;
	tmthingnewz = newz;

	tmthing = thing;
	tmx = thing->x;
	tmy = thing->y;

	tmbbox[BOXTOP] = tmy + tmthing->radius;
	tmbbox[BOXBOTTOM] = tmy - tmthing->radius;
	tmbbox[BOXRIGHT] = tmx + tmthing->radius;
	tmbbox[BOXLEFT] = tmx - tmthing->radius;

	xl = (tmbbox[BOXLEFT] - bmaporgx)>>MAPBLOCKSHIFT;
	xh = (tmbbox[BOXRIGHT] - bmaporgx)>>MAPBLOCKSHIFT;
	yl = (tmbbox[BOXBOTTOM] - bmaporgy)>>MAPBLOCKSHIFT;
	yh = (tmbbox[BOXTOP] - bmaporgy)>>MAPBLOCKSHIFT;

	for (bx=xl ; bx<=xh ; bx++)
		for (by=yl ; by<=yh ; by++)
			P_BlockThingsIterator(bx,by,PIT_UpdateThingZ);

	thing->z = tmthingnewz;

	return !tmblocked;
}

