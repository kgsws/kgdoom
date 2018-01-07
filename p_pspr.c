#include "doomdef.h"
#include "d_event.h"


#include "m_random.h"
#include "p_local.h"
#include "s_sound.h"

// State.
#include "doomstat.h"

#include "p_pspr.h"

// [kg] weapon change
#include "st_stuff.h"

// [kg] LUA support
#include "kg_lua.h"

#ifdef SERVER
#include <netinet/in.h>
#include "network.h"
#include "sv_cmds.h"
#endif

#define LOWERSPEED		FRACUNIT*6
#define RAISESPEED		FRACUNIT*6

#define WEAPONBOTTOM	128*FRACUNIT
#define WEAPONTOP		32*FRACUNIT


// plasma cells for a bfg attack
#define BFGCELLS		40		

extern void *player_action;

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
	
    do
    {
	if (!stnum)
	{
	    // object removed itself
	    psp->state = NULL;
	    break;	
	}
	
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
		
//    if (player->pendingweapon == wp_chainsaw)
//	S_StartSound (player->mo, sfx_sawup, SOUND_WEAPON);
		
    newstate = weaponinfo[player->pendingweapon].upstate;

    player->pendingweapon = wp_nochange;
    player->psprites[ps_weapon].sy = WEAPONBOTTOM;

    P_SetPsprite (player, ps_weapon, newstate);
}

//
// P_CheckAmmo
// Returns true if there is enough ammo to shoot.
// If not, selects the next weapon to use.
//
boolean P_CheckAmmo (player_t* player)
{
    ammotype_t		ammo;
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
    return false;	
}

//
// P_FireWeapon.
//
void P_FireWeapon (player_t* player)
{
    statenum_t	newstate;

    if(player->health <= 0)
	return;

    if (!P_CheckAmmo (player))
	return;

//    P_SetMobjState (player->mo, S_PLAY_ATK1);
    newstate = weaponinfo[player->readyweapon].atkstate;
    P_SetPsprite (player, ps_weapon, newstate);
    P_NoiseAlert (player->mo, player->mo);
}

//
// P_DropWeapon
// Player died, so put the weapon away.
//
void P_DropWeapon (player_t* player)
{
    P_SetPsprite (player,
		  ps_weapon,
		  weaponinfo[player->readyweapon].downstate);
}

