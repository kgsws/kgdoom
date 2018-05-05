#include "doomdef.h"
#include "i_system.h"
#include "r_local.h"

#include "doomdef.h"
#include "doomdata.h"

#include "m_bbox.h"
#include "m_swap.h"

#include "v_video.h"

#include "w_wad.h"


// Each screen is [SCREENWIDTH*SCREENHEIGHT]; 
byte*				screens[1];	
 
int				dirtybox[4]; 

// [kg] colormaps
byte *v_colormap_normal;
// colormap for old GFX
byte *v_patch_colormap;

void V_Init()
{
	// load all colormaps
	v_colormap_normal = W_CacheLumpName("COLORMAP");
	v_patch_colormap = v_colormap_normal;
}

//
// V_MarkRect 
// 
void
V_MarkRect
( int		x,
  int		y,
  int		width,
  int		height ) 
{ 
    M_AddToBox (dirtybox, x, y); 
    M_AddToBox (dirtybox, x+width-1, y+height-1); 
}

//
// V_DrawPatchXY1
// Masks a column based masked pic to the screen. 
// [kg] unscaled
//
void
V_DrawPatchXY1
( int		x,
  int		y,
  patch_t*	patch,
  byte*         colormap )
{ 

    int		count;
    int		col; 
    column_t*	column; 
    byte*	desttop;
    byte*	dest;
    byte*	source; 
    int		w; 
	 
#ifdef RANGECHECK 
    if (x<0
	||x+SHORT(patch->width) >SCREENWIDTH
	|| y<0
	|| y+SHORT(patch->height)>SCREENHEIGHT )
    {
//      fprintf( stderr, "Patch at %d,%d exceeds LFB\n", x,y );
      // No I_Error abort - what is up with TNT.WAD?
      printf("V_DrawPatchXY1: bad patch (ignored)\n");
      return;
    }
#endif 
 
    col = 0; 
    desttop = screens[0]+y*SCREENWIDTH+x; 

    w = SHORT(patch->width); 

    for ( ; col<w ; x++, col++, desttop++)
    { 
	column = (column_t *)((byte *)patch + LONG(patch->columnofs[col])); 

	// step through the posts in a column 
	while (column->topdelta != 0xff ) 
	{ 
	    source = (byte *)column + 3; 
	    dest = desttop + column->topdelta*SCREENWIDTH; 
	    count = column->length; 
			 
	    while (count--) 
	    { 
		*dest = colormap[*source++];
		dest += SCREENWIDTH; 
	    } 
	    column = (column_t *)(  (byte *)column + column->length 
				    + 4 ); 
	} 
    }			 
} 
 
//
// V_DrawPatchXY2
// Masks a column based masked pic to the screen. 
// [kg] scale 2x
//
void
V_DrawPatchXY2
( int		x,
  int		y,
  patch_t*	patch,
  byte*         colormap )
{ 
    int		count;
    int		col; 
    column_t*	column; 
    byte*	desttop;
    byte*	dest1;
    byte*	dest2;
    byte*	source; 
    int		w;

#ifdef RANGECHECK 
    if (x<0
	||x+SHORT(patch->width)*2 >SCREENWIDTH
	|| y<0
	|| y+SHORT(patch->height)*2>SCREENHEIGHT )
    {
//      fprintf( stderr, "Patch at %d,%d exceeds LFB\n", x,y );
      // No I_Error abort - what is up with TNT.WAD?
      printf("V_DrawPatchXY2: bad pos %ix%i (ignored)\n", x, y);
      return;
    }
#endif 

    col = 0; 
    desttop = screens[0]+y*SCREENWIDTH+x; 

    w = SHORT(patch->width); 

    for ( ; col<w ; x++, col++, desttop += 2)
    { 
	column = (column_t *)((byte *)patch + LONG(patch->columnofs[col])); 

	// step through the posts in a column 
	while (column->topdelta != 0xff ) 
	{ 
	    source = (byte *)column + 3; 
	    dest1 = desttop + column->topdelta*SCREENWIDTH*2;
	    dest2 = dest1 + SCREENWIDTH;
	    count = column->length; 
			 
	    while (count--) 
	    {
		register uint8_t c = colormap[*source++];
		dest1[0] = c;
		dest1[1] = c;
		dest2[0] = c;
		dest2[1] = c;
		dest1 += SCREENWIDTH * 2;
		dest2 += SCREENWIDTH * 2;
	    } 
	    column = (column_t *)(  (byte *)column + column->length 
				    + 4 ); 
	} 
    }
} 

