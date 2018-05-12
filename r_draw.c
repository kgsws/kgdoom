#include "doomdef.h"

#include "i_system.h"
#include "z_zone.h"
#include "w_wad.h"

#include "r_local.h"

// Needs access to LFB (guess what).
#include "v_video.h"

// State.
#include "doomstat.h"


// status bar height at bottom of screen
#define SBARHEIGHT		32

//
// All drawing to the view buffer is accomplished in this file.
// The other refresh files only know about ccordinates,
//  not the architecture of the frame buffer.
// Conveniently, the frame buffer is a linear one,
//  and we need only the base address,
//  and the total size == width*height*depth/8.,
//


byte*		viewimage; 
int		viewwidth;
int		scaledviewwidth;
int		viewheight;
int		viewwindowx;
int		viewwindowy; 
byte*		ylookup[SCREENHEIGHT];
int		columnofs[SCREENWIDTH];

//
// R_DrawColumn
// Source is the top of the column to scale.
//
int			dc_x; 
int			dc_yl; 
int			dc_yh; 
fixed_t			dc_iscale; 
fixed_t			dc_texturemid;
int			dc_holestep;

// [kg] updated handling of colorization
uint8_t *dc_colormap; // this is light offset
uint8_t *dc_translation; // this is color remap
uint8_t *dc_lightcolor; // this is sector light color
uint8_t *dc_table; // this is color blend

// first pixel in a column (possibly virtual) 
byte*	dc_source;
int	dc_src_height;
int	dc_src_width;
int	dc_column;
byte	dc_fz_buf[1024];
byte 	*dc_fz_buffer;

boolean dc_is_sprite;

// just for profiling 
int			dccount;

// for wall textured planes
int dc_texture;

//
// A column is a vertical slice/span from a wall texture that,
//  given the DOOM style restrictions on the view orientation,
//  will always have constant z depth.
// Thus a special case loop for very fast rendering can
//  be used. It has also been used with Wolfenstein 3D.
// 
// [kg] shared with solid walls and sprites
void R_DrawColumn(void)
{ 
    int			count; 
    byte*		dest; 
    fixed_t		frac;
    fixed_t		fracstep;	 
 
    count = dc_yh - dc_yl; 

    // Zero length, column does not exceed a pixel.
    if (count < 0) 
	return; 
				 
#ifdef RANGECHECK 
    if ((unsigned)dc_x >= SCREENWIDTH
	|| dc_yl < 0
	|| dc_yh >= SCREENHEIGHT) 
	I_Error ("R_DrawColumn: %i to %i at %i", dc_yl, dc_yh, dc_x); 
#endif 

    // Framebuffer destination address.
    // Use ylookup LUT to avoid multiply with ScreenWidth.
    // Use columnofs LUT for subwindows?
    dest = ylookup[dc_yl] + columnofs[dc_x];

    // Determine scaling,
    //  which is the only mapping to be done.
    fracstep = dc_iscale; 
    frac = dc_texturemid + (dc_yl-centery)*fracstep;
    if(frac < 0)
    {
	// [kg] fix offset
	if(dc_is_sprite)
		// sprite column can't start negative
		frac = 0;
	else
	{
		// wall column wrap-around
		fixed_t tx = dc_src_height << FRACBITS;
		frac += ((-frac + (tx-1)) / tx) * tx;
	}
    }

    // Inner loop that does the actual texture mapping,
    //  e.g. a DDA-lile scaling.
    // This is as fast as it gets.
    do 
    {
	// Re-map color indices from wall texture column
	//  using a lighting/special effects LUT.
	*dest = dc_colormap[dc_lightcolor[dc_source[(frac>>FRACBITS) % dc_src_height]]];
	
	dest += SCREENWIDTH; 
	frac += fracstep;
	
    } while (count--); 
}

// [kg] used for sprites only
void R_DrawColumnTabled0(void)
{ 
    int			count; 
    byte*		dest; 
    fixed_t		frac;
    fixed_t		fracstep;	 
 
    count = dc_yh - dc_yl; 

    // Zero length, column does not exceed a pixel.
    if (count < 0) 
	return; 
				 
#ifdef RANGECHECK 
    if ((unsigned)dc_x >= SCREENWIDTH
	|| dc_yl < 0
	|| dc_yh >= SCREENHEIGHT) 
	I_Error ("R_DrawColumn: %i to %i at %i", dc_yl, dc_yh, dc_x); 
#endif 

    // Framebuffer destination address.
    // Use ylookup LUT to avoid multiply with ScreenWidth.
    // Use columnofs LUT for subwindows? 
    dest = ylookup[dc_yl] + columnofs[dc_x];  

    // Determine scaling,
    //  which is the only mapping to be done.
    fracstep = dc_iscale; 
    frac = dc_texturemid + (dc_yl-centery)*fracstep;
    // [kg] sprite column can't start negative
    if(frac < 0)
	frac = 0;

    // Inner loop that does the actual texture mapping,
    //  e.g. a DDA-lile scaling.
    // This is as fast as it gets.
    do 
    {
	// Re-map color indices from wall texture column
	//  using a lighting/special effects LUT.
	uint8_t *src = &dc_source[(frac>>FRACBITS) % dc_src_height];
	*dest = dc_table[dc_colormap[dc_lightcolor[dc_translation[*src]]] + *dest * 256];

	dest += SCREENWIDTH; 
	frac += fracstep;
	
    } while (count--); 
}

// [kg] used for sprites only
void R_DrawColumnTabled1(void)
{ 
    int			count; 
    byte*		dest; 
    fixed_t		frac;
    fixed_t		fracstep;	 
 
    count = dc_yh - dc_yl; 

    // Zero length, column does not exceed a pixel.
    if (count < 0) 
	return; 
				 
#ifdef RANGECHECK 
    if ((unsigned)dc_x >= SCREENWIDTH
	|| dc_yl < 0
	|| dc_yh >= SCREENHEIGHT) 
	I_Error ("R_DrawColumn: %i to %i at %i", dc_yl, dc_yh, dc_x); 
#endif 

    // Framebuffer destination address.
    // Use ylookup LUT to avoid multiply with ScreenWidth.
    // Use columnofs LUT for subwindows? 
    dest = ylookup[dc_yl] + columnofs[dc_x];  

    // Determine scaling,
    //  which is the only mapping to be done.
    fracstep = dc_iscale; 
    frac = dc_texturemid + (dc_yl-centery)*fracstep;
    // [kg] sprite column can't start negative
    if(frac < 0)
	frac = 0;

    // Inner loop that does the actual texture mapping,
    //  e.g. a DDA-lile scaling.
    // This is as fast as it gets.
    do 
    {
	// Re-map color indices from wall texture column
	//  using a lighting/special effects LUT.
	uint8_t *src = &dc_source[(frac>>FRACBITS) % dc_src_height];
	*dest = dc_table[dc_colormap[dc_lightcolor[dc_translation[*src]]] * 256 + *dest];

	dest += SCREENWIDTH; 
	frac += fracstep;
	
    } while (count--); 
}

