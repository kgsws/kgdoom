#include <stdlib.h>

#include "doomdef.h"
#include "doomstat.h"

#include "i_system.h"
#include "z_zone.h"
#include "m_argv.h"
#include "m_random.h"
#include "w_wad.h"

#include "r_local.h"
#include "p_local.h"

#include "g_game.h"

#include "s_sound.h"

// State.
#include "r_state.h"

#ifdef SERVER
#include <netinet/in.h>
#include "network.h"
#include "sv_cmds.h"
#endif

boolean P_ExtraLineSpecial(mobj_t *mobj, line_t *line, int side, int act);

// [kg] new animation definitions

animdef_t *anim_top;
animdef_t *anim_cur;

animdef_t *P_AddAnimation(int target, int ticrate, int count)
{
	animdef_t *ret;

	ret = malloc(sizeof(animdef_t) + sizeof(uint16_t) * count);
	if(!ret)
		I_Error("P_AddAnimation: out of memory");

	ret->target = target;
	ret->ticrate = ticrate;
	ret->count = count;
	ret->next = NULL;

	if(anim_cur)
		anim_cur->next = ret;
	anim_cur = ret;
	if(!anim_top)
		anim_top = ret;

	return ret;
}

//
// UTILITIES
//

//
// getSide()
// Will return a side_t*
//  given the number of the current sector,
//  the line number, and the side (0/1) that you want.
//
side_t*
getSide
( int		currentSector,
  int		line,
  int		side )
{
    return &sides[ (sectors[currentSector].lines[line])->sidenum[side] ];
}


//
// getSector()
// Will return a sector_t*
//  given the number of the current sector,
//  the line number and the side (0/1) that you want.
//
sector_t*
getSector
( int		currentSector,
  int		line,
  int		side )
{
    return sides[ (sectors[currentSector].lines[line])->sidenum[side] ].sector;
}


//
// twoSided()
// Given the sector number and the line number,
//  it will tell you whether the line is two-sided or not.
//
int
twoSided
( int	sector,
  int	line )
{
    return (sectors[sector].lines[line])->flags & LF_TWOSIDED;
}




//
// getNextSector()
// Return sector_t * of sector next to current.
// NULL if not two-sided line
//
sector_t*
getNextSector
( line_t*	line,
  sector_t*	sec )
{
    if (!(line->flags & LF_TWOSIDED))
	return NULL;
		
    if (line->frontsector == sec)
	return line->backsector;
	
    return line->frontsector;
}



//
// P_FindLowestFloorSurrounding()
// FIND LOWEST FLOOR HEIGHT IN SURROUNDING SECTORS
//
fixed_t	P_FindLowestFloorSurrounding(sector_t* sec)
{
    int			i;
    line_t*		check;
    sector_t*		other;
    fixed_t		floor = sec->floorheight;
	
    for (i=0 ;i < sec->linecount ; i++)
    {
	check = sec->lines[i];
	other = getNextSector(check,sec);

	if (!other)
	    continue;
	
	if (other->floorheight < floor)
	    floor = other->floorheight;
    }
    return floor;
}



//
// P_FindHighestFloorSurrounding()
// FIND HIGHEST FLOOR HEIGHT IN SURROUNDING SECTORS
//
fixed_t	P_FindHighestFloorSurrounding(sector_t *sec)
{
    int			i;
    line_t*		check;
    sector_t*		other;
    fixed_t		floor = ONFLOORZ;
	
    for (i=0 ;i < sec->linecount ; i++)
    {
	check = sec->lines[i];
	other = getNextSector(check,sec);
	
	if (!other)
	    continue;
	
	if (other->floorheight > floor)
	    floor = other->floorheight;
    }
    return floor;
}



//
// P_FindNextHighestFloor
// FIND NEXT HIGHEST FLOOR IN SURROUNDING SECTORS
// Note: this should be doable w/o a fixed array.

// 20 adjoining sectors max!
#define MAX_ADJOINING_SECTORS    	20

fixed_t
P_FindNextHighestFloor
( sector_t*	sec)
{
    int			i;
    int			h;
    int			min;
    line_t*		check;
    sector_t*		other;
    fixed_t		height = sec->floorheight;

    
    fixed_t		heightlist[MAX_ADJOINING_SECTORS];		

    for (i=0, h=0 ;i < sec->linecount ; i++)
    {
	check = sec->lines[i];
	other = getNextSector(check,sec);

	if (!other)
	    continue;
	
	if (other->floorheight > height)
	    heightlist[h++] = other->floorheight;

	// Check for overflow. Exit.
	if ( h >= MAX_ADJOINING_SECTORS )
	{
//	    fprintf( stderr, "Sector with more than 20 adjoining sectors\n" );
	    break;
	}
    }
    
    // Find lowest height in list
    if (!h)
	return sec->floorheight;
		
    min = heightlist[0];
    
    // Range checking? 
    for (i = 1;i < h;i++)
	if (heightlist[i] < min)
	    min = heightlist[i];
			
    return min;
}


//
// FIND LOWEST CEILING IN THE SURROUNDING SECTORS
//
fixed_t
P_FindLowestCeilingSurrounding(sector_t* sec)
{
    int			i;
    line_t*		check;
    sector_t*		other;
    fixed_t		height = MAXINT;
	
    for (i=0 ;i < sec->linecount ; i++)
    {
	check = sec->lines[i];
	other = getNextSector(check,sec);

	if (!other)
	    continue;

	if (other->ceilingheight < height)
	    height = other->ceilingheight;
    }
    return height;
}


