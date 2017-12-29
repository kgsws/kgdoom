#include <stdio.h>

#include "doomdef.h"

#include "i_system.h"
#include "i_video.h"
#include "z_zone.h"
#include "m_random.h"
#include "w_wad.h"

#include "doomdef.h"

#include "m_swap.h"

#include "g_game.h"

#include "st_stuff.h"
#include "st_lib.h"
#include "r_local.h"

#include "p_local.h"
#include "p_inter.h"

#include "am_map.h"
#include "m_cheat.h"

#include "s_sound.h"

// Needs access to LFB.
#include "v_video.h"

// [kg] blood translation
#include "r_draw.h"

// State.
#include "doomstat.h"

// Data.
#include "dstrings.h"
#include "sounds.h"

//
// STATUS BAR DATA
//

// [kg] new status bar
#define STBAR_GLOBAL_Y	(SCREENHEIGHT - 64)

#define STBAR_HEALTH_X	100
#define STBAR_HEALTH_Y	STBAR_GLOBAL_Y

#define STBAR_ARMOR_X	216
#define STBAR_ARMOR_Y	STBAR_GLOBAL_Y

#define STBAR_AMMO_X	(SCREENWIDTH - 16)
#define STBAR_AMMO_Y	STBAR_GLOBAL_Y

#define STBAR_KEY_X	(SCREENWIDTH - 8)
#define STBAR_KEY_Y	4

#define STBAR_WEAP_X	(SCREENWIDTH/2)
#define STBAR_WEAP_Y	(SCREENHEIGHT/2)

// Palette indices.
// For damage/bonus red-/gold-shifts
#define STARTREDPALS		1
#define STARTBONUSPALS		9
#define NUMREDPALS			8
#define NUMBONUSPALS		4
// Radiation suit, green shift.
#define RADIATIONPAL		13

// crosshair
static uint8_t pointer[] =
{
	8, 0,	// witdh
	8, 0,	// height
	0, 0,	// left
	0, 0,	// top
	40, 0, 0, 0, // col0
	40, 0, 0, 0, // col1
	40, 0, 0, 0, // col2
	47, 0, 0, 0, // col3
	47, 0, 0, 0, // col4
	40, 0, 0, 0, // col5
	40, 0, 0, 0, // col6
	40, 0, 0, 0, // col7
	// colA
	3, // top
	2, // length
	0, // garbage
	0, 0, // pixels
	0, // garbage
	0xFF, // end
	// colB
	0, // top
	8, // length
	0, // garbage
	0, 0, 0, 0, 0, 0, 0, 0, // pixels
	0, // garbage
	0xFF, // end
};

// main player in game
static player_t*	plyr; 

// lump number for PLAYPAL
static int		lu_palette;

// 3 key-cards, 3 skulls
static patch_t*		keys[NUMCARDS];

// backgrounds
static patch_t*		hp_back;
static patch_t*		armor_back[2];
static patch_t*		pack_back;
static patch_t*		ammo_back[NUMAMMO];

static patch_t*		weap_back[NUMWEAPONS];
static patch_t*		empty_back;

// death timeout
static int deathtime;

// tables
static char *const ammo_icon[NUMAMMO] = {"AMMOA0", "SBOXA0", "CELPA0", "BROKA0"};
static char *const weap_icon[NUMWEAPONS] = {"PUNGC0", "PISGC0", "SHOTA0", "MGUNA0", "LAUNA0", "PLASA0", "BFUGA0", "CSAWA0", "SGN2A0"};
static const int weap_offs_x[8] = {0  , 200, 300, 200 , 0   , -200, -300, -200};
static const int weap_offs_y[8] = {200, 150, 0  , -150, -200, -150, 0   , 150};

// [kg] weapon menu
boolean in_weapon_menu;
static weapontype_t weapon_change;
static weapontype_t weapon_select;
static int weapon_shotgun = wp_shotgun;

