#include "doomdef.h"
#include "i_system.h"
#include "doomdef.h"
#include "p_local.h"

#include "g_game.h"

#include "s_sound.h"

// State.
#include "doomstat.h"
#include "r_state.h"
#include "r_data.h"

#include "z_zone.h"

#include "kg_lua.h"

#ifdef SERVER
#include <netinet/in.h>
#include "network.h"
#include "sv_cmds.h"
#endif

//
// CHANGE THE TEXTURE OF A WALL SWITCH TO ITS OPPOSITE
//

int		*switchlist; // [kg] 'unlimited' count
int		numswitches;
button_t        buttonlist[MAXBUTTONS];

//
// P_InitSwitchList
// Only called at game initialization.
// [kg] complete rewrite, support generic SWx* name
//
void P_InitSwitchList(void)
{
	int i;
	numswitches = 0;

	// first count all pairs
	for(i = 0; i < numtextures; i++)
	{
		if(!memcmp(textures[i]->name, "SW1", 3))
		{
			int j;
			for(j = 0; j < numtextures; j++)
			{
				if(!memcmp(textures[j]->name, "SW2", 3) && !strncasecmp(textures[i]->name + 3, textures[j]->name + 3, 5))
				{
					// found valid pair
					numswitches++;
					break;
				}
			}
		}
	}

	// then allocate memory
	switchlist = Z_Malloc(numswitches * 2 * sizeof(int), PU_STATIC, NULL);

	// and now load them all
	numswitches = 0;
	for(i = 0; i < numtextures; i++)
	{
		if(!memcmp(textures[i]->name, "SW1", 3))
		{
			int j;
			for(j = 0; j < numtextures; j++)
			{
				if(!memcmp(textures[j]->name, "SW2", 3) && !strncasecmp(textures[i]->name + 3, textures[j]->name + 3, 5))
				{
					switchlist[numswitches++] = i;
					switchlist[numswitches++] = j;
					break;
				}
			}
		}
	}
	numswitches /= 2;
}


//
// Start a button counting down till it turns off.
//
void
P_StartButton
( line_t*	line,
  bwhere_e	w,
  int		texture,
  int		time,
  int sound )
{
    int		i;
    mobj_t *sndorg = (mobj_t *)(linedef_side ? line->backsector : line->frontsector);
    
    // See if button is already pressed
    for (i = 0;i < MAXBUTTONS;i++)
    {
	if (buttonlist[i].btimer
	    && buttonlist[i].line == line)
	{
	    // [kg] reset timer
	    buttonlist[i].btimer = time;
	    return;
	}
    }
    

    
    for (i = 0;i < MAXBUTTONS;i++)
    {
	if (!buttonlist[i].btimer)
	{
	    buttonlist[i].line = line;
	    buttonlist[i].where = w;
	    buttonlist[i].btexture = texture;
	    buttonlist[i].btimer = time;
	    buttonlist[i].soundorg = sndorg;
	    buttonlist[i].sound = sound;
	    return;
	}
    }
    
    I_Error("P_StartButton: no button slots left!");
}

//
// Function that changes wall texture.
// Tell it if switch is ok to use again (1=yes, it's a button).
//
void
P_ChangeSwitchTexture
( line_t*	line,
  int 		sound0,
  int 		sound1,
  int btntime )
{
    int     texTop;
    int     texMid;
    int     texBot;
    int     i;
    int     useAgain = 0;
    mobj_t *sndorg = (mobj_t *)(linedef_side ? line->backsector : line->frontsector);

    if(isHexen)
    {
	if(line->flags & ELF_ACT_REPEAT)
	    useAgain = 1;
    } else
    {
	if(line->special)
	    useAgain = 1;
    }

    texTop = sides[line->sidenum[linedef_side]].toptexture;
    texMid = sides[line->sidenum[linedef_side]].midtexture;
    texBot = sides[line->sidenum[linedef_side]].bottomtexture;

    for (i = 0;i < numswitches*2;i++)
    {
	if (switchlist[i] == texTop)
	{
	    sides[line->sidenum[linedef_side]].toptexture = switchlist[i^1];
#ifdef SERVER
	    // tell clients about this
	    SV_ChangeSidedef(&sides[line->sidenum[0]], SV_SIDEF_TEX_TOP);
	    SV_StartLineSound(line, sound0);
#else
	    S_StartSound(sndorg, sound0, SOUND_BODY);
#endif
	    if (useAgain)
		P_StartButton(line, top, switchlist[i], btntime, sound1);

	    return;
	}
	else
	{
	    if (switchlist[i] == texMid)
	    {
		sides[line->sidenum[linedef_side]].midtexture = switchlist[i^1];
#ifdef SERVER
		// tell clients about this
		SV_ChangeSidedef(&sides[line->sidenum[0]], SV_SIDEF_TEX_MID);
		SV_StartLineSound(line, sound0);
#else
		S_StartSound(sndorg, sound0, SOUND_BODY);
#endif
		if (useAgain)
		    P_StartButton(line, middle, switchlist[i], btntime, sound1);

		return;
	    }
	    else
	    {
		if (switchlist[i] == texBot)
		{
		    sides[line->sidenum[linedef_side]].bottomtexture = switchlist[i^1];
#ifdef SERVER
		    // tell clients about this
		    SV_ChangeSidedef(&sides[line->sidenum[0]], SV_SIDEF_TEX_BOT);
		    SV_StartLineSound(line, sound0);
#else
		    S_StartSound(sndorg, sound0, SOUND_BODY);
#endif
		    if (useAgain)
			P_StartButton(line, bottom, switchlist[i], btntime, sound1);

		    return;
		}
	    }
	}
    }
}

