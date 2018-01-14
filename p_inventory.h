// new inventory system
// by kgsws

#ifndef __INVENTORY__
#define __INVENTORY__


typedef struct inventory_s
{
	struct inventory_s *next;
	struct inventory_s *prev;
	mobjinfo_t *type;
	int count;
	int maxcount;
} inventory_t;

// free memory
void P_RemoveInventory(mobj_t *mo);
void P_DestroyInventory(inventory_t *inv);
// give or take inventory
int P_GiveInventory(mobj_t *mo, mobjinfo_t *type, int count);
// set custom maximum
int P_MaxInventory(mobj_t *mo, mobjinfo_t *type, int count);
// check inventory amount; max is optional
int P_CheckInventory(mobj_t *mo, mobjinfo_t *type, int *max);
// debug info
void P_DumpInventory(mobj_t *mo);

#endif