// [kg] ctrl
extern int absmousex;
extern int absmousey;
extern int joyxmove;
extern int joyymove;
extern int joyxmove2;
extern int joyymove2;

// Massive bunches of cheat shit
//  to keep it from being easy to figure them out.
// Yeah, right...
unsigned char	cheat_mus_seq[] =
{
    "idmus\x01\0\0"
};

unsigned char	cheat_choppers_seq[] =
{
    "idchoppers"
};

unsigned char	cheat_god_seq[] =
{
    "iddqd"
};

unsigned char	cheat_ammo_seq[] =
{
    "idkfa"
};

unsigned char	cheat_ammonokey_seq[] =
{
    "idfa"
};

// Smashing Pumpkins Into Samml Piles Of Putried Debris. 
unsigned char	cheat_noclip_seq[] =
{
    "idspispopd"
};

//
unsigned char	cheat_commercial_noclip_seq[] =
{
    "idclip"
}; 

// [kg] inf. ammo
unsigned char	cheat_infammo_seq[] =
{
    "kgammo"
};

// [kg] inf. health
unsigned char	cheat_infhealth_seq[] =
{
    "kgheal"
};

// [kg] kill all visible monsters
unsigned char	cheat_viskill_seq[] =
{
    "kgkill"
};

// [kg] kill aura
unsigned char	cheat_killaura_seq[] =
{
    "kgdeath"
};

// [kg] safe aura
unsigned char	cheat_safeaura_seq[] =
{
    "kgsafe"
};

// [kg] revenge aura
unsigned char	cheat_revengeaura_seq[] =
{
    "kgavenge"
};

// [kg] slow motion
unsigned char	cheat_slowmo_seq[] =
{
    "kgslowmo"
};

// [kg] free/auto aim
unsigned char	cheat_aim_seq[] =
{
    "kgaim"
};

unsigned char	cheat_powerup_seq[7][10] =
{
    { "idbeholdv" }, 	// beholdv
    { "idbeholds" }, 	// beholds
    { "idbeholdi" }, 	// beholdi
    { "idbeholdr" }, 	// beholdr
    { "idbeholda" }, 	// beholda
    { "idbeholdl" }, 	// beholdl
    { "idbehold" }	// behold
};


unsigned char	cheat_clev_seq[] =
{
    "idclev\x01\0\0"
};

/*
// my position cheat
unsigned char	cheat_mypos_seq[] =
{
    "idmypos"
}; 
*/

// Now what?
cheatseq_t	cheat_mus = { cheat_mus_seq, 0 };
cheatseq_t	cheat_god = { cheat_god_seq, 0 };
cheatseq_t	cheat_ammo = { cheat_ammo_seq, 0 };
cheatseq_t	cheat_ammonokey = { cheat_ammonokey_seq, 0 };
cheatseq_t	cheat_noclip = { cheat_noclip_seq, 0 };
cheatseq_t	cheat_commercial_noclip = { cheat_commercial_noclip_seq, 0 };

cheatseq_t	cheat_infammo = { cheat_infammo_seq, 0 };
cheatseq_t	cheat_infhealth = { cheat_infhealth_seq, 0 };
cheatseq_t	cheat_viskill = { cheat_viskill_seq, 0 };
cheatseq_t	cheat_killaura = { cheat_killaura_seq, 0 };
cheatseq_t	cheat_safeaura = { cheat_safeaura_seq, 0 };
cheatseq_t	cheat_revengeaura = { cheat_revengeaura_seq, 0 };
cheatseq_t	cheat_slowmo = { cheat_slowmo_seq, 0 };
cheatseq_t	cheat_aim = { cheat_aim_seq, 0 };

cheatseq_t	cheat_powerup[7] =
{
    { cheat_powerup_seq[0], 0 },
    { cheat_powerup_seq[1], 0 },
    { cheat_powerup_seq[2], 0 },
    { cheat_powerup_seq[3], 0 },
    { cheat_powerup_seq[4], 0 },
    { cheat_powerup_seq[5], 0 },
    { cheat_powerup_seq[6], 0 }
};

