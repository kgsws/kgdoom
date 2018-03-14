#include "doomdef.h"
#include "dstrings.h"

#include "d_main.h"

#include "i_system.h"
#include "i_video.h"
#include "z_zone.h"
#include "v_video.h"
#include "w_wad.h"

#include "r_local.h"


#include "hu_stuff.h"

#include "g_game.h"

#include "m_argv.h"
#include "m_swap.h"

#include "s_sound.h"

#include "doomstat.h"

// Data.
#include "m_menu.h"



extern patch_t*		hu_font[HU_FONTSIZE];
extern boolean		message_dontfuckwithme;

extern boolean		chat_on;		// in heads-up code

//
// defaulted values
//
int			mouseSensitivity;       // has default

// Show messages has default, 0 = off, 1 = on
int			showMessages;
	

// Blocky mode, has default, 0 = high, 1 = normal
int			detailLevel;		
int			screenblocks;		// has default

// temp for screenblocks (0-9)
int			screenSize;		

// -1 = no quicksave slot picked!
int			quickSaveSlot;          

 // 1 = message to be printed
int			messageToPrint;
// ...and here is the message string!
char*			messageString;
// [kg] message cancel sound
int			messageSound;

// message x & y
int			messx;			
int			messy;
int			messageLastMenuActive;

// timed message = no input from user
boolean			messageNeedsInput;     

void    (*messageRoutine)(int response);

#define SAVESTRINGSIZE 	24

char gammamsg[5][26] =
{
    GAMMALVL0,
    GAMMALVL1,
    GAMMALVL2,
    GAMMALVL3,
    GAMMALVL4
};

// we are going to be entering a savegame string
int			saveStringEnter;              
int             	saveSlot;	// which slot to save in
int			saveCharIndex;	// which char we're editing
// old save description before edit
char			saveOldString[SAVESTRINGSIZE];  

boolean			inhelpscreens;
boolean			menuactive;

#define SKULLXOFF		-32
#define LINEHEIGHT		16

extern boolean		sendpause;
char			savegamestrings[10][SAVESTRINGSIZE];

char	endstring[160];


//
// MENU TYPEDEFS
//
typedef struct
{
    char	name[32];
    // choice = menu item #.
    // if status = 2,
    //   choice=0:leftarrow,1:rightarrow
    void	(*routine)(int choice);
    // 0 = no cursor here, 1 = ok, 2 = arrows ok
    int8_t	status;
} menuitem_t;

typedef struct menu_s
{
    short		numitems;	// # of menu items
    short		small;		// [kg] special menu with small font
    struct menu_s*	prevMenu;	// previous menu
    menuitem_t*		menuitems;	// menu items
    void		(*routine)();	// draw routine
    short		x;
    short		y;		// x,y of menu
    short		lastOn;		// last item user was on in menu
} menu_t;

short		itemOn;			// menu item skull is on
short		skullAnimCounter;	// skull animation counter
short		whichSkull;		// which skull to draw

// graphic name of skulls
// warning: initializer-string for array of chars is too long
char    skullName[2][/*8*/9] = {"M_SKULL1","M_SKULL2"};

// current menudef
menu_t*	currentMenu;                          

//
// PROTOTYPES
//
void M_NewGame(int choice);
void M_Episode(int choice);
void M_ChooseSkill(int choice);
void M_LoadGame(int choice);
void M_SaveGame(int choice);
void M_Options(int choice);
void M_EndGame(int choice);
void M_ReadThis(int choice);
void M_ReadThis2(int choice);
void M_QuitDOOM(int choice);

void M_ChangeMessages(int choice);
void M_ChangeSensitivity(int choice);
void M_SfxVol(int choice);
void M_MusicVol(int choice);
void M_ChangeDetail(int choice);
void M_SizeDisplay(int choice);
void M_StartGame(int choice);
void M_Sound(int choice);

void M_FinishReadThis(int choice);
void M_LoadSelect(int choice);
void M_SaveSelect(int choice);
void M_ReadSaveStrings(void);
void M_QuickSave(void);
void M_QuickLoad(void);

void M_DrawMainMenu(void);
void M_DrawReadThis1(void);
void M_DrawReadThis2(void);
void M_DrawNewGame(void);
void M_DrawEpisode(void);
void M_DrawOptions(void);
void M_DrawSound(void);
void M_DrawLoad(void);
void M_DrawSave(void);

void M_DrawSaveLoadBorder(int x,int y);
void M_SetupNextMenu(menu_t *menudef);
void M_DrawThermo(int x,int y,int thermWidth,int thermDot);
void M_DrawEmptyCell(menu_t *menu,int item);
void M_DrawSelCell(menu_t *menu,int item);
void M_StartControlPanel(void);
void M_StartMessage(char *string,void *routine,boolean input);
void M_StopMessage(void);
void M_ClearMenus (void);

// [kg] new options
void M_ToggleStick();
void M_ChangeKey(int choice);


