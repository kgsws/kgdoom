#include <stdio.h>
#include <stdlib.h>


#include "doomdef.h"
#include "m_swap.h"

#include "i_system.h"
#include "z_zone.h"
#include "w_wad.h"

#include "r_local.h"

#include "doomstat.h"

#include "p_local.h"
#include "kg_3dfloor.h"

#define MINZ				(FRACUNIT*4)
#define BASEYCENTER			109

//void R_DrawColumn (void);
//void R_DrawFuzzColumn (void);



typedef struct
{
    int		x1;
    int		x2;
	
    int		column;
    int		topclip;
    int		bottomclip;

} maskdraw_t;



//
// Sprite rotation 0 is facing the viewer,
//  rotation 1 is one angle turn CLOCKWISE around the axis.
// This is not the same as the angle,
//  which increases counter clockwise (protractor).
// There was a lot of stuff grabbed wrong, so I changed it...
//
fixed_t		pspritescale;
fixed_t		pspriteiscale;

lighttable_t**	spritelights;

// constant arrays
//  used for psprite clipping and initializing clipping
short		negonearray[SCREENWIDTH];
short		screenheightarray[SCREENWIDTH];


//
// INITIALIZATION FUNCTIONS
//

// variables used to look up
//  and range check thing_t sprites patches
spritedef_t*	sprites;
int		numsprites;

spriteframe_t	sprtemp[29];
int		maxframe;
char*		spritename;

// [kg] 3D floor clipping
int height_count;
fixed_t height_top;
fixed_t height_bot;
int clip_top;
int clip_bot;
extern fixed_t rw_scale;

//
// R_InstallSpriteLump
// Local function for R_InitSprites.
//
void
R_InstallSpriteLump
( int		lump,
  unsigned	frame,
  unsigned	rotation,
  boolean	flipped )
{
    int		r;
	
    if (frame >= 29 || rotation > 8)
	I_Error("R_InstallSpriteLump: "
		"Bad frame characters in lump %i", lump);
	
    if ((int)frame > maxframe)
	maxframe = frame;
		
    if (rotation == 0)
    {
	// the lump should be used for all rotations
/*	if (sprtemp[frame].rotate == false)
	{
		printf("R_InitSprites: Sprite %.4s frame %c has multip rot=0 lump\n", spritename, 'A'+frame);
		return;
	}

	if (sprtemp[frame].rotate == true)
	{
		printf("R_InitSprites: Sprite %.4s frame %c has rotations and a rot=0 lump\n", spritename, 'A'+frame);
		return;
	}
*/
	sprtemp[frame].rotate = false;
	for (r=0 ; r<8 ; r++)
	{
	    sprtemp[frame].lump[r] = lump;
	    sprtemp[frame].flip[r] = (byte)flipped;
	}
	return;
    }
	
    // the lump is only used for one rotation
/*    if (sprtemp[frame].rotate == false)
	{
		printf("R_InitSprites: Sprite %.4s frame %c has rotations and a rot=0 lump\n", spritename, 'A'+frame);
		return;
	}
*/
    sprtemp[frame].rotate = true;

    // make 0 based
    rotation--;		
/*    if (sprtemp[frame].lump[rotation] != -1)
	I_Error ("R_InitSprites: Sprite %.4s : %c : %c "
		 "has two lumps mapped to it",
		 spritename, 'A'+frame, '1'+rotation);
*/
    sprtemp[frame].lump[rotation] = lump;
    sprtemp[frame].flip[rotation] = (byte)flipped;
}