//
// [kg] to avoid bad frames on network games
void P_SetAttack(player_t *player)
{
	if(player->health <= 0)
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
void
A_WeaponReady
( player_t*	player,
  pspdef_t*	psp )
{	
    statenum_t	newstate;
    int		angle;

    if(player->health > 0)
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
    
    // check for change
    //  if player is dead, put the weapon away
    if (player->pendingweapon != wp_nochange || !player->health)
    {
	// change weapon
	//  (pending weapon should allready be validated)
	newstate = weaponinfo[player->readyweapon].downstate;
	P_SetPsprite (player, ps_weapon, newstate);
	return;	
    }

    // check for fire
    //  the missile launcher and bfg do not auto fire
    if (!(player->cheats & CF_SPECTATOR) && player->cmd.buttons & BT_ATTACK)
    {
	if ( !player->attackdown
	     || (player->readyweapon != wp_missile
		 && player->readyweapon != wp_bfg) )
	{
	    player->attackdown = true;
	    P_FireWeapon (player);		
	    return;
	}
    }
    else
	player->attackdown = false;
    
    // bob the weapon based on movement speed
    angle = (128*leveltime)&FINEMASK;
    psp->sx = FRACUNIT + FixedMul (player->bob, finecosine[angle]);
    angle &= FINEANGLES/2-1;
    psp->sy = WEAPONTOP + FixedMul (player->bob, finesine[angle]);
}



//
// A_ReFire
// The player can re-fire the weapon
// without lowering it entirely.
//
void A_ReFire
( player_t*	player,
  pspdef_t*	psp )
{
    
    // check for fire
    //  (if a weaponchange is pending, let it go through instead)
    if ( (player->cmd.buttons & BT_ATTACK) 
	 && player->pendingweapon == wp_nochange
	 && player->health)
    {
	player->refire++;
	P_FireWeapon (player);
    }
    else
    {
	player->refire = 0;
	P_CheckAmmo (player);
    }
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
void
A_Lower
( player_t*	player,
  pspdef_t*	psp )
{	
    psp->sy += LOWERSPEED;

    // Is already down.
    if (psp->sy < WEAPONBOTTOM )
	return;

    // Player is dead.
    if (player->playerstate == PST_DEAD)
    {
	psp->sy = WEAPONBOTTOM;

	// don't bring weapon back up
	return;		
    }
    
    // The old weapon has been lowered off the screen,
    // so change the weapon and start raising it
    if (!player->health)
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
A_Raise
( player_t*	player,
  pspdef_t*	psp )
{
    statenum_t	newstate;

    psp->sy -= RAISESPEED;

    if (psp->sy > WEAPONTOP )
	return;
    
    psp->sy = WEAPONTOP;
    
    // The weapon has been raised all the way,
    //  so change to the ready state.
    newstate = weaponinfo[player->readyweapon].readystate;

    P_SetPsprite (player, ps_weapon, newstate);
}



//
// A_GunFlash
//
void
A_GunFlash
( player_t*	player,
  pspdef_t*	psp ) 
{
    if(player->health <= 0)
	return;
//    P_SetMobjState (player->mo, S_PLAY_ATK2);
    P_SetPsprite (player,ps_flash,weaponinfo[player->readyweapon].flashstate);
}



//
// WEAPON ATTACKS
//


//
// A_Punch
//
void
A_Punch
( player_t*	player,
  pspdef_t*	psp ) 
{
/*    angle_t	angle;
    int		damage;
    int		slope;
	
    damage = (P_Random ()%10+1)<<1;

    if (player->powers[pw_strength])	
	damage *= 10;

    angle = player->mo->angle;
    angle += (P_Random()-P_Random())<<18;

    if(sv_freeaim)
	slope = player->mo->pitch;
    else
	slope = P_AimLineAttack (player->mo, angle, MELEERANGE);
    P_LineAttack (player->mo, angle, MELEERANGE, slope, damage);

    // turn to face target
    if (linetarget)
    {
	S_StartSound (player->mo, sfx_punch, SOUND_WEAPON);
#ifndef SERVER
	player->mo->angle = R_PointToAngle2 (player->mo->x,
					     player->mo->y,
					     linetarget->x,
					     linetarget->y);
#endif
    }*/
}


//
// A_Saw
//
void
A_Saw
( player_t*	player,
  pspdef_t*	psp ) 
{
/*    angle_t	angle;
    int		damage;
    int		slope;

    damage = 2*(P_Random ()%10+1);
    angle = player->mo->angle;
    angle += (P_Random()-P_Random())<<18;
    
    // use meleerange + 1 se the puff doesn't skip the flash
    slope = P_AimLineAttack (player->mo, angle, MELEERANGE+1);
    P_LineAttack (player->mo, angle, MELEERANGE+1, slope, damage);

    if (!linetarget)
    {
	S_StartSound (player->mo, sfx_sawful, SOUND_WEAPON);
	return;
    }
    S_StartSound (player->mo, sfx_sawhit, SOUND_WEAPON);
	
    // turn to face target
    angle = R_PointToAngle2 (player->mo->x, player->mo->y,
			     linetarget->x, linetarget->y);
    if (angle - player->mo->angle > ANG180)
    {
	if (angle - player->mo->angle < -ANG90/20)
	    player->mo->angle = angle + ANG90/21;
	else
	    player->mo->angle -= ANG90/20;
    }
    else
    {
	if (angle - player->mo->angle > ANG90/20)
	    player->mo->angle = angle - ANG90/21;
	else
	    player->mo->angle += ANG90/20;
    }
//    player->mo->flags |= MF_JUSTATTACKED;*/
}



//
// A_FireMissile
//
void
A_FireMissile
( player_t*	player,
  pspdef_t*	psp ) 
{
	if(!(player->cheats & CF_INFAMMO))
		player->ammo[weaponinfo[player->readyweapon].ammo]--;
#ifndef SERVER
	if(netgame)
		return;
#endif
//	P_SpawnPlayerMissile (player->mo, MT_ROCKET);
}


//
// A_FireBFG
//
void
A_FireBFG
( player_t*	player,
  pspdef_t*	psp ) 
{
	if(!(player->cheats & CF_INFAMMO))
		player->ammo[weaponinfo[player->readyweapon].ammo] -= BFGCELLS;
#ifndef SERVER
	if(netgame)
		return;
#endif
//	P_SpawnPlayerMissile (player->mo, MT_BFG);
}



//
// A_FirePlasma
//
void
A_FirePlasma
( player_t*	player,
  pspdef_t*	psp ) 
{
	if(!(player->cheats & CF_INFAMMO))
		player->ammo[weaponinfo[player->readyweapon].ammo]--;
	P_SetPsprite (player, ps_flash, weaponinfo[player->readyweapon].flashstate+(P_Random ()&1) );
#ifndef SERVER
	if(netgame)
		return;
#endif
//	P_SpawnPlayerMissile (player->mo, MT_PLASMA);
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

