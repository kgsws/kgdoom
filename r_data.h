#ifndef __R_DATA__
#define __R_DATA__

#include "r_defs.h"
#include "r_state.h"

#ifdef __GNUG__
#pragma interface
#endif

// [kg] default white color, black fog
int colormap_lump;

// [kg] rewrite for 2D array storage
// palette index 0 can be transparent
typedef struct
{
	char name[8];		
	uint16_t width;
	uint16_t height;

	uint8_t material;
	uint8_t flags;
	uint8_t scalex;
	uint8_t scaley;

	uint8_t *data;
} texture_t;

extern int numtextures;
extern texture_t *textures;

extern int flatstart;

extern uint8_t transpixel;

// [kg] better looks
extern boolean r_fakecontrast;

// Retrieve column data for span blitting.
byte*
R_GetColumn
( int		tex,
  int		col );


// I/O, setting up the stuff.
void R_InitData (void);
void R_PrecacheLevel (void);


// Retrieval.
// Floor/ceiling opaque texture tiles,
// lookup by name. For animation?
int R_FlatNumForName (char* name);


// Called by P_Ticker for switches and animations,
// returns the texture number for the texture name.
int R_TextureNumForName (char *name);
int R_CheckTextureNumForName (char *name);

#endif

