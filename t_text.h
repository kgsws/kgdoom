// Text output
// by kgsws
// using VGA(?) font

#define VIDEO_STDOUT

#ifdef LINUX
#define BASE_PATH	""
#else
#define BASE_PATH	"/sd/switch/kgdoom/"
#endif

void T_Init();
void T_Enable(int en);
void T_Colors(int front, int back);
void T_InitWads();
void T_PutChar(uint8_t c);

