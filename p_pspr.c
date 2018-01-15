#include "doomdef.h"
#include "d_event.h"


#include "m_random.h"
#include "p_local.h"
#include "s_sound.h"

// State.
#include "doomstat.h"

#include "p_pspr.h"

#include "p_generic.h"

// [kg] weapon change
#include "st_stuff.h"

// [kg] LUA support
#include "kg_lua.h"

#ifdef SERVER
#include <netinet/in.h>
#include "network.h"
#include "sv_cmds.h"
#endif

#define LOWERSPEED	FRACUNIT*6
#define RAISESPEED	FRACUNIT*6

#define WEAPONBOTTOM	128*FRACUNIT
#define WEAPONTOP	32*FRACUNIT

static pspdef_t *anim_psp;

//
// P_SetPsprite
//
void
P_SetPsprite
( player_t*	player,
  int		position,
  statenum_t	stnum ) 
{
    pspdef_t*	psp;
    state_t*	state;

    psp = &player->psprites[position];
    anim_psp = psp;
	
    do
    {
	if(!stnum || stnum == STATE_NULL_NEXT)
	{
	    // object removed itself
	    psp->state = NULL;
	    break;	
	}

	// [kg] animation 'alias'
	if(stnum & STATE_ANIMATION)
		stnum = L_StateFromAlias(&mobjinfo[player->readyweapon], stnum);

	state = &states[stnum];
	psp->state = state;
	psp->tics = state->tics;	// could be 0
/*
	if (state->misc1)
	{
	    // coordinate set
	    psp->sx = state->misc1 << FRACBITS;
	    psp->sy = state->misc2 << FRACBITS;
	}
*/
	// [kg] call LUA function
	L_StateCall(state, player->mo);
	if(!psp->state)
	    break;

	stnum = psp->state->nextstate;
	
    } while (!psp->tics);
    // an initial state of 0 could cycle through
}



//
// P_CalcSwing
//	
fixed_t		swingx;
fixed_t		swingy;

void P_CalcSwing (player_t*	player)
{
    fixed_t	swing;
    int		angle;
	
    // OPTIMIZE: tablify this.
    // A LUT would allow for different modes,
    //  and add flexibility.

    swing = player->bob;

    angle = (FINEANGLES/70*leveltime)&FINEMASK;
    swingx = FixedMul ( swing, finesine[angle]);

    angle = (FINEANGLES/70*leveltime+FINEANGLES/2)&FINEMASK;
    swingy = -FixedMul ( swingx, finesine[angle]);
}



//
// P_BringUpWeapon
// Starts bringing the pending weapon up
// from the bottom of the screen.
// Uses player
//
void P_BringUpWeapon (player_t* player)
{
    statenum_t	newstate;

    if (player->pendingweapon == wp_nochange)
	player->pendingweapon = player->readyweapon;

    player->psprites[ps_weapon].sy = WEAPONBOTTOM;

    // [kg] raise weapon
    newstate = mobjinfo[player->pendingweapon].weaponraise;
    if(!newstate)
    {
	// [kg] or set ready
	newstate = mobjinfo[player->pendingweapon].weaponready;
	player->psprites[ps_weapon].sy = WEAPONTOP;
    }

    player->readyweapon = player->pendingweapon;
    player->pendingweapon = wp_nochange;

    P_SetPsprite (player, ps_weapon, newstate);
}

//
// P_CheckAmmo
// Returns true if there is enough ammo to shoot.
// If not, selects the next weapon to use.
//
boolean P_CheckAmmo (player_t* player)
{
/*    ammotype_t		ammo;
    int			count;

    // [kg] new cheat
    if(player->cheats & CF_INFAMMO)
	return true;

    ammo = weaponinfo[player->readyweapon].ammo;

    // Minimal amount for one shot varies.
    if (player->readyweapon == wp_bfg)
	count = BFGCELLS;
    else if (player->readyweapon == wp_supershotgun)
	count = 2;	// Double barrel.
    else
	count = 1;	// Regular.

    // Some do not need ammunition anyway.
    // Return if current ammunition sufficient.
    if (ammo == am_noammo || player->ammo[ammo] >= count)
	return true;
#ifndef SERVER
    // Out of ammo, pick a weapon to change to.
    // Preferences are set here.
    {
	weapontype_t wpn = wp_nochange;

	if (player->weaponowned[wp_plasma]
	    && player->ammo[am_cell]
	    && (gamemode != shareware) )
	{
	    wpn = wp_plasma;
	}
	else if (player->weaponowned[wp_supershotgun] 
		 && player->ammo[am_shell]>2
		 && (gamemode == commercial) )
	{
	    wpn = wp_supershotgun;
	}
	else if (player->weaponowned[wp_chaingun]
		 && player->ammo[am_clip])
	{
	    wpn = wp_chaingun;
	}
	else if (player->weaponowned[wp_shotgun]
		 && player->ammo[am_shell])
	{
	    wpn = wp_shotgun;
	}
	else if (player->ammo[am_clip])
	{
	    wpn = wp_pistol;
	}
	else if (player->weaponowned[wp_chainsaw])
	{
	    wpn = wp_chainsaw;
	}
	else if (player->weaponowned[wp_missile]
		 && player->ammo[am_misl])
	{
	    wpn = wp_missile;
	}
	else if (player->weaponowned[wp_bfg]
		 && player->ammo[am_cell]>40
		 && (gamemode != shareware) )
	{
	    wpn = wp_bfg;
	}
	else
	{
	    // If everything fails.
	    wpn = wp_fist;
	}

	ST_SetNewWeapon(wpn);
    }

    // Now set appropriate weapon overlay.
    P_SetPsprite (player,
		  ps_weapon,
		  weaponinfo[player->readyweapon].downstate);
#endif
    return false;*/
	return true;
}

