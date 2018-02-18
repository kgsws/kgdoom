#include <stdlib.h>

#include "doomdef.h"

#include "i_system.h"

#include "doomdef.h"
#include "doomstat.h"

#include "p_local.h"

#include "r_local.h"
#include "r_sky.h"

#include "kg_3dfloor.h"

// OPTIMIZE: closed two sided lines as single sided

// True if any of the segs textures might be visible.
boolean		segtextured;	

// False if the back side is the same plane.
boolean		markfloor;	
boolean		markceiling;

boolean		maskedtexture;
int		toptexture;
int		bottomtexture;
int		midtexture;


angle_t		rw_normalangle;
// angle to line origin
int		rw_angle1;	

//
// regular wall
//
int		rw_x;
int		rw_stopx;
angle_t		rw_centerangle;
fixed_t		rw_offset;
fixed_t		rw_distance;
fixed_t		rw_scale;
fixed_t		rw_scalestep;
fixed_t		rw_midtexturemid;
fixed_t		rw_toptexturemid;
fixed_t		rw_bottomtexturemid;

int		worldtop;
int		worldbottom;
int		worldhigh;
int		worldlow;

fixed_t		pixhigh;
fixed_t		pixlow;
fixed_t		pixhighstep;
fixed_t		pixlowstep;

fixed_t		topfrac;
fixed_t		topstep;

fixed_t		bottomfrac;
fixed_t		bottomstep;

// [kg] for wall stripes
fixed_t	stripebot;
fixed_t	stripebotstep;
fixed_t	stripetop;
fixed_t	stripetopstep;
fixed_t stripetexturemid;
int stripetexture;

lighttable_t**	walllights;

short*		maskedtexturecol;

// [kg] for 3D midtex stage
extern int height_count;
extern fixed_t height_top;
extern fixed_t height_bot;
extern int clip_top;
extern int clip_bot;

// [kg] dummy colfunc
void dummy_draw()
{
}

// [kg] for stripe calculation
void R_GetHeightFrac(fixed_t height, fixed_t *frac, fixed_t *step)
{
	fixed_t tmphigh = (height - viewz) >> 4;
	*frac = (centeryfrac>>4) - FixedMul (tmphigh, rw_scale);
	*step = -FixedMul (rw_scalestep,tmphigh);
}

// [kg] single masked texture run
void R_DrawMaskedSegRange(int x1, int x2, int texnum, int topc, int tops, int botc, int bots)
{
    unsigned	index;
    column_t*	col;

    for (dc_x = x1 ; dc_x <= x2 ; dc_x++)
    {
	// [kg] add 3D clip
	if(height_top != ONCEILINGZ)
	{
	    clip_top = (topc+HEIGHTUNIT-1)>>HEIGHTBITS;
	    topc += tops;
	    if(clip_top < 0)
		clip_top = 0;
	} else
	    clip_top = -1;
	if(height_bot != ONFLOORZ)
	{
	    clip_bot = ((botc+HEIGHTUNIT-1)>>HEIGHTBITS)-1;
	    botc += bots;
	    if(clip_bot < 0)
		goto skip;
	} else
	    clip_bot = -1;
	// calculate lighting
	if(height_count ^ (maskedtexturecol[dc_x] & 0x8000))
	{
	    if (!fixedcolormap)
	    {
		index = spryscale>>LIGHTSCALESHIFT;

		if (index >=  MAXLIGHTSCALE )
		    index = MAXLIGHTSCALE-1;

		dc_colormap = walllights[index];
	    }
			
	    sprtopscreen = centeryfrac - FixedMul(dc_texturemid, spryscale);
	    dc_iscale = 0xffffffffu / (unsigned)spryscale;

	    // draw the texture
	    col = (column_t *)( 
		(byte *)R_GetColumn(texnum,maskedtexturecol[dc_x]) -3);
			
	    R_DrawMaskedColumn (col);
	}
skip:
	spryscale += rw_scalestep;
    }
}

