// new pickup handling
// by kgsws

// palette flash
#define BONUSADD 6

// pickup types; returned by pickup function
#define SPECIAL_DONTPICKUP	0
#define SPECIAL_ITEM	1
#define SPECIAL_AMMO	2
#define SPECIAL_WEAPON	3
#define SPECIAL_KEY	4
#define SPECIAL_POWER	5
#define SPECIAL_SUPERPOWER	6
#define SPECIAL_REMOVE	7 // anything >= 7

#define SPECIAL_NOFLASH_FLAG	0x100	// for players, don't flash screen

void P_TouchSpecialThing(mobj_t *special, mobj_t *toucher);

