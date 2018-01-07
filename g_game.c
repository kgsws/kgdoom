#include <string.h>
#include <stdlib.h>

#include "doomdef.h" 
#include "doomstat.h"

#include "z_zone.h"
#include "f_finale.h"
#include "m_argv.h"
#include "m_misc.h"
#include "m_menu.h"
#include "m_random.h"
#include "i_system.h"

#include "p_setup.h"
#include "p_saveg.h"
#include "p_tick.h"

#include "d_main.h"

#include "wi_stuff.h"
#include "hu_stuff.h"
#include "st_stuff.h"
#include "am_map.h"

// Needs access to LFB.
#include "v_video.h"

#include "w_wad.h"

#include "p_local.h" 

#include "s_sound.h"

// Data.
#include "dstrings.h"
#include "sounds.h"

// SKY handling - still the wrong place.
#include "r_data.h"
#include "r_sky.h"

#include "g_game.h"

#ifdef SERVER
#include <netinet/in.h>
#include "network.h"
#include "sv_cmds.h"
#endif


#define SAVEGAMESIZE	0x2c000
#define SAVESTRINGSIZE	24



void	G_PlayerReborn (int player); 
void	G_InitNew (skill_t skill, int episode, int map); 
 
void	G_DoReborn (int playernum); 
 
void	G_DoLoadLevel (void); 
void	G_DoNewGame (void); 
void	G_DoLoadGame (void); 
void	G_DoCompleted (void); 
void	G_DoVictory (void); 
void	G_DoWorldDone (void); 
void	G_DoSaveGame (void); 
 
 
gameaction_t    gameaction; 
gamestate_t     gamestate; 
skill_t         gameskill; 
boolean		respawnmonsters;
int             gameepisode; 
int             gamemap; 
 
boolean         paused; 
boolean         sendpause;             	// send a pause event next tic 
boolean         sendsave;             	// send a save event next tic 
boolean         usergame;               // ok to save / end game 
 
boolean         nodrawers;              // for comparative timing purposes 
boolean         noblit;                 // for comparative timing purposes 
int             starttime;          	// for comparative timing purposes  	 
 
boolean         viewactive; 
 
boolean         deathmatch;           	// only if started as net death 
int         netgame;                // only true if packets are broadcast 

#ifdef SERVER
boolean         playeringame[MAXPLAYERS];
player_t        players[MAXPLAYERS];
#else
// [kg] last slot for local spectator
boolean         playeringame[MAXPLAYERS+1];
player_t        players[MAXPLAYERS+1];
// [kg] singleplayer respawn backup
player_t        prespawn;
#endif

int             consoleplayer;          // player taking events and displaying 
int             displayplayer;          // view being displayed 
int             gametic; 
int             levelstarttic;          // gametic at level start 
int             totalkills, totalitems, totalsecret;    // for intermission 
 
boolean         precache = true;        // if true, load all graphics at start 
 
wbstartstruct_t wminfo;               	// parms for world map / intermission 
 
byte*		savebuffer;

 
// 
// controls (have defaults) 
// 
int             key_right;
int		key_left;

int		key_lup;
int		key_ldown;

int		key_up;
int		key_down;
int             key_strafeleft;
int		key_straferight;
int             key_fire;
int		key_use;
int		key_strafe;
int		key_speed;
 
int             mousebfire;
int             mousebstrafe;
int             mousebforward;
int		mousebuse;


#define MAXPLMOVE		(forwardmove[1]) 
 
#define TURBOTHRESHOLD	0x32

fixed_t		forwardmove[2] = {0x19, 0x32};
fixed_t		sidemove[2] = {0x18, 0x28};
fixed_t		anglemove = 1280 << 16;
short		pitchmove = 50;

fixed_t		joyforwardmove[2] = {550*2, 550};
fixed_t		joysidemove[2] = {770*2, 770};

#define SLOWTURNTICS	6 
 
boolean         gamekeydown[NUMKEYS]; 
int             turnheld;				// for accelerative turning 
 
boolean		mousearray[4]; 
boolean*	mousebuttons = &mousearray[1];		// allow [-1]

// mouse values are used once 
int             mousex;
int		mousey;         

int             dclicktime;
int		dclickstate;
int		dclicks; 
int             dclicktime2;
int		dclickstate2;
int		dclicks2;

// joystick values are repeated 
int             joyxmove;
int		joyymove;
int             joyxmove2;
int		joyymove2;
boolean		joybuttons[NUM_JOYCON_BUTTONS];
 
int		savegameslot; 
char		savedescription[32];

// [kg] joycon info
extern int i_ctrl_roles;
extern int i_ctrl_btn[];
 
 
#define	BODYQUESIZE	32

mobj_t*		bodyque[BODYQUESIZE]; 
int		bodyqueslot; 
 
void*		statcopy;				// for statistics driver
 
