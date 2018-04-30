#include "doomdef.h"

#include "i_system.h"
#include "z_zone.h"

#include "m_swap.h"

#include "w_wad.h"

#include "doomdef.h"
#include "r_local.h"
#include "p_local.h"

#include "doomstat.h"
#include "r_sky.h"

#ifdef LINUX
#include  <alloca.h>
#endif


#include "r_data.h"

//
// Graphics.
// DOOM graphics for walls and sprites
// is stored in vertical runs of opaque pixels (posts).
// A column is composed of zero or more posts,
// a patch or sprite is composed of zero or more columns.
// 

// [kg] complete rewrite of texture storage
// all textures are now stored as 2D array - no more multiple posts
// palette index 0 can be transparent

//
// Texture definition.
// Each texture is composed of one or more patches,
// with patches being lumps stored in the WAD.
// The lumps are referenced by number, and patched
// into the rectangular texture space using origin
// and possibly other attributes.
//
typedef struct
{
    short	originx;
    short	originy;
    short	patch;
    short	stepdir;
    short	colormap;
} mappatch_t;


//
// Texture definition.
// A DOOM wall texture is a list of patches
// which are to be combined in a predefined order.
//
typedef struct __attribute__((packed))
{
    char		name[8];
    uint8_t		material;	// [kg] custom texture materials
    uint8_t		flags;		// [kg] unused; ZDoom has only 0x80 flag
    uint8_t		scalex;		// [kg] TODO: as in ZDoom
    uint8_t		scaley;		// [kg] TODO: as in ZDoom
    short		width;
    short		height;
    uint32_t		obsolete;	// columndirectory
    short		patchcount;
    mappatch_t	patches[];
} maptexture_t;

typedef struct __attribute__((packed))
{
	uint32_t count;
	uint32_t index[];
} mtexdir_t;

typedef struct
{
	char name[8];
} onepname_t;

typedef struct __attribute__((packed))
{
	uint32_t count;
	onepname_t list[];
} pnames_t;

// [kg] for better looks
boolean r_fakecontrast = true;

// [kg] multiple wads support
int	firstflat[MAXWADS];
int	lastflat[MAXWADS];

int	firstspritelump[MAXWADS];
int	lastspritelump[MAXWADS];

int		numflats;
int		flatstart;
int		numtextures;
texture_t*	textures;

pnames_t *pnames;

uint8_t transpixel;
uint8_t *pixelstorage;
uint8_t *pixelpos;

// for global animation
int *texturetranslation;

lighttable_t	*colormaps;

static void R_GenerateComposite(int texnum)
{
}

//
// R_GetColumn
//
byte*
R_GetColumn
( int		tex,
  int		col )
{
	texture_t *tx = &textures[tex];
	if(col < 0)
		col += (((-col) + (tx->width - 1)) / tx->width) * tx->width;
	return tx->data + tx->height * (col % tx->width);
}

// [kg] render patch to texture
void R_RenderToTexture(int w, int h, int x, int y, uint8_t *dst, patch_t *patch)
{
	int start;
	int pw = SHORT(patch->width);
	int ph = SHORT(patch->height);

	if(x > w)
		return;
	if(y > h)
		return;
	if(x + pw < 0)
		return;
	if(y + ph < 0)
		return;

	if(x < 0)
	{
		start = -x;
		x = 0;
		pw -= start;
	} else
		start = 0;

	if(x + pw > w)
		pw -= (x + pw) - w;

	while(pw--)
	{
		column_t *column = (column_t *)((byte *)patch + LONG(patch->columnofs[start]));
		while(column->topdelta != 0xff)
		{
			int tpos = column->topdelta + y;
			uint8_t *source = (byte *)column + 3;
			uint8_t *dest = (dst + x * h) + tpos;
			int count = column->length;
			while(count--)
			{
				if(tpos < h && tpos >= 0)
				{
					if(*source)
						*dest = *source;
					else
						*dest = transpixel;
				}
				source++;
				dest++;
				tpos++;
			}
			column = (column_t *)(  (byte *)column + column->length + 4 );
		}
		start++;
		x++;
	}
}

