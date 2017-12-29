#include "z_zone.h"
#include "doomdef.h"
#include "p_local.h"

#include "s_sound.h"


// State.
#include "doomstat.h"
#include "r_state.h"

// Data.
#include "dstrings.h"
#include "sounds.h"

#ifdef SERVER
#include <netinet/in.h>
#include "network.h"
#include "sv_cmds.h"
#endif

#if 0
//
// Sliding door frame information
//
slidename_t	slideFrameNames[MAXSLIDEDOORS] =
{
    {"GDOORF1","GDOORF2","GDOORF3","GDOORF4",	// front
     "GDOORB1","GDOORB2","GDOORB3","GDOORB4"},	// back
	 
    {"\0","\0","\0","\0"}
};
#endif


//
// VERTICAL DOORS
//

//
// T_VerticalDoor
//
void T_VerticalDoor (vldoor_t* door)
{
    result_e	res;
	
    switch(door->direction)
    {
      case 0:
	// WAITING
	if (!--door->topcountdown)
	{
	    switch(door->type)
	    {
		case -1:
			// [kg] generic
			door->direction = -1;
			if(door->speed > VDOORSPEED_THRSH)
				S_StartSound((mobj_t *)&door->sector->soundorg, sfx_bdcls, SOUND_BODY);
			else
				S_StartSound((mobj_t *)&door->sector->soundorg, sfx_dorcls, SOUND_BODY);
		break;

	      case blazeRaise:
		door->direction = -1; // time to go back down
		S_StartSound((mobj_t *)&door->sector->soundorg,
			     sfx_bdcls, SOUND_BODY);
		break;
		
	      case normal:
		door->direction = -1; // time to go back down
		S_StartSound((mobj_t *)&door->sector->soundorg,
			     sfx_dorcls, SOUND_BODY);
		break;
		
	      case close30ThenOpen:
		door->direction = 1;
		S_StartSound((mobj_t *)&door->sector->soundorg,
			     sfx_doropn, SOUND_BODY);
		break;
		
	      default:
		break;
	    }
#ifdef SERVER
	    // [kg] tell clients about this
	    SV_SectorDoor(door);
#endif
	}
	break;
	
      case 2:
	//  INITIAL WAIT
	if (!--door->topcountdown)
	{
	    switch(door->type)
	    {
	      case raiseIn5Mins:
		door->direction = 1;
		door->type = normal;
		S_StartSound((mobj_t *)&door->sector->soundorg,
			     sfx_doropn, SOUND_BODY);
		break;
		
	      default:
		break;
	    }
#ifdef SERVER
	    // [kg] tell clients about this
	    SV_SectorDoor(door);
#endif
	}
	break;
	
      case -1:
	// DOWN
	res = T_MovePlane(door->sector,
			  door->speed,
			  door->sector->floorheight,
			  false,1,door->direction);
	if (res == pastdest)
	{
	    switch(door->type)
	    {
		case -1:
			// [kg] generic
			door->sector->specialdata = NULL;
			P_RemoveThinker (&door->thinker);  // unlink and free
		break;
	      case blazeRaise:
	      case blazeClose:
		door->sector->specialdata = NULL;
		P_RemoveThinker (&door->thinker);  // unlink and free
		break;
		
	      case normal:
	      case justClose:
		door->sector->specialdata = NULL;
		P_RemoveThinker (&door->thinker);  // unlink and free
		break;
		
	      case close30ThenOpen:
		door->direction = 0;
		door->topcountdown = 35*30;
		break;
		
	      default:
		break;
	    }
	}
	else if (res == crushed)
	{
	    switch(door->type)
	    {
		case -1:
			// [kg] generic
			if(door->topwait < 0)
				// blocked for single action
				break;
			// go back up
			door->direction = 1;
			if(door->speed > VDOORSPEED_THRSH)
				S_StartSound((mobj_t *)&door->sector->soundorg,sfx_bdopn, SOUND_BODY);
			else
				S_StartSound((mobj_t *)&door->sector->soundorg,sfx_doropn, SOUND_BODY);
		break;
	      case blazeClose:
	      case justClose:		// DO NOT GO BACK UP!
		break;
		
	      default:
		door->direction = 1;
		S_StartSound((mobj_t *)&door->sector->soundorg,
			     sfx_doropn, SOUND_BODY);
		break;
	    }
#ifdef SERVER
	    // [kg] tell clients about this
	    SV_SectorDoor(door);
#endif
	}
	break;
	
      case 1:
	// UP
	res = T_MovePlane(door->sector,
			  door->speed,
			  door->topheight,
			  false,1,door->direction);
	
	if (res == pastdest)
	{
	    switch(door->type)
	    {
		case -1: // [kg] generic
			if(door->topwait <= 0)
			{
				// done here
				door->sector->specialdata = NULL;
				P_RemoveThinker (&door->thinker);
			}
			// otherwise wait at top
	      case blazeRaise:
	      case normal:
		door->direction = 0; // wait at top
		door->topcountdown = door->topwait;
		break;
		
	      case close30ThenOpen:
	      case blazeOpen:
	      case open:
		door->sector->specialdata = NULL;
		P_RemoveThinker (&door->thinker);  // unlink and free
		break;
		
	      default:
		break;
	    }
	}
	break;
    }
}


