// generic thinkers
// by kgsws
#include "z_zone.h"
#include "doomdef.h"
#include "p_local.h"
#include "s_sound.h"
#include "doomstat.h"
#include "r_state.h"
#include "w_wad.h"

#include "p_generic.h"

#include "kg_lua.h"

generic_plane_t *crush_gp;

//
// Ceiling
//

void T_GenericCeiling(generic_plane_t *gp)
{
	fixed_t speed;
	sector_t *sec = gp->info.sector;

	if(gp->speed <= 0)
		// suspended
		return;

	crush_gp = gp;

	if(gp->info.startz < gp->info.stopz)
	{
		// raise
		if(sec->ceilingheight + gp->speed > gp->info.stopz)
			speed = gp->info.stopz - sec->ceilingheight;
		else
			speed = gp->speed;
		sec->ceilingheight += speed;
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
				sec->ceilingheight -= speed;
				P_ChangeSector(sec, L_NoRef(), L_NoRef());
			}
		}
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
		sec->ceilingdata = NULL;
		sec->ceilingpic = gp->info.stoppic;
		L_FinishGeneric(gp, false);
		P_RemoveThinker(&gp->thinker);
	} else
	{
	    if(!(leveltime&7) && gp->info.movesound)
		S_StartSound((mobj_t *)&sec->soundorg, gp->info.movesound, SOUND_BODY);
	}
}

generic_plane_t *P_GenericSectorCeiling(generic_info_t *info)
{
	generic_plane_t *gp;

	// check speed
	if(info->speed <= 0)
		return NULL;

	// remove old one
	if(info->sector->ceilingdata)
	{
		L_FinishGeneric(info->sector->ceilingdata, true);
		P_RemoveThinker(info->sector->ceilingdata);
	}
	// add new one
	gp = Z_Malloc(sizeof(generic_plane_t), PU_LEVSPEC, 0);
	P_AddThinker(&gp->thinker, TT_GENPLANE);
	gp->info = *info;
	gp->thinker.function.acp1 = (actionf_p1)T_GenericCeiling;
	gp->lua_action = L_NoRef();
	gp->lua_arg = L_NoRef();
	gp->lua_crush = L_NoRef();
	info->sector->ceilingdata = gp;
	// set starting parameters
	gp->gp = &info->sector->ceilingdata;
	gp->speed = gp->info.speed;
	info->sector->ceilingheight = info->startz;
	info->sector->ceilingpic = info->startpic;
	if(info->startsound)
		S_StartSound((mobj_t *)&info->sector->soundorg, info->startsound, SOUND_BODY);

	return gp;
}

//
// Floor
//

void T_GenericFloor(generic_plane_t *gp)
{
	fixed_t speed;
	sector_t *sec = gp->info.sector;

	if(gp->speed <= 0)
		// suspended
		return;

	crush_gp = gp;

	if(gp->info.startz > gp->info.stopz)
	{
		// lower
		if(sec->floorheight - gp->speed < gp->info.stopz)
			speed = sec->floorheight - gp->info.stopz;
		else
			speed = gp->speed;
		sec->floorheight -= speed;
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
				sec->floorheight += speed;
				P_ChangeSector(sec, L_NoRef(), L_NoRef());
			}
		}
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
		sec->floordata = NULL;
		sec->floorpic = gp->info.stoppic;
		L_FinishGeneric(gp, false);
		P_RemoveThinker(&gp->thinker);
	} else
	{
	    if(!(leveltime&7) && gp->info.movesound)
		S_StartSound((mobj_t *)&sec->soundorg, gp->info.movesound, SOUND_BODY);
	}
}

generic_plane_t *P_GenericSectorFloor(generic_info_t *info)
{
	generic_plane_t *gp;

	// check speed
	if(info->speed <= 0)
		return NULL;

	// remove old one
	if(info->sector->floordata)
	{
		L_FinishGeneric(info->sector->floordata, true);
		P_RemoveThinker(info->sector->floordata);
	}
	// add new one
	gp = Z_Malloc(sizeof(generic_plane_t), PU_LEVSPEC, 0);
	P_AddThinker(&gp->thinker, TT_GENPLANE);
	gp->info = *info;
	gp->thinker.function.acp1 = (actionf_p1)T_GenericFloor;
	gp->lua_action = L_NoRef();
	gp->lua_arg = L_NoRef();
	gp->lua_crush = L_NoRef();
	info->sector->floordata = gp;
	// set starting parameters
	gp->gp = &info->sector->floordata;
	gp->speed = gp->info.speed;
	info->sector->floorheight = info->startz;
	info->sector->floorpic = info->startpic;
	if(info->startsound)
		S_StartSound((mobj_t *)&info->sector->soundorg, info->startsound, SOUND_BODY);

	return gp;
}