// [kg] parse TEXTUREx lump, fill array
int R_PrepareTextures(mtexdir_t *tdir, int idx)
{
	int i, j;
	int count = tdir->count;
	maptexture_t *mtex;

	for(i = 0; i < count; i++, idx++)
	{
		mtex = ((void*)tdir) + tdir->index[i];
		memcpy(textures[idx].name, mtex->name, 8);
		textures[idx].width = mtex->width;
		textures[idx].height = mtex->height;
		textures[idx].material = mtex->material;
		textures[idx].flags = mtex->flags;
		textures[idx].scalex = mtex->scalex;
		textures[idx].scaley = mtex->scaley;
		textures[idx].data = pixelpos;
		Z_Enlarge(pixelstorage, mtex->width * mtex->height);
		// render texture
		memset(pixelpos, 0, mtex->width * mtex->height);
		for(j = 0; j < mtex->patchcount; j++)
		{
			if(mtex->patches[j].patch > pnames->count)
				I_Error("invalid patch in texture texture %.8s", mtex->name);
			R_RenderToTexture(mtex->width, mtex->height, mtex->patches[j].originx, mtex->patches[j].originy, pixelpos, W_CacheLumpName(pnames->list[mtex->patches[j].patch].name));
		}
		pixelpos += mtex->width * mtex->height;
	}

	return idx;
}

// [kg] parse and allocate memory for texture array
void R_InitTextures (void)
{
	int i, tmp, idx;
	mtexdir_t *tdir;
	uint8_t *pal;
	uint8_t *pac;

	// locate duplicate color in palette
	pal = W_CacheLumpName("PLAYPAL");
	pac = pal + 3;
	for(i = 1; i < 256; i++, pac += 3)
	{
		if(pal[0] == pac[0] && pal[1] == pac[1] && pal[2] == pac[2])
		{
			transpixel = i;
			break;
		}
	}

	// get patch names
	pnames = W_CacheLumpName("PNAMES");

	// count textures
	tdir = W_CacheLumpName("TEXTURE1");
	numtextures = tdir->count;
	i = W_CheckNumForName("TEXTURE2");
	if(i >= 0)
	{
		tdir = W_CacheLumpNum(i);
		numtextures += tdir->count;
	}
	// add flats amount and store first flat offset
	flatstart = numtextures;
	numtextures += numflats;

	// default animation table
	texturetranslation = Z_Malloc(numtextures * sizeof(int), PU_STATIC, NULL);
	for(i = 0; i < numtextures; i++)
		texturetranslation[i] = i;

	// texture list
	textures = Z_Malloc(numtextures * sizeof(texture_t), PU_STATIC, NULL);

	// prepare pixel storage - dummy allocation
	// this zone will be enlarged and filled with texture data
	pixelstorage = Z_Malloc(4, PU_STATIC, NULL);
	pixelpos = pixelstorage + 4;

	// add textures
	idx = R_PrepareTextures(W_CacheLumpName("TEXTURE1"), 0);
	i = W_CheckNumForName("TEXTURE2");
	if(i >= 0)
		idx = R_PrepareTextures(W_CacheLumpNum(i), idx);

	// add flats
	for(i = 0; i < MAXWADS; i++)
	{
		if(lastflat[i] > 0 && firstflat[i] > 0)
		{
			int num = lastflat[i] - firstflat[i] + 1;
			for(tmp = 0; tmp < num; tmp++)
			{
				int lump = (i << 24) | (firstflat[i] + tmp);
				memcpy(textures[idx].name, W_LumpNumName(lump), 8);
				textures[idx].width = 64;
				textures[idx].height = 64;
				textures[idx].material = 0;
				textures[idx].flags = 0;
				textures[idx].scalex = 8;
				textures[idx].scaley = 8;
				textures[idx].data = pixelpos;
				Z_Enlarge(pixelstorage, 64 * 64);
				memcpy(pixelpos, W_CacheLumpNum(lump), 64 * 64);
				pixelpos += 64 * 64;
				idx++;
			}
		}
	}
}

//
// R_InitFlats
//
static boolean cb_FirstFlat(int lump)
{
	firstflat[lump >> 24] = (lump & 0xFFFFFF) + 1;
	return false;
}

static boolean cb_LastFlat(int lump)
{
	lastflat[lump >> 24] = (lump & 0xFFFFFF) - 1;
	return false;
}

