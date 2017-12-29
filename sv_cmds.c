// serverside commands
// by kgsws
#include <netinet/in.h>
#include "doomdef.h"
#include "doomstat.h"
#include "d_main.h"
#include "d_net.h"
#include "d_player.h"
#include "p_local.h"
#include "w_wad.h"
#include "r_defs.h"
#include "p_spec.h"
#include "m_swap.h"

#include "network.h"
#include "sv_cmds.h"

#include "s_sound.h"
#include "sounds.h"

//
// local stuff

void SV_ChangeSectorCmd(sector_t *ss, uint16_t info)
{
	NET_SetupCommand(SVM_CHANGE_SECTOR);

	if(info & SV_SECTF_USE_TAG)
		NET_AddUint16((uint16_t)ss->tag);
	else
		NET_AddUint16((uint16_t)(ss-sectors));
	NET_AddUint16(info);

	if(info & SV_SECTF_FLOORZ)
		NET_AddUint32((uint32_t)ss->floorheight);
	if(info & SV_SECTF_CEILINGZ)
		NET_AddUint32((uint32_t)ss->ceilingheight);
	if(info & SV_SECTF_FLOORPIC)
		NET_AddUint32((uint16_t)ss->floorpic);
	if(info & SV_SECTF_CEILINGPIC)
		NET_AddUint32((uint16_t)ss->ceilingpic);
	if(info & SV_SECTF_LIGHTLEVEL)
		NET_AddUint16((uint16_t)ss->lightlevel);
	if(info & SV_SECTF_SPECIAL)
		NET_AddUint16((uint16_t)ss->special);
	if(info & SV_SECTF_TAG)
		NET_AddUint16((uint16_t)ss->tag);
}

static void SV_ChangeSidedefCmd(side_t *side, uint8_t info)
{
	NET_SetupCommand(SVM_CHANGE_LSIDE);

	NET_AddUint16((uint16_t)(side - sides));
	NET_AddByte(info);

	if(info & SV_SIDEF_OFFSETX)
		NET_AddUint16((uint16_t)(side->textureoffset / FRACUNIT));
	if(info & SV_SIDEF_OFFSETY)
		NET_AddUint16((uint16_t)(side->rowoffset / FRACUNIT));
	if(info & SV_SIDEF_TEX_TOP)
		NET_AddUint16((uint16_t)side->toptexture);
	if(info & SV_SIDEF_TEX_MID)
		NET_AddUint16((uint16_t)side->midtexture);
	if(info & SV_SIDEF_TEX_BOT)
		NET_AddUint16((uint16_t)side->bottomtexture);
}

static void SV_SpawnMobjCommand(mobj_t *mo, uint32_t info)
{
	if(mo->netid < 0)
		return;

	if(info & SV_MOBJF_AUTO)
	{
		info &= ~SV_MOBJF_AUTO;
		// autodetect some changes
		if(mo->momx)
			info |= SV_MOBJF_MOMX;
		if(mo->momy)
			info |= SV_MOBJF_MOMY;
		if(mo->momz)
			info |= SV_MOBJF_MOMZ;
		if(mo->radius != mo->info->radius)
			info |= SV_MOBJF_RADIUS;
		if(mo->height != mo->info->height)
			info |= SV_MOBJF_HEIGHT;
		if(mo->flags != mo->info->flags)
			info |= SV_MOBJF_FLAGS;
		if(mo->subsector)
		{
			if(mo->floorz != mo->subsector->sector->floorheight)
				info |= SV_MOBJF_FLOORZ;
			if(mo->ceilingz != mo->subsector->sector->ceilingheight)
				info |= SV_MOBJF_FLOORZ;
		}
	}

	// TODO: health

	if(mo->floorz != mo->subsector->sector->floorheight)
		info |= SV_MOBJF_FLOORZ;
	if(mo->ceilingz != mo->subsector->sector->ceilingheight)
		info |= SV_MOBJF_CEILZ;

	NET_SetupCommand(SVM_SPAWN_MOBJ);

	NET_AddUint32(mo->netid);
	NET_AddUint32((uint32_t)mo->x);
	NET_AddUint32((uint32_t)mo->y);
	NET_AddUint32((uint32_t)mo->z);
	NET_AddUint32(mo->angle);
	NET_AddUint32((uint32_t)(mo->state - states));
	NET_AddUint16(mo->type);

	// optional fields
	NET_AddUint32(info);

	if(info & SV_MOBJF_RADIUS)
		NET_AddUint32(mo->radius);
	if(info & SV_MOBJF_HEIGHT)
		NET_AddUint32(mo->height);
	if(info & SV_MOBJF_FLAGS)
		NET_AddUint32(mo->flags);
	if(info & SV_MOBJF_HEALTH)
		NET_AddUint32((uint32_t)mo->health);
	if(info & SV_MOBJF_FLOORZ)
		NET_AddUint32((uint32_t)mo->floorz);
	if(info & SV_MOBJF_CEILZ)
		NET_AddUint32((uint32_t)mo->ceilingz);

	if(info & SV_MOBJF_MOMX)
		NET_AddUint32((uint32_t)mo->momx);
	if(info & SV_MOBJF_MOMY)
		NET_AddUint32((uint32_t)mo->momy);
	if(info & SV_MOBJF_MOMZ)
		NET_AddUint32((uint32_t)mo->momz);
	if(info & SV_MOBJF_PITCH)
		NET_AddUint32((uint32_t)mo->pitch);
	if(info & SV_MOBJF_TARGET)
	{
		if(mo->target)
			NET_AddUint32((uint32_t)mo->target->netid);
		else
			NET_AddUint32((uint32_t)-1);
	}
	if(info & SV_MOBJF_TRACER)
	{
		if(mo->tracer)
			NET_AddUint32((uint32_t)mo->tracer->netid);
		else
			NET_AddUint32((uint32_t)-1);
	}
}

