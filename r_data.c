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
    boolean		masked;	
    short		width;
    short		height;
//    void		**columndirectory;	// OBSOLETE
	uint32_t obsolete;
    short		patchcount;
    mappatch_t	patches[1];
} maptexture_t;

// [kg] for better looks
boolean r_fakecontrast = true;

// [kg] multiple wads support
int	firstflat[MAXWADS];
int	lastflat[MAXWADS];

int	firstspritelump[MAXWADS];
int	lastspritelump[MAXWADS];

int		numtextures;
texture_t**	textures;


int*			texturewidthmask;
// needed for texture pegging
fixed_t*		textureheight;		
int*			texturecompositesize;
int**			texturecolumnlump;
unsigned  **texturecolumnofs;  // killough 4/9/98: make 32-bit
byte**			texturecomposite;

// for global animation
int*		flattranslation[MAXWADS];
int*		texturetranslation;//[MAXWADS];

lighttable_t	*colormaps;


//
// MAPTEXTURE_T CACHING
// When a texture is first needed,
//  it counts the number of composite columns
//  required in the texture and allocates space
//  for a column directory and any new columns.
// The directory will simply point inside other patches
//  if there is only one patch in a given column,
//  but any columns with multiple patches
//  will have new column_ts generated.
//



//
// R_DrawColumnInCache
// Clip and draw a column
//  from a patch into a cached post.
//
// Rewritten by Lee Killough for performance and to fix Medusa bug
//

static void R_DrawColumnInCache(const column_t *patch, byte *cache,
				int originy, int cacheheight, byte *marks)
{
  while (patch->topdelta != 0xff)
    {
      int count = patch->length;
      int position = originy + patch->topdelta;

      if (position < 0)
        {
          count += position;
          position = 0;
        }

      if (position + count > cacheheight)
        count = cacheheight - position;

      if (count > 0)
        {
          memcpy (cache + position, (byte *)patch + 3, count);

          // killough 4/9/98: remember which cells in column have been drawn,
          // so that column can later be converted into a series of posts, to
          // fix the Medusa bug.

          memset (marks + position, 0xff, count);
        }

      patch = (column_t *)((byte *) patch + patch->length + 4);
    }
}

//
// R_GenerateComposite
// Using the texture definition,
//  the composite texture is created from the patches,
//  and each column is cached.
//
// Rewritten by Lee Killough for performance and to fix Medusa bug

static void R_GenerateComposite(int texnum)
{
  byte *block = Z_Malloc(texturecompositesize[texnum], PU_STATIC,
                         (void **) &texturecomposite[texnum]);
  texture_t *texture = textures[texnum];
  // Composite the columns together.
  texpatch_t *patch = texture->patches;
  int *collump = texturecolumnlump[texnum];
  unsigned *colofs = texturecolumnofs[texnum]; // killough 4/9/98: make 32-bit
  int i = texture->patchcount;
  // killough 4/9/98: marks to identify transparent regions in merged textures
  byte *marks = Z_Calloc(texture->width, texture->height), *source;

  for (; --i >=0; patch++)
    {
      patch_t *realpatch = W_CacheLumpNum(patch->patch);
      int x, x1 = patch->originx, x2 = x1 + SHORT(realpatch->width);
      const int *cofs = realpatch->columnofs - x1;

      if (x1 < 0)
        x1 = 0;
      if (x2 > texture->width)
        x2 = texture->width;
      for (x = x1; x < x2 ; x++)
        if (collump[x] == -1)      // Column has multiple patches?
          // killough 1/25/98, 4/9/98: Fix medusa bug.
          R_DrawColumnInCache((column_t*)((byte*) realpatch + LONG(cofs[x])),
                              block + colofs[x], patch->originy,
			      texture->height, marks + x*texture->height);
    }

  // killough 4/9/98: Next, convert multipatched columns into true columns,
  // to fix Medusa bug while still allowing for transparent regions.

  source = Z_Malloc(texture->height, PU_STATIC, NULL);       // temporary column
  for (i=0; i < texture->width; i++)
    if (collump[i] == -1)                 // process only multipatched columns
      {
        column_t *col = (column_t *)(block + colofs[i] - 3);  // cached column
        const byte *mark = marks + i * texture->height;
        int j = 0;

        // save column in temporary so we can shuffle it around
        memcpy(source, (byte *) col + 3, texture->height);

        for (;;)  // reconstruct the column by scanning transparency marks
          {
	    unsigned len;        // killough 12/98

            while (j < texture->height && !mark[j]) // skip transparent cells
              j++;

            if (j >= texture->height)           // if at end of column
              {
                col->topdelta = -1;             // end-of-column marker
                break;
              }

            col->topdelta = j;                  // starting offset of post

	    // killough 12/98:
	    // Use 32-bit len counter, to support tall 1s multipatched textures

	    for (len = 0; j < texture->height && mark[j]; j++)
              len++;                    // count opaque cells

	    col->length = len; // killough 12/98: intentionally truncate length

            // copy opaque cells from the temporary back into the column
            memcpy((byte *) col + 3, source + col->topdelta, len);
            col = (column_t *)((byte *) col + len + 4); // next post
          }
      }
  Z_Free(source);         // free temporary column
  Z_Free(marks);          // free transparency marks

  // Now that the texture has been built in column cache,
  // it is purgable from zone memory.

  Z_ChangeTag(block, PU_CACHE);
}