//
// R_RenderMaskedSegRange
//
void
R_RenderMaskedSegRange
( drawseg_t*	ds,
  int		x1,
  int		x2 )
{
    int		lightnum;
    int		texnum;
    int botc, topc;
    int bots, tops;
    extraplane_t *pl;

    // Calculate light table.
    // Use different light tables
    //   for horizontal / vertical / diagonal. Diagonal?
    // OPTIMIZE: get rid of LIGHTSEGSHIFT globally
    curline = ds->curline;
    frontsector = curline->frontsector;
    backsector = curline->backsector;

    // [kg] get correct light
    lightnum = (frontsector->lightlevel >> LIGHTSEGSHIFT)+extralight;
    if(height_top != ONCEILINGZ)
    {
	pl = frontsector->exfloor;
	while(pl)
	{
	    if(height_top <= *pl->height)
	    {
		lightnum = (*pl->lightlevel >> LIGHTSEGSHIFT)+extralight;
		break;
	    }
	    pl = pl->next;
	}
    }

    if(r_fakecontrast)
    {
	if (curline->v1->y == curline->v2->y)
	    lightnum--;
	else if (curline->v1->x == curline->v2->x)
	    lightnum++;

	if (lightnum < 0)		
	    walllights = scalelight[0];
	else if (lightnum >= LIGHTLEVELS)
	    walllights = scalelight[LIGHTLEVELS-1];
	else
	    walllights = scalelight[lightnum];
    } else
	walllights = scalelight[lightnum];

    maskedtexturecol = ds->maskedtexturecol;

    rw_scalestep = ds->scalestep;		
    mfloorclip = ds->sprbottomclip;
    mceilingclip = ds->sprtopclip;

    // [kg] 3D clip
    if(height_bot != ONFLOORZ)
    {
	int temp = height_bot - viewz;
	temp >>= 4;
	botc = (centeryfrac>>4) - FixedMul (temp, ds->scale1);
	bots = -FixedMul (rw_scalestep,temp);
    }
    if(height_top != ONCEILINGZ)
    {
	int temp = height_top - viewz;
	temp >>= 4;
	topc = (centeryfrac>>4) - FixedMul (temp, ds->scale1);
	tops = -FixedMul (rw_scalestep,temp);
    }

    if (fixedcolormap)
	dc_colormap = fixedcolormap;

    // [kg] skip offset, if needed
    if(ds->x1 != x1)
    {
	topc += tops * (x1 - ds->x1);
	botc += bots * (x1 - ds->x1);
    }

    // draw all sides
    pl = backsector->exfloor;
    while(pl)
    {
	if(height_top > pl->source->floorheight && height_top <= pl->source->ceilingheight)
	{
	    spryscale = ds->scale1 + (x1 - ds->x1)*rw_scalestep;
	    // texture
	    texnum = texturetranslation[sides[pl->line->sidenum[0]].midtexture];
	    if(texnum)
	    {
		// find positioning
		if(pl->line->flags & LF_DONTPEGBOTTOM)
		    dc_texturemid = (*pl->height + textureheight[texnum]) - viewz;
		else
		    dc_texturemid = *pl->height - viewz;
		dc_texturemid += sides[pl->line->sidenum[0]].rowoffset;
		// draw the columns
		R_DrawMaskedSegRange(x1, x2, texnum, topc, tops, botc, bots);
	    }
	}
	// next
	pl = pl->next;
    }

    // draw mid texture
    if(curline->sidedef->midtexture)
    {
	spryscale = ds->scale1 + (x1 - ds->x1)*rw_scalestep;
	// texture
	texnum = texturetranslation[curline->sidedef->midtexture];
	// find positioning
	if (curline->linedef->flags & LF_DONTPEGBOTTOM)
	{
	    dc_texturemid = frontsector->floorheight > backsector->floorheight ? frontsector->floorheight : backsector->floorheight;
	    dc_texturemid = dc_texturemid + textureheight[texnum] - viewz;
	}
	else
	{
	    dc_texturemid =frontsector->ceilingheight<backsector->ceilingheight ? frontsector->ceilingheight : backsector->ceilingheight;
	    dc_texturemid = dc_texturemid - viewz;
	}
	dc_texturemid += curline->sidedef->rowoffset;
	// draw the columns
	R_DrawMaskedSegRange(x1, x2, texnum, topc, tops, botc, bots);
    }

    // do clipping now
    for (dc_x = x1 ; dc_x <= x2 ; dc_x++)
	maskedtexturecol[dc_x] = (maskedtexturecol[dc_x] & 0x7FFF) | height_count;
}




//
// R_RenderSegLoop
// Draws zero, one, or two textures (and possibly a masked
//  texture) for walls.
// Can draw or mark the starting pixel of floor and ceiling
//  textures.
// CALLED: CORE LOOPING ROUTINE.
//

