#include "m_menu.h"
#include "p_local.h"
#include "i_system.h"
#include "i_video.h"
#include "g_game.h"
#include "doomdef.h"
#include "doomstat.h"
#include "st_stuff.h"
#include "w_wad.h"
#include "r_local.h"
#include "v_video.h"
#include "m_menu.h"
#include "hu_stuff.h"
#include "m_swap.h"

// [kg] new gameplay stuff

int sv_deathmatch;
int sv_slowmo;
int sv_freeaim = 1;
int sv_itemrespawn;	// in ticks
int sv_ammorespawn;	// in ticks
int sv_weaponrespawn;	// in ticks
int sw_powerrespawn;	// in ticks
int sv_superrespawn;	// in ticks

int playercount;

int net_mobjid;

static patch_t *background;

char network_message[512];

extern patch_t*		hu_font[HU_FONTSIZE];

//
// NETWORKING
//

void D_ProcessEvents (void); 
void G_BuildTiccmd (ticcmd_t *cmd); 
void D_DoAdvanceDemo (void);

//
// NetUpdate
//
int      gametime;

void NetUpdate (void)
{

}

#ifndef SERVER
//
// [kg] network screen

void D_StartNet()
{
	int lump = W_CheckNumForName("INTERPIC");

	if(lump < 0)
		lump = W_CheckNumForName("WIMAP0");

	gamestate = GS_NETWORK;
	background = (patch_t*)W_CacheLumpNum(lump);
	V_DrawPatch(0, 0, 0, background);

	gametime = I_GetTime();
}

void D_NetDrawer()
{
	int i;
	int start, x, y;
	char *msg = network_message;
	char *ptr = network_message;

	if(!netgame)
		return;

	if(gamestate == GS_NETWORK || netgame < 3)
		V_DrawPatch(0, 0, 0, background);

	// message
	if(*msg && !menuactive)
	{
		y = 100 - M_StringHeight(network_message)/2;
		while(1)
		{
			if(*msg == '\n' || !*msg)
			{
				char old = *msg;

				*msg = 0;
				x = 160 - M_StringWidth(ptr) / 2;
				M_WriteText(x, y, ptr);
				y += SHORT(hu_font[0]->height) + 1;

				*msg = old;
				ptr = msg + 1;
			}
			if(!*msg)
				break;
			msg++;
		}
	}
}
#endif

//
// TryRunTics
//

extern	boolean	advancedemo;

void TryRunTics (void)
{
	int newtics;
	int nowtime;

	// check time
	nowtime = I_GetTime();

	// normal speed
	newtics = nowtime - gametime;
	gametime = nowtime;

	// run the count ticks
	for(nowtime = 0; nowtime < newtics; nowtime++)
	{
		if(( (!netgame && in_weapon_menu) || sv_slowmo ) && (gametime+nowtime) & 1)
			// half speed
			continue;
#ifndef SERVER
		I_StartTic ();
		D_ProcessEvents ();
		G_BuildTiccmd(&players[consoleplayer].cmd);
		if (advancedemo)
			D_DoAdvanceDemo ();
		M_Ticker ();
#endif
		G_Ticker ();
		gametic++;
	}
}

