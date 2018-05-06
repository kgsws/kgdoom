#include "doomdef.h"
#include "i_system.h"

#include "z_zone.h"

#include "m_swap.h"

#include "hu_stuff.h"
#include "w_wad.h"

#include "s_sound.h"
#include "v_video.h"

#include "doomstat.h"

#include "kg_text.h"

// Data.
#include "dstrings.h"

//
// Locally used constants, shortcuts.
//
#define HU_TITLE	(mapnames[(gameepisode-1)*9+gamemap-1])
#define HU_TITLE2	(mapnames2[gamemap-1])
#define HU_TITLEP	(mapnamesp[gamemap-1])
#define HU_TITLET	(mapnamest[gamemap-1])
#define HU_TITLEHEIGHT	1

static player_t*	plr;
static boolean		always_off = false;

static boolean		message_on;
boolean			message_dontfuckwithme;
static boolean		message_nottobefuckedwith;

static int		message_counter;

extern int		showMessages;
extern boolean		automapactive;

static boolean		headsupactive = false;

// [kg] player message
char plr_message[256];

// [kg] new hud messages
static hudmsg_t *hudmsg_top;
static void *hudmsg_font;
int hudmsg_scale;
int hudmsg_align;
static uint8_t *hudmsg_color;

//
// Builtin map names.
// The actual names can be found in DStrings.h.
//
/*
char*	mapnames[] =	// DOOM shareware/registered/retail (Ultimate) names.
{

    HUSTR_E1M1,
    HUSTR_E1M2,
    HUSTR_E1M3,
    HUSTR_E1M4,
    HUSTR_E1M5,
    HUSTR_E1M6,
    HUSTR_E1M7,
    HUSTR_E1M8,
    HUSTR_E1M9,

    HUSTR_E2M1,
    HUSTR_E2M2,
    HUSTR_E2M3,
    HUSTR_E2M4,
    HUSTR_E2M5,
    HUSTR_E2M6,
    HUSTR_E2M7,
    HUSTR_E2M8,
    HUSTR_E2M9,

    HUSTR_E3M1,
    HUSTR_E3M2,
    HUSTR_E3M3,
    HUSTR_E3M4,
    HUSTR_E3M5,
    HUSTR_E3M6,
    HUSTR_E3M7,
    HUSTR_E3M8,
    HUSTR_E3M9,

    HUSTR_E4M1,
    HUSTR_E4M2,
    HUSTR_E4M3,
    HUSTR_E4M4,
    HUSTR_E4M5,
    HUSTR_E4M6,
    HUSTR_E4M7,
    HUSTR_E4M8,
    HUSTR_E4M9,

    "NEWLEVEL",
    "NEWLEVEL",
    "NEWLEVEL",
    "NEWLEVEL",
    "NEWLEVEL",
    "NEWLEVEL",
    "NEWLEVEL",
    "NEWLEVEL",
    "NEWLEVEL"
};

char*	mapnames2[] =	// DOOM 2 map names.
{
    HUSTR_1,
    HUSTR_2,
    HUSTR_3,
    HUSTR_4,
    HUSTR_5,
    HUSTR_6,
    HUSTR_7,
    HUSTR_8,
    HUSTR_9,
    HUSTR_10,
    HUSTR_11,
	
    HUSTR_12,
    HUSTR_13,
    HUSTR_14,
    HUSTR_15,
    HUSTR_16,
    HUSTR_17,
    HUSTR_18,
    HUSTR_19,
    HUSTR_20,
	
    HUSTR_21,
    HUSTR_22,
    HUSTR_23,
    HUSTR_24,
    HUSTR_25,
    HUSTR_26,
    HUSTR_27,
    HUSTR_28,
    HUSTR_29,
    HUSTR_30,
    HUSTR_31,
    HUSTR_32
};


char*	mapnamesp[] =	// Plutonia WAD map names.
{
    PHUSTR_1,
    PHUSTR_2,
    PHUSTR_3,
    PHUSTR_4,
    PHUSTR_5,
    PHUSTR_6,
    PHUSTR_7,
    PHUSTR_8,
    PHUSTR_9,
    PHUSTR_10,
    PHUSTR_11,
	
    PHUSTR_12,
    PHUSTR_13,
    PHUSTR_14,
    PHUSTR_15,
    PHUSTR_16,
    PHUSTR_17,
    PHUSTR_18,
    PHUSTR_19,
    PHUSTR_20,
	
    PHUSTR_21,
    PHUSTR_22,
    PHUSTR_23,
    PHUSTR_24,
    PHUSTR_25,
    PHUSTR_26,
    PHUSTR_27,
    PHUSTR_28,
    PHUSTR_29,
    PHUSTR_30,
    PHUSTR_31,
    PHUSTR_32
};


char *mapnamest[] =	// TNT WAD map names.
{
    THUSTR_1,
    THUSTR_2,
    THUSTR_3,
    THUSTR_4,
    THUSTR_5,
    THUSTR_6,
    THUSTR_7,
    THUSTR_8,
    THUSTR_9,
    THUSTR_10,
    THUSTR_11,
	
    THUSTR_12,
    THUSTR_13,
    THUSTR_14,
    THUSTR_15,
    THUSTR_16,
    THUSTR_17,
    THUSTR_18,
    THUSTR_19,
    THUSTR_20,
	
    THUSTR_21,
    THUSTR_22,
    THUSTR_23,
    THUSTR_24,
    THUSTR_25,
    THUSTR_26,
    THUSTR_27,
    THUSTR_28,
    THUSTR_29,
    THUSTR_30,
    THUSTR_31,
    THUSTR_32
};
*/

