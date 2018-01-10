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
  int		time )
{
    int		i;
    
    // See if button is already pressed
    for (i = 0;i < MAXBUTTONS;i++)
    {
	if (buttonlist[i].btimer
	    && buttonlist[i].line == line)
	{
	    
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
	    buttonlist[i].soundorg = (mobj_t *)&line->frontsector->soundorg;
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
  int 		useAgain )
{
    int     texTop;
    int     texMid;
    int     texBot;
    int     i;
    int     sound = 0;//sfx_swtchn;

    // EXIT SWITCH?
    if (line->special == 11)
	sound = 0;//sfx_swtchx;

    if (!useAgain)
	line->special = 0;

    texTop = sides[line->sidenum[0]].toptexture;
    texMid = sides[line->sidenum[0]].midtexture;
    texBot = sides[line->sidenum[0]].bottomtexture;

    for (i = 0;i < numswitches*2;i++)
    {
	if (switchlist[i] == texTop)
	{
	    sides[line->sidenum[0]].toptexture = switchlist[i^1];
#ifdef SERVER
	    // tell clients about this
	    SV_ChangeSidedef(&sides[line->sidenum[0]], SV_SIDEF_TEX_TOP);
	    SV_StartLineSound(line, sound);
#else
	    S_StartSound(buttonlist->soundorg,sound, SOUND_BODY);
#endif
	    if (useAgain)
		P_StartButton(line,top,switchlist[i],BUTTONTIME);

	    return;
	}
	else
	{
	    if (switchlist[i] == texMid)
	    {
		sides[line->sidenum[0]].midtexture = switchlist[i^1];
#ifdef SERVER
		// tell clients about this
		SV_ChangeSidedef(&sides[line->sidenum[0]], SV_SIDEF_TEX_MID);
		SV_StartLineSound(line, sound);
#else
		S_StartSound(buttonlist->soundorg,sound, SOUND_BODY);
#endif
		if (useAgain)
		    P_StartButton(line, middle,switchlist[i],BUTTONTIME);

		return;
	    }
	    else
	    {
		if (switchlist[i] == texBot)
		{
		    sides[line->sidenum[0]].bottomtexture = switchlist[i^1];
#ifdef SERVER
		    // tell clients about this
		    SV_ChangeSidedef(&sides[line->sidenum[0]], SV_SIDEF_TEX_BOT);
		    SV_StartLineSound(line, sound);
#else
		    S_StartSound(buttonlist->soundorg,sound, SOUND_BODY);
#endif
		    if (useAgain)
			P_StartButton(line, bottom,switchlist[i],BUTTONTIME);

		    return;
		}
	    }
	}
    }
}






//
// P_UseSpecialLine
// Called when a thing uses a special line.
// Only the front sides of lines are usable.
boolean P_ExtraLineSpecial(mobj_t *mobj, line_t *line, int side, int act);
//
boolean
P_UseSpecialLine
( mobj_t*	thing,
  line_t*	line,
  int		side )
{
    if(isHexen)
	return P_ExtraLineSpecial(thing, line, side, EXTRA_USE);
/*
    // Err...
    // Use the back sides of VERY SPECIAL lines...
    if (side)
    {
	switch(line->special)
	{
	  case 124:
	    // Sliding door open&close
	    // UNUSED?
	    break;

	  default:
	    return false;
	    break;
	}
    }
*/
    
    // Switches that other things can activate.
    if (!thing->player)
    {
	// never open secret doors
	if (line->flags & ML_SECRET)
	    return false;
	
	switch(line->special)
	{
	  case 1: 	// MANUAL DOOR RAISE
	  case 32:	// MANUAL BLUE
	  case 33:	// MANUAL RED
	  case 34:	// MANUAL YELLOW
	    break;
	    
	  default:
	    return false;
	    break;
	}
    }

    
    // do something  
    switch (line->special)
    {
	// MANUALS
      case 1:		// Vertical Door
      case 26:		// Blue Door/Locked
      case 27:		// Yellow Door /Locked
      case 28:		// Red Door /Locked

      case 31:		// Manual door open
      case 32:		// Blue locked door open
      case 33:		// Red locked door open
      case 34:		// Yellow locked door open

      case 117:		// Blazing door raise
      case 118:		// Blazing door open
	EV_VerticalDoor (line, thing);
	break;
	
	//UNUSED - Door Slide Open&Close
	// case 124:
	// EV_SlidingDoor (line, thing);
	// break;

	// SWITCHES
      case 7:
	// Build Stairs
	if (EV_BuildStairs(line,build8))
	    P_ChangeSwitchTexture(line,0);
	break;

      case 9:
	// Change Donut
	if (EV_DoDonut(line))
	    P_ChangeSwitchTexture(line,0);
	break;
	
      case 11:
	// Exit level
	P_ChangeSwitchTexture(line,0);
	G_ExitLevel ();
	break;
	
      case 14:
	// Raise Floor 32 and change texture
	if (EV_DoPlat(line,raiseAndChange,32))
	    P_ChangeSwitchTexture(line,0);
	break;
	
      case 15:
	// Raise Floor 24 and change texture
	if (EV_DoPlat(line,raiseAndChange,24))
	    P_ChangeSwitchTexture(line,0);
	break;
	
      case 18:
	// Raise Floor to next highest floor
	if (EV_DoFloor(line, raiseFloorToNearest))
	    P_ChangeSwitchTexture(line,0);
	break;
	
      case 20:
	// Raise Plat next highest floor and change texture
	if (EV_DoPlat(line,raiseToNearestAndChange,0))
	    P_ChangeSwitchTexture(line,0);
	break;
	
      case 21:
	// PlatDownWaitUpStay
	if (EV_DoPlat(line,downWaitUpStay,0))
	    P_ChangeSwitchTexture(line,0);
	break;
	
      case 23:
	// Lower Floor to Lowest
	if (EV_DoFloor(line,lowerFloorToLowest))
	    P_ChangeSwitchTexture(line,0);
	break;
	
      case 29:
	// Raise Door
	if (EV_DoDoor(line,normal))
	    P_ChangeSwitchTexture(line,0);
	break;
	
      case 41:
	// Lower Ceiling to Floor
	if (EV_DoCeiling(line,lowerToFloor))
	    P_ChangeSwitchTexture(line,0);
	break;
	
      case 71:
	// Turbo Lower Floor
	if (EV_DoFloor(line,turboLower))
	    P_ChangeSwitchTexture(line,0);
	break;
	
      case 49:
	// Ceiling Crush And Raise
	if (EV_DoCeiling(line,crushAndRaise))
	    P_ChangeSwitchTexture(line,0);
	break;
	
      case 50:
	// Close Door
	if (EV_DoDoor(line,justClose))
	    P_ChangeSwitchTexture(line,0);
	break;
	
      case 51:
	// Secret EXIT
	P_ChangeSwitchTexture(line,0);
	G_SecretExitLevel ();
	break;
	
      case 55:
	// Raise Floor Crush
	if (EV_DoFloor(line,raiseFloorCrush))
	    P_ChangeSwitchTexture(line,0);
	break;
	
      case 101:
	// Raise Floor
	if (EV_DoFloor(line,raiseFloor))
	    P_ChangeSwitchTexture(line,0);
	break;
	
      case 102:
	// Lower Floor to Surrounding floor height
	if (EV_DoFloor(line,lowerFloor))
	    P_ChangeSwitchTexture(line,0);
	break;
	
      case 103:
	// Open Door
	if (EV_DoDoor(line,open))
	    P_ChangeSwitchTexture(line,0);
	break;
	
      case 111:
	// Blazing Door Raise (faster than TURBO!)
	if (EV_DoDoor (line,blazeRaise))
	    P_ChangeSwitchTexture(line,0);
	break;
	
      case 112:
	// Blazing Door Open (faster than TURBO!)
	if (EV_DoDoor (line,blazeOpen))
	    P_ChangeSwitchTexture(line,0);
	break;
	
      case 113:
	// Blazing Door Close (faster than TURBO!)
	if (EV_DoDoor (line,blazeClose))
	    P_ChangeSwitchTexture(line,0);
	break;
	
      case 122:
	// Blazing PlatDownWaitUpStay
	if (EV_DoPlat(line,blazeDWUS,0))
	    P_ChangeSwitchTexture(line,0);
	break;
	
      case 127:
	// Build Stairs Turbo 16
	if (EV_BuildStairs(line,turbo16))
	    P_ChangeSwitchTexture(line,0);
	break;
	
      case 131:
	// Raise Floor Turbo
	if (EV_DoFloor(line,raiseFloorTurbo))
	    P_ChangeSwitchTexture(line,0);
	break;
	
      case 133:
	// BlzOpenDoor BLUE
      case 135:
	// BlzOpenDoor RED
      case 137:
	// BlzOpenDoor YELLOW
	if (EV_DoLockedDoor (line,blazeOpen,thing))
	    P_ChangeSwitchTexture(line,0);
	break;
	
      case 140:
	// Raise Floor 512
	if (EV_DoFloor(line,raiseFloor512))
	    P_ChangeSwitchTexture(line,0);
	break;
	
	// BUTTONS
      case 42:
	// Close Door
	if (EV_DoDoor(line,justClose))
	    P_ChangeSwitchTexture(line,1);
	break;
	
      case 43:
	// Lower Ceiling to Floor
	if (EV_DoCeiling(line,lowerToFloor))
	    P_ChangeSwitchTexture(line,1);
	break;
	
      case 45:
	// Lower Floor to Surrounding floor height
	if (EV_DoFloor(line,lowerFloor))
	    P_ChangeSwitchTexture(line,1);
	break;
	
      case 60:
	// Lower Floor to Lowest
	if (EV_DoFloor(line,lowerFloorToLowest))
	    P_ChangeSwitchTexture(line,1);
	break;
	
      case 61:
	// Open Door
	if (EV_DoDoor(line,open))
	    P_ChangeSwitchTexture(line,1);
	break;
	
      case 62:
	// PlatDownWaitUpStay
	if (EV_DoPlat(line,downWaitUpStay,1))
	    P_ChangeSwitchTexture(line,1);
	break;
	
      case 63:
	// Raise Door
	if (EV_DoDoor(line,normal))
	    P_ChangeSwitchTexture(line,1);
	break;
	
      case 64:
	// Raise Floor to ceiling
	if (EV_DoFloor(line,raiseFloor))
	    P_ChangeSwitchTexture(line,1);
	break;
	
      case 66:
	// Raise Floor 24 and change texture
	if (EV_DoPlat(line,raiseAndChange,24))
	    P_ChangeSwitchTexture(line,1);
	break;
	
      case 67:
	// Raise Floor 32 and change texture
	if (EV_DoPlat(line,raiseAndChange,32))
	    P_ChangeSwitchTexture(line,1);
	break;
	
      case 65:
	// Raise Floor Crush
	if (EV_DoFloor(line,raiseFloorCrush))
	    P_ChangeSwitchTexture(line,1);
	break;
	
      case 68:
	// Raise Plat to next highest floor and change texture
	if (EV_DoPlat(line,raiseToNearestAndChange,0))
	    P_ChangeSwitchTexture(line,1);
	break;
	
      case 69:
	// Raise Floor to next highest floor
	if (EV_DoFloor(line, raiseFloorToNearest))
	    P_ChangeSwitchTexture(line,1);
	break;
	
      case 70:
	// Turbo Lower Floor
	if (EV_DoFloor(line,turboLower))
	    P_ChangeSwitchTexture(line,1);
	break;
	
      case 114:
	// Blazing Door Raise (faster than TURBO!)
	if (EV_DoDoor (line,blazeRaise))
	    P_ChangeSwitchTexture(line,1);
	break;
	
      case 115:
	// Blazing Door Open (faster than TURBO!)
	if (EV_DoDoor (line,blazeOpen))
	    P_ChangeSwitchTexture(line,1);
	break;
	
      case 116:
	// Blazing Door Close (faster than TURBO!)
	if (EV_DoDoor (line,blazeClose))
	    P_ChangeSwitchTexture(line,1);
	break;
	
      case 123:
	// Blazing PlatDownWaitUpStay
	if (EV_DoPlat(line,blazeDWUS,0))
	    P_ChangeSwitchTexture(line,1);
	break;
	
      case 132:
	// Raise Floor Turbo
	if (EV_DoFloor(line,raiseFloorTurbo))
	    P_ChangeSwitchTexture(line,1);
	break;
	
      case 99:
	// BlzOpenDoor BLUE
      case 134:
	// BlzOpenDoor RED
      case 136:
	// BlzOpenDoor YELLOW
	if (EV_DoLockedDoor (line,blazeOpen,thing))
	    P_ChangeSwitchTexture(line,1);
	break;
	
      case 138:
	// Light Turn On
	EV_LightTurnOn(line,255);
	P_ChangeSwitchTexture(line,1);
	break;
	
      case 139:
	// Light Turn Off
	EV_LightTurnOn(line,35);
	P_ChangeSwitchTexture(line,1);
	break;
			
    }
	
    return true;
}