//
// EV_DoLockedDoor
// Move a locked door up/down
//

int
EV_DoLockedDoor
( line_t*	line,
  vldoor_e	type,
  mobj_t*	thing )
{
    player_t*	p;
	
    p = thing->player;
	
    if (!p)
	return 0;
		
    switch(line->special)
    {
      case 99:	// Blue Lock
      case 133:
	if ( !p )
	    return 0;
	if (!p->cards[it_bluecard] && !p->cards[it_blueskull])
	{
	    p->message = PD_BLUEO;
	    S_StartSound(p->mo, sfx_oof, SOUND_BODY);
#ifdef SERVER
	    // tell player about this
	    SV_PlayerMessage(p - players, PD_BLUEO, sfx_oof);
#endif
	    return 0;
	}
	break;
	
      case 134: // Red Lock
      case 135:
	if ( !p )
	    return 0;
	if (!p->cards[it_redcard] && !p->cards[it_redskull])
	{
	    p->message = PD_REDO;
	    S_StartSound(p->mo, sfx_oof, SOUND_BODY);
#ifdef SERVER
	    // tell player about this
	    SV_PlayerMessage(p - players, PD_REDO, sfx_oof);
#endif
	    return 0;
	}
	break;
	
      case 136:	// Yellow Lock
      case 137:
	if ( !p )
	    return 0;
	if (!p->cards[it_yellowcard] &&
	    !p->cards[it_yellowskull])
	{
	    p->message = PD_YELLOWO;
	    S_StartSound(p->mo, sfx_oof, SOUND_BODY);
#ifdef SERVER
	    // tell player about this
	    SV_PlayerMessage(p - players, PD_YELLOWO, sfx_oof);
#endif
	    return 0;
	}
	break;	
    }

    return EV_DoDoor(line,type);
}


int
EV_DoDoor
( line_t*	line,
  vldoor_e	type )
{
    int		secnum,rtn;
    sector_t*	sec;
    vldoor_t*	door;
	
    secnum = -1;
    rtn = 0;
    
    while ((secnum = P_FindSectorFromLineTag(line,secnum)) >= 0)
    {
	sec = &sectors[secnum];
	if (sec->specialdata)
	    continue;
		
	
	// new door thinker
	rtn = 1;
	door = Z_Malloc (sizeof(*door), PU_LEVSPEC, 0);
	P_AddThinker (&door->thinker);
	sec->specialdata = door;

	door->thinker.function.acp1 = (actionf_p1) T_VerticalDoor;
	door->sector = sec;
	door->type = type;
	door->topwait = VDOORWAIT;
	door->speed = VDOORSPEED;
		
	switch(type)
	{
	  case blazeClose:
	    door->topheight = P_FindLowestCeilingSurrounding(sec);
	    door->topheight -= 4*FRACUNIT;
	    door->direction = -1;
	    door->speed = VDOORSPEED * 4;
	    S_StartSound((mobj_t *)&door->sector->soundorg,
			 sfx_bdcls, SOUND_BODY);
	    break;
	    
	  case justClose:
	    door->topheight = P_FindLowestCeilingSurrounding(sec);
	    door->topheight -= 4*FRACUNIT;
	    door->direction = -1;
	    S_StartSound((mobj_t *)&door->sector->soundorg,
			 sfx_dorcls, SOUND_BODY);
	    break;
	    
	  case close30ThenOpen:
	    door->topheight = sec->ceilingheight;
	    door->direction = -1;
	    S_StartSound((mobj_t *)&door->sector->soundorg,
			 sfx_dorcls, SOUND_BODY);
	    break;
	    
	  case blazeRaise:
	  case blazeOpen:
	    door->direction = 1;
	    door->topheight = P_FindLowestCeilingSurrounding(sec);
	    door->topheight -= 4*FRACUNIT;
	    door->speed = VDOORSPEED * 4;
	    if (door->topheight != sec->ceilingheight)
		S_StartSound((mobj_t *)&door->sector->soundorg,
			     sfx_bdopn, SOUND_BODY);
	    break;
	    
	  case normal:
	  case open:
	    door->direction = 1;
	    door->topheight = P_FindLowestCeilingSurrounding(sec);
	    door->topheight -= 4*FRACUNIT;
	    if (door->topheight != sec->ceilingheight)
		S_StartSound((mobj_t *)&door->sector->soundorg,
			     sfx_doropn, SOUND_BODY);
	    break;
	    
	  default:
	    break;
	}
#ifdef SERVER
	// [kg] tell clients about this
	// TODO: replace with 'tag' ?
	SV_SectorDoor(door);
#endif
    }
    return rtn;
}


