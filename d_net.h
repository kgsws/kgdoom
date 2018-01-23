#ifndef __D_NET__
#define __D_NET__

#include "d_player.h"


#ifdef __GNUG__
#pragma interface
#endif

// [kg] new gameplay stuff

extern int sv_deathmatch;
extern int sv_slowmo;
extern int sv_freeaim;
extern int sv_itemrespawn;
extern int sv_ammorespawn;
extern int sv_weaponrespawn;
extern int sw_powerrespawn;
extern int sv_superrespawn;

extern int playercount;

extern int net_mobjid;

extern char network_message[512];

//
// Network play related stuff.
// There is a data struct that stores network
//  communication related stuff, and another
//  one that defines the actual packets to
//  be transmitted.
//

// Networking and tick handling related.
#define BACKUPTICS		TICRATE

// Create any new ticcmds and broadcast to other players.
void NetUpdate (void);

// Broadcasts special packets to other players
//  to notify of game exit
void D_QuitNetGame (void);

//? how many ticks to run?
void TryRunTics (void);

// [kg] network screen
void D_StartNet();
void D_NetDrawer();

#endif

