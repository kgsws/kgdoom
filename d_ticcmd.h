#ifndef __D_TICCMD__
#define __D_TICCMD__

#include "doomtype.h"
#include "tables.h"

#ifdef __GNUG__
#pragma interface
#endif

// The data sampled per tick (single player)
// and transmitted to other peers (multiplayer).
// Mainly movements/button commands per game tick,
// plus a checksum for internal state consistency.
typedef struct
{
	angle_t	angle;
	fixed_t	pitch;
	int8_t	forwardmove;	// *2048 for move
	int8_t	sidemove;	// *2048 for move
	uint8_t	buttons;
	uint8_t	weapon;		// [kg] weapon change
} ticcmd_t;



#endif