int G_CmdChecksum (ticcmd_t* cmd) 
{ 
    int		i;
    int		sum = 0; 
	 
    for (i=0 ; i< sizeof(*cmd)/4 - 1 ; i++) 
	sum += ((int *)cmd)[i]; 
		 
    return sum; 
} 
 
#ifndef SERVER
//
// G_BuildTiccmd
// Builds a ticcmd from all of the available inputs.
// 
void G_BuildTiccmd (ticcmd_t* cmd) 
{ 
    int		i; 
    boolean	strafe;
    boolean	bstrafe; 
    int		speed;
    int		tspeed; 
    int		forward;
    int		side;

    angle_t	angleturn;
    fixed_t     pitchturn;
    mobj_t *mo = players[consoleplayer].mo;
    
    ticcmd_t*	base;

    base = I_BaseTiccmd ();		// empty, or external driver
    memcpy (cmd,base,sizeof(*cmd));

    if(!mo)
	return;

    angleturn = mo->angle;
    pitchturn = mo->pitch;

    speed = !(gamekeydown[key_speed] || joybuttons[joybspeed]);
 
    forward = side = 0;

    // turning
    if (gamekeydown[key_right])
	angleturn -= anglemove;
    if (gamekeydown[key_left])
	angleturn += anglemove;
    if (gamekeydown[key_ldown])
	pitchturn -= pitchmove;
    if (gamekeydown[key_lup])
	pitchturn += pitchmove;

    if(i_ctrl_roles)
    {
	angleturn -= joyxmove * 2345;
	pitchturn += joyymove / 6;
    } else
    {
	angleturn -= joyxmove2 * 2345;
	pitchturn += joyymove2 / 6;
    }

    if(i_ctrl_roles)
    {
	side = joyxmove2 / joysidemove[speed];
	forward = joyymove2 / joyforwardmove[speed];
    } else
    {
	side = joyxmove / joysidemove[speed];
	forward = joyymove / joyforwardmove[speed];
    }

    if (gamekeydown[key_up]) 
	forward += forwardmove[speed]; 
    if (gamekeydown[key_down]) 
	forward -= forwardmove[speed]; 

    if (gamekeydown[key_straferight]) 
	side += sidemove[speed]; 
    if (gamekeydown[key_strafeleft]) 
	side -= sidemove[speed];

    if (gamekeydown[key_fire] || mousebuttons[mousebfire] || joybuttons[joybfire]) 
	cmd->buttons |= BT_ATTACK; 
 
    if (gamekeydown[key_use] || joybuttons[joybuse] || mousebuttons[mousebuse] ) 
    { 
	cmd->buttons |= BT_USE;
	// clear double clicks if hit use button 
	dclicks = 0;                   
    } 

    // weapon menu
    i = ST_GetNewWeapon();
    if(i < NUMWEAPONS)
    { 
	cmd->buttons |= BT_CHANGE;
	cmd->weapon = i;
    } else
	// or send current weapon
	cmd->weapon = players[consoleplayer].readyweapon;

    // mouse
    if (mousebuttons[mousebforward]) 
	forward += forwardmove[speed];
    
    // forward double click
    if (mousebuttons[mousebforward] != dclickstate && dclicktime > 1 ) 
    { 
	dclickstate = mousebuttons[mousebforward]; 
	if (dclickstate) 
	    dclicks++; 
	if (dclicks == 2) 
	{ 
	    cmd->buttons |= BT_USE; 
	    dclicks = 0; 
	} 
	else 
	    dclicktime = 0; 
    } 
    else 
    { 
	dclicktime++; 
	if (dclicktime > 20) 
	{ 
	    dclicks = 0; 
	    dclickstate = 0; 
	} 
    }

    angleturn -= mousex << 16;
    pitchturn -= mousey;

    mousex = mousey = 0;

    if (forward > MAXPLMOVE) 
	forward = MAXPLMOVE; 
    else if (forward < -MAXPLMOVE) 
	forward = -MAXPLMOVE; 
    if (side > MAXPLMOVE) 
	side = MAXPLMOVE; 
    else if (side < -MAXPLMOVE) 
	side = -MAXPLMOVE; 
 
    cmd->forwardmove = forward;
    cmd->sidemove = side;
    cmd->angle = angleturn;
    cmd->pitch = pitchturn;
} 
#endif

//
// G_DoLoadLevel 
//
 