//
// R_InitSpriteDefs
// Pass a null terminated list of sprite names
//  (4 chars exactly) to be used.
// Builds the sprite rotation matrixes to account
//  for horizontally flipped sprites.
// Will report an error if the lumps are inconsistant. 
// Only called at startup.
//
// Sprite lump names are 4 characters for the actor,
//  a letter for the frame, and a number for the rotation.
// A sprite that is flippable will have an additional
//  letter/number appended.
// The rotation character can be 0 to signify no rotations.
//
void R_InitSpriteDefs()
{ 
    int		i;
    int		l;
    int		intname;
    int		frame;
    int		rotation;
    int		patched;
    int wnum;
		
    numsprites = numsnames;

    if(!numsprites)
	I_Error("R_InitSprites: there are no sprites");

    sprites = Z_Malloc(numsprites * sizeof(spritedef_t), PU_STATIC, NULL);

    // scan all the lump names for each of the names,
    //  noting the highest frame letter.
    // Just compare 4 characters as ints
    for (i=0 ; i<numsprites ; i++)
    {
	spritename = sprnames[i].t;
	memset (sprtemp,-1, sizeof(sprtemp));
		
	maxframe = -1;
	
	// scan the lumps [kg] from multiple wads
	for(wnum = 0; wnum < MAXWADS; wnum++)
	if(firstspritelump[wnum] > 0 && lastspritelump[wnum] > 0)
	//  filling in the frames for whatever is found
	for (l = firstspritelump[wnum]; l <= lastspritelump[wnum]; l++)
	{
	    patched = (wnum << 24) | l;
//	    if (*(int *)lumpinfo[l].name == intname)
	    if(W_LumpCheckSprite(patched, sprnames[i].t))
	    {
		char *name = W_LumpNumName(patched);

		frame = name[4] - 'A';
		rotation = name[5] - '0';

		R_InstallSpriteLump (patched, frame, rotation, false);

		if (name[6])
		{
		    frame = name[6] - 'A';
		    rotation = name[7] - '0';
		    R_InstallSpriteLump (patched, frame, rotation, true);
		}
	    }
	}
	
	// check the frames that were found for completeness
	if (maxframe == -1)
	{
	    sprites[i].numframes = 0;
	    continue;
	}
		
	maxframe++;
	
	for (frame = 0 ; frame < maxframe ; frame++)
	{
	    switch ((int)sprtemp[frame].rotate)
	    {
	      case -1:
		// no rotations were found for that frame at all
		printf("R_InitSprites: No patches found for %.4s frame %c\n", sprnames[i].t, frame+'A');
		break;
		
	      case 0:
		// only the first rotation is needed
		break;
			
	      case 1:
		// must have all 8 frames
		for (rotation=0 ; rotation<8 ; rotation++)
		    if (sprtemp[frame].lump[rotation] == -1)
		    {
//			I_Error ("R_InitSprites: Sprite %.4s frame %c is missing rotations", sprnames[i], frame+'A');
			printf("R_InitSprites: Sprite %.4s frame %c is missing rotation %i\n", sprnames[i].t, frame+'A', rotation);
		    }
		break;
	    }
	}

	// allocate space for the frames present and copy sprtemp to it
	sprites[i].numframes = maxframe;
	sprites[i].spriteframes = 
	    Z_Malloc (maxframe * sizeof(spriteframe_t), PU_STATIC, NULL);
	memcpy (sprites[i].spriteframes, sprtemp, maxframe*sizeof(spriteframe_t));
    }

}


#ifndef SERVER

//
// GAME FUNCTIONS
//
vissprite_t	vissprites[MAXVISSPRITES];
vissprite_t*	vissprite_p;
int		newvissprite;



//
// R_InitSprites
// Called at program start.
//
void R_InitSprites()
{
    int		i;
	
    for (i=0 ; i<SCREENWIDTH ; i++)
    {
	negonearray[i] = -1;
    }
	
    R_InitSpriteDefs();
}



//
// R_ClearSprites
// Called at frame start.
//
void R_ClearSprites (void)
{
    vissprite_p = vissprites;
}


//
// R_NewVisSprite
//
vissprite_t	overflowsprite;

vissprite_t* R_NewVisSprite (void)
{
    if (vissprite_p == &vissprites[MAXVISSPRITES])
	return &overflowsprite;
    
    vissprite_p++;
    return vissprite_p-1;
}



//
// R_DrawMaskedColumn
// Used for sprites and masked mid textures.
// Masked means: partly transparent, i.e. stored
//  in posts/runs of opaque pixels.
//
short*		mfloorclip;
short*		mceilingclip;

fixed_t		spryscale;
fixed_t		sprtopscreen;