// [kg] used for mid walls only
void R_DrawColumnMasked(void)
{ 
    int			count; 
    byte*		dest; 
    fixed_t		frac;
    fixed_t		fracstep;	 
 
    count = dc_yh - dc_yl; 

    // Zero length, column does not exceed a pixel.
    if (count < 0) 
	return; 
				 
#ifdef RANGECHECK 
    if ((unsigned)dc_x >= SCREENWIDTH
	|| dc_yl < 0
	|| dc_yh >= SCREENHEIGHT) 
	I_Error ("R_DrawColumn: %i to %i at %i", dc_yl, dc_yh, dc_x); 
#endif 

    // Framebuffer destination address.
    // Use ylookup LUT to avoid multiply with ScreenWidth.
    // Use columnofs LUT for subwindows? 
    dest = ylookup[dc_yl] + columnofs[dc_x];  

    // Determine scaling,
    //  which is the only mapping to be done.
    fracstep = dc_iscale; 
    frac = dc_texturemid + (dc_yl-centery)*fracstep;
    if(frac < 0)
    {
	// [kg] wall column wrap-around
	fixed_t tx = dc_src_height << FRACBITS;
	frac += (((-frac) + (tx-1)) / tx) * tx;
    }

    // Inner loop that does the actual texture mapping,
    //  e.g. a DDA-lile scaling.
    // This is as fast as it gets.
    do 
    {
	// Re-map color indices from wall texture column
	//  using a lighting/special effects LUT.
	uint8_t *src = &dc_source[(frac>>FRACBITS) % dc_src_height];
	if(*src) // [kg] transparet pixel
	    *dest = dc_colormap[dc_lightcolor[*src]];
	
	dest += SCREENWIDTH; 
	frac += fracstep;
	
    } while (count--); 
}

// [kg] used for mid walls only
void R_DrawColumnTabled0Masked(void)
{ 
    int			count; 
    byte*		dest; 
    fixed_t		frac;
    fixed_t		fracstep;	 
 
    count = dc_yh - dc_yl; 

    // Zero length, column does not exceed a pixel.
    if (count < 0) 
	return; 
				 
#ifdef RANGECHECK 
    if ((unsigned)dc_x >= SCREENWIDTH
	|| dc_yl < 0
	|| dc_yh >= SCREENHEIGHT) 
	I_Error ("R_DrawColumn: %i to %i at %i", dc_yl, dc_yh, dc_x); 
#endif 

    // Framebuffer destination address.
    // Use ylookup LUT to avoid multiply with ScreenWidth.
    // Use columnofs LUT for subwindows? 
    dest = ylookup[dc_yl] + columnofs[dc_x];  

    // Determine scaling,
    //  which is the only mapping to be done.
    fracstep = dc_iscale; 
    frac = dc_texturemid + (dc_yl-centery)*fracstep;
    if(frac < 0)
    {
	// [kg] wall column wrap-around
	fixed_t tx = dc_src_height << FRACBITS;
	frac += (((-frac) + (tx-1)) / tx) * tx;
    }

    // Inner loop that does the actual texture mapping,
    //  e.g. a DDA-lile scaling.
    // This is as fast as it gets.
    do 
    {
	// Re-map color indices from wall texture column
	//  using a lighting/special effects LUT.
	uint8_t *src = &dc_source[(frac>>FRACBITS) % dc_src_height];
	if(*src) // [kg] transparet pixel
	    *dest = dc_table[dc_colormap[dc_lightcolor[*src]] + *dest * 256];

	dest += SCREENWIDTH; 
	frac += fracstep;
	
    } while (count--); 
}

// [kg] used for mid walls only
void R_DrawColumnTabled1Masked(void)
{ 
    int			count; 
    byte*		dest; 
    fixed_t		frac;
    fixed_t		fracstep;	 
 
    count = dc_yh - dc_yl; 

    // Zero length, column does not exceed a pixel.
    if (count < 0) 
	return; 
				 
#ifdef RANGECHECK 
    if ((unsigned)dc_x >= SCREENWIDTH
	|| dc_yl < 0
	|| dc_yh >= SCREENHEIGHT) 
	I_Error ("R_DrawColumn: %i to %i at %i", dc_yl, dc_yh, dc_x); 
#endif 

    // Framebuffer destination address.
    // Use ylookup LUT to avoid multiply with ScreenWidth.
    // Use columnofs LUT for subwindows? 
    dest = ylookup[dc_yl] + columnofs[dc_x];  

    // Determine scaling,
    //  which is the only mapping to be done.
    fracstep = dc_iscale; 
    frac = dc_texturemid + (dc_yl-centery)*fracstep;
    if(frac < 0)
    {
	// [kg] wall column wrap-around
	fixed_t tx = dc_src_height << FRACBITS;
	frac += (((-frac) + (tx-1)) / tx) * tx;
    }

    // Inner loop that does the actual texture mapping,
    //  e.g. a DDA-lile scaling.
    // This is as fast as it gets.
    do 
    {
	// Re-map color indices from wall texture column
	//  using a lighting/special effects LUT.
	uint8_t *src = &dc_source[(frac>>FRACBITS) % dc_src_height];
	if(*src) // [kg] transparet pixel
	    *dest = dc_table[dc_colormap[dc_lightcolor[*src]] * 256 + *dest];

	dest += SCREENWIDTH; 
	frac += fracstep;
	
    } while (count--); 
}

// [kg] column with holes (skip every pixel)
// sprites only
void R_DrawColumnHoley (void)
{ 
    int			count; 
    byte*		dest; 
    fixed_t		frac;
    fixed_t		fracstep;

    int holestep = dc_holestep;
 
    count = dc_yh - dc_yl; 

    // Zero length, column does not exceed a pixel.
    if (count < 0) 
	return; 
				 
#ifdef RANGECHECK 
    if ((unsigned)dc_x >= SCREENWIDTH
	|| dc_yl < 0
	|| dc_yh >= SCREENHEIGHT) 
	I_Error ("R_DrawColumn: %i to %i at %i", dc_yl, dc_yh, dc_x); 
#endif 

    // Framebuffer destination address.
    // Use ylookup LUT to avoid multiply with ScreenWidth.
    // Use columnofs LUT for subwindows? 
    dest = ylookup[dc_yl] + columnofs[dc_x];  

    // Determine scaling,
    //  which is the only mapping to be done.
    fracstep = dc_iscale; 
    frac = dc_texturemid + (dc_yl-centery)*fracstep;
    if(frac < 0)
	// [kg] sprite column can't start negative
	frac = 0;

    // Inner loop that does the actual texture mapping,
    //  e.g. a DDA-lile scaling.
    // This is as fast as it gets.
    do 
    {
	// Re-map color indices from wall texture column
	//  using a lighting/special effects LUT.
	if((holestep ^ dc_x ^ dc_yl) & 1)
	*dest = dc_colormap[dc_lightcolor[dc_source[(frac>>FRACBITS) % dc_src_height]]];
	
	dest += SCREENWIDTH; 
	frac += fracstep;
	holestep++;
	
    } while (count--); 
}

