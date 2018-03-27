// singleplayer game recording
// also used as game saves
// by kgsws
#include "doomdef.h"
#include "doomstat.h"
#include "i_system.h"
#include "p_local.h"
#include "p_inventory.h"
#include "m_random.h"
#include "z_zone.h"
#include "st_stuff.h"
#include "kg_record.h"
#include "t_text.h"

int rec_is_playback;

static void *rec_buff;
static void *rec_ptr;
static void *rec_end;

static int rec_ticnt;
static int rec_ticdup;

static ticcmd_t rec_old_cmd;

static void rec_add_uint8(uint8_t in)
{
	*((uint8_t*)rec_ptr) = in;
	rec_ptr += sizeof(uint8_t);
}

static void rec_add_uint16(uint16_t in)
{
	*((uint16_t*)rec_ptr) = in;
	rec_ptr += sizeof(uint16_t);
}

static void rec_add_uint32(uint32_t in)
{
	*((uint32_t*)rec_ptr) = in;
	rec_ptr += sizeof(uint32_t);
}

static int rec_get_uint8(uint8_t *out)
{
	if(rec_ptr > rec_end - sizeof(uint8_t))
		return 1;
	*out = *((uint8_t*)rec_ptr);
	rec_ptr += sizeof(uint8_t);
	return 0;
}

static int rec_get_uint16(uint16_t *out)
{
	if(rec_ptr > rec_end - sizeof(uint16_t))
		return 1;
	*out = *((uint16_t*)rec_ptr);
	rec_ptr += sizeof(uint16_t);
	return 0;
}

static int rec_get_uint32(uint32_t *out)
{
	if(rec_ptr > rec_end - sizeof(uint32_t))
		return 1;
	*out = *((uint32_t*)rec_ptr);
	rec_ptr += sizeof(uint32_t);
	return 0;
}

void rec_init()
{
	rec_buff = malloc(RECORDING_SIZE);
	if(!rec_buff)
		I_Error("failed to allocate recording buffer");
	rec_ptr = rec_buff;
	rec_end = rec_buff + RECORDING_SIZE;
}

void rec_reset()
{
	player_t *p = &players[consoleplayer];

	rec_ticnt = 0;
	rec_ticdup = 0;
	rec_old_cmd = *I_BaseTiccmd();

	// reset pointer
	rec_ptr = rec_buff;
	// add header
	rec_add_uint32(RECORDING_HEAD);
	rec_add_uint32(RECORDING_VER);
	// TODO: WADs checksum
	rec_add_uint32(0xFFAA5581);
	// map lump
	rec_add_uint32(level_lump);
	// title (16 bytes); filled later
	rec_add_uint32(0x6f6d6564);
	rec_add_uint32(0);
	rec_add_uint32(0);
	rec_add_uint32(0);
	// doom map numbers
	rec_add_uint8(gamemap);
	rec_add_uint8(gameepisode);
	rec_add_uint8(gameskill);
	rec_add_uint8(0);
	// PRNG seeds
	rec_add_uint32(prndindex);
	rec_add_uint32(rand_m_z);
	rec_add_uint32(rand_m_w);
	// add player data
	rec_add_uint32(p->playerstate == PST_LIVE);
	if(p->playerstate == PST_LIVE)
	{
		int i;
		inventory_t *inv;
		// inventory size
		i = 0;
		inv = p->inventory;
		while(inv)
		{
			i++;
			inv = inv->next;
		}
		// full player state
		rec_add_uint32(p->health);
		rec_add_uint32(p->armorpoints);
		rec_add_uint16(p->armortype - mobjinfo);
		rec_add_uint16(i);
		rec_add_uint16(p->readyweapon);
		rec_add_uint16(p->pendingweapon);
		// inventory items
		inv = p->inventory;
		while(inv)
		{
			rec_add_uint32(inv->type - mobjinfo);
			rec_add_uint32(inv->count);
			rec_add_uint32(inv->maxcount);
			inv = inv->next;
		}
	}
}

void rec_ticcmd(ticcmd_t *cmd)
{
	if(I_CompareTiccmd(cmd, &rec_old_cmd))
	{
		// duplicate tic
		rec_ticdup++;
		return;
	}

	if(rec_ticdup)
	{
		// store dupliacate count
		rec_add_uint32(rec_ticdup << 8);
		rec_ticdup = 0;
	}

	rec_old_cmd = *cmd;

	rec_add_uint8(1); // full
	rec_add_uint8(cmd->forwardmove);
	rec_add_uint8(cmd->sidemove);
	rec_add_uint8(cmd->buttons);
	rec_add_uint32(cmd->angle);
	rec_add_uint32(cmd->pitch);
	rec_add_uint32(cmd->weapon);
	rec_ticnt++;
	if(!(rec_ticnt & 63))
	{
		mobj_t *mo = players[consoleplayer].mo;
		rec_add_uint32(mo->x ^ mo->y ^ mo->z ^ mo->health ^ mo->armorpoints);
	}
}