void G_DoLoadLevel (void) 
{ 
    int             i; 

    // Set the sky map.
    // First thing, we have a dummy sky texture name,
    //  a flat. The data is in the WAD only because
    //  we look for an actual index, instead of simply
    //  setting one.
    skyflatnum = R_FlatNumForName ( SKYFLATNAME );

    // DOOM determines the sky texture to be used
    // depending on the current episode, and the game version.
    if ( (gamemode == commercial)
	 || ( gamemode == pack_tnt )
	 || ( gamemode == pack_plut ) )
    {
	skytexture = R_TextureNumForName ("SKY3");
	if (gamemap < 12)
	    skytexture = R_TextureNumForName ("SKY1");
	else
	    if (gamemap < 21)
		skytexture = R_TextureNumForName ("SKY2");
    }

    levelstarttic = gametic;        // for time calculation
    
    gamestate = GS_LEVEL; 

#ifdef SERVER
    for (i=0 ; i<MAXPLAYERS ; i++) 
#else
    for (i=0 ; i<=MAXPLAYERS ; i++) 
#endif
    { 
	if (playeringame[i] && players[i].playerstate == PST_DEAD) 
	    players[i].playerstate = PST_REBORN; 
	memset (players[i].frags,0,sizeof(players[i].frags)); 
    } 
		 
    P_SetupLevel (gameepisode, gamemap, 0, gameskill);    
    displayplayer = consoleplayer;		// view the guy you are playing    
    starttime = I_GetTime (); 
    gameaction = ga_nothing; 
    Z_CheckHeap ();
    
    // clear cmd building stuff
    memset (gamekeydown, 0, sizeof(gamekeydown)); 
    joyxmove = joyymove = 0;
    joyxmove2 = joyymove2 = 0;
    mousex = mousey = 0; 
    sendpause = sendsave = paused = false; 
    memset (mousearray, 0, sizeof(mousearray)); 
    memset (joybuttons, 0, sizeof(joybuttons)); 
}

#ifndef SERVER
//
// G_Responder  
// Get info needed to make ticcmd_ts for the players.
// 
boolean G_Responder (event_t* ev) 
{ 
    int i;

    // allow spy mode changes
    if (gamestate == GS_LEVEL && ev->type == ev_keydown 
	&& ev->data1 == KEY_F12 && !deathmatch )
    {
	// spy mode 
	do 
	{ 
	    displayplayer++;
	    if (displayplayer == MAXPLAYERS+1) 
		displayplayer = 0; 
	} while (!playeringame[displayplayer] && displayplayer != consoleplayer); 
	return true; 
    }

    // any other key pops up menu if in demos
    if (gameaction == ga_nothing && gamestate == GS_DEMOSCREEN) 
    { 
	if (ev->type == ev_keydown ||  
	    (ev->type == ev_mouse && ev->data1) || 
	    (ev->type == ev_joystick && ev->data1) ) 
	{ 
	    M_StartControlPanel (); 
	    return true; 
	} 
	return false; 
    }
    
    if (gamestate == GS_LEVEL) 
    { 
#if 0 
	if (devparm && ev->type == ev_keydown && ev->data1 == ';') 
	{ 
	    G_DeathMatchSpawnPlayer (0); 
	    return true; 
	} 
#endif 
	if (HU_Responder (ev)) 
	    return true;	// chat ate the event 
	if (ST_Responder (ev)) 
	    return true;	// status window ate it 
	if (AM_Responder (ev)) 
	    return true;	// automap ate it 
    } 
	 
    if (gamestate == GS_FINALE) 
    { 
	if (F_Responder (ev)) 
	    return true;	// finale ate the event 
    } 
	 
    switch (ev->type) 
    { 
      case ev_keydown: 
	if (ev->data1 == KEY_PAUSE) 
	{ 
	    sendpause = true; 
	    return true; 
	} 
	if (ev->data1 <NUMKEYS) 
	    gamekeydown[ev->data1] = true; 
	return true;    // eat key down events 
 
      case ev_keyup: 
	if (ev->data1 <NUMKEYS) 
	    gamekeydown[ev->data1] = false; 
	return false;   // always let key up events filter down 
		 
      case ev_mouse:
	if(in_weapon_menu)
	{
	    mousebuttons[0] = 0;
	    mousebuttons[1] = 0;
	    mousebuttons[2] = 0;
	    mousex = 0;
	    mousey = 0;
	} else
	{
	    mousebuttons[0] = ev->data1 & 1; 
	    mousebuttons[1] = ev->data1 & 2; 
	    mousebuttons[2] = ev->data1 & 4; 
	    mousex = ev->data2 * mouseSensitivity * 7;
	    mousey = ev->data3 * mouseSensitivity * 7;
	}
	return true;    // eat events 

// [kg] joycons
      case ev_joystick:
	if(in_weapon_menu)
	{
	    for(i = 0; i < NUM_JOYCON_BUTTONS; i++)
		joybuttons[i] = 0;
	    joyxmove = 0;
	    joyymove = 0;
	} else
	{
	    for(i = 0; i < NUM_JOYCON_BUTTONS; i++)
		joybuttons[i] = ev->data1 & (1 << i_ctrl_btn[i]);
	    joyxmove = ev->data2;
	    joyymove = ev->data3;
	}
	return false;    // don't eat events

      case ev_joystick2:
	if(in_weapon_menu)
	{
	    joyxmove2 = 0;
	    joyymove2 = 0;
	} else
	{
	    joyxmove2 = ev->data2;
	    joyymove2 = ev->data3;
	}
	return true;    // eat events
 
      default: 
	break; 
    } 
 
    return false; 
} 
#endif 