// [kg] for mid walls
void R_DrawColumnHoleyMasked(void)
{ 
    int			count; 
    byte*		dest; 
    fixed_t		frac;
    fixed_t		fracstep;

    int holestep = dc_holestep;
 
    count = dc_yh - dc_yl; 

    // Zero length, column does not exceed a pixel.
    if (count < 0) 
	return; 
				 
#ifdef RANGECHECK 
    if ((unsigned)dc_x >= SCREENWIDTH
	|| dc_yl < 0
	|| dc_yh >= SCREENHEIGHT) 
	I_Error ("R_DrawColumn: %i to %i at %i", dc_yl, dc_yh, dc_x); 
#endif 

    // Framebuffer destination address.
    // Use ylookup LUT to avoid multiply with ScreenWidth.
    // Use columnofs LUT for subwindows? 
    dest = ylookup[dc_yl] + columnofs[dc_x];  

    // Determine scaling,
    //  which is the only mapping to be done.
    fracstep = dc_iscale; 
    frac = dc_texturemid + (dc_yl-centery)*fracstep;
    if(frac < 0)
    {
	// [kg] wall column wrap-around
	fixed_t tx = dc_src_height << FRACBITS;
	frac += (((-frac) + (tx-1)) / tx) * tx;
    }

    // Inner loop that does the actual texture mapping,
    //  e.g. a DDA-lile scaling.
    // This is as fast as it gets.
    do 
    {
	// Re-map color indices from wall texture column
	//  using a lighting/special effects LUT.
	uint8_t *src = &dc_source[(frac>>FRACBITS) % dc_src_height];
	if(*src && (holestep ^ dc_x ^ dc_yl) & 1)
	    *dest = dc_colormap[dc_lightcolor[*src]];
	
	dest += SCREENWIDTH; 
	frac += fracstep;
	holestep++;
	
    } while (count--); 
}

//
// Spectre/Invisibility.
//

// [kg] fuzz effect is now totally different
// basicaly atempt to "merge" pixels behind
static int dc_fz_top;
static int dc_fz_bot;

// for sprites
void R_DrawFuzzColumn(void)
{ 
    int			count;
    byte*		dest;
    byte*		col;
    byte*		cola;
    int			srcpos;
    fixed_t		frac;
    fixed_t		fracstep;
    uint8_t		val;

    count = dc_yh - dc_yl; 

    // Zero length, column does not exceed a pixel.
    if (count < 0) 
	return;
				 
#ifdef RANGECHECK 
    if ((unsigned)dc_x >= SCREENWIDTH
	|| dc_yl < 0
	|| dc_yh >= SCREENHEIGHT) 
	I_Error ("R_DrawColumn: %i to %i at %i", dc_yl, dc_yh, dc_x); 
#endif 

    // Framebuffer destination address.
    // Use ylookup LUT to avoid multiply with ScreenWidth.
    // Use columnofs LUT for subwindows? 
    dest = ylookup[dc_yl] + columnofs[dc_x];

    // Determine scaling,
    //  which is the only mapping to be done.
    fracstep = dc_iscale; 
    frac = dc_texturemid + (dc_yl-centery)*fracstep;
	// [kg] sprite column can't start negative
	frac = 0;

    if(dc_column != dc_x)
    {
	do
	{
		*dest = dc_fz_buffer[frac >> FRACBITS];
		dest += SCREENWIDTH; 
		frac += fracstep;

	} while (count--);
	dc_fz_buffer += dc_src_height;
	return;
    }

    col = ylookup[dc_yl] + columnofs[dc_column];
    cola = col;
    val = dc_translation[*col];

    srcpos = frac >> FRACBITS;

    // Inner loop that does the actual texture mapping,
    //  e.g. a DDA-lile scaling.
    // This is as fast as it gets.
    while(1)
    {
	int oldpos = srcpos;

	*dest = val;
	dc_fz_buffer[srcpos] = val;

	count--;
	if(!count)
	    break;

	cola += SCREENWIDTH;
	frac += fracstep;
	srcpos = frac >> FRACBITS;
	if(srcpos != oldpos)
	{
		col = cola;
		val = dc_translation[*col];
	}
	dest += SCREENWIDTH;
    }

    dc_fz_buffer += dc_src_height;
}

// for sprites
void R_DrawFuzzColumnTabled0(void)
{ 
    int			count;
    byte*		dest;
    byte*		col;
    byte*		cola;
    int			srcpos;
    fixed_t		frac;
    fixed_t		fracstep;
    uint8_t		val;

    count = dc_yh - dc_yl; 

    // Zero length, column does not exceed a pixel.
    if (count < 0) 
	return;
				 
#ifdef RANGECHECK 
    if ((unsigned)dc_x >= SCREENWIDTH
	|| dc_yl < 0
	|| dc_yh >= SCREENHEIGHT) 
	I_Error ("R_DrawColumn: %i to %i at %i", dc_yl, dc_yh, dc_x); 
#endif 

    // Framebuffer destination address.
    // Use ylookup LUT to avoid multiply with ScreenWidth.
    // Use columnofs LUT for subwindows? 
    dest = ylookup[dc_yl] + columnofs[dc_x];

    // Determine scaling,
    //  which is the only mapping to be done.
    fracstep = dc_iscale; 
    frac = dc_texturemid + (dc_yl-centery)*fracstep;
	// [kg] sprite column can't start negative
	frac = 0;

    if(dc_column != dc_x)
    {
	do
	{
		*dest = dc_fz_buffer[frac >> FRACBITS];
		dest += SCREENWIDTH; 
		frac += fracstep;

	} while (count--);
	dc_fz_buffer += dc_src_height;
	return;
    }

    col = ylookup[dc_yl] + columnofs[dc_column];
    cola = col;
    srcpos = frac >> FRACBITS;
    val = dc_table[dc_colormap[dc_lightcolor[dc_translation[dc_source[srcpos]]]] + *col * 256];

    // Inner loop that does the actual texture mapping,
    //  e.g. a DDA-lile scaling.
    // This is as fast as it gets.
    while(1)
    {
	int oldpos = srcpos;

	*dest = val;
	dc_fz_buffer[srcpos] = val;

	count--;
	if(!count)
	    break;

	cola += SCREENWIDTH;
	frac += fracstep;
	srcpos = frac >> FRACBITS;
	if(srcpos != oldpos)
	{
		col = cola;
		val = dc_table[dc_colormap[dc_lightcolor[dc_translation[dc_source[srcpos]]]] + *col * 256];
	}
	dest += SCREENWIDTH;
    }

    dc_fz_buffer += dc_src_height;
}