//
// EV_VerticalDoor : open a door manually, no tag value
//
void
EV_VerticalDoor
( line_t*	line,
  mobj_t*	thing )
{
    player_t*	player;
    int		secnum;
    sector_t*	sec;
    vldoor_t*	door;
    int		side;
	
    side = 0;	// only front sides can be used

    //	Check for locks
    player = thing->player;
		
    switch(line->special)
    {
      case 26: // Blue Lock
      case 32:
	if ( !player )
	    return;
	
	if (!player->cards[it_bluecard] && !player->cards[it_blueskull])
	{
	    player->message = PD_BLUEK;
	    S_StartSound(player->mo, sfx_oof, SOUND_BODY);
#ifdef SERVER
	    // tell player about this
	    SV_PlayerMessage(player - players, PD_BLUEK, sfx_oof);
#endif
	    return;
	}
	break;
	
      case 27: // Yellow Lock
      case 34:
	if ( !player )
	    return;
	
	if (!player->cards[it_yellowcard] &&
	    !player->cards[it_yellowskull])
	{
	    player->message = PD_YELLOWK;
	    S_StartSound(player->mo, sfx_oof, SOUND_BODY);
#ifdef SERVER
	    // tell player about this
	    SV_PlayerMessage(player - players, PD_YELLOWK, sfx_oof);
#endif
	    return;
	}
	break;
	
      case 28: // Red Lock
      case 33:
	if ( !player )
	    return;
	
	if (!player->cards[it_redcard] && !player->cards[it_redskull])
	{
	    player->message = PD_REDK;
	    S_StartSound(player->mo, sfx_oof, SOUND_BODY);
#ifdef SERVER
	    // tell player about this
	    SV_PlayerMessage(player - players, PD_REDK, sfx_oof);
#endif
	    return;
	}
	break;
    }
	
    // if the sector has an active thinker, use it
    sec = sides[ line->sidenum[side^1]] .sector;
    secnum = sec-sectors;

    if (sec->specialdata)
    {
	door = sec->specialdata;
	switch(line->special)
	{
	  case	1: // ONLY FOR "RAISE" DOORS, NOT "OPEN"s
	  case	26:
	  case	27:
	  case	28:
	  case	117:
	    if (door->direction == -1)
	    {
		door->direction = 1;	// go back up
		// [kg] play sounds
		if(line->special == 117)
			S_StartSound((mobj_t *)&sec->soundorg,sfx_bdopn, SOUND_BODY);
		else
			S_StartSound((mobj_t *)&sec->soundorg,sfx_doropn, SOUND_BODY);
	    } else
	    {
		if (!thing->player)
		    return;		// JDC: bad guys never close doors
		
		door->direction = -1;	// start going down immediately
		// [kg] play sounds
		if(line->special == 117)
			S_StartSound((mobj_t *)&sec->soundorg,sfx_bdcls, SOUND_BODY);
		else
			S_StartSound((mobj_t *)&sec->soundorg,sfx_dorcls, SOUND_BODY);
	    }
#ifdef SERVER
	    // [kg] tell clients about this
	    SV_SectorDoor(door);
#endif
	    return;
	}
    }
	
    // for proper sound
    switch(line->special)
    {
      case 117:	// BLAZING DOOR RAISE
      case 118:	// BLAZING DOOR OPEN
	S_StartSound((mobj_t *)&sec->soundorg,sfx_bdopn, SOUND_BODY);
	break;
	
      case 1:	// NORMAL DOOR SOUND
      case 31:
	S_StartSound((mobj_t *)&sec->soundorg,sfx_doropn, SOUND_BODY);
	break;
	
      default:	// LOCKED DOOR SOUND
	S_StartSound((mobj_t *)&sec->soundorg,sfx_doropn, SOUND_BODY);
	break;
    }
	
    
    // new door thinker
    door = Z_Malloc (sizeof(*door), PU_LEVSPEC, 0);
    P_AddThinker (&door->thinker);
    sec->specialdata = door;
    door->thinker.function.acp1 = (actionf_p1) T_VerticalDoor;
    door->sector = sec;
    door->direction = 1;
    door->speed = VDOORSPEED;
    door->topwait = VDOORWAIT;

    switch(line->special)
    {
      case 1:
      case 26:
      case 27:
      case 28:
	door->type = normal;
	break;
	
      case 31:
      case 32:
      case 33:
      case 34:
	door->type = open;
	line->special = 0;
	break;
	
      case 117:	// blazing door raise
	door->type = blazeRaise;
	door->speed = VDOORSPEED*4;
	break;
      case 118:	// blazing door open
	door->type = blazeOpen;
	line->special = 0;
	door->speed = VDOORSPEED*4;
	break;
    }
    
    // find the top and bottom of the movement range
    door->topheight = P_FindLowestCeilingSurrounding(sec);
    door->topheight -= 4*FRACUNIT;

#ifdef SERVER
    // [kg] tell clients about this
    SV_SectorDoor(door);
#endif
}


