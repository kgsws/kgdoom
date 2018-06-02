// Text output
// by kgsws
// using VGA(?) font
#ifdef LINUX
#define _GNU_SOURCE
#endif
#include <stdio.h>
#include <dirent.h>
#include "doomdef.h"
#include "doomdata.h"
#include "doomstat.h"
#include "t_text.h"
#include "i_video.h"
#include "v_video.h"
#include "w_wad.h"
#include "d_event.h"

#include "vga_font.h"

#include "p_local.h"
#include "z_zone.h"
#include "t_ini.h"

static int txt_x;
static int txt_y;
static int txt_color;
static int txt_back;
static uint8_t *txt_ptr;
FILE *txt_f;
FILE *old_stdout;

#ifdef VIDEO_STDOUT
typedef struct
{
	char path[32];
	char name[32];
	char author[32];
	char iwad[32];
	char startmap[8];
	uint8_t gamemode;
	uint8_t custom;
	uint8_t allow_pwads;
	uint8_t na;
} iwad_item_t;

typedef struct
{
	char name[32];
} pwad_item_t;

static iwad_item_t *iwad_list;
static pwad_item_t pwad_list[16] = {"-= NO PWAD =-"};
static int wad_count;
static int wad_pick;
static int pwad_count;

const char *const game_mode_list[] =
{
	"shareware",
	"registered",
	"commercial",
	"retail",
	NULL
};

#endif

uint8_t text_palette[768] =
{
	0x00, 0x00, 0x00,
	0x80, 0x00, 0x00,
	0x00, 0x80, 0x00,
	0x80, 0x80, 0x00,
	0x00, 0x00, 0x80,
	0x80, 0x00, 0x80,
	0x00, 0x80, 0x80,
	0x80, 0x80, 0x80,
	0x40, 0x40, 0x40,
	0xFF, 0x40, 0x40,
	0x40, 0xFF, 0x40,
	0xFF, 0xFF, 0x40,
	0x40, 0x40, 0xFF,
	0xFF, 0x40, 0xFF,
	0x40, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF
};

void T_PutChar(uint8_t c)
{
	uint8_t i;
	int y;
	const uint8_t *src = vga_font + c * 16;
	uint8_t *dst;

	if(c == '\n')
		txt_x = SCREENWIDTH;

	if(txt_x >= SCREENWIDTH)
	{
		txt_x = 0;
		if(txt_y + 16 >= SCREENHEIGHT)
		{
			uint8_t *ssrc = screens[0] + 16 * SCREENWIDTH;
			uint8_t *end = screens[0] + txt_y * SCREENWIDTH;
			dst = screens[0];

			while(dst < end)
			{
				*dst = *ssrc;
				dst++;
				ssrc++;
			}
			memset(screens[0] + txt_y * SCREENWIDTH, 0, 16 * SCREENWIDTH);
		} else
			txt_y += 16;
		txt_ptr = screens[0] + txt_y * SCREENWIDTH;
		I_FinishUpdate();
	}

	if(c != '\n')
	{
		dst = txt_ptr;
		for(y = 0; y < 16; y++)
		{
			for(i = 128; i; i >>= 1)
			{
				if(*src & i)
					*dst = txt_color;
				else
					*dst = txt_back;
				dst++;
			}
			src++;
			dst += SCREENWIDTH - 8;
		}
		txt_x += 8;
		txt_ptr += 8;
	}
}

#ifdef LINUX
ssize_t T_CustomWrite(void *cookie, const char *buf, size_t size)
#else
static int T_CustomWrite(struct _reent *reent, void *v, const char *buf, int size)
#endif
{
	ssize_t s = size;

	while(s--)
	{
		T_PutChar((uint8_t)*buf);
		buf++;
	}
	return size;
}

void T_Init()
{
	I_SetPalette(text_palette);
	txt_x = 0;
	txt_y = 0;
	txt_back = 0;
	txt_color = 7;
	memset(screens[0], 0, SCREENWIDTH * SCREENHEIGHT);
	I_FinishUpdate();
	txt_ptr = screens[0];

	old_stdout = stdout;

#ifdef LINUX
	cookie_io_functions_t cf;
	cf.write = T_CustomWrite;

	txt_f = fopencookie(NULL, "w", cf);
	if(!txt_f)
	{
		printf("T_Init: failed to create text output\n");
		return;
	}
#else
	// this exact sequence will redirect stdout
	static FILE custom_stdout;
	custom_stdout._write = T_CustomWrite;
	custom_stdout._flags = __SWR | __SNBF;
	custom_stdout._bf._base = (void*)1;
	txt_f = &custom_stdout;
#endif
	stdout = txt_f;
}