//
// DOOM MENU
//
enum
{
    newgame = 0,
    options,
//    loadgame,
//    savegame,
    readthis,
    quitdoom,
    main_end
} main_e;

menuitem_t MainMenu[]=
{
    {"M_NGAME",M_NewGame, 1},
    {"M_OPTION",M_Options, 1},
//    {"M_LOADG",M_LoadGame, 1},
//    {"M_SAVEG",M_SaveGame, 1},
    // Another hickup with Special edition.
    {"M_RDTHIS",M_ReadThis, 1},
    {"M_QUITG",M_QuitDOOM, 1}
};

menu_t  MainDef =
{
    main_end,
    0,
    NULL,
    MainMenu,
    M_DrawMainMenu,
    97,64,
    0
};


//
// EPISODE SELECT
//
enum
{
    ep1,
    ep2,
    ep3,
    ep4,
    ep_end
} episodes_e;

menuitem_t EpisodeMenu[]=
{
    {"M_EPI1", M_Episode, 1},
    {"M_EPI2", M_Episode, 1},
    {"M_EPI3", M_Episode, 1},
    {"M_EPI4", M_Episode, 1}
};

menu_t  EpiDef =
{
    ep_end,		// # of menu items
    0,
    &MainDef,		// previous menu
    EpisodeMenu,	// menuitem_t ->
    M_DrawEpisode,	// drawing routine ->
    48,63,              // x,y
    ep1			// lastOn
};

//
// NEW GAME
//
enum
{
    killthings,
    toorough,
    hurtme,
    violence,
    nightmare,
    newg_end
} newgame_e;

menuitem_t NewGameMenu[]=
{
    {"M_JKILL",	M_ChooseSkill, 1},
    {"M_ROUGH",	M_ChooseSkill, 1},
    {"M_HURT",	M_ChooseSkill, 1},
    {"M_ULTRA",	M_ChooseSkill, 1},
    {"M_NMARE",	M_ChooseSkill, 1}
};

menu_t  NewDef =
{
    newg_end,		// # of menu items
    0,
    &EpiDef,		// previous menu
    NewGameMenu,	// menuitem_t ->
    M_DrawNewGame,	// drawing routine ->
    48,63,              // x,y
    hurtme		// lastOn
};



//
// OPTIONS MENU
// [kg] complete rewrite
//
enum
{
	opt_i0,
	opt_i1,
	opt_i2,
	opt_i3,
	opt_i4,
	opt_i5,
	opt_end
} options_e;

menuitem_t OptionsMenu[]=
{
    {"STICK ROLES", M_ToggleStick, 1},
    {"SHOOT", M_ChangeKey, 1},
    {"SHOOT SECONDARY", M_ChangeKey, 1},
    {"USE", M_ChangeKey, 1},
    {"WEAPONS", M_ChangeKey, 1},
    {"WALK", M_ChangeKey, 1},
    {"TOGGLE MAP", M_ChangeKey, 1},
};

menu_t  OptionsDef =
{
    opt_end,
    1,
    &MainDef,
    OptionsMenu,
    M_DrawOptions,
    60,80,
    0
};

//
// Read This! MENU 1 & 2
//
enum
{
    rdthsempty1,
    read1_end
} read_e;

menuitem_t ReadMenu1[] =
{
    {"",M_ReadThis2,1}
};

menu_t  ReadDef1 =
{
    read1_end,
    1,
    &MainDef,
    ReadMenu1,
    M_DrawReadThis1,
    280,185,
    0
};

enum
{
    rdthsempty2,
    read2_end
} read_e2;

menuitem_t ReadMenu2[]=
{
    {"",M_FinishReadThis,1}
};

menu_t  ReadDef2 =
{
    read2_end,
    1,
    &ReadDef1,
    ReadMenu2,
    M_DrawReadThis2,
    330,175,
    0
};

//
// LOAD GAME MENU
//
enum
{
    load1,
    load2,
    load3,
    load4,
    load5,
    load6,
    load_end
} load_e;

menuitem_t LoadMenu[]=
{
    {"", M_LoadSelect, 1},
    {"", M_LoadSelect, 1},
    {"", M_LoadSelect, 1},
    {"", M_LoadSelect, 1},
    {"", M_LoadSelect, 1},
    {"", M_LoadSelect, 1}
};

menu_t  LoadDef =
{
    load_end,
    0,
    &MainDef,
    LoadMenu,
    M_DrawLoad,
    80,54,
    0
};

//
// SAVE GAME MENU
//
menuitem_t SaveMenu[]=
{
    {"", M_SaveSelect, 1},
    {"", M_SaveSelect, 1},
    {"", M_SaveSelect, 1},
    {"", M_SaveSelect, 1},
    {"", M_SaveSelect, 1},
    {"", M_SaveSelect, 1}
};

menu_t  SaveDef =
{
    load_end,
    0,
    &MainDef,
    SaveMenu,
    M_DrawSave,
    80,54,
    0
};

