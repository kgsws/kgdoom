#include <math.h>

#include "z_zone.h"

#include "m_swap.h"
#include "m_bbox.h"

#include "m_random.h"

#include "g_game.h"

#include "i_system.h"
#include "w_wad.h"

#include "doomdef.h"
#include "p_local.h"

#include "s_sound.h"

#include "doomstat.h"

#include "p_generic.h"
#include "p_inventory.h"
#include "kg_lua.h"
#include "kg_3dfloor.h"
#include "kg_record.h"

void	P_SpawnMapThing (mapthing_hexen_t*	mthing);

int level_lump;
char level_name[9];

//
// MAP related Lookup tables.
// Store VERTEXES, LINEDEFS, SIDEDEFS, etc.
//
int		numvertexes;
vertex_t*	vertexes;

int		numsegs;
seg_t*		segs;

int		numsectors;
sector_t*	sectors;

int		numsubsectors;
subsector_t*	subsectors;

int		numnodes;
node_t*		nodes;

int		numlines;
line_t*		lines;

int		numsides;
side_t*		sides;

int isHexen;

// BLOCKMAP
// Created from axis aligned bounding box
// of the map, a rectangular array of
// blocks of size ...
// Used to speed up collision detection
// by spatial subdivision in 2D.
//
// Blockmap size.
int		bmapwidth;
int		bmapheight;	// size in mapblocks
uint16_t 	*blockmap;	// int for larger maps
// offsets in blockmap are from here
uint16_t 	*blockmaplump;
fixed_t		bmaporgx;
fixed_t		bmaporgy;
// for thing chains
blocklink_t**	blocklinks;		


// REJECT
// For fast sight rejection.
// Speeds up enemy AI by skipping detailed
//  LineOf Sight calculation.
// Without special effect, this could be
//  used as a PVS lookup as well.
//
byte*		rejectmatrix;


// Maintain single and multi player starting spots.
#define MAX_DEATHMATCH_STARTS	10

mapthing_hexen_t	deathmatchstarts[MAX_DEATHMATCH_STARTS];
mapthing_hexen_t*	deathmatch_p;
mapthing_hexen_t	playerstarts[MAXPLAYERS];

boolean is_setup;


extern char	savename[256];
extern  int	skytexture;

//
// P_LoadVertexes
//
void P_LoadVertexes (int lump)
{
    byte*		data;
    int			i;
    mapvertex_t*	ml;
    vertex_t*		li;

    // Determine number of lumps:
    //  total lump length / vertex record length.
    numvertexes = W_LumpLength (lump) / sizeof(mapvertex_t);

    // Allocate zone memory for buffer.
    vertexes = Z_Malloc (numvertexes*sizeof(vertex_t),PU_LEVEL,0);	

    // Load data into cache.
    data = W_CacheLumpNum (lump);
	
    ml = (mapvertex_t *)data;
    li = vertexes;

    // Copy and convert vertex coordinates,
    // internal representation as fixed.
    for (i=0 ; i<numvertexes ; i++, li++, ml++)
    {
	li->x = SHORT(ml->x)<<FRACBITS;
	li->y = SHORT(ml->y)<<FRACBITS;
    }

    // Free buffer memory.
//    Z_Free (data);
}