//
// Caller
//

generic_plane_t *P_GenericSectorCaller(generic_call_t *info, int dest)
{
	generic_plane_t *gp;
	generic_plane_t **ptr = (generic_plane_t **)&info->sector->floordata;

	// check ticrate
	if(info->ticrate <= 0)
		return NULL;

	ptr += dest;

	// remove old one
	if(*ptr)
	{
		L_FinishGeneric(*ptr, true);
		P_RemoveThinker((thinker_t*)*ptr);
	}
	// add new one
	gp = Z_Malloc(sizeof(generic_plane_t), PU_LEVSPEC, 0);
	P_AddThinker(&gp->thinker, TT_SECCALL);
	gp->call = *info;
	gp->thinker.function.acp1 = (actionf_p1)T_GenericCaller;
	gp->lua_action = L_NoRef();
	gp->lua_arg = L_NoRef();
	gp->lua_crush = L_NoRef();
	*ptr = gp;
	// set starting parameters
	gp->gp = (void**)ptr;
	gp->call.curtics = gp->call.ticrate;
	gp->speed = gp->call.ticrate;

	return gp;
}

//
// Texture scroll
//

void T_TexScroll(generic_line_t *ga)
{
	ga->side->textureoffset += ga->scroll.x;
	ga->side->rowoffset += ga->scroll.y;
}

generic_line_t *P_TextureScroller(line_t *line, fixed_t x, fixed_t y, int side)
{
	generic_line_t *ga;
	generic_line_t **ptr = (generic_line_t **)&line->specialdata[side];

	// check speed
	if(!x && !y)
		return NULL;

	// check sidedef
	if(line->sidenum[side] < 0)
		return NULL;

	// remove old one
	if(*ptr)
		P_RemoveThinker((thinker_t*)*ptr);
	// add new one
	ga = Z_Malloc(sizeof(generic_line_t), PU_LEVSPEC, 0);
	P_AddThinker(&ga->thinker, TT_SCROLLTEX);
	ga->thinker.function.acp1 = (actionf_p1)T_TexScroll;
	ga->side = &sides[line->sidenum[side]];
	ga->scroll.x = x;
	ga->scroll.y = y;
	*ptr = ga;

	return ga;
}

//
// Mobj ticker
//

void P_AddMobjTicker(mobj_t *mo, int id, int ticrate, int action, int arg, int patch)
{
	generic_ticker_t *gt = mo->generic_ticker;

	// find last or same
	while(gt)
	{
		if(gt->id == id)
			break;
		if(!gt->next)
			break;
		gt = gt->next;
	}

	if(!gt || gt->id != id)
	{
		// create new
		generic_ticker_t *new;
		new = Z_Malloc(sizeof(generic_ticker_t), PU_LEVEL, 0);
		new->next = NULL;
		new->id = id;
		new->ticrate = ticrate;
		new->curtics = ticrate;
		new->lua_action = action;
		new->lua_arg = arg;
		if(patch >= 0)
			new->patch = W_CacheLumpNum(patch);			
		else
			new->patch = NULL;

		if(gt)
			// add to chain
			gt->next = new;
		else
			// new chain
			mo->generic_ticker = new;

		return;
	}

	// modify existing
	L_Unref(&gt->lua_action);
	L_Unref(&gt->lua_arg);
	gt->ticrate = ticrate;
	gt->curtics = ticrate;
	gt->lua_action = action;
	gt->lua_arg = arg;
	if(patch >= 0)
		gt->patch = W_CacheLumpNum(patch);			
	else
		gt->patch = NULL;

}

void P_RemoveMobjTicker(mobj_t *mo, int id)
{
	generic_ticker_t *gt = mo->generic_ticker;
	generic_ticker_t **last = &mo->generic_ticker;

	while(gt)
	{
		generic_ticker_t *cur = gt;
		gt = gt->next;

		if(cur->id == id)
		{
			// remove this ticker
			*last = cur->next;
			// unref
			L_Unref(&cur->lua_action);
			L_Unref(&cur->lua_arg);
			// free
			Z_Free(cur);
			// finished
			break;
		}
		// check next
		last = &cur->next;
	}
}

void P_RemoveMobjTickers(mobj_t *mo)
{
	generic_ticker_t *gt = mo->generic_ticker;
	while(gt)
	{
		generic_ticker_t *cur = gt;
		gt = gt->next;
		// unref
		L_Unref(&cur->lua_action);
		L_Unref(&cur->lua_arg);
		// free
		Z_Free(cur);
	}
	mo->generic_ticker = NULL;
}

