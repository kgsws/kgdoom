// clientside commands
// by kgsws
#include <netinet/in.h>
#include "doomdef.h"
#include "doomstat.h"
#include "d_main.h"
#include "d_net.h"
#include "d_player.h"
#include "p_local.h"
#include "g_game.h"
#include "p_mobj.h"
#include "z_zone.h"
#include "r_defs.h"
#include "p_spec.h"
#include "p_generic.h"
#include "i_system.h"
#include "hu_stuff.h"
#include "p_pickup.h"

#include "network.h"
#include "cl_cmds.h"

#include "s_sound.h"

// local player prediction
#define NUM_LOCAL_TICCMDS	128

typedef struct
{
	fixed_t x, y, z;
	fixed_t mx, my, mz;
	fixed_t fz, cz;
} player_pos_t;

int local_player_predict;
static ticcmd_t local_cmds[NUM_LOCAL_TICCMDS];
static player_pos_t local_pos;

// message
static char net_hudmsg[64];

//
// local stuff

static void CL_CheckMobjSoundSlot(mobj_t *mo, uint32_t info)
{
	int sound;

	if(netgame < 3)
		// no sounds yet
		return;

	switch(info & SV_MOBJF_SOUNDMASK)
	{
		case SV_MOBJF_SOUND_SEE:
			sound = mo->info->seesound;
		break;
		case SV_MOBJF_SOUND_ATTACK:
			sound = mo->info->attacksound;
		break;
		case SV_MOBJF_SOUND_PAIN:
			sound = mo->info->painsound;
		break;
		case SV_MOBJF_SOUND_DEATH:
			sound = mo->info->deathsound;
		break;
		case SV_MOBJF_SOUND_ACTIVE:
			sound = mo->info->activesound;
		break;
		default:
			return;
	}
	if(sound)
		S_StartSound(mo, sound, SOUND_BODY);
}

static void CL_ApplyChangeSector(sector_t *sec, sector_t *source, uint16_t info)
{
	if(info & SV_SECTF_FLOORZ)
		sec->floorheight = source->floorheight;
	if(info & SV_SECTF_CEILINGZ)
		sec->ceilingheight = source->ceilingheight;
	if(info & SV_SECTF_FLOORPIC)
		sec->floorpic = source->floorpic;
	if(info & SV_SECTF_CEILINGPIC)
		sec->ceilingpic = source->ceilingpic;
	if(info & SV_SECTF_LIGHTLEVEL)
		sec->lightlevel = source->lightlevel;
	if(info & SV_SECTF_SPECIAL)
		sec->special = source->special;
	if(info & SV_SECTF_TAG)
		sec->tag = source->tag;

	if(info & SV_SECTF_REMOVE_ACTION && sec->specialdata)
		P_RemoveThinker(sec->specialdata);
}

//
// server command parsing

void CL_CmdConnected()
{
	int i;
	char *motd;

	NET_GetByte((uint8_t*)&startmap);
	NET_GetByte((uint8_t*)&startepisode);
	NET_GetByte((uint8_t*)&startskill);
	NET_GetByte((uint8_t*)&deathmatch);
	NET_GetByte((uint8_t*)&sv_freeaim);
	NET_GetString(&motd);

	if(netgame > 1)
	{
		if(startskill != gameskill || startmap != gamemap || startepisode != gameepisode) // TODO: better way to detect map change
		{
			printf("- server requesting map change\n");
			// map change
			netgame = 1;
			// stop sounds
			S_StopSounds();
			// clear all players
			consoleplayer = MAXPLAYERS;
			displayplayer = MAXPLAYERS;
			for(i = 0; i < MAXPLAYERS+1; i++)
			{
				if(players[i].mo)
					players[i].mo->player = NULL;
				players[i].mo = NULL;
				playeringame[i] = false;
			}
			playeringame[consoleplayer] = true;
			// message timeout
			net_timeout = gametic + TICRATE;
		}
		// no MOTD
		motd = NULL;
	}

	if(netgame == 1)
	{
		if(motd)
			strncpy(network_message, motd, sizeof(network_message)-1);
		gamestate = GS_NETWORK;
		netgame = 2;
		net_timeout = 0;
		G_DeferedInitNew(startskill, startepisode, startmap);
	}
}