//
// V_DrawPatch
// Masks a column based masked pic to the screen. 
// [kg] 3x scale
//
void
V_DrawPatchXY3
( int		x,
  int		y,
  patch_t*	patch,
  byte*         colormap,
  boolean       mirror )
{
    int		count;
    int		col; 
    column_t*	column; 
    byte*	desttop;
    byte*	dest1;
    byte*	dest2;
    byte*	dest3;
    byte*	source; 
    int		w, of;

#ifdef RANGECHECK 
    if (x<0
	||x+SHORT(patch->width)*3 >SCREENWIDTH
	|| y<0
	|| y+SHORT(patch->height)*3>SCREENHEIGHT )
    {
//      fprintf( stderr, "Patch at %d,%d exceeds LFB\n", x,y );
      // No I_Error abort - what is up with TNT.WAD?
      printf("V_DrawPatchXY3: bad pos %ix%i (ignored)\n", x, y);
      return;
    }
#endif 

    col = 0;

    w = SHORT(patch->width);

    if(mirror)
    {
	of = -3;
	desttop = screens[0]+y*SCREENWIDTH+x+(w*3-3);
    } else
    {
	of = 3;
	desttop = screens[0]+y*SCREENWIDTH+x;
    }

    for ( ; col<w ; x++, col++, desttop += of)
    {
	column = (column_t *)((byte *)patch + LONG(patch->columnofs[col])); 

	// step through the posts in a column 
	while (column->topdelta != 0xff ) 
	{
	    source = (byte *)column + 3; 
	    dest1 = desttop + column->topdelta*SCREENWIDTH*3;
	    dest2 = dest1 + SCREENWIDTH;
	    dest3 = dest2 + SCREENWIDTH;
	    count = column->length; 

	    while (count--) 
	    {
		register uint8_t c = colormap[*source++];
		dest1[0] = c;
		dest1[1] = c;
		dest1[2] = c;
		dest2[0] = c;
		dest2[1] = c;
		dest2[2] = c;
		dest3[0] = c;
		dest3[1] = c;
		dest3[2] = c;
		dest1 += SCREENWIDTH * 3;
		dest2 += SCREENWIDTH * 3;
		dest3 += SCREENWIDTH * 3;
	    } 
	    column = (column_t *)(  (byte *)column + column->length 
				    + 4 ); 
	}
    }
} 

//
// V_DrawPatch
// Masks a column based masked pic to the screen. 
// [kg] modified for GUI scaling (offset + 3x scale)
//
void
V_DrawPatch
( int		x,
  int		y,
  int		scrn,
  patch_t*	patch ) 
{
	y -= SHORT(patch->topoffset);
	x -= SHORT(patch->leftoffset);
	x *= 3;
	y *= 3;
	x += KG_GUI_OFFSET_X;
	y += KG_GUI_OFFSET_Y;
	V_DrawPatchXY3(x, y, patch, v_patch_colormap, 0);
}