void T_Enable(int en)
{
	if(en)
	{
		if(stdout == txt_f)
			return;
		stdout = txt_f;
		txt_x = 0;
		txt_y = 0;
		txt_back = 0;
		txt_color = 7;
		memset(screens[0], 0, SCREENWIDTH * SCREENHEIGHT);
		I_SetPalette(text_palette);
		I_FinishUpdate();
		txt_ptr = screens[0];
	} else
		stdout = old_stdout;
}

void T_Colors(int front, int back)
{
	if(stdout == txt_f)
		fflush(stdout);
	txt_color = front;
	txt_back = back;
}

#ifdef VIDEO_STDOUT
void T_WriteXY(int x, int y, const char *text)
{
	txt_x = x;
	txt_y = y;
	txt_ptr = screens[0] + y * SCREENWIDTH + x;
	while(*text)
	{
		T_PutChar(*text);
		text++;
	}
}

void T_Box(int x, int y, int w, int h)
{
	int i;

	w -= 2;
	h -= 2;
	if(w < 0 || h < 0)
		return;

	txt_x = x;
	txt_y = y;
	txt_ptr = screens[0] + y * SCREENWIDTH + x;

	T_PutChar(0xC9);
	for(i = 0; i < w; i++)
		T_PutChar(0xCD);
	T_PutChar(0xBB);

	for(i = 0; i < h; i++)
	{
		txt_y += 16;
		txt_x = x;
		txt_ptr = screens[0] + txt_y * SCREENWIDTH + txt_x;
		T_PutChar(0xBA);
		txt_x = x + 8 * w + 8;
		txt_ptr = screens[0] + txt_y * SCREENWIDTH + txt_x;
		T_PutChar(0xBA);
	}

	txt_x = x;
	txt_y += 16;
	txt_ptr = screens[0] + txt_y * SCREENWIDTH + txt_x;

	T_PutChar(0xC8);
	for(i = 0; i < w; i++)
		T_PutChar(0xCD);
	T_PutChar(0xBC);
}

static int check_file(const char *path)
{
	FILE *f;

	f = fopen(path, "r");
	if(f)
	{
		fclose(f);
		return 1;
	}
	return 0;
}

static void check_iwad(const char *path, const char *name, int gm)
{
	if(!path || check_file(path))
	{
		strcpy(iwad_list[wad_count].path, path);
		strcpy(iwad_list[wad_count].name, name);
		iwad_list[wad_count].iwad[0] = 0;
		iwad_list[wad_count].gamemode = gm;
		iwad_list[wad_count].custom = 0;
		iwad_list[wad_count].allow_pwads = 1;
		wad_count++;
	}
}

static int get_gamemode(const char *val)
{
	int i = 0;
	const char *const *list = game_mode_list;

	while(*list)
	{
		if(!strcmp(*list, val))
			return i;
		i++;
		list++;
	}
	return shareware;
}