// for sprites
void R_DrawFuzzColumnTabled1(void)
{ 
    int			count;
    byte*		dest;
    byte*		col;
    byte*		cola;
    int			srcpos;
    fixed_t		frac;
    fixed_t		fracstep;
    uint8_t		val;

    count = dc_yh - dc_yl; 

    // Zero length, column does not exceed a pixel.
    if (count < 0) 
	return;
				 
#ifdef RANGECHECK 
    if ((unsigned)dc_x >= SCREENWIDTH
	|| dc_yl < 0
	|| dc_yh >= SCREENHEIGHT) 
	I_Error ("R_DrawColumn: %i to %i at %i", dc_yl, dc_yh, dc_x); 
#endif 

    // Framebuffer destination address.
    // Use ylookup LUT to avoid multiply with ScreenWidth.
    // Use columnofs LUT for subwindows? 
    dest = ylookup[dc_yl] + columnofs[dc_x];

    // Determine scaling,
    //  which is the only mapping to be done.
    fracstep = dc_iscale; 
    frac = dc_texturemid + (dc_yl-centery)*fracstep;
	// [kg] sprite column can't start negative
	frac = 0;

    if(dc_column != dc_x)
    {
	do
	{
		*dest = dc_fz_buffer[frac >> FRACBITS];
		dest += SCREENWIDTH; 
		frac += fracstep;

	} while (count--);
	dc_fz_buffer += dc_src_height;
	return;
    }

    col = ylookup[dc_yl] + columnofs[dc_column];
    cola = col;
    srcpos = frac >> FRACBITS;
    val = dc_table[dc_colormap[dc_lightcolor[dc_translation[dc_source[srcpos]]]] * 256 + *col];

    // Inner loop that does the actual texture mapping,
    //  e.g. a DDA-lile scaling.
    // This is as fast as it gets.
    while(1)
    {
	int oldpos = srcpos;

	*dest = val;
	dc_fz_buffer[srcpos] = val;

	count--;
	if(!count)
	    break;

	cola += SCREENWIDTH;
	frac += fracstep;
	srcpos = frac >> FRACBITS;
	if(srcpos != oldpos)
	{
		col = cola;
		val = dc_table[dc_colormap[dc_lightcolor[dc_translation[dc_source[srcpos]]]] * 256 + *col];
	}
	dest += SCREENWIDTH;
    }

    dc_fz_buffer += dc_src_height;
}

// for walls
void R_DrawFuzzColumnMasked(void)
{ 
    int			count;
    byte*		dest;
    byte*		col;
    byte*		cola;
    int			srcpos;
    fixed_t		frac;
    fixed_t		fracstep;
    uint8_t		val;
    int			ppos, newtop;

    count = (dc_yh - dc_yl) + 1;

    // Zero length, column does not exceed a pixel.
    if (count <= 0) 
	return; 
				 
#ifdef RANGECHECK 
    if ((unsigned)dc_x >= SCREENWIDTH
	|| dc_yl < 0
	|| dc_yh >= SCREENHEIGHT) 
	I_Error ("R_DrawColumn: %i to %i at %i", dc_yl, dc_yh, dc_x); 
#endif 

    if(dc_column == dc_x)
    {
	dc_fz_top = sizeof(dc_fz_buf);
	dc_fz_bot = 0;
    }

    // Framebuffer destination address.
    // Use ylookup LUT to avoid multiply with ScreenWidth.
    // Use columnofs LUT for subwindows? 
    dest = ylookup[dc_yl] + columnofs[dc_x];

    // Determine scaling,
    //  which is the only mapping to be done.
    fracstep = dc_iscale; 
    frac = dc_texturemid + (dc_yl-centery)*fracstep;
    if(frac < 0)
	frac = 0;

    // [kg] for new effect
    col = ylookup[dc_yl] + columnofs[dc_column];
    cola = col;
    srcpos = frac >> FRACBITS;
    ppos = srcpos & 1023;
    newtop = ppos;
    val = dc_translation[*col];

    // [kg] top uniquie mapping
    while(ppos < dc_fz_top)
    {
	int oldpos = srcpos;

	if(dc_source[srcpos % dc_src_height]) // [kg] transparet pixel
	    *dest = val;
	dc_fz_buf[ppos] = val;

	dest += SCREENWIDTH;
	cola += SCREENWIDTH;
	frac += fracstep;
	srcpos = frac >> FRACBITS;
	ppos = srcpos & 1023;

	if(srcpos != oldpos)
	{
		col = cola;
		val = dc_translation[*col];
	}

	count--;
	if(!count)
	    goto stop;
    }

    // [kg] copied part
    val = dc_fz_buf[ppos];
    while(ppos <= dc_fz_bot && ppos >= dc_fz_top)
    {
	int oldpos = srcpos;

	if(dc_source[srcpos % dc_src_height]) // [kg] transparet pixel
		*dest = val;

	dest += SCREENWIDTH; 
	frac += fracstep;
	cola += SCREENWIDTH;
	srcpos = frac >> FRACBITS;
	ppos = srcpos & 1023;

	if(srcpos != oldpos)
	{
		col = cola;
		val = dc_fz_buf[ppos];
	}

	count--;
	if(!count)
	    goto stop;
    }

    // [kg] bottom unique mapping
    if(count)
    while(1)
    {
	int oldpos = srcpos;

	if(dc_source[srcpos % dc_src_height]) // [kg] transparet pixel
	    *dest = val;
	dc_fz_buf[ppos] = val;

	dest += SCREENWIDTH;
	cola += SCREENWIDTH;
	frac += fracstep;
	srcpos = frac >> FRACBITS;
	ppos = srcpos & 1023;

	if(srcpos != oldpos)
	{
		col = cola;
		val = dc_translation[*col];
	}

	count--;
	if(!count)
	    goto stop;
    }

stop:
    if(newtop < dc_fz_top)
	dc_fz_top = newtop;
    if(ppos > dc_fz_bot)
	dc_fz_bot = ppos;

}