//
// P_LoadSegs
//
void P_LoadSegs (int lump)
{
    byte*		data;
    int			i;
    mapseg_t*		ml;
    seg_t*		li;
    line_t*		ldef;
    int			linedef;
    uint16_t		side;
	
    numsegs = W_LumpLength (lump) / sizeof(mapseg_t);
    segs = Z_Malloc (numsegs*sizeof(seg_t),PU_LEVEL,0);	
    memset (segs, 0, numsegs*sizeof(seg_t));
    data = W_CacheLumpNum (lump);
	
    ml = (mapseg_t *)data;
    li = segs;
    for (i=0 ; i<numsegs ; i++, li++, ml++)
    {
	uint16_t v1, v2;

	v1 = SHORT(ml->v1);
	v2 = SHORT(ml->v2);

	if(v1 > numvertexes || v2 > numvertexes)
	    I_Error("P_LoadSegs: seg %i uses invalid vertex %i or %i", i, v1, v2);

	li->v1 = &vertexes[v1];
	li->v2 = &vertexes[v2];
					
	li->angle = (SHORT(ml->angle))<<16;
	li->offset = (SHORT(ml->offset))<<16;
	linedef = SHORT(ml->linedef);
	if(linedef > numlines)
	    I_Error("P_LoadSegs: seg %i uses invalid linedef %i", i, linedef);
	ldef = &lines[linedef];
	li->linedef = ldef;
	side = SHORT(ml->side);
	if(side > 1)
	    I_Error("P_LoadSegs: seg %i uses invalid side idx %i", i, side);
	li->sidedef = &sides[ldef->sidenum[side]];
	li->frontsector = sides[ldef->sidenum[side]].sector;
	if (ldef->flags & LF_TWOSIDED)
	    li->backsector = sides[ldef->sidenum[side^1]].sector;
	else
	    li->backsector = 0;
    }
	
//    Z_Free (data);
}


//
// P_LoadSubsectors
//
void P_LoadSubsectors (int lump)
{
    byte*		data;
    int			i;
    mapsubsector_t*	ms;
    subsector_t*	ss;
	
    numsubsectors = W_LumpLength (lump) / sizeof(mapsubsector_t);
    subsectors = Z_Malloc (numsubsectors*sizeof(subsector_t),PU_LEVEL,0);	
    data = W_CacheLumpNum (lump);
	
    ms = (mapsubsector_t *)data;
    memset (subsectors,0, numsubsectors*sizeof(subsector_t));
    ss = subsectors;
    
    for (i=0 ; i<numsubsectors ; i++, ss++, ms++)
    {
	ss->numlines = SHORT(ms->numsegs);
	ss->firstline = SHORT(ms->firstseg);
	if(ss->firstline + ss->numlines > numsegs)
	    I_Error("P_LoadSubsectors: subsector %i uses bad seg range %i + %i", i, ss->firstline, ss->numlines);
    }
	
//    Z_Free (data);
}



//
// P_LoadSectors
//
void P_LoadSectors (int lump)
{
    byte*		data;
    int			i;
    mapsector_t*	ms;
    sector_t*		ss;
	
    numsectors = W_LumpLength (lump) / sizeof(mapsector_t);
    sectors = Z_Malloc (numsectors*sizeof(sector_t),PU_LEVEL,0);	
    memset (sectors, 0, numsectors*sizeof(sector_t));
    data = W_CacheLumpNum (lump);
	
    ms = (mapsector_t *)data;
    ss = sectors;
    for (i=0 ; i<numsectors ; i++, ss++, ms++)
    {
	ss->soundorg.thinker.lua_type = TT_SECTOR;
	ss->floorheight = SHORT(ms->floorheight)<<FRACBITS;
	ss->ceilingheight = SHORT(ms->ceilingheight)<<FRACBITS;
	ss->floorpic = R_FlatNumForName(ms->floorpic);
	ss->ceilingpic = R_FlatNumForName(ms->ceilingpic);
	ss->lightlevel = SHORT(ms->lightlevel);
	ss->special = SHORT(ms->special);
	ss->tag = SHORT(ms->tag);
	ss->thinglist = NULL;
	ss->damage = 0;
	ss->damagetype = 0;
	ss->damagetick = 0;
	ss->flags = 0;
	// [kg] default color
	ss->colormap.lump = colormap_lump;
	ss->colormap.idx = 0;
	ss->colormap.data = colormaps;
    }
	
//    Z_Free (data);
}