#ifdef SERVER
void SV_ExitLevel();
#endif

//
// G_Ticker
// Make ticcmd_ts for the players.
//
void G_Ticker (void) 
{ 
    int		i;
    int		buf; 
    ticcmd_t*	cmd;
    
    // do player reborns if needed
#ifndef SERVER
    if(!netgame)
#endif
    for (i=0 ; i<MAXPLAYERS ; i++) 
	if (playeringame[i] && (players[i].playerstate == PST_REBORN || players[i].playerstate == PST_RESPAWN))
	    G_DoReborn (i);

    // do things to change the game state
    while (gameaction != ga_nothing) 
    { 
	switch (gameaction) 
	{ 
	  case ga_loadlevel: 
	    G_DoLoadLevel (); 
	    break; 
	  case ga_newgame: 
	    G_DoNewGame (); 
	    break; 
#ifndef SERVER
	  case ga_loadgame: 
	    G_DoLoadGame (); 
	    break; 
	  case ga_savegame: 
	    G_DoSaveGame (); 
	    break; 
#endif
	  case ga_completed: 
#ifdef SERVER
	    SV_ExitLevel();
#else
	    G_DoCompleted (); 
#endif
	    break; 
	  case ga_victory: 
#ifndef SERVER
	    F_StartFinale ();
#endif
	    break; 
	  case ga_worlddone: 
	    G_DoWorldDone (); 
	    break; 
	  case ga_screenshot: 
//	    M_ScreenShot (); 
	    gameaction = ga_nothing; 
	    break; 
	  case ga_nothing: 
	    break; 
	} 
    }
#ifdef SERVER
    switch (gamestate) 
    { 
      case GS_LEVEL:
	P_Ticker ();
	break;
    }
#else
    // do main actions
    switch (gamestate) 
    { 
      case GS_LEVEL:
	P_Ticker ();
	ST_Ticker ();
	AM_Ticker ();
	HU_Ticker ();
	break; 
	 
      case GS_INTERMISSION: 
	WI_Ticker (); 
	break; 
			 
      case GS_FINALE: 
	F_Ticker (); 
	break; 
 
      case GS_DEMOSCREEN: 
	D_PageTicker (); 
	break; 
    }
#endif
} 
 
 
//
// PLAYER STRUCTURE FUNCTIONS
// also see P_SpawnPlayer in P_Things
//


//
// G_PlayerFinishLevel
// Can when a player completes a level.
//
void G_PlayerFinishLevel (int player)
{ 
    player_t*	p; 
	 
    p = &players[player]; 
	 
    memset (p->powers, 0, sizeof (p->powers)); 
    memset (p->cards, 0, sizeof (p->cards)); 
    p->mo->flags &= ~MF_SHADOW;		// cancel invisibility 
    p->extralight = 0;			// cancel gun flashes 
    p->fixedcolormap = 0;		// cancel ir gogles 
    p->damagecount = 0;			// no palette changes 
    p->bonuscount = 0;
#ifndef SERVER
    memcpy(&prespawn, p, sizeof(player_t));
#endif
} 
 

//
// G_PlayerReborn
// Called after a player dies 
// almost everything is cleared and initialized
// [kg] handle level reload, restore inventory
//
void G_PlayerReborn (int player) 
{ 
    player_t*	p;
#ifdef SERVER
    int		i;
    int		frags[MAXPLAYERS];
    int		killcount;
    int		itemcount;
    int		secretcount;
#endif

    p = &players[player];

#ifdef SERVER
    memcpy (frags,players[player].frags,sizeof(frags)); 
    killcount = players[player].killcount; 
    itemcount = players[player].itemcount; 
    secretcount = players[player].secretcount; 

    memset (p, 0, sizeof(*p)); 

    memcpy (players[player].frags, frags, sizeof(players[player].frags)); 
    players[player].killcount = killcount; 
    players[player].itemcount = itemcount; 
    players[player].secretcount = secretcount;

    for (i=0 ; i<NUMAMMO ; i++)
	p->maxammo[i] = maxammo[i];

    p->playerstate = PST_LIVE;
    p->health = MAXHEALTH;
    p->readyweapon = p->pendingweapon = wp_pistol;
    p->weaponowned[wp_fist] = true;
    p->weaponowned[wp_pistol] = true;
    p->ammo[am_clip] = 50;
#else
    memcpy(p, &prespawn, sizeof(player_t));
    p->mo = NULL;
#endif
    p->usedown = p->attackdown = true;	// don't do anything immediately 
}

