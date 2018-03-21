// singleplayer game recording
// also used as game saves
// by kgsws
#include "doomdef.h"
#include "doomstat.h"
#include "i_system.h"
#include "p_local.h"
#include "p_inventory.h"
#include "m_random.h"
#include "kg_record.h"

int rec_is_playback;

static void *rec_buff;
static void *rec_ptr;
static void *rec_end;

static int rec_ticnt;

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

	// reset pointer
	rec_ptr = rec_buff;
	// add header
	rec_add_uint32(RECORDING_HEAD);
	rec_add_uint32(RECORDING_VER);
	// TODO: WADs checksum
	rec_add_uint32(0xFFAA5581);
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
		// full player state
		rec_add_uint32(p->health);
		rec_add_uint32(p->armorpoints);
		rec_add_uint32(p->armortype - mobjinfo);
		// inventory size
		i = 0;
		inv = p->inventory;
		while(inv)
		{
			i++;
			inv = inv->next;
		}
		rec_add_uint32(i);
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

void rec_save(const char *path)
{
	FILE *f = fopen(path, "wb");
	if(f)
	{
		fwrite(rec_buff, 1, rec_ptr - rec_buff, f);
		fclose(f);
	}
}

void rec_load(const char *path, int type)
{
	uint32_t tmp;
	int size;
	FILE *f = fopen(path, "rb");
	if(f)
	{
		rec_ticnt = 0;
		fseek(f, 0, SEEK_END);
		size = ftell(f);
		fseek(f, 0, SEEK_SET);
		if(size & 3 || size < 32)
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
		// prandom index
		rec_get_uint32(&tmp);
		prndindex = tmp & 0xFF;
		rec_get_uint32(&rand_m_z);
		rec_get_uint32(&rand_m_w);
		// player state
		rec_get_uint32(&tmp);
		if(tmp)
			I_Error("TODO: alive player inventory");
		players[consoleplayer].playerstate = PST_REBORN;
		rec_is_playback = type;
	}
}

void rec_get_ticcmd(ticcmd_t *cmd)
{
	// TODO: record ending
	uint8_t tmp8;
	uint32_t tmp32;

	if(rec_get_uint8(&tmp8))
	{
		rec_is_playback = 0;
		rec_end = rec_buff + RECORDING_SIZE;
		return;
	}

	if(tmp8 != 1)
		I_Error("TODO: same ticcmd");

	rec_get_uint8((uint8_t*)&cmd->forwardmove);
	rec_get_uint8((uint8_t*)&cmd->sidemove);
	rec_get_uint8((uint8_t*)&cmd->buttons);
	rec_get_uint32((uint32_t*)&cmd->angle);
	rec_get_uint32((uint32_t*)&cmd->pitch);
	rec_get_uint32(&tmp32);
	if(tmp32 >= numobjtypes)
		I_Error("invalid weapon type");
	cmd->weapon = tmp32;

	rec_ticnt++;

	if(!(rec_ticnt & 63))
	{
		mobj_t *mo = players[consoleplayer].mo;
		rec_get_uint32(&tmp32);
		if(tmp32 != (mo->x ^ mo->y ^ mo->z ^ mo->health ^ mo->armorpoints))
			I_Error("save is inconsistent");
	}
}