// for walls
void R_DrawFuzzColumnTabled0Masked(void)
{ 
    int			count;
    byte*		dest;
    byte*		col;
    byte*		cola;
    int			srcpos;
    fixed_t		frac;
    fixed_t		fracstep;
    uint8_t		val;
    int			ppos, newtop;

    count = (dc_yh - dc_yl) + 1;

    // Zero length, column does not exceed a pixel.
    if (count <= 0) 
	return; 
				 
#ifdef RANGECHECK 
    if ((unsigned)dc_x >= SCREENWIDTH
	|| dc_yl < 0
	|| dc_yh >= SCREENHEIGHT) 
	I_Error ("R_DrawColumn: %i to %i at %i", dc_yl, dc_yh, dc_x); 
#endif 

    if(dc_column == dc_x)
    {
	dc_fz_top = sizeof(dc_fz_buf);
	dc_fz_bot = 0;
    }

    // Framebuffer destination address.
    // Use ylookup LUT to avoid multiply with ScreenWidth.
    // Use columnofs LUT for subwindows? 
    dest = ylookup[dc_yl] + columnofs[dc_x];

    // Determine scaling,
    //  which is the only mapping to be done.
    fracstep = dc_iscale; 
    frac = dc_texturemid + (dc_yl-centery)*fracstep;
    if(frac < 0)
    if(frac < 0)
	frac = 0;

    // [kg] for new effect
    col = ylookup[dc_yl] + columnofs[dc_column];
    cola = col;
    srcpos = frac >> FRACBITS;
    ppos = srcpos & 1023;
    newtop = ppos;
    val = dc_table[dc_colormap[dc_lightcolor[dc_source[srcpos]]] + *col * 256];

    // [kg] top uniquie mapping
    while(ppos < dc_fz_top)
    {
	int oldpos = srcpos;

	if(dc_source[srcpos % dc_src_height]) // [kg] transparet pixel
	    *dest = val;
	dc_fz_buf[ppos] = val;

	dest += SCREENWIDTH;
	cola += SCREENWIDTH;
	frac += fracstep;
	srcpos = frac >> FRACBITS;
	ppos = srcpos & 1023;

	if(srcpos != oldpos)
	{
		col = cola;
		val = dc_table[dc_colormap[dc_lightcolor[dc_source[srcpos]]] + *col * 256];
	}

	count--;
	if(!count)
	    goto stop;
    }

    // [kg] copied part
    val = dc_fz_buf[ppos];
    while(ppos <= dc_fz_bot && ppos >= dc_fz_top)
    {
	int oldpos = srcpos;

	if(dc_source[srcpos % dc_src_height]) // [kg] transparet pixel
		*dest = val;

	dest += SCREENWIDTH; 
	frac += fracstep;
	cola += SCREENWIDTH;
	srcpos = frac >> FRACBITS;
	ppos = srcpos & 1023;

	if(srcpos != oldpos)
	{
		col = cola;
		val = dc_fz_buf[ppos];
	}

	count--;
	if(!count)
	    goto stop;
    }

    // [kg] bottom unique mapping
    if(count)
    while(1)
    {
	int oldpos = srcpos;

	if(dc_source[srcpos % dc_src_height]) // [kg] transparet pixel
	    *dest = val;
	dc_fz_buf[ppos] = val;

	dest += SCREENWIDTH;
	cola += SCREENWIDTH;
	frac += fracstep;
	srcpos = frac >> FRACBITS;
	ppos = srcpos & 1023;

	if(srcpos != oldpos)
	{
		col = cola;
		val = dc_table[dc_colormap[dc_lightcolor[dc_source[srcpos]]] + *col * 256];
	}

	count--;
	if(!count)
	    goto stop;
    }

stop:
    if(newtop < dc_fz_top)
	dc_fz_top = newtop;
    if(ppos > dc_fz_bot)
	dc_fz_bot = ppos;

}

// for walls
void R_DrawFuzzColumnTabled1Masked(void)
{ 
    int			count;
    byte*		dest;
    byte*		col;
    byte*		cola;
    int			srcpos;
    fixed_t		frac;
    fixed_t		fracstep;
    uint8_t		val;
    int			ppos, newtop;

    count = (dc_yh - dc_yl) + 1;

    // Zero length, column does not exceed a pixel.
    if (count <= 0) 
	return; 
				 
#ifdef RANGECHECK 
    if ((unsigned)dc_x >= SCREENWIDTH
	|| dc_yl < 0
	|| dc_yh >= SCREENHEIGHT) 
	I_Error ("R_DrawColumn: %i to %i at %i", dc_yl, dc_yh, dc_x); 
#endif 

    if(dc_column == dc_x)
    {
	dc_fz_top = sizeof(dc_fz_buf);
	dc_fz_bot = 0;
    }

    // Framebuffer destination address.
    // Use ylookup LUT to avoid multiply with ScreenWidth.
    // Use columnofs LUT for subwindows? 
    dest = ylookup[dc_yl] + columnofs[dc_x];

    // Determine scaling,
    //  which is the only mapping to be done.
    fracstep = dc_iscale; 
    frac = dc_texturemid + (dc_yl-centery)*fracstep;
    if(frac < 0)
	frac = 0;

    // [kg] for new effect
    col = ylookup[dc_yl] + columnofs[dc_column];
    cola = col;
    srcpos = frac >> FRACBITS;
    ppos = srcpos & 1023;
    newtop = ppos;
    val = dc_table[dc_colormap[dc_lightcolor[dc_source[srcpos]]] * 256 + *col];

    // [kg] top uniquie mapping
    while(ppos < dc_fz_top)
    {
	int oldpos = srcpos;

	if(dc_source[srcpos % dc_src_height]) // [kg] transparet pixel
	    *dest = val;
	dc_fz_buf[ppos] = val;

	dest += SCREENWIDTH;
	cola += SCREENWIDTH;
	frac += fracstep;
	srcpos = frac >> FRACBITS;
	ppos = srcpos & 1023;

	if(srcpos != oldpos)
	{
		col = cola;
		val = dc_table[dc_colormap[dc_lightcolor[dc_source[srcpos]]] * 256 + *col];
	}

	count--;
	if(!count)
	    goto stop;
    }

    // [kg] copied part
    val = dc_fz_buf[ppos];
    while(ppos <= dc_fz_bot && ppos >= dc_fz_top)
    {
	int oldpos = srcpos;

	if(dc_source[srcpos % dc_src_height]) // [kg] transparet pixel
		*dest = val;

	dest += SCREENWIDTH; 
	frac += fracstep;
	cola += SCREENWIDTH;
	srcpos = frac >> FRACBITS;
	ppos = srcpos & 1023;

	if(srcpos != oldpos)
	{
		col = cola;
		val = dc_fz_buf[ppos];
	}

	count--;
	if(!count)
	    goto stop;
    }

    // [kg] bottom unique mapping
    if(count)
    while(1)
    {
	int oldpos = srcpos;

	if(dc_source[srcpos % dc_src_height]) // [kg] transparet pixel
	    *dest = val;
	dc_fz_buf[ppos] = val;

	dest += SCREENWIDTH;
	cola += SCREENWIDTH;
	frac += fracstep;
	srcpos = frac >> FRACBITS;
	ppos = srcpos & 1023;

	if(srcpos != oldpos)
	{
		col = cola;
		val = dc_table[dc_colormap[dc_lightcolor[dc_source[srcpos]]] * 256 + *col];
	}

	count--;
	if(!count)
	    goto stop;
    }

stop:
    if(newtop < dc_fz_top)
	dc_fz_top = newtop;
    if(ppos > dc_fz_bot)
	dc_fz_bot = ppos;

}