void SV_UpdateMobjCommand(mobj_t *mo, uint32_t info)
{
	NET_SetupCommand(SVM_UPDATE_MOBJ);

	NET_AddUint32((uint32_t)mo->netid);
	NET_AddUint32(info);

	if(info & SV_MOBJF_X)
		NET_AddUint32((uint32_t)mo->x);
	if(info & SV_MOBJF_Y)
		NET_AddUint32((uint32_t)mo->y);
	if(info & SV_MOBJF_Z)
		NET_AddUint32((uint32_t)mo->z);
	if(info & SV_MOBJF_ANGLE)
		NET_AddUint32((uint32_t)mo->angle);
	if(info & SV_MOBJF_STATE)
		NET_AddUint32((uint32_t)(mo->state - states));
	if(info & SV_MOBJF_RADIUS)
		NET_AddUint32((uint32_t)mo->radius);
	if(info & SV_MOBJF_HEIGHT)
		NET_AddUint32((uint32_t)mo->height);
	if(info & SV_MOBJF_FLAGS)
		NET_AddUint32((uint32_t)mo->flags);
	if(info & SV_MOBJF_HEALTH)
		NET_AddUint32((uint32_t)mo->health);
	if(info & SV_MOBJF_FLOORZ)
		NET_AddUint32((uint32_t)mo->floorz);
	if(info & SV_MOBJF_CEILZ)
		NET_AddUint32((uint32_t)mo->ceilingz);
	if(info & SV_MOBJF_MOMX)
		NET_AddUint32((uint32_t)mo->momx);
	if(info & SV_MOBJF_MOMY)
		NET_AddUint32((uint32_t)mo->momy);
	if(info & SV_MOBJF_MOMZ)
		NET_AddUint32((uint32_t)mo->momz);
	if(info & SV_MOBJF_PITCH)
		NET_AddUint32((uint32_t)mo->pitch);
	if(info & SV_MOBJF_TARGET)
	{
		if(mo->target)
			NET_AddUint32((uint32_t)mo->target->netid);
		else
			NET_AddUint32((uint32_t)-1);
	}
	if(info & SV_MOBJF_TRACER)
	{
		if(mo->tracer)
			NET_AddUint32((uint32_t)mo->tracer->netid);
		else
			NET_AddUint32((uint32_t)-1);
	}
	if(info & SV_MOBJF_MDIR)
		NET_AddByte((uint8_t)mo->movedir);
}

void SV_PlayerInventoryCommand(player_t *pl)
{
	int i;
	uint16_t weapons = 0;
	uint8_t cards = 0;

	for(i = 0; i < NUMWEAPONS; i++)
		if(pl->weaponowned[i])
			weapons |= (1 << i);

	for(i = 0; i < NUMCARDS; i++)
		if(pl->cards[i])
			cards |= (1 << i);

	if(pl->backpack) // last 'card' is a backpack
		cards |= 0x80;

	NET_SetupCommand(SVM_PLAYER_INVENTORY);

	NET_AddByte((uint8_t)(pl - players));
	NET_AddUint16(weapons);
	NET_AddByte(cards);
	for(i = 0; i < NUMAMMO; i++)
		NET_AddUint16((uint16_t)pl->ammo[i]);
	for(i = 0; i < NUMPOWERS; i++)
		NET_AddUint16((uint16_t)pl->powers[i]);
}