void CL_CmdDisconnected()
{
	char *reason;

	NET_GetString(&reason);
	// TODO: graphical
	I_Error("Disconnected from server: %s", reason);
}

void CL_CmdChangeSector()
{
	uint16_t info;
	uint16_t secnum;
	sector_t tsec;

	NET_GetUint16(&secnum);
	NET_GetUint16(&info);

	if(info & SV_SECTF_FLOORZ)
		NET_GetUint32((uint32_t*)&tsec.floorheight);
	if(info & SV_SECTF_CEILINGZ)
		NET_GetUint32((uint32_t*)&tsec.ceilingheight);
	if(info & SV_SECTF_FLOORPIC)
		NET_GetUint32((uint32_t*)&tsec.floorpic);
	if(info & SV_SECTF_CEILINGPIC)
		NET_GetUint32((uint32_t*)&tsec.ceilingpic);
	if(info & SV_SECTF_LIGHTLEVEL)
		NET_GetUint16((uint16_t*)&tsec.lightlevel);
	if(info & SV_SECTF_SPECIAL)
		NET_GetUint16((uint16_t*)&tsec.special);
	if(info & SV_SECTF_TAG)
		NET_GetUint16((uint16_t*)&tsec.tag);

	if(netgame < 3)
		return;

	if(info & SV_SECTF_USE_TAG)
	{
		int i;
		sector_t *sector = sectors;

		for(i = 0; i < numsectors; i++, sector++)
		{
			if(sector->tag != secnum)
				continue;
			CL_ApplyChangeSector(sector, &tsec, info);
		}
	} else
	{
		if(secnum > numsectors)
			return;
		CL_ApplyChangeSector(&sectors[secnum], &tsec, info);
	}
}

void CL_CmdChangeSidedef()
{
	uint16_t sidenum;
	uint8_t info;
	side_t side;
	side_t *sd;

	NET_GetUint16(&sidenum);
	NET_GetByte(&info);

	if(info & SV_SIDEF_OFFSETX)
		NET_GetUint16((uint16_t*)&side.textureoffset);
	if(info & SV_SIDEF_OFFSETY)
		NET_GetUint16((uint16_t*)&side.rowoffset);
	if(info & SV_SIDEF_TEX_TOP)
		NET_GetUint16((uint16_t*)&side.toptexture);
	if(info & SV_SIDEF_TEX_MID)
		NET_GetUint16((uint16_t*)&side.midtexture);
	if(info & SV_SIDEF_TEX_BOT)
		NET_GetUint16((uint16_t*)&side.bottomtexture);

	if(netgame < 3)
		return;

	if(sidenum >= numsides)
		return;

	sd = &sides[sidenum];

	if(info & SV_SIDEF_OFFSETX)
		sd->textureoffset = side.textureoffset * FRACUNIT;
	if(info & SV_SIDEF_OFFSETY)
		sd->rowoffset = side.rowoffset * FRACUNIT;
	if(info & SV_SIDEF_TEX_TOP)
		sd->toptexture = side.toptexture;
	if(info & SV_SIDEF_TEX_MID)
		sd->midtexture = side.midtexture;
	if(info & SV_SIDEF_TEX_BOT)
		sd->bottomtexture = side.bottomtexture;
}

