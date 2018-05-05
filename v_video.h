#ifndef __V_VIDEO__
#define __V_VIDEO__

#include "doomtype.h"

#include "doomdef.h"

// Needed because we are refering to patches.
#include "r_data.h"

//
// VIDEO
//

#define CENTERY			(SCREENHEIGHT/2)

// [kg] original GUI scaling
#define KG_GUI_OFFSET_X	((SCREENWIDTH/2)-(320+160))
#define KG_GUI_OFFSET_Y	((SCREENHEIGHT/2)-(200+100))

// [kg] patch positioning
typedef enum
{
	V_HALLIGN_NONE, // position by patch offset
	V_HALLIGN_LEFT,
	V_HALLIGN_CENTER,
	V_HALLIGN_RIGHT
} v_hallign_t;
typedef enum
{
	V_VALLIGN_NONE, // position by patch offset
	V_VALLIGN_TOP,
	V_VALLIGN_CENTER,
	V_VALLIGN_BOTTOM
} v_vallign_t;

// [kg] color remapping
extern byte *v_colormap_normal;
// colormap for old GFX
extern byte *v_patch_colormap;

// [kg] only one framebuffer for all
extern	byte*		screens[1];

extern  int	dirtybox[4];

// [kg] init, load data
void V_Init();

// [kg] original GUI drawing; scrn is unused
void
V_DrawPatch
( int		x,
  int		y,
  int		scrn,
  patch_t*	patch);

// [kg] draw at 1x scale
void
V_DrawPatchXY1
( int		x,
  int		y,
  patch_t*	patch,
  byte*         colormap );

// [kg] draw at 2x scale
void
V_DrawPatchXY2
( int		x,
  int		y,
  patch_t*	patch,
  byte*         colormap );

// [kg] draw at 3x scale
void
V_DrawPatchXY3
( int		x,
  int		y,
  patch_t*	patch,
  byte*         colormap,
  boolean       mirror );

// [kg] new, combined drawing
void
V_DrawPatchNew
( int		x,
  int		y,
  patch_t*	patch,
  byte*         colormap,
  v_hallign_t   halgn,
  v_vallign_t   valgn,
  int scale );

// [kg] remap screen behind a patch using colormap
void
V_DrawPatchRemap1
( int		x,
  int		y,
  patch_t*	patch,
  byte*         colormap );

//
//

void
V_CopyRect
( int		srcx,
  int		srcy,
  int		srcscrn,
  int		width,
  int		height,
  int		destx,
  int		desty,
  int		destscrn );

// Draw a linear block of pixels into the view buffer.
void
V_DrawBlock1
( int		x,
  int		y,
  byte*         colormap,
  int		width,
  int		height,
  byte*		src );
void
V_DrawBlock2
( int		x,
  int		y,
  byte*         colormap,
  int		width,
  int		height,
  byte*		src );
void
V_DrawBlock3
( int		x,
  int		y,
  byte*         colormap,
  int		width,
  int		height,
  byte*		src );

// Reads a linear block of pixels into the view buffer.
void
V_GetBlock
( int		x,
  int		y,
  int		scrn,
  int		width,
  int		height,
  byte*		dest );


void
V_MarkRect
( int		x,
  int		y,
  int		width,
  int		height );

// [kg] fade screen
void V_FadeScreen(uint8_t *colormap, int level);

#endif