//
// FIND HIGHEST CEILING IN THE SURROUNDING SECTORS
//
fixed_t	P_FindHighestCeilingSurrounding(sector_t* sec)
{
    int		i;
    line_t*	check;
    sector_t*	other;
    fixed_t	height = 0;
	
    for (i=0 ;i < sec->linecount ; i++)
    {
	check = sec->lines[i];
	other = getNextSector(check,sec);

	if (!other)
	    continue;

	if (other->ceilingheight > height)
	    height = other->ceilingheight;
    }
    return height;
}



//
// RETURN NEXT SECTOR # THAT LINE TAG REFERS TO
//
int
P_FindSectorFromLineTag
( line_t*	line,
  int		start )
{
    int	i;
	
    for (i=start+1;i<numsectors;i++)
	if (sectors[i].tag == line->tag)
	    return i;
    
    return -1;
}

// [kg] generic find
int
P_FindSectorFromTag
( int tag,
  int		start )
{
    int	i;
	
    for (i=start+1;i<numsectors;i++)
	if (sectors[i].tag == tag)
	    return i;
    
    return -1;
}

//
// Find minimum light from an adjacent sector
//
int
P_FindMinSurroundingLight
( sector_t*	sector)
{
    int		i;
    int		min;
    line_t*	line;
    sector_t*	check;

    min = 255;
    for (i=0 ; i < sector->linecount ; i++)
    {
	line = sector->lines[i];
	check = getNextSector(line,sector);

	if (!check)
	    continue;

	if (check->lightlevel < min)
	    min = check->lightlevel;
    }
    return min;
}

int
P_FindMaxSurroundingLight
( sector_t*	sector)
{
    int		i;
    int		min;
    line_t*	line;
    sector_t*	check;
	
    min = 0;
    for (i=0 ; i < sector->linecount ; i++)
    {
	line = sector->lines[i];
	check = getNextSector(line,sector);

	if (!check)
	    continue;

	if (check->lightlevel > min)
	    min = check->lightlevel;
    }
    return min;
}

//
// EVENTS
// Events are operations triggered by using, crossing,
// or shooting special lines, or by timed thinkers.
//

//
// P_ShootSpecialLine - IMPACT SPECIALS
// Called when a thing shoots a special line.
//
void
P_aShootSpecialLine
( mobj_t*	thing,
  line_t*	line )
{
	P_ExtraLineSpecial(thing, line, 0, EXTRA_HITSCAN);
}

//
// P_UpdateSpecials
// Animate planes, scroll walls, etc.
//
boolean		levelTimer;
int		levelTimeCount;

void P_UpdateSpecials (void)
{
    int		pic;
    int		i;
    line_t	*line;
    animdef_t	*anim = anim_top;

    //	LEVEL TIMER
    if (levelTimer == true)
    {
	levelTimeCount--;
	if (!levelTimeCount)
	    G_ExitLevel();
    }

    //	ANIMATE FLATS AND TEXTURES GLOBALLY
    while(anim)
    {
	for(i = 0; i < anim->count; i++)
	{
	    int basepic = anim->anim[i];
	    pic = anim->anim[( (leveltime/anim->ticrate + i)%anim->count )];
	    if(
		anim->target < 0 ||
		(basepic < flatstart && anim->target == 0) ||
		(basepic >= flatstart && anim->target == 1)
	    )
		texturetranslation[basepic] = pic;
	}
	anim = anim->next;
    }

    //	DO BUTTONS
    for (i = 0; i < MAXBUTTONS; i++)
	if (buttonlist[i].btimer)
	{
	    buttonlist[i].btimer--;
	    if (!buttonlist[i].btimer)
	    {
#ifdef SERVER
		uint16_t info = 0;
#endif
		switch(buttonlist[i].where)
		{
		  case top:
		    sides[buttonlist[i].line->sidenum[0]].toptexture = buttonlist[i].btexture;
#ifdef SERVER
		    info = SV_SIDEF_TEX_TOP;
#endif
		    break;
		    
		  case middle:
		    sides[buttonlist[i].line->sidenum[0]].midtexture = buttonlist[i].btexture;
#ifdef SERVER
		    info = SV_SIDEF_TEX_MID;
#endif
		    break;
		    
		  case bottom:
		    sides[buttonlist[i].line->sidenum[0]].bottomtexture = buttonlist[i].btexture;
#ifdef SERVER
		    info = SV_SIDEF_TEX_BOT;
#endif
		    break;
		}
#ifdef SERVER
		// tell clients about this
		if(info)
		{
		    SV_ChangeSidedef(&sides[buttonlist[i].line->sidenum[0]], info);
		    SV_StartLineSound(buttonlist[i].line, buttonlist[i].sound);
		}
#else
		S_StartSound(buttonlist[i].soundorg, buttonlist[i].sound, SOUND_BODY);
#endif
		memset(&buttonlist[i],0,sizeof(button_t));
	    }
	}
	
}

//
// SPECIAL SPAWNING
//

//
// P_SpawnSpecials
// After the map has been loaded, scan for specials
//  that spawn thinkers
//

// Parses command line parameters.
void P_SpawnSpecials (void)
{
    int i;

    for (i = 0;i < MAXBUTTONS;i++)
	memset(&buttonlist[i],0,sizeof(button_t));
}