void CL_CmdSpawnMobj()
{
	int netid;
	fixed_t x, y, z;
	angle_t angle;
	int state, flags;
	uint16_t type;
	uint32_t info;

	state_t *st;
	mobj_t *mo;
	mobjinfo_t *minf;

	NET_GetUint32((uint32_t*)&netid);
	NET_GetUint32((uint32_t*)&x);
	NET_GetUint32((uint32_t*)&y);
	NET_GetUint32((uint32_t*)&z);
	NET_GetUint32((uint32_t*)&angle);
	NET_GetUint32((uint32_t*)&state);
	NET_GetUint16(&type);
	NET_GetUint32(&info);

	if(netgame < 3)
	{
		// not yet
		if(info & SV_MOBJF_RADIUS)
			NET_GetUint32((uint32_t*)&x);
		if(info & SV_MOBJF_HEIGHT)
			NET_GetUint32((uint32_t*)&x);
		if(info & SV_MOBJF_FLAGS)
			NET_GetUint32((uint32_t*)&x);
		if(info & SV_MOBJF_HEALTH)
			NET_GetUint32((uint32_t*)&x);
		if(info & SV_MOBJF_FLOORZ)
			NET_GetUint32((uint32_t*)&x);
		if(info & SV_MOBJF_CEILZ)
			NET_GetUint32((uint32_t*)&x);
		if(info & SV_MOBJF_MOMX)
			NET_GetUint32((uint32_t*)&x);
		if(info & SV_MOBJF_MOMY)
			NET_GetUint32((uint32_t*)&x);
		if(info & SV_MOBJF_MOMZ)
			NET_GetUint32((uint32_t*)&x);
		if(info & SV_MOBJF_PITCH)
			NET_GetUint32((uint32_t*)&x);
		if(info & SV_MOBJF_TARGET)
			NET_GetUint32((uint32_t*)&x);
		if(info & SV_MOBJF_TRACER)
			NET_GetUint32((uint32_t*)&x);
		if(info & SV_MOBJF_MDIR)
			NET_GetByte((uint8_t*)&x);
		return;
	}

	mo = P_MobjByNetId(netid);
	if(mo)
		// this ID already exist; remove
		P_RemoveMobj(mo);

	// create new MOBJ
	mo = Z_Malloc(sizeof(mobj_t), PU_LEVEL, NULL);
	memset(mo, 0, sizeof(mobj_t));
	// add thinker
	mo->thinker.function.acp1 = (actionf_p1)P_MobjThinker;
	P_AddThinker(&mo->thinker);

	mo->x = x;
	mo->y = y;
	mo->z = z;
	mo->netid = netid;
	mo->angle = angle;

	// set type
	minf = &mobjinfo[type];
	mo->type = type;
	mo->info = minf;
	mo->radius = minf->radius;
	mo->height = minf->height;
	mo->flags = minf->flags;
	mo->health = minf->spawnhealth;

	// apply aditional changes
	if(info & SV_MOBJF_RADIUS)
		NET_GetUint32((uint32_t*)&mo->radius);
	if(info & SV_MOBJF_HEIGHT)
		NET_GetUint32((uint32_t*)&mo->height);
	if(info & SV_MOBJF_FLAGS)
		NET_GetUint32((uint32_t*)&mo->flags);
	if(info & SV_MOBJF_HEALTH)
		NET_GetUint32((uint32_t*)&mo->health);
	if(info & SV_MOBJF_FLOORZ)
		NET_GetUint32((uint32_t*)&mo->floorz);
	if(info & SV_MOBJF_CEILZ)
		NET_GetUint32((uint32_t*)&mo->ceilingz);
	if(info & SV_MOBJF_MOMX)
		NET_GetUint32((uint32_t*)&mo->momx);
	if(info & SV_MOBJF_MOMY)
		NET_GetUint32((uint32_t*)&mo->momy);
	if(info & SV_MOBJF_MOMZ)
		NET_GetUint32((uint32_t*)&mo->momz);
	if(info & SV_MOBJF_PITCH)
		NET_GetUint32((uint32_t*)&mo->pitch);

	if(info & SV_MOBJF_TARGET)
	{
		NET_GetUint32((uint32_t*)&netid);
		mo->target = P_MobjByNetId(netid);
	}

	if(info & SV_MOBJF_TRACER)
	{
		NET_GetUint32((uint32_t*)&netid);
		mo->tracer = P_MobjByNetId(netid);
	}

	if(info & SV_MOBJF_MDIR)
	{
		uint8_t md;
		NET_GetByte(&md);
		mo->movedir = md;
	}

	// set state
	st = &states[state];
	mo->state = st;
	mo->tics = st->tics;
	mo->sprite = st->sprite;
	mo->frame = st->frame;

	// set position now
	P_SetThingPosition(mo);

	// update heights
	if(!(info & SV_MOBJF_FLOORZ))
		mo->floorz = mo->subsector->sector->floorheight;		
	if(!(info & SV_MOBJF_CEILZ))
		mo->ceilingz = mo->subsector->sector->ceilingheight;

	// check sound
	CL_CheckMobjSoundSlot(mo, info);
}