//
// G_CheckSpot  
// Returns false if the player cannot be respawned
// at the given mapthing_t spot  
// because something is occupying it 
//
void P_SpawnPlayer (mapthing_hexen_t* mthing, int netplayer);
 
boolean
G_CheckSpot
( int		playernum,
  mapthing_hexen_t*	mthing ) 
{ 
    fixed_t		x;
    fixed_t		y; 
    subsector_t*	ss; 
    angle_t		an; 
    mobj_t*		mo; 
    int			i;

#ifndef SERVER
    if (!players[playernum].mo)
    {
	// first spawn of level, before corpses
	for (i=0 ; i<playernum ; i++)
	    if (players[i].mo->x == mthing->x << FRACBITS
		&& players[i].mo->y == mthing->y << FRACBITS)
		return false;	
	return true;
    }
#endif

    x = mthing->x << FRACBITS; 
    y = mthing->y << FRACBITS; 

#ifdef SERVER
    if (players[playernum].mo && !P_CheckPosition (players[playernum].mo, x, y) ) 
	return false; 
 
    // flush an old corpse if needed
    if (bodyqueslot >= BODYQUESIZE)
	P_RemoveMobj (bodyque[bodyqueslot%BODYQUESIZE], true); 

    if(players[playernum].mo)
    {
	bodyque[bodyqueslot%BODYQUESIZE] = players[playernum].mo; 
	bodyqueslot++; 
    }
#else
    if (!P_CheckPosition (players[playernum].mo, x, y) ) 
	return false; 

    // flush an old corpse if needed
    if (bodyqueslot >= BODYQUESIZE)
	P_RemoveMobj (bodyque[bodyqueslot%BODYQUESIZE]);

    bodyque[bodyqueslot%BODYQUESIZE] = players[playernum].mo; 
    bodyqueslot++; 
#endif

    // spawn a teleport fog 
    ss = R_PointInSubsector (x,y); 
    an = (angle_t)( ANG45 * (mthing->angle/45) ) >> ANGLETOFINESHIFT; 

    mo = P_SpawnMobj (x+20*finecosine[an], y+20*finesine[an] 
		      , ss->sector->floorheight 
		      , MT_TFOG);

    if (mo->info->seesound) 
	S_StartSound (mo, mo->info->seesound, SOUND_BODY);
#ifdef SERVER
    // tell clients about this
    SV_SpawnMobj(mo, SV_MOBJF_SOUND_SEE);
#endif

    return true; 
} 


//
// G_DeathMatchSpawnPlayer 
// Spawns a player at one of the random death match spots 
// called at level load and each death 
//
void G_DeathMatchSpawnPlayer (int playernum) 
{ 
    int             i,j; 
    int				selections; 
	 
    selections = deathmatch_p - deathmatchstarts; 
    if (selections < 4) 
	I_Error ("Only %i deathmatch spots, 4 required", selections); 
 
    for (j=0 ; j<20 ; j++) 
    { 
	i = P_Random() % selections; 
	if (G_CheckSpot (playernum, &deathmatchstarts[i]) ) 
	{ 
	    deathmatchstarts[i].type = playernum+1; 
	    P_SpawnPlayer (&deathmatchstarts[i], playernum); 
	    return; 
	} 
    } 
 
    // no good spot, so the player will probably get stuck 
    P_SpawnPlayer (&playerstarts[playernum], -1);
} 

//
// G_DoReborn 
// 
void G_DoReborn (int playernum) 
{ 
    int                             i; 
	 
    if (!netgame)
    {
	// reload the level from scratch
	gameaction = ga_loadlevel;
    }
    else 
    {
	// respawn at the start

	// first dissasociate the corpse 
	if(players[playernum].mo)
		players[playernum].mo->player = NULL;   

	// spawn at random spot if in death match 
	if (deathmatch) 
	{ 
	    G_DeathMatchSpawnPlayer (playernum); 
	    return; 
	} 

	if (G_CheckSpot (playernum, &playerstarts[playernum]) ) 
	{ 
	    P_SpawnPlayer (&playerstarts[playernum], playernum); 
	    return; 
	}

	// try to spawn at one of the other players spots 
	for (i=0 ; i<MAXPLAYERS ; i++)
	{
	    if (G_CheckSpot (playernum, &playerstarts[i]) ) 
	    { 
		playerstarts[i].type = playernum+1;	// fake as other player 
		P_SpawnPlayer (&playerstarts[i], playernum); 
		playerstarts[i].type = i+1;		// restore 
		return; 
	    }	    
	    // he's going to be inside something.  Too bad.
	}
	P_SpawnPlayer (&playerstarts[playernum], playernum); 
    }
} 
 
 
void G_ScreenShot (void) 
{ 
    gameaction = ga_screenshot; 
} 
 


