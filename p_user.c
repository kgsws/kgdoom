#include "doomdef.h"
#include "d_event.h"

#include "p_local.h"

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

boolean		onground;


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

		if ((player->cheats & CF_NOMOMENTUM) || !onground)
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
    onground = (player->mo->z <= player->mo->floorz);
	
    if (cmd->forwardmove && onground)
	P_Thrust (player, player->mo->angle, cmd->forwardmove*2048);
    
    if (cmd->sidemove && onground)
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
    onground = (player->mo->z <= player->mo->floorz);

    P_CalcHeight (player);

#ifndef SERVER
    }
#endif
	
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
    else if (player->damagecount)
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
    
    // chain saw run forward
    cmd = &player->cmd;
    if (player->mo->flags & MF_JUSTATTACKED)
    {
	cmd->forwardmove = 0xc800/512;
	cmd->sidemove = 0;
	player->mo->flags &= ~MF_JUSTATTACKED;
    }
			
	
    if (player->playerstate == PST_DEAD)
    {
	P_DeathThink (player);
	return;
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
    if (player->mo->subsector->sector->special)
	P_PlayerInSpecialSector (player);
    
    // Check for weapon change.
    if (cmd->buttons & BT_CHANGE)
    {
	// The actual changing of the weapon is done
	//  when the weapon psprite can do it
	//  (read: not in the middle of an attack).
	player->pendingweapon = cmd->weapon;
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
    
    // Counters, time dependend power ups.

    // Strength counts up to diminish fade.
    if (player->powers[pw_strength])
	player->powers[pw_strength]++;	
		
    if (player->powers[pw_invulnerability])
	player->powers[pw_invulnerability]--;

    if (player->powers[pw_invisibility])
	if (! --player->powers[pw_invisibility] )
	    player->mo->flags &= ~MF_SHADOW;
			
    if (player->powers[pw_infrared])
	player->powers[pw_infrared]--;
		
    if (player->powers[pw_ironfeet])
	player->powers[pw_ironfeet]--;
		
    if (player->damagecount)
	player->damagecount--;
		
    if (player->bonuscount)
	player->bonuscount--;

    
    // Handling colormaps.
    if (player->powers[pw_invulnerability])
    {
	if (player->powers[pw_invulnerability] > 4*32
	    || (player->powers[pw_invulnerability]&8) )
	    player->fixedcolormap = INVERSECOLORMAP;
	else
	    player->fixedcolormap = 0;
    }
    else if (player->powers[pw_infrared])	
    {
	if (player->powers[pw_infrared] > 4*32
	    || (player->powers[pw_infrared]&8) )
	{
	    // almost full bright
	    player->fixedcolormap = 1;
	}
	else
	    player->fixedcolormap = 0;
    }
    else
	player->fixedcolormap = 0;
}