void CL_CmdSpawnPlayer()
{
	uint8_t plnum, info, weapon;
	int netid;
	angle_t angle = 0;
	uint8_t rt = 0;
	player_t *pl;
	mobj_t *mo;

	NET_GetByte(&plnum);
	NET_GetByte(&info);
	NET_GetByte(&weapon);
	NET_GetUint32((uint32_t*)&netid);

	if(info & SV_PLAYL_TELEP)
	{
		NET_GetUint32((uint32_t*)&angle);
		NET_GetByte(&rt);
	}

	if(netgame < 3)
		return;

	if(plnum > MAXPLAYERS)
		return;

	pl = &players[plnum];

	pl->playerstate = info & SV_PLAYL_MASK;

	if(netid < 0)
	{
		// remove player
		playeringame[plnum] = false;
		// unlink mobj
		if(pl->mo)
		{
			pl->mo->player = NULL;
			// old body is not lagging
			pl->mo->flags &= ~MF_HOLEY;
		}
		pl->mo = NULL;
		// check local
		if(plnum == consoleplayer)
		{
			consoleplayer = MAXPLAYERS;
			displayplayer = MAXPLAYERS;
		}
		// check specatator
		if(plnum == displayplayer)
			displayplayer = consoleplayer;
	} else
	{
		// add / respawn player
		mo = P_MobjByNetId(netid);

		if(!mo)
			I_Error("CL_CmdSpawnPlayer: No body for player!");

		// unlink old mobj
		if(pl->mo)
			pl->mo->player = NULL;
		// link new one
		pl->mo = mo;
		mo->player = pl;

		// player is here
		playeringame[plnum] = true;

		// set weapon
		pl->readyweapon = weapon;

		// check local
		if(info & SV_PLAYL_LOCAL)
		{
			// assign local player
			consoleplayer = plnum;
			displayplayer = plnum;
			pl->viewheight = VIEWHEIGHT;
			pl->cheats = 0;
			pl->damagecount = 0;
			pl->bonuscount = 0;
			// enable pickup text
			HU_Start();
			// fix angle
			if(info & SV_PLAYL_TELEP)
			{
				pl->mo->angle = angle;
				if(rt == 255)
					pl->mo->reactiontime = -1;
				else
					pl->mo->reactiontime = rt;
			}
			// clear local tics
			memset(local_cmds, 0, sizeof(local_cmds));
			// server sends inventory after this
		} else
		{
			int i;
			// always predict weapon fire
			pl->cheats = CF_INFAMMO;
			for(i = 0; i < NUMWEAPONS; i++)
				pl->weaponowned[i] = true;
			pl->health = 1;
		}
		P_SetupPsprites(pl);
	}
}

void CL_CmdCeiling()
{
	uint16_t secnum;
	uint16_t info;
	generic_info_t gen;
	sector_t *ss;

	NET_GetUint16(&secnum); // TODO: SV_MOVEF_USE_TAG?
	NET_GetUint16(&info);

	NET_GetUint32((uint32_t*)&gen.startz);
	NET_GetUint32((uint32_t*)&gen.stopz);
	NET_GetUint32((uint32_t*)&gen.speed);
	NET_GetUint32((uint32_t*)&gen.crushspeed);

	ss = &sectors[secnum];

	gen.sector = ss;
	gen.startpic = ss->ceilingpic;
	gen.stoppic = ss->ceilingpic;
	gen.startsound = 0;
	gen.stopsound = 0;
	gen.movesound = 0;

	if(info & SV_MOVEF_STARTPIC)
		NET_GetUint32((uint32_t*)&gen.startpic);
	if(info & SV_MOVEF_STOPPIC)
		NET_GetUint32((uint32_t*)&gen.stoppic);
	else
	if(info & SV_MOVEF_STARTPIC)
		gen.stoppic = gen.startpic;
	if(info & SV_MOVEF_STARTSOUND)
		NET_GetUint16((uint16_t*)&gen.startsound);
	if(info & SV_MOVEF_STOPSOUND)
		NET_GetUint16((uint16_t*)&gen.stopsound);
	if(info & SV_MOVEF_MOVESOUND)
		NET_GetUint16((uint16_t*)&gen.movesound);

	if(netgame < 3)
		return;

	P_GenericSectorCeiling(ss, &gen);
}