// DOOM Par Times
int pars[4][10] = 
{ 
    {0}, 
    {0,30,75,120,90,165,180,180,30,165}, 
    {0,90,90,90,120,90,360,240,30,170}, 
    {0,90,45,90,150,90,90,165,30,135} 
}; 

// DOOM II Par Times
int cpars[32] =
{
    30,90,120,120,90,150,120,120,270,90,	//  1-10
    210,150,150,150,210,150,420,150,210,150,	// 11-20
    240,150,180,150,150,300,330,420,300,180,	// 21-30
    120,30					// 31-32
};
 

//
// G_DoCompleted 
//
boolean		secretexit;
extern char*	pagename;
 
void G_ExitLevel (void) 
{ 
    secretexit = false; 
    gameaction = ga_completed; 
} 

// Here's for the german edition.
void G_SecretExitLevel (void) 
{ 
    // IF NO WOLF3D LEVELS, NO SECRET EXIT!
    if ( (gamemode == commercial)
      && (W_CheckNumForName("map31")<0))
	secretexit = false;
    else
	secretexit = true; 
    gameaction = ga_completed; 
} 

#ifndef SERVER
void G_DoCompleted (void) 
{
    int             i; 
	 
    gameaction = ga_nothing; 

    for (i=0 ; i<=MAXPLAYERS ; i++) 
	if (playeringame[i]) 
	    G_PlayerFinishLevel (i);        // take away cards and stuff 

    if (automapactive) 
	AM_Stop (); 

    if ( gamemode != commercial)
	switch(gamemap)
	{
	  case 8:
	    gameaction = ga_victory;
	    return;
	  case 9: 
	    for (i=0 ; i<MAXPLAYERS ; i++) 
		players[i].didsecret = true; 
	    break;
	}
		
//#if 0  Hmmm - why?
    if ( (gamemap == 8)
	 && (gamemode != commercial) ) 
    {
	// victory 
	gameaction = ga_victory; 
	return; 
    } 
	 
    if ( (gamemap == 9)
	 && (gamemode != commercial) ) 
    {
	// exit secret level 
	for (i=0 ; i<MAXPLAYERS ; i++) 
	    players[i].didsecret = true; 
    } 
//#endif
    
	 
    wminfo.didsecret = players[consoleplayer].didsecret; 
    wminfo.epsd = gameepisode -1; 
    wminfo.last = gamemap -1;
    
    // wminfo.next is 0 biased, unlike gamemap
    if ( gamemode == commercial)
    {
	if (secretexit)
	    switch(gamemap)
	    {
	      case 15: wminfo.next = 30; break;
	      case 31: wminfo.next = 31; break;
	    }
	else
	    switch(gamemap)
	    {
	      case 31:
	      case 32: wminfo.next = 15; break;
	      default: wminfo.next = gamemap;
	    }
    }
    else
    {
	if (secretexit) 
	    wminfo.next = 8; 	// go to secret level 
	else if (gamemap == 9) 
	{
	    // returning from secret level 
	    switch (gameepisode) 
	    { 
	      case 1: 
		wminfo.next = 3; 
		break; 
	      case 2: 
		wminfo.next = 5; 
		break; 
	      case 3: 
		wminfo.next = 6; 
		break; 
	      case 4:
		wminfo.next = 2;
		break;
	    }                
	} 
	else 
	    wminfo.next = gamemap;          // go to next level 
    }
		 
    wminfo.maxkills = totalkills; 
    wminfo.maxitems = totalitems; 
    wminfo.maxsecret = totalsecret; 
    wminfo.maxfrags = 0; 
    if ( gamemode == commercial )
	wminfo.partime = 35*cpars[gamemap-1]; 
    else
	wminfo.partime = 35*pars[gameepisode][gamemap]; 
    wminfo.pnum = consoleplayer; 
 
    for (i=0 ; i<MAXPLAYERS ; i++) 
    { 
	wminfo.plyr[i].in = playeringame[i]; 
	wminfo.plyr[i].skills = players[i].killcount; 
	wminfo.plyr[i].sitems = players[i].itemcount; 
	wminfo.plyr[i].ssecret = players[i].secretcount; 
	wminfo.plyr[i].stime = leveltime; 
	memcpy (wminfo.plyr[i].frags, players[i].frags 
		, sizeof(wminfo.plyr[i].frags)); 
    } 
 
    gamestate = GS_INTERMISSION; 
    viewactive = false; 
    automapactive = false; 
 
    if (statcopy)
	memcpy (statcopy, &wminfo, sizeof(wminfo));

    WI_Start (&wminfo); 
} 
#endif

