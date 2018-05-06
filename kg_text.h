// Hud messages
// by kgsws

extern int hud_font_scale;

void HT_Init(void **font, int *scale);
void HT_SetSmallFont();

int HT_PutChar(int x, int y, uint16_t num);
void HT_PutText(int x, int y, const char *text);
int HT_TextWidth(const char *text);
int HT_FontHeight();
void HT_SetSmallFont(int scale, uint8_t *cm);
void HT_SetFont(void *font, int scale, uint8_t *cm);
void *HT_CheckFont(char *font);
void HT_SetColormap(uint8_t *cm);

