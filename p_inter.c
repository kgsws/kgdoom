// Data.
#include "doomdef.h"
#include "dstrings.h"

#include "doomstat.h"

#include "m_random.h"
#include "i_system.h"

#include "am_map.h"

#include "p_local.h"

#include "s_sound.h"

#ifdef __GNUG__
#pragma implementation "p_inter.h"
#endif
#include "p_inter.h"

#ifdef SERVER
#include <netinet/in.h>
#include "network.h"
#include "sv_cmds.h"
#endif

#define BONUSADD	6

// a weapon is found with two clip loads,
// a big item has five clip loads
int	maxammo[NUMAMMO] = {200, 50, 300, 50}; // [kg] TODO: check if used, remove
int	clipammo[NUMAMMO] = {10, 4, 20, 1}; // [kg] used only for backpack now


//
// GET STUFF
//

//
// P_GiveAmmo
// Num is the number of clip loads,
// not the individual count (0= 1/2 clip).
// Returns false if the ammo can't be picked up at all
//

boolean
P_GiveAmmo
( player_t*	player,
  ammotype_t	ammo,
  int		num )
{
#ifndef SERVER
    int		oldammo;
#endif

    if (ammo > NUMAMMO)
	I_Error ("P_GiveAmmo: bad type %i", ammo);
		
    if ( player->ammo[ammo] == player->maxammo[ammo]  )
	return false;

    if(!num)
	num = clipammo[ammo];

    if (gameskill == sk_baby
	|| gameskill == sk_nightmare)
    {
	// give double ammo in trainer mode,
	// you'll need in nightmare
	num <<= 1;
    }

#ifndef SERVER
    oldammo = player->ammo[ammo];
#endif
    player->ammo[ammo] += num;

    if (player->ammo[ammo] > player->maxammo[ammo])
	player->ammo[ammo] = player->maxammo[ammo];

#ifndef SERVER
    // If non zero ammo, 
    // don't change up weapons,
    // player was lower on purpose.
    if (oldammo)
	return true;

    // We were down to zero,
    // so select a new weapon.
    // Preferences are not user selectable.
    switch (ammo)
    {
      case am_clip:
	if (player->readyweapon == wp_fist)
	{
	    if (player->weaponowned[wp_chaingun])
		player->pendingweapon = wp_chaingun;
	    else
		player->pendingweapon = wp_pistol;
	}
	break;
	
      case am_shell:
	if (player->readyweapon == wp_fist
	    || player->readyweapon == wp_pistol)
	{
	    if (player->weaponowned[wp_shotgun])
		player->pendingweapon = wp_shotgun;
	}
	break;
	
      case am_cell:
	if (player->readyweapon == wp_fist
	    || player->readyweapon == wp_pistol)
	{
	    if (player->weaponowned[wp_plasma])
		player->pendingweapon = wp_plasma;
	}
	break;
	
      case am_misl:
	if (player->readyweapon == wp_fist)
	{
	    if (player->weaponowned[wp_missile])
		player->pendingweapon = wp_missile;
	}
      default:
	break;
    }
#endif
    return true;
}

//
// P_GiveArmor
// Returns false if the armor is worse
// than the current armor.
//
boolean
P_GiveArmor
( player_t*	player,
  int		armortype )
{
    int		hits;
	
    hits = armortype*100;
    if (player->armorpoints >= hits)
	return false;	// don't pick up
		
    player->armortype = armortype;
    player->armorpoints = hits;
	
    return true;
}

