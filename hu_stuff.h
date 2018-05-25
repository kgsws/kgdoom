#ifndef __HU_STUFF_H__
#define __HU_STUFF_H__

#include "d_event.h"


//
// Globally visible constants.
//
#define HU_FONTSTART	'!'	// the first font characters
#define HU_FONTEND	'_'	// the last font characters

// Calculate # of glyphs in font.
#define HU_FONTSIZE	(HU_FONTEND - HU_FONTSTART + 1)	

#define HU_BROADCAST	5

#define HU_MSGREFRESH	KEY_ENTER
#define HU_MSGX		0
#define HU_MSGY		0
#define HU_MSGWIDTH	64	// in characters
#define HU_MSGHEIGHT	1	// in lines

#define HU_MSGTIMEOUT	(4*TICRATE)

// [kg] new HUD messages
typedef struct hudmsg_s
{
	struct hudmsg_s *next;
	int id, x, y, tics, scale, align;
	void *font;
	void *colormap;
	int line_count;
	uint8_t data[]; // placeholder for stuff below
} hudmsg_t;

typedef struct
{
	uint16_t len;
	uint16_t width;
	char text[];
} hudmsgline_t;

extern int hudmsg_align;
extern int hudmsg_scale;

//
// HEADS UP TEXT
//

void HU_Init(void);
void HU_Start(void);

boolean HU_Responder(event_t* ev);

void HU_Ticker(void);
void HU_Drawer(void);
void HU_Erase(void);

// [kg] HUD messages
void HU_MessageAlign(int align);
void HU_MessagePlain(int id, int x, int y, int tics, const char *text);

#endif

