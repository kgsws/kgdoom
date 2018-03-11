#ifndef __D_EVENT__
#define __D_EVENT__


#include "doomtype.h"


//
// Event handling.
//

// Input event types.
typedef enum
{
    ev_keydown,
    ev_keyup,
    ev_mouse,
    ev_joystick,
    ev_joystick2
} evtype_t;

// Event structure.
typedef struct
{
    evtype_t	type;
    int		data1;		// keys / mouse/joystick buttons
    int		data2;		// mouse/joystick x move
    int		data3;		// mouse/joystick y move
} event_t;

 
typedef enum
{
    ga_nothing,
    ga_loadlevel,
    ga_newgame,
    ga_loadgame,
    ga_savegame,
    ga_completed,
    ga_victory,
    ga_worlddone,
    ga_screenshot
} gameaction_t;



//
// Button/action code definitions.
//
typedef enum
{
    // Press "Fire".
    BT_ATTACK		= 1,
    BT_ALTATTACK	= 2,
    // Use button, to open doors, activate switches.
    BT_USE		= 4,
    // Flag, weapon change pending.
    // If true, 'weapon' hold weapon num.
    BT_CHANGE		= 8,
} buttoncode_t;




//
// GLOBAL VARIABLES
//
#define MAXEVENTS		64

extern  event_t		events[MAXEVENTS];
extern  int             eventhead;
extern	int		eventtail;

extern  gameaction_t    gameaction;


#endif