//
// V_DrawPatchNew
// Masks a column based masked pic to the screen. 
// [kg] multipurpose drawing
//
void
V_DrawPatchNew
( int		x,
  int		y,
  patch_t*	patch,
  byte*         colormap,
  v_hallign_t   halgn,
  v_vallign_t   valgn,
  int scale ) 
{
	int xo = 0;
	int yo = 0;

	if(!scale) // hidden
		return;

	switch(halgn)
	{
		case V_HALLIGN_NONE:
			xo = SHORT(patch->leftoffset) * 6;
		break;
		case V_HALLIGN_CENTER:
			xo = SHORT(patch->width) * 3;
		break;
		case V_HALLIGN_RIGHT:
			xo = SHORT(patch->width) * 6;
		break;
	}

	switch(valgn)
	{
		case V_VALLIGN_NONE:
			yo = SHORT(patch->topoffset) * 6;
		break;
		case V_VALLIGN_CENTER:
			yo = SHORT(patch->height) * 3;
		break;
		case V_VALLIGN_BOTTOM:
			yo = SHORT(patch->height) * 6;
		break;
	}

	switch(scale)
	{
		case 1:
			x -= xo / 6;
			y -= yo / 6;
			V_DrawPatchXY1(x, y, patch, colormap);
		break;
		case 2:
			x -= xo / 3;
			y -= yo / 3;
			V_DrawPatchXY2(x, y, patch, colormap);
		break;
		case 3:
			x -= xo / 2;
			y -= yo / 2;
			V_DrawPatchXY3(x, y, patch, colormap, 0);
		break;
	}
}

//
// V_DrawPatchRemap1
// [kg] use colormap against a screen, unscaled
//
void
V_DrawPatchRemap1
( int		x,
  int		y,
  patch_t*	patch,
  byte*         colormap )
{ 

    int		count;
    int		col; 
    column_t*	column; 
    byte*	desttop;
    byte*	dest;
    int		w; 
	 
#ifdef RANGECHECK 
    if (x<0
	||x+SHORT(patch->width) >SCREENWIDTH
	|| y<0
	|| y+SHORT(patch->height)>SCREENHEIGHT )
    {
//      fprintf( stderr, "Patch at %d,%d exceeds LFB\n", x,y );
      // No I_Error abort - what is up with TNT.WAD?
      printf("V_DrawPatchXY1: bad patch (ignored)\n");
      return;
    }
#endif 
 
    col = 0; 
    desttop = screens[0]+y*SCREENWIDTH+x; 

    w = SHORT(patch->width); 

    for ( ; col<w ; x++, col++, desttop++)
    { 
	column = (column_t *)((byte *)patch + LONG(patch->columnofs[col])); 

	// step through the posts in a column 
	while (column->topdelta != 0xff ) 
	{ 
	    dest = desttop + column->topdelta*SCREENWIDTH; 
	    count = column->length; 
			 
	    while (count--) 
	    { 
		*dest = colormap[*dest];
		dest += SCREENWIDTH; 
	    } 
	    column = (column_t *)(  (byte *)column + column->length 
				    + 4 ); 
	} 
    }			 
}
 
//
// V_DrawPatchFlipped 
// Masks a column based masked pic to the screen.
// Flips horizontally, e.g. to mirror face.
// [kg] modified for GUI scaling (offset + 3x scale)
//
void
V_DrawPatchFlipped
( int		x,
  int		y,
  int		scrn,
  patch_t*	patch ) 
{ 
	y -= SHORT(patch->topoffset);
	x -= SHORT(patch->leftoffset);
	x *= 3;
	y *= 3;
	x += KG_GUI_OFFSET_X;
	y += KG_GUI_OFFSET_Y;
	V_DrawPatchXY3(x, y, patch, v_colormap_normal, 1);
}

//
// V_DrawBlock
// Draw a linear block of pixels into the view buffer.
//
// [kg] modified for new font rendering
void V_DrawBlock1(int x, int y, byte *colormap, int width, int height, byte *src)
{
    byte *dest;

#ifdef RANGECHECK
    if (x<0
	||x+width >SCREENWIDTH
	|| y<0
	|| y+height>SCREENHEIGHT )
    {
//	I_Error ("Bad V_DrawBlock");
	return;
    }
#endif

//    V_MarkRect (x, y, width, height); 

    dest = screens[0] + y*SCREENWIDTH+x; 

    while (height--) 
    {
	int count = width;
	while(count--)
	{
		if(*src)
			*dest = colormap[*src];
		src++;
		dest++;
	}
	dest += SCREENWIDTH - width;
    }
}