//
// R_DrawTranslatedColumn
// Used to draw player sprites
//  with the green colorramp mapped to others.
// Could be used with different translation
//  tables, e.g. the lighter colored version
//  of the BaronOfHell, the HellKnight, uses
//  identical sprites, kinda brightened up.
//
// [kg] sprites and solid walls
void R_DrawTranslatedColumn (void)
{ 
    int			count; 
    byte*		dest; 
    fixed_t		frac;
    fixed_t		fracstep;	 
 
    count = dc_yh - dc_yl; 
    if (count < 0) 
	return; 
				 
#ifdef RANGECHECK 
    if ((unsigned)dc_x >= SCREENWIDTH
	|| dc_yl < 0
	|| dc_yh >= SCREENHEIGHT)
    {
	I_Error ( "R_DrawColumn: %i to %i at %i",
		  dc_yl, dc_yh, dc_x);
    }
    
#endif 
    
    dest = ylookup[dc_yl] + columnofs[dc_x]; 

    // Looks familiar.
    fracstep = dc_iscale; 
    frac = dc_texturemid + (dc_yl-centery)*fracstep;
    if(frac < 0)
    {
	// [kg] fix offset
	if(dc_is_sprite)
		// sprite column can't start negative
		frac = 0;
	else
	{
		// wall column wrap-around
		fixed_t tx = dc_src_height << FRACBITS;
		frac += ((-frac + (tx-1)) / tx) * tx;
	}
    }

    // Here we do an additional index re-mapping.
    do 
    {
	// Translation tables are used
	//  to map certain colorramps to other ones,
	//  used with PLAY sprites.
	// Thus the "green" ramp of the player 0 sprite
	//  is mapped to gray, red, black/indigo. 
	*dest = dc_colormap[dc_lightcolor[dc_translation[dc_source[(frac>>FRACBITS) % dc_src_height]]]];
	dest += SCREENWIDTH;
	
	frac += fracstep; 
    } while (count--); 
}

// [kg] mid walls only
/*void R_DrawTranslatedColumnMasked (void)
{ 
    int			count; 
    byte*		dest; 
    fixed_t		frac;
    fixed_t		fracstep;	 
 
    count = dc_yh - dc_yl; 
    if (count < 0) 
	return; 
				 
#ifdef RANGECHECK 
    if ((unsigned)dc_x >= SCREENWIDTH
	|| dc_yl < 0
	|| dc_yh >= SCREENHEIGHT)
    {
	I_Error ( "R_DrawColumn: %i to %i at %i",
		  dc_yl, dc_yh, dc_x);
    }
    
#endif 
    
    dest = ylookup[dc_yl] + columnofs[dc_x]; 

    // Looks familiar.
    fracstep = dc_iscale; 
    frac = dc_texturemid + (dc_yl-centery)*fracstep;
    if(frac < 0)
    {
	// [kg] wall column wrap-around
	fixed_t tx = dc_src_height << FRACBITS;
	frac += (((-frac) + (tx-1)) / tx) * tx;
    }

    // Here we do an additional index re-mapping.
    do 
    {
	// Translation tables are used
	//  to map certain colorramps to other ones,
	//  used with PLAY sprites.
	// Thus the "green" ramp of the player 0 sprite
	//  is mapped to gray, red, black/indigo.
	uint8_t *src = &dc_source[(frac>>FRACBITS) % dc_src_height];
	if(*src) // [kg] transparet pixel
	    *dest = dc_colormap[dc_lightcolor[dc_translation[*src]]];

	dest += SCREENWIDTH;
	frac += fracstep; 
    } while (count--); 
} */

//
// R_DrawSpan 
// With DOOM style restrictions on view orientation,
//  the floors and ceilings consist of horizontal slices
//  or spans with constant z depth.
// However, rotation around the world z axis is possible,
//  thus this mapping, while simpler and faster than
//  perspective correct texture mapping, has to traverse
//  the texture at an angle in all but a few cases.
// In consequence, flats are not stored by column (like walls),
//  and the inner loop has to step in texture space u and v.
//
int			ds_y; 
int			ds_x1; 
int			ds_x2;

fixed_t			ds_xfrac; 
fixed_t			ds_yfrac; 
fixed_t			ds_xstep; 
fixed_t			ds_ystep;

// just for profiling
int			dscount;


//
// Draws the actual span.
void R_DrawSpan (void)
{ 
    uint32_t		xfrac;
    uint32_t		yfrac;
    byte*		dest;
    int			count;
    int			spot;
	 
#ifdef RANGECHECK 
    if (ds_x2 < ds_x1
	|| ds_x1<0
	|| ds_x2>=SCREENWIDTH  
	|| (unsigned)ds_y>SCREENHEIGHT)
    {
	I_Error( "R_DrawSpan: %i to %i at %i",
		 ds_x1,ds_x2,ds_y);
    }
//	dscount++; 
#endif 

    xfrac = dc_src_height << FRACBITS;
    yfrac = dc_src_width << FRACBITS;
    xfrac = ((0x80000000 / xfrac)*xfrac) + ds_xfrac;
    yfrac = ((0x80000000 / yfrac)*yfrac) + ds_yfrac;

    dest = ylookup[ds_y] + columnofs[ds_x1];

    // We do not check for zero spans here?
    count = ds_x2 - ds_x1; 

    do 
    {
	// Current texture index in u,v.
//	spot = ((yfrac>>(16-6))&(63*64)) + ((xfrac>>16)&63);
	spot = (((xfrac>>16) % dc_src_height) * dc_src_width) + ((yfrac>>16) % dc_src_width);

	// Lookup pixel from flat texture tile,
	//  re-index using light/colormap.
	*dest++ = dc_colormap[dc_lightcolor[dc_source[spot]]];

	// Next step in u,v.
	xfrac += ds_xstep; 
	yfrac += ds_ystep;
	
    } while (count--); 
}

void R_DrawSpanMasked(void)
{ 
    uint32_t		xfrac;
    uint32_t		yfrac; 
    byte*		dest; 
    int			count;
    int			spot; 
	 
#ifdef RANGECHECK 
    if (ds_x2 < ds_x1
	|| ds_x1<0
	|| ds_x2>=SCREENWIDTH  
	|| (unsigned)ds_y>SCREENHEIGHT)
    {
	I_Error( "R_DrawSpan: %i to %i at %i",
		 ds_x1,ds_x2,ds_y);
    }
//	dscount++; 
#endif 

    
    xfrac = dc_src_height << FRACBITS;
    yfrac = dc_src_width << FRACBITS;
    xfrac = ((0x80000000 / xfrac)*xfrac) + ds_xfrac;
    yfrac = ((0x80000000 / yfrac)*yfrac) + ds_yfrac;

    dest = ylookup[ds_y] + columnofs[ds_x1];

    // We do not check for zero spans here?
    count = ds_x2 - ds_x1; 

    do 
    {
	// Current texture index in u,v.
//	spot = ((yfrac>>(16-6))&(63*64)) + ((xfrac>>16)&63);
	spot = (((xfrac>>16) % dc_src_height) * dc_src_width) + ((yfrac>>16) % dc_src_width);

	// Lookup pixel from flat texture tile,
	//  re-index using light/colormap.
	if(dc_source[spot])
	    *dest = dc_colormap[dc_lightcolor[dc_source[spot]]];
	dest++;

	// Next step in u,v.
	xfrac += ds_xstep; 
	yfrac += ds_ystep;
	
    } while (count--); 
}