void R_RenderSegLoop (int horizon)
{
    angle_t		angle;
    unsigned		index;
    int			yl;
    int			yh;
    int			mid;
    fixed_t		texturecolumn;
    int			top;
    int			bottom;

    //texturecolumn = 0;				// shut up compiler warning

	// [kg] line horizon or fake line
	if(horizon)
	{
		segtextured = 0;
		midtexture = 0;
		toptexture = 0;
		bottomtexture = 0;
		maskedtexture = 0;
	}

    for ( ; rw_x < rw_stopx ; rw_x++)
    {
	// mark floor / ceiling areas

	if(horizon)
		yl = (viewheight / 2)+1;
	else
		yl = (topfrac+HEIGHTUNIT-1)>>HEIGHTBITS;

	// no space above wall?
	if (yl < ceilingclip[rw_x]+1)
	    yl = ceilingclip[rw_x]+1;
	
	if (markceiling)
	{
	    top = ceilingclip[rw_x]+1;
	    bottom = yl-1;

	    if (bottom >= floorclip[rw_x])
		bottom = floorclip[rw_x]-1;

	    if (top <= bottom)
	    {
		ceilingplane->top[rw_x] = top;
		ceilingplane->bottom[rw_x] = bottom;
	    }
	}
		
	if(horizon)
		yh = viewheight / 2;
	else
		yh = bottomfrac>>HEIGHTBITS;

	if (yh >= floorclip[rw_x])
	    yh = floorclip[rw_x]-1;

	if (markfloor)
	{
	    top = yh+1;
	    bottom = floorclip[rw_x]-1;
	    if (top <= ceilingclip[rw_x])
		top = ceilingclip[rw_x]+1;
	    if (top <= bottom)
	    {
		floorplane->top[rw_x] = top;
		floorplane->bottom[rw_x] = bottom;
	    }
	}
	
	// texturecolumn and lighting are independent of wall tiers
	if (segtextured)
	{
	    // calculate texture offset
	    angle = (rw_centerangle + xtoviewangle[rw_x])>>ANGLETOFINESHIFT;
	    texturecolumn = rw_offset-FixedMul(finetangent[angle],rw_distance);
	    texturecolumn >>= FRACBITS;
	    // calculate lighting
	    index = rw_scale>>LIGHTSCALESHIFT;

	    if (index >=  MAXLIGHTSCALE )
		index = MAXLIGHTSCALE-1;

	    dc_colormap = walllights[index];
	    dc_x = rw_x;
	    dc_iscale = 0xffffffffu / (unsigned)rw_scale;
	}
	
	// draw the wall tiers
	if (midtexture)
	{
	    // single sided line
	    dc_yl = yl;
	    dc_yh = yh;
	    dc_texturemid = rw_midtexturemid;
	    dc_source = R_GetColumn(midtexture,texturecolumn);
	    colfunc ();
	    ceilingclip[rw_x] = viewheight;
	    floorclip[rw_x] = -1;
	}
	else
	{
	    // two sided line
	    if (toptexture)
	    {
		// top wall
		mid = pixhigh>>HEIGHTBITS;
		pixhigh += pixhighstep;

		if (mid >= floorclip[rw_x])
		    mid = floorclip[rw_x]-1;

		if (mid >= yl)
		{
		    dc_yl = yl;
		    dc_yh = mid;
		    dc_texturemid = rw_toptexturemid;
		    dc_source = R_GetColumn(toptexture,texturecolumn);
		    colfunc ();
		    ceilingclip[rw_x] = mid;
		}
		else
		    ceilingclip[rw_x] = yl-1;
	    }
	    else
	    {
		// no top wall
		if (markceiling)
		    ceilingclip[rw_x] = yl-1;
	    }
			
	    if (bottomtexture)
	    {
		// bottom wall
		mid = (pixlow+HEIGHTUNIT-1)>>HEIGHTBITS;
		pixlow += pixlowstep;

		// no space above wall?
		if (mid <= ceilingclip[rw_x])
		    mid = ceilingclip[rw_x]+1;
		
		if (mid <= yh)
		{
		    dc_yl = mid;
		    dc_yh = yh;
		    dc_texturemid = rw_bottomtexturemid;
		    dc_source = R_GetColumn(bottomtexture,
					    texturecolumn);
		    colfunc ();
		    floorclip[rw_x] = mid;
		}
		else
		    floorclip[rw_x] = yh+1;
	    }
	    else
	    {
		// no bottom wall
		if (markfloor)
		    floorclip[rw_x] = yh+1;
	    }
			
	    if (maskedtexture)
	    {
		// save texturecol
		//  for backdrawing of masked mid texture
		maskedtexturecol[rw_x] = texturecolumn & 0x7FFF; // [kg] update for 3D rendering
	    }
	}
		
	rw_scale += rw_scalestep;
	topfrac += topstep;
	bottomfrac += bottomstep;
    }
}

