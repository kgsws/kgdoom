// 3D floors
// by kgsws

typedef struct height3d_s
{
	struct height3d_s *prev;
	struct height3d_s *next;
	fixed_t height;
} height3d_t;

typedef struct extraplane_s
{
	struct extraplane_s *next;
	line_t *line;
	sector_t *source;
	fixed_t *height;
	int *pic;
	short *lightlevel;
	int validcount;
	short *clip;
	uint16_t blocking;
	// hitscan
	boolean hitover;
	// renderer
	int *renderstyle;
	uint8_t **rendertable;
} extraplane_t;

extern boolean fakeclip;
extern short *fakecliptop;
extern short *fakeclipbot;
extern extraplane_t *fakeplane;

extern height3d_t height3top;
extern height3d_t height3bot;

void e3d_CleanPlanes();
void e3d_AddExtraFloor(sector_t *dst, sector_t *src, line_t *line, int block);

void e3d_Reset();
short *e3d_NewClip(short *source);
void e3d_NewHeight(fixed_t height);