//
// R_GenerateLookup
//
// Rewritten by Lee Killough for performance and to fix Medusa bug
//

static void R_GenerateLookup(int texnum, int *const errors)
{
  const texture_t *texture = textures[texnum];

  // Composited texture not created yet.

  int *collump = texturecolumnlump[texnum];
  unsigned *colofs = texturecolumnofs[texnum]; // killough 4/9/98: make 32-bit

  // killough 4/9/98: keep count of posts in addition to patches.
  // Part of fix for medusa bug for multipatched 2s normals.

  struct {
    unsigned patches, posts;
  } *count = Z_Calloc(sizeof *count, texture->width);

  // killough 12/98: First count the number of patches per column.

  const texpatch_t *patch = texture->patches;
  int i = texture->patchcount;
  while (--i >= 0)
    {
      int pat = patch->patch;
      const patch_t *realpatch = W_CacheLumpNum(pat);
      int x, x1 = patch++->originx, x2 = x1 + SHORT(realpatch->width);
      const int *cofs = realpatch->columnofs - x1;
      
      if (x2 > texture->width)
	x2 = texture->width;
      if (x1 < 0)
	x1 = 0;
      for (x = x1 ; x<x2 ; x++)
	{
	  count[x].patches++;
	  collump[x] = pat;
	  colofs[x] = LONG(cofs[x])+3;
	}
    }

  // killough 4/9/98: keep a count of the number of posts in column,
  // to fix Medusa bug while allowing for transparent multipatches.
  //
  // killough 12/98:
  // Post counts are only necessary if column is multipatched,
  // so skip counting posts if column comes from a single patch.
  // This allows arbitrarily tall textures for 1s walls.
  //
  // If texture is >= 256 tall, assume it's 1s, and hence it has
  // only one post per column. This avoids crashes while allowing
  // for arbitrarily tall multipatched 1s textures.

  if (texture->patchcount > 1 && texture->height < 256)
    {
      // killough 12/98: Warn about a common column construction bug
      unsigned limit = texture->height*3+3; // absolute column size limit
      int badcol = devparm;                 // warn only if -devparm used

      for (i = texture->patchcount, patch = texture->patches; --i >= 0;)
	{
	  int pat = patch->patch;
	  const patch_t *realpatch = W_CacheLumpNum(pat);
	  int x, x1 = patch++->originx, x2 = x1 + SHORT(realpatch->width);
	  const int *cofs = realpatch->columnofs - x1;
	  
	  if (x2 > texture->width)
	    x2 = texture->width;
	  if (x1 < 0)
	    x1 = 0;

	  for (x = x1 ; x<x2 ; x++)
	    if (count[x].patches > 1)        // Only multipatched columns
	      {
		const column_t *col =
		  (column_t*)((byte*) realpatch+LONG(cofs[x]));
		const byte *base = (const byte *) col;

		// count posts
		for (;col->topdelta != 0xff; count[x].posts++)
		  if ((unsigned)((byte *) col - base) <= limit)
		    col = (column_t *)((byte *) col + col->length + 4);
		  else
		    { // killough 12/98: warn about column construction bug
		      if (badcol)
			{
			  badcol = 0;
			  printf("\nWarning: Texture %8.8s "
				 "(height %d) has bad column(s)"
				 " starting at x = %d.",
				 texture->name, texture->height, x);
			}
		      break;
		    }
	      }
	}
    }

  // Now count the number of columns
  //  that are covered by more than one patch.
  // Fill in the lump / offset, so columns
  //  with only a single patch are all done.

  texturecomposite[texnum] = 0;

  {
    int x = texture->width;
    int height = texture->height;
    int csize = 0, err = 0;        // killough 10/98

    while (--x >= 0)
      {
	if (!count[x].patches)     // killough 4/9/98
        {
	  if (devparm)
	    {
	      // killough 8/8/98
	      printf("\nR_GenerateLookup:"
		     " Column %d is without a patch in texture %.8s",
		     x, texture->name);
	      ++*errors;
	    }
	  else
	    err = 1;               // killough 10/98
        }
        if (count[x].patches > 1)       // killough 4/9/98
          {
            // killough 1/25/98, 4/9/98:
            //
            // Fix Medusa bug, by adding room for column header
            // and trailer bytes for each post in merged column.
            // For now, just allocate conservatively 4 bytes
            // per post per patch per column, since we don't
            // yet know how many posts the merged column will
            // require, and it's bounded above by this limit.

            collump[x] = -1;              // mark lump as multipatched
            colofs[x] = csize + 3;        // three header bytes in a column
	    // killough 12/98: add room for one extra post
            csize += 4*count[x].posts+5;  // 1 stop byte plus 4 bytes per post
          }
        csize += height;                  // height bytes of texture data
      }

    texturecompositesize[texnum] = csize;
    
    if (err)       // killough 10/98: non-verbose output
      {
	printf("\nR_GenerateLookup: Column without a patch in texture %.8s",
	       texture->name);
	++*errors;
      }
  }
  Z_Free(count);                    // killough 4/9/98
}



