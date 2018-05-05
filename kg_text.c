// Hud messages
// by kgsws
#include "doomdef.h"
#include "doomstat.h"
#include "i_system.h"
#include "i_video.h"
#include "v_video.h"
#include "w_wad.h"
#include "z_zone.h"
#include "m_swap.h"
#include "r_data.h"

#include "kg_text.h"

// kfn info
typedef struct
{
	uint32_t id;
	uint8_t format;
	uint8_t reserved;
	uint16_t line_height;
	uint16_t num_ranges;
} __attribute__((packed)) kfn_head_t;

// character info
typedef struct
{
	uint16_t width, height;
	int16_t x, y, space;
	uint32_t pixo;
} __attribute__((packed)) kfn_cinfo_t;

// ranges
typedef struct
{
	uint16_t first;
	uint16_t count;
} kfn_range_t;

// for some reason there is letter 'Y' graphics present in every IWAD
// however, this letter is actually showing up as 'I'
// so, match this data and ignore it if found
static const uint8_t hack_char[] = {0x04, 0x00, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x00, 0x00, 0x00, 0x24, 0x00, 0x00, 0x00, 0x30, 0x00, 0x00, 0x00, 0x3C, 0x00, 0x00, 0x00, 0x00, 0x07, 0xBF, 0xBF, 0xBF, 0xBF, 0xBF, 0xBF, 0xBF, 0xBF, 0xBF, 0xFF, 0x00, 0x07, 0xBF, 0xBF, 0xB1, 0xB5, 0xB5, 0xB5, 0xB5, 0xBF, 0xBF, 0xFF, 0x00, 0x07, 0xBF, 0xBF, 0xB1, 0xB3, 0xB6, 0xB5, 0xB3, 0xBF, 0xBF, 0xFF, 0x00, 0x07, 0xBF, 0xBF, 0xBF, 0xBF, 0xBF, 0xBF, 0xBF, 0xBF, 0xBF, 0xFF};

static int hack_char_active;
static int font_height;
static uint32_t pixel_offs;
static kfn_head_t *doom_small_font;
static kfn_head_t *hud_font;
static uint8_t *hud_colormap;
int hud_font_scale;
static void (*hud_draw_func)(int, int, uint8_t*, int, int, uint8_t*);

static uint16_t fake_space[4] = {4, 0, 0, 0};

static void *hud_funcs[] =
{
	V_DrawBlock1,
	V_DrawBlock2,
	V_DrawBlock3
};

// [kg] render patch to texture
// this is not same as in r_data.c
static void HT_RenderToTexture(uint8_t *dst, patch_t *patch)
{
	int x = 0;
	int pw = SHORT(patch->width);
	int ph = SHORT(patch->height);
	int w = pw;

	memset(dst, 0, pw * ph);

	while(w--)
	{
		column_t *column = (column_t *)((byte *)patch + LONG(patch->columnofs[x]));
		while(column->topdelta != 0xff)
		{
			uint8_t *source = (byte *)column + 3;
			uint8_t *dest = dst + x + column->topdelta * pw;
			int count = column->length;
			while(count--)
			{
				if(*source)
					*dest = *source;
				else
					*dest = transpixel;
				source++;
				dest += pw;
			}
			column = (column_t *)(  (byte *)column + column->length + 4 );
		}
		x++;
	}
}

static void *HT_AddRangeDoom(kfn_head_t *font, void *ptr, char *basename, int start, int count)
{
	int i;
	char name[24];
	patch_t *patch;
	kfn_range_t *range = ptr;

	Z_Enlarge(font, sizeof(kfn_range_t) + count * sizeof(kfn_cinfo_t));
	range->first = start;
	range->count = count;
	ptr += sizeof(kfn_range_t);

	for(i = 0; i < count; i++)
	{
		int lump;
		kfn_cinfo_t *info = ptr;
		if(hack_char_active && start + i == 121)
			lump = -1;
		else
		{
			sprintf(name, basename, start + i);
			lump = W_CheckNumForName(name);
		}
		if(lump < 0)
		{
			if(start + i == 0x20)
				patch = (patch_t*)fake_space;
			else
			{
				sprintf(name, basename, (start + i) & 0xDF);
				patch = W_CacheLumpName(name);
				// TODO: avoid duplicating character pixels
			}
		} else
			patch = W_CacheLumpNum(lump);
		info->width = SHORT(patch->width);
		info->height = SHORT(patch->height);
		info->space = info->width;
		info->x = -SHORT(patch->leftoffset);
		info->y = -SHORT(patch->topoffset);
		if(font_height < info->height)
			font_height = info->height;
		info->pixo = pixel_offs;
		pixel_offs += info->width * info->height;
		ptr += sizeof(kfn_cinfo_t);
	}

	return ptr;
}

