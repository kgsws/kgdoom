#include "doomdef.h"
#include "d_event.h"

#include "p_local.h"

#include "p_inventory.h"
#include "st_stuff.h"

#include "i_system.h"
#include "kg_record.h"

#include "doomstat.h"

#ifndef SERVER
#include "cl_cmds.h"
#endif

// Index of the special effects (INVUL inverse) map.
#define INVERSECOLORMAP		32


//
// Movement.
//

// 16 pixels of bob
#define MAXBOB	0x100000	

//
// P_Thrust
// Moves the given origin along a given angle.
//
void
P_Thrust
( player_t*	player,
  angle_t	angle,
  fixed_t	move ) 
{
    angle >>= ANGLETOFINESHIFT;
    
    player->mo->momx += FixedMul(move,finecosine[angle]); 
    player->mo->momy += FixedMul(move,finesine[angle]);
}




//
// P_CalcHeight
// Calculate the walking / running height adjustment
//
void P_CalcHeight (player_t* player) 
{
	int	angle;
	fixed_t	bob;

	if(player->cheats & CF_SPECTATOR || !player->mo->info->bobz)
	{
		bob = 0;
	} else
	{
		// Regular movement bobbing
		// (needs to be calculated for gun swing
		// even if not on ground)
		// OPTIMIZE: tablify angle
		// Note: a LUT allows for effects
		//  like a ramp with low health.
		player->bob = FixedMul (player->mo->momx, player->mo->momx) + FixedMul (player->mo->momy,player->mo->momy);

		player->bob >>= 2;

		if (player->bob > MAXBOB)
			player->bob = MAXBOB;

		// [kg] scale
		player->bob = FixedMul(player->bob, player->mo->info->bobz / 16);

		if ((player->cheats & CF_NOMOMENTUM) || !player->mo->onground)
		{
			player->viewz = player->mo->z + player->mo->info->viewz;

			if (player->viewz > player->mo->ceilingz-4*FRACUNIT)
			player->viewz = player->mo->ceilingz-4*FRACUNIT;

			player->viewz = player->mo->z + player->viewheight;
			return;
		}

		angle = (FINEANGLES/20*leveltime)&FINEMASK;
		bob = FixedMul ( player->bob/2, finesine[angle]);
	}

	// move viewheight
	if (player->playerstate == PST_LIVE)
	{
		player->viewheight += player->deltaviewheight;

		if (player->viewheight > player->mo->info->viewz)
		{
			player->viewheight = player->mo->info->viewz;
			player->deltaviewheight = 0;
		}

		if (player->viewheight < player->mo->info->viewz/2)
		{
			player->viewheight = player->mo->info->viewz/2;
			if (player->deltaviewheight <= 0)
				player->deltaviewheight = 1;
		}

		if (player->deltaviewheight)	
		{
			player->deltaviewheight += FRACUNIT/4;
			if (!player->deltaviewheight)
				player->deltaviewheight = 1;
		}
	}
	player->viewz = player->mo->z + player->viewheight + bob;

	if (player->viewz > player->mo->ceilingz-4*FRACUNIT)
		player->viewz = player->mo->ceilingz-4*FRACUNIT;

	if (player->viewz <= player->mo->floorz)
		player->viewz = player->mo->floorz + 1;
}



//
// P_MovePlayer
//
void P_MovePlayer (player_t* player)
{
    ticcmd_t*		cmd;
#ifndef SERVER
    player_t *localpl = &players[consoleplayer];
#endif
	
    cmd = &player->cmd;

#ifndef SERVER
    if(!netgame || player == localpl)
    {
#endif
	if(!player->mo->reactiontime)
	    player->mo->angle = cmd->angle;
    if(sv_freeaim)
    {
	player->mo->pitch = cmd->pitch;
	if(player->mo->pitch > 50000)
	    player->mo->pitch = 50000;
	if(player->mo->pitch < -50000)
	    player->mo->pitch = -50000;
    } else
	player->mo->pitch = 0;
#ifndef SERVER
    }
#endif

    // Do not let the player control movement
    //  if not onground.

    if (cmd->forwardmove && player->mo->onground)
	P_Thrust (player, player->mo->angle, cmd->forwardmove*2048);
    
    if (cmd->sidemove && player->mo->onground)
	P_Thrust (player, player->mo->angle-ANG90, cmd->sidemove*2048);

#ifndef SERVER
    if(local_player_predict)
	return;

    if(!netgame || player == localpl)
#endif
{}
/*    if ( (cmd->forwardmove || cmd->sidemove)
	 && player->mo->state == &states[S_PLAY] )
    {
	P_SetMobjState (player->mo, S_PLAY_RUN1);
    }*/ // TODO: handle player animations
}	



//
// P_DeathThink
// Fall on your face when dying.
// Decrease POV height to floor height.
//
#define ANG5   	(ANG90/18)