//
// P_LoadNodes
//
void P_LoadNodes (int lump)
{
    byte*	data;
    int		i;
    int		j;
    int		k;
    mapnode_t*	mn;
    node_t*	no;
	
    numnodes = W_LumpLength (lump) / sizeof(mapnode_t);

	if(!numnodes)
	{
		I_Error("Empty nodes detected!");
	}

    nodes = Z_Malloc (numnodes*sizeof(node_t),PU_LEVEL,0);	
    data = W_CacheLumpNum (lump);

	if(isHexen && ( *((uint32_t*)data) == 0x444f4e5a || *((uint32_t*)data) == 0x444f4e58 ) )
	{
		I_Error("ZDoom nodes (%c) detected!", *data);
	}
	
    mn = (mapnode_t *)data;
    no = nodes;
    
    for (i=0 ; i<numnodes ; i++, no++, mn++)
    {
	no->x = SHORT(mn->x)<<FRACBITS;
	no->y = SHORT(mn->y)<<FRACBITS;
	no->dx = SHORT(mn->dx)<<FRACBITS;
	no->dy = SHORT(mn->dy)<<FRACBITS;
	for (j=0 ; j<2 ; j++)
	{
	    no->children[j] = SHORT(mn->children[j]);
	    for (k=0 ; k<4 ; k++)
		no->bbox[j][k] = SHORT(mn->bbox[j][k])<<FRACBITS;
	}
    }
	
//    Z_Free (data);
}


//
// P_LoadThings
//
void P_LoadThings (int lump)
{
    byte*		data;
    int			i;
    mapthing_t*		mt;
    int			numthings;
    boolean		spawn;
    mapthing_hexen_t	tmt;
	
    data = W_CacheLumpNum (lump);
    numthings = W_LumpLength (lump) / sizeof(mapthing_t);

    tmt.z = -0x8000;//ONFLOORZ;
	
    mt = (mapthing_t *)data;
    for (i=0 ; i<numthings ; i++, mt++)
    {
	tmt.x = SHORT(mt->x);
	tmt.y = SHORT(mt->y);
	tmt.angle = SHORT(mt->angle);
	tmt.type = SHORT(mt->type);
	tmt.flags = SHORT(mt->options);
	
	P_SpawnMapThing (&tmt);
    }

//    Z_Free (data);
}

// HEXEN version
void P_LoadThings_H (int lump)
{
	byte *data;
	int i;
	mapthing_hexen_t *mt;
	int numthings;
	boolean spawn;

	data = W_CacheLumpNum (lump);
	numthings = W_LumpLength (lump) / sizeof(mapthing_hexen_t);

	mt = (mapthing_hexen_t *)data;
	for (i=0 ; i<numthings ; i++, mt++)
	{
		mt->tid = SHORT(mt->tid);
		mt->x = SHORT(mt->x);
		mt->y = SHORT(mt->y);
		mt->z = SHORT(mt->z);
		mt->angle = SHORT(mt->angle);
		mt->type = SHORT(mt->type);
		mt->flags = SHORT(mt->flags);

		P_SpawnMapThing (mt);
	}

//	Z_Free (data);
}