int SV_FindPlayerSlot()
{
	int i;

	for(i = 0; i < maxplayers; i++)
		if(!playeringame[i])
			break;
	return i;
}

//
// client command parsing

void SV_CmdConnected()
{
	NET_SetupCommand(SVM_CONNECT);
	NET_AddByte(startmap);
	NET_AddByte(startepisode);
	NET_AddByte(startskill);
	NET_AddByte(deathmatch);
	NET_AddByte(sv_freeaim);
	NET_AddString(msg_motd);
	NET_SendCommand(clientnum);
}

int SV_CmdPacketLoss()
{
	uint32_t num;

	NET_GetUint32(&num);

	if(num >= client->packet_head)
	{
		// wat?
		SV_Disconnect(clientnum, "Too little lost packets?");
		return 1;
	}

	if(client->packet_head - num >= MAX_PACKET_COUNT)
	{
		// too many lost packets
		SV_Disconnect(clientnum, "Too many lost packets.");
		return 1;
	}
	printf("- resend packets from %i (%i) to client %i\n", num, client->packet_head - num, clientnum);
	// resend all packets
	client->packet_tail = num;
	return 0;
}

void SV_CmdLoaded(uint32_t check)
{
	int i;
	thinker_t *think;
	mapsector_t *ms = W_CacheLumpNum(level_lump + ML_SECTORS);
	sector_t *ss = sectors;
	mapsidedef_t *msd = W_CacheLumpNum(level_lump + ML_SIDEDEFS);
	side_t *sd = sides;

	// got trough all sectors and find changes
	for(i = 0; i < numsectors; i++, ss++, ms++)
	{
		uint16_t info = 0;

		if(ss->floorheight != SHORT(ms->floorheight)<<FRACBITS)
			info |= SV_SECTF_FLOORZ;
		if(ss->ceilingheight != SHORT(ms->ceilingheight)<<FRACBITS)
			info |= SV_SECTF_CEILINGZ;
		if(ss->floorpic != R_FlatNumForName(ms->floorpic))
			info |= SV_SECTF_FLOORPIC;
		if(ss->ceilingpic != R_FlatNumForName(ms->ceilingpic))
			info |= SV_SECTF_CEILINGPIC;
		if(ss->lightlevel != SHORT(ms->lightlevel))
			info |= SV_SECTF_LIGHTLEVEL;
		if(ss->special != SHORT(ms->special))
			info |= SV_SECTF_SPECIAL;
		if(ss->tag != SHORT(ms->tag))
			info |= SV_SECTF_TAG;

		if(info)
		{
			SV_ChangeSectorCmd(ss, info);
			NET_SendCommand(clientnum);
		}
	}

	// get trough all sidedefs and find changes
	for(i = 0; i < numsides; i++, msd++, sd++)
	{
		uint8_t info = 0;

		if(sd->textureoffset != SHORT(msd->textureoffset)<<FRACBITS)
			info |= SV_SIDEF_OFFSETX;			
		if(sd->rowoffset != SHORT(msd->rowoffset)<<FRACBITS)
			info |= SV_SIDEF_OFFSETY;
		if(sd->toptexture != R_TextureNumForName(msd->toptexture))
			info |= SV_SIDEF_TEX_TOP;
		if(sd->midtexture != R_TextureNumForName(msd->midtexture))
			info |= SV_SIDEF_TEX_MID;
		if(sd->bottomtexture != R_TextureNumForName(msd->bottomtexture))
			info |= SV_SIDEF_TEX_BOT;

		if(info)
		{
			SV_ChangeSidedefCmd(sd, info);
			NET_SendCommand(clientnum);
		}
	}

	// send all spawned mobjs
	for(think = thinkercap.next; think != &thinkercap; think = think->next)
	{
		if(think->function.acv != P_MobjThinker)
		// Not a mobj thinker
			continue;
		SV_SpawnMobjCommand((mobj_t *)think, SV_MOBJF_AUTO);
		NET_SendCommand(clientnum);
		if(((mobj_t *)think)->player)
		{
			uint8_t info;
			player_t *pl = ((mobj_t *)think)->player;

			info = pl->playerstate;
			// check for this player
			if(pl == client->player)
				info |= SV_PLAYL_LOCAL | SV_PLAYL_TELEP;
			// this is a player
			NET_SetupCommand(SVM_SPAWN_PLAYER);
			NET_AddByte(pl - players);
			NET_AddByte(info);
			NET_AddByte((uint8_t)pl->readyweapon);
			NET_AddUint32(pl->mo->netid);
			if(info & SV_PLAYL_TELEP)
			{
				NET_AddUint32(pl->mo->angle);
				NET_AddByte((uint8_t)pl->mo->reactiontime);
			}
			NET_SendCommand(clientnum);
		}
	}

	// send inventory, if playing
	if(client->player)
	{
		SV_PlayerInventoryCommand(client->player);
		NET_SendCommand(clientnum);
	}
}

