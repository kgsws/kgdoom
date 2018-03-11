#include "doomdef.h"

#include "doomstat.h"
#include "i_system.h"
#include "v_video.h"
#include "m_argv.h"
#include "d_main.h"
#include "z_zone.h"

typedef union
{
	struct
	{
		uint8_t red;
		uint8_t green;
		uint8_t blue;
		uint8_t alpha;
	} channel;
	uint32_t value;
} palcol_t;

static palcol_t colors[256];
static int palette_changed;

int i_ctrl_bnt_mask = 0b111000000001111001111111111; // mask of allowed in-game buttons
int i_ctrl_roles = 0;
int i_ctrl_btn[] = {7, 8, 6, 5, 1, 3};

const char *const i_ctrl_names[] =
{
	"A",
	"B",
	"X",
	"Y",
	"Left Stick",
	"Right Stick",
	"L",
	"R",
	"ZL",
	"ZR",
	"Plus",
	"Minus",
	"Left",
	"Up",
	"Right",
	"Down",
	"Left Stick Left",
	"Left Stick Up",
	"Left Stick Right",
	"Left Stick Down",
	"Right Stick Left",
	"Right Stick Up",
	"Right Stick Right",
	"Right Stick Down",
	"SL",
	"SR",
	"touch"
};

#ifdef LINUX
#include <SDL/SDL.h>

// mouse
int absmousex;
int absmousey;

SDL_Surface *screen;
#else

// video
surface_t vidsurf;
revent_h vsync;
uint16_t swizzle[128*128];

// controls
#define SWBTN(x)	(1 << (x))
static hid_shared_memory_t *hmem;
static hid_controller_state_t *cst;
static uint64_t last_stamp;

// old status
static uint64_t buttons;
static int oldjoy0x;
static int oldjoy0y;
static int oldjoy1x;
static int oldjoy1y;

// controllers
int check_presence(hid_controller_t *c)
{
	int i;

	for(i = 0; i < c->main.num_entries; i++)
	{
		if(c->main.entries[i].controller_state & 1)
		{
			cst = &c->main;
			last_stamp = 0;
			printf("Doom: got controller #%i\n", i);
			return 0;
		}
	}
	return 1;
}

// swizling

static int pdep(uint32_t mask, uint32_t value)
{
	uint32_t out = 0;
	for (int shift = 0; shift < 32; shift++)
	{
		uint32_t bit = 1u << shift;
		if (mask & bit)
		{
			if (value & 1)
				out |= bit;
			value >>= 1;
		}
	}
	return out;
}

static uint32_t swizzle_x(uint32_t v) { return pdep(~0x7B4u, v); }
static uint32_t swizzle_y(uint32_t v) { return pdep(0x7B4, v); }

static void init_swizzle()
{
	int x, y;
	int i = 0;
	uint32_t offs_x0 = swizzle_x(0);
	uint32_t offs_y = swizzle_y(0);
	uint32_t x_mask = swizzle_x(~0u);
	uint32_t y_mask = swizzle_y(~0u);
	uint32_t offs_x;

	for(y = 0; y < 128; y++)
	{
		offs_x = offs_x0;
		for(x = 0; x < 128; x++)
		{
			swizzle[i++] = offs_y + offs_x;
			offs_x = (offs_x - x_mask) & x_mask;
		}
		offs_y = (offs_y - y_mask) & y_mask;
	}
}

static void place_tile(uint32_t *dst, uint8_t *src)
{
	int x, y, i;

	i = 0;
	for(y = 0; y < 128; y++)
	{
		for(x = 0; x < 128; x++)
		{
			dst[swizzle[i++]] = colors[*src].value;
			src++;
		}
		src += SCREENWIDTH - 128;
	}
}

#endif