//
// P_LoadLineDefs
// Also counts secret lines for intermissions.
//
void P_LoadLineDefs (int lump)
{
    byte*		data;
    int			i;
    maplinedef_t*	mld;
    line_t*		ld;
    vertex_t*		v1;
    vertex_t*		v2;
	
    numlines = W_LumpLength (lump) / sizeof(maplinedef_t);
    lines = Z_Malloc (numlines*sizeof(line_t),PU_LEVEL,0);	
    memset (lines, 0, numlines*sizeof(line_t));
    data = W_CacheLumpNum (lump);
	
    mld = (maplinedef_t *)data;
    ld = lines;
    for (i=0 ; i<numlines ; i++, mld++, ld++)
    {
	uint16_t v1idx, v2idx;
	ld->soundorg.thinker.lua_type = TT_LINE;
	ld->flags = SHORT(mld->flags) & 0x01FF;
	ld->special = SHORT(mld->special);
	ld->tag = SHORT(mld->tag);
	v1idx = SHORT(mld->v1);
	v2idx = SHORT(mld->v2);
	if(v1idx > numvertexes || v1idx > numvertexes)
		I_Error("P_LoadLineDefs: line %i uses bad vertex %i or %i", i, v1idx, v2idx);
	v1 = ld->v1 = &vertexes[v1idx];
	v2 = ld->v2 = &vertexes[v2idx];
	ld->dx = v2->x - v1->x;
	ld->dy = v2->y - v1->y;

	if (!ld->dx)
	    ld->slopetype = ST_VERTICAL;
	else if (!ld->dy)
	    ld->slopetype = ST_HORIZONTAL;
	else
	{
	    if (FixedDiv (ld->dy , ld->dx) > 0)
		ld->slopetype = ST_POSITIVE;
	    else
		ld->slopetype = ST_NEGATIVE;
	}
		
	if (v1->x < v2->x)
	{
	    ld->bbox[BOXLEFT] = v1->x;
	    ld->bbox[BOXRIGHT] = v2->x;
	}
	else
	{
	    ld->bbox[BOXLEFT] = v2->x;
	    ld->bbox[BOXRIGHT] = v1->x;
	}

	if (v1->y < v2->y)
	{
	    ld->bbox[BOXBOTTOM] = v1->y;
	    ld->bbox[BOXTOP] = v2->y;
	}
	else
	{
	    ld->bbox[BOXBOTTOM] = v2->y;
	    ld->bbox[BOXTOP] = v1->y;
	}

	ld->sidenum[0] = SHORT(mld->sidenum[0]);
	ld->sidenum[1] = SHORT(mld->sidenum[1]);

	if((uint16_t)ld->sidenum[0] > numsides)
	    I_Error("P_LoadLineDefs: line %i uses invalid front sidedef %i", i, ld->sidenum[0]);
	ld->frontsector = sides[ld->sidenum[0]].sector;

	if (ld->sidenum[1] != -1)
	{
	    if((uint16_t)ld->sidenum[1] > numsides)
		I_Error("P_LoadLineDefs: line %i uses invalid back sidedef %i", i, ld->sidenum[1]);
	    ld->backsector = sides[ld->sidenum[1]].sector;
	} else
	{
	    ld->backsector = 0;
	    ld->flags &= ~LF_TWOSIDED;
	}

	// [kg] blocking
	ld->blocking = 0;
	if(!ld->backsector)
	    ld->blocking = 0xFFFF;
	if(ld->flags & LF_BLOCKING)
	    ld->blocking |= 3; // players and monsters
	if(ld->flags & LF_BLOCKMONSTERS)
	    ld->blocking |= 2; // monsters

	// [kg] soundorg
	ld->soundorg.x = ((v2->x - v1->x) / 2) + v1->x;
	ld->soundorg.y = ((v2->y - v1->y) / 2) + v1->y;
    }
	
//    Z_Free (data);
}