//
// G_WorldDone 
//
void G_WorldDone (void) 
{ 
    gameaction = ga_worlddone; 

    if (secretexit) 
	players[consoleplayer].didsecret = true; 
#ifndef SERVER
    if ( gamemode == commercial )
    {
	switch (gamemap)
	{
	  case 15:
	  case 31:
	    if (!secretexit)
		break;
	  case 6:
	  case 11:
	  case 20:
	  case 30:
	    F_StartFinale ();
	    break;
	}
    }
#endif
} 
 
void G_DoWorldDone (void) 
{        
    gamestate = GS_LEVEL; 
    gamemap = wminfo.next+1; 
    G_DoLoadLevel (); 
    gameaction = ga_nothing; 
    viewactive = true; 
} 
 

#ifndef SERVER
//
// G_InitFromSavegame
// Can be called by the startup code or the menu task. 
//
extern boolean setsizeneeded;
void R_ExecuteSetViewSize (void);

char	savename[256];

void G_LoadGame (char* name) 
{ 
    strcpy (savename, name); 
    gameaction = ga_loadgame; 
} 
 
#define VERSIONSIZE		16 

void G_DoLoadGame (void) 
{ 
/*    int		length; 
    int		i; 
    int		a,b,c; 
    char	vcheck[VERSIONSIZE]; 
	 
    gameaction = ga_nothing; 
	 
    length = M_ReadFile (savename, &savebuffer); 
    save_p = savebuffer + SAVESTRINGSIZE;
    
    // skip the description field 
    memset (vcheck,0,sizeof(vcheck)); 
    sprintf (vcheck,"version %i",VERSION); 
    if (strcmp (save_p, vcheck)) 
	return;				// bad version 
    save_p += VERSIONSIZE; 
			 
    gameskill = *save_p++; 
    gameepisode = *save_p++; 
    gamemap = *save_p++; 
    for (i=0 ; i<MAXPLAYERS ; i++) 
	playeringame[i] = *save_p++; 

    // load a base level 
    G_InitNew (gameskill, gameepisode, gamemap); 
 
    // get the times 
    a = *save_p++; 
    b = *save_p++; 
    c = *save_p++; 
    leveltime = (a<<16) + (b<<8) + c; 
	 
    // dearchive all the modifications
    P_UnArchivePlayers (); 
    P_UnArchiveWorld (); 
    P_UnArchiveThinkers (); 
    P_UnArchiveSpecials (); 
 
    if (*save_p != 0x1d) 
	I_Error ("Bad savegame");
    
    // done 
    Z_Free (savebuffer); 
 
    if (setsizeneeded)
	R_ExecuteSetViewSize ();
    
    // draw the pattern into the back screen
//    R_FillBackScreen ();   */
} 
 

//
// G_SaveGame
// Called by the menu task.
// Description is a 24 byte text string 
//
void
G_SaveGame
( int	slot,
  char*	description ) 
{ 
    savegameslot = slot; 
    strcpy (savedescription, description); 
    sendsave = true; 
} 
 
void G_DoSaveGame (void) 
{ 
/*    char	name[100]; 
    char	name2[VERSIONSIZE]; 
    char*	description; 
    int		length; 
    int		i; 
	
    if (M_CheckParm("-cdrom"))
	sprintf(name,"c:\\doomdata\\"SAVEGAMENAME"%d.dsg",savegameslot);
    else
	sprintf (name,SAVEGAMENAME"%d.dsg",savegameslot); 
    description = savedescription; 
	 
    save_p = savebuffer = screens[1]+0x4000; 
	 
    memcpy (save_p, description, SAVESTRINGSIZE); 
    save_p += SAVESTRINGSIZE; 
    memset (name2,0,sizeof(name2)); 
    sprintf (name2,"version %i",VERSION); 
    memcpy (save_p, name2, VERSIONSIZE); 
    save_p += VERSIONSIZE; 
	 
    *save_p++ = gameskill; 
    *save_p++ = gameepisode; 
    *save_p++ = gamemap; 
    for (i=0 ; i<MAXPLAYERS ; i++) 
	*save_p++ = playeringame[i]; 
    *save_p++ = leveltime>>16; 
    *save_p++ = leveltime>>8; 
    *save_p++ = leveltime; 
 
    P_ArchivePlayers (); 
    P_ArchiveWorld (); 
    P_ArchiveThinkers (); 
    P_ArchiveSpecials (); 
	 
    *save_p++ = 0x1d;		// consistancy marker 
	 
    length = save_p - savebuffer; 
    if (length > SAVEGAMESIZE) 
	I_Error ("Savegame buffer overrun"); 
    M_WriteFile (name, savebuffer, length); 
    gameaction = ga_nothing; 
    savedescription[0] = 0;		 
	 
    players[consoleplayer].message = GGSAVED; 

    // draw the pattern into the back screen
//    R_FillBackScreen ();	*/
} 
#endif

//
// G_InitNew
// Can be called by the startup code or the menu task,
// consoleplayer, displayplayer, playeringame[] should be set. 
//
skill_t	d_skill; 
int     d_episode; 
int     d_map;