void R_DrawMaskedColumn (column_t* column)
{
    int		topscreen;
    int 	bottomscreen;
    fixed_t	basetexturemid;
	
    basetexturemid = dc_texturemid;
	
    for ( ; column->topdelta != 0xff ; ) 
    {
	// calculate unclipped screen coordinates
	//  for post
	topscreen = sprtopscreen + spryscale*column->topdelta;
	bottomscreen = topscreen + spryscale*column->length;

	dc_yl = (topscreen+FRACUNIT-1)>>FRACBITS;
	dc_yh = (bottomscreen-1)>>FRACBITS;

	if (dc_yh >= mfloorclip[dc_x])
	    dc_yh = mfloorclip[dc_x]-1;

	if (dc_yl <= mceilingclip[dc_x])
	    dc_yl = mceilingclip[dc_x]+1;

	if(clip_bot >= 0 && dc_yh >= clip_bot)
	    dc_yh = clip_bot;

	if(clip_top >= 0 && dc_yl <= clip_top)
	    dc_yl = clip_top;

	if (dc_yl <= dc_yh)
	{
	    dc_source = (byte *)column + 3;
	    dc_texturemid = basetexturemid - (column->topdelta<<FRACBITS);
	    // dc_source = (byte *)column + 3 - column->topdelta;

	    // Drawn by either R_DrawColumn
	    //  or (SHADOW) R_DrawFuzzColumn.
	    colfunc ();	
	}
	column = (column_t *)(  (byte *)column + column->length + 4);
    }
	
    dc_texturemid = basetexturemid;
}

//
// R_DrawVisSprite
//  mfloorclip and mceilingclip should also be set.
//
void
R_DrawVisSprite
( vissprite_t*		vis,
  int			x1,
  int			x2 )
{
    column_t*		column;
    int			texturecolumn;
    fixed_t		frac;
    patch_t*		patch;
    boolean bot_clip = true;

    // TODO: translucent floors
    if(vis->mo)
    {
	if(height_top < viewz && vis->mo->z >= height_top)
	{
	    vis->was_hidden = true;
	    return;
	}
	if(vis->was_hidden)
	{
	    vis->was_hidden = false;
	    bot_clip = false;
	}
    }

    patch = W_CacheLumpNum (vis->patch);

    if(vis->mo)
    {
	// [kg] custom colormap; first
	if(vis->translation)
		dc_colormap = vis->mo->colormap.data;
	else
	if(fixedcolormap)
	{
		// forced colormap
		dc_colormap = fixedcolormap;
		dc_lightcolor = colormaps;
	} else
	if(vis->mo->frame & FF_FULLBRIGHT)
	{
		// full bright
		dc_colormap = colormaps;
		dc_lightcolor = colormaps;
	} else
	{
		// diminished light
		extraplane_t *pl;
		int index = vis->scale >> (LIGHTSCALESHIFT);
		int lightnum = (vis->mo->subsector->sector->lightlevel >> LIGHTSEGSHIFT)+extralight;

		dc_lightcolor = vis->mo->subsector->sector->colormap.data;

		if(index >= MAXLIGHTSCALE) 
			index = MAXLIGHTSCALE-1;

		// [kg] get correct light
		if(height_top != ONCEILINGZ)
		{
			pl = vis->mo->subsector->sector->exfloor;
			while(pl)
			{
				if(height_top <= *pl->height)
				{
					lightnum = (*pl->lightlevel >> LIGHTSEGSHIFT)+extralight;
					dc_lightcolor = pl->source->colormap.data;
					break;
				}
				pl = pl->next;
			}
		}

		if (lightnum < 0)
			spritelights = scalelight[0];
		else if (lightnum >= LIGHTLEVELS)
			spritelights = scalelight[LIGHTLEVELS-1];
		else
			spritelights = scalelight[lightnum];
		dc_colormap = spritelights[index];
	}

	if(vis->mo->flags & MF_HOLEY)
		colfunc = R_DrawColumnHoley;
	if(vis->mo->flags & MF_SHADOW)
		colfunc = fuzzcolfunc;

	if(vis->translation)
	{
		colfunc = R_DrawTranslatedColumn;
		dc_translation = vis->translation;
	}
    } else
    {
	// player sprite
	if(fixedcolormap)
		dc_colormap = fixedcolormap;
	else
		colfunc = fuzzcolfunc;
    }

    dc_iscale = abs(vis->xiscale);
    dc_texturemid = vis->texturemid;
    frac = vis->startfrac;
    spryscale = vis->scale;
    sprtopscreen = centeryfrac - FixedMul(dc_texturemid,spryscale);

    clip_bot = -1;
    clip_top = -1;
    if(!vis->mo)
    {
	dc_texturemid += FixedMul(((centery-viewheight/2)<<FRACBITS), vis->xiscale);
	sprtopscreen += (viewheight/2-centery)<<FRACBITS;
    } else
    {
	if(height_bot != ONFLOORZ && bot_clip)
	{
	    clip_bot = (centeryfrac - FixedMul(height_bot - viewz, spryscale)) / FRACUNIT;
	    if(clip_bot < 0)
		return;
	}
	if(height_top != ONCEILINGZ)
	{
	    clip_top = ((centeryfrac - FixedMul(height_top - viewz, spryscale)) / FRACUNIT) - 1;
	    if(clip_top >= SCREENHEIGHT)
		return;
	}
    }

    for (dc_x=vis->x1 ; dc_x<=vis->x2 ; dc_x++, frac += vis->xiscale)
    {
	texturecolumn = frac>>FRACBITS;
#ifdef RANGECHECK
	if (texturecolumn < 0 || texturecolumn >= SHORT(patch->width))
	    I_Error ("R_DrawSpriteRange: bad texturecolumn");
//	break;
#endif
	column = (column_t *) ((byte *)patch +
			       LONG(patch->columnofs[texturecolumn]));
	R_DrawMaskedColumn (column);
    }

    colfunc = R_DrawColumn;
}