void SV_CmdJoin()
{
	uint8_t team;
	int plnum;

	NET_GetByte(&team);

	if(team == 0xFF)
	{
		// TODO: spectate
		// client->clientstate = 1;
	} else
	{
		if(!client->player)
		{
			plnum = SV_FindPlayerSlot();
			if(plnum < maxplayers)
			{
				printf("- player %i entered the game\n", plnum);
				playeringame[plnum] = true;
				client->player = &players[plnum];
				client->player->playerstate = PST_REBORN;
				playerclient[plnum] = clientnum;
				client->clientstate = 2;
				client->move_tic = gametic;
				playercount++;
			}
		}
	}
}

void SV_CmdTick()
{
	int gtic;
	ticcmd_t tcmd;

	NET_GetUint32((uint32_t*)&gtic);
	NET_GetUint32((uint32_t*)&tcmd.angle);
	NET_GetUint32((uint32_t*)&tcmd.pitch);
	NET_GetByte((uint8_t*)&tcmd.forwardmove);
	NET_GetByte((uint8_t*)&tcmd.sidemove);
	NET_GetByte((uint8_t*)&tcmd.buttons);
	NET_GetByte((uint8_t*)&tcmd.weapon);

	if(!client->player || !client->player->mo)
		return;

	client->gametic = gtic;
	client->player->cmd = tcmd;

	disable_player_think = 0;
	P_PlayerThink(client->player);
	if(client->player->mo)
		P_MobjThinker(client->player->mo);
}

//
// server commands

void SV_Disconnect(int cl, const char *msg)
{
	// TODO
	printf("- disconnect client %i, reason: %s\n", cl, msg);
}

void SV_KeepAlive(int cl)
{
	NET_SetupUnreliable(SVM_KEEP_ALIVE);
	NET_SendCommand(cl);
}

void SV_ChangeSector(sector_t *ss, uint16_t info)
{
	SV_ChangeSectorCmd(ss, info);
	NET_SendCommandAll(-1);
}

void SV_ChangeSidedef(side_t *side, uint8_t info)
{
	SV_ChangeSidedefCmd(side, info);
	NET_SendCommandAll(-1);
}

void SV_SpawnMobj(mobj_t *mobj, uint32_t info)
{
	SV_SpawnMobjCommand(mobj, info);
	NET_SendCommandAll(-1);
}

void SV_SpawnPlayer(int plnum)
{
	player_t *pl = &players[plnum];
	uint8_t info = pl->playerstate;
	int cl = playerclient[plnum];

	if(!pl->mo)
	{
		// despawn player
		NET_SetupCommand(SVM_SPAWN_PLAYER);
		NET_AddByte(plnum);
		NET_AddByte(0);
		NET_AddByte(0);
		NET_AddUint32((uint32_t)-1);
		NET_SendCommandAll(-1);
		return;
	}

	// new mobj
	SV_SpawnMobjCommand(pl->mo, 0);
	NET_SendCommandAll(-1);

	// new player (this client only)
	NET_SetupCommand(SVM_SPAWN_PLAYER);
	NET_AddByte(plnum);
	NET_AddByte(info | SV_PLAYL_LOCAL | SV_PLAYL_TELEP);
	NET_AddByte((uint8_t)pl->readyweapon);
	NET_AddUint32(pl->mo->netid);
	NET_AddUint32(pl->mo->angle);
	NET_AddByte((uint8_t)pl->mo->reactiontime);
	NET_SendCommand(cl);

	// new player (everyone but this client)
	NET_SetupCommand(SVM_SPAWN_PLAYER);
	NET_AddByte(plnum);
	NET_AddByte(info);
	NET_AddByte((uint8_t)pl->readyweapon);
	NET_AddUint32(pl->mo->netid);
	NET_SendCommandAll(cl);

	// also send inventory
	SV_PlayerInventory(pl);
}