#ifndef SERVER
// [kg] reset saved inventory
void G_ResetPlayer()
{
	// [kg] reset saved inventory
	memset(&prespawn, 0, sizeof(player_t));
	prespawn.health = MAXHEALTH;
	prespawn.readyweapon = prespawn.pendingweapon = wp_pistol;
	prespawn.weaponowned[wp_fist] = true;
	prespawn.weaponowned[wp_pistol] = true;
	prespawn.ammo[am_clip] = 50;
	for (int i=0 ; i<NUMAMMO ; i++)
		prespawn.maxammo[i] = maxammo[i];
}
#endif
 
void
G_DeferedInitNew
( skill_t	skill,
  int		episode,
  int		map) 
{ 
    d_skill = skill; 
    d_episode = episode; 
    d_map = map; 
    gameaction = ga_newgame;
}


void G_DoNewGame (void) 
{
	if(!netgame)
	{
		deathmatch = false;
		playeringame[1] = playeringame[2] = playeringame[3] = 0;
		respawnparm = false;
		fastparm = false;
		nomonsters = false;
		consoleplayer = 0;
	}
#ifndef SERVER
	G_ResetPlayer();
#endif
	G_InitNew (d_skill, d_episode, d_map); 
	gameaction = ga_nothing;
} 

// The sky texture to be used instead of the F_SKY1 dummy.
extern  int	skytexture; 


void
G_InitNew
( skill_t	skill,
  int		episode,
  int		map ) 
{ 
    int             i; 
	 
    if (paused) 
    { 
	paused = false; 
	S_ResumeSound (); 
    } 
	

    if (skill > sk_nightmare) 
	skill = sk_nightmare;


    // This was quite messy with SPECIAL and commented parts.
    // Supposedly hacks to make the latest edition work.
    // It might not work properly.
    if (episode < 1)
      episode = 1; 

    if ( gamemode == retail )
    {
      if (episode > 4)
	episode = 4;
    }
    else if ( gamemode == shareware )
    {
      if (episode > 1) 
	   episode = 1;	// only start episode 1 on shareware
    }  
    else
    {
      if (episode > 3)
	episode = 3;
    }
    

  
    if (map < 1) 
	map = 1;
    
    if ( (map > 9)
	 && ( gamemode != commercial) )
      map = 9; 
		 
    M_ClearRandom (); 
	 
    if (skill == sk_nightmare || respawnparm )
	respawnmonsters = true;
    else
	respawnmonsters = false;
		
    if (fastparm || (skill == sk_nightmare && gameskill != sk_nightmare) )
    { 
	for (i=S_SARG_RUN1 ; i<=S_SARG_PAIN2 ; i++) 
	    states[i].tics >>= 1; 
	mobjinfo[MT_BRUISERSHOT].speed = 20*FRACUNIT; 
	mobjinfo[MT_HEADSHOT].speed = 20*FRACUNIT; 
	mobjinfo[MT_TROOPSHOT].speed = 20*FRACUNIT; 
    } 
    else if (skill != sk_nightmare && gameskill == sk_nightmare) 
    { 
	for (i=S_SARG_RUN1 ; i<=S_SARG_PAIN2 ; i++) 
	    states[i].tics <<= 1; 
	mobjinfo[MT_BRUISERSHOT].speed = 15*FRACUNIT; 
	mobjinfo[MT_HEADSHOT].speed = 10*FRACUNIT; 
	mobjinfo[MT_TROOPSHOT].speed = 10*FRACUNIT; 
    } 
	 
			 
    // force players to be initialized upon first level load         
    for (i=0 ; i<MAXPLAYERS ; i++) 
	players[i].playerstate = PST_REBORN; 

    if(!netgame) 
	usergame = true;
    paused = false;
    viewactive = true; 
    gameepisode = episode; 
    gamemap = map; 
    gameskill = skill;
#ifndef SERVER
    automapactive = false; 
#endif
    viewactive = true;
    
    // set the sky map for the episode
    if ( gamemode == commercial)
    {
	skytexture = R_TextureNumForName ("SKY3");
	if (gamemap < 12)
	    skytexture = R_TextureNumForName ("SKY1");
	else
	    if (gamemap < 21)
		skytexture = R_TextureNumForName ("SKY2");
    }
    else
	switch (episode) 
	{ 
	  case 1: 
	    skytexture = R_TextureNumForName ("SKY1"); 
	    break; 
	  case 2: 
	    skytexture = R_TextureNumForName ("SKY2"); 
	    break; 
	  case 3: 
	    skytexture = R_TextureNumForName ("SKY3"); 
	    break; 
	  case 4:	// Special Edition sky
	    skytexture = R_TextureNumForName ("SKY4");
	    break;
	} 
 
    G_DoLoadLevel (); 
} 

