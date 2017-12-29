// new pickup handling
// by kgsws
#include "doomdef.h"
#include "doomstat.h"
#include "p_local.h"
#include "p_inter.h"
#include "p_pickup.h"

#include "s_sound.h"
#include "sounds.h"

#ifdef SERVER
#include <netinet/in.h>
#include "network.h"
#include "sv_cmds.h"
#endif

#include "dstrings.h"

// pickup types; returned by pickup function
#define SPECIAL_DONTPICKUP	-1
#define SPECIAL_ITEM	0
#define SPECIAL_WEAPON	1
#define SPECIAL_KEY	2
#define SPECIAL_POWER	3
#define SPECIAL_SUPERPOWER	4

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
	int sound, type;
	player_t *pl;

	if(toucher->z > special->z + special->info->height)
		// out of reach
		return;

	if(toucher->z + toucher->info->height < special->z)
		// out of reach
		return;

	if(!special->info->action.acp2i)
		return;

	sound = special->info->activesound;
	pl = toucher->player;

	if(!pl || pl->health <= 0)
		// only alive players can do this
		return;

	type = special->info->action.acp2i(pl, special);
	if(type == SPECIAL_DONTPICKUP)
		return;
	if(special->flags & MF_COUNTITEM)
		pl->itemcount++;
	P_RemoveSpecial(special, type);
#ifdef SERVER
	// tell client about it
	SV_PlayerPickup(pl, special);
	SV_PlayerInventory(pl);
#else
	if(special->info->arg)
		pl->message = (char *)special->info->arg;
	pl->bonuscount += BONUSADD;
	if(pl == &players[consoleplayer] && sound)
		S_StartSound(toucher, sound, SOUND_PICKUP);
#endif		
}

//
// item pickup functions

int A_PickGreenArmor(player_t *pl, mobj_t *mo)
{
#ifndef SERVER
	if(!netgame)
#endif
	{
		if(!P_GiveArmor(pl, 1))
			return SPECIAL_DONTPICKUP;
	}
	return SPECIAL_ITEM;
}

int A_PickBlueArmor(player_t *pl, mobj_t *mo)
{
#ifndef SERVER
	if(!netgame)
#endif
	{
		if(!P_GiveArmor(pl, 2))
			return SPECIAL_DONTPICKUP;
	}
	return SPECIAL_ITEM;
}

int A_PickHealthBonus(player_t *pl, mobj_t *mo)
{
#ifndef SERVER
	if(!netgame)
#endif
	{
		pl->health++;
		if(pl->health > 200)
			pl->health = 200;
		pl->mo->health = pl->health;
	}
	return SPECIAL_ITEM;
}

int A_PickArmorBonus(player_t *pl, mobj_t *mo)
{
#ifndef SERVER
	if(!netgame)
#endif
	{
		pl->armorpoints++;
		if(pl->armorpoints > 200)
			pl->armorpoints = 200;
		if(!pl->armortype)
			pl->armortype = 1;
	}
	return SPECIAL_ITEM;
}

int A_PickKey(player_t *pl, mobj_t *mo)
{
#ifndef SERVER
	if(!netgame)
#endif
	{
		int key = mo->info->damage;
		if(pl->cards[key])
			return SPECIAL_DONTPICKUP;
		pl->cards[key] = true;
	}
	return SPECIAL_KEY;
}

int A_PickHealth10(player_t *pl, mobj_t *mo)
{
#ifndef SERVER
	if(!netgame)
#endif
	{
		if(pl->health >= MAXHEALTH)
			return SPECIAL_DONTPICKUP;
		pl->health += 10;
		if(pl->health > MAXHEALTH)
			pl->health = MAXHEALTH;
		pl->mo->health = pl->health;
	}
	return SPECIAL_ITEM;
}

int A_PickHealth25(player_t *pl, mobj_t *mo)
{
	int oldhp = pl->health;
#ifndef SERVER
	if(!netgame)
#endif
	{
		if(pl->health >= MAXHEALTH)
			return SPECIAL_DONTPICKUP;
		pl->health += 25;
		if(pl->health > MAXHEALTH)
			pl->health = MAXHEALTH;
		pl->mo->health = pl->health;
	}
	if(oldhp < 25)
		pl->message = GOTMEDINEED;
	else
		pl->message = GOTMEDIKIT;
	return SPECIAL_ITEM;
}

int A_PickSoulSphere(player_t *pl, mobj_t *mo)
{
#ifndef SERVER
	if(!netgame)
#endif
	{
		pl->health += 100;
		if(pl->health > 200)
			pl->health = 200;
		pl->mo->health = pl->health;
	}
	return SPECIAL_POWER;
}

int A_PickPower(player_t *pl, mobj_t *mo)
{
#ifndef SERVER
	if(!netgame)
#endif
	{
		if(!P_GivePower(pl, mo->info->damage))
			return SPECIAL_DONTPICKUP;
	}
	if(mo->info->speed)
		return SPECIAL_SUPERPOWER;
	else
		return SPECIAL_POWER;
}

int A_PickMegaSphere(player_t *pl, mobj_t *mo)
{
#ifndef SERVER
	if(!netgame)
#endif
	{
		pl->health = 200;
		pl->mo->health = pl->health;
		P_GiveArmor(pl,2);
	}
	return SPECIAL_POWER;
}

int A_PickAmmo(player_t *pl, mobj_t *mo)
{
#ifndef SERVER
	if(!netgame)
#endif
	{
		int count = mo->info->speed;
		if(mo->flags & MF_DROPPED)
			count /= 2;
		if(!P_GiveAmmo(pl, mo->info->damage, count))
			return SPECIAL_DONTPICKUP;
	}
	return SPECIAL_ITEM;
}

int A_PickBackpack(player_t *pl, mobj_t *mo)
{
#ifndef SERVER
	if(!netgame)
#endif
	{
		int i;
		if(!pl->backpack)
		{
			for(i = 0; i < NUMAMMO; i++)
				pl->maxammo[i] *= 2;
			pl->backpack = true;
		}
		for(i = 0; i < NUMAMMO; i++)
			P_GiveAmmo(pl, i, 0);
	}
	return SPECIAL_ITEM;
}

int A_PickWeapon(player_t *pl, mobj_t *mo)
{
#ifndef SERVER
	if(!netgame)
#endif
	{
		int weapon = mo->info->damage;
		int ammo = weaponinfo[weapon].ammo;
		boolean nope = true;

		if(!(mo->flags & MF_DROPPED) && sv_weaponrespawn < 0 && pl->weaponowned[weapon])
			// weird deathmatch type; pickup only once
			return SPECIAL_DONTPICKUP;

		if(!pl->weaponowned[weapon])
			nope = false;

		if(ammo < NUMAMMO)
		{
			int count = mo->info->speed;

			if(mo->flags & MF_DROPPED)
				count /= 2;
			else
			if(sv_weaponrespawn < 0)
				count = (count / 2) * 5;

			if(P_GiveAmmo(pl, ammo, count))
				nope = false;
		}

#ifndef SERVER
		if(!pl->weaponowned[weapon])
			pl->pendingweapon = weapon;
#endif
		pl->weaponowned[weapon] = true;

		if(nope)
			return SPECIAL_DONTPICKUP;
	}
	return SPECIAL_WEAPON;
}