void SV_SectorDoor(vldoor_t *door)
{
	uint16_t info = SV_MOVEF_STARTSOUND;

	if(!door->direction)
		// this happens sometimes
		return;

	NET_SetupCommand(SVM_CEILING);

	NET_AddUint16((uint16_t)(door->sector - sectors));
	NET_AddUint16(info);

	NET_AddUint32((uint32_t)door->sector->ceilingheight);
	if(door->direction < 0)
		NET_AddUint32((uint32_t)door->sector->floorheight);
	else
		NET_AddUint32((uint32_t)door->topheight);
	NET_AddUint32((uint32_t)door->speed);
	NET_AddUint32(0);

	if(door->direction < 0)
	{
		if(door-> speed > VDOORSPEED_THRSH)
			NET_AddUint16(sfx_bdcls);
		else
			NET_AddUint16(sfx_dorcls);
	} else
	{
		if(door-> speed > VDOORSPEED_THRSH)
			NET_AddUint16(sfx_bdopn);
		else
			NET_AddUint16(sfx_doropn);
	}

	NET_SendCommandAll(-1);
}

void SV_SectorCeiling(ceiling_t *ceiling)
{
	uint16_t info = SV_MOVEF_MOVESOUND;
	fixed_t crush = 0;

	if(!ceiling->direction)
		return;

	switch(ceiling->type)
	{
		case silentCrushAndRaise:
			info &= ~SV_MOVEF_MOVESOUND;
			info |= SV_MOVEF_STOPSOUND;
		break;
	}

	if(ceiling->crush)
		crush = CEILSPEED / 2;

	NET_SetupCommand(SVM_CEILING);

	NET_AddUint16((uint16_t)(ceiling->sector - sectors));
	NET_AddUint16(info);

	if(ceiling->direction > 0)
	{
		NET_AddUint32((uint32_t)ceiling->sector->ceilingheight);
		NET_AddUint32((uint32_t)ceiling->topheight);
	} else
	{
		NET_AddUint32((uint32_t)ceiling->sector->ceilingheight);
		NET_AddUint32((uint32_t)ceiling->bottomheight);
	}

	NET_AddUint32((uint32_t)ceiling->speed);
	NET_AddUint32(crush);

	if(info & SV_MOVEF_STOPSOUND)
		NET_AddUint16((uint16_t)sfx_pstop);
	if(info & SV_MOVEF_MOVESOUND)
		NET_AddUint16((uint16_t)sfx_stnmov);

	NET_SendCommandAll(-1);
}

void SV_SectorFloor(floormove_t *floor)
{
	uint16_t info = SV_MOVEF_MOVESOUND;
	fixed_t crush = 0;

	switch(floor->type)
	{
		case raiseFloorCrush:
			crush = floor->speed;
		break;
		case raiseFloor24AndChange:
			info |= SV_MOVEF_STARTPIC;
		break;
		case donutRaise:
		case lowerAndChange:
			info |= SV_MOVEF_STOPPIC;
		break;
	}

	NET_SetupCommand(SVM_FLOOR);

	NET_AddUint16((uint16_t)(floor->sector - sectors));
	NET_AddUint16(info);

	NET_AddUint32((uint32_t)floor->sector->floorheight);
	NET_AddUint32((uint32_t)floor->floordestheight);
	NET_AddUint32((uint32_t)floor->speed);
	NET_AddUint32(crush);

	if(info & SV_MOVEF_STARTPIC)
		NET_AddUint32(floor->sector->floorpic);
	if(info & SV_MOVEF_STOPPIC)
		NET_AddUint32(floor->texture);
	// SV_MOVEF_MOVESOUND is always present
	NET_AddUint16((uint16_t)sfx_stnmov);

	NET_SendCommandAll(-1);
}