#ifdef LINUX
int xlatekey(int sym, int *key)
{
	switch(sym)
	{
		case SDLK_LEFT:
			*key = KEY_LEFTARROW;
		return 1;
		case SDLK_RIGHT:
			*key = KEY_RIGHTARROW;
		return 1;
		case SDLK_DOWN:
			*key = KEY_DOWNARROW;
		return 1;
		case SDLK_UP:
			*key = KEY_UPARROW;
		return 1;
		case SDLK_ESCAPE:
			*key = KEY_ESCAPE;
		return 1;
		case SDLK_RETURN:
			*key = KEY_ENTER;
		return 1;
		case SDLK_TAB:
			*key = KEY_TAB;
		return 1;

		case SDLK_BACKSPACE:
		case SDLK_DELETE:
			*key = KEY_BACKSPACE;
		return 1;
		case SDLK_PAUSE:
			*key = KEY_PAUSE;
		return 1;

		case SDLK_LSHIFT:
		case SDLK_RSHIFT:
			*key = KEY_RSHIFT;
		return 1;

		case SDLK_LCTRL:
		case SDLK_RCTRL:
			*key = KEY_RCTRL;
		return 1;

		case SDLK_LALT:
		case SDLK_RALT:
			*key = KEY_RALT;
		return 1;

		case SDLK_KP_ENTER:
			*key = '#';
		return 1;

		default:
			if(sym >= SDLK_KP0 && sym <= SDLK_KP9)
			{
				*key = '0' + sym - SDLK_KP0;
				return 1;
			}
			if(sym >= SDLK_SPACE && sym <= SDLK_z)
			{
				*key = sym;
				return 1;
			}
	}
	return 0;
}
#endif

void I_ShutdownGraphics(void)
{
#ifdef LINUX
	SDL_Quit();
#endif
}

void GrabMouse(int grab)
{
#ifdef LINUX
	if(grab)
	{
		SDL_ShowCursor(SDL_DISABLE);
		SDL_WM_GrabInput(SDL_GRAB_ON);
	} else {
		SDL_WM_GrabInput(SDL_GRAB_OFF);
		SDL_ShowCursor(SDL_ENABLE);
	}
#endif
}

static int	lastmousex = 0;
static int	lastmousey = 0;
boolean		mousemoved = false;
boolean		shmFinished;
static uint32_t jbtndbg;