//
// P_FireWeapon.
//
void P_FireWeapon(player_t* player, int offs)
{
	statenum_t	newstate;

	if(player->mo->health <= 0)
		return;

	//    P_SetMobjState (player->mo, S_PLAY_ATK1); // TODO

	newstate = mobjinfo[player->readyweapon].weaponfiremain;
	if(!newstate)
		return;

	// skip 'offs' states
	while(offs--)
	{
		if(states[newstate].nextstate == STATE_NULL_NEXT)
			newstate++;
		else
			newstate = states[newstate].nextstate;
		if(newstate & STATE_ANIMATION || !newstate)
			// nope
			return;
	}

	P_SetPsprite (player, ps_weapon, newstate);
}

//
// [kg] 
void P_WeaponRefire(player_t *player, int offset)
{
	// check for fire
	//  (if a weaponchange is pending, let it go through instead)
	if ( (player->cmd.buttons & BT_ATTACK) // TODO: alt attack check
		&& player->pendingweapon == wp_nochange
		&& player->mo->health)
	{
		player->refire++;
		P_FireWeapon(player, offset);
	} else
		player->refire = 0;
}

//
// [kg]
void P_WeaponFlash(player_t *player, int offs)
{
	statenum_t	newstate;

	if(player->mo->health <= 0)
		return;

	//    P_SetMobjState (player->mo, S_PLAY_ATK2); // TODO

	newstate = mobjinfo[player->readyweapon].weaponflashmain;
	if(!newstate)
		return;

	// skip 'offs' states
	while(offs--)
	{
		if(states[newstate].nextstate == STATE_NULL_NEXT)
			newstate++;
		else
			newstate = states[newstate].nextstate;
		if(newstate & STATE_ANIMATION || !newstate)
			// nope
			return;
	}

	P_SetPsprite (player, ps_flash, newstate);
}

//
// P_DropWeapon
// Player died, so put the weapon away.
//
void P_DropWeapon (player_t* player)
{
	P_SetPsprite(player, ps_weapon, mobjinfo[player->readyweapon].weaponlower);
	P_SetPsprite(player, ps_flash, S_NULL);
}

//
// [kg] to avoid bad frames on network games
void P_SetAttack(player_t *player)
{
	if(player->mo->health <= 0)
		return;
//	P_SetMobjState (player->mo, S_PLAY_ATK2);
}

//
// A_WeaponReady
// The player can fire the weapon
// or change to another weapon at this time.
// Follows after getting weapon up,
// or after previous attack/fire sequence.
//
void A_WeaponReady(mobj_t *mo)
{
    statenum_t	newstate;
    int		angle;
    player_t*	player;

    player = mo->player;
    if(!player)
	return;

    if(player->mo->health > 0)
    {
	// get out of attack state // TODO: handle player animation
/*	if(player->mo->state == &states[S_PLAY_ATK1] || player->mo->state == &states[S_PLAY_ATK2])
	{
	    P_SetMobjState (player->mo, S_PLAY);
	}
*/
/*	if (player->readyweapon == wp_chainsaw
	    && psp->state == &states[S_SAW])
	{
	    S_StartSound (player->mo, sfx_sawidl, SOUND_WEAPON);
	}*/
    }

    //  if player is dead, put the weapon away
    if(!player->mo->health)
    {
	P_SetPsprite(player, ps_weapon, S_NULL);
	P_SetPsprite(player, ps_flash, S_NULL);
    }
    // check for change
    if (player->pendingweapon != wp_nochange)
    {
	// change weapon
	//  (pending weapon should allready be validated)
	newstate = mobjinfo[player->readyweapon].weaponlower;
	if(!newstate)
		P_BringUpWeapon(player);
	else
		P_SetPsprite(player, ps_weapon, newstate);
	return;
    }

    // check for fire
    //  the missile launcher and bfg do not auto fire
    if (!(player->cheats & CF_SPECTATOR) && player->cmd.buttons & BT_ATTACK)
    {
/*	if ( !player->attackdown
	     || (player->readyweapon != wp_missile
		 && player->readyweapon != wp_bfg) )*/
	{
	    player->attackdown = true;
	    P_FireWeapon(player, 0);
	    return;
	}
    }
    else
	player->attackdown = false;

    // bob the weapon based on movement speed
    angle = (128*leveltime)&FINEMASK;
    anim_psp->sx = FRACUNIT + FixedMul (player->bob, finecosine[angle]);
    angle &= FINEANGLES/2-1;
    anim_psp->sy = WEAPONTOP + FixedMul (player->bob, finesine[angle]);
}