// [kg] this one renders stripe of wall
void R_RenderSegLoopStripe()
{
    angle_t		angle;
    unsigned		index;
    int			yl;
    int			yh;
    int			midT, midB;
    fixed_t		texturecolumn;
    int			top;
    int			bottom;

    int	old_rwx = rw_x;
    int	old_rwscale = rw_scale;
    fixed_t extra_topfrac = topfrac;
    fixed_t extra_bottomfrac = bottomfrac;
    fixed_t extra_stripetop = stripetop;
    fixed_t extra_stripebot = stripebot;

    //texturecolumn = 0;				// shut up compiler warning

    for ( ; rw_x < rw_stopx ; rw_x++)
    {
	yl = (extra_topfrac+HEIGHTUNIT-1)>>HEIGHTBITS;

	if (yl < ceilingclip[rw_x]+1)
	    yl = ceilingclip[rw_x]+1;
	
	yh = extra_bottomfrac>>HEIGHTBITS;

	if (yh >= floorclip[rw_x])
	    yh = floorclip[rw_x]-1;

	// texturecolumn and lighting are independent of wall tiers
	{
	    // calculate texture offset
	    angle = (rw_centerangle + xtoviewangle[rw_x])>>ANGLETOFINESHIFT;
	    texturecolumn = rw_offset-FixedMul(finetangent[angle],rw_distance);
	    texturecolumn >>= FRACBITS;
	    // calculate lighting
	    index = rw_scale>>LIGHTSCALESHIFT;

	    if (index >=  MAXLIGHTSCALE )
		index = MAXLIGHTSCALE-1;

	    dc_colormap = walllights[index];
	    dc_x = rw_x;
	    dc_iscale = 0xffffffffu / (unsigned)rw_scale;
	}

	// draw wall
	midT = extra_stripebot>>HEIGHTBITS;
	extra_stripebot += stripebotstep;
	midB = (extra_stripetop+HEIGHTUNIT-1)>>HEIGHTBITS;
	extra_stripetop += stripetopstep;

	if(midT >= floorclip[rw_x])
	    midT = floorclip[rw_x]-1;

	if (midB <= ceilingclip[rw_x])
	    midB = ceilingclip[rw_x]+1;

	if(midT >= yl && midB <= yh)
	{
	    dc_yl = midB;
	    dc_yh = midT;
	    dc_texturemid = stripetexturemid;
	    dc_source = R_GetColumn(stripetexture,texturecolumn);
	    colfunc ();
	}

	rw_scale += rw_scalestep;
	extra_topfrac += topstep;
	extra_bottomfrac += bottomstep;
    }

    rw_x = old_rwx;
    rw_scale = old_rwscale;
}

// [kg] this one is for fake rendering
void R_RenderSegLoopFake()
{
    int			yl;
    int			yh;
    int			mid;
    int			top;
    int			bottom;
    short *useclipbot = floorclip;
    short *usecliptop = ceilingclip;

    if(!fakeclip)
    {
	if(fakecliptop)
	    usecliptop = fakecliptop;
	if(fakeclipbot)
	    useclipbot = fakeclipbot;
    }

    for ( ; rw_x < rw_stopx ; rw_x++)
    {
	// mark floor / ceiling areas
	yl = (topfrac+HEIGHTUNIT-1)>>HEIGHTBITS;

	// no space above wall?
	if (yl < usecliptop[rw_x]+1)
	    yl = usecliptop[rw_x]+1;
	
	if (markceiling)
	{
	    top = usecliptop[rw_x]+1;
	    bottom = yl-1;

	    if (bottom >= useclipbot[rw_x])
		bottom = useclipbot[rw_x]-1;

	    if (top <= bottom)
	    {
		ceilingplane->top[rw_x] = top;
		ceilingplane->bottom[rw_x] = bottom;
	    }
	}
		
	yh = bottomfrac>>HEIGHTBITS;

	if (yh >= useclipbot[rw_x])
	    yh = useclipbot[rw_x]-1;

	if (markfloor)
	{
	    top = yh+1;
	    bottom = useclipbot[rw_x]-1;
	    if (top <= usecliptop[rw_x])
		top = usecliptop[rw_x]+1;
	    if (top <= bottom)
	    {
		floorplane->top[rw_x] = top;
		floorplane->bottom[rw_x] = bottom;
	    }
	}

	// top wall
	if(fakeclip && fakecliptop)
	{
	    if(worldhigh < worldtop)
	    {
		mid = pixhigh >> HEIGHTBITS;
		pixhigh += pixhighstep;

		if(mid >= floorclip[rw_x])
		    mid = floorclip[rw_x]-1;

		if(mid >= yl)
		    fakecliptop[rw_x] = mid;
		else
		    fakecliptop[rw_x] = yl-1;
	    } else
		fakecliptop[rw_x] = yl-1;
	}

	// bottom wall
	if(fakeclip && fakeclipbot)
	{
	    if(worldlow > worldbottom)
	    {
		mid = (pixlow+HEIGHTUNIT-1)>>HEIGHTBITS;
		pixlow += pixlowstep;

		if(mid <= ceilingclip[rw_x])
		    mid = ceilingclip[rw_x]+1;

		if(mid <= yh)
		    fakeclipbot[rw_x] = mid;
		else
		    fakeclipbot[rw_x] = yh+1;
	    } else
		fakeclipbot[rw_x] = yh+1;
	}

	topfrac += topstep;
	bottomfrac += bottomstep;
    }
}