cheatseq_t	cheat_choppers = { cheat_choppers_seq, 0 };
cheatseq_t	cheat_clev = { cheat_clev_seq, 0 };
//cheatseq_t	cheat_mypos = { cheat_mypos_seq, 0 };


// 
extern char*	mapnames[];


//
// STATUS BAR CODE
//

extern int i_ctrl_btn[];

// Respond to keyboard input events,
//  intercept cheats.
boolean
ST_Responder (event_t* ev)
{
	int i;
	angle_t an;
	player_t *plr = &players[consoleplayer];

	if(in_weapon_menu)
	{
		if(ev->type == ev_joystick)
		{
			if(!(ev->data1 & (1 << i_ctrl_btn[joybweapons])))
			{
				GrabMouse(1);
				in_weapon_menu = false;
				if(weapon_select < NUMWEAPONS && weapon_select != plr->readyweapon && plr->weaponowned[weapon_select])
					weapon_change = weapon_select;
				return true;
			}
		}
		// weapon picker
#ifdef LINUX
		if(ev->type == ev_mouse)
		{
			int tmx = absmousex - SCREENWIDTH / 2;
			int tmy = absmousey - SCREENHEIGHT / 2;

			if(abs(tmx) > 100 || abs(tmy) > 100)
			{
				an = R_PointToAngle2(0, 0, tmx * FRACUNIT, tmy * FRACUNIT) + (ANG45 / 2);
#else
		if(ev->type == ev_joystick)
		{
			int tmx = ev->data2;
			int tmy = ev->data3;

			if(abs(tmx) > 100 || abs(tmy) > 100)
			{
				an = R_PointToAngle2(0, 0, tmx * 8, tmy * -8) + (ANG45 / 2);
#endif
				if(an < ANG45)
				{
					if(gamemode != commercial)
						weapon_select = wp_shotgun;
					else
					if(weapon_select == wp_nochange)
					{
						if(weapon_shotgun == wp_shotgun)
							weapon_shotgun = wp_supershotgun;
						else
							weapon_shotgun = wp_shotgun;
					}
					weapon_select = weapon_shotgun;
				} else
				if(an < ANG45*2)
				{
					weapon_select = wp_pistol;
				} else
				if(an < ANG45*3)
				{
					weapon_select = wp_fist;
				} else
				if(an < (angle_t)ANG45*4)
				{
					weapon_select = wp_chainsaw;

				} else
				if(an < (angle_t)ANG45*5)
				{
					weapon_select = wp_bfg;
				} else
				if(an < (angle_t)ANG45*6)
				{
					weapon_select = wp_plasma;
				} else
				if(an < (angle_t)ANG45*7)
				{
					weapon_select = wp_missile;
				} else
				{
					weapon_select = wp_chaingun;
				}
			} else
				weapon_select = wp_nochange;
		}
	} else
	if(!(plr->cheats & CF_SPECTATOR))
	{
//#ifdef LINUX
#if 0
		if(ev->type == ev_keydown && ev->data1 == '5')
		{
#else
		if(ev->type == ev_joystick && ev->data1 & (1 << i_ctrl_btn[joybweapons]))
		{
#endif
			GrabMouse(0);
			in_weapon_menu = true;
			weapon_change = wp_nochange;
			weapon_select = wp_nochange;
			if(weapon_shotgun == wp_shotgun && !plr->weaponowned[wp_shotgun])
				weapon_shotgun = wp_supershotgun;
			if(weapon_shotgun == wp_supershotgun && !plr->weaponowned[wp_supershotgun])
				weapon_shotgun = wp_shotgun;
			// cancel all game keys
			memset(gamekeydown, 0, sizeof(gamekeydown));
			return true;
		}
	}

  // cheats, to be moved [kg]
  if (ev->type == ev_keydown)
  {
    if (!netgame)
    {
      // b. - enabled for more debug fun.
      // if (gameskill != sk_nightmare) {

	// [kg] inf. ammo
	if(cht_CheckCheat(&cheat_infammo, ev->data1))
	{
		plyr->cheats ^= CF_INFAMMO;
		if(plyr->cheats & CF_INFAMMO)
			plyr->message = "Infinite ammo ON";
		else
			plyr->message = "Infinite ammo OFF";
	} else
	// [kg] inf. ammo
	if(cht_CheckCheat(&cheat_infhealth, ev->data1))
	{
		plyr->cheats ^= CF_INFHEALTH;
		if(plyr->cheats & CF_INFHEALTH)
			plyr->message = "Infinite health ON";
		else
			plyr->message = "Infinite health OFF";
	} else
	// [kg] kill aura
	if(cht_CheckCheat(&cheat_killaura, ev->data1))
	{
		int old = plyr->cheats & CF_AURAMASK;
		plyr->cheats &= ~CF_AURAMASK;
		if(old == CF_DEATHAURA)
			plyr->message = "Death aura OFF";
		else 
		{
			plyr->cheats |= CF_DEATHAURA;
			plyr->message = "Death aura ON";
		}
	} else
	// [kg] safe aura
	if(cht_CheckCheat(&cheat_safeaura, ev->data1))
	{
		int old = plyr->cheats & CF_AURAMASK;
		plyr->cheats &= ~CF_AURAMASK;
		if(old == CF_SAFEAURA)
			plyr->message = "Safe aura OFF";
		else 
		{
			plyr->cheats |= CF_SAFEAURA;
			plyr->message = "Safe aura ON";
		}
	} else
	// [kg] revenge aura
	if(cht_CheckCheat(&cheat_revengeaura, ev->data1))
	{
		int old = plyr->cheats & CF_AURAMASK;
		plyr->cheats &= ~CF_AURAMASK;
		if(old == CF_REVENGEAURA)
			plyr->message = "Revenge aura OFF";
		else 
		{
			plyr->cheats |= CF_REVENGEAURA;
			plyr->message = "Revenge aura ON";
		}
	} else
	// [kg] kill all visible monsters
	if(cht_CheckCheat(&cheat_viskill, ev->data1))
	{
		mobj_t *mo;
		thinker_t *think;

		for(think = thinkercap.next; think != &thinkercap; think = think->next)
		{
			if(think->function.acv != P_MobjThinker)
			// Not a mobj thinker
				continue;
			mo = (mobj_t *)think;
			if(!mo->player && mo->flags & MF_SHOOTABLE && mo->health > 0 && P_CheckSight(plyr->mo, mo))
			{
				P_DamageMobj(mo, plyr->mo, plyr->mo, INSTANTKILL);
			}
		}
	} else
	// [kg] slow motion
	if(cht_CheckCheat(&cheat_slowmo, ev->data1))
	{
		plyr->cheats ^= CF_SLOWMO;
		if(plyr->cheats & CF_SLOWMO)
			plyr->message = "Slow motion";
		else 
			plyr->message = "Normal speed";
	} else
	// [kg] free/auto aim
	if(cht_CheckCheat(&cheat_aim, ev->data1))
	{
		sv_freeaim ^= 1;
		if(sv_freeaim)
			plyr->message = "Free aim";
		else 
			plyr->message = "Auto aim";
	} else
      // 'dqd' cheat for toggleable god mode
      if (cht_CheckCheat(&cheat_god, ev->data1))
      {
	plyr->cheats ^= CF_GODMODE;
	if (plyr->cheats & CF_GODMODE)
	{
	  if (plyr->mo)
	    plyr->mo->health = 100;
	  
	  plyr->health = 100;
	  plyr->message = STSTR_DQDON;
	}
	else 
	  plyr->message = STSTR_DQDOFF;
      }
      // 'fa' cheat for killer fucking arsenal
      else if (cht_CheckCheat(&cheat_ammonokey, ev->data1))
      {
	plyr->armorpoints = 200;
	plyr->armortype = 2;
	
	for (i=0;i<NUMWEAPONS;i++)
	  plyr->weaponowned[i] = true;
	
	for (i=0;i<NUMAMMO;i++)
	  plyr->ammo[i] = plyr->maxammo[i];
	
	plyr->message = STSTR_FAADDED;
      }
      // 'kfa' cheat for key full ammo
      else if (cht_CheckCheat(&cheat_ammo, ev->data1))
      {
	plyr->armorpoints = 200;
	plyr->armortype = 2;
	
	for (i=0;i<NUMWEAPONS;i++)
	  plyr->weaponowned[i] = true;
	
	for (i=0;i<NUMAMMO;i++)
	  plyr->ammo[i] = plyr->maxammo[i];
	
	for (i=0;i<NUMCARDS;i++)
	  plyr->cards[i] = true;
	
	plyr->message = STSTR_KFAADDED;
      }
      // 'mus' cheat for changing music
      else if (cht_CheckCheat(&cheat_mus, ev->data1))
      {
	
	char	buf[3];
	int		musnum;
	
	plyr->message = STSTR_MUS;
	cht_GetParam(&cheat_mus, buf);
	
	if (gamemode == commercial)
	{
	  musnum = mus_runnin + (buf[0]-'0')*10 + buf[1]-'0' - 1;
	  
	  if (((buf[0]-'0')*10 + buf[1]-'0') > 35)
	    plyr->message = STSTR_NOMUS;
	  else
	    S_ChangeMusic(musnum, 1);
	}
	else
	{
	  musnum = mus_e1m1 + (buf[0]-'1')*9 + (buf[1]-'1');
	  
	  if (((buf[0]-'1')*9 + buf[1]-'1') > 31)
	    plyr->message = STSTR_NOMUS;
	  else
	    S_ChangeMusic(musnum, 1);
	}
      }
      // Simplified, accepting both "noclip" and "idspispopd".
      // no clipping mode cheat
      else if ( cht_CheckCheat(&cheat_noclip, ev->data1) 
		|| cht_CheckCheat(&cheat_commercial_noclip,ev->data1) )
      {	
	plyr->cheats ^= CF_NOCLIP;
	
	if (plyr->cheats & CF_NOCLIP)
	  plyr->message = STSTR_NCON;
	else
	  plyr->message = STSTR_NCOFF;
      }
      // 'behold?' power-up cheats
      for (i=0;i<6;i++)
      {
	if (cht_CheckCheat(&cheat_powerup[i], ev->data1))
	{
	  if (!plyr->powers[i])
	    P_GivePower( plyr, i);
	  else if (i!=pw_strength)
	    plyr->powers[i] = 1;
	  else
	    plyr->powers[i] = 0;
	  
	  plyr->message = STSTR_BEHOLDX;
	}
      }
      
      // 'behold' power-up menu
      if (cht_CheckCheat(&cheat_powerup[6], ev->data1))
      {
	plyr->message = STSTR_BEHOLD;
      }
      // 'choppers' invulnerability & chainsaw
      else if (cht_CheckCheat(&cheat_choppers, ev->data1))
      {
	plyr->weaponowned[wp_chainsaw] = true;
	plyr->powers[pw_invulnerability] = true;
	plyr->message = STSTR_CHOPPERS;
      }
/*      // 'mypos' for player position
      else if (cht_CheckCheat(&cheat_mypos, ev->data1))
      {
	static char	buf[ST_MSGWIDTH];
	sprintf(buf, "ang=0x%x;x,y=(0x%x,0x%x)",
		players[consoleplayer].mo->angle,
		players[consoleplayer].mo->x,
		players[consoleplayer].mo->y);
	plyr->message = buf;
      }*/
    }
    
    // 'clev' change-level cheat
    if (cht_CheckCheat(&cheat_clev, ev->data1))
    {
      char		buf[3];
      int		epsd;
      int		map;
      
      cht_GetParam(&cheat_clev, buf);
      
      if (gamemode == commercial)
      {
	epsd = 0;
	map = (buf[0] - '0')*10 + buf[1] - '0';
      }
      else
      {
	epsd = buf[0] - '0';
	map = buf[1] - '0';
      }

      // Catch invalid maps.
      if (epsd < 1)
	return false;

      if (map < 1)
	return false;
      
      // Ohmygod - this is not going to work.
      if ((gamemode == retail)
	  && ((epsd > 4) || (map > 9)))
	return false;

      if ((gamemode == registered)
	  && ((epsd > 3) || (map > 9)))
	return false;

      if ((gamemode == shareware)
	  && ((epsd > 1) || (map > 9)))
	return false;

      if ((gamemode == commercial)
	&& (( epsd > 1) || (map > 34)))
	return false;

      // So be it.
      plyr->message = STSTR_CLEV;
      G_DeferedInitNew(gameskill, epsd, map);
    }    
  }
  return false;
}

void ST_Ticker (void)
{
}

static int st_palette = -1;

void ST_doPaletteStuff(void)
{

    int		palette;
    byte*	pal;
    int		cnt = 0;
    int		bzc;

    cnt = plyr->damagecount;

    if (plyr->powers[pw_strength])
    {
	// slowly fade the berzerk out
  	bzc = 12 - (plyr->powers[pw_strength]>>6);

	if (bzc > cnt)
	    cnt = bzc;
    }
	
    if (cnt)
    {
	palette = (cnt+7)>>3;
	
	if (palette >= NUMREDPALS)
	    palette = NUMREDPALS-1;

	palette += STARTREDPALS;
    }

    else if (plyr->bonuscount)
    {
	palette = (plyr->bonuscount+7)>>3;

	if (palette >= NUMBONUSPALS)
	    palette = NUMBONUSPALS-1;

	palette += STARTBONUSPALS;
    }

    else if ( plyr->powers[pw_ironfeet] > 4*32
	      || plyr->powers[pw_ironfeet]&8)
	palette = RADIATIONPAL;
    else
	palette = 0;

    if (palette != st_palette)
    {
	st_palette = palette;
	pal = (byte *) W_CacheLumpNum (lu_palette)+palette*768;
	I_SetPalette (pal);
    }

}

void ST_Drawer (boolean fullscreen, boolean refresh)
{
	int x, temp;
	byte *cmap;
	byte *imap = v_colormap_normal + 256*32;

	// [kg] only new fullscreen status bar, sorry
	plyr = &players[displayplayer];

	// Do red-/gold-shifts from damage/items
	ST_doPaletteStuff();

	//
	// new status bar

	if(plyr->cheats & CF_SPECTATOR)
		return;

	if(plyr->playerstate == PST_DEAD)
	{
		if(in_weapon_menu)
			GrabMouse(1);
		in_weapon_menu = false;
		weapon_change = wp_nochange;
		if(!deathtime)
			// you are dead
			return;
		deathtime--;
	} else
		deathtime = TICRATE / 2;

	// crosshair
	if(sv_freeaim)
		V_DrawPatchRemap1((SCREENWIDTH / 2) - 4, SCREENHEIGHT / 2, (patch_t*)pointer, imap);

	// health
	V_DrawPatchNew(STBAR_HEALTH_X, STBAR_HEALTH_Y, hp_back, v_colormap_normal, V_HALLIGN_RIGHT, V_VALLIGN_NONE, 2);
	if(plyr->cheats & CF_GODMODE)
		cmap = imap;
	else
	if(plyr->cheats & CF_INFHEALTH)
		cmap = bloodlationtables + 256;
	else
		cmap = v_colormap_normal;
	STlib_drawNum(STBAR_HEALTH_X, STBAR_HEALTH_Y + 8, plyr->health < 0 ? 0 : plyr->health, cmap);

	// armor
	if(plyr->armorpoints)
	{
		if(plyr->armortype)
			V_DrawPatchNew(STBAR_ARMOR_X, STBAR_ARMOR_Y, armor_back[plyr->armortype-1], v_colormap_normal, V_HALLIGN_RIGHT, V_VALLIGN_NONE, 2);
		STlib_drawNum(STBAR_ARMOR_X, STBAR_ARMOR_Y + 8, plyr->armorpoints, v_colormap_normal);
	}

	// ammo
	temp = am_noammo;
	switch(plyr->readyweapon)
	{
		case wp_pistol:
		case wp_chaingun:
			temp = am_clip;
		break;
		case wp_shotgun:
		case wp_supershotgun:
			temp = am_shell;
		break;
		case wp_missile:
			temp = am_misl;
		break;
		case wp_plasma:
		case wp_bfg:
			temp = am_cell;
		break;
	}
	if(temp != am_noammo)
	{
		if(plyr->backpack)
			V_DrawPatchNew(STBAR_AMMO_X - 4, STBAR_AMMO_Y + 12, pack_back, v_colormap_normal, V_HALLIGN_RIGHT, V_VALLIGN_CENTER, 2);
		V_DrawPatchNew(STBAR_AMMO_X, STBAR_AMMO_Y, ammo_back[temp], v_colormap_normal, V_HALLIGN_RIGHT, V_VALLIGN_NONE, 2);
		STlib_drawNum(STBAR_AMMO_X, STBAR_AMMO_Y + 8, plyr->ammo[temp], plyr->cheats & CF_INFAMMO ? bloodlationtables + 256 : v_colormap_normal);
	}

	// keys
	x = STBAR_KEY_X;
	for(temp = 0; temp < NUMCARDS; temp++)
	{
		if(plyr->cards[temp])
		{
			V_DrawPatchNew(x, STBAR_KEY_Y, keys[temp], v_colormap_normal, V_HALLIGN_RIGHT, V_VALLIGN_TOP, 3);
			x -= SHORT(keys[temp]->width) * 3 + 3;
		}
	}

	//
	// weapon selection menu

	if(menuactive)
		in_weapon_menu = false;

	if(in_weapon_menu)
	{
		cmap = v_colormap_normal + 256 * 24;
		V_FadeScreen(colormaps, 16);
		for(x = 0; x < 8; x++)
		{
			int scale = 2;

			if(gamemode == commercial && x == wp_shotgun)
			{
				// shotguns handling
				if(weapon_select == wp_supershotgun)
				{
					V_DrawPatchNew(STBAR_WEAP_X + weap_offs_x[x], STBAR_WEAP_Y + weap_offs_y[x], weap_back[wp_supershotgun], plyr->weaponowned[wp_supershotgun] ? v_colormap_normal : cmap, V_HALLIGN_CENTER, V_VALLIGN_CENTER, 3);
					V_DrawPatchNew(STBAR_WEAP_X + weap_offs_x[x] + 150, STBAR_WEAP_Y + weap_offs_y[x], weap_back[wp_shotgun], plyr->weaponowned[wp_shotgun] ? v_colormap_normal : cmap, V_HALLIGN_CENTER, V_VALLIGN_CENTER, 2);
				} else
				if(weapon_select == wp_shotgun)
				{
					V_DrawPatchNew(STBAR_WEAP_X + weap_offs_x[x], STBAR_WEAP_Y + weap_offs_y[x], weap_back[wp_shotgun], plyr->weaponowned[wp_shotgun] ? v_colormap_normal : cmap, V_HALLIGN_CENTER, V_VALLIGN_CENTER, 3);
					V_DrawPatchNew(STBAR_WEAP_X + weap_offs_x[x] + 150, STBAR_WEAP_Y + weap_offs_y[x], weap_back[wp_supershotgun], plyr->weaponowned[wp_supershotgun] ? v_colormap_normal : cmap, V_HALLIGN_CENTER, V_VALLIGN_CENTER, 2);
				} else
				if(weapon_shotgun == wp_shotgun)
				{
					V_DrawPatchNew(STBAR_WEAP_X + weap_offs_x[x], STBAR_WEAP_Y + weap_offs_y[x], weap_back[wp_shotgun], plyr->weaponowned[wp_shotgun] ? v_colormap_normal : cmap, V_HALLIGN_CENTER, V_VALLIGN_CENTER, 2);
					V_DrawPatchNew(STBAR_WEAP_X + weap_offs_x[x] + 150, STBAR_WEAP_Y + weap_offs_y[x], weap_back[wp_supershotgun], plyr->weaponowned[wp_supershotgun] ? v_colormap_normal : cmap, V_HALLIGN_CENTER, V_VALLIGN_CENTER, 2);
				} else
				{
					V_DrawPatchNew(STBAR_WEAP_X + weap_offs_x[x], STBAR_WEAP_Y + weap_offs_y[x], weap_back[wp_supershotgun], plyr->weaponowned[wp_supershotgun] ? v_colormap_normal : cmap, V_HALLIGN_CENTER, V_VALLIGN_CENTER, 2);
					V_DrawPatchNew(STBAR_WEAP_X + weap_offs_x[x] + 150, STBAR_WEAP_Y + weap_offs_y[x], weap_back[wp_shotgun], plyr->weaponowned[wp_shotgun] ? v_colormap_normal : cmap, V_HALLIGN_CENTER, V_VALLIGN_CENTER, 2);
				}
				continue;
			}

			if(weapon_select == x)
				scale++;

			if(weap_back[x])
				V_DrawPatchNew(STBAR_WEAP_X + weap_offs_x[x], STBAR_WEAP_Y + weap_offs_y[x], weap_back[x], plyr->weaponowned[x] ? v_colormap_normal : cmap, V_HALLIGN_CENTER, V_VALLIGN_CENTER, x > 1 ? scale : scale-1);
			else
				V_DrawPatchNew(STBAR_WEAP_X + weap_offs_x[x], STBAR_WEAP_Y + weap_offs_y[x], empty_back, v_colormap_normal, V_HALLIGN_CENTER, V_VALLIGN_CENTER, 3);
		}
	}
}

void ST_SetNewWeapon(weapontype_t wpn)
{
	weapon_change = wpn;
}

weapontype_t ST_GetNewWeapon()
{
	weapontype_t ret = weapon_change;
	weapon_change = wp_nochange;
	// shotguns
	if(ret == wp_shotgun || ret == wp_supershotgun)
		weapon_shotgun = ret;
	return ret;
}

void ST_loadGraphics(void)
{
	int i;
	char namebuf[9];

	STlib_init();

	// key cards
	for(i = 0; i < NUMCARDS; i++)
	{
		sprintf(namebuf, "STKEYS%d", i);
		keys[i] = (patch_t *) W_CacheLumpName(namebuf);
	}

	// health
	hp_back = (patch_t *) W_CacheLumpName("MEDIA0");

	// armor
	armor_back[0] = (patch_t *) W_CacheLumpName("ARM1A0");
	armor_back[1] = (patch_t *) W_CacheLumpName("ARM2A0");

	// ammo
	for(i = 0; i < NUMAMMO; i++)
	{
		// some lumps are not present in shareware
		int lump = W_CheckNumForName(ammo_icon[i]);
		if(lump >= 0)
			ammo_back[i] = W_CacheLumpNum(lump);
	}
	pack_back = (patch_t *) W_CacheLumpName("BPAKA0");

	// weapons
	for(i = 0; i < NUMWEAPONS; i++)
	{
		// some lumps are not present in shareware
		int lump = W_CheckNumForName(weap_icon[i]);
		if(lump >= 0)
			weap_back[i] = W_CacheLumpNum(lump);
	}
	empty_back = W_CacheLumpName("STPB3");
}

void ST_Stop (void)
{
	I_SetPalette (W_CacheLumpNum (lu_palette));
}

void ST_Init (void)
{
	weapon_change = wp_nochange;
	lu_palette = W_GetNumForName ("PLAYPAL");
	ST_loadGraphics();
}

