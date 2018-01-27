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

//
// GET STUFF
//

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
	if(player->mo->health < 100)
		player->mo->health = 100;
	player->powers[power] = 1;
#ifndef SERVER
//	if (player->readyweapon != wp_fist)
//	    player->pendingweapon = wp_fist;
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
  int 		damage,
  int		damagetype )
{
    unsigned	ang;
    player_t*	player;
    fixed_t	thrust;
    int		temp;
    boolean	isneg = false;

    // [kg] negative damage is still damage
    // however, it pulls into source
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

    // [kg] damage resistence
    if(damagetype < NUMDAMAGETYPES && damage != INSTANTKILL && damage != INSTANTGIB)
    {
	if(target->damagescale[damagetype] == 255)
	    damage = INSTANTGIB;
	else
	    damage = (damage * (target->damagescale[damagetype] - (DEFAULT_DAMAGE_SCALE-10))) / 10;
    }

    player = target->player;

    // [kg] scaled into negative values
    if(damage < 0)
    {
	// healing
	int max = target->health - target->info->spawnhealth;
	damage = damage < max ? max : damage;
	if(damage >= 0)
	    // can't heal anymore
	    return;
	target->health -= damage;
	if(player)
	{
	    player->healcount -= damage;
	    if(player->healcount > 20)
		// do not mess up screen like damage can
		player->healcount = 20;
	}
	return;
    }

    if(!damage)
	// no damage
	return;

    if ( target->flags & MF_SKULLFLY )
    {
	target->momx = target->momy = target->momz = 0;
    }

    if (player && gameskill == sk_baby && damage < 1000)
	damage >>= 1; 	// take half damage in trainer mode

    if(!damage)
	damage = 1;

    // push / pull
    if (inflictor
	&& !(target->flags & MF_NOCLIP)
	&& damage != INSTANTKILL && damage != INSTANTGIB
	&& target->info->mass)
    {
	ang = R_PointToAngle2 ( inflictor->x,
				inflictor->y,
				target->x,
				target->y);

	thrust = damage*(FRACUNIT>>3)*100/target->info->mass;

	// make fall forwards sometimes
	if ( !(target->flags & MF_NODEATHPULL)
	     && damage < 40
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
    else
    if(damage != INSTANTGIB)
    {
	// [kg] armor for every mobj
	if(target->armortype && target->armortype->damage)
	{
	    int	saved;

	    saved = (damage * target->armortype->damage) / 100;
	    if (target->armorpoints <= saved)
	    {
		// armor is used up
		saved = target->armorpoints;
		target->armortype = NULL;
	    }
	    target->armorpoints -= saved;
	    damage -= saved;
	}
    }

    // player specific
    if (player)
    {
	// Below certain threshold,
	// ignore damage in GOD mode, or with INVUL power.
	if ( damage < 100000
	     && ( (player->cheats&CF_GODMODE)
		  || player->powers[pw_invulnerability] ) )
	{
	    return;
	}
	
	player->damagecount += damage;	// add damage after armor / invuln

	if (player->damagecount > 100)
	    player->damagecount = 100;	// teleport stomp does 10k points...
	
	temp = damage < 100 ? damage : 100;

	if (player == &players[displayplayer])
	    I_Tactile (40,10,40+temp*2);
    }

    // [kg] new cheat
    if(player && (player->cheats & CF_AURAMASK) == CF_REVENGEAURA && source && !source->player)
	P_DamageMobj(source, player->mo, player->mo, INSTANTKILL, NUMDAMAGETYPES);

    // [kg] set attacker, before entering pain or death state
    target->attacker = source;
    target->damagercv = damagetype;

    // do the damage	
    target->health -= damage;	
    if (target->health <= 0)
    {
	// [kg] new cheat
	if(player && player->cheats & CF_INFHEALTH)
	{
		target->health = 1;
	} else
	{
		P_KillMobj (source, target);
#ifdef SERVER
		SV_UpdateMobj(target, SV_MOBJF_AUTO | SV_MOBJF_HEALTH | SV_MOBJF_STATE | SV_MOBJF_TARGET);
#endif
		return;
	}
    }

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
    if ( target->flags & MF_ISMONSTER && (!target->threshold) && source && source != target)
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

