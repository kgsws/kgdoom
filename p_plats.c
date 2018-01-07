#include "i_system.h"
#include "z_zone.h"
#include "m_random.h"

#include "doomdef.h"
#include "p_local.h"

// State.
#include "doomstat.h"
#include "r_state.h"

#ifdef SERVER
#include <netinet/in.h>
#include "network.h"
#include "sv_cmds.h"
#endif

//
// Move a plat up and down
//
void T_PlatRaise(plat_t* plat)
{
    result_e	res;
	
    switch(plat->status)
    {
      case up:
	res = T_MovePlane(plat->sector,
			  plat->speed,
			  plat->high,
			  plat->crush,0,1);
					
	if (plat->type == raiseAndChange
	    || plat->type == raiseToNearestAndChange)
	{
//	    if (!(leveltime&7))
//		S_StartSound((mobj_t *)&plat->sector->soundorg, sfx_stnmov, SOUND_BODY);
	}
	
				
	if (res == crushed && (!plat->crush))
	{
	    plat->count = plat->wait;
	    plat->status = down;
//	    S_StartSound((mobj_t *)&plat->sector->soundorg, sfx_pstart, SOUND_BODY);
#ifdef SERVER
	    // tell clients about this
	    SV_SectorPlatform(plat);
#endif
	}
	else
	{
	    if (res == pastdest)
	    {
		plat->count = plat->wait;
		plat->status = waiting;
//		S_StartSound((mobj_t *)&plat->sector->soundorg, sfx_pstop, SOUND_BODY);

		switch(plat->type)
		{
		  case blazeDWUS:
		  case downWaitUpStay:
		    P_RemoveActivePlat(plat);
		    break;
		    
		  case raiseAndChange:
		  case raiseToNearestAndChange:
		    P_RemoveActivePlat(plat);
		    break;
		    
		  default:
		    break;
		}
	    }
	}
	break;
	
      case	down:
	res = T_MovePlane(plat->sector,plat->speed,plat->low,false,0,-1);

	if (res == pastdest)
	{
	    plat->count = plat->wait;
	    plat->status = waiting;
//	    S_StartSound((mobj_t *)&plat->sector->soundorg,sfx_pstop, SOUND_BODY);
	}
	break;
	
      case	waiting:
	if (!--plat->count)
	{
	    if (plat->sector->floorheight == plat->low)
		plat->status = up;
	    else
		plat->status = down;
//	    S_StartSound((mobj_t *)&plat->sector->soundorg,sfx_pstart, SOUND_BODY);
#ifdef SERVER
	    // tell clients about this
	    SV_SectorPlatform(plat);
#endif
	}
      case	in_stasis:
	break;
    }
}