// HEXEN version
void P_LoadLineDefs_H(int lump)
{
	byte *data;
	int i;
	maplinedef_hexen_t *mld;
	line_t *ld;
	vertex_t *v1;
	vertex_t *v2;

	numlines = W_LumpLength (lump) / sizeof(maplinedef_hexen_t);
	lines = Z_Malloc (numlines*sizeof(line_t),PU_LEVEL,0);	
	memset (lines, 0, numlines*sizeof(line_t));
	data = W_CacheLumpNum (lump);

	mld = (maplinedef_hexen_t *)data;
	ld = lines;
	for (i=0 ; i<numlines ; i++, mld++, ld++)
	{
		uint16_t v1idx, v2idx;
		ld->soundorg.thinker.lua_type = TT_LINE;
		ld->flags = SHORT(mld->flags);
		ld->special = mld->special;
		memcpy(ld->arg, mld->arg, 5);
		v1idx = SHORT(mld->v1);
		v2idx = SHORT(mld->v2);
		if(v1idx > numvertexes || v1idx > numvertexes)
			I_Error("P_LoadLineDefs: line %i uses bad vertex %i or %i", i, v1idx, v2idx);
		v1 = ld->v1 = &vertexes[v1idx];
		v2 = ld->v2 = &vertexes[v2idx];
		ld->dx = v2->x - v1->x;
		ld->dy = v2->y - v1->y;

		if (!ld->dx)
			ld->slopetype = ST_VERTICAL;
		else if (!ld->dy)
			ld->slopetype = ST_HORIZONTAL;
		else
		{
			if (FixedDiv (ld->dy , ld->dx) > 0)
				ld->slopetype = ST_POSITIVE;
			else
				ld->slopetype = ST_NEGATIVE;
		}

		if (v1->x < v2->x)
		{
			ld->bbox[BOXLEFT] = v1->x;
			ld->bbox[BOXRIGHT] = v2->x;
		}
		else
		{
			ld->bbox[BOXLEFT] = v2->x;
			ld->bbox[BOXRIGHT] = v1->x;
		}

		if (v1->y < v2->y)
		{
			ld->bbox[BOXBOTTOM] = v1->y;
			ld->bbox[BOXTOP] = v2->y;
		}
		else
		{
			ld->bbox[BOXBOTTOM] = v2->y;
			ld->bbox[BOXTOP] = v1->y;
		}

		ld->sidenum[0] = SHORT(mld->sidenum[0]);
		ld->sidenum[1] = SHORT(mld->sidenum[1]);

		if((uint16_t)ld->sidenum[0] > numsides)
			I_Error("P_LoadLineDefs: line %i uses invalid front sidedef %i", i, ld->sidenum[0]);
		ld->frontsector = sides[ld->sidenum[0]].sector;

		if (ld->sidenum[1] != -1)
		{
			if((uint16_t)ld->sidenum[1] > numsides)
				I_Error("P_LoadLineDefs: line %i uses invalid back sidedef %i", i, ld->sidenum[1]);
			ld->backsector = sides[ld->sidenum[1]].sector;
		} else
		{
			ld->backsector = 0;
			ld->flags &= ~LF_TWOSIDED;
		}

		// [kg] blocking
		ld->blocking = 0;
		if(!ld->backsector)
		    ld->blocking = 0xFFFF;
		if(ld->flags & ELF_TOTAL_BLOCK)
		    ld->blocking |= 15; // players and monsters and shooting
		if(ld->flags & LF_BLOCKING)
		    ld->blocking |= 3; // players and monsters
		if(ld->flags & LF_BLOCKMONSTERS)
		    ld->blocking |= 2; // monsters
		if(ld->flags & ELF_BLOCK_PLAYER)
		    ld->blocking |= 1; // players

		// [kg] soundorg
		ld->soundorg.x = ((v2->x - v1->x) / 2) + v1->x;
		ld->soundorg.y = ((v2->y - v1->y) / 2) + v1->y;
	}

//	Z_Free (data);
}


//
// P_LoadSideDefs
//
void P_LoadSideDefs (int lump)
{
    byte*		data;
    int			i;
    mapsidedef_t*	msd;
    side_t*		sd;
	
    numsides = W_LumpLength (lump) / sizeof(mapsidedef_t);
    sides = Z_Malloc (numsides*sizeof(side_t),PU_LEVEL,0);	
    memset (sides, 0, numsides*sizeof(side_t));
    data = W_CacheLumpNum (lump);
	
    msd = (mapsidedef_t *)data;
    sd = sides;
    for (i=0 ; i<numsides ; i++, msd++, sd++)
    {
	uint16_t sec;
	sd->textureoffset = SHORT(msd->textureoffset)<<FRACBITS;
	sd->rowoffset = SHORT(msd->rowoffset)<<FRACBITS;
	sd->toptexture = R_TextureNumForName(msd->toptexture);
	sd->bottomtexture = R_TextureNumForName(msd->bottomtexture);
	sd->midtexture = R_TextureNumForName(msd->midtexture);
	sec = SHORT(msd->sector);
	if(sec > numsectors)
	    I_Error("P_LoadSideDefs: side %i used invalid sector %i", i, sec);
	sd->sector = &sectors[sec];
    }
	
//    Z_Free (data);
}