//
// Spawn a door that closes after 30 seconds
//
void P_SpawnDoorCloseIn30 (sector_t* sec)
{
    vldoor_t*	door;
	
    door = Z_Malloc ( sizeof(*door), PU_LEVSPEC, 0);

    P_AddThinker (&door->thinker);

    sec->specialdata = door;
    sec->special = 0;

    door->thinker.function.acp1 = (actionf_p1)T_VerticalDoor;
    door->sector = sec;
    door->direction = 0;
    door->type = normal;
    door->speed = VDOORSPEED;
    door->topcountdown = 30 * 35;
}

//
// Spawn a door that opens after 5 minutes
//
void
P_SpawnDoorRaiseIn5Mins
( sector_t*	sec,
  int		secnum )
{
    vldoor_t*	door;
	
    door = Z_Malloc ( sizeof(*door), PU_LEVSPEC, 0);
    
    P_AddThinker (&door->thinker);

    sec->specialdata = door;
    sec->special = 0;

    door->thinker.function.acp1 = (actionf_p1)T_VerticalDoor;
    door->sector = sec;
    door->direction = 2;
    door->type = raiseIn5Mins;
    door->speed = VDOORSPEED;
    door->topheight = P_FindLowestCeilingSurrounding(sec);
    door->topheight -= 4*FRACUNIT;
    door->topwait = VDOORWAIT;
    door->topcountdown = 5 * 60 * 35;
}

//
// [kg] more generic doors

void EV_GenericDoor(sector_t *altsec, int tag, int lightag, int speed, int wait, int dir)
{
	vldoor_t *door;
	sector_t *sec;
	int secnum = -1;
	int count = -1; // MAX

	if(!tag)
	{
		if(!altsec)
			return;
		count = 1;
		// make it find correct sector next
		tag = altsec->tag;
		secnum = (altsec - sectors) - 1;
	}

	while((secnum = P_FindSectorFromTag(tag, secnum)) >= 0 && count--)
	{
		sec = &sectors[secnum];
		if(sec->specialdata)
		{
			door = sec->specialdata;
			if(door->thinker.function.acp1 != (actionf_p1) T_VerticalDoor)
				continue;
			if(door->topwait < 0)
				// blocked for single action
				continue;
			// change direction
			if(door->direction < 0)
			{
				door->direction = 1;
				if(door->speed > VDOORSPEED_THRSH)
					S_StartSound((mobj_t *)&sec->soundorg,sfx_bdopn, SOUND_BODY);
				else
					S_StartSound((mobj_t *)&sec->soundorg,sfx_doropn, SOUND_BODY);
			} else
			{
				door->direction = -1;
				if(door->speed > VDOORSPEED_THRSH)
					S_StartSound((mobj_t *)&door->sector->soundorg, sfx_bdcls, SOUND_BODY);
				else
					S_StartSound((mobj_t *)&door->sector->soundorg, sfx_dorcls, SOUND_BODY);
			}
			continue;
		}

		// new door thinker
		door = Z_Malloc (sizeof(*door), PU_LEVSPEC, 0);
		P_AddThinker (&door->thinker);
		sec->specialdata = door;
		door->thinker.function.acp1 = (actionf_p1) T_VerticalDoor;
		door->sector = sec;
		door->type = -1; // [kg] generic
		door->direction = dir;
		door->speed = VDOORSPEED_GENERIC * speed;
		door->topwait = wait;
		door->topheight = P_FindLowestCeilingSurrounding(sec);
		door->topheight -= 4*FRACUNIT;
		door->topcountdown = 0;
		if(dir < 0)
		{
			if(door->speed > VDOORSPEED_THRSH)
				S_StartSound((mobj_t *)&door->sector->soundorg, sfx_bdcls, SOUND_BODY);
			else
				S_StartSound((mobj_t *)&door->sector->soundorg, sfx_dorcls, SOUND_BODY);
		} else
		{
			if(door->speed > VDOORSPEED_THRSH)
				S_StartSound((mobj_t *)&sec->soundorg,sfx_bdopn, SOUND_BODY);
			else
				S_StartSound((mobj_t *)&sec->soundorg,sfx_doropn, SOUND_BODY);
		}
	}
}

