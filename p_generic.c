// generic floor / ceiling movement
// by kgsws
#include "z_zone.h"
#include "doomdef.h"
#include "p_local.h"
#include "s_sound.h"
#include "doomstat.h"
#include "r_state.h"

#include "p_generic.h"

#include "kg_lua.h"

//
// Ceiling
//

void T_GenericCeiling(generic_plane_t *gp)
{
	fixed_t speed;
	sector_t *sec = gp->info.sector;

	if(gp->info.startz < gp->info.stopz)
	{
		// raise
		if(sec->ceilingheight + gp->speed > gp->info.stopz)
			speed = gp->info.stopz - sec->ceilingheight;
		else
			speed = gp->speed;
		sec->ceilingheight += speed;
		P_ChangeSector(sec, gp->lua_crush, gp->lua_arg);
	} else
	if(gp->info.startz > gp->info.stopz)
	{
		// lower
		if(sec->ceilingheight - gp->speed < gp->info.stopz)
			speed = sec->ceilingheight - gp->info.stopz;
		else
			speed = gp->speed;
		sec->ceilingheight -= speed;
		if(P_ChangeSector(sec, gp->lua_crush, gp->lua_arg))
		{
			// move blocked
			if(gp->info.crushspeed)
			{
				// change speed and continue;
				gp->speed = gp->info.crushspeed;
			} else
			{
				// return back
				sec->ceilingheight += speed;
				P_ChangeSector(sec, L_NoRef(), L_NoRef());
			}
		}
	}
	// finished ?
	if(sec->ceilingheight == gp->info.stopz)
	{
		if(gp->info.stopsound)
			S_StartSound((mobj_t *)&sec->soundorg, gp->info.stopsound, SOUND_BODY);
		sec->specialdata = NULL;
		sec->ceilingpic = gp->info.stoppic;
		L_FinishGeneric(gp, false);
		P_RemoveThinker(&gp->thinker);
	} else
	{
	    if(!(leveltime&7) && gp->info.movesound)
		S_StartSound((mobj_t *)&sec->soundorg, gp->info.movesound, SOUND_BODY);
	}
}

void P_GenericSectorCeiling(generic_info_t *info)
{
	generic_plane_t *gp;

	// remove old one
	if(info->sector->specialdata)
	{
		L_FinishGeneric(info->sector->specialdata, true);
		P_RemoveThinker(info->sector->specialdata);
	}
	// check speed
	if(info->speed <= 0)
		return;
	// add new one
	gp = Z_Malloc(sizeof(generic_plane_t), PU_LEVSPEC, 0);
	P_AddThinker(&gp->thinker, TT_GENPLANE);
	gp->info = *info;
	gp->thinker.function.acp1 = (actionf_p1)T_GenericCeiling;
	gp->lua_action = L_NoRef();
	gp->lua_arg = L_NoRef();
	gp->lua_crush = L_NoRef();
	info->sector->specialdata = gp;
	// set starting parameters
	gp->speed = gp->info.speed;
	info->sector->ceilingheight = info->startz;
	info->sector->ceilingpic = info->startpic;
	if(info->startsound)
		S_StartSound((mobj_t *)&info->sector->soundorg, info->startsound, SOUND_BODY);
}

//
// Floor
//

void T_GenericFloor(generic_plane_t *gp)
{
	fixed_t speed;
	sector_t *sec = gp->info.sector;

	if(gp->info.startz > gp->info.stopz)
	{
		// lower
		if(sec->floorheight - gp->speed < gp->info.stopz)
			speed = sec->floorheight - gp->info.stopz;
		else
			speed = gp->speed;
		sec->floorheight -= speed;
		P_ChangeSector(sec, gp->lua_crush, gp->lua_arg);
	} else
	if(gp->info.startz < gp->info.stopz)
	{
		// raise
		if(sec->floorheight + gp->speed > gp->info.stopz)
			speed = gp->info.stopz - sec->floorheight;
		else
			speed = gp->speed;
		sec->floorheight += speed;
		if(P_ChangeSector(sec, gp->lua_crush, gp->lua_arg))
		{
			// move blocked
			if(gp->info.crushspeed)
			{
				// change speed and continue;
				gp->speed = gp->info.crushspeed;
			} else
			{
				// return back
				sec->floorheight -= speed;
				P_ChangeSector(sec, L_NoRef(), L_NoRef());
			}
		}
	}
	// finished ?
	if(sec->floorheight == gp->info.stopz)
	{
		if(gp->info.stopsound)
			S_StartSound((mobj_t *)&sec->soundorg, gp->info.stopsound, SOUND_BODY);
		sec->specialdata = NULL;
		sec->floorpic = gp->info.stoppic;
		L_FinishGeneric(gp, false);
		P_RemoveThinker(&gp->thinker);
	} else
	{
	    if(!(leveltime&7) && gp->info.movesound)
		S_StartSound((mobj_t *)&sec->soundorg, gp->info.movesound, SOUND_BODY);
	}
}

void P_GenericSectorFloor(generic_info_t *info)
{
	generic_plane_t *gp;

	// remove old one
	if(info->sector->specialdata)
	{
		L_FinishGeneric(info->sector->specialdata, true);
		P_RemoveThinker(info->sector->specialdata);
	}
	// check speed
	if(info->speed <= 0)
		return;
	// add new one
	gp = Z_Malloc(sizeof(generic_plane_t), PU_LEVSPEC, 0);
	P_AddThinker(&gp->thinker, TT_GENPLANE);
	gp->info = *info;
	gp->thinker.function.acp1 = (actionf_p1)T_GenericFloor;
	gp->lua_action = L_NoRef();
	gp->lua_arg = L_NoRef();
	gp->lua_crush = L_NoRef();
	info->sector->specialdata = gp;
	// set starting parameters
	gp->speed = gp->info.speed;
	info->sector->floorheight = info->startz;
	info->sector->floorpic = info->startpic;
	if(info->startsound)
		S_StartSound((mobj_t *)&info->sector->soundorg, info->startsound, SOUND_BODY);
}

//
// Caller
//

void P_GenericSectorCaller(generic_call_t *info)
{
	generic_plane_t *gp;

	// remove old one
	if(info->sector->specialdata)
	{
		L_FinishGeneric(info->sector->specialdata, true);
		P_RemoveThinker(info->sector->specialdata);
	}
	// check ticrate
	if(info->ticrate <= 0)
		return;
	// add new one
	gp = Z_Malloc(sizeof(generic_plane_t), PU_LEVSPEC, 0);
	P_AddThinker(&gp->thinker, TT_SECCALL);
	gp->call = *info;
	gp->thinker.function.acp1 = (actionf_p1)T_GenericCaller;
	gp->lua_action = L_NoRef();
	gp->lua_arg = L_NoRef();
	gp->lua_crush = L_NoRef();
	info->sector->specialdata = gp;
	// set starting parameters
	gp->call.curtics = gp->call.ticrate;
}