//
// [kg] new options menu
//

extern int i_ctrl_bnt_mask;
extern int i_ctrl_roles;
extern int i_ctrl_btn[];
extern const char *const i_ctrl_names[];

int menu_get_key = -1;

void M_ToggleStick()
{
	i_ctrl_roles = !i_ctrl_roles;
}

void M_ChangeKey(int choice)
{
	menu_get_key = choice - 1;
	M_StartMessage("Press a button ...\n\nPress X to cancel.", NULL, false);
	messageToPrint = 2;
}

static void M_OptColormap(int opt)
{
	if(opt == itemOn)
		v_patch_colormap = v_colormap_normal;
	else
		v_patch_colormap = v_colormap_normal + (256*16);
}

void M_Options(int choice)
{
	M_SetupNextMenu(&OptionsDef);
}

void M_DrawOptions()
{
	int i, x, y;
	const char *tmp;

	// title
	V_DrawPatch(100,30,0,W_CacheLumpName("M_OPTION"));

	// options
	x = 320 - currentMenu->x;
	y = currentMenu->y;

	// stick roles
	M_OptColormap(0);
	tmp = i_ctrl_roles ? "MOVE / LOOK" : "LOOK / MOVE";
	M_WriteText(x - M_StringWidth(tmp), y, tmp);
	y += SHORT(hu_font[0]->height) + 1;

	// keys
	for(i = 0; i < NUM_JOYCON_BUTTONS; i++)
	{
		tmp = i_ctrl_names[i_ctrl_btn[i]];
		M_OptColormap(i + 1);
		M_WriteText(x - M_StringWidth(tmp), y, tmp);
		y += SHORT(hu_font[0]->height) + 1;
	}

	// done
	v_patch_colormap = v_colormap_normal;
}

//
// M_ReadSaveStrings
//  read the strings from the savegame files
//
void M_ReadSaveStrings(void)
{
    int             handle;
    int             count;
    int             i;
    char    name[256];
	
    for (i = 0;i < load_end;i++)
    {
	sprintf(name,SAVEGAMENAME"%d.dsg",i);

//	handle = open (name, O_RDONLY | 0, 0666);
//	if (handle == -1)
	{
	    strcpy(&savegamestrings[i][0],EMPTYSTRING);
	    LoadMenu[i].status = 0;
	    continue;
	}
//	count = read (handle, &savegamestrings[i], SAVESTRINGSIZE);
//	close (handle);
//	LoadMenu[i].status = 1;
    }
}


//
// M_LoadGame & Cie.
//
void M_DrawLoad(void)
{
    int             i;
	
    V_DrawPatch (72,28,0,W_CacheLumpName("M_LOADG"));
    for (i = 0;i < load_end; i++)
    {
	M_DrawSaveLoadBorder(LoadDef.x,LoadDef.y+LINEHEIGHT*i);
	M_WriteText(LoadDef.x,LoadDef.y+LINEHEIGHT*i,savegamestrings[i]);
    }
}



//
// Draw border for the savegame description
//
void M_DrawSaveLoadBorder(int x,int y)
{
    int             i;
	
    V_DrawPatch (x-8,y+7,0,W_CacheLumpName("M_LSLEFT"));
	
    for (i = 0;i < 24;i++)
    {
	V_DrawPatch (x,y+7,0,W_CacheLumpName("M_LSCNTR"));
	x += 8;
    }

    V_DrawPatch (x,y+7,0,W_CacheLumpName("M_LSRGHT"));
}



//
// User wants to load this game
//
void M_LoadSelect(int choice)
{
    char    name[256];
	
    if (M_CheckParm("-cdrom"))
	sprintf(name,"c:\\doomdata\\"SAVEGAMENAME"%d.dsg",choice);
    else
	sprintf(name,SAVEGAMENAME"%d.dsg",choice);
    G_LoadGame (name);
    M_ClearMenus ();
}

//
// Selected from DOOM menu
//
void M_LoadGame (int choice)
{
    if (netgame)
    {
	M_StartMessage(LOADNET,NULL,false);
	return;
    }
	
    M_SetupNextMenu(&LoadDef);
    M_ReadSaveStrings();
}


//
//  M_SaveGame & Cie.
//
void M_DrawSave(void)
{
    int             i;
	
    V_DrawPatch (72,28,0,W_CacheLumpName("M_SAVEG"));
    for (i = 0;i < load_end; i++)
    {
	M_DrawSaveLoadBorder(LoadDef.x,LoadDef.y+LINEHEIGHT*i);
	M_WriteText(LoadDef.x,LoadDef.y+LINEHEIGHT*i,savegamestrings[i]);
    }
	
    if (saveStringEnter)
    {
	i = M_StringWidth(savegamestrings[saveSlot]);
	M_WriteText(LoadDef.x + i,LoadDef.y+LINEHEIGHT*saveSlot,"_");
    }
}

