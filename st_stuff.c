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

#include "p_inventory.h"

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

// [kg] weapon menu
typedef struct weaponlist_s
{
	struct weaponlist_s *next;
	patch_t *patch;
	int type;
	boolean owned;
} weaponlist_t;

// crosshair
static patch_t*		pointer;

// main player in game
static player_t*	plyr; 

// lump number for PLAYPAL
static int		lu_palette;

// backgrounds
static patch_t*		hp_back;
static patch_t*		pack_back;
//static patch_t*		ammo_back[NUMAMMO];

// death timeout
static int deathtime;

// tables
//static char *const ammo_icon[NUMAMMO] = {"AMMOA0", "SBOXA0", "CELPA0", "BROKA0"};
static char *const weap_icon[MAXWEAPONS] = {"PUNGC0", "PISGC0", "SHOTA0", "MGUNA0", "LAUNA0", "PLASA0", "BFUGA0", "CSAWA0", "SGN2A0"};
static const int weap_offs_x[8] = {0  , 200, 300, 200 , 0   , -200, -300, -200};
static const int weap_offs_y[8] = {200, 150, 0  , -150, -200, -150, 0   , 150};

// [kg] weapon menu
static weaponlist_t *weapon_list;
static weaponlist_t *weapon_last;
static angle_t weapon_astep;
boolean in_weapon_menu;
static weapontype_t weapon_change;
static weapontype_t weapon_select;

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
				if(weapon_select != wp_nochange)
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

			if(abs(tmx) > 120 || abs(tmy) > 120)
			{
				an = R_PointToAngle2(0, 0, tmx * FRACUNIT, tmy * FRACUNIT);
#else
		if(ev->type == ev_joystick)
		{
			int tmx = ev->data2;
			int tmy = ev->data3;

			if(abs(tmx) > 10000 || abs(tmy) > 10000)
			{
				an = R_PointToAngle2(0, 0, tmx * 8, tmy * -8);
#endif
				an = (weapon_astep/2) - an + ANG90;

				angle_t angle = 0;
				weaponlist_t *list = weapon_list;

				weapon_select = wp_nochange;
				while(list)
				{
					if(list->patch)
					{
						if(an >= angle && an < angle + weapon_astep && list->owned)
						{
							weapon_select = list->type;
							break;
						}
						angle += weapon_astep;
					}
					list = list->next;
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
			int weapon_count = 0;
			weaponlist_t *list = weapon_list;

			// prepare menu
			GrabMouse(0);
			in_weapon_menu = true;
			weapon_change = wp_nochange;
			weapon_select = wp_nochange;

			// cancel all game keys
			memset(gamekeydown, 0, sizeof(gamekeydown));

			// scan for current amount
			while(list)
			{
//				if(list->owned)	// TODO: menu option
				if(list->patch)
					weapon_count++;
				list = list->next;
			}

			if(weapon_count > 0)
				weapon_astep = (ANG180 / weapon_count)*2;
			else
				weapon_astep = 0;

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
				P_DamageMobj(mo, plyr->mo, plyr->mo, INSTANTKILL, NUMDAMAGETYPES);
			}
		}
	} else
	// [kg] slow motion
	if(cht_CheckCheat(&cheat_slowmo, ev->data1))
	{
		sv_slowmo = !sv_slowmo;
		if(sv_slowmo)
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
	  plyr->message = STSTR_DQDON;
	}
	else 
	  plyr->message = STSTR_DQDOFF;
      }
      // 'fa' cheat for killer fucking arsenal
      else if (cht_CheckCheat(&cheat_ammonokey, ev->data1))
      {
	plyr->mo->armorpoints = 200;
//	plyr->mo->armortype = 2;
	
//	plyr->weaponowned = -1;
	
//	for (i=0;i<NUMAMMO;i++)
//	  plyr->ammo[i] = plyr->maxammo[i];

	weaponlist_t *list = weapon_list;
	while(list)
	{
		if(list->patch)
			P_GiveInventory(plyr->mo, &mobjinfo[list->type], 1);
		list = list->next;
	}
	
	plyr->message = STSTR_FAADDED;
      }
      // 'mus' cheat for changing music
      else if (cht_CheckCheat(&cheat_mus, ev->data1))
      {
/*	
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
*/      }
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

    int		palette = 0;
    byte*	pal;
    int		cnt = 0;
    int		bzc;

    cnt = plyr->damagecount;

