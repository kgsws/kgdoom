// 3D floors
// by kgsws
#include "z_zone.h"
#include "doomdef.h"
#include "p_local.h"
#include "doomstat.h"
#include "r_state.h"
#include "p_generic.h"

#include "kg_3dfloor.h"

#include "kg_lua.h"

typedef struct clip_s
{
	struct clip_s *next;
	short clip[];
} clip_t;

boolean fakeclip;
short *fakecliptop;
short *fakeclipbot;
extraplane_t *fakeplane;

clip_t *topclip;

extraplane_t *e3d_AddFloorPlane(extraplane_t **dest, sector_t *sec)
{
	extraplane_t *pl = *dest;
	extraplane_t *new;

	while(pl)
	{
		if(sec->ceilingheight < *pl->height)
			break;
		dest = &pl->next;
		pl = pl->next;
	}

	new = Z_Malloc(sizeof(extraplane_t), PU_LEVEL, NULL);
	*dest = new;
	new->next = pl;
	new->height = &sec->ceilingheight;
	new->pic = &sec->ceilingpic;
	new->lightlevel = &sec->lightlevel;

	return new;
}

extraplane_t *e3d_AddCeilingPlane(extraplane_t **dest, sector_t *sec)
{
	extraplane_t *pl = *dest;
	extraplane_t *new;

	while(pl)
	{
		if(sec->floorheight > *pl->height)
			break;
		dest = &pl->next;
		pl = pl->next;
	}

	new = Z_Malloc(sizeof(extraplane_t), PU_LEVEL, NULL);
	*dest = new;
	new->next = pl;
	new->height = &sec->floorheight;
	new->pic = &sec->floorpic;
	new->lightlevel = &sec->lightlevel;

	return new;
}

void e3d_Reset()
{
	clip_t *cl = topclip;

	while(cl)
	{
		clip_t *ff = cl;
		cl = cl->next;
		Z_Free(ff);
	}

	topclip = NULL;
}

short *e3d_NewClip(short *source)
{
	clip_t *new;

	new = Z_Malloc(sizeof(clip_t) + SCREENWIDTH * sizeof(short), PU_STATIC, NULL);
	new->next = topclip;
	memcpy(new->clip, source, SCREENWIDTH * sizeof(short));

	topclip = new;

	return new->clip;
}

