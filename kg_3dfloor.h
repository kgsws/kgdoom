// 3D floors
// by kgsws

typedef struct extraplane_s
{
	struct extraplane_s *next;
	sector_t *source;
	fixed_t *height;
	int *pic;
	short *lightlevel;
	int validcount;
	short *clip;
} extraplane_t;

extern boolean fakeclip;
extern short *fakecliptop;
extern short *fakeclipbot;
extern extraplane_t *fakeplane;

void e3d_AddExtraFloor(sector_t *dst, sector_t *src, line_t *line);

void e3d_Reset();
short *e3d_NewClip(short *source);