//
// R_GetColumn
//
byte*
R_GetColumn
( int		tex,
  int		col )
{
    int		lump;
    int		ofs;
	
    col &= texturewidthmask[tex];
    lump = texturecolumnlump[tex][col];
    ofs = texturecolumnofs[tex][col];
    
    if (lump > 0)
	return (byte *)W_CacheLumpNum(lump)+ofs;

    if (!texturecomposite[tex])
	R_GenerateComposite (tex);

    return texturecomposite[tex] + ofs;
}




void R_InitTextures (void)
{
  maptexture_t *mtexture;
  texture_t    *texture;
  mappatch_t   *mpatch;
  texpatch_t   *patch;
  int  i, j;
  int  *maptex;
  int  *maptex1, *maptex2;
  char name[9];
  char *names;
  char *name_p;
  int  *patchlookup;
  int  totalwidth;
  int  nummappatches;
  int  offset;
  int  maxoff, maxoff2;
  int  numtextures1, numtextures2;
  int  *directory;
  int  errors = 0;

  // Load the patch names from pnames.lmp.
  name[8] = 0;
  names = W_CacheLumpName("PNAMES");
  nummappatches = LONG(*((int *)names));
  name_p = names+4;
  patchlookup = Z_Malloc(nummappatches*sizeof(*patchlookup), PU_STATIC, NULL);  // killough

  for (i=0 ; i<nummappatches ; i++)
    {
      strncpy (name,name_p+i*8, 8);
      patchlookup[i] = W_CheckNumForName(name);
      if (patchlookup[i] == -1)
        {
          // killough 4/17/98:
          // Some wads use sprites as wall patches, so repeat check and
          // look for sprites this time, but only if there were no wall
          // patches found. This is the same as allowing for both, except
          // that wall patches always win over sprites, even when they
          // appear first in a wad. This is a kludgy solution to the wad
          // lump namespace problem.

          patchlookup[i] = (W_CheckNumForName)(name);

          if (patchlookup[i] == -1 && devparm)	    // killough 8/8/98
            printf("\nWarning: patch %.8s, index %d does not exist",name,i);
        }
    }
//  Z_Free(names);

  // Load the map texture definitions from textures.lmp.
  // The data is contained in one or two lumps,
  //  TEXTURE1 for shareware, plus TEXTURE2 for commercial.

  maptex = maptex1 = W_CacheLumpName("TEXTURE1");
  numtextures1 = LONG(*maptex);
  maxoff = W_LumpLength(W_GetNumForName("TEXTURE1"));
  directory = maptex+1;

  if (W_CheckNumForName("TEXTURE2") != -1)
    {
      maptex2 = W_CacheLumpName("TEXTURE2");
      numtextures2 = LONG(*maptex2);
      maxoff2 = W_LumpLength(W_GetNumForName("TEXTURE2"));
    }
  else
    {
      maptex2 = NULL;
      numtextures2 = 0;
      maxoff2 = 0;
    }
  numtextures = numtextures1 + numtextures2;

  // killough 4/9/98: make column offsets 32-bit;
  // clean up malloc-ing to use sizeof

  textures = Z_Malloc(numtextures*sizeof*textures, PU_STATIC, 0);
  texturecolumnlump =
    Z_Malloc(numtextures*sizeof*texturecolumnlump, PU_STATIC, 0);
  texturecolumnofs =
    Z_Malloc(numtextures*sizeof*texturecolumnofs, PU_STATIC, 0);
  texturecomposite =
    Z_Malloc(numtextures*sizeof*texturecomposite, PU_STATIC, 0);
  texturecompositesize =
    Z_Malloc(numtextures*sizeof*texturecompositesize, PU_STATIC, 0);
  texturewidthmask =
    Z_Malloc(numtextures*sizeof*texturewidthmask, PU_STATIC, 0);
  textureheight = Z_Malloc(numtextures*sizeof*textureheight, PU_STATIC, 0);

  totalwidth = 0;

  {  // Really complex printing shit...
    int temp1 = W_GetNumForName("S_START");
    int temp2 = W_GetNumForName("S_END") - 1;

    // 1/18/98 killough:  reduce the number of initialization dots
    // and make more accurate

    int temp3 = 8+(temp2-temp1+255)/128 + (numtextures+255)/128;  // killough
    putchar('[');
    for (i = 0; i < temp3; i++)
      putchar(' ');
    putchar(']');
    for (i = 0; i < temp3; i++)
      putchar('\x8');
  }

  for (i=0 ; i<numtextures ; i++, directory++)
    {
      if (!(i&127))          // killough
        putchar('.');

      if (i == numtextures1)
        {
          // Start looking in second texture file.
          maptex = maptex2;
          maxoff = maxoff2;
          directory = maptex+1;
        }

      offset = LONG(*directory);

      if (offset > maxoff)
        I_Error("R_InitTextures: bad texture directory");

      mtexture = (maptexture_t *) ( (byte *)maptex + offset);

      texture = textures[i] =
        Z_Malloc(sizeof(texture_t) +
                 sizeof(texpatch_t)*(SHORT(mtexture->patchcount)-1),
                 PU_STATIC, 0);

      texture->width = SHORT(mtexture->width);
      texture->height = SHORT(mtexture->height);
      texture->patchcount = SHORT(mtexture->patchcount);

      memcpy(texture->name, mtexture->name, sizeof(texture->name));
      mpatch = mtexture->patches;
      patch = texture->patches;

      for (j=0 ; j<texture->patchcount ; j++, mpatch++, patch++)
        {
          patch->originx = SHORT(mpatch->originx);
          patch->originy = SHORT(mpatch->originy);
          patch->patch = patchlookup[SHORT(mpatch->patch)];
          if (patch->patch == -1)
            {	      // killough 8/8/98
              printf("\nR_InitTextures: Missing patch %d in texture %.8s",
                     SHORT(mpatch->patch), texture->name); // killough 4/17/98
              ++errors;
            }
        }

      // killough 4/9/98: make column offsets 32-bit;
      // clean up malloc-ing to use sizeof
      // killough 12/98: fix sizeofs
      texturecolumnlump[i] =
        Z_Malloc(texture->width*sizeof**texturecolumnlump, PU_STATIC,0);
      texturecolumnofs[i] =
        Z_Malloc(texture->width*sizeof**texturecolumnofs, PU_STATIC,0);

      for (j=1; j*2 <= texture->width; j<<=1)
        ;
      texturewidthmask[i] = j-1;
      textureheight[i] = texture->height<<FRACBITS;

      totalwidth += texture->width;
    }
 
  Z_Free(patchlookup);         // killough

/*  Z_Free(maptex1);
  if (maptex2)
    Z_Free(maptex2);
*/
  if (errors)
    I_Error("\n\n%d errors.", errors);
    
  // Precalculate whatever possible.
  for (i=0 ; i<numtextures ; i++)
    R_GenerateLookup(i, &errors);

  if (errors)
//    I_Error("\n\n%d errors.", errors);
    printf("\n\n%d errors.\n", errors);

  // Create translation table for global animation.
  // killough 4/9/98: make column offsets 32-bit;
  // clean up malloc-ing to use sizeof

  texturetranslation =
    Z_Malloc((numtextures+1)*sizeof*texturetranslation, PU_STATIC, 0);

  for (i=0 ; i<numtextures ; i++)
    texturetranslation[i] = i;

  // killough 1/31/98: Initialize texture hash table
/*  for (i = 0; i<numtextures; i++)
    textures[i]->index = -1;
  while (--i >= 0)
    {
      int j = W_LumpNameHash(textures[i]->name) % (unsigned) numtextures;
      textures[i]->next = textures[j]->index;   // Prepend to chain
      textures[j]->index = i;
    }*/
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

    W_ForEachName("F_START", cb_FirstFlat);
    W_ForEachName("F_END", cb_LastFlat);

    for(i = 0; i < MAXWADS; i++)
    {
	if(lastflat[i] > 0 && firstflat[i] > 0)
	{
		int num = lastflat[i] - firstflat[i] + 1;

		// Create translation table for global animation.
		flattranslation[i] = Z_Malloc (num*sizeof(int), PU_STATIC, 0);

		for(j = 0; j < num; j++)
			flattranslation[i][j] = firstflat[i] + j;
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
	colormaps = W_CacheLumpName("COLORMAP");
/*    int	lump, length;
    
    // Load in the light tables, 
    //  256 byte align tables.
    lump = W_GetNumForName("COLORMAP"); 
    length = W_LumpLength (lump) + 255; 
    colormaps = Z_Malloc (length, PU_STATIC, 0); 
// kgTODO:
//    colormaps = (byte *)( ((int)colormaps + 255)&~0xff); 
    W_ReadLump (lump,colormaps);*/
}



//
// R_InitData
// Locates all the lumps
//  that will be used by all views
// Must be called after W_Init.
//
void R_InitData (void)
{
    R_InitTextures ();
    printf ("\nInitTextures");
    R_InitFlats ();
    printf ("\nInitFlats");
    R_InitSpriteLumps ();
    printf ("\nInitSprites");
    R_InitColormaps ();
    printf ("\nInitColormaps");
}



//
// R_FlatNumForName
// Retrieval, get a flat number for a flat name.
//
static int flat_lump;

int cb_FlatNumForName(int lump)
{
	int wnum = lump >> 24;

	lump &= 0xFFFFFF;
	if(lump >= firstflat[wnum] && lump <= lastflat[wnum])
		flat_lump = (wnum << 24) | lump;
	return false;	
}

int R_FlatNumForName (char* name)
{
	flat_lump = -1;
	W_ForEachName(name, cb_FlatNumForName);

	if(flat_lump == -1)
	{
		printf("R_FlatNumForName: %.8s not found\n", name);
		return 0;
	}
	return flat_lump;
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
		
    for (i=0 ; i<numtextures ; i++)
	if (!strncasecmp (textures[i]->name, name, 8) )
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
//	I_Error ("R_TextureNumForName: %s not found",	 name);
	printf("R_TextureNumForName: %s not found\n", name);
	return 0;
    }
    return i;
}