//
// M_Responder calls this when user is finished
//
void M_DoSave(int slot)
{
    G_SaveGame (slot,savegamestrings[slot]);
    M_ClearMenus ();

    // PICK QUICKSAVE SLOT YET?
    if (quickSaveSlot == -2)
	quickSaveSlot = slot;
}

//
// User wants to save. Start string input for M_Responder
//
void M_SaveSelect(int choice)
{
    // we are going to be intercepting all chars
    saveStringEnter = 1;
    
    saveSlot = choice;
    strcpy(saveOldString,savegamestrings[choice]);
    if (!strcmp(savegamestrings[choice],EMPTYSTRING))
	savegamestrings[choice][0] = 0;
    saveCharIndex = strlen(savegamestrings[choice]);
}

//
// Selected from DOOM menu
//
void M_SaveGame (int choice)
{
    if (!usergame)
    {
	M_StartMessage(SAVEDEAD,NULL,false);
	return;
    }
	
    if (gamestate != GS_LEVEL)
	return;
	
    M_SetupNextMenu(&SaveDef);
    M_ReadSaveStrings();
}



//
//      M_QuickSave
//
char    tempstring[80];

void M_QuickSaveResponse(int ch)
{
    if (ch == 'y' && ch != KEY_ENTER)
    {
	M_DoSave(quickSaveSlot);
//	S_StartSound(NULL,sfx_swtchx,0);
    }
}

void M_QuickSave(void)
{
    if (!usergame)
    {
//	S_StartSound(NULL,sfx_oof,0);
	return;
    }

    if (gamestate != GS_LEVEL)
	return;
	
    if (quickSaveSlot < 0)
    {
	M_StartControlPanel();
	M_ReadSaveStrings();
	M_SetupNextMenu(&SaveDef);
	quickSaveSlot = -2;	// means to pick a slot now
	return;
    }
    sprintf(tempstring,QSPROMPT,savegamestrings[quickSaveSlot]);
    M_StartMessage(tempstring,M_QuickSaveResponse,true);
}



//
// M_QuickLoad
//
void M_QuickLoadResponse(int ch)
{
    if (ch == 'y' && ch != KEY_ENTER)
    {
	M_LoadSelect(quickSaveSlot);
//	S_StartSound(NULL,sfx_swtchx,0);
    }
}


void M_QuickLoad(void)
{
    if (netgame)
    {
	M_StartMessage(QLOADNET,NULL,false);
	return;
    }
	
    if (quickSaveSlot < 0)
    {
	M_StartMessage(QSAVESPOT,NULL,false);
	return;
    }
    sprintf(tempstring,QLPROMPT,savegamestrings[quickSaveSlot]);
    M_StartMessage(tempstring,M_QuickLoadResponse,true);
}




//
// Read This Menus
// Had a "quick hack to fix romero bug"
//
void M_DrawReadThis1(void)
{
    inhelpscreens = true;
    switch ( gamemode )
    {
      case commercial:
	V_DrawPatch (0,0,0,W_CacheLumpName("HELP"));
	break;
      case shareware:
      case registered:
      case retail:
	V_DrawPatch (0,0,0,W_CacheLumpName("HELP1"));
	break;
      default:
	break;
    }
    return;
}



//
// Read This Menus - optional second page.
//
void M_DrawReadThis2(void)
{
    inhelpscreens = true;
    switch ( gamemode )
    {
      case retail:
      case commercial:
	// This hack keeps us from having to change menus.
	V_DrawPatch (0,0,0,W_CacheLumpName("CREDIT"));
	break;
      case shareware:
      case registered:
	V_DrawPatch (0,0,0,W_CacheLumpName("HELP2"));
	break;
      default:
	break;
    }
    return;
}

//
// M_DrawMainMenu
//
void M_DrawMainMenu(void)
{
    V_DrawPatch (94,2,0,W_CacheLumpName("M_DOOM"));
}




//
// M_NewGame
//
void M_DrawNewGame(void)
{
    V_DrawPatch (96,14,0,W_CacheLumpName("M_NEWG"));
    V_DrawPatch (54,38,0,W_CacheLumpName("M_SKILL"));
}

void M_NewGame(int choice)
{
    if (netgame)
    {
	M_StartMessage(NEWGAME,NULL,false);
	return;
    }
	
    if ( gamemode == commercial )
	M_SetupNextMenu(&NewDef);
    else
	M_SetupNextMenu(&EpiDef);
}


//
//      M_Episode
//
int     epi;

void M_DrawEpisode(void)
{
    V_DrawPatch (54,38,0,W_CacheLumpName("M_EPISOD"));
}

void M_VerifyNightmare(int ch)
{
    if (ch != 'y' && ch != KEY_ENTER)
	return;
		
    G_DeferedInitNew(nightmare,epi+1,1);
    M_ClearMenus ();
}