//
// R_StoreWallRange
// A wall segment will be drawn
//  between start and stop pixels (inclusive).
//
void
R_StoreWallRange
( int	start,
  int	stop )
{
    fixed_t		hyp;
    fixed_t		sineval;
    angle_t		distangle, offsetangle;
    fixed_t		vtop;
    int			lightnum;

    // don't overflow and crash
    if (ds_p == &drawsegs[MAXDRAWSEGS])
	return;		
		
#ifdef RANGECHECK
    if (start >=viewwidth || start > stop)
	I_Error ("Bad R_RenderWallRange: %i to %i", start , stop);
#endif
    
    sidedef = curline->sidedef;
    linedef = curline->linedef;

    // mark the segment as visible for auto map
    linedef->flags |= LF_MAPPED;
    
    // calculate rw_distance for scale calculation
    rw_normalangle = curline->angle + ANG90;
    offsetangle = abs(rw_normalangle-rw_angle1);
    
    if (offsetangle > ANG90)
	offsetangle = ANG90;

    distangle = ANG90 - offsetangle;
    hyp = R_PointToDist (curline->v1->x, curline->v1->y);
    sineval = finesine[distangle>>ANGLETOFINESHIFT];
    rw_distance = FixedMul (hyp, sineval);
		
	
    ds_p->x1 = rw_x = start;
    ds_p->x2 = stop;
    ds_p->curline = curline;
    ds_p->scalestep = 0;
    rw_stopx = stop+1;
    
    // calculate scale at both ends and step
    ds_p->scale1 = rw_scale = 
	R_ScaleFromGlobalAngle (viewangle + xtoviewangle[start]);
    
    if (stop > start )
    {
	ds_p->scale2 = R_ScaleFromGlobalAngle (viewangle + xtoviewangle[stop]);
	ds_p->scalestep = rw_scalestep = 
	    (ds_p->scale2 - rw_scale) / (stop-start);
    }
    else
    {
	ds_p->scale2 = ds_p->scale1;
    }
    
    // calculate texture boundaries
    //  and decide if floor / ceiling marks are needed
    worldtop = frontsector->ceilingheight - viewz;
    worldbottom = frontsector->floorheight - viewz;
	
    midtexture = toptexture = bottomtexture = maskedtexture = 0;
    ds_p->maskedtexturecol = NULL;
	
    if (!backsector)
    {
	// single sided line
	midtexture = texturetranslation[sidedef->midtexture];
	// a single sided line is terminal, so it must mark ends
	markfloor = markceiling = true;
	if (linedef->flags & LF_DONTPEGBOTTOM)
	{
	    vtop = frontsector->floorheight +
		textureheight[sidedef->midtexture];
	    // bottom of texture at bottom
	    rw_midtexturemid = vtop - viewz;	
	}
	else
	{
	    // top of texture at top
	    rw_midtexturemid = worldtop;
	}
	rw_midtexturemid += sidedef->rowoffset;

	ds_p->silhouette = SIL_BOTH;
	ds_p->sprtopclip = screenheightarray;
	ds_p->sprbottomclip = negonearray;
	ds_p->bsilheight = MAXINT;
	ds_p->tsilheight = MININT;
    }
    else
    {
	// two sided line
	ds_p->sprtopclip = ds_p->sprbottomclip = NULL;
	ds_p->silhouette = 0;
	
	if (frontsector->floorheight > backsector->floorheight)
	{
	    ds_p->silhouette = SIL_BOTTOM;
	    ds_p->bsilheight = frontsector->floorheight;
	}
	else if (backsector->floorheight > viewz)
	{
	    ds_p->silhouette = SIL_BOTTOM;
	    ds_p->bsilheight = MAXINT;
	    // ds_p->sprbottomclip = negonearray;
	}
	
	if (frontsector->ceilingheight < backsector->ceilingheight)
	{
	    ds_p->silhouette |= SIL_TOP;
	    ds_p->tsilheight = frontsector->ceilingheight;
	}
	else if (backsector->ceilingheight < viewz)
	{
	    ds_p->silhouette |= SIL_TOP;
	    ds_p->tsilheight = MININT;
	    // ds_p->sprtopclip = screenheightarray;
	}
		
	if (backsector->ceilingheight <= frontsector->floorheight)
	{
	    ds_p->sprbottomclip = negonearray;
	    ds_p->bsilheight = MAXINT;
	    ds_p->silhouette |= SIL_BOTTOM;
	}
	
	if (backsector->floorheight >= frontsector->ceilingheight)
	{
	    ds_p->sprtopclip = screenheightarray;
	    ds_p->tsilheight = MININT;
	    ds_p->silhouette |= SIL_TOP;
	}
	
	worldhigh = backsector->ceilingheight - viewz;
	worldlow = backsector->floorheight - viewz;
		
	// hack to allow height changes in outdoor areas
	if (frontsector->ceilingpic == skyflatnum 
	    && backsector->ceilingpic == skyflatnum)
	{
	    worldtop = worldhigh;
	}
	
			
	if (worldlow != worldbottom 
	    || backsector->floorpic != frontsector->floorpic
	    || backsector->lightlevel != frontsector->lightlevel
	    // [kg] 3D floor check
	    || backsector->exfloor || frontsector->exfloor
	)
	{
	    markfloor = true;
	}
	else
	{
	    // same plane on both sides
	    markfloor = false;
	}
	
			
	if (worldhigh != worldtop 
	    || backsector->ceilingpic != frontsector->ceilingpic
	    || backsector->lightlevel != frontsector->lightlevel
	    // [kg] 3D floor check
	    || backsector->exceiling || frontsector->exceiling
	)
	{
	    markceiling = true;
	}
	else
	{
	    // same plane on both sides
	    markceiling = false;
	}
	
	if (backsector->ceilingheight <= frontsector->floorheight
	    || backsector->floorheight >= frontsector->ceilingheight)
	{
	    // closed door
	    markceiling = markfloor = true;
	}
	

	if (worldhigh < worldtop)
	{
	    // top texture
	    toptexture = texturetranslation[sidedef->toptexture];
	    if (linedef->flags & LF_DONTPEGTOP)
	    {
		// top of texture at top
		rw_toptexturemid = worldtop;
	    }
	    else
	    {
		vtop =
		    backsector->ceilingheight
		    + textureheight[sidedef->toptexture];
		
		// bottom of texture
		rw_toptexturemid = vtop - viewz;	
	    }
	}
	if (worldlow > worldbottom)
	{
	    // bottom texture
	    bottomtexture = texturetranslation[sidedef->bottomtexture];

	    if (linedef->flags & LF_DONTPEGBOTTOM )
	    {
		// bottom of texture at bottom
		// top of texture at top
		rw_bottomtexturemid = worldtop;
	    }
	    else	// top of texture at top
		rw_bottomtexturemid = worldlow;
	}
	rw_toptexturemid += sidedef->rowoffset;
	rw_bottomtexturemid += sidedef->rowoffset;
	
	// allocate space for masked texture tables
	if (sidedef->midtexture || (!fakeclip && backsector && backsector->exfloor))
	{
	    // masked midtexture
	    maskedtexture = true;
	    if(!sidedef->midtexture)
		ds_p->silhouette = 0xff;
	    ds_p->maskedtexturecol = maskedtexturecol = lastopening - rw_x;
	    lastopening += rw_stopx - rw_x;
	}
    }
    
    // calculate rw_offset (only needed for textured lines)
    segtextured = midtexture | toptexture | bottomtexture | maskedtexture;

    if (segtextured && !fakeclip)
    {
	offsetangle = rw_normalangle-rw_angle1;
	
	if (offsetangle > ANG180)
	    offsetangle = -offsetangle;

	if (offsetangle > ANG90)
	    offsetangle = ANG90;

	sineval = finesine[offsetangle >>ANGLETOFINESHIFT];
	rw_offset = FixedMul (hyp, sineval);

	if (rw_normalangle-rw_angle1 < ANG180)
	    rw_offset = -rw_offset;

	rw_offset += sidedef->textureoffset + curline->offset;
	rw_centerangle = ANG90 + viewangle - rw_normalangle;
	
	// calculate light table
	//  use different light tables
	//  for horizontal / vertical / diagonal
	// OPTIMIZE: get rid of LIGHTSEGSHIFT globally
	if (!fixedcolormap)
	{
	    lightnum = (frontsector->lightlevel >> LIGHTSEGSHIFT)+extralight;

	    if(r_fakecontrast)
	    {
		if (curline->v1->y == curline->v2->y)
		    lightnum--;
		else if (curline->v1->x == curline->v2->x)
		    lightnum++;

		if (lightnum < 0)		
		    walllights = scalelight[0];
		else if (lightnum >= LIGHTLEVELS)
		    walllights = scalelight[LIGHTLEVELS-1];
		else
		    walllights = scalelight[lightnum];
	    } else
		walllights = scalelight[lightnum];
	}
    }
    
    // if a floor / ceiling plane is on the wrong side
    //  of the view plane, it is definitely invisible
    //  and doesn't need to be marked.
    
  
    if (frontsector->floorheight >= viewz)
    {
	// above view plane
	markfloor = false;
    }
    
    if (frontsector->ceilingheight <= viewz 
	&& frontsector->ceilingpic != skyflatnum)
    {
	// below view plane
	markceiling = false;
    }
    
    // calculate incremental stepping values for texture edges
    worldtop >>= 4;
    worldbottom >>= 4;
	
    topstep = -FixedMul (rw_scalestep, worldtop);
    topfrac = (centeryfrac>>4) - FixedMul (worldtop, rw_scale);

    bottomstep = -FixedMul (rw_scalestep,worldbottom);
    bottomfrac = (centeryfrac>>4) - FixedMul (worldbottom, rw_scale);
	
    if (backsector)
    {	
	worldhigh >>= 4;
	worldlow >>= 4;

	if (worldhigh < worldtop)
	{
	    pixhigh = (centeryfrac>>4) - FixedMul (worldhigh, rw_scale);
	    pixhighstep = -FixedMul (rw_scalestep,worldhigh);
	}
	
	if (worldlow > worldbottom)
	{
	    pixlow = (centeryfrac>>4) - FixedMul (worldlow, rw_scale);
	    pixlowstep = -FixedMul (rw_scalestep,worldlow);
	}
    }

    // [kg] fake check
    if(!ceilingplane)
	markceiling = false;
    if(!floorplane)
	markfloor = false;
    
    // render it
    if (markceiling)
	ceilingplane = R_CheckPlane (ceilingplane, rw_x, rw_stopx-1);

    if (markfloor)
	floorplane = R_CheckPlane (floorplane, rw_x, rw_stopx-1);

    // [kg] fake check
    if(fakeclip)
    {
	R_RenderSegLoopFake();
	return;
    } else
    {
	void *ocolfunc = colfunc;
	boolean is_horizon = curline->linedef && curline->linedef->sidenum[1] == -2;
	// [kg] sides split rendering from 3D light
	if(segtextured && !fixedcolormap && !is_horizon && frontsector->exfloor)
	{
		fixed_t check_bot = ONCEILINGZ;
		fixed_t check_top = ONFLOORZ;
		extraplane_t *pl = frontsector->exfloor;

		if(backsector)
		{
			stripetexture = bottomtexture;
			stripetexturemid = rw_bottomtexturemid;
			check_bot = backsector->floorheight;
			check_top = backsector->ceilingheight;
		} else
		{
			stripetexture = midtexture;
			stripetexturemid = rw_midtexturemid;
		}

		stripetop = bottomfrac;
		stripetopstep = bottomstep;

		while(1)
		{
			fixed_t height;
			int lightlevel;
			boolean repeat = false;

			if(pl)
			{
				// take values from correct plane
				height = *pl->height;
				lightlevel = *pl->lightlevel;
			} else
			{
				// topmost level; take values from sector
				height = frontsector->ceilingheight;
				lightlevel = frontsector->lightlevel;
			}

			// check for ceiling
			if(height > frontsector->ceilingheight)
				height = frontsector->ceilingheight;

			// check for skip
			if(pl && check_bot == ONCEILINGZ && height <= check_top)
			{
				pl = pl->next;
				continue;
			}

			// check for backsector floor
			if(height > check_bot)
			{
				height = check_bot;
				check_bot = ONCEILINGZ;
				repeat = true;
			}

			// checks
			if(	height >= frontsector->floorheight || frontsector->floorpic == skyflatnum // hidden planes
			) {
				// set bottom to previous top
				stripebot = stripetop;
				stripebotstep = stripetopstep;

				// set new top
				R_GetHeightFrac(height, &stripetop, &stripetopstep);

				// pick new light
				lightnum = (lightlevel >> LIGHTSEGSHIFT)+extralight;
				if(r_fakecontrast)
				{
					if (curline->v1->y == curline->v2->y)
						lightnum--;
					else if (curline->v1->x == curline->v2->x)
						lightnum++;

					if (lightnum < 0)		
						walllights = scalelight[0];
					else if (lightnum >= LIGHTLEVELS)
						walllights = scalelight[LIGHTLEVELS-1];
					else
						walllights = scalelight[lightnum];
				} else
					walllights = scalelight[lightnum];

				// draw this stripe
				R_RenderSegLoopStripe();
			}

			// next range
			if((!pl || height == frontsector->ceilingheight) && !repeat)
				// done
				break;
			if(repeat)
			{
				// change texture, check this range again
				stripetexture = toptexture;
				stripetexturemid = rw_toptexturemid;
				// move to top edge
				R_GetHeightFrac(check_top, &stripetop, &stripetopstep);
			} else
				pl = pl->next;
		}
		colfunc = dummy_draw;
	}
	R_RenderSegLoop(is_horizon); // [kg] line horizon check
	colfunc = ocolfunc;
    }

    // save sprite clipping info
    if ( ((ds_p->silhouette & SIL_TOP) || maskedtexture)
	 && !ds_p->sprtopclip)
    {
	memcpy (lastopening, ceilingclip+start, 2*(rw_stopx-start));
	ds_p->sprtopclip = lastopening - start;
	lastopening += rw_stopx - start;
    }
    
    if ( ((ds_p->silhouette & SIL_BOTTOM) || maskedtexture)
	 && !ds_p->sprbottomclip)
    {
	memcpy (lastopening, floorclip+start, 2*(rw_stopx-start));
	ds_p->sprbottomclip = lastopening - start;
	lastopening += rw_stopx - start;	
    }

    if (maskedtexture && !(ds_p->silhouette&SIL_TOP))
    {
	ds_p->silhouette |= SIL_TOP;
	ds_p->tsilheight = MININT;
    }
    if (maskedtexture && !(ds_p->silhouette&SIL_BOTTOM))
    {
	ds_p->silhouette |= SIL_BOTTOM;
	ds_p->bsilheight = MAXINT;
    }
    // [kg] no clipping for 3D sides
    if(ds_p->silhouette & 4)
	ds_p->silhouette = 0;

    ds_p++;
}