void CL_CmdFloor()
{
	uint16_t secnum;
	uint16_t info;
	generic_info_t gen;
	sector_t *ss;

	NET_GetUint16(&secnum); // TODO: SV_MOVEF_USE_TAG?
	NET_GetUint16(&info);

	NET_GetUint32((uint32_t*)&gen.startz);
	NET_GetUint32((uint32_t*)&gen.stopz);
	NET_GetUint32((uint32_t*)&gen.speed);
	NET_GetUint32((uint32_t*)&gen.crushspeed);

	ss = &sectors[secnum];

	gen.sector = ss;
	gen.startpic = ss->floorpic;
	gen.stoppic = ss->floorpic;
	gen.startsound = 0;
	gen.stopsound = 0;
	gen.movesound = 0;

	if(info & SV_MOVEF_STARTPIC)
		NET_GetUint32((uint32_t*)&gen.startpic);
	if(info & SV_MOVEF_STOPPIC)
		NET_GetUint32((uint32_t*)&gen.stoppic);
	else
	if(info & SV_MOVEF_STARTPIC)
		gen.stoppic = gen.startpic;
	if(info & SV_MOVEF_STARTSOUND)
		NET_GetUint16((uint16_t*)&gen.startsound);
	if(info & SV_MOVEF_STOPSOUND)
		NET_GetUint16((uint16_t*)&gen.stopsound);
	if(info & SV_MOVEF_MOVESOUND)
		NET_GetUint16((uint16_t*)&gen.movesound);

	if(netgame < 3)
		return;

	P_GenericSectorFloor(ss, &gen);
}

void CL_CmdPlayerInfo()
{
	int16_t health, armor;
	uint8_t pst, dmg;
	angle_t angle = 0;
	uint8_t rt = 0;
	player_t *pl = &players[consoleplayer];

	NET_GetUint32((uint32_t*)&local_player_predict);
	NET_GetUint32((uint32_t*)&local_pos.x);
	NET_GetUint32((uint32_t*)&local_pos.y);
	NET_GetUint32((uint32_t*)&local_pos.z);
	NET_GetUint32((uint32_t*)&local_pos.mx);
	NET_GetUint32((uint32_t*)&local_pos.my);
	NET_GetUint32((uint32_t*)&local_pos.mz);
	NET_GetUint32((uint32_t*)&local_pos.fz);
	NET_GetUint32((uint32_t*)&local_pos.cz);
	NET_GetUint16((uint16_t*)&health);
	NET_GetUint16((uint16_t*)&armor);
	NET_GetByte(&dmg);
	NET_GetByte(&pst);

	if(pst & SV_PLAYL_TELEP)
	{
		NET_GetUint32((uint32_t*)&angle);
		NET_GetByte(&rt);
	}

	if(netgame < 3)
		return;

	pl->damagecount += dmg;
	if(pl->damagecount > 100)
		pl->damagecount = 100;

	pl->armorpoints = armor & 0xFFF;
	pl->armortype = armor >> 12;

	pl->playerstate = pst;
	if(health < 0)
		pl->health = 0;
	else
		pl->health = health;
	if(pl->mo)
	{
		if(pst & SV_PLAYL_TELEP)
		{
			pl->mo->angle = angle;
			pl->mo->reactiontime = rt;
		}
		pl->mo->health = health;
	}
}