// [kg] get sprite values
static int R_GetSpriteWidth(int lump)
{
	patch_t	*patch;
	patch = W_CacheLumpNum(lump);
	return SHORT(patch->width)<<FRACBITS;
}

static int R_GetSpriteLeftOffset(int lump)
{
	patch_t	*patch;
	patch = W_CacheLumpNum(lump);
	return SHORT(patch->leftoffset)<<FRACBITS;
}

static int R_GetSpriteTopOffset(int lump)
{
	patch_t	*patch;
	patch = W_CacheLumpNum(lump);
	return SHORT(patch->topoffset)<<FRACBITS;
}

//
// R_ProjectSprite
// Generates a vissprite for a thing
//  if it might be visible.
//
void R_ProjectSprite (mobj_t* thing)
{
    fixed_t		tr_x;
    fixed_t		tr_y;
    
    fixed_t		gxt;
    fixed_t		gyt;
    
    fixed_t		tx;
    fixed_t		tz;

    fixed_t		xscale;
    
    int			x1;
    int			x2;

    spritedef_t*	sprdef;
    spriteframe_t*	sprframe;
    int			lump;
    
    unsigned		rot;
    boolean		flip;
    
    vissprite_t*	vis;
    
    angle_t		ang;
    fixed_t		iscale;
    
    // transform the origin point
    tr_x = thing->x - viewx;
    tr_y = thing->y - viewy;
	
    gxt = FixedMul(tr_x,viewcos); 
    gyt = -FixedMul(tr_y,viewsin);
    
    tz = gxt-gyt; 

    // thing is behind view plane?
    if (tz < MINZ)
	return;
    
    xscale = FixedDiv(projection, tz);
	
    gxt = -FixedMul(tr_x,viewsin); 
    gyt = FixedMul(tr_y,viewcos); 
    tx = -(gyt+gxt); 

    // too far off the side?
    if (abs(tx)>(tz<<2))
	return;

    // [kg] hidden marker? TODO: option to show markers
    if(thing->state == &states[S_NULL])
	return;
    
    // decide which patch to use for sprite relative to player
#ifdef RANGECHECK
    if ((unsigned)thing->sprite >= numsprites)
	I_Error ("R_ProjectSprite: invalid sprite number %i ",
		 thing->sprite);
#endif
    sprdef = &sprites[thing->sprite];
#ifdef RANGECHECK
    if ( (thing->frame&FF_FRAMEMASK) >= sprdef->numframes )
//	I_Error ("R_ProjectSprite: invalid sprite frame %i : %i ", thing->sprite, thing->frame);
	return;
#endif
    sprframe = &sprdef->spriteframes[ thing->frame & FF_FRAMEMASK];

    if (sprframe->rotate)
    {
	// choose a different rotation based on player view
	ang = R_PointToAngle (thing->x, thing->y);
	rot = (ang-thing->angle+(unsigned)(ANG45/2)*9)>>29;
	lump = sprframe->lump[rot];
	flip = (boolean)sprframe->flip[rot];
    }
    else
    {
	// use single rotation for all views
	lump = sprframe->lump[0];
	flip = (boolean)sprframe->flip[0];
    }
    
    // calculate edges of the shape
    tx -= R_GetSpriteLeftOffset(lump);
    x1 = (centerxfrac + FixedMul (tx,xscale) ) >>FRACBITS;

    // off the right side?
    if (x1 > viewwidth)
	return;
    
    tx += R_GetSpriteWidth(lump);
    x2 = ((centerxfrac + FixedMul (tx,xscale) ) >>FRACBITS) - 1;

    // off the left side
    if (x2 < 0)
	return;
    
    // store information in a vissprite
    vis = R_NewVisSprite ();
    vis->mo = thing;
    vis->was_hidden = false;
    vis->translation = thing->translation.data;
    vis->scale = xscale;
    vis->gx = thing->x;
    vis->gy = thing->y;
    vis->gz = thing->z;
    vis->gzt = thing->z + R_GetSpriteTopOffset(lump);
    vis->texturemid = vis->gzt - viewz;
    vis->x1 = x1 < 0 ? 0 : x1;
    vis->x2 = x2 >= viewwidth ? viewwidth-1 : x2;	
    iscale = FixedDiv (FRACUNIT, xscale);

    if (flip)
    {
	vis->startfrac = R_GetSpriteWidth(lump)-1;
	vis->xiscale = -iscale;
    }
    else
    {
	vis->startfrac = 0;
	vis->xiscale = iscale;
    }

    if (vis->x1 > x1)
	vis->startfrac += vis->xiscale*(vis->x1-x1);
    vis->patch = lump;

}




