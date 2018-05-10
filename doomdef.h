#ifndef __DOOMDEF__
#define __DOOMDEF__

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <inttypes.h>
//#include <values.h>
#include <ctype.h>

#ifdef LINUX
#include <inttypes.h>
#define true 1
#define false 0
#else
#include <libtransistor/nx.h>
#endif

#ifndef MAXINT
#define MAXINT 0x7FFFFFFF
#endif

#ifndef MININT
#define MININT -0x80000000
#endif

#ifndef MAXSHORT
#define MAXSHORT 0x7FFF
#endif

#ifndef MINSHORT
#define MINSHORT -0x8000
#endif

#include <sys/socket.h>

// [kg] maximum of loaded wads
// WAD slot 15 is used for other hax stuff
#define MAXWADS	15

//
// Global parameters/defines.
//
// DOOM version
enum { VERSION =  110 };


// Game mode handling - identify IWAD version
//  to handle IWAD dependend animations etc.
typedef enum
{
  shareware,	// DOOM 1 shareware, E1, M9
  registered,	// DOOM 1 registered, E3, M27
  commercial,	// DOOM 2 retail, E1 M34
  // DOOM 2 german edition not handled
  retail,	// DOOM 1 retail, E4, M36
  indetermined	// Well, no IWAD found.
  
} GameMode_t;


// Mission packs - might be useful for TC stuff?
typedef enum
{
  doom,		// DOOM 1
  doom2,	// DOOM 2
  pack_tnt,	// TNT mission pack
  pack_plut,	// Plutonia pack
  none

} GameMission_t;

// [kg] joycon buttons
enum
{
	joybfire,
	joybfirealt,
	joybuse,
	joybweapons,
	joybspeed,
	joymap,
	NUM_JOYCON_BUTTONS
};

// If rangecheck is undefined,
// most parameter validation debugging code will not be compiled
#define RANGECHECK

// Do or do not use external soundserver.
// The sndserver binary to be run separately
//  has been introduced by Dave Taylor.
// The integrated sound support is experimental,
//  and unfinished. Default is synchronous.
// Experimental asynchronous timer based is
//  handled by SNDINTR. 
//#define SNDSERV  1
//#define SNDINTR  1


// This one switches between MIT SHM (no proper mouse)
// and XFree86 DGA (mickey sampling). The original
// linuxdoom used SHM, which is default.
//#define X11_DGA		1

// It is educational but futile to change this
//  scaling e.g. to 2. Drawing of status bar,
//  menues etc. is tied to the scale implied
//  by the graphics.
#define	SCREEN_MUL		1
#define	INV_ASPECT_RATIO	0.625 // 0.75, ideally

// Defines suck. C sucks.
// C++ might sucks for OOP, but it sure is a better C.
// So there.

#define SCREENWIDTH  1280
#define SCREENHEIGHT 720

//SCREEN_MUL*BASE_WIDTH //320
//(int)(SCREEN_MUL*BASE_WIDTH*INV_ASPECT_RATIO) //200

//
// For resize of screen, at start of game.
// It will not work dynamically, see visplanes.
//
#define	BASE_WIDTH SCREENWIDTH

// The maximum number of players, multiplayer/networking.
#define MAXPLAYERS		64

// State updates, number of tics / second.
#define TICRATE		35

// The current state of the game: whether we are
// playing, gazing at the intermission screen,
// the game final animation, or a demo. 
typedef enum
{
    GS_LEVEL,
    GS_INTERMISSION,
    GS_FINALE,
    GS_DEMOSCREEN,
    GS_NETWORK
} gamestate_t;

//
// Difficulty/skill settings/filters.
//

// Skill flags.
#define	MTF_EASY		1
#define	MTF_NORMAL		2
#define	MTF_HARD		4

// Deaf monsters/do not react to sound.
#define	MTF_AMBUSH		8

typedef enum
{
    sk_baby,
    sk_easy,
    sk_medium,
    sk_hard,
    sk_nightmare
} skill_t;


// [kg] new handling for Lua
typedef uint16_t weapontype_t;
#define wp_nochange	0
#define MAXWEAPONS	64 // limited by uint64_t bit field