void CL_CmdUpdateMobj()
{
	int netid;
	uint32_t info;
	player_extra_t plex = {0};
	mobj_t garbage;
	mobj_t *mo;
	uint32_t state;
	boolean isUnset = false;

	NET_GetUint32((uint32_t*)&netid);
	NET_GetUint32(&info);

	if(netgame < 3)
		mo = NULL;
	else
		mo = P_MobjByNetId(netid);
	if(!mo)
	{
		// read as garbage
		mo = &garbage;
	} else
	{
		// TODO: fix this logic, it's broken
/*		if((mo->flags & (MF_NOBLOCKMAP | MF_NOSECTOR)) == (MF_NOBLOCKMAP | MF_NOSECTOR))
			isUnset = true;
		else
		if(info & (SV_MOBJF_X | SV_MOBJF_Y))
*/		{
			P_UnsetThingPosition(mo);
			isUnset = true;
		}
	}

	if(info & SV_MOBJF_X)
		NET_GetUint32((uint32_t*)&mo->x);
	if(info & SV_MOBJF_Y)
		NET_GetUint32((uint32_t*)&mo->y);
	if(info & SV_MOBJF_Z)
		NET_GetUint32((uint32_t*)&mo->z);
	if(info & SV_MOBJF_ANGLE)
		NET_GetUint32((uint32_t*)&mo->angle);
	if(info & SV_MOBJF_STATE)
		NET_GetUint32((uint32_t*)&state);
	if(info & SV_MOBJF_RADIUS)
		NET_GetUint32((uint32_t*)&mo->radius);
	if(info & SV_MOBJF_HEIGHT)
		NET_GetUint32((uint32_t*)&mo->height);
	if(info & SV_MOBJF_FLAGS)
		NET_GetUint32((uint32_t*)&mo->flags);
	if(info & SV_MOBJF_HEALTH)
		NET_GetUint32((uint32_t*)&mo->health);
	if(info & SV_MOBJF_FLOORZ)
		NET_GetUint32((uint32_t*)&mo->floorz);
	if(info & SV_MOBJF_CEILZ)
		NET_GetUint32((uint32_t*)&mo->ceilingz);
	if(info & SV_MOBJF_MOMX)
		NET_GetUint32((uint32_t*)&mo->momx);
	if(info & SV_MOBJF_MOMY)
		NET_GetUint32((uint32_t*)&mo->momy);
	if(info & SV_MOBJF_MOMZ)
		NET_GetUint32((uint32_t*)&mo->momz);
	if(info & SV_MOBJF_PITCH)
		NET_GetUint32((uint32_t*)&mo->pitch);

	if(info & SV_MOBJF_TARGET)
	{
		NET_GetUint32((uint32_t*)&netid);
		mo->target = P_MobjByNetId(netid);
	}

	if(info & SV_MOBJF_TRACER)
	{
		NET_GetUint32((uint32_t*)&netid);
		mo->tracer = P_MobjByNetId(netid);
	}

	if(info & SV_MOBJF_MDIR)
	{
		uint8_t md;
		NET_GetByte(&md);
		mo->movedir = md;
	}

	if(info & SV_MOBJF_PLAYER)
	{
		NET_GetByte(&plex.info);
		NET_GetByte(&plex.weapon);
	}

	if(isUnset/* && (mo->flags & (MF_NOBLOCKMAP | MF_NOSECTOR)) != (MF_NOBLOCKMAP | MF_NOSECTOR)*/)
		P_SetThingPosition(mo);

	// update heights
	if(mo != &garbage)
	{
		if(info & SV_MOBJF_STATE)
			P_SetMobjState(mo, state);
		if(mo->subsector)
		{
			if(!(info & SV_MOBJF_FLOORZ))
				mo->floorz = mo->subsector->sector->floorheight;		
			if(!(info & SV_MOBJF_CEILZ))
				mo->ceilingz = mo->subsector->sector->ceilingheight;
		}
		// check sound
		CL_CheckMobjSoundSlot(mo, info);
		// check player stuff (but not local one)
		if(mo->player)
		{
			// health
			if(info & SV_MOBJF_HEALTH)
				mo->player->health = mo->health;
			// animation, buttons
			if(info & SV_MOBJF_PLAYER)
			{
				// "buttons"
				mo->player->cmd.buttons = 0;
				if(plex.info & SV_PLAYI_LAG)
				{
					// player is lagging
					mo->momx = 0;
					mo->momy = 0;
					mo->momz = 0;
					mo->flags |= MF_HOLEY; // different rendering
				} else
				{
					mo->flags &= ~MF_HOLEY; // player is not lagging
					if(plex.info & SV_PLAYI_USE)
						mo->player->cmd.buttons |= BT_USE;
					if(plex.info & SV_PLAYI_ATTACK)
						mo->player->cmd.buttons |= BT_ATTACK;
					else
					if(mo->health > 0
						&& mo->state != &states[S_PLAY_ATK1] && mo->state != &states[S_PLAY_ATK2]
						&& mo->state != &states[S_PLAY_PAIN] && mo->state != &states[S_PLAY_PAIN2]
					) {
						// animation (alive and not attacking)
						if(plex.info & SV_PLAYI_MOVING)
						{
							if(mo->state == &states[S_PLAY])
								P_SetMobjState(mo, S_PLAY_RUN1);
						} else
						if(mo->momx > -STOPSPEED && mo->momx < STOPSPEED && mo->momy > -STOPSPEED && mo->momy < STOPSPEED)
						{
							if(mo->state != &states[S_PLAY])
								P_SetMobjState(mo, S_PLAY);
						}
					}
				}
				// weapon
				mo->player->readyweapon = plex.weapon;
			}
		}
	}
}