void I_GetEvent(void)
{
#ifdef LINUX
	SDL_Event SDLevent;
	event_t event;
	static int buttons = 0;

	while(SDL_PollEvent(&SDLevent))
	{
		switch(SDLevent.type)
		{
			case SDL_QUIT:
				I_Error("Quick Quit\n");
			break;
			case SDL_KEYDOWN:
				event.type = ev_keydown;
				if(xlatekey(SDLevent.key.keysym.sym, &event.data1))
					D_PostEvent(&event);
				else
				if(SDLevent.key.keysym.sym >= SDLK_F1 && SDLevent.key.keysym.sym <= SDLK_F12)
				{
					jbtndbg |= 1 << (SDLevent.key.keysym.sym - SDLK_F1);
					event.type = ev_joystick;
					event.data1 = jbtndbg;
					event.data2 = 0;
					event.data3 = 0;
					D_PostEvent(&event);
				}
			break;
			case SDL_KEYUP:
				event.type = ev_keyup;
				if(xlatekey(SDLevent.key.keysym.sym, &event.data1))
					D_PostEvent(&event);
				else
				if(SDLevent.key.keysym.sym >= SDLK_F1 && SDLevent.key.keysym.sym <= SDLK_F12)
				{
					jbtndbg &= ~(1 << (SDLevent.key.keysym.sym - SDLK_F1));
					event.type = ev_joystick;
					event.data1 = jbtndbg;
					event.data2 = 0;
					event.data3 = 0;
					D_PostEvent(&event);
				}
			break;
			case SDL_MOUSEBUTTONDOWN:
				if(SDLevent.button.button == SDL_BUTTON_LEFT)
					buttons |= 1;
				if(SDLevent.button.button == SDL_BUTTON_RIGHT)
					buttons |= 2;
				if(SDLevent.button.button == SDL_BUTTON_MIDDLE)
					buttons |= 4;
				event.type = ev_mouse;
				event.data1 = buttons;
				event.data2 = 0;
				event.data3 = 0;
				D_PostEvent(&event);
			break;
			case SDL_MOUSEBUTTONUP:
				if(SDLevent.button.button == SDL_BUTTON_LEFT)
					buttons &= ~1;
				if(SDLevent.button.button == SDL_BUTTON_RIGHT)
					buttons &= ~2;
				if(SDLevent.button.button == SDL_BUTTON_MIDDLE)
					buttons &= ~4;
				event.type = ev_mouse;
				event.data1 = buttons;
				event.data2 = 0;
				event.data3 = 0;
				D_PostEvent(&event);
			break;
			case SDL_MOUSEMOTION:
				absmousex = SDLevent.motion.x;
				absmousey = SDLevent.motion.y;
				event.type = ev_mouse;
				event.data1 = buttons;
				event.data2 = SDLevent.motion.xrel*8;
				event.data3 = SDLevent.motion.yrel*32;
				D_PostEvent(&event);
			break;
		}
	}
#else
	if(cst)
	{
		// get keys
		int i;
		int lost = 1;
		int read = 0;
		uint64_t btn = 0;
		int j0x = 0;
		int j0y = 0;
		int j1x = 0;
		int j1y = 0;

		for(i = 0; i < cst->num_entries; i++)
		{
			if(cst->entries[i].controller_state & 1)
			{
				lost = 0;
				if(cst->entries[i].timestamp > last_stamp)
				{
					read++;
					btn |= cst->entries[i].button_state;
					j0x += (int32_t)cst->entries[i].right_stick_x;
					j0y += (int32_t)cst->entries[i].right_stick_y;
					j1x += (int32_t)cst->entries[i].left_stick_x;
					j1y += (int32_t)cst->entries[i].left_stick_y;
				}
			}
		}

		if(lost)
			cst = NULL;
		if(read)
		{
			// got valid input
			event_t event;
			uint64_t change = buttons ^ btn;

			j0x /= read;
			j0y /= read;
			j1x /= read;
			j1y /= read;
/*
			if(btn & SWBTN(22)) // R: right
				j0x = 127;
			if(btn & SWBTN(20)) // R: left
				j0x = -127;
			if(btn & SWBTN(21)) // R: up
				j0y = -127;
			if(btn & SWBTN(23)) // R: down
				j0y = 127;

			if(btn & SWBTN(18)) // L: right
				j1x = 127;
			if(btn & SWBTN(16)) // L: left
				j1x = -127;
			if(btn & SWBTN(17)) // L: up
				j1y = -127;
			if(btn & SWBTN(19)) // L: down
				j1y = 127;
*/
			// mask out digital joycons changes
			change &= 0b11000000001111111111111111;

			// detect changes for this joycon; append buttons here
			if(change || oldjoy0x != j0x || oldjoy0y != j0y)
			{
				event.type = ev_joystick;
				event.data1 = btn;
				event.data2 = j0x;
				event.data3 = j0y;
				D_PostEvent(&event);
			}

			// detect changes for this joycon; no buttons here
			if(oldjoy1x != j1x || oldjoy1y != j1y)
			{
				event.type = ev_joystick2;
				event.data1 = btn;
				event.data2 = j1x;
				event.data3 = j1y;
				D_PostEvent(&event);
			}

			buttons = btn;
			oldjoy0x = j0x;
			oldjoy0y = j0y;
			oldjoy1x = j1x;
			oldjoy1y = j1y;
		}
	} else
	{
		// scan for controller
		int i;
		hid_controller_t *controller = hmem->controllers;

		for(i = 0; i < 10; i++)
		{
			if(!check_presence(controller))
				break;
			controller++;
		}
	}
#endif
}