void SV_SectorPlatform(plat_t *plat)
{
	uint16_t info = SV_MOVEF_STARTSOUND | SV_MOVEF_STOPSOUND;
	fixed_t crush = 0;

	if(plat->status != down && plat->status != up)
		return;

	if(plat->crush)
		crush = plat->speed;

	switch(plat->type)
	{
		case raiseAndChange:
			info = SV_MOVEF_STARTPIC | SV_MOVEF_MOVESOUND;
		break;
		case raiseToNearestAndChange:
			info = SV_MOVEF_STARTPIC | SV_MOVEF_MOVESOUND;
		break;
	}

	NET_SetupCommand(SVM_FLOOR);

	NET_AddUint16((uint16_t)(plat->sector - sectors));
	NET_AddUint16(info);

	NET_AddUint32((uint32_t)plat->sector->floorheight);
	if(plat->status == up)
		NET_AddUint32((uint32_t)plat->high);
	else
		NET_AddUint32((uint32_t)plat->low);
	NET_AddUint32((uint32_t)plat->speed);
	NET_AddUint32(crush);

	if(info & SV_MOVEF_STARTPIC)
		NET_AddUint32(plat->sector->floorpic);
//	if(info & SV_MOVEF_STOPPIC)
//		NET_AddUint32(plat->floorpic);
	if(info & SV_MOVEF_STARTSOUND)
		NET_AddUint16((uint16_t)sfx_pstart);
	if(info & SV_MOVEF_STOPSOUND)
		NET_AddUint16((uint16_t)sfx_pstop);
	if(info & SV_MOVEF_MOVESOUND)
		NET_AddUint16((uint16_t)sfx_stnmov);

	NET_SendCommandAll(-1);
}

void SV_UpdateLocalPlayer(int cl)
{
	player_t *pl = clientinfo[cl].player;
	int plnum = pl - players;
	mobj_t *mo = pl->mo;
	uint16_t armor;
	uint8_t pst = pl->playerstate;

	if(!mo)
		return;

	if(playerupdate[plnum])
		// there are more changes, must be reliable
		NET_SetupCommand(SVM_PLAYER_INFO);
	else
		// can be unreliable
		NET_SetupUnreliable(SVM_PLAYER_INFO);

	// teleport check
	if(mo->reactiontime)
		pst |= SV_PLAYL_TELEP;

	armor = (pl->armorpoints & 0xFFF) | (pl->armortype << 12);

	NET_AddUint32((uint32_t)clientinfo[cl].gametic);
	NET_AddUint32((uint32_t)mo->x);
	NET_AddUint32((uint32_t)mo->y);
	NET_AddUint32((uint32_t)mo->z);
	NET_AddUint32((uint32_t)mo->momx);
	NET_AddUint32((uint32_t)mo->momy);
	NET_AddUint32((uint32_t)mo->momz);
	NET_AddUint32((uint32_t)mo->floorz);
	NET_AddUint32((uint32_t)mo->ceilingz);
	NET_AddUint16((int16_t)mo->health);
	NET_AddUint16((int16_t)armor);
	NET_AddByte((uint8_t)pl->damagecount);
	NET_AddByte(pst);
	if(pst & SV_PLAYL_TELEP)
	{
		NET_AddUint32((uint32_t)mo->angle);
		NET_AddByte(pl->mo->reactiontime);
	}

	pl->damagecount = 0;

	// send player update
	NET_SendCommand(cl);

	// mobj specific updates
	if(playerupdate[plnum])
	{
		// maybe send MOBJ info
		uint32_t info = playerupdate[plnum] & ~(SV_MOBJF_POSITION | SV_MOBJF_FLOORZ | SV_MOBJF_CEILZ | SV_MOBJF_MOMENTNUM | SV_MOBJF_HEALTH);
		if(info)
		{
			SV_UpdateMobjCommand(mo, info);
			NET_SendCommand(playerclient[plnum]);
		}
		playerupdate[plnum] = 0;
	}
}

