#ifndef __D_PLAYER__
#define __D_PLAYER__


// The player data structure depends on a number
// of other structs: items (internal inventory),
// animation states (closely tied to the sprites
// used to represent them, unfortunately).
#include "p_pspr.h"

// In addition, the player is just a special
// case of the generic moving object/actor.
#include "p_mobj.h"

// Finally, for odd reasons, the player input
// is buffered within the player data struct,
// as commands per game tick.
#include "d_ticcmd.h"

#ifdef __GNUG__
#pragma interface
#endif




//
// Player states.
//
typedef enum
{
    // Playing or camping.
    PST_LIVE,
    // Dead on the ground, view follows killer.
    PST_DEAD,
    // Ready to restart/respawn
    PST_REBORN,
    // [kg] multiplayer next level, keep inventory
    PST_RESPAWN
} playerstate_t;


//
// Player internal flags, for cheats and debug.
//
typedef enum
{
	// No clipping, walk through barriers.
	CF_NOCLIP		= 1,
	// No damage, no health loss.
	CF_GODMODE		= 2,
	// Not really a cheat, just a debug aid.
	CF_NOMOMENTUM	= 4,
	// [kg] inf. ammo; also used for other multiplayer players
	CF_INFAMMO = 8,
	// [kg] inf. health
	CF_INFHEALTH = 16,
	// [kg] auras
	CF_AURA0 = 32,
	CF_AURA1 = 64,

	// spectator mark
	CF_SPECTATOR = 0x80000000
} cheat_t;

#define CF_AURAMASK	(CF_AURA0|CF_AURA1)
#define CF_DEATHAURA	CF_AURA0
#define CF_SAFEAURA	CF_AURA1
#define CF_REVENGEAURA	(CF_AURA0|CF_AURA1)

//
// Extended player object info: player_t
//
typedef struct player_s
{
    degenthinker_t	think;

    mobj_t*		mo;
    playerstate_t	playerstate;
    ticcmd_t		cmd;

    // Determine POV,
    //  including viewpoint bobbing during movement.
    // Focal origin above r.z
    fixed_t		viewz;
    // Base height above floor for viewz.
    fixed_t		viewheight;
    // Bob/squat speed.
    fixed_t         	deltaviewheight;
    // bounded/scaled total momentum.
    fixed_t         	bob;	

    // Power ups. invinc and invis are tic counters.
    int			powers[NUMPOWERS];
    boolean		cards[NUMCARDS];
    boolean		backpack;
    
    // Frags, kills of other players.
    int			frags[MAXPLAYERS];
    
    // Is wp_nochange if not changing.
    weapontype_t	readyweapon;
    weapontype_t	pendingweapon;

    // True if button down last tic.
    int			attackdown;
    int			usedown;

    // Bit flags, for cheats and debug.
    // See cheat_t, above.
    int			cheats;		

    // Refired shots are less accurate.
    int			refire;		

     // For intermission stats.
    int			killcount;
    int			itemcount;
    int			secretcount;

    // Hint messages.
    const char*		message;	
    
    // For screen flashing (red or bright).
    int			damagecount;
    int			bonuscount;

    // So gun flashes light up areas.
    int			extralight;

    // Current PLAYPAL, ???
    //  can be set to REDCOLORMAP for pain, etc.
    int			fixedcolormap;

    // Player skin colorshift,
    //  0-3 for which color to draw player.
    int			colormap;	

    // Overlay view sprites (gun, etc).
    pspdef_t		psprites[NUMPSPRITES];

    // True if secret level has been done.
    boolean		didsecret;

    // [kg] inventory; only used on level change
    struct inventory_s *inventory;

} player_t;


//
// INTERMISSION
// Structure passed e.g. to WI_Start(wb)
//
typedef struct
{
    boolean	in;	// whether the player is in game
    
    // Player stats, kills, collected items etc.
    int		skills;
    int		sitems;
    int		ssecret;
    int		stime; 
    int		frags[4];
    int		score;	// current score on entry, modified on return
  
} wbplayerstruct_t;

typedef struct
{
    int		epsd;	// episode # (0-2)

    // if true, splash the secret level
    boolean	didsecret;
    
    // previous and next levels, origin 0
    int		last;
    int		next;	
    
    int		maxkills;
    int		maxitems;
    int		maxsecret;
    int		maxfrags;

    // the par time
    int		partime;
    
    // index of this player in game
    int		pnum;	

    wbplayerstruct_t	plyr[MAXPLAYERS];

} wbstartstruct_t;


#endif

