// 3D floors
// by kgsws
#include "z_zone.h"
#include "doomdef.h"
#include "p_local.h"
#include "doomstat.h"
#include "r_state.h"
#include "p_generic.h"
#include "m_bbox.h"

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

height3d_t height3top = {.height = ONCEILINGZ};
height3d_t height3bot = {.height = ONFLOORZ};

static boolean PIT_Check3DSectorSpawn(mobj_t *thing)
{
	P_CheckPosition(thing, thing->x, thing->y);
	thing->floorz = tmfloorz;
	thing->ceilingz = tmceilingz;
	return true;
}

extraplane_t *e3d_AddFloorPlane(extraplane_t **dest, sector_t *sec, line_t *line, int block)
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
	new->line = line;
	new->source = sec;
	new->height = &sec->ceilingheight;
	new->pic = &sec->ceilingpic;
	new->lightlevel = &sec->lightlevel;
	new->validcount = 0;
	new->blocking = block;
	new->render = &line->render;

	return new;
}

extraplane_t *e3d_AddCeilingPlane(extraplane_t **dest, sector_t *sec, line_t *line, int block)
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
	new->line = line;
	new->source = sec;
	new->height = &sec->floorheight;
	new->pic = &sec->floorpic;
	new->lightlevel = &sec->lightlevel;
	new->validcount = 0;
	new->blocking = block;
	new->render = &line->render;

	return new;
}

void e3d_AddExtraFloor(sector_t *dst, sector_t *src, line_t *line, int block)
{
	int x;
	int y;

	// check
	extraplane_t *pl = dst->exfloor;
	while(pl)
	{
		if(pl->source == src)
		{
			// already added; change blocking and line
			pl->line = line;
			pl->blocking = block;
			pl->render = &line->render;
			// do this for ceiling too
			pl = dst->exceiling;
			while(pl)
			{
				if(pl->source == src)
				{
					pl->line = line;
					pl->blocking = block;
					pl->render = &line->render;
				}
				pl = pl->next;
			}
			goto fcheck;
		}
		pl = pl->next;
	}
	// add planes
	e3d_AddFloorPlane(&dst->exfloor, src, line, block);
	e3d_AddCeilingPlane(&dst->exceiling, src, line, block);
	// update thing heights
fcheck:
	for(x = dst->blockbox[BOXLEFT]; x <= dst->blockbox[BOXRIGHT]; x++)
		for(y = dst->blockbox[BOXBOTTOM]; y <= dst->blockbox[BOXTOP]; y++)
			P_BlockThingsIterator(x, y, PIT_Check3DSectorSpawn);
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

	height3d_t *hh = height3bot.next;

	while(hh && hh != &height3top)
	{
		height3d_t *hf = hh;
		hh = hh->next;
		Z_Free(hf);
	}

	height3top.prev = &height3bot;
	height3bot.next = &height3top;
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

void e3d_NewHeight(fixed_t height)
{
	height3d_t *hh = &height3bot;
	height3d_t *new;

	while(hh)
	{
		if(hh->height == height)
			return;
		if(hh->height > height)
			break;
		hh = hh->next;
	}

	new = Z_Malloc(sizeof(height3d_t), PU_STATIC, NULL);
	new->next = hh;
	new->prev = hh->prev;
	hh->prev->next = new;
	hh->prev = new;
	new->height = height;
}

