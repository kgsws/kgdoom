#include "z_zone.h"
#include "doomdef.h"
#include "p_local.h"

#include "s_sound.h"

// State.
#include "doomstat.h"
#include "r_state.h"

// Data.
#include "sounds.h"

#ifdef SERVER
#include <netinet/in.h>
#include "network.h"
#include "sv_cmds.h"
#endif

//
// CEILINGS
//

//
// T_MoveCeiling
//

void T_MoveCeiling (ceiling_t* ceiling)
{
    result_e	res;
	
    switch(ceiling->direction)
    {
      case 0:
	// IN STASIS
	break;
      case 1:
	// UP
	res = T_MovePlane(ceiling->sector,
			  ceiling->speed,
			  ceiling->topheight,
			  false,1,ceiling->direction);
	
	if (!(leveltime&7))
	{
	    switch(ceiling->type)
	    {
	      case silentCrushAndRaise:
		break;
	      default:
		S_StartSound((mobj_t *)&ceiling->sector->soundorg,
			     sfx_stnmov, SOUND_BODY);
		// ?
		break;
	    }
	}
	
	if (res == pastdest)
	{
	    switch(ceiling->type)
	    {
	      case raiseToHighest:
		P_RemoveActiveCeiling(ceiling);
		break;
		
	      case silentCrushAndRaise:
		S_StartSound((mobj_t *)&ceiling->sector->soundorg,
			     sfx_pstop, SOUND_BODY);
	      case fastCrushAndRaise:
	      case crushAndRaise:
		ceiling->direction = -1;
#ifdef SERVER
		// tell clients about this
		SV_SectorCeiling(ceiling);
#endif
		break;
		
	      default:
		break;
	    }
	    
	}
	break;
	
      case -1:
	// DOWN
	res = T_MovePlane(ceiling->sector,
			  ceiling->speed,
			  ceiling->bottomheight,
			  ceiling->crush,1,ceiling->direction);
	
	if (!(leveltime&7))
	{
	    switch(ceiling->type)
	    {
	      case silentCrushAndRaise: break;
	      default:
		S_StartSound((mobj_t *)&ceiling->sector->soundorg,
			     sfx_stnmov, SOUND_BODY);
	    }
	}
	
	if (res == pastdest)
	{
	    switch(ceiling->type)
	    {
	      case silentCrushAndRaise:
		S_StartSound((mobj_t *)&ceiling->sector->soundorg,
			     sfx_pstop, SOUND_BODY);
	      case crushAndRaise:
		ceiling->speed = CEILSPEED;
	      case fastCrushAndRaise:
		ceiling->direction = 1;
#ifdef SERVER
		// tell clients about this
		SV_SectorCeiling(ceiling);
#endif
		break;

	      case lowerAndCrush:
	      case lowerToFloor:
		P_RemoveActiveCeiling(ceiling);
		break;

	      default:
		break;
	    }
	}
	else // ( res != pastdest )
	{
	    if (res == crushed)
	    {
		switch(ceiling->type)
		{
		  case silentCrushAndRaise:
		  case crushAndRaise:
		  case lowerAndCrush:
		    ceiling->speed = CEILSPEED / 8;
		    break;

		  default:
		    break;
		}
	    }
	}
	break;
    }
}


//
// EV_DoCeiling
// Move a ceiling up/down and all around!
//
int
EV_DoCeiling
( line_t*	line,
  ceiling_e	type )
{
    int		secnum;
    int		rtn;
    sector_t*	sec;
    ceiling_t*	ceiling;
	
    secnum = -1;
    rtn = 0;
    
    //	Reactivate in-stasis ceilings...for certain types.
    switch(type)
    {
      case fastCrushAndRaise:
      case silentCrushAndRaise:
      case crushAndRaise:
	P_ActivateInStasisCeiling(line);
      default:
	break;
    }
	
    while ((secnum = P_FindSectorFromLineTag(line,secnum)) >= 0)
    {
	sec = &sectors[secnum];
	if (sec->specialdata)
	    continue;
	
	// new door thinker
	rtn = 1;
	ceiling = Z_Malloc (sizeof(*ceiling), PU_LEVSPEC, 0);
	P_AddThinker (&ceiling->thinker);
	sec->specialdata = ceiling;
	ceiling->thinker.function.acp1 = (actionf_p1)T_MoveCeiling;
	ceiling->sector = sec;
	ceiling->crush = false;
	
	switch(type)
	{
	  case fastCrushAndRaise:
	    ceiling->crush = true;
	    ceiling->topheight = sec->ceilingheight;
	    ceiling->bottomheight = sec->floorheight + (8*FRACUNIT);
	    ceiling->direction = -1;
	    ceiling->speed = CEILSPEED * 2;
	    break;

	  case silentCrushAndRaise:
	  case crushAndRaise:
	    ceiling->crush = true;
	    ceiling->topheight = sec->ceilingheight;
	  case lowerAndCrush:
	  case lowerToFloor:
	    ceiling->bottomheight = sec->floorheight;
	    if (type != lowerToFloor)
		ceiling->bottomheight += 8*FRACUNIT;
	    ceiling->direction = -1;
	    ceiling->speed = CEILSPEED;
	    break;

	  case raiseToHighest:
	    ceiling->topheight = P_FindHighestCeilingSurrounding(sec);
	    ceiling->direction = 1;
	    ceiling->speed = CEILSPEED;
	    break;
	}
		
	ceiling->tag = sec->tag;
	ceiling->type = type;
#ifdef SERVER
	// tell clients about this
	SV_SectorCeiling(ceiling);
#endif
    }
    return rtn;
}

//
// Remove a ceiling's thinker
//
void P_RemoveActiveCeiling(ceiling_t* c)
{
	c->sector->specialdata = NULL;
	P_RemoveThinker(&c->thinker);
}

//
// Restart a ceiling that's in-stasis
//
void P_ActivateInStasisCeiling(line_t* line)
{
	thinker_t *think;
	ceiling_t *c;

	if(!thinkercap.next)
		return;

	for(think = thinkercap.next; think != &thinkercap; think = think->next)
	{
		if(think->function.acv != T_MoveCeiling)
			continue;
		c = (ceiling_t *)think;
		if(c->tag != line->tag)
			continue;
		if(c->direction)
			continue;
		c->direction = c->olddirection;
#ifdef SERVER
		// tell clients about this
		SV_SectorCeiling(c);
#endif
	}
}



//
// EV_CeilingCrushStop
// Stop a ceiling from crushing!
//
int	EV_CeilingCrushStop(line_t	*line)
{
	thinker_t *think;
	ceiling_t *c;
	int ret = 0;

	if(!thinkercap.next)
		return 0;

	for(think = thinkercap.next; think != &thinkercap; think = think->next)
	{
		if(think->function.acv != T_MoveCeiling)
			continue;
		c = (ceiling_t *)think;
		if(c->tag != line->tag)
			continue;
		if(c->direction)
			continue;
		ret = 1;
		c->olddirection = c->direction;
		c->direction = 0;
#ifdef SERVER
		// tell clients about this
		SV_ChangeSector(c->sector, SV_SECTF_REMOVE_ACTION | SV_SECTF_CEILINGZ);
#endif
	}

	return ret;
}