// [kg] with table
void R_DrawSpanTabled0Masked(void)
{ 
    uint32_t		xfrac;
    uint32_t		yfrac; 
    byte*		dest; 
    int			count;
    int			spot; 
	 
#ifdef RANGECHECK 
    if (ds_x2 < ds_x1
	|| ds_x1<0
	|| ds_x2>=SCREENWIDTH  
	|| (unsigned)ds_y>SCREENHEIGHT)
    {
	I_Error( "R_DrawSpan: %i to %i at %i",
		 ds_x1,ds_x2,ds_y);
    }
//	dscount++; 
#endif 

    
    xfrac = dc_src_height << FRACBITS;
    yfrac = dc_src_width << FRACBITS;
    xfrac = ((0x80000000 / xfrac)*xfrac) + ds_xfrac;
    yfrac = ((0x80000000 / yfrac)*yfrac) + ds_yfrac;

    dest = ylookup[ds_y] + columnofs[ds_x1];

    // We do not check for zero spans here?
    count = ds_x2 - ds_x1; 

    do 
    {
	// Current texture index in u,v.
//	spot = ((yfrac>>(16-6))&(63*64)) + ((xfrac>>16)&63);
	spot = (((xfrac>>16) % dc_src_height) * dc_src_width) + ((yfrac>>16) % dc_src_width);

	// Lookup pixel from flat texture tile,
	//  re-index using light/colormap.
	if(dc_source[spot])
	    *dest = dc_table[dc_colormap[dc_lightcolor[dc_source[spot]]] + *dest * 256];
	dest++;

	// Next step in u,v.
	xfrac += ds_xstep; 
	yfrac += ds_ystep;
	
    } while (count--); 
}

// [kg] with table
void R_DrawSpanTabled1Masked(void)
{ 
    uint32_t		xfrac;
    uint32_t		yfrac; 
    byte*		dest; 
    int			count;
    int			spot; 
	 
#ifdef RANGECHECK 
    if (ds_x2 < ds_x1
	|| ds_x1<0
	|| ds_x2>=SCREENWIDTH  
	|| (unsigned)ds_y>SCREENHEIGHT)
    {
	I_Error( "R_DrawSpan: %i to %i at %i",
		 ds_x1,ds_x2,ds_y);
    }
//	dscount++; 
#endif 

    
    xfrac = dc_src_height << FRACBITS;
    yfrac = dc_src_width << FRACBITS;
    xfrac = ((0x80000000 / xfrac)*xfrac) + ds_xfrac;
    yfrac = ((0x80000000 / yfrac)*yfrac) + ds_yfrac;

    dest = ylookup[ds_y] + columnofs[ds_x1];

    // We do not check for zero spans here?
    count = ds_x2 - ds_x1; 

    do 
    {
	// Current texture index in u,v.
//	spot = ((yfrac>>(16-6))&(63*64)) + ((xfrac>>16)&63);
	spot = (((xfrac>>16) % dc_src_height) * dc_src_width) + ((yfrac>>16) % dc_src_width);

	// Lookup pixel from flat texture tile,
	//  re-index using light/colormap.
	if(dc_source[spot])
	    *dest = dc_table[dc_colormap[dc_lightcolor[dc_source[spot]]] * 256 + *dest];
	dest++;

	// Next step in u,v.
	xfrac += ds_xstep; 
	yfrac += ds_ystep;
	
    } while (count--); 
}

// [kg] draw fuzz span
void R_DrawSpanFuzz (void)
{ 
    uint32_t		xfrac;
    uint32_t		yfrac; 
    byte*		dest; 
    int			count;
    int			spot;
    int		oldspot;
    uint8_t	val;
	 
#ifdef RANGECHECK 
    if (ds_x2 < ds_x1
	|| ds_x1<0
	|| ds_x2>=SCREENWIDTH  
	|| (unsigned)ds_y>SCREENHEIGHT)
    {
	I_Error( "R_DrawSpan: %i to %i at %i",
		 ds_x1,ds_x2,ds_y);
    }
//	dscount++; 
#endif 

    
    xfrac = dc_src_height << FRACBITS;
    yfrac = dc_src_width << FRACBITS;
    xfrac = ((0x80000000 / xfrac)*xfrac) + ds_xfrac;
    yfrac = ((0x80000000 / yfrac)*yfrac) + ds_yfrac;

    dest = ylookup[ds_y] + columnofs[ds_x1];

    // We do not check for zero spans here?
    count = ds_x2 - ds_x1;

    oldspot = -1;

    do 
    {
	// Current texture index in u,v.
	spot = (((xfrac>>16) % dc_src_height) * dc_src_width) + ((yfrac>>16) % dc_src_width);

	if(spot != oldspot)
	{
	    oldspot = spot;
	    val = dc_translation[*dest];
	}

	if(dc_source[spot])
	    *dest = val;
	dest++;

	// Next step in u,v.
	xfrac += ds_xstep; 
	yfrac += ds_ystep;

    } while (count--); 
}

// [kg] draw fuzz with table
void R_DrawSpanTabled0Fuzz (void)
{ 
    uint32_t		xfrac;
    uint32_t		yfrac; 
    byte*		dest; 
    int			count;
    int			spot;
    int		oldspot;
    uint8_t	val;
	 
#ifdef RANGECHECK 
    if (ds_x2 < ds_x1
	|| ds_x1<0
	|| ds_x2>=SCREENWIDTH  
	|| (unsigned)ds_y>SCREENHEIGHT)
    {
	I_Error( "R_DrawSpan: %i to %i at %i",
		 ds_x1,ds_x2,ds_y);
    }
//	dscount++; 
#endif 

    
    xfrac = dc_src_height << FRACBITS;
    yfrac = dc_src_width << FRACBITS;
    xfrac = ((0x80000000 / xfrac)*xfrac) + ds_xfrac;
    yfrac = ((0x80000000 / yfrac)*yfrac) + ds_yfrac;

    dest = ylookup[ds_y] + columnofs[ds_x1];

    // We do not check for zero spans here?
    count = ds_x2 - ds_x1;

    oldspot = -1;

    do 
    {
	// Current texture index in u,v.
	spot = (((xfrac>>16) % dc_src_height) * dc_src_width) + ((yfrac>>16) % dc_src_width);

	if(spot != oldspot)
	{
	    oldspot = spot;
	    val = dc_table[dc_colormap[dc_lightcolor[dc_source[spot]]] + *dest * 256];
	}

	if(dc_source[spot])
	    *dest = val;
	dest++;

	// Next step in u,v.
	xfrac += ds_xstep; 
	yfrac += ds_ystep;

    } while (count--); 
}