void R_InitFlats (void)
{
    int i, j, f;

    for(i = 0; i < MAXWADS; i++)
    {
	firstflat[i] = -1;
	lastflat[i] = -1;
    }

    // normal lumps
    W_ForEachName("F_START", cb_FirstFlat);
    W_ForEachName("F_END", cb_LastFlat);

    for(i = 0; i < MAXWADS; i++)
    {
	if(lastflat[i] > 0 && firstflat[i] > 0)
	{
		int num = lastflat[i] - firstflat[i] + 1;
		if(num > 0)
			numflats += num;
	}
    }
}


//
// R_InitSpriteLumps
//
static boolean cb_FirstSprite(int lump)
{
	firstspritelump[lump >> 24] = (lump & 0xFFFFFF) + 1;
	return false;
}

static boolean cb_LastSprite(int lump)
{
	lastspritelump[lump >> 24] = (lump & 0xFFFFFF) - 1;
	return false;
}

void R_InitSpriteLumps (void)
{
    int i, j, f;

    for(i = 0; i < MAXWADS; i++)
    {
	firstspritelump[i] = -1;
	lastspritelump[i] = -1;
    }

    W_ForEachName("S_START", cb_FirstSprite);
    W_ForEachName("S_END", cb_LastSprite);

/*    int		i;
    patch_t	*patch;
	
    firstspritelump = W_GetNumForName ("S_START") + 1;
    lastspritelump = W_GetNumForName ("S_END") - 1;
    
    numspritelumps = lastspritelump - firstspritelump + 1;
	
    for (i=0 ; i< numspritelumps ; i++)
    {
	if (!(i&63))
	    printf (".");

	patch = W_CacheLumpNum (firstspritelump+i);
	spritewidth[i] = SHORT(patch->width)<<FRACBITS;
	spriteoffset[i] = SHORT(patch->leftoffset)<<FRACBITS;
	spritetopoffset[i] = SHORT(patch->topoffset)<<FRACBITS;
    }*/
}



//
// R_InitColormaps
//
void R_InitColormaps (void)
{
	colormap_lump = W_GetNumForName("COLORMAP");
	colormaps = W_CacheLumpNum(colormap_lump);
}


//
// R_InitData
// Locates all the lumps
//  that will be used by all views
// Must be called after W_Init.
//
void R_InitData (void)
{
    R_InitFlats ();
    printf ("\nInitFlats");
    R_InitTextures ();
    printf ("\nInitTextures");
    R_InitSpriteLumps ();
    printf ("\nInitSprites");
    R_InitColormaps ();
    printf ("\nInitColormaps");
}



//
// R_FlatNumForName
// Retrieval, get a flat number for a flat name.
//

int R_FlatNumForName (char* name)
{
	int i;

	if(name[0] == '-' && name[1] == 0)
		return 0;

	for(i = 0; i < numflats; i++)
		if(!strncasecmp(textures[flatstart + i].name, name, 8))
			return flatstart + i;

	// atempt to use wall textures
	if(isHexen)
	{
		for(i = 0; i < flatstart; i++)
			if(!strncasecmp(textures[i].name, name, 8))
				return i;
	}

	printf("R_FlatNumForName: %.8s not found\n", name);
	return 0;
}


//
// R_CheckTextureNumForName
// Check whether texture is available.
// Filter out NoTexture indicator.
//
int	R_CheckTextureNumForName (char *name)
{
    int		i;

    // "NoTexture" marker.
    if (name[0] == '-')
	return 0;

    for(i = 0; i < flatstart; i++)
	if(!strncasecmp(textures[i].name, name, 8))
	    return i;

    return -1;
}



//
// R_TextureNumForName
// Calls R_CheckTextureNumForName,
//  aborts with error message.
//
int	R_TextureNumForName (char* name)
{
	int		i;

	i = R_CheckTextureNumForName (name);

	if (i==-1)
	{
		// atempt to use flat textures
		if(isHexen)
		{
			for(i = 0; i < numflats; i++)
				if(!strncasecmp(textures[flatstart + i].name, name, 8))
					return flatstart + i;
		}
		printf("R_TextureNumForName: %.8s not found\n", name);
		return 0;
	}
	return i;
}