//
// R_AddSprites
// During BSP traversal, this adds sprites by sector.
//
void R_AddSprites (sector_t* sec)
{
    mobj_t*		thing;
    int			lightnum;

    // BSP is traversed by subsector.
    // A sector might have been split into several
    //  subsectors during BSP building.
    // Thus we check whether its already added.
    if (sec->validcount == validcount)
	return;

    // Well, now it will be done.
    sec->validcount = validcount;

    // Handle all things in sector.
    for (thing = sec->thinglist ; thing ; thing = thing->snext)
	R_ProjectSprite (thing);
}


//
// R_DrawPSprite
//
void R_DrawPSprite (pspdef_t* psp)
{
    fixed_t		tx;
    int			x1;
    int			x2;
    spritedef_t*	sprdef;
    spriteframe_t*	sprframe;
    int			lump;
    boolean		flip;
    vissprite_t*	vis;
    vissprite_t		avis;
    
    // decide which patch to use
#ifdef RANGECHECK
    if ( (unsigned)psp->state->sprite >= numsprites)
	I_Error ("R_ProjectSprite: invalid sprite number %i ",
		 psp->state->sprite);
#endif
    sprdef = &sprites[psp->state->sprite];
#ifdef RANGECHECK
    if ( (psp->state->frame & FF_FRAMEMASK)  >= sprdef->numframes)
//	I_Error ("R_ProjectSprite: invalid sprite frame %i : %i ", psp->state->sprite, psp->state->frame);
	return;
#endif
    sprframe = &sprdef->spriteframes[ psp->state->frame & FF_FRAMEMASK ];

    lump = sprframe->lump[0];
    flip = (boolean)sprframe->flip[0];
    
    // calculate edges of the shape
    tx = psp->sx-160*FRACUNIT;
	
    tx -= R_GetSpriteLeftOffset(lump);
    x1 = (centerxfrac + FixedMul (tx,pspritescale) ) >>FRACBITS;

    // off the right side
    if (x1 > viewwidth)
	return;		

    tx += R_GetSpriteWidth(lump);
    x2 = ((centerxfrac + FixedMul (tx, pspritescale) ) >>FRACBITS) - 1;

    // off the left side
    if (x2 < 0)
	return;
    
    // store information in a vissprite
    vis = &avis;
    vis->translation = NULL;
    vis->mo = NULL;
    vis->texturemid = (BASEYCENTER<<FRACBITS)+FRACUNIT/2-(psp->sy-R_GetSpriteTopOffset(lump));
    vis->x1 = x1 < 0 ? 0 : x1;
    vis->x2 = x2 >= viewwidth ? viewwidth-1 : x2;	
    vis->scale = pspritescale;
    
    if (flip)
    {
	vis->xiscale = -pspriteiscale;
	vis->startfrac = R_GetSpriteWidth(lump)-1;
    }
    else
    {
	vis->xiscale = pspriteiscale;
	vis->startfrac = 0;
    }
    
    if (vis->x1 > x1)
	vis->startfrac += vis->xiscale*(vis->x1-x1);

    vis->patch = lump;

    if (viewplayer->mo->flags & MF_SHADOW)
    {
	// shadow draw
	fixedcolormap = NULL;
    }
    else if (fixedcolormap)
    {
	// fixed color
	dc_lightcolor = colormaps;
    }
    else if (psp->state->frame & FF_FULLBRIGHT)
    {
	// full bright
	fixedcolormap = colormaps;
	dc_lightcolor = colormaps;
    }
    else
    {
	// local light
	fixedcolormap = spritelights[MAXLIGHTSCALE-1];
    }

    R_DrawVisSprite (vis, vis->x1, vis->x2);
}