void CL_CmdRemoveMobj()
{
	int netid;
	mobj_t *mo;

	NET_GetUint32((uint32_t*)&netid);

	if(netgame < 3)
		return;

	mo = P_MobjByNetId(netid);

	if(mo)
		P_RemoveMobj(mo);
}

void CL_CmdPlayerPickup()
{
	int sound;
	uint16_t type;
	mobj_t fake;
	player_t *pl = &players[consoleplayer];

	NET_GetUint16(&type);

	if(netgame < 3)
		return;

	if(!pl->mo)
		return;

	fake.type = type;
	fake.info = &mobjinfo[type];

	if(!fake.info->action.acp2i)
		return;

	fake.info->action.acp2i(pl, &fake);

	if(fake.info->arg)
		pl->message = (char *)fake.info->arg;
	pl->bonuscount += BONUSADD;
	if(fake.info->activesound)
		S_StartSound(pl->mo, fake.info->activesound, SOUND_PICKUP);
}

void CL_CmdPlayerInventory()
{
	player_t *pl;
	int i;
	uint8_t plnum;
	uint16_t weapons;
	uint8_t cards;
	uint16_t powers[NUMPOWERS];
	uint16_t ammo[NUMAMMO];

	NET_GetByte(&plnum);
	NET_GetUint16(&weapons);
	NET_GetByte(&cards); // last 'card' is a backpack
	for(i = 0; i < NUMAMMO; i++)
		NET_GetUint16(&ammo[i]);
	for(i = 0; i < NUMPOWERS; i++)
		NET_GetUint16(&powers[i]);

	if(plnum > MAXPLAYERS)
		return;

	pl = &players[plnum];

	for(i = 0; i < NUMWEAPONS; i++)
		pl->weaponowned[i] = !!(weapons & (1 << i));
	for(i = 0; i < NUMCARDS; i++)
		pl->cards[i] = !!(cards & (1 << i));
	for(i = 0; i < NUMAMMO; i++)
		pl->ammo[i] = ammo[i];
	for(i = 0; i < NUMPOWERS; i++)
		pl->powers[i] = powers[i];

	pl->backpack = !!(cards & 0x80);
}

void CL_CmdPlayerMessage()
{
	player_t *pl = &players[consoleplayer];
	char *msg;
	uint16_t sound;

	NET_GetString(&msg);
	NET_GetUint16(&sound);

	if(netgame < 3)
		return;

	if(sound)
		S_StartSound(pl->mo, sound, SOUND_BODY);
	strncpy(net_hudmsg, msg, sizeof(net_hudmsg)-1);
	pl->message = net_hudmsg;
}