//
// A_ReFire
// The player can re-fire the weapon
// without lowering it entirely.
//
void A_WeaponRefire(mobj_t *mo)
{
	player_t *player = mo->player;

	if(!player)
		return;

	P_WeaponRefire(player, 0);
}


void
A_CheckReload
( player_t*	player,
  pspdef_t*	psp )
{
    P_CheckAmmo (player);
#if 0
    if (player->ammo[am_shell]<2)
	P_SetPsprite (player, ps_weapon, S_DSNR1);
#endif
}



//
// A_Lower
// Lowers current weapon,
//  and changes weapon at bottom.
//
void A_WeaponLower(mobj_t *mo)
{
    player_t *player = mo->player;

    if(!player)
	return;

    anim_psp->sy += LOWERSPEED;

    // Is already down.
    if (anim_psp->sy < WEAPONBOTTOM )
	return;

    // Player is dead.
    if (player->playerstate == PST_DEAD)
    {
	anim_psp->sy = WEAPONBOTTOM;

	// don't bring weapon back up
	return;		
    }
    
    // The old weapon has been lowered off the screen,
    // so change the weapon and start raising it
    if (!player->mo->health)
    {
	// Player is dead, so keep the weapon off screen.
	P_SetPsprite (player,  ps_weapon, S_NULL);
	return;	
    }
	
    player->readyweapon = player->pendingweapon; 

    P_BringUpWeapon (player);
}


//
// A_Raise
//
void
A_WeaponRaise(mobj_t *mo)
{
    statenum_t	newstate;
    player_t *player = mo->player;

    anim_psp->sy -= RAISESPEED;

    if (anim_psp->sy > WEAPONTOP )
	return;
    
    anim_psp->sy = WEAPONTOP;
    
    // The weapon has been raised all the way,
    //  so change to the ready state.
    newstate = mobjinfo[player->readyweapon].weaponready;

    P_SetPsprite (player, ps_weapon, newstate);
}



//
// A_GunFlash
//
void
A_WeaponFlash(mobj_t *mo)
{
	if(mo->player)
		P_WeaponFlash(mo->player, 0);
}


//
// P_BulletSlope
// Sets a slope so a near miss is at aproximately
// the height of the intended target
//
fixed_t		bulletslope;


void P_BulletSlope (mobj_t*	mo)
{
    angle_t	an;

    // see which target is to be aimed at
    if(sv_freeaim)
	bulletslope = mo->pitch;
    else
    {
	an = mo->angle;
	bulletslope = P_AimLineAttack (mo, an, 16*64*FRACUNIT, NULL);

	if (!linetarget)
	{
	    an += 1<<26;
	    bulletslope = P_AimLineAttack (mo, an, 16*64*FRACUNIT, NULL);
	    if (!linetarget)
	    {
		an -= 2<<26;
		bulletslope = P_AimLineAttack (mo, an, 16*64*FRACUNIT, NULL);
	    }
	}
    }
}

//
// light flash
//
void A_Light0 (player_t *player, pspdef_t *psp)
{
    player->extralight = 0;
}

void A_Light1 (player_t *player, pspdef_t *psp)
{
    player->extralight = 1;
}

void A_Light2 (player_t *player, pspdef_t *psp)
{
    player->extralight = 2;
}

//
// P_SetupPsprites
// Called at start of level for each player.
//
void P_SetupPsprites (player_t* player) 
{
    int	i;
	
    // remove all psprites
    for (i=0 ; i<NUMPSPRITES ; i++)
	player->psprites[i].state = NULL;

    // spawn the gun
    player->pendingweapon = player->readyweapon;
    P_BringUpWeapon (player);
}

//
// P_MovePsprites
// Called every tic by player thinking routine.
//
void P_MovePsprites (player_t* player) 
{
    int		i;
    pspdef_t*	psp;
    state_t*	state;

    psp = &player->psprites[0];
    for (i=0 ; i<NUMPSPRITES ; i++, psp++)
    {
	// a null state means not active
	if ( (state = psp->state) )	
	{
	    // drop tic count and possibly change state

	    // a -1 tic count never changes
	    if (psp->tics != -1)	
	    {
		psp->tics--;
		if (!psp->tics)
		    P_SetPsprite (player, i, psp->state->nextstate);
	    }				
	}
    }
    
    player->psprites[ps_flash].sx = player->psprites[ps_weapon].sx;
    player->psprites[ps_flash].sy = player->psprites[ps_weapon].sy;
}