// [kg] this one is for fake rendering
void
R_StoreWallRangeFake
( int	start,
  int	stop )
{
    fixed_t		hyp;
    fixed_t		sineval;
    angle_t		distangle, offsetangle;
    fixed_t		vtop;
    int			lightnum;

    if(fakeclip)
    {
	R_StoreWallRange(start, stop);
	return;
    }

    // don't overflow and crash
    if (ds_p == &drawsegs[MAXDRAWSEGS])
	return;		
		
#ifdef RANGECHECK
    if (start >=viewwidth || start > stop)
	I_Error ("Bad R_RenderWallRange: %i to %i", start , stop);
#endif
    
    sidedef = curline->sidedef;
    linedef = curline->linedef;

    // mark the segment as visible for auto map
    linedef->flags |= LF_MAPPED;
    
    // calculate rw_distance for scale calculation
    rw_normalangle = curline->angle + ANG90;
    offsetangle = abs(rw_normalangle-rw_angle1);
    
    if (offsetangle > ANG90)
	offsetangle = ANG90;

    distangle = ANG90 - offsetangle;
    hyp = R_PointToDist (curline->v1->x, curline->v1->y);
    sineval = finesine[distangle>>ANGLETOFINESHIFT];
    rw_distance = FixedMul (hyp, sineval);
		
	
    ds_p->x1 = rw_x = start;
    ds_p->x2 = stop;
    ds_p->curline = curline;
    rw_stopx = stop+1;
    
    // calculate scale at both ends and step
    ds_p->scale1 = rw_scale = 
	R_ScaleFromGlobalAngle (viewangle + xtoviewangle[start]);
    
    if (stop > start )
    {
	ds_p->scale2 = R_ScaleFromGlobalAngle (viewangle + xtoviewangle[stop]);
	ds_p->scalestep = rw_scalestep = 
	    (ds_p->scale2 - rw_scale) / (stop-start);
    }
    else
    {
	ds_p->scale2 = ds_p->scale1;
    }
    
    // calculate texture boundaries
    //  and decide if floor / ceiling marks are needed
    worldtop = *fakeplane->height - viewz;
    worldbottom = worldtop;

    midtexture = toptexture = bottomtexture = maskedtexture = 0;
    ds_p->maskedtexturecol = NULL;

    // calculate incremental stepping values for texture edges
    worldtop >>= 4;
    worldbottom >>= 4;
	
    topstep = -FixedMul (rw_scalestep, worldtop);
    topfrac = (centeryfrac>>4) - FixedMul (worldtop, rw_scale);

    bottomstep = -FixedMul (rw_scalestep,worldbottom);
    bottomfrac = (centeryfrac>>4) - FixedMul (worldbottom, rw_scale);

    // render it
    if(ceilingplane)
    {
	ceilingplane = R_CheckPlane (ceilingplane, rw_x, rw_stopx-1);
	markceiling = true;
    } else
	markceiling = false;
    
    if(floorplane)
    {
	floorplane = R_CheckPlane (floorplane, rw_x, rw_stopx-1);
	markfloor = true;
    } else
	markfloor = false;

    R_RenderSegLoopFake();
}

