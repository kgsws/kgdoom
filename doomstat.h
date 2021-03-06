#ifndef __D_STATE__
#define __D_STATE__

// We need globally shared data structures,
//  for defining the global state variables.
#include "doomdata.h"
#include "d_net.h"

// We need the playr data structure as well.
#include "d_player.h"


#ifdef __GNUG__
#pragma interface
#endif



// ------------------------
// Command line parameters.
//
extern  boolean	nomonsters;	// checkparm of -nomonsters
extern  boolean	respawnparm;	// checkparm of -respawn
extern  boolean	fastparm;	// checkparm of -fast

extern  boolean	devparm;	// DEBUG: launched with -devparm



// -----------------------------------------------------
// Game Mode - identify IWAD as shareware, retail etc.
//
extern GameMode_t	gamemode;
extern GameMission_t	gamemission;

// -------------------------------------------
// Selected skill type, map etc.
//

// Defaults for menu, methinks.
extern  skill_t		startskill;
extern  int             startepisode;
extern	int		startmap;

extern  boolean		autostart;

// Selected by user. 
extern  skill_t         gameskill;
extern  int		gameepisode;
extern  int		gamemap;

// Nightmare mode flag, single player.
extern  boolean         respawnmonsters;

// Netgame? Only true if >1 player.
extern  int	netgame;

// Flag: true only if started as net deathmatch.
// An enum might handle altdeath/cooperative better.
extern  boolean	deathmatch;	
	
// -------------------------
// Internal parameters for sound rendering.
// These have been taken from the DOS version,
//  but are not (yet) supported with Linux
//  (e.g. no sound volume adjustment with menu.

// These are not used, but should be (menu).
// From m_menu.c:
//  Sound FX volume has default, 0 - 15
//  Music volume has default, 0 - 15
// These are multiplied by 8.
extern int snd_SfxVolume;      // maximum volume for sound
extern int snd_MusicVolume;    // maximum volume for music

// Current music/sfx card - index useless
//  w/o a reference LUT in a sound module.
// Ideally, this would use indices found
//  in: /usr/include/linux/soundcard.h
extern int snd_MusicDevice;
extern int snd_SfxDevice;
// Config file? Same disclaimer as above.
extern int snd_DesiredMusicDevice;
extern int snd_DesiredSfxDevice;


// -------------------------
// Status flags for refresh.
//

// Depending on view size - no status bar?
// Note that there is no way to disable the
//  status bar explicitely.
extern  boolean statusbaractive;

extern  boolean automapactive;	// In AutoMap mode?
extern  boolean	menuactive;	// Menu overlayed?
extern  boolean	paused;		// Game Pause?


extern  boolean		viewactive;

extern  boolean		nodrawers;
extern  boolean		noblit;

extern	int		viewwindowx;
extern	int		viewwindowy;
extern	int		viewheight;
extern	int		viewwidth;
extern	int		scaledviewwidth;






// This one is related to the 3-screen display mode.
// ANG90 = left side, ANG270 = right
extern  int	viewangleoffset;

// Player taking events, and displaying.
extern  int	consoleplayer;	
extern  int	displayplayer;


// -------------------------------------
// Scores, rating.
// Statistics on a given map, for intermission.
//
extern  int	totalkills;
extern	int	totalitems;
extern	int	totalsecret;

// Timer, for scores.
extern  int	levelstarttic;	// gametic at level start
extern  int	leveltime;	// tics in game play for par

//?
extern  gamestate_t     gamestate;

extern boolean usergame;

//-----------------------------
// Internal parameters, fixed.
// These are set by the engine, and not changed
//  according to user inputs. Partly load from
//  WAD, partly set at startup time.

extern	int		gametic;

#ifdef SERVER
extern	player_t	players[MAXPLAYERS];
extern  boolean		playeringame[MAXPLAYERS];
#else
// [kg] last slot for local spectator
extern	player_t	players[MAXPLAYERS+1];
extern  boolean		playeringame[MAXPLAYERS+1];
#endif

// Player spawn spots for deathmatch.
#define MAX_DM_STARTS   10
extern  mapthing_hexen_t      deathmatchstarts[MAX_DM_STARTS];
extern  mapthing_hexen_t*	deathmatch_p;

// Player spawn spots.
extern  mapthing_hexen_t      playerstarts[MAXPLAYERS];

// Intermission stats.
// Parameters for world map / intermission.
extern  wbstartstruct_t		wminfo;	


// LUT of ammunition limits for each kind.
// This doubles with BackPack powerup item.
extern  int		maxammo[NUMAMMO];





//-----------------------------------------
// Internal parameters, used for engine.
//

// File handling stuff.
extern	char		basedefault[1024];
extern  FILE*		debugfile;

// if true, load all graphics at level load
extern  boolean         precache;


// wipegamestate can be set to -1
//  to force a wipe on the next draw
extern  gamestate_t     wipegamestate;

extern  int             mouseSensitivity;
//?
// debug flag to cancel adaptiveness
extern  boolean         singletics;	

extern  int             bodyqueslot;



// Needed to store the number of the dummy sky flat.
// Used for rendering,
//  as well as tracking projectiles etc.
extern int		skyflatnum;



// Netgame stuff (buffers and pointers, i.e. indices).

extern	int		rndindex;


#endif

