// new inventory system
// by kgsws
#include "z_zone.h"
#include "doomdef.h"
#include "p_local.h"
#include "s_sound.h"
#include "doomstat.h"
#include "r_state.h"
#include "info.h"
#include "p_mobj.h"
#include "p_inventory.h"
#include "st_stuff.h"

static inventory_t *ret_inv;

void P_RemoveInventory(mobj_t *mo)
{
	if(mo->player == &players[consoleplayer])
		ST_ClearInventory();
	P_DestroyInventory(mo->inventory);
	mo->inventory = NULL;
}

void P_DestroyInventory(inventory_t *inv)
{
	while(inv)
	{
		inventory_t *cur = inv;
		inv = inv->next;
		Z_Free(cur);
	}
}

int P_GiveInventory(mobj_t *mo, mobjinfo_t *type, int count)
{
	int max;
	inventory_t *inv;

	ret_inv = NULL;

	// locate and modify existing slot
	inv = mo->inventory;
	while(inv)
	{
		inventory_t *cur = inv;
		inv = inv->next;
		if(cur->type == type)
		{
			ret_inv = cur;

			max = cur->maxcount;

			if(max < 0)
				// depletable item
				max = -max;

			if(cur->count + count > max)
			{
				// can't add over maximum
				count -= max - cur->count; // return leftover amount
				cur->count = max;
				goto hud_check;
			}
			cur->count += count;
			if(cur->count < 0)
			{
				// cant take under 0
				count = -cur->count; // return untaken amount
				cur->count = 0;
			} else
				count = 0;
			if(cur->count == 0 && cur->maxcount)
			{
				// free this item slot
				if(cur->prev)
					cur->prev->next = inv;
				if(inv)
					inv->prev = cur->prev;
				if(cur == mo->inventory)
					mo->inventory = inv;
				Z_Free(cur);
			}
			goto hud_check;
		}
	}

	// is it take inventory?
	if(count < 0)
	{
		count = -count; // return untaken amount
		goto hud_check;
	}

	// can give 0?
	if(!count && type->maxcount >= 0)
	{
		count = 0;
		goto hud_check;
	}

	// add new inventory slot
	max = type->maxcount;
	inv = Z_Malloc(sizeof(inventory_t), PU_STATIC, NULL);
	inv->next = mo->inventory;
	inv->prev = NULL;
	inv->type = type;
	inv->count = count;
	inv->maxcount = max;

	if(mo->inventory)
		mo->inventory->prev = inv;
	mo->inventory = inv;

	if(max < 0)
		// depletable item
		max = -max;

	if(count > max)
	{
		count -= max - inv->count; // return leftover amount
		inv->count = max;
	} else
		count = 0;

	ret_inv = inv;

hud_check:
	// given HUD item?
	if(ret_inv && mo->player == &players[consoleplayer])
		ST_CheckInventory(type - mobjinfo, ret_inv->count);

	return count;
}

int P_MaxInventory(mobj_t *mo, mobjinfo_t *type, int count)
{
	inventory_t *inv;

	inv = mo->inventory;
	while(inv)
	{
		if(inv->type == type)
		{
			// set new maximum
			inv->maxcount = count;

			if(count < 0)
				// depletable item
				count = -count;

			if(inv->count > count)
			{
				// too many items, remove some
				int ret = count - inv->count;
				inv->count = count;
				return ret;
			} else
				return 0;
		}
		inv = inv->next;
	}

	if(count < 0)
	{
		// add depletable item
		P_GiveInventory(mo, type, 0);
		if(ret_inv)
			ret_inv->maxcount = count;
	}

	return 0;
}

int P_CheckInventory(mobj_t *mo, mobjinfo_t *type, int *max)
{
	inventory_t *inv;

	inv = mo->inventory;
	while(inv)
	{
		if(inv->type == type)
		{
			if(max)
				*max = inv->maxcount;
			return inv->count;
		}
		inv = inv->next;
	}
	if(max)
		*max = 0;
	return 0;
}

void P_DumpInventory(mobj_t *mo)
{
	inventory_t *inv = mo->inventory;

	while(inv)
	{
		printf("item %p: %i / %i\n", inv->type, inv->count, inv->maxcount);
		inv = inv->next;
	}

	mo->inventory = NULL;
}

