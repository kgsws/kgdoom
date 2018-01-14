// new pickup handling
// by kgsws
#include "doomdef.h"
#include "doomstat.h"
#include "p_local.h"
#include "p_inter.h"
#include "p_pickup.h"

#include "s_sound.h"
#include "kg_lua.h"

#ifdef SERVER
#include <netinet/in.h>
#include "network.h"
#include "sv_cmds.h"
#endif

// [kg] decide to remove or hide item
void P_RemoveSpecial(mobj_t* th, int rt)
{
	int respawntime;

	if(th->flags & MF_DROPPED)
	{
#ifdef SERVER
		P_RemoveMobj(th, true);
#else
		P_RemoveMobj(th);
#endif
		return;
	}

	switch(rt)
	{
		case SPECIAL_ITEM:
			respawntime = sv_itemrespawn;
		break;
		case SPECIAL_AMMO:
			respawntime = sv_ammorespawn;
		break;
		case SPECIAL_WEAPON:
			respawntime = sv_weaponrespawn;
		break;
		case SPECIAL_KEY:
			respawntime = netgame ? -1 : 0;
		break;
		case SPECIAL_POWER:
			respawntime = sw_powerrespawn;
		break;
		case SPECIAL_SUPERPOWER:
			respawntime = sv_superrespawn;
		break;
		default:
			respawntime = 0;
		break;
	}

	if(!respawntime)
	{
#ifdef SERVER
		P_RemoveMobj(th, true);
#else
		P_RemoveMobj(th);
#endif
		return;
	}
	if(respawntime < 0)
		return;
	// hide item
	P_UnsetThingPosition(th);
	th->flags |= MF_NOSECTOR | MF_NOBLOCKMAP;
	P_SetMobjState(th, S_ITEMRESPAWN0);
	// set respawn time
	th->tics = respawntime;
#ifdef SERVER
	// tell clients about this
	// client th->tics is -1, which is good
	SV_UpdateMobj(th, SV_MOBJF_FLAGS | SV_MOBJF_STATE);
#endif
}

// [kg] unhide item and spawn IFOG
void A_RespawnSpecial(mobj_t* th)
{
#ifndef SERVER
	if(netgame)
		return;
#endif
	subsector_t*	ss;
	mobj_t*		mo;
	fixed_t		x = th->x;
	fixed_t		y = th->y;

	// spawn a teleport fog at the new spot
	ss = R_PointInSubsector (x,y);
	mo = P_SpawnMobj (x, y, ss->sector->floorheight , MT_IFOG);
	if(mo->info->seesound)
	    S_StartSound (mo, mo->info->seesound, SOUND_BODY);
#ifdef SERVER
	// tell clients about this
	SV_SpawnMobj(mo, SV_MOBJF_SOUND_SEE);
#endif
	// unhide item
	P_SetMobjState(th, th->info->spawnstate);
	th->flags &= ~(MF_NOSECTOR | MF_NOBLOCKMAP);
	P_SetThingPosition(th);
#ifdef SERVER
	// tell clients about this
	SV_UpdateMobj(th, SV_MOBJF_FLAGS | SV_MOBJF_STATE);
#endif
}

// [kg] new touch handler
void P_TouchSpecialThing(mobj_t *special, mobj_t *toucher)
{
	int sound, type, flash;
	player_t *pl;

	if(toucher->health <= 0)
		// you are dead
		return;

	if(toucher->z > special->z + special->info->height)
		// out of reach
		return;

	if(toucher->z + toucher->info->height < special->z)
		// out of reach
		return;

	sound = special->info->activesound;
	pl = toucher->player;

	flash = L_TouchSpecial(special, toucher);
	type = flash & 0xFF;
	if(type == SPECIAL_DONTPICKUP)
		return;
	if(special->flags & MF_COUNTITEM)
		pl->itemcount++;
	P_RemoveSpecial(special, type);

	if(pl && !(flash & SPECIAL_NOFLASH_FLAG))
		pl->bonuscount += BONUSADD;
	if(pl == &players[consoleplayer] && sound)
		S_StartSound(toucher, sound, SOUND_PICKUP);
}