static void add_pwad(char *name)
{
	int i, ret;
	FILE *f;
	wadinfo_t info;
	lumpinfo_t lump;
	char pwad_path[256];

	sprintf(pwad_path, BASE_PATH"pwads/%s", name);

	// check this wad
	f = fopen(pwad_path, "rb");
	if(!f)
		return;

	fread(&info, 1, sizeof(wadinfo_t), f);

	if(info.id != 0x44415749 && info.id != 0x44415750)
	{
		// invalid file
		fclose(f);
		return;
	}

	// locate game info
	fseek(f, info.offs, SEEK_SET);
	for(i = 0; i < info.numlumps; i++)
	{
		ret = fread(&lump, 1, sizeof(lumpinfo_t), f);
		if(ret != sizeof(lumpinfo_t))
		{
			fclose(f);
			return;
		}
		if(lump.size && *((uint64_t*)lump.name) == 0x4f464e494744474b)
		{
			// got GAMEINFO lump
			fseek(f, lump.offset, SEEK_SET);
			i = lump.size;
			if(i > sizeof(pwad_path))
				i = sizeof(pwad_path);
			ret = fread(pwad_path, 1, i, f);
			if(ret > 0)
			{
				Ini_Init(pwad_path, ret);
				if(Ini_GetSection("kgdoom", 0))
				{
					// get game options
					const char *val;
					iwad_item_t iwad;

					// defaults
					iwad.custom = 1;
					iwad.allow_pwads = 1;
					iwad.gamemode = shareware;
					iwad.iwad[0] = 0;
					iwad.author[0] = 0;
					iwad.startmap[0] = 0;
					sprintf(iwad.name, "%.24s", name);
					sprintf(iwad.path, "%.24s", name);

					// game name
					val = Ini_GetValue("name");
					if(val)
						sprintf(iwad.name, "%.31s", val);
					// game author
					val = Ini_GetValue("author");
					if(val)
						sprintf(iwad.author, "%.31s", val);
					// game start map
					val = Ini_GetValue("startmap");
					if(val)
						sprintf(iwad.startmap, "%.31s", val);
					// game type
					val = Ini_GetValue("game");
					if(val)
						iwad.gamemode = get_gamemode(val);
					// optional IWAD
					val = Ini_GetValue("iwad");
					if(val)
						sprintf(iwad.iwad, "%.31s", val);
					// PWAD disable
					val = Ini_GetValue("no_pwads");
					if(val && *val == '1')
						iwad.allow_pwads = 0;

					// add to IWAD menu
					Z_Enlarge(iwad_list, sizeof(iwad_item_t));
					memcpy(&iwad_list[wad_count], &iwad, sizeof(iwad_item_t));
					wad_count++;
				}
			}
			break;
		}
	}

	fclose(f);

	strncpy(pwad_list[pwad_count].name, name, 24);
	pwad_count++;
}

static void check_pwads(const char *dir)
{
	DIR *d;
	struct dirent *de;

	pwad_count = 1;

	d = opendir(dir);
	if(d)
	{
		de = readdir(d);
		while(de && pwad_count < 16)
		{
			if(de->d_name[0] != '.')
			{
				add_pwad(de->d_name);
			}
			de = readdir(d);
		}
		closedir(d);
	}

	if(pwad_count == 1)
		pwad_count = 0;
}

static void process_input()
{
	event_t *ev;
	for( ; eventtail != eventhead ; eventtail++, eventtail &= (MAXEVENTS-1) )
	{
		ev = &events[eventtail];
		if(ev->type == ev_joystick)
		{
			if(ev->data1 & (1 << 0))
			{
				ev->type = ev_keydown;
				ev->data1 = KEY_ENTER;
			}
			if(ev->data1 & (1 << 13))
			{
				ev->type = ev_keydown;
				ev->data1 = KEY_UPARROW;
			}
			if(ev->data1 & (1 << 15))
			{
				ev->type = ev_keydown;
				ev->data1 = KEY_DOWNARROW;
			}
		}
		if(ev->type == ev_keydown)
		{
			switch(ev->data1)
			{
				case KEY_UPARROW:
					if(wad_pick)
					{
						txt_color = 0;
						T_Box(512, 144 + 32 * wad_pick, 32, 3);
						wad_pick--;
						txt_color = 15;
						T_Box(512, 144 + 32 * wad_pick, 32, 3);
						I_FinishUpdate();
					}
				break;
				case KEY_DOWNARROW:
					if(wad_pick < wad_count-1)
					{
						txt_color = 0;
						T_Box(512, 144 + 32 * wad_pick, 32, 3);
						wad_pick++;
						txt_color = 15;
						T_Box(512, 144 + 32 * wad_pick, 32, 3);
						I_FinishUpdate();
					}
				break;
				case KEY_ENTER:
					wad_count = 0;
				break;
			}
		}
	}
}

void I_GetEvent();