void M_ChooseSkill(int choice)
{
    if (choice == nightmare)
    {
	M_StartMessage(NIGHTMARE,M_VerifyNightmare,true);
	return;
    }
	
    G_DeferedInitNew(choice,epi+1,1);
    M_ClearMenus ();
}

void M_Episode(int choice)
{
    if ( (gamemode == shareware)
	 && choice)
    {
	M_StartMessage(SWSTRING,NULL,false);
	M_SetupNextMenu(&ReadDef1);
	return;
    }

    // Yet another hack...
    if ( (gamemode == registered)
	 && (choice > 2))
    {
//      fprintf( stderr,
//	       "M_Episode: 4th episode requires UltimateDOOM\n");
      choice = 0;
    }
	 
    epi = choice;
    M_SetupNextMenu(&NewDef);
}


//
// M_ReadThis
//
void M_ReadThis(int choice)
{
    choice = 0;
    M_SetupNextMenu(&ReadDef1);
}

void M_ReadThis2(int choice)
{
    choice = 0;
    M_SetupNextMenu(&ReadDef2);
}

void M_FinishReadThis(int choice)
{
    choice = 0;
    M_SetupNextMenu(&MainDef);
}




//
// M_QuitDOOM
//
int     quitsounds[8] =
{
    0,// sfx_pldeth,
    0,// sfx_dmpain,
    0,// sfx_popain,
    0,// sfx_slop,
    0,// sfx_telept,
    0,// sfx_posit1,
    0,// sfx_posit3,
    0,// sfx_sgtatk
};

int     quitsounds2[8] =
{
    0,// sfx_vilact,
    0,// sfx_getpow,
    0,// sfx_boscub,
    0,// sfx_slop,
    0,// sfx_skeswg,
    0,// sfx_kntdth,
    0,// sfx_bspact,
    0,// sfx_sgtatk
};



void M_QuitResponse(int ch)
{
    if (ch != 'y' && ch != KEY_ENTER)
	return;
    if (!netgame)
    {
	if (gamemode == commercial)
	    S_StartSound(NULL,quitsounds2[(gametic>>2)&7],0);
	else
	    S_StartSound(NULL,quitsounds[(gametic>>2)&7],0);
//	I_WaitVBL(105); TODO
    }
    I_Quit ();
}




void M_QuitDOOM(int choice)
{
	sprintf(endstring,"%s\n\n"DOSY, endmsg[ (gametic%(NUM_QUITMESSAGES-2))+1 ]);
	M_StartMessage(endstring,M_QuitResponse,true);
}




void M_ChangeSensitivity(int choice)
{
    switch(choice)
    {
      case 0:
	if (mouseSensitivity)
	    mouseSensitivity--;
	break;
      case 1:
	if (mouseSensitivity < 9)
	    mouseSensitivity++;
	break;
    }
}




void M_ChangeDetail(int choice)
{
    choice = 0;
    detailLevel = 1 - detailLevel;

    // FIXME - does not work. Remove anyway?
//    fprintf( stderr, "M_ChangeDetail: low detail mode n.a.\n");

    return;
    
    /*R_SetViewSize (screenblocks, detailLevel);

    if (!detailLevel)
	players[consoleplayer].message = DETAILHI;
    else
	players[consoleplayer].message = DETAILLO;*/
}




void M_SizeDisplay(int choice)
{
    switch(choice)
    {
      case 0:
	if (screenSize > 0)
	{
	    screenblocks--;
	    screenSize--;
	}
	break;
      case 1:
	if (screenSize < 8)
	{
	    screenblocks++;
	    screenSize++;
	}
	break;
    }
	

    R_SetViewSize (screenblocks, detailLevel);
}




//
//      Menu Functions
//
void
M_DrawThermo
( int	x,
  int	y,
  int	thermWidth,
  int	thermDot )
{
    int		xx;
    int		i;

    xx = x;
    V_DrawPatch (xx,y,0,W_CacheLumpName("M_THERML"));
    xx += 8;
    for (i=0;i<thermWidth;i++)
    {
	V_DrawPatch (xx,y,0,W_CacheLumpName("M_THERMM"));
	xx += 8;
    }
    V_DrawPatch (xx,y,0,W_CacheLumpName("M_THERMR"));

    V_DrawPatch ((x+8) + thermDot*8,y,
		       0,W_CacheLumpName("M_THERMO"));
}



void
M_DrawEmptyCell
( menu_t*	menu,
  int		item )
{
    V_DrawPatch (menu->x - 10,        menu->y+item*LINEHEIGHT - 1, 0,
		       W_CacheLumpName("M_CELL1"));
}

void
M_DrawSelCell
( menu_t*	menu,
  int		item )
{
    V_DrawPatch (menu->x - 10,        menu->y+item*LINEHEIGHT - 1, 0,
		       W_CacheLumpName("M_CELL2"));
}


void
M_StartMessage
( char*		string,
  void*		routine,
  boolean	input )
{
    messageLastMenuActive = menuactive;
    messageToPrint = 1;
    messageString = string;
    messageRoutine = routine;
    messageNeedsInput = input;
    messageSound = 0;// sfx_swtchx;
    menuactive = true;
	GrabMouse(0);
    return;
}

