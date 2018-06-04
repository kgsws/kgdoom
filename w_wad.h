#ifndef __W_WAD__
#define __W_WAD__


#ifdef __GNUG__
#pragma interface
#endif

//
// TYPES
//
typedef struct
{
	// Should be "IWAD" or "PWAD".
	uint32_t id;
	uint32_t numlumps;
	uint32_t offs;
} wadinfo_t;

//
// WADFILE I/O related stuff.
//
typedef struct
{
	uint32_t offset;
	uint32_t size;
	char name[8];
} lumpinfo_t;

extern int numwads;

void	W_LoadWad(const char *name);

int	W_CheckNumForName (char* name);
int	W_GetNumForName (char* name);

int	W_GetNumForNameLua (const char* tex, boolean optional);

int	W_LumpLength (int lump);
void    W_ReadLump (int lump, void *dest);

void*	W_CacheLumpNum (int lump);
void*	W_CacheLumpName (char* name);

char *W_LumpNumName(int lump);

int W_LumpCheckSprite(int lump, const char *name);

void W_ForEachName(const char *name, boolean (*func)(int));

int W_GetWadSize(const char *name);

#endif