//
// I_StartFrame
//
void I_StartFrame (void)
{
#ifdef LINUX
	if(SDL_MUSTLOCK(screen))
		SDL_LockSurface(screen);
#endif
	// input
	I_GetEvent();
}

//
// I_StartTic
//
void I_StartTic (void)
{
    mousemoved = false;
}


//
// I_UpdateNoBlit
//
void I_UpdateNoBlit (void)
{
    // what is this?
}

//
// I_FinishUpdate
//
extern size_t log_length;
void I_FinishUpdate (void)
{
#ifdef LINUX
	if(palette_changed)
		SDL_SetPalette(screen, SDL_PHYSPAL, (void*)colors, 0, 256);

	if(SDL_MUSTLOCK(screen))
		SDL_UnlockSurface(screen);

	SDL_Flip(screen);
#else
	int x, y, i;
	result_t r;
	uint32_t handle_idx;
	uint8_t *src = screens[0];
	uint32_t *dst;

	// vsync
	svcWaitSynchronization(&handle_idx, &vsync, 1, 33333333);
	svcResetSignal(vsync);
	// get buffer
	r = surface_dequeue_buffer(&vidsurf, &dst);
	if(r || !dst)
	{
//		I_Error("I_FinishUpdate: surface_dequeue_buffer failed 0x%08X", r);
		printf("I_FinishUpdate: surface_dequeue_buffer failed 0x%08X\n", r);
		return;
	}
	// copy pixels
	for(y = 0; y < 6; y++)
	{
		for(x = 0; x < 10; x++)
		{
			place_tile(dst, src);
			src += 128;
			dst += 128 * 128;
		}
		src += 127 * SCREENWIDTH;
	}
	// update buffer
	r = surface_queue_buffer(&vidsurf);
	if(r)
		I_Error("I_FinishUpdate: surface_queue_buffer failed 0x%08X", r);
#endif
}

//
// I_ReadScreen
//
void I_ReadScreen (byte* scr)
{
    memcpy (scr, screens[0], SCREENWIDTH*SCREENHEIGHT);
}

//
// Palette stuff.
//

//
// I_SetPalette
//
void I_SetPalette (byte* palette)
{
	register int	i;
	palette_changed = 1;
	// set the X colormap entries
	for (i=0 ; i<256 ; i++)
	{
		colors[i].channel.red = *palette++;
		colors[i].channel.green = *palette++;
		colors[i].channel.blue = *palette++;
		colors[i].channel.alpha = 0xFF;
	}
}

void I_InitGraphics(void)
{
#ifdef LINUX
	if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0)
	{
		I_Error("SDL_Init error\n");
		return;
	}
	if(!(screen = SDL_SetVideoMode(SCREENWIDTH, SCREENHEIGHT, 8, SDL_HWSURFACE)))
	{
		SDL_Quit();
		I_Error("SDL_SetVideoMode error\n");
		return;
	}
	screens[0] = screen->pixels;
#else
	result_t r;
	// allocate memory
//	screens[0] = Z_Malloc(SCREENWIDTH * SCREENHEIGHT, PU_STATIC, NULL);
	screens[0] = Z_Malloc(1280 * 768, PU_STATIC, NULL);
	// init video
	r = gpu_initialize();
	if(r)
		I_Error("I_InitGraphics: gpu_initialize failed 0x%08X");
	r = vi_init();
	if(r)
		I_Error("I_InitGraphics: vi_init failed 0x%08X");
	r = display_init();
	if(r)
		I_Error("I_InitGraphics: display_init failed 0x%08X");
	r = display_open_layer(&vidsurf);
	if(r)
		I_Error("I_InitGraphics: display_open_layer failed 0x%08X");
	r = display_get_vsync_event(&vsync);
	if(r)
		I_Error("I_InitGraphics: display_get_vsync_event failed 0x%08X");
	// init swizzle table
	init_swizzle();
	// init controls too
	hmem = hid_get_shared_memory();
#endif
}