void M_StopMessage(void)
{
    menuactive = messageLastMenuActive;
    messageToPrint = 0;
    GrabMouse(!menuactive);
}

//
// Find string width from hu_font chars
//
int M_StringWidth(const char* string)
{
    int             i;
    int             w = 0;
    int             c;
	
    for (i = 0;i < strlen(string);i++)
    {
	c = (uint8_t)toupper(string[i]) - HU_FONTSTART;
	if (c < 0 || c >= HU_FONTSIZE)
	    w += 4;
	else
	    w += SHORT (hu_font[c]->width);
    }
		
    return w;
}



//
//      Find string height from hu_font chars
//
int M_StringHeight(const char* string)
{
    int             i;
    int             h;
    int             height = SHORT(hu_font[0]->height) + 1;
	
    h = height;
    for (i = 0;i < strlen(string);i++)
	if (string[i] == '\n')
	    h += height;
		
    return h;
}


//
//      Write a string using the hu_font
//
void
M_WriteText
( int		x,
  int		y,
  const char*		string)
{
    int		w;
    const char*	ch;
    int		c;
    int		cx;
    int		cy;
		

    ch = string;
    cx = x;
    cy = y;
	
    while(1)
    {
	c = *ch++;
	if (!c)
	    break;
	if (c == '\n')
	{
	    cx = x;
	    cy += 12;
	    continue;
	}
		
	c = toupper(c) - HU_FONTSTART;
	if (c < 0 || c>= HU_FONTSIZE)
	{
	    cx += 4;
	    continue;
	}
		
	w = SHORT (hu_font[c]->width);
	if (cx+w > SCREENWIDTH)
	    break;
	V_DrawPatch(cx, cy, 0, hu_font[c]);
	cx+=w;
    }
}



//
// CONTROL PANEL
//

