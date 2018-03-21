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
#include "kg_record.h"

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

void D_InitNet()
{
	int lump = W_CheckNumForName("INTERPIC");

	if(lump < 0)
		lump = W_CheckNumForName("WIMAP0");

	background = (patch_t*)W_CacheLumpNum(lump);

	gametime = I_GetTime();
}

void D_StartNet()
{
	V_DrawPatch(0, 0, 0, background);
	gamestate = GS_NETWORK;
}

void D_NetDrawer()
{
	int i;
	int start, x, y;
	char *msg = network_message;
	char *ptr = network_message;

	if(!netgame && rec_is_playback <= 1)
		return;

	if(gamestate == GS_NETWORK || netgame < 3 || rec_is_playback > 1)
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

static void RunOneTic()
{
#ifndef SERVER
	I_StartTic();
	D_ProcessEvents();
	if(!rec_is_playback)
		G_BuildTiccmd(&players[consoleplayer].cmd);
	if(advancedemo)
		D_DoAdvanceDemo();
	M_Ticker();
#endif
	G_Ticker();
	gametic++;
}

void TryRunTics (void)
{
	int newtics;
	int nowtime;

	if(rec_is_playback > 1)
	{
		// [kg] loading a "save" is in fact demo playback at full speed
		if(rec_is_playback == 2)
		{
			// show "loading"
			memset(screens[0], 0, SCREENWIDTH * SCREENHEIGHT);
			sprintf(network_message, "LOADING ...");
			D_NetDrawer();
			I_FinishUpdate();
			network_message[0] = 0;
		}
		while(rec_is_playback > 1)
			RunOneTic();
		gametime = I_GetTime();
		return;
	}

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
		RunOneTic();
	}
}

