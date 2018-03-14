// Text output
// by kgsws
// using VGA(?) font
#ifdef LINUX
#define _GNU_SOURCE
#include <stdio.h>
#endif
#include "doomdef.h"
#include "doomdata.h"
#include "t_text.h"
#include "i_video.h"
#include "v_video.h"

#include "vga_font.h"

static int txt_x;
static int txt_y;
static int txt_color;
static int txt_back;
static uint8_t *txt_ptr;
#ifdef LINUX
FILE *txt_f;
FILE *old_stdout;
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
			memcpy(screens[0], screens[0] + 16 * SCREENWIDTH, txt_y * SCREENWIDTH);
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

ssize_t T_CustomWrite(void *cookie, const char *buf, size_t size)
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

#ifdef LINUX
	cookie_io_functions_t cf;
	cf.write = T_CustomWrite;

	txt_f = fopencookie(NULL, "w", cf);
	if(!txt_f)
	{
		printf("T_Init: failed to create text output\n");
		return;
	}
	old_stdout = stdout;
	stdout = txt_f;
#endif
}

void T_Enable(int en)
{
	if(en)
	{
		stdout = txt_f;
		txt_x = 0;
		txt_y = 0;
		txt_back = 0;
		txt_color = 7;
		memset(screens[0], 0, SCREENWIDTH * SCREENHEIGHT);
		I_FinishUpdate();
		txt_ptr = screens[0];
	} else
		stdout = old_stdout;
}