//
// M_Responder
//
boolean M_Responder (event_t* ev)
{
    int             ch;
    int             i;
    static  int     joywait = 0;
    static  int     mousewait = 0;
    static  int     lasty = 0;
    static  int     mousex = 0;
    static  int     lastx = 0;

    ch = -1;
/*	
    if (ev->type == ev_joystick && joywait < I_GetTime())
    {
	if (ev->data3 == -1)
	{
	    ch = KEY_UPARROW;
	    joywait = I_GetTime() + 5;
	}
	else if (ev->data3 == 1)
	{
	    ch = KEY_DOWNARROW;
	    joywait = I_GetTime() + 5;
	}
		
	if (ev->data2 == -1)
	{
	    ch = KEY_LEFTARROW;
	    joywait = I_GetTime() + 2;
	}
	else if (ev->data2 == 1)
	{
	    ch = KEY_RIGHTARROW;
	    joywait = I_GetTime() + 2;
	}
		
	if (ev->data1&1)
	{
	    ch = KEY_ENTER;
	    joywait = I_GetTime() + 5;
	}
	if (ev->data1&2)
	{
	    ch = KEY_BACKSPACE;
	    joywait = I_GetTime() + 5;
	}
    }
    else
    {
	if (ev->type == ev_mouse && mousewait < I_GetTime())
	{
	    mousex += ev->data2;
	    if (mousex < lastx-30)
	    {
		ch = KEY_LEFTARROW;
		mousewait = I_GetTime() + 5;
		mousex = lastx -= 30;
	    }
	    else if (mousex > lastx+30)
	    {
		ch = KEY_RIGHTARROW;
		mousewait = I_GetTime() + 5;
		mousex = lastx += 30;
	    }
		
	    if (ev->data1&1)
	    {
		ch = KEY_ENTER;
		mousewait = I_GetTime() + 15;
	    }
			
	    if (ev->data1&2)
	    {
		ch = KEY_BACKSPACE;
		mousewait = I_GetTime() + 15;
	    }
	}
	else
	    if (ev->type == ev_keydown)
	    {
		ch = ev->data1;
	    }
    }
*/
    // [kg] joycons
    if(ev->type == ev_joystick)
    {
	if(menu_get_key >= 0)
	{
		if(!(ev->data1 & 0xC00)) // switch: + or -
		{
			ev->data1 &= i_ctrl_bnt_mask;
			if(ev->data1)
			{
				// assign button
				for(i = 0; i < 32; i++)
					if(ev->data1 & (1 << i))
					{
						i_ctrl_btn[menu_get_key] = i;
						menu_get_key = -1;
						// back to menu
						messageSound = 0; // sfx_pistol;
						ch = KEY_ESCAPE;
						break;
					}
			}
			if(ch != KEY_ESCAPE)
				return true;
		}
	} else
	{
		// menu navigation
		if(ev->data1 & (1 << 12))
			ch = KEY_LEFTARROW;
		if(ev->data1 & (1 << 13))
			ch = KEY_UPARROW;
		if(ev->data1 & (1 << 14))
			ch = KEY_RIGHTARROW;
		if(ev->data1 & (1 << 15))
			ch = KEY_DOWNARROW;
		if(ev->data1 & (1 << 0)) // A
			ch = KEY_ENTER;
	}
	if(ev->data1 & 0xC00) // switch: + or -
		ch = KEY_ESCAPE;
    } else
    if (ev->type == ev_keydown)
    {
	ch = ev->data1;
    }

    if (ch == -1)
	return false;

    // Save Game string input
    if (saveStringEnter)
    {
	switch(ch)
	{
	  case KEY_BACKSPACE:
	    if (saveCharIndex > 0)
	    {
		saveCharIndex--;
		savegamestrings[saveSlot][saveCharIndex] = 0;
	    }
	    break;
				
	  case KEY_ESCAPE:
	    saveStringEnter = 0;
	    strcpy(&savegamestrings[saveSlot][0],saveOldString);
	    break;
				
	  case KEY_ENTER:
	    saveStringEnter = 0;
	    if (savegamestrings[saveSlot][0])
		M_DoSave(saveSlot);
	    break;
				
	  default:
	    ch = toupper(ch);
	    if (ch != 32)
		if (ch-HU_FONTSTART < 0 || ch-HU_FONTSTART >= HU_FONTSIZE)
		    break;
	    if (ch >= 32 && ch <= 127 &&
		saveCharIndex < SAVESTRINGSIZE-1 &&
		M_StringWidth(savegamestrings[saveSlot]) <
		(SAVESTRINGSIZE-2)*8)
	    {
		savegamestrings[saveSlot][saveCharIndex++] = ch;
		savegamestrings[saveSlot][saveCharIndex] = 0;
	    }
	    break;
	}
	return true;
    }
    
    // Take care of any messages that need input
    if (messageToPrint)
    {
	if (messageNeedsInput == true &&
	    !(ch == ' ' || ch == 'n' || ch == 'y' || ch == KEY_ESCAPE || ch == KEY_ENTER))
	    return false;
		
	menuactive = messageLastMenuActive;
	if (messageRoutine)
	    messageRoutine(ch);

	S_StartSound(NULL,messageSound,0);

	if(messageToPrint == 2)
	{
	    messageToPrint = 0;
	    return true;
	} else
	{
	    messageToPrint = 0;
	    menuactive = false;
	    GrabMouse(1);
	    return true;
	}
    }
	
    if (devparm && ch == KEY_F1)
    {
	G_ScreenShot ();
	return true;
    }

    // Pop-up menu?
    if (!menuactive)
    {
	if (ch == KEY_ESCAPE)
	{
	    M_StartControlPanel ();
//	    S_StartSound(NULL,sfx_swtchn,0);
	    return true;
	}
	return false;
    }

    
    // Keys usable within menu
    switch (ch)
    {
      case KEY_DOWNARROW:
	do
	{
	    if (itemOn+1 > currentMenu->numitems-1)
		itemOn = 0;
	    else itemOn++;
//	    S_StartSound(NULL,sfx_pstop,0);
	} while(currentMenu->menuitems[itemOn].status==-1);
	return true;
		
      case KEY_UPARROW:
	do
	{
	    if (!itemOn)
		itemOn = currentMenu->numitems-1;
	    else itemOn--;
//	    S_StartSound(NULL,sfx_pstop,0);
	} while(currentMenu->menuitems[itemOn].status==-1);
	return true;

      case KEY_LEFTARROW:
	if (currentMenu->menuitems[itemOn].routine &&
	    currentMenu->menuitems[itemOn].status == 2)
	{
//	    S_StartSound(NULL,sfx_stnmov,0);
	    currentMenu->menuitems[itemOn].routine(0);
	}
	return true;
		
      case KEY_RIGHTARROW:
	if (currentMenu->menuitems[itemOn].routine &&
	    currentMenu->menuitems[itemOn].status == 2)
	{
//	    S_StartSound(NULL,sfx_stnmov,0);
	    currentMenu->menuitems[itemOn].routine(1);
	}
	return true;

      case KEY_ENTER:
	if (currentMenu->menuitems[itemOn].routine &&
	    currentMenu->menuitems[itemOn].status)
	{
	    currentMenu->lastOn = itemOn;
	    if (currentMenu->menuitems[itemOn].status == 2)
	    {
		currentMenu->menuitems[itemOn].routine(1);      // right arrow
//		S_StartSound(NULL,sfx_stnmov,0);
	    }
	    else
	    {
		currentMenu->menuitems[itemOn].routine(itemOn);
//		S_StartSound(NULL,sfx_pistol,0);
	    }
	}
	return true;
		
      case KEY_ESCAPE:
	currentMenu->lastOn = itemOn;
	M_ClearMenus ();
//	S_StartSound(NULL,sfx_swtchx,0);
	return true;
		
      case KEY_BACKSPACE:
	currentMenu->lastOn = itemOn;
	if (currentMenu->prevMenu)
	{
	    currentMenu = currentMenu->prevMenu;
	    itemOn = currentMenu->lastOn;
//	    S_StartSound(NULL,sfx_swtchn,0);
	}
	return true;

    }

    if(netgame)
	return true;
    else
	return false;
}