/*    if (plyr->powers[pw_strength])
    {
	// slowly fade the berzerk out
  	bzc = 12 - (plyr->powers[pw_strength]>>6);

	if (bzc > cnt)
	    cnt = bzc;
    }
*/	
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
/*
    else if ( plyr->powers[pw_ironfeet] > 4*32
	      || plyr->powers[pw_ironfeet]&8)
	palette = RADIATIONPAL;
    else
	palette = 0;
*/
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

	if(!plyr->mo)
		return;

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
	if(sv_freeaim && pointer)
		V_DrawPatchRemap1((SCREENWIDTH / 2) - 4, SCREENHEIGHT / 2, pointer, imap);

	// health
	V_DrawPatchNew(STBAR_HEALTH_X, STBAR_HEALTH_Y, hp_back, v_colormap_normal, V_HALLIGN_RIGHT, V_VALLIGN_NONE, 2);
	if(plyr->cheats & CF_GODMODE)
		cmap = imap;
	else
	cmap = v_colormap_normal;
	STlib_drawNum(STBAR_HEALTH_X, STBAR_HEALTH_Y + 8, plyr->mo->health < 0 ? 0 : plyr->mo->health, cmap);

	// armor
	if(plyr->mo->armorpoints)
	{
		if(plyr->mo->armortype)
		{
			int lump;

			lump = R_GetStateLump(plyr->mo->armortype->spawnstate);
			if(lump)
				V_DrawPatchNew(STBAR_ARMOR_X, STBAR_ARMOR_Y, (patch_t*)W_CacheLumpNum(lump), v_colormap_normal, V_HALLIGN_RIGHT, V_VALLIGN_NONE, 2);
		}
		STlib_drawNum(STBAR_ARMOR_X, STBAR_ARMOR_Y + 8, plyr->mo->armorpoints, v_colormap_normal);
	}

	// ammo
/*	temp = am_noammo;
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
*/
	// keys
/*	x = STBAR_KEY_X;
	for(temp = 0; temp < NUMCARDS; temp++)
	{
		if(plyr->cards[temp])
		{
			V_DrawPatchNew(x, STBAR_KEY_Y, keys[temp], v_colormap_normal, V_HALLIGN_RIGHT, V_VALLIGN_TOP, 3);
			x -= SHORT(keys[temp]->width) * 3 + 3;
		}
	}
*/
	//
	// weapon selection menu

	if(menuactive)
		in_weapon_menu = false;

	if(in_weapon_menu)
	{
		weapontype_t wcur;
		weaponlist_t *list = weapon_list;
		angle_t angle = 0;

		if(weapon_select != wp_nochange)
			wcur = weapon_select;
		else
		{
			wcur = plyr->pendingweapon;
			if(wcur == wp_nochange)
				wcur = plyr->readyweapon;
		}

		cmap = v_colormap_normal + 256 * 14;
		V_FadeScreen(colormaps, 16);

		while(list)
		{
			if(list->patch)
			{
				fixed_t x, y;
				angle_t ang;

				ang = angle >> ANGLETOFINESHIFT;
				x = finesine[ang] / 192;
				y = finecosine[ang] / 256;

				V_DrawPatchNew(SCREENWIDTH/2 + x, SCREENHEIGHT/2 + y, list->patch, list->owned ? v_colormap_normal : cmap, V_HALLIGN_CENTER, V_VALLIGN_CENTER, list->type == wcur ? 3 : 2);
				angle += weapon_astep;
			}
			list = list->next;
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
	return ret;
}

void ST_loadGraphics(void)
{
	int i;
	char namebuf[9];

	STlib_init();

	// pointer
	i = W_CheckNumForName("POINTER");
	if(i >= 0)
		pointer = (patch_t *)W_CacheLumpNum(i);

	// key cards
/*	for(i = 0; i < NUMCARDS; i++)
	{
		sprintf(namebuf, "STKEYS%d", i);
		keys[i] = (patch_t *) W_CacheLumpName(namebuf);
	}
*/
	// health
	hp_back = (patch_t *) W_CacheLumpName("MEDIA0");

	// ammo
/*	for(i = 0; i < NUMAMMO; i++)
	{
		// some lumps are not present in shareware
		int lump = W_CheckNumForName(ammo_icon[i]);
		if(lump >= 0)
			ammo_back[i] = W_CacheLumpNum(lump);
	}
*/	pack_back = (patch_t *) W_CacheLumpName("BPAKA0");
}

void ST_AddWeaponType(int type, char *patch)
{
	int pnum;
	weaponlist_t *list;

	list = malloc(sizeof(weaponlist_t));
	if(!list)
		I_Error("ST_AddWeaponType: memory allocation error");

	pnum = W_CheckNumForName(patch);
	list->next = NULL;
	if(pnum >= 0)
		list->patch = W_CacheLumpNum(pnum);
	else
		list->patch = NULL;
	list->type = type;
	list->owned = false;

	if(weapon_last)
		weapon_last->next = list;
	else
		weapon_list = list;

	weapon_last = list;
}

void ST_CheckWeaponInventory(int type, int count)
{
	weaponlist_t *list = weapon_list;

	while(list)
	{
		if(list->type == type)
			list->owned = !!count;
		list = list->next;
	}
}

void ST_ClearWeapons()
{
	weaponlist_t *list = weapon_list;

	while(list)
	{
		list->owned = 0;
		list = list->next;
	}
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