// [kg] double pixels
void V_DrawBlock2(int x, int y, byte *colormap, int width, int height, byte *src)
{
    byte *dest;

#ifdef RANGECHECK
    if (x<0
	||x+width*2 >SCREENWIDTH
	|| y<0
	|| y+height*2>SCREENHEIGHT )
    {
//	I_Error ("Bad V_DrawBlock");
	return;
    }
#endif

//    V_MarkRect (x, y, width, height); 

    dest = screens[0] + y*SCREENWIDTH+x; 

    while (height--) 
    {
	int count = width;
	while(count--)
	{
		uint8_t idx = *src;
		if(idx)
		{
			*dest = colormap[idx];
			dest++;
			*dest = colormap[idx];
			dest++;
		} else
			dest += 2;
		src++;
	}
	dest += SCREENWIDTH - width*2;
	src -= width;
	count = width;
	while(count--)
	{
		uint8_t idx = *src;
		if(idx)
		{
			*dest = colormap[idx];
			dest++;
			*dest = colormap[idx];
			dest++;
		} else
			dest += 2;
		src++;
	}
	dest += SCREENWIDTH - width*2;
    }
}

// [kg] triple pixels
void V_DrawBlock3(int x, int y, byte *colormap, int width, int height, byte *src)
{
    byte *dest;

#ifdef RANGECHECK
    if (x<0
	||x+width*3 >SCREENWIDTH
	|| y<0
	|| y+height*3>SCREENHEIGHT )
    {
//	I_Error ("Bad V_DrawBlock");
	return;
    }
#endif

//    V_MarkRect (x, y, width, height); 

    dest = screens[0] + y*SCREENWIDTH+x; 

    while (height--) 
    {
	int count = width;
	while(count--)
	{
		uint8_t idx = *src;
		if(idx)
		{
			*dest = colormap[idx];
			dest++;
			*dest = colormap[idx];
			dest++;
			*dest = colormap[idx];
			dest++;
		} else
			dest += 3;
		src++;
	}
	dest += SCREENWIDTH - width*3;
	src -= width;
	count = width;
	while(count--)
	{
		uint8_t idx = *src;
		if(idx)
		{
			*dest = colormap[idx];
			dest++;
			*dest = colormap[idx];
			dest++;
			*dest = colormap[idx];
			dest++;
		} else
			dest += 3;
		src++;
	}
	dest += SCREENWIDTH - width*3;
	src -= width;
	count = width;
	while(count--)
	{
		uint8_t idx = *src;
		if(idx)
		{
			*dest = colormap[idx];
			dest++;
			*dest = colormap[idx];
			dest++;
			*dest = colormap[idx];
			dest++;
		} else
			dest += 3;
		src++;
	}
	dest += SCREENWIDTH - width*3;
    }
}

//
// V_GetBlock
// Gets a linear block of pixels from the view buffer.
//
void
V_GetBlock
( int		x,
  int		y,
  int		scrn,
  int		width,
  int		height,
  byte*		dest ) 
{ 
    byte*	src; 
	 
#ifdef RANGECHECK 
    if (x<0
	||x+width >SCREENWIDTH
	|| y<0
	|| y+height>SCREENHEIGHT 
	|| (unsigned)scrn>4 )
    {
	I_Error ("Bad V_DrawBlock");
    }
#endif 
 
    src = screens[0] + y*SCREENWIDTH+x; 

    while (height--) 
    { 
	memcpy (dest, src, width); 
	src += SCREENWIDTH; 
	dest += width; 
    } 
} 

// [kg] fade screen
void V_FadeScreen(uint8_t *colormap, int level)
{
	int i;
	uint8_t *pixel = screens[0];
	colormap += level * 256;

	for(i = 0; i < SCREENWIDTH * SCREENHEIGHT; i++, pixel++)
		*pixel = colormap[*pixel];
}