// [kg] draw fuzz with table
void R_DrawSpanTabled1Fuzz (void)
{ 
    uint32_t		xfrac;
    uint32_t		yfrac; 
    byte*		dest; 
    int			count;
    int			spot;
    int		oldspot;
    uint8_t	val;
	 
#ifdef RANGECHECK 
    if (ds_x2 < ds_x1
	|| ds_x1<0
	|| ds_x2>=SCREENWIDTH  
	|| (unsigned)ds_y>SCREENHEIGHT)
    {
	I_Error( "R_DrawSpan: %i to %i at %i",
		 ds_x1,ds_x2,ds_y);
    }
//	dscount++; 
#endif 

    
    xfrac = dc_src_height << FRACBITS;
    yfrac = dc_src_width << FRACBITS;
    xfrac = ((0x80000000 / xfrac)*xfrac) + ds_xfrac;
    yfrac = ((0x80000000 / yfrac)*yfrac) + ds_yfrac;

    dest = ylookup[ds_y] + columnofs[ds_x1];

    // We do not check for zero spans here?
    count = ds_x2 - ds_x1;

    oldspot = -1;

    do 
    {
	// Current texture index in u,v.
	spot = (((xfrac>>16) % dc_src_height) * dc_src_width) + ((yfrac>>16) % dc_src_width);

	if(spot != oldspot)
	{
	    oldspot = spot;
	    val = dc_table[dc_colormap[dc_lightcolor[dc_source[spot]]] * 256 + *dest];
	}

	if(dc_source[spot])
	    *dest = val;
	dest++;

	// Next step in u,v.
	xfrac += ds_xstep; 
	yfrac += ds_ystep;

    } while (count--); 
}

//
// R_InitBuffer 
// Creats lookup tables that avoid
//  multiplies and other hazzles
//  for getting the framebuffer address
//  of a pixel to draw.
//
void
R_InitBuffer
( int		width,
  int		height ) 
{ 
    int		i; 

    // Handle resize,
    //  e.g. smaller view windows
    //  with border and/or status bar.
    viewwindowx = (SCREENWIDTH-width) >> 1; 

    // Column offset. For windows.
    for (i=0 ; i<width ; i++) 
	columnofs[i] = viewwindowx + i;

    // Samw with base row offset.
    if (width == SCREENWIDTH) 
	viewwindowy = 0; 
    else 
	viewwindowy = (SCREENHEIGHT-SBARHEIGHT-height) >> 1; 

    // Preclaculate all row offsets.
    for (i=0 ; i<height ; i++)
	ylookup[i] = screens[0] + (i+viewwindowy)*SCREENWIDTH; 
} 


//
// [kg] setup correct rendering function
//

void R_SetupRenderFunc(int style, void *table, void *translation)
{
	// TODO: translation for effects (holey)
	switch(style)
	{
		case RENDER_FUZZ:
			dc_translation = table;
			colfunc = R_DrawFuzzColumn;
		break;
		case RENDER_FUZZ_TABLE:
			if(translation)
				dc_translation = translation;
			else
				dc_translation = colormaps;
			dc_table = table;
			colfunc = R_DrawFuzzColumnTabled0;
		break;
		case RENDER_FUZZ_TABLEI:
			if(translation)
				dc_translation = translation;
			else
				dc_translation = colormaps;
			dc_table = table;
			colfunc = R_DrawFuzzColumnTabled1;
		break;
		case RENDER_HOLEY0:
			colfunc = R_DrawColumnHoley;
			dc_holestep = 0;
		break;
		case RENDER_HOLEY1:
			colfunc = R_DrawColumnHoley;
			dc_holestep = 1;
		break;
		case RENDER_TABLE:
			if(translation)
				dc_translation = translation;
			else
				dc_translation = colormaps;
			dc_table = table;
			colfunc = R_DrawColumnTabled0;
		break;
		case RENDER_TABLEI:
			if(translation)
				dc_translation = translation;
			else
				dc_translation = colormaps;
			dc_table = table;
			colfunc = R_DrawColumnTabled1;
		break;
		default:
			if(translation)
			{
				dc_translation = translation;
				colfunc = R_DrawTranslatedColumn;
			} else
				colfunc = R_DrawColumn;
		break;
	}
}

void R_SetupRenderFuncWall(int style, void *table, void *translation)
{
	switch(style)
	{
		case RENDER_FUZZ:
			dc_translation = table;
			colfunc = R_DrawFuzzColumnMasked;
		break;
		case RENDER_FUZZ_TABLE:
			dc_table = table;
			colfunc = R_DrawFuzzColumnTabled0Masked;
		break;
		case RENDER_FUZZ_TABLEI:
			dc_table = table;
			colfunc = R_DrawFuzzColumnTabled1Masked;
		break;
		case RENDER_HOLEY0:
			colfunc = R_DrawColumnHoleyMasked;
			dc_holestep = 0;
		break;
		case RENDER_HOLEY1:
			colfunc = R_DrawColumnHoleyMasked;
			dc_holestep = 1;
		break;
		case RENDER_TABLE:
			dc_table = table;
			colfunc = R_DrawColumnTabled0Masked;
		break;
		case RENDER_TABLEI:
			dc_table = table;
			colfunc = R_DrawColumnTabled1Masked;
		break;
		default:
			colfunc = R_DrawColumnMasked;
		break;
	}
}

void R_SetupRenderFuncSpan(int style, void *table, void *translation, boolean masked)
{
	// TODO: holey & translated rendering
	switch(style)
	{
		case RENDER_FUZZ:
			dc_translation = table;
			spanfunc = R_DrawSpanFuzz;
		break;
		case RENDER_HOLEY0:
			dc_holestep = 0;
			spanfunc = R_DrawSpan; // TODO
		break;
		case RENDER_HOLEY1:
			dc_holestep = 1;
			spanfunc = R_DrawSpan; // TODO
		break;
		case RENDER_TABLE:
			dc_table = table;
			spanfunc = R_DrawSpanTabled0Masked;
		break;
		case RENDER_TABLEI:
			dc_table = table;
			spanfunc = R_DrawSpanTabled1Masked;
		break;
		case RENDER_FUZZ_TABLE:
			dc_table = table;
			spanfunc = R_DrawSpanTabled0Fuzz;
		break;
		case RENDER_FUZZ_TABLEI:
			dc_table = table;
			spanfunc = R_DrawSpanTabled1Fuzz;
		break;
		default:
			// TODO dc_translation
			if(masked)
				spanfunc = R_DrawSpanMasked;
			else
				spanfunc = R_DrawSpan;
		break;
	}
}