void HU_Init(void)
{
	HT_Init(&hudmsg_font, &hudmsg_scale);
	hudmsg_color = W_CacheLumpName("COLORMAP");
}

void HU_Stop(void)
{
    headsupactive = false;
}

void HU_Start(void)
{

    int		i;
    char*	s;

    if (headsupactive)
	HU_Stop();

    plr = &players[consoleplayer];
    message_on = false;
    message_dontfuckwithme = false;
    message_nottobefuckedwith = false;

    headsupactive = true;
}

void HU_Drawer(void)
{
	boolean changed = false;
	hudmsg_t *msg = hudmsg_top;
	// hud messages
	while(msg)
	{
		int x = msg->x;
		changed = true;
		HT_SetFont(msg->font, msg->scale, msg->colormap);
		switch(msg->align)
		{
			case 1:
				x -= HT_TextWidth(msg->text) / 2;
			break;
			case 2:
				x -= HT_TextWidth(msg->text);
			break;
		}
		HT_PutText(x, msg->y, msg->text);
		msg = msg->next;
	}
	// "pickup" message
	if(plr_message[0])
	{
		HT_SetSmallFont(2, v_colormap_normal);
		HT_PutText(0, 0, plr_message);
		changed = true;
	}
	if(changed)
		HT_SetSmallFont(3, v_colormap_normal);
}

void HU_Erase(void)
{
	hudmsg_t *msg = hudmsg_top;

	plr_message[0] = 0;

	while(msg)
	{
		hudmsg_t *tmp = msg;
		msg = msg->next;
		free(tmp);
	}
	hudmsg_top = NULL;
}

void HU_Ticker(void)
{
	int i, rc;
	char c;

	hudmsg_t *msg = hudmsg_top;
	hudmsg_t **mld = &hudmsg_top;
	// hud messages
	while(msg)
	{
		if(msg->tics > 0)
		{
			msg->tics--;
			if(!msg->tics)
			{
				hudmsg_t *tmp = msg;
				*mld = msg->next;
				msg = msg->next;
				free(tmp);
				continue;
			}
		}
		mld = &msg->next;
		msg = msg->next;
	}

	// tick down message counter if message is up
	if (message_counter && !--message_counter)
	{
		plr_message[0] = 0;
		message_on = false;
		message_nottobefuckedwith = false;
	}

	if (showMessages || message_dontfuckwithme)
	{
		// display message if necessary
		if ((plr->message && !message_nottobefuckedwith)
		|| (plr->message && message_dontfuckwithme))
		{
			strncpy(plr_message, plr->message, sizeof(plr_message));
			plr->message = 0;
			message_on = true;
			message_counter = HU_MSGTIMEOUT;
			message_nottobefuckedwith = message_dontfuckwithme;
			message_dontfuckwithme = 0;
		}
	} // else message_on = false;
}

boolean HU_Responder(event_t *ev)
{
	return false;
}

//
// [kg] HUD messages

void HU_MessagePlain(int id, int x, int y, int tics, const char *text)
{
	int len = strlen(text);
	hudmsg_t *msg = hudmsg_top;
	hudmsg_t **mld = &hudmsg_top;
	hudmsg_t *new = NULL;

	if(len)
	{
		new = malloc(sizeof(hudmsg_t) + len + 1);
		if(!new)
			I_Error("HU_MessagePlain: Memory allocation error.");
	}

	while(msg)
	{
		if(msg->id == id)
		{
			hudmsg_t *tmp = msg;
			msg = msg->next;
			free(tmp);
			break;
		}
		mld = &msg->next;
		msg = msg->next;
	}

	if(!new)
	{
		*mld = msg;
		return;
	}

	new->id = id;
	new->x = x;
	new->y = y;
	new->tics = tics;
	new->scale = hudmsg_scale;
	new->align = hudmsg_align;
	new->font = hudmsg_font;
	new->colormap = hudmsg_color;
	strcpy(new->text, text);
	new->next = msg;
	*mld = new;
}

