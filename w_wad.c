#include "doomdef.h"
#include "doomstat.h"
#include "doomtype.h"
#include "m_swap.h"
#include "i_system.h"
#include "z_zone.h"

#ifdef __GNUG__
#pragma implementation "w_wad.h"
#endif
#include "w_wad.h"

#include "t_text.h"

// [kg] rewritten lump handling, WADs are in memory

//
// GLOBALS
//

// Location of each lump on disk.

lumpinfo_t*	lumpinfo[MAXWADS];
int		numlumps[MAXWADS];
void		*wadbuf[MAXWADS];
int		numwads;

//
// LUMP BASED ROUTINES.
//

#define LOADSIZE	393216

void W_LoadWad(const char *name)
{
	// TODO: WAD checks (size, lumps, ID)

	if(numwads == MAXWADS)
		I_Error("W_Init: too many wads");

	FILE *f = fopen(name, "rb");

	if(!f)
		I_Error("W_Init: can't open WAD '%s'", name);

	wadbuf[numwads] = Z_Malloc(LOADSIZE, PU_STATIC, NULL);

	{
		uint8_t *dst = wadbuf[numwads];
		printf("%s:\n", name);
#ifdef LINUX
		fflush(stdout);
#endif
		while(1)
		{
			int got;
			got = fread(dst, 1, LOADSIZE, f);
			dst += got;
#ifdef VIDEO_STDOUT
			T_PutChar('.');
			I_FinishUpdate();
#endif
			if(got < LOADSIZE)
				break;
			Z_Enlarge(wadbuf[numwads], LOADSIZE);
		}
		T_PutChar('\n');
	}

	fclose(f);

	// [kg] assuming included WAD is real and correct
	wadinfo_t *info = (void*)wadbuf[numwads];

	lumpinfo[numwads] = (void*)(wadbuf[numwads] + info->offs);
	numlumps[numwads] = info->numlumps;
	numwads++;
}

//
// W_CheckNumForName
// Returns -1 if name not found.
//

#ifdef LINUX
void strupr (char* s)
{
    while (*s) { *s = toupper(*s); s++; }
}
#endif

int W_CheckNumForName (char* name)
{
	union
	{
		char s[9];
		uint64_t x;
	} name8;

	uint64_t v1;
	int wnum;

	// make the name into two integers for easy compares
	strncpy (name8.s,name,8);

	// in case the name was a fill 8 chars
	name8.s[8] = 0;

	// case insensitive
	strupr (name8.s);		

	v1 = name8.x;

	// scan backwards so patch lump files take precedence
	wnum = numwads;
	while(wnum--)
	{
		int lnum = numlumps[wnum];
		lumpinfo_t *lump_p;

		lump_p = lumpinfo[wnum] + lnum;

		while(lnum--)
		{
			lump_p--;
			if ( *(uint64_t *)lump_p->name == v1)
				return lnum | (wnum << 24);
		}
	}

	// TFB. Not found.
	return -1;
}




//
// W_GetNumForName
// Calls W_CheckNumForName, but bombs out if not found.
//
int W_GetNumForName (char* name)
{
    int	i;

    i = W_CheckNumForName (name);
    
    if (i == -1)
      I_Error ("W_GetNumForName: %.8s not found!", name);
      
    return i;
}

//
// W_GetNumForNameLua
// [kg] same as W_GetNumForName with "-" for zero
// used in Lua API
int W_GetNumForNameLua (const char* tex, boolean optional)
{
	int lump;

	if(tex[0] == '-' && tex[1] == 0)
		return 0;
	else
	{
		char temp[8];
		strncpy(temp, tex, sizeof(temp));
		lump = W_CheckNumForName(temp);
		if(lump < 0)
		{
			printf("Lua: lump %.8s was not found\n", tex);
			return 0;
		}
	}

	return lump;
}

//
// W_LumpLength
// Returns the buffer size needed to load the given lump.
//
int W_LumpLength (int lump)
{
	int wnum = lump >> 24;

	lump &= 0xFFFFFF;

	if(wnum >= MAXWADS || lump >= numlumps[wnum])
		I_Error ("W_LumpLength: lump >= numlumps");

	return lumpinfo[wnum][lump].size;
}

/*

//
// W_ReadLump
// Loads the lump into the given buffer,
//  which must be >= W_LumpLength().
//
void
W_ReadLump
( int		lump,
  void*		dest )
{
	if ((unsigned)lump >= numlumps)
		I_Error ("W_ReadLump: %i >= numlumps",lump);
	memcpy(dest, wadbuf + lumpinfo[lump].offset, lumpinfo[lump].size);
}
*/

//
// W_CacheLumpNum
//
void *W_CacheLumpNum (int lump)
{
	int wnum = lump >> 24;

	lump &= 0xFFFFFF;

	if(wnum >= MAXWADS || lump >= numlumps[wnum])
		I_Error ("W_CacheLumpNum: lump >= numlumps");

	return wadbuf[wnum] + lumpinfo[wnum][lump].offset;
}



//
// W_CacheLumpName
//
void *W_CacheLumpName (char* name)
{
	return W_CacheLumpNum (W_GetNumForName(name));
}

//
// [kg] name from lump num

char *W_LumpNumName(int lump)
{
	int wnum = lump >> 24;

	lump &= 0xFFFFFF;

	if(wnum >= MAXWADS || lump >= numlumps[wnum])
		return NULL; // this can happen for maps; because of HEXEN mode check

	return lumpinfo[wnum][lump].name;
}

//
// [kg] check name

int W_LumpCheckSprite(int lump, const char *name)
{
	int wnum = lump >> 24;
	lump &= 0xFFFFFF;

	return !memcmp(name, lumpinfo[wnum][lump].name, 4);
}

//
// [kg]
void W_ForEachName(const char *name, boolean (*func)(int))
{
	union
	{
		char s[9];
		uint64_t x;
	} name8;

	uint64_t v1;
	int wnum;

	// make the name into two integers for easy compares
	strncpy (name8.s,name,8);

	// in case the name was a fill 8 chars
	name8.s[8] = 0;

	// case insensitive
	strupr (name8.s);		

	v1 = name8.x;

	// scan, and call callback for each match
	for(wnum = 0; wnum < numwads; wnum++)
	{
		int lnum;
		lumpinfo_t *lump_p;

		lump_p = lumpinfo[wnum];

		for(lnum = 0; lnum < numlumps[wnum]; lnum++)
		{
			if ( *(uint64_t *)lump_p->name == v1)
				if(func(lnum | (wnum << 24)))
					return;
			lump_p++;
		}
	}

}