//
// P_GivePower
//
boolean
P_GivePower
( player_t*	player,
  int /*powertype_t*/	power )
{
    if (power == pw_invulnerability)
    {
	player->powers[power] = INVULNTICS;
	return true;
    }
    
    if (power == pw_invisibility)
    {
	player->powers[power] = INVISTICS;
	player->mo->flags |= MF_SHADOW;
	return true;
    }
    
    if (power == pw_infrared)
    {
	player->powers[power] = INFRATICS;
	return true;
    }
    
    if (power == pw_ironfeet)
    {
	player->powers[power] = IRONTICS;
	return true;
    }
    
    if (power == pw_strength)
    {
	if(player->health < 100)
	{
		player->health = 100;
		player->mo->health = 100;
	}
	player->powers[power] = 1;
#ifndef SERVER
	if (player->readyweapon != wp_fist)
	    player->pendingweapon = wp_fist;
#endif
	return true;
    }
	
    if (player->powers[power])
	return false;	// already got it
		
    player->powers[power] = 1;
    return true;
}

//
// KillMobj
//
void
P_KillMobj
( mobj_t*	source,
  mobj_t*	target )
{
    mobjtype_t	item;
    mobj_t*	mo;
	
    target->flags &= ~(MF_SHOOTABLE|MF_FLOAT|MF_SKULLFLY);

//    if (target->type != MT_SKULL) // TODO: handle somehow
	target->flags &= ~MF_NOGRAVITY;

    target->flags |= MF_CORPSE|MF_DROPOFF;
    target->height >>= 2;

    if (source && source->player)
    {
	// count for intermission
	if (target->flags & MF_COUNTKILL)
	    source->player->killcount++;	

	if (target->player)
	    source->player->frags[target->player-players]++;
    }
    else if (!netgame && (target->flags & MF_COUNTKILL) )
    {
	// count all monster deaths,
	// even those caused by other monsters
	players[0].killcount++;
    }
    
    if (target->player)
    {
	// count environment kills against you
	if (!source)	
	    target->player->frags[target->player-players]++;
			
	target->flags &= ~MF_SOLID;
	target->player->playerstate = PST_DEAD;
	P_DropWeapon (target->player);
#ifndef SERVER
	if (target->player == &players[consoleplayer]
	    && automapactive)
	{
	    // don't die in auto map,
	    // switch view prior to dying
	    AM_Stop ();
	}
#endif
    }

    if (target->health < -target->info->spawnhealth 
	&& target->info->xdeathstate)
    {
	P_SetMobjState (target, target->info->xdeathstate);
    }
    else
    if(target->info->deathstate)
	P_SetMobjState (target, target->info->deathstate);
    target->tics -= P_Random()&3;

    if (target->tics < 1)
	target->tics = 1;
}




