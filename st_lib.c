#include <ctype.h>

#include "doomdef.h"

#include "z_zone.h"
#include "v_video.h"

#include "m_swap.h"

#include "i_system.h"

#include "w_wad.h"

#include "st_stuff.h"
#include "st_lib.h"
#include "r_local.h"


// in AM_map.c
extern boolean		automapactive; 

// 0-9, tall numbers
static patch_t*		tallnum[10];

// tall % sign
static patch_t*		tallpercent;

// minus sign
static patch_t*		sttminus;

// 0-9, short, yellow (,different!) numbers
static patch_t*		shortnum[10];

// init numbers
void STlib_init(void)
{
    int i;
    char	namebuf[9];

    // Load the numbers, tall and short
    for (i=0;i<10;i++)
    {
	sprintf(namebuf, "STTNUM%d", i);
	tallnum[i] = (patch_t *) W_CacheLumpName(namebuf);

	sprintf(namebuf, "STYSNUM%d", i);
	shortnum[i] = (patch_t *) W_CacheLumpName(namebuf);
    }

    // Load percent key.
    tallpercent = (patch_t *) W_CacheLumpName("STTPRCNT");

    // Load minus sign
    sttminus = (patch_t *) W_CacheLumpName("STTMINUS");
}

// [kg] just draw a number, scaled
//
void
STlib_drawNum
(int x, int y, int num, byte *colormap)
{
	int neg;
	int w = SHORT(tallnum[0]->width) * 2;

	if(num < 0)
	{
		num = -num;
		neg = 1;
	} else
		neg = 0;

	// in the special case of 0, you draw 0
	if(!num)
		V_DrawPatchNew(x - w, y, tallnum[ 0 ], colormap, V_HALLIGN_NONE, V_VALLIGN_NONE, 2);

	// draw the new number
	while(num)
	{
		x -= w;
		V_DrawPatchNew(x, y, tallnum[ num % 10 ], colormap, V_HALLIGN_NONE, V_VALLIGN_NONE, 2);
		num /= 10;
	}

	// draw a minus sign if necessary
	if(neg)
		V_DrawPatchNew(x - 8, y, sttminus, colormap, V_HALLIGN_NONE, V_VALLIGN_NONE, 2);
}