void T_InitWads()
{
	iwad_item_t iwad;
	int i, yy;

	T_WriteXY(480, 0, " _         _____");
	T_WriteXY(480, 16, "| |       |  __ \\");
	T_WriteXY(480, 32, "| | ____ _| |  | | ___   ___  _ __ ___");
	T_WriteXY(480, 48, "| |/ / _` | |  | |/ _ \\ / _ \\| '_ ` _ \\");
	T_WriteXY(480, 64, "|   < (_| | |__| | (_) | (_) | | | | | |");
	T_WriteXY(480, 80, "|_|\\_\\__, |_____/ \\___/ \\___/|_| |_| |_|");
	T_WriteXY(480, 96, "      __/ |");
	T_WriteXY(480, 112, "     |___/");

	I_FinishUpdate();

	// pick IWAD
	iwad_list = Z_Malloc(sizeof(iwad_item_t) * 8, PU_STATIC, NULL);

	check_iwad(BASE_PATH"doom1.wad", "Shareware Doom", shareware);
	check_iwad(BASE_PATH"doom.wad", "Registered Doom", registered);
	check_iwad(BASE_PATH"doomu.wad", "Ultimate Doom", retail);
	check_iwad(BASE_PATH"doom2.wad", "Doom 2: Hell On Earth", commercial);
	check_iwad(BASE_PATH"plutonia.wad", "The Plutonia Experiment", commercial);
	check_iwad(BASE_PATH"tnt.wad", "TNT: Evilution", commercial);
	check_iwad(BASE_PATH"freedoom1.wad", "FreeDoom 1", retail);
	check_iwad(BASE_PATH"freedoom2.wad", "FreeDoom 2", commercial);

	check_pwads(BASE_PATH"pwads");

	for(i = 0, yy = 160; i < wad_count; i++, yy += 32)
		T_WriteXY(544, yy, iwad_list[i].name);

	txt_color = 15;
	T_Box(512, 144, 32, 3);

	I_FinishUpdate();

	while(wad_count > 0)
	{
		I_GetEvent();
		process_input();
	}

	// copy & free
	memcpy(&iwad, &iwad_list[wad_pick], sizeof(iwad_item_t));
	Z_Free(iwad_list);

	// pick PWAD(s)
	if(iwad.allow_pwads)
		wad_count = pwad_count;
	else
	{
		wad_count = 0;
		wad_pick = 0;
	}

	memset(screens[0] + 144 * SCREENWIDTH, 0, SCREENWIDTH * SCREENHEIGHT - 144 * SCREENWIDTH);
	I_FinishUpdate();

	if(wad_count > 0)
	{
		wad_pick = 0;
		txt_color = 7;

		for(i = 0, yy = 160; i < wad_count; i++, yy += 32)
			T_WriteXY(544, yy, pwad_list[i].name);

		txt_color = 15;
		T_Box(512, 144, 32, 3);

		I_FinishUpdate();

		while(wad_count > 0)
		{
			I_GetEvent();
			process_input();
		}
	}

	// reset text
	memset(screens[0], 0, SCREENWIDTH * SCREENHEIGHT);
	txt_color = 7;
	txt_x = 0;
	txt_y = 0;
	txt_ptr = screens[0];
	printf("Loading WADs ...\n");
#ifdef LINUX
	fflush(stdout);
#endif

	// IWAD
	gamemode = iwad.gamemode;
	if(iwad.custom)
	{
		char pwad_path[256];

		if(iwad.iwad[0])
		{
			// load actual IWAD first
			sprintf(pwad_path, BASE_PATH"%s", iwad.iwad);
			W_LoadWad(pwad_path);
		}
		// now load game WAD
		sprintf(pwad_path, BASE_PATH"pwads/%s", iwad.path);
		W_LoadWad(pwad_path);
		// apply game options
		memcpy(game_startmap, iwad.startmap, 8);
	} else
	{
		// load common wad first
		W_LoadWad(BASE_PATH"kgdoom.wad");
		// load main IWAD
		W_LoadWad(iwad.path);
	}

	// PWAD
	if(wad_pick)
	{
		char pwad_path[256];
		sprintf(pwad_path, BASE_PATH"pwads/%s", pwad_list[wad_pick].name);
		W_LoadWad(pwad_path);
	}
}
#endif

#ifndef LINUX
void T_SetStdout(void *func)
{
	static FILE custom_stdout;
	custom_stdout._write = func;
	custom_stdout._flags = __SWR | __SNBF;
	custom_stdout._bf._base = (void*)1;
	stdout = &custom_stdout;
}
#endif