#ifdef LINUX
void rec_save(const char *path, const char *title)
{
	FILE *f;
#else
void rec_save(const char *path_meh, const char *title)
{
	FILE *f;
	char path[256];

	sprintf(path, BASE_PATH"%s", path_meh);
#endif

	if(title)
		strncpy(rec_buff + 16, title, 16);

	if(rec_ticdup)
	{
		// store dupliacate count
		rec_add_uint32(rec_ticdup << 8);
		rec_ticdup = 0;
	}

	f = fopen(path, "wb");
	if(f)
	{
		fwrite(rec_buff, 1, rec_ptr - rec_buff, f);
		fclose(f);
	}
}

#ifdef LINUX
void rec_load(const char *path, int type)
#else
void rec_load(const char *path_meh, int type)
#endif
{
	uint32_t tmp;
	int size, count;
	inventory_t **inv;
	inventory_t *prev;
	player_t *p = &players[consoleplayer];
#ifndef LINUX
	char path[256];

	sprintf(path, BASE_PATH"%s", path_meh);
#endif

	FILE *f = fopen(path, "rb");
	if(!f)
		I_Error("savegame file does not exist");

	// clear inventory & stuff
	if(p->inventory)
	{
		P_DestroyInventory(p->inventory);
		p->inventory = NULL;
	}

	// load state
	rec_old_cmd = *I_BaseTiccmd();
	rec_ticnt = 0;
	rec_ticdup = 0;
	fseek(f, 0, SEEK_END);
	size = ftell(f);
	fseek(f, 0, SEEK_SET);
	if(size & 3 || size < 64)
		I_Error("invalid saved size");
	fread(rec_buff, 1, size, f);
	fclose(f);
	rec_ptr = rec_buff;
	rec_end = rec_buff + size;
	// header check
	rec_get_uint32(&tmp);
	if(tmp != RECORDING_HEAD)
		I_Error("invalid save file");
	// version check
	rec_get_uint32(&tmp);
	if(tmp != RECORDING_VER)
		I_Error("invalid save version");
	// TODO: WADs checksum
	rec_get_uint32(&tmp);
	// map lump
	rec_get_uint32(&tmp);
	level_lump = tmp; // TODO: check validity
	// skip title
	rec_get_uint32(&tmp);
	rec_get_uint32(&tmp);
	rec_get_uint32(&tmp);
	rec_get_uint32(&tmp);
	// doom level numbers
	rec_get_uint32(&tmp);
	gamemap = tmp & 0xFF;
	gameepisode = (tmp >> 8) & 0xFF;
	gameskill = (tmp >> 16) & 0xFF;
	// prandom index
	rec_get_uint32(&tmp);
	prndindex = tmp & 0xFF;
	rec_get_uint32(&rand_m_z);
	rec_get_uint32(&rand_m_w);
	// player state
	ST_ClearInventory();
	rec_get_uint32(&tmp);
	if(tmp)
	{
		p->playerstate = PST_LIVE;
		// health
		rec_get_uint32(&tmp);
		p->health = tmp;
		// armor count
		rec_get_uint32(&tmp);
		p->armorpoints = tmp;
		// armor type
		rec_get_uint32(&tmp);
		if((tmp & 0xFFFF) >= numobjtypes)
			I_Error("invalid armor type");
		p->armortype = &mobjinfo[tmp & 0xFFFF];
		// inventory count
		count = tmp >> 16;
		// weapon
		rec_get_uint32(&tmp);
		p->readyweapon = tmp & 0xFFFF;
		p->pendingweapon = tmp >> 16;
		if(p->readyweapon >= numobjtypes || p->pendingweapon >= numobjtypes)
			I_Error("invalid weapon type");
		// inventory
		prev = NULL;
		inv = &p->inventory;
		while(count--)
		{
			int32_t max, cnt;
			inventory_t *new;

			if(rec_get_uint32(&tmp))
				I_Error("incomplete save");
			if(tmp >= numobjtypes)
				I_Error("invalid item type");
			rec_get_uint32((uint32_t*)&cnt);
			rec_get_uint32((uint32_t*)&max);
			new = Z_Malloc(sizeof(inventory_t), PU_STATIC, NULL);
			new->prev = prev;
			new->type = &mobjinfo[tmp];
			new->count = cnt;
			new->maxcount = max;
			prev = new;
			*inv = new;
			inv = &new->next;
			ST_CheckInventory(tmp, cnt);
		}
		*inv = NULL;
	} else
		p->playerstate = PST_REBORN;
	rec_is_playback = type;
}

void rec_get_ticcmd(ticcmd_t *cmd)
{
	uint32_t tmp32;

	if(rec_ticdup)
	{
		rec_ticdup--;
		*cmd = rec_old_cmd;
		return;
	}

	if(rec_get_uint32(&tmp32))
	{
		rec_is_playback = 0;
		rec_end = rec_buff + RECORDING_SIZE;
		return;
	}

	if(!(tmp32 & 0xFF))
	{
		rec_ticdup = (tmp32 >> 8) - 1;
		*cmd = rec_old_cmd;
		return;
	}

	cmd->forwardmove = (int8_t)((tmp32 >> 8) & 0xFF);
	cmd->sidemove = (int8_t)((tmp32 >> 16) & 0xFF);
	cmd->buttons = (int8_t)((tmp32 >> 24) & 0xFF);
	rec_get_uint32((uint32_t*)&cmd->angle);
	rec_get_uint32((uint32_t*)&cmd->pitch);
	rec_get_uint32(&tmp32);
	if(tmp32 >= numobjtypes)
		I_Error("invalid weapon type");
	cmd->weapon = tmp32;

	rec_old_cmd = *cmd;

	rec_ticnt++;

	if(!(rec_ticnt & 63))
	{
		mobj_t *mo = players[consoleplayer].mo;
		rec_get_uint32(&tmp32);
		if(tmp32 != (mo->x ^ mo->y ^ mo->z ^ mo->health ^ mo->armorpoints))
			I_Error("save is inconsistent");
	}
}