//
// R_DrawPlayerSprites
//
void R_DrawPlayerSprites (void)
{
    int		i;
    int		lightnum;
    pspdef_t*	psp;

    if(!viewplayer)
	return;

    if(viewplayer->cheats & CF_SPECTATOR)
	return;

    if(netgame && netgame < 2)
	return;

    if(!viewplayer)
	return;

    if(!viewplayer->mo)
	return;

    if(!viewplayer->mo->subsector)
	return;

    if(!viewplayer->mo->subsector->sector)
	return;

    // get light level
    lightnum = (viewplayer->mo->subsector->sector->lightlevel >> LIGHTSEGSHIFT) + extralight;
    dc_lightcolor = viewplayer->mo->subsector->sector->colormap.data;

    if (lightnum < 0)		
	spritelights = scalelight[0];
    else if (lightnum >= LIGHTLEVELS)
	spritelights = scalelight[LIGHTLEVELS-1];
    else
	spritelights = scalelight[lightnum];
    
    // clip to screen bounds
    mfloorclip = screenheightarray;
    mceilingclip = negonearray;
    
    // add all active psprites
    for (i=0, psp=viewplayer->psprites;
	 i<NUMPSPRITES;
	 i++,psp++)
    {
	if (psp->state)
	    R_DrawPSprite (psp);
    }
}




//
// R_SortVisSprites
//
vissprite_t	vsprsortedhead;


void R_SortVisSprites (void)
{
    int			i;
    int			count;
    vissprite_t*	ds;
    vissprite_t*	best;
    vissprite_t		unsorted;
    fixed_t		bestscale;

    count = vissprite_p - vissprites;
	
    unsorted.next = unsorted.prev = &unsorted;

    if (!count)
	return;
		
    for (ds=vissprites ; ds<vissprite_p ; ds++)
    {
	ds->next = ds+1;
	ds->prev = ds-1;
    }
    
    vissprites[0].prev = &unsorted;
    unsorted.next = &vissprites[0];
    (vissprite_p-1)->next = &unsorted;
    unsorted.prev = vissprite_p-1;
    
    // pull the vissprites out by scale
    //best = 0;		// shut up the compiler warning
    vsprsortedhead.next = vsprsortedhead.prev = &vsprsortedhead;
    for (i=0 ; i<count ; i++)
    {
	bestscale = MAXINT;
	for (ds=unsorted.next ; ds!= &unsorted ; ds=ds->next)
	{
	    if (ds->scale < bestscale)
	    {
		bestscale = ds->scale;
		best = ds;
	    }
	}
	best->next->prev = best->prev;
	best->prev->next = best->next;
	best->next = &vsprsortedhead;
	best->prev = vsprsortedhead.prev;
	vsprsortedhead.prev->next = best;
	vsprsortedhead.prev = best;
    }
}