//
// M_StartControlPanel
//
void M_StartControlPanel (void)
{
    // intro might call this repeatedly
    if (menuactive)
	return;
    
    menuactive = 1;
	GrabMouse(0);
    currentMenu = &MainDef;         // JDC
    itemOn = currentMenu->lastOn;   // JDC
}


//
// M_Drawer
// Called after the view has been rendered,
// but before it has been blitted.
//
void M_Drawer (void)
{
    static short	x;
    static short	y;
    short		i;
    short		max;
    char		string[40];
    int			start;

    inhelpscreens = false;

    
    // Horiz. & Vertically center string and print it.
    if (messageToPrint)
    {
	start = 0;
	y = 100 - M_StringHeight(messageString)/2;
	while(*(messageString+start))
	{
	    for (i = 0;i < strlen(messageString+start);i++)
		if (*(messageString+start+i) == '\n')
		{
		    memset(string,0,40);
		    strncpy(string,messageString+start,i);
		    start += i+1;
		    break;
		}
				
	    if (i == strlen(messageString+start))
	    {
		strcpy(string,messageString+start);
		start += i;
	    }

	    x = 160 - M_StringWidth(string)/2;
	    M_WriteText(x,y,string);
	    y += SHORT(hu_font[0]->height)+1;
	}
	return;
    }

    if (!menuactive)
	return;

    // [kg] fade screen
    V_FadeScreen(colormaps, 16);

    if (currentMenu->routine)
	currentMenu->routine();         // call Draw routine
    
    // DRAW MENU
    x = currentMenu->x;
    y = currentMenu->y;
    max = currentMenu->numitems;

    if(currentMenu->small)
    {
	for (i=0;i<max;i++)
	{
	    if (currentMenu->menuitems[i].name[0])
	    {
		if(i == itemOn)
			v_patch_colormap = v_colormap_normal;
		else
			v_patch_colormap = v_colormap_normal + (256*16);
		M_WriteText(x, y, currentMenu->menuitems[i].name);
	    }
	    y += SHORT(hu_font[0]->height) + 1;
	}
	v_patch_colormap = v_colormap_normal;
    } else
    {
	for (i=0;i<max;i++)
	{
	    if (currentMenu->menuitems[i].name[0])
	    {
		V_DrawPatch (x,y,0,W_CacheLumpName(currentMenu->menuitems[i].name));
	    }
	    y += LINEHEIGHT;
	}
	// DRAW SKULL
	V_DrawPatch(x + SKULLXOFF,currentMenu->y - 5 + itemOn*LINEHEIGHT, 0, W_CacheLumpName(skullName[whichSkull]));
    }

}


//
// M_ClearMenus
//
void M_ClearMenus (void)
{
    menuactive = 0;
	GrabMouse(1);
    // if (!netgame && usergame && paused)
    //       sendpause = true;
}




//
// M_SetupNextMenu
//
void M_SetupNextMenu(menu_t *menudef)
{
    currentMenu = menudef;
    itemOn = currentMenu->lastOn;
}


//
// M_Ticker
//
void M_Ticker (void)
{
    if (--skullAnimCounter <= 0)
    {
	whichSkull ^= 1;
	skullAnimCounter = 8;
    }
}


//
// M_Init
//
void M_Init (void)
{
//    GrabMouse(1); // intentionaly disabled
    currentMenu = &MainDef;
    menuactive = 0;
    itemOn = currentMenu->lastOn;
    whichSkull = 0;
    skullAnimCounter = 10;
    screenSize = screenblocks - 3;
    messageToPrint = 0;
    messageString = NULL;
    messageLastMenuActive = menuactive;
    quickSaveSlot = -1;

    // Here we could catch other version dependencies,
    //  like HELP1/2, and four episodes.

  
    switch ( gamemode )
    {
      case commercial:
	// This is used because DOOM 2 had only one HELP
        //  page. I use CREDIT as second page now, but
	//  kept this hack for educational purposes.
	MainMenu[readthis] = MainMenu[quitdoom];
	MainDef.numitems--;
	MainDef.y += 8;
	NewDef.prevMenu = &MainDef;
	ReadDef1.routine = M_DrawReadThis1;
	ReadDef1.x = 330;
	ReadDef1.y = 165;
	ReadMenu1[0].routine = M_FinishReadThis;
	break;
      case shareware:
	// Episode 2 and 3 are handled,
	//  branching to an ad screen.
      case registered:
	// We need to remove the fourth episode.
	EpiDef.numitems--;
	break;
      case retail:
	// We are fine.
      default:
	break;
    }
    
}