// [kg] custom damage types
// damage is scaled before armor effects
#define NUMDAMAGETYPES	32
#define DAMAGE_SCALE	20		// 5% steps
#define DEFAULT_DAMAGE_SCALE	74	// 74 = 100%; 255 = kill; 254 = 900%; 0 = -270% (healing)

// [kg] rendering
enum
{
	RENDER_NORMAL, // must be first
	RENDER_SHADOW,
	RENDER_HOLEY0,
	RENDER_HOLEY1,
	RENDER_TABLE,
	RENDER_TABLEI,
};

typedef struct
{
	int renderstyle;
	uint8_t *rendertable;
} render_t;

// [kg] custom colormaps
typedef struct
{
	int lump;	// source lump in WAD; 0 = none
	int idx;	// selected colormap index
	uint8_t *data;	// pointer to already ofset colormap data
} colormap_t;

//
// DOOM keyboard definition.
// This is the stuff configured by Setup.Exe.
// Most key data are simple ascii (uppercased).
//
#define KEY_RIGHTARROW	0xae
#define KEY_LEFTARROW	0xac
#define KEY_UPARROW	0xad
#define KEY_DOWNARROW	0xaf
#define KEY_ESCAPE	27
#define KEY_ENTER	13
#define KEY_TAB		9
#define KEY_F1		(0x80+0x3b)
#define KEY_F2		(0x80+0x3c)
#define KEY_F3		(0x80+0x3d)
#define KEY_F4		(0x80+0x3e)
#define KEY_F5		(0x80+0x3f)
#define KEY_F6		(0x80+0x40)
#define KEY_F7		(0x80+0x41)
#define KEY_F8		(0x80+0x42)
#define KEY_F9		(0x80+0x43)
#define KEY_F10		(0x80+0x44)
#define KEY_F11		(0x80+0x57)
#define KEY_F12		(0x80+0x58)

#define KEY_BACKSPACE	127
#define KEY_PAUSE	0xff

#define KEY_EQUALS	0x3d
#define KEY_MINUS	0x2d

#define KEY_RSHIFT	(0x80+0x36)
#define KEY_RCTRL	(0x80+0x1d)
#define KEY_RALT	(0x80+0x38)

#define KEY_LALT	KEY_RALT



// DOOM basic types (boolean),
//  and max/min values.
//#include "doomtype.h"

// Fixed point.
//#include "m_fixed.h"

// Endianess handling.
//#include "m_swap.h"


// Binary Angles, sine/cosine/atan lookups.
//#include "tables.h"

// Event type.
//#include "d_event.h"

// Game function, skills.
//#include "g_game.h"

// All external data is defined here.
//#include "doomdata.h"

// All important printed strings.
// Language selection (message strings).
//#include "dstrings.h"

// Player is a special actor.
//struct player_s;


//#include "d_items.h"
//#include "d_player.h"
//#include "p_mobj.h"
//#include "d_net.h"

// PLAY
//#include "p_tick.h"




// Header, generated by sound utility.
// The utility was written by Dave Taylor.
//#include "sounds.h"


// HEXEN type lines
enum
{
	EXTRA_USE,
	EXTRA_CROSS,
	EXTRA_HITSCAN,
	EXTRA_BUMP,
};

enum
{
	ELF_ACT_REPEAT = 0x0200,
	ELF_ACT_TYPE_MASK = 0x1C00,
	ELF_CROSS_PLAYER = 0,
	ELF_ACT_PLAYER = 0x0400,
	ELF_CROSS_MONSTER = 0x0800,
	ELF_HIT_PROJECTILE = 0x0C00,
	ELF_HIT_PLAYER = 0x1000,
	ELF_CROSS_PROJECTILE = 0x1400,
	ELF_ACT_PASSLUA = 0x1800,
	ELF_ANY_ACT = 0x2000,
	ELF_BLOCK_PLAYER = 0x4000,
	ELF_TOTAL_BLOCK = 0x8000
};


#endif          // __DOOMDEF__