//
// P_DamageMobj
// Damages both enemies and players
// "inflictor" is the thing that caused the damage
//  creature or missile, can be NULL (slime, etc)
// "source" is the thing to target after taking damage
//  creature or NULL
// Source and inflictor are the same for melee attacks.
// Source can be NULL for slime, barrel explosions
// and other environmental stuff.
//
void
P_DamageMobj
( mobj_t*	target,
  mobj_t*	inflictor,
  mobj_t*	source,
  int 		damage )
{
    unsigned	ang;
    int		saved;
    player_t*	player;
    fixed_t	thrust;
    int		temp;
    boolean	isneg = false;

    if(damage < 0)
    {
	isneg = true;
	damage = -damage;
    }

#ifdef SERVER
    // [kg] no damage in overtime, unless it is forced
    if(exit_countdown && damage < 1000)
	return;
#endif
    if ( !(target->flags & MF_SHOOTABLE) )
	return;	// shouldn't happen...
		
    if (target->health <= 0)
	return;

    if ( target->flags & MF_SKULLFLY )
    {
	target->momx = target->momy = target->momz = 0;
    }

    player = target->player;

    if (player && gameskill == sk_baby && damage < 1000)
	damage >>= 1; 	// take half damage in trainer mode
		

    // Some close combat weapons should not
    // inflict thrust and push the victim out of reach,
    // thus kick away unless using the chainsaw.
    if (inflictor
	&& !(target->flags & MF_NOCLIP)
	&& damage != INSTANTKILL
	&& target->info->mass)
    {
	ang = R_PointToAngle2 ( inflictor->x,
				inflictor->y,
				target->x,
				target->y);

	thrust = damage*(FRACUNIT>>3)*100/target->info->mass;

	// make fall forwards sometimes; TODO: what with this?
	if ( damage < 40
	     && damage > target->health
	     && target->z - inflictor->z > 64*FRACUNIT
	     && (P_Random ()&1) )
	{
	    ang += ANG180;
	    thrust *= 4;
	}

	if(isneg)
	    ang += ANG180;
		
	ang >>= ANGLETOFINESHIFT;
	target->momx += FixedMul (thrust, finecosine[ang]);
	target->momy += FixedMul (thrust, finesine[ang]);
    }

    if(damage == INSTANTKILL)
	damage = target->health + (target->info->spawnhealth - 1);

    // player specific
    if (player)
    {
	// end of game hell hack
	if (target->subsector->sector->special == 11
	    && damage >= target->health)
	{
	    damage = target->health - 1;
	}
	

	// Below certain threshold,
	// ignore damage in GOD mode, or with INVUL power.
	if ( damage < 1000
	     && ( (player->cheats&CF_GODMODE)
		  || player->powers[pw_invulnerability] ) )
	{
	    return;
	}
	
	if (player->armortype)
	{
	    if (player->armortype == 1)
		saved = damage/3;
	    else
		saved = damage/2;
	    
	    if (player->armorpoints <= saved)
	    {
		// armor is used up
		saved = player->armorpoints;
		player->armortype = 0;
	    }
	    player->armorpoints -= saved;
	    damage -= saved;
	}
	player->health = player->mo->health - damage; 	// mirror mobj health here for Dave
	if (player->health < 0)
	    player->health = 0;
	
	player->attacker = source;
	player->damagecount += damage;	// add damage after armor / invuln

	if (player->damagecount > 100)
	    player->damagecount = 100;	// teleport stomp does 10k points...
	
	temp = damage < 100 ? damage : 100;

	if (player == &players[consoleplayer])
	    I_Tactile (40,10,40+temp*2);
    }

    // [kg] new cheat
    if(player && (player->cheats & CF_AURAMASK) == CF_REVENGEAURA && source && !source->player)
	P_DamageMobj(source, player->mo, player->mo, INSTANTKILL);

    // do the damage	
    target->health -= damage;	
    if (target->health <= 0)
    {
	// [kg] new cheat
	if(player && player->cheats & CF_INFHEALTH)
	{
		target->health = 1;
		player->health = 1;
	} else
	{
		P_KillMobj (source, target);
#ifdef SERVER
		SV_UpdateMobj(target, SV_MOBJF_AUTO | SV_MOBJF_HEALTH | SV_MOBJF_STATE | SV_MOBJF_TARGET);
#endif
		return;
	}
    }

    // [kg] set attacker, before entering pain state
    target->attacker = source;

    if ( (P_Random () < target->info->painchance)
	 && !(target->flags&MF_SKULLFLY) )
    {
	target->flags |= MF_JUSTHIT;	// fight back!
	if(target->info->painstate)
	    P_SetMobjState (target, target->info->painstate);
    }
			
    target->reactiontime = 0;		// we're awake now...	

/*    if ( (!target->threshold || target->type == MT_VILE)
	 && source && source != target
	 && source->type != MT_VILE)*/
    if ( (!target->threshold) && source && source != target)
    {
	// if not intent on another player,
	// chase after this one
	target->target = source;
	target->threshold = BASETHRESHOLD;
	if (target->state == &states[target->info->spawnstate]
	    && target->info->seestate != S_NULL)
	    P_SetMobjState (target, target->info->seestate);
    }
#ifdef SERVER
    SV_UpdateMobj(target, SV_MOBJF_AUTO | SV_MOBJF_STATE | SV_MOBJF_TARGET);
#endif
}