//
// P_LoadBlockMap
//
void P_LoadBlockMap (int lump)
{
	int		i;
	int		count, numblocks;
	uint16_t	*tmpblock;

	tmpblock = W_CacheLumpNum (lump);
	numblocks = W_LumpLength (lump) / sizeof(uint16_t);
	blockmaplump = Z_Malloc(numblocks * sizeof(uint16_t), PU_LEVEL, NULL);
	blockmap = blockmaplump+4;

	for (i=0 ; i < numblocks ; i++)
		blockmaplump[i] = SHORT(tmpblock[i]);

	bmaporgx = blockmaplump[0]<<FRACBITS;
	bmaporgy = blockmaplump[1]<<FRACBITS;
	bmapwidth = blockmaplump[2];
	bmapheight = blockmaplump[3];

	// clear out mobj chains
	count = sizeof(blocklink_t*) * bmapwidth * bmapheight;
	blocklinks = Z_Malloc (count,PU_LEVEL, 0);
	memset (blocklinks, 0, count);

	// [kg] check blockmap validity
	count = bmapwidth * bmapheight;
	tmpblock = blockmap;
	for(i = 0; i < count; i++)
	{
		uint16_t *list;
		if(*tmpblock > numblocks)
			I_Error("P_LoadBlockMap: block index %i out of bounds %i", *tmpblock, numblocks);
		for(list = blockmaplump + *tmpblock; *list != 0xFFFF; list++)
		{
			if(list > blockmaplump + numblocks)
				I_Error("P_LoadBlockMap: line index %i out of bounds %i", (int)(list - blockmaplump), numblocks);
			if(*list > numlines)
				I_Error("P_LoadBlockMap: invalid line %i", *list);
		}
		tmpblock++;
	}
}



//
// P_GroupLines
// Builds sector line lists and subsector sector numbers.
// Finds block bounding boxes for sectors.
//
void P_GroupLines (void)
{
    line_t**		linebuffer;
    int			i;
    int			j;
    int			total;
    line_t*		li;
    sector_t*		sector;
    subsector_t*	ss;
    seg_t*		seg;
    fixed_t		bbox[4];
    int			block;
	
    // look up sector number for each subsector
    ss = subsectors;
    for (i=0 ; i<numsubsectors ; i++, ss++)
    {
	seg = &segs[ss->firstline];
	ss->sector = seg->sidedef->sector;
    }

    // count number of lines in each sector
    li = lines;
    total = 0;
    for (i=0 ; i<numlines ; i++, li++)
    {
	total++;
	li->frontsector->linecount++;

	if (li->backsector && li->backsector != li->frontsector)
	{
	    li->backsector->linecount++;
	    total++;
	}
    }
	
    // build line tables for each sector	
    linebuffer = Z_Malloc (total*sizeof(line_t*), PU_LEVEL, 0);
    sector = sectors;
    for (i=0 ; i<numsectors ; i++, sector++)
    {
	M_ClearBox (bbox);
	sector->lines = linebuffer;
	li = lines;
	for (j=0 ; j<numlines ; j++, li++)
	{
	    if (li->frontsector == sector || li->backsector == sector)
	    {
		*linebuffer++ = li;
		M_AddToBox (bbox, li->v1->x, li->v1->y);
		M_AddToBox (bbox, li->v2->x, li->v2->y);
	    }
	}
	if (linebuffer - sector->lines != sector->linecount)
	    I_Error ("P_GroupLines: miscounted");
			
	// set the degenmobj_t to the middle of the bounding box
	sector->soundorg.x = (bbox[BOXRIGHT]+bbox[BOXLEFT])/2;
	sector->soundorg.y = (bbox[BOXTOP]+bbox[BOXBOTTOM])/2;
		
	// adjust bounding box to map blocks
	block = (bbox[BOXTOP]-bmaporgy)>>MAPBLOCKSHIFT;
	block = block >= bmapheight ? bmapheight-1 : block;
	sector->blockbox[BOXTOP]=block;

	block = (bbox[BOXBOTTOM]-bmaporgy)>>MAPBLOCKSHIFT;
	block = block < 0 ? 0 : block;
	sector->blockbox[BOXBOTTOM]=block;

	block = (bbox[BOXRIGHT]-bmaporgx)>>MAPBLOCKSHIFT;
	block = block >= bmapwidth ? bmapwidth-1 : block;
	sector->blockbox[BOXRIGHT]=block;

	block = (bbox[BOXLEFT]-bmaporgx)>>MAPBLOCKSHIFT;
	block = block < 0 ? 0 : block;
	sector->blockbox[BOXLEFT]=block;
    }
	
}