void P_DeathThink (player_t* player)
{
    angle_t		angle;
    angle_t		delta;

    P_MovePsprites (player);

#ifndef SERVER
    if(!local_player_predict)
    {
#endif
	
    // fall to the ground
    if (player->viewheight > 6*FRACUNIT)
	player->viewheight -= FRACUNIT;

    if (player->viewheight < 6*FRACUNIT)
	player->viewheight = 6*FRACUNIT;

    // [kg] slope
    if(player->mo->pitch > 0)
	player->mo->pitch -= 3000;

    if(player->mo->pitch < 0)
	player->mo->pitch += 3000;

    player->deltaviewheight = 0;

    P_CalcHeight (player);

#ifndef SERVER
    }
#endif
/*
    if (player->attacker && player->attacker != player->mo)
    {
	angle = R_PointToAngle2 (player->mo->x,
				 player->mo->y,
				 player->attacker->x,
				 player->attacker->y);
	
	delta = angle - player->mo->angle;
	
	if (delta < ANG5 || delta > (unsigned)-ANG5)
	{
	    // Looking at killer,
	    //  so fade damage flash down.
	    player->mo->angle = angle;

	    if (player->damagecount)
		player->damagecount--;
	}
	else if (delta < ANG180)
	    player->mo->angle += ANG5;
	else
	    player->mo->angle -= ANG5;
    }
    else*/ if (player->damagecount)
	player->damagecount--;


    if (player->cmd.buttons & BT_USE)
	player->playerstate = PST_REBORN;
}



//
// P_PlayerThink
//
void P_PlayerThink (player_t* player)
{
    ticcmd_t*		cmd;

    if(!player->mo)
	// [kg] this will happen in multiplayer
	return;

    // fixme: do this in the cheat code
    if (player->cheats & CF_NOCLIP)
	player->mo->flags |= MF_NOCLIP;
    else
	player->mo->flags &= ~MF_NOCLIP;
    
    cmd = &player->cmd;

#ifndef SERVER
    // [kg] recording / savegame
    if(rec_is_playback)
    {
	if(player->playerstate == PST_LIVE)
	    rec_get_ticcmd(&player->cmd);
	else
	    player->cmd = *I_BaseTiccmd();
    }
    if(!netgame && !rec_is_playback)
	rec_ticcmd(cmd);
#endif

    // chain saw run forward
    if (player->mo->flags & MF_JUSTATTACKED)
    {
	cmd->forwardmove = 0xc800/512;
	cmd->sidemove = 0;
	player->mo->flags &= ~MF_JUSTATTACKED;
    }
			
	
    if (player->playerstate == PST_DEAD)
    {
	if(player->mo->health > 0)
	    player->playerstate = PST_LIVE;
	else
	{
	    if(rec_is_playback)
	    {
		rec_is_playback = 0;
		player->cmd.buttons = 0;
	    }
	    P_DeathThink (player);
	    return;
	}
    }
    
    // Move around.
    // Reactiontime is used to prevent movement
    //  for a bit after a teleport.
    // [kg] allow permanent freeze with negative number
    if(player->mo->reactiontime)
    {
	if(player->mo->reactiontime > 0)
	    player->mo->reactiontime--;
    } else
	P_MovePlayer (player);

#ifdef SERVER
    P_CalcHeight (player);
#else
    if(local_player_predict)
	return;

    P_CalcHeight (player);

    if(!netgame)
#endif
    {
	sector_t *sector = player->mo->subsector->sector;
	if(sector->flags & SF_SECRET)
	{
	    sector->flags &= ~SF_SECRET;
	    sector->flags |= SF_WAS_SECRET;
	    player->secretcount++;
	}
    }
    
    // Check for weapon change.
    if (cmd->buttons & BT_CHANGE)
    {
	// The actual changing of the weapon is done
	//  when the weapon psprite can do it
	//  (read: not in the middle of an attack).
	// [kg] only allow to pick owned weapons that have icon, if allowed to change weapon
	if(P_CheckInventory(player->mo, &mobjinfo[cmd->weapon], NULL) && ST_PickableWeapon(cmd->weapon) && !player->force_weapon && !player->hide_stbar)
	{
		player->pendingweapon = cmd->weapon;
		player->lua_weapon_change = 0;
	}
    }
    
    // check for use
    if (cmd->buttons & BT_USE)
    {
	if (!player->usedown)
	{
	    if(!(player->cheats & CF_SPECTATOR))
		P_UseLines (player);
	    player->usedown = true;
	}
    }
    else
	player->usedown = false;
    
    // cycle psprites
    P_MovePsprites (player);

    if (player->damagecount)
	player->damagecount--;

    if (player->bonuscount)
	player->bonuscount--;

    if (player->healcount)
	player->healcount--;

}

