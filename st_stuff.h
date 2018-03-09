#ifndef __STSTUFF_H__
#define __STSTUFF_H__

#include "doomtype.h"
#include "d_event.h"

// Size of statusbar.
// Now sensitive for scaling.
#define ST_HEIGHT	32*SCREEN_MUL
#define ST_WIDTH	SCREENWIDTH
#define ST_Y		(SCREENHEIGHT - ST_HEIGHT)


//
// STATUS BAR
//

// [kg] picking a weapon
boolean in_weapon_menu;

// Called by main loop.
boolean ST_Responder (event_t* ev);

// Called by main loop.
void ST_Ticker (void);

// Called by main loop.
void ST_Drawer (boolean fullscreen, boolean refresh);

// Called when the console player is spawned on each level.
void ST_Start (void);

// Called by startup code.
void ST_Init (void);

// [kg] weapon selection menu
void ST_AddWeaponType(int type, char *patch, int ammo0type, int ammo1type);
weapontype_t ST_GetNewWeapon();
void ST_SetNewWeapon(weapontype_t wpn);
void ST_ReadyWeapon(void *pl);

// [kg] other gfx
void ST_ClearInventory();
void ST_CheckInventory(int type, int count);
void ST_AddKeyType(int type, char *patch);

// States for status bar code.
typedef enum
{
    AutomapState,
    FirstPersonState
    
} st_stateenum_t;


// States for the chat code.
typedef enum
{
    StartChatState,
    WaitDestState,
    GetChatState
    
} st_chatstateenum_t;


boolean ST_Responder(event_t* ev);



#endif