//
// P_SetupLevel
//
void
P_SetupLevel
( int		episode,
  int		map,
  int		playermask,
  skill_t	skill)
{
    int		i;
    char	lumpname[9];
    int		lumpnum;
    thinker_t *think;

    is_setup = true;
	
    totalkills = totalitems = totalsecret = wminfo.maxfrags = 0;
    wminfo.partime = 180;
    for (i=0 ; i<MAXPLAYERS ; i++)
    {
	players[i].killcount = 0;
	players[i].secretcount = 0;
	players[i].itemcount = 0;
	players[i].mo = NULL;
    }

    // Initial height of PointOfView
    // will be set by player think.
    players[consoleplayer].viewz = 1; 

    // Make sure all sounds are stopped before Z_FreeTags.
    S_Start ();			

    // [kg] cleanup old thinkers
    if(thinkercap.next)
    for(think = thinkercap.next; think != &thinkercap; think = think->next)
    {
	switch(think->lua_type)
	{
		case TT_MOBJ:
			((mobj_t*)think)->player = NULL;
			P_RemoveInventory((mobj_t*)think);
			P_RemoveMobjTickers((mobj_t*)think);
		break;
		case TT_GENPLANE:
		case TT_SECCALL:
			L_FinishGeneric((generic_plane_t*)think, true);
		break;
	}
    }

    // [kg] cleanup any 3D planes
    e3d_CleanPlanes();

    Z_FreeTags (PU_LEVEL, PU_PURGELEVEL-1);

    // UNUSED W_Profile ();
    P_InitThinkers ();

    if(level_name[0])
    {
	// [kg] custom map name
	memcpy(lumpname, level_name, 8);
    } else
    {
	// find map name
	if ( gamemode == commercial)
	{
	    sprintf(lumpname,"MAP%02i", map);
	    gameepisode = 0;
	}
	else
	{
	    lumpname[0] = 'E';
	    lumpname[1] = '0' + episode;
	    lumpname[2] = 'M';
	    lumpname[3] = '0' + map;
	    lumpname[4] = 0;
	}
    }

    lumpnum = W_GetNumForName (lumpname);

    level_lump = lumpnum;

    // [kg] game saving / loading

#ifndef SERVER
    if(gameaction == ga_loadgame)
    {
	rec_load(savename, 2);
	lumpnum = level_lump;
    }
    if(!rec_is_playback)
    {
#endif
	// [kg] new random
	M_ClearRandom();
#ifndef SERVER
	// [kg] prepare recording / savegame
	if(!netgame)
	    rec_reset();
    }
#endif

    // DOOM determines the sky texture to be used
    // depending on the current episode, and the game version.
    // set the sky map for the episode
    if ( gamemode == commercial)
    {
	skytexture = R_TextureNumForName ("SKY3");
	if (gamemap < 12)
	    skytexture = R_TextureNumForName ("SKY1");
	else
	    if (gamemap < 21)
		skytexture = R_TextureNumForName ("SKY2");
    }
    else
	switch (episode) 
	{ 
	  case 1: 
	    skytexture = R_TextureNumForName ("SKY1"); 
	    break; 
	  case 2: 
	    skytexture = R_TextureNumForName ("SKY2"); 
	    break; 
	  case 3: 
	    skytexture = R_TextureNumForName ("SKY3"); 
	    break; 
	  case 4:	// Special Edition sky
	    skytexture = R_TextureNumForName ("SKY4");
	    break;
	} 

    // [kg] continue loading

    {
	char *tmp = W_LumpNumName(level_lump);
	level_name[8] = 0;
	strncpy(level_name, tmp, 8);
    }
	
    leveltime = 0;

	{
		void *ptr = W_LumpNumName(lumpnum+ML_BEHAVIOR);
		if(ptr && *((uint64_t*)ptr) == 0x524f495641484542)
		{
			isHexen = 1;
			printf("%s; HEXEN MAP FORMAT\n", level_name);
		} else {
			isHexen = 0;
			printf("%s; DOOM MAP FORMAT\n", level_name);
		}
	}
	
    // note: most of this ordering is important	
    P_LoadVertexes (lumpnum+ML_VERTEXES);
    P_LoadSectors (lumpnum+ML_SECTORS);
    P_LoadSideDefs (lumpnum+ML_SIDEDEFS);

    if(isHexen)
	P_LoadLineDefs_H(lumpnum+ML_LINEDEFS);
    else
	P_LoadLineDefs(lumpnum+ML_LINEDEFS);

    P_LoadBlockMap (lumpnum+ML_BLOCKMAP);
    P_LoadNodes (lumpnum+ML_NODES);
    P_LoadSegs (lumpnum+ML_SEGS);
    P_LoadSubsectors (lumpnum+ML_SSECTORS);
	
    rejectmatrix = W_CacheLumpNum (lumpnum+ML_REJECT);
    P_GroupLines ();

    bodyqueslot = 0;
    deathmatch_p = deathmatchstarts;

    if(isHexen)
	P_LoadThings_H(lumpnum+ML_THINGS);
    else
	P_LoadThings(lumpnum+ML_THINGS);
    
    // if deathmatch, randomly spawn the active players
    if (sv_deathmatch)
    {
	for (i=0 ; i<MAXPLAYERS ; i++)
	    if (playeringame[i])
	    {
		players[i].mo = NULL;
		G_DeathMatchSpawnPlayer (i);
	    }
			
    }

#ifndef SERVER
    // [kg] multiplayer spectator spawn
    if(netgame)
    {
	if(!players[consoleplayer].mo)
	{
		int count = (int)(deathmatch_p - deathmatchstarts);
		if(!count)
			I_Error("P_SpawnPlayer: no spectator spawns");
		// [kg] alternative spectator spawn
		P_SpawnPlayer(&deathmatchstarts[rand() % count], -1);
	}
	players[consoleplayer].mo->health = 0;
    }
#endif

    // set up world state
    P_SpawnSpecials ();
	
    // build subsector connect matrix
    //	UNUSED P_ConnectSubsectors ();

    //printf ("free memory: 0x%x\n", Z_FreeMemory());

    is_setup = false;

    // [kg] do Lua stuff now
    L_SetupMap();
}



//
// P_Init
//
void P_Init (void)
{
    P_InitSwitchList ();
    P_InitPicAnims ();
#ifdef SERVER
    R_InitSpriteDefs();
#else
    R_InitSprites();
#endif
}