//
// R_DrawSprite
//
void R_DrawSprite (vissprite_t* spr)
{
    drawseg_t*		ds;
    short		clipbot[SCREENWIDTH];
    short		cliptop[SCREENWIDTH];
    int			x;
    int			r1;
    int			r2;
    fixed_t		scale;
    fixed_t		lowscale;
    int			silhouette;
		
    for (x = spr->x1 ; x<=spr->x2 ; x++)
	clipbot[x] = cliptop[x] = -2;
    
    // Scan drawsegs from end to start for obscuring segs.
    // The first drawseg that has a greater scale
    //  is the clip seg.
    for (ds=ds_p-1 ; ds >= drawsegs ; ds--)
    {
	// determine if the drawseg obscures the sprite
	if (ds->x1 > spr->x2
	    || ds->x2 < spr->x1
	    || (!ds->silhouette
		&& !ds->maskedtexturecol) )
	{
	    // does not cover sprite
	    continue;
	}
			
	r1 = ds->x1 < spr->x1 ? spr->x1 : ds->x1;
	r2 = ds->x2 > spr->x2 ? spr->x2 : ds->x2;

	if (ds->scale1 > ds->scale2)
	{
	    lowscale = ds->scale2;
	    scale = ds->scale1;
	}
	else
	{
	    lowscale = ds->scale1;
	    scale = ds->scale2;
	}
		
	if (scale < spr->scale
	    || ( lowscale < spr->scale
		 && !R_PointOnSegSide (spr->gx, spr->gy, ds->curline) ) )
	{
	    // masked mid texture?
	    if (ds->maskedtexturecol)	
		R_RenderMaskedSegRange (ds, r1, r2);
	    // seg is behind sprite
	    continue;			
	}

	
	// clip this piece of the sprite
	silhouette = ds->silhouette;
	
	if (spr->gz >= ds->bsilheight)
	    silhouette &= ~SIL_BOTTOM;

	if (spr->gzt <= ds->tsilheight)
	    silhouette &= ~SIL_TOP;
			
	if (silhouette == 1)
	{
	    // bottom sil
	    for (x=r1 ; x<=r2 ; x++)
		if (clipbot[x] == -2)
		    clipbot[x] = ds->sprbottomclip[x];
	}
	else if (silhouette == 2)
	{
	    // top sil
	    for (x=r1 ; x<=r2 ; x++)
		if (cliptop[x] == -2)
		    cliptop[x] = ds->sprtopclip[x];
	}
	else if (silhouette == 3)
	{
	    // both
	    for (x=r1 ; x<=r2 ; x++)
	    {
		if (clipbot[x] == -2)
		    clipbot[x] = ds->sprbottomclip[x];
		if (cliptop[x] == -2)
		    cliptop[x] = ds->sprtopclip[x];
	    }
	}
		
    }
    
    // all clipping has been performed, so draw the sprite

    // check for unclipped columns
    for (x = spr->x1 ; x<=spr->x2 ; x++)
    {
	if (clipbot[x] == -2)		
	    clipbot[x] = viewheight;

	if (cliptop[x] == -2)
	    cliptop[x] = -1;
    }
		
    mfloorclip = clipbot;
    mceilingclip = cliptop;
    R_DrawVisSprite (spr, spr->x1, spr->x2);
}




//
// R_DrawMasked
//
void R_DrawMaskedClip (void)
{
    vissprite_t *spr;
    drawseg_t *ds;

    if (vissprite_p > vissprites)
    {
	// draw all vissprites back to front
	for (spr = vsprsortedhead.next ;
	     spr != &vsprsortedhead ;
	     spr=spr->next)
	{
	    
	    R_DrawSprite (spr);
	}
    }

    // render any remaining masked mid textures
    for (ds=ds_p-1 ; ds >= drawsegs ; ds--)
	if (ds->maskedtexturecol)
	    R_RenderMaskedSegRange (ds, ds->x1, ds->x2);

    // toggle clip for midtex
    height_count ^= 0x8000;
}

void R_DrawMasked (void)
{
	height3d_t *hh;

	R_SortVisSprites ();

	// [kg] reset toggle for midtex
	height_count = 0x8000;

	// [kg] draw from top to viewz
	height_top = ONCEILINGZ;
	hh = height3top.prev;
	while(1)
	{
		if(hh->height < viewz)
			break;
		height_bot = hh->height;
		R_DrawMaskedClip();
		height_top = height_bot;
		R_DrawPlanes(hh->height);
		hh = hh->prev;
	}

	// [kg] draw from bottom to viewz
	hh = &height3bot;
	while(1)
	{
		height_bot = hh->height;
		height_top = hh->next->height;
		R_DrawMaskedClip();
		if(hh->next->height >= viewz)
			break;
		R_DrawPlanes(hh->next->height);
		hh = hh->next;
	}

	// draw the psprites on top of everything
	//  but does not draw on side views
	clip_bot = -1;
	clip_top = -1;
	if (!viewangleoffset)		
		R_DrawPlayerSprites ();
}

#endif

// [kg] get simple sprite lump
int R_GetStateLump(statenum_t snum)
{
	state_t *state;
	spritedef_t *sprdef;
	spriteframe_t *sprframe;

	state = &states[snum];
	sprdef = &sprites[state->sprite];
	sprframe = &sprdef->spriteframes[state->frame & FF_FRAMEMASK];

	return sprframe->lump[0];
}