void SV_UpdateOtherPlayers(int cl)
{
	int i;
	uint8_t pinfo;
	mobj_t *mo;

	for(i = 0; i < maxplayers; i++)
	{
		mo = players[i].mo;
		if(playeringame[i] && mo && clientinfo[cl].player != &players[i])
		{
			NET_SetupUnreliable(SVM_UPDATE_MOBJ);

			NET_AddUint32((uint32_t)mo->netid);
			NET_AddUint32(SV_MOBJF_POSITION | SV_MOBJF_ANGLE | SV_MOBJF_FLOORZ | SV_MOBJF_CEILZ | SV_MOBJF_MOMENTNUM | SV_MOBJF_PLAYER);

			NET_AddUint32((uint32_t)mo->x);
			NET_AddUint32((uint32_t)mo->y);
			NET_AddUint32((uint32_t)mo->z);

			NET_AddUint32((uint32_t)mo->angle);

			NET_AddUint32((uint32_t)mo->floorz);
			NET_AddUint32((uint32_t)mo->ceilingz);

			NET_AddUint32((uint32_t)mo->momx);
			NET_AddUint32((uint32_t)mo->momy);
			NET_AddUint32((uint32_t)mo->momz);

			pinfo = 0;
			if(mo->player->cmd.buttons & BT_USE)
				pinfo |= SV_PLAYI_USE;
			if(mo->player->cmd.buttons & BT_ATTACK)
				pinfo |= SV_PLAYI_ATTACK;
			if(mo->player->cmd.forwardmove || mo->player->cmd.sidemove)
				pinfo |= SV_PLAYI_MOVING;
			if(clientinfo[playerclient[i]].move_tic + SERVER_PLAYER_LAGGING < gametic)
				pinfo |= SV_PLAYI_LAG;

			NET_AddByte(pinfo);
			NET_AddByte((uint8_t)mo->player->readyweapon);

			NET_SendCommand(cl);
		}
	}
}

void SV_UpdateMobj(mobj_t *mo, uint32_t info)
{
	player_t *pl;
	int plnum;

	if(!info || mo->netid < 0)
		return;

	pl = mo->player;

	if(info & SV_MOBJF_AUTO)
	{
		info &= ~SV_MOBJF_AUTO;
		// autodetect some changes
		if(mo->momx || mo->momy || mo->momz)
			info |= SV_MOBJF_X | SV_MOBJF_Y | SV_MOBJF_Z | SV_MOBJF_MOMX | SV_MOBJF_MOMY | SV_MOBJF_MOMZ;
		if(mo->radius != mo->info->radius)
			info |= SV_MOBJF_RADIUS;
		if(mo->height != mo->info->height)
			info |= SV_MOBJF_HEIGHT;
		if(mo->flags != mo->info->flags)
			info |= SV_MOBJF_FLAGS;
		if(mo->subsector)
		{
			if(mo->floorz != mo->subsector->sector->floorheight)
				info |= SV_MOBJF_FLOORZ;
			if(mo->ceilingz != mo->subsector->sector->ceilingheight)
				info |= SV_MOBJF_FLOORZ;
		}
	}

	if(!info)
		return;

	if(pl)
	{
		plnum = pl - players;

		// do not update these for target player
		if(info & (SV_MOBJF_POSITION | SV_MOBJF_FLOORZ | SV_MOBJF_CEILZ | SV_MOBJF_MOMENTNUM | SV_MOBJF_HEALTH))
		{
			playerupdate[plnum] = info;
			// tell everyone else
			SV_UpdateMobjCommand(mo, info);
			NET_SendCommandAll(playerclient[plnum]);
			return;
		}
	}

	SV_UpdateMobjCommand(mo, info);
	NET_SendCommandAll(-1);
}

void SV_RemoveMobj(mobj_t *mo)
{
	if(mo->netid < 0)
		return;
	NET_SetupCommand(SVM_REMOVE_MOBJ);
	NET_AddUint32((uint32_t)mo->netid);
	NET_SendCommandAll(-1);
}

void SV_PlayerPickup(player_t *pl, mobj_t *mo)
{
	int cl = playerclient[pl - players];
	NET_SetupCommand(SVM_PLAYER_PICKUP);
	NET_AddUint16((uint16_t)mo->type);
	NET_SendCommand(cl);
}

void SV_PlayerInventory(player_t *pl)
{
	SV_PlayerInventoryCommand(pl);
	NET_SendCommand(playerclient[pl - players]);
}

void SV_PlayerMessage(int pl, const char *msg, int sound)
{
	NET_SetupCommand(SVM_PLAYER_MESSAGE);
	NET_AddString(msg);
	NET_AddUint16((uint16_t)sound);
	NET_SendCommand(playerclient[pl]);
}

void SV_StartLineSound(line_t *line, int sound)
{
	NET_SetupCommand(SVM_SOUND);
	NET_AddUint32((uint32_t)((SV_SOUNDO_LINE << 24) | (uint32_t)(line - lines)));
	NET_AddUint16((uint16_t)sound);
	NET_AddByte(SOUND_BODY);
	NET_SendCommandAll(-1);
}