// [kg] build KFN font in memory from WAD lumps
static kfn_head_t *HT_DoomFont(char *basename)
{
	int i, j;
	char name[24];
	int start, count;
	kfn_head_t *font;
	void *ptr, *pixptr;
	uint32_t pixo;
	int last = -1;
	int num_ranges = 0;

	hack_char_active = 0;

	pixel_offs = 0;
	font_height = 0;

	font = Z_Malloc(sizeof(kfn_head_t), PU_STATIC, NULL);
	ptr = &font[1];

	// build character ranges
	for(i = 1; i < 65536; i++)
	{
		int lump, len;
		len = sprintf(name, basename, i);
		if(len <= 8 && i != 65536)
		{
			lump = W_CheckNumForName(name);
			if(lump >= 0 && W_LumpLength(lump) == sizeof(hack_char) && !memcmp(hack_char, W_CacheLumpNum(lump), sizeof(hack_char)))
			{
				hack_char_active = 1;
				lump = -1;
			}
		} else
			lump = -1;
		if(lump < 0)
		{
			if(i == 0x20)
				lump = 666;
			if(i >= 'a' && i <= 'z')
			{
				sprintf(name, basename, i & 0xDF);
				lump = W_CheckNumForName(name);
			}
		}
		if(lump >= 0)
		{
			if(last < 0)
			{
				// new range
				count = 1;
				start = i;
				last = i;
			} else
			{
				if(i != last + 1)
				{
					// close last range
					ptr = HT_AddRangeDoom(font, ptr, basename, start, count);
					num_ranges++;
					// new range
					count = 1;
					start = i;
					last = i;
				} else
				{
					count++;
					last++;
				}
			}
		} else
		{
			if(last > 0)
			{
				// close this range
				ptr = HT_AddRangeDoom(font, ptr, basename, start, count);
				num_ranges++;
				last = -1;
			}
		}
	}

	// store properties
	font->line_height = font_height;
	font->num_ranges = num_ranges;

	// store pixel start
	pixptr = ptr;
	pixo = ptr - ((void*)font);

	// add pixels
	Z_Enlarge(font, pixel_offs);
	ptr = &font[1];
	for(i = 0; i < num_ranges; i++)
	{
		kfn_range_t *range = ptr;
		ptr += sizeof(kfn_range_t);
		for(j = 0; j < range->count; j++)
		{
			int lump;
			kfn_cinfo_t *info = ptr;
			ptr += sizeof(kfn_cinfo_t);
			info->pixo += pixo;
			if(hack_char_active && range->first + j == 121)
				lump = -1;
			else
			{
				sprintf(name, basename, range->first + j);
				lump = W_CheckNumForName(name);
			}
			if(lump < 0)
			{
				sprintf(name, basename, (range->first + j) & 0xDF);
				lump = W_CheckNumForName(name);
			}
			if(lump >= 0)
				HT_RenderToTexture(((void*)font) + info->pixo, W_CacheLumpNum(lump));
		}
	}

	printf("HT_DoomFont: %i ranges\n", num_ranges);
	return font;
}

static const char *HT_DecodeUTF8(const char *src, uint16_t *dst)
{
	if((*src & 0xE0) == 0xC0)
	{
		if((src[1] & 0xC0) != 0x80)
		{
			*dst = 0;
			return src + 1;
		}
		*dst = ((*src & 0x1F) << 6) | (src[1] & 0x3F);
		return src + 2;
	} else
	if((*src & 0xF0) == 0xE0)
	{
		if((src[1] & 0xC0) != 0x80 || (src[2] & 0xC0) != 0x80)
		{
			*dst = 0;
			return src + 1;
		}
		*dst = ((*src & 0x0F) << 12) | ((src[1] & 0x3F) << 6) | (src[2] & 0x3F);
		return src + 3;
	}
	*dst = *src;
	return src + 1;
}

void HT_Init()
{
	doom_small_font = HT_DoomFont("STCFN%.3d");
	HT_SetSmallFont(3, NULL);
}

void HT_SetSmallFont(int scale, uint8_t *cm)
{
	if(scale > 3)
		scale = 3;
	if(scale < 1)
		scale = 1;
	hud_font = doom_small_font;
	if(cm)
		hud_colormap = cm;
	else
		hud_colormap = W_CacheLumpName("COLORMAP");
	hud_draw_func = hud_funcs[scale-1];
	hud_font_scale = scale;
}

int HT_PutChar(int x, int y, uint16_t num)
{
	int i;
	void *ptr = &hud_font[1];

	for(i = 0; i < hud_font->num_ranges; i++)
	{
		kfn_range_t *range = ptr;
		if(range->first > num)
			// character is not present in this font
			return 0;
		if(num >= range->first && num < range->first + range->count)
		{
			// found correct range
			kfn_cinfo_t *info = ptr + sizeof(kfn_range_t) + sizeof(kfn_cinfo_t) * (num - range->first);
			// draw
			if(info->width && info->height && hud_draw_func)
				hud_draw_func(x + info->x * hud_font_scale, y + info->y * hud_font_scale, hud_colormap, info->width, info->height, ((void*)hud_font) + info->pixo);
			// done
			return info->space * hud_font_scale;
		} else
			// try next range
			ptr += sizeof(kfn_range_t) + sizeof(kfn_cinfo_t) * range->count;
	}
	return 0;
}

void HT_PutText(int x, int y, const char *text)
{
	while(*text)
	{
		uint16_t num;
		text = HT_DecodeUTF8(text, &num);
		x += HT_PutChar(x, y, num);
	}
}

int HT_TextWidth(const char *text)
{
	int x = 0;
	void *df = hud_draw_func;

	hud_draw_func = NULL;
	while(*text)
	{
		uint16_t num;
		text = HT_DecodeUTF8(text, &num);
		x += HT_PutChar(0, 0, num);
	}
	hud_draw_func = df;
	return x;
}

int HT_FontHeight()
{
	return hud_font->line_height;
}

void HT_SetColormap(uint8_t *cm)
{
	hud_colormap = cm;
}