//
// Do Platforms
//  "amount" is only used for SOME platforms.
//
int
EV_DoPlat
( line_t*	line,
  plattype_e	type,
  int		amount )
{
    plat_t*	plat;
    int		secnum;
    int		rtn;
    sector_t*	sec;
	
    secnum = -1;
    rtn = 0;

    
    //	Activate all <type> plats that are in_stasis
    switch(type)
    {
      case perpetualRaise:
	P_ActivateInStasis(line->tag);
	break;
	
      default:
	break;
    }
	
    while ((secnum = P_FindSectorFromLineTag(line,secnum)) >= 0)
    {
	sec = &sectors[secnum];

	if (sec->specialdata)
	    continue;
	
	// Find lowest & highest floors around sector
	rtn = 1;
	plat = Z_Malloc( sizeof(*plat), PU_LEVSPEC, 0);
	P_AddThinker(&plat->thinker, TT_INVALID);
		
	plat->type = type;
	plat->sector = sec;
	plat->sector->specialdata = plat;
	plat->thinker.function.acp1 = (actionf_p1) T_PlatRaise;
	plat->crush = false;
	plat->tag = line->tag;
	
	switch(type)
	{
	  case raiseToNearestAndChange:
	    plat->speed = PLATSPEED/2;
	    sec->floorpic = sides[line->sidenum[0]].sector->floorpic;
	    plat->high = P_FindNextHighestFloor(sec,sec->floorheight);
	    plat->wait = 0;
	    plat->status = up;
	    // NO MORE DAMAGE, IF APPLICABLE
	    sec->special = 0;		

//	    S_StartSound((mobj_t *)&sec->soundorg,sfx_stnmov, SOUND_BODY);
	    break;
	    
	  case raiseAndChange:
	    plat->speed = PLATSPEED/2;
	    sec->floorpic = sides[line->sidenum[0]].sector->floorpic;
	    plat->high = sec->floorheight + amount*FRACUNIT;
	    plat->wait = 0;
	    plat->status = up;

//	    S_StartSound((mobj_t *)&sec->soundorg,sfx_stnmov, SOUND_BODY);
	    break;
	    
	  case downWaitUpStay:
	    plat->speed = PLATSPEED * 4;
	    plat->low = P_FindLowestFloorSurrounding(sec);

	    if (plat->low > sec->floorheight)
		plat->low = sec->floorheight;

	    plat->high = sec->floorheight;
	    plat->wait = 35*PLATWAIT;
	    plat->status = down;
//	    S_StartSound((mobj_t *)&sec->soundorg,sfx_pstart, SOUND_BODY);
	    break;
	    
	  case blazeDWUS:
	    plat->speed = PLATSPEED * 8;
	    plat->low = P_FindLowestFloorSurrounding(sec);

	    if (plat->low > sec->floorheight)
		plat->low = sec->floorheight;

	    plat->high = sec->floorheight;
	    plat->wait = 35*PLATWAIT;
	    plat->status = down;
//	    S_StartSound((mobj_t *)&sec->soundorg,sfx_pstart, SOUND_BODY);
	    break;
	    
	  case perpetualRaise:
	    plat->speed = PLATSPEED;
	    plat->low = P_FindLowestFloorSurrounding(sec);

	    if (plat->low > sec->floorheight)
		plat->low = sec->floorheight;

	    plat->high = P_FindHighestFloorSurrounding(sec);

	    if (plat->high < sec->floorheight)
		plat->high = sec->floorheight;

	    plat->wait = 35*PLATWAIT;
	    plat->status = P_Random()&1;

//	    S_StartSound((mobj_t *)&sec->soundorg,sfx_pstart, SOUND_BODY);
	    break;
	}
#ifdef SERVER
	// tell clients about this
	SV_SectorPlatform(plat);
#endif
    }
    return rtn;
}



void P_ActivateInStasis(int tag)
{
	thinker_t *think;
	plat_t *plat;

	if(!thinkercap.next)
		return;

	for(think = thinkercap.next; think != &thinkercap; think = think->next)
	{
		if(think->function.acv != T_PlatRaise)
			continue;
		plat = (plat_t *)think;
		if(plat->tag != tag)
			continue;
		if(plat->status != in_stasis)
			continue;
		plat->status = plat->oldstatus;
#ifdef SERVER
		// tell clients about this
		SV_SectorPlatform(plat);
#endif
	}
}

void EV_StopPlat(line_t* line)
{
	thinker_t *think;
	plat_t *plat;

	if(!thinkercap.next)
		return;

	for(think = thinkercap.next; think != &thinkercap; think = think->next)
	{
		if(think->function.acv != T_PlatRaise)
			continue;
		plat = (plat_t *)think;
		if(plat->tag != line->tag)
			continue;
		if(plat->status == in_stasis)
			continue;
		plat->oldstatus = plat->status;
		plat->status = in_stasis;
#ifdef SERVER
		// tell clients about this
		SV_ChangeSector(plat->sector, SV_SECTF_REMOVE_ACTION | SV_SECTF_FLOORZ);
#endif
	}
}

void P_RemoveActivePlat(plat_t* plat)
{
	plat->sector->specialdata = NULL;
	P_RemoveThinker(&plat->thinker);
}