void CL_CmdSound()
{
	uint16_t sound;
	uint8_t chan;
	uint32_t origin;
	mobj_t *mo = NULL;

	NET_GetUint32(&origin);
	NET_GetUint16(&sound);
	NET_GetByte(&chan);

	if(netgame < 3)
		return;

	switch(origin >> 24)
	{
		case SV_SOUNDO_MOBJ:
			mo = P_MobjByNetId(origin & 0xFFFFFF);
		break;
		case SV_SOUNDO_SECTOR:
			origin &= 0xFFFFFF;
			if(origin > numsectors)
				return;
			mo = (mobj_t*)&sectors[origin].soundorg;
		break;
		case SV_SOUNDO_LINE:
			origin &= 0xFFFFFF;
			if(origin > numlines)
				return;
			mo = (mobj_t*)&lines[origin].frontsector->soundorg;
		break;
	}

	if(sound && mo)
		S_StartSound(mo, sound, chan);
}

//
// client commands

void CL_Connect()
{
	NET_SetupCommand(CLM_CONNECT);
	NET_AddUint32(NET_VERSION);
	NET_AddUint32(0); // TODO: some check
}

void CL_Disconnect()
{
	int i = 16;

	// send this packet many times
	while(i--)
	{
		NET_SetupCommand(CLM_DISCONNECT);
		NET_SendCommand();
	}
}

void CL_Loaded()
{
	NET_SetupCommand(CLM_LOADED);
}

void CL_RequestResend(uint32_t last)
{
	NET_SetupCommand(CLM_PACKET_LOSS);
	NET_AddUint32(last);
	// send right now
	NET_SendCommand();
}

void CL_KeepAlive()
{
	NET_SetupCommand(CLM_KEEP_ALIVE);
}

void CL_Join(int team)
{
	NET_SetupCommand(CLM_JOIN);
	if(team)
		NET_AddByte(0xFF);
	else
		NET_AddByte(0);
}

void CL_TicCmd(ticcmd_t *cmd)
{
	local_cmds[gametic % NUM_LOCAL_TICCMDS] = *cmd;

	NET_SetupCommand(CLM_TICK);
	NET_AddUint32((uint32_t)gametic);
	NET_AddUint32((uint32_t)cmd->angle);
	NET_AddUint32((uint32_t)cmd->pitch);
	NET_AddByte((uint8_t)cmd->forwardmove);
	NET_AddByte((uint8_t)cmd->sidemove);
	NET_AddByte((uint8_t)cmd->buttons);
	NET_AddByte((uint8_t)cmd->weapon);
}

//
// prediction

void CL_PredictPlayer()
{
	// send current ticcmd
	if(!(players[consoleplayer].cheats & CF_SPECTATOR))
		CL_TicCmd(&players[consoleplayer].cmd);

	if(!local_player_predict)
		return;

	if(local_player_predict == gametic)
	{
		local_player_predict = 0;
		return;
	}

	ticcmd_t backup;
	player_t *pl = &players[consoleplayer];
	mobj_t *mo = pl->mo;

	if(consoleplayer == MAXPLAYERS || !mo)
	{
		local_player_predict = 0;
		return;
	}

	backup = pl->cmd;
	P_UnsetThingPosition(mo);
	mo->x = local_pos.x;
	mo->y = local_pos.y;
	mo->z = local_pos.z;
	P_SetThingPosition(mo);
	mo->floorz = local_pos.fz;
	mo->ceilingz = local_pos.cz;
	mo->momx = local_pos.mx;
	mo->momy = local_pos.my;
	mo->momz = local_pos.mz;
	while(local_player_predict < gametic-1)
	{
		pl->cmd = local_cmds[local_player_predict % NUM_LOCAL_TICCMDS];
		P_PlayerThink(pl);
		P_MobjThinker(mo);
		local_player_predict++;
	}
	pl->cmd = backup;

	local_player_predict = 0;
}

