// 3D floors
// by kgsws

typedef struct extraplane_s
{
	struct extraplane_s *next;
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

extraplane_t *e3d_AddFloorPlane(extraplane_t **dest, sector_t *sec);
extraplane_t *e3d_AddCeilingPlane(extraplane_t **dest, sector_t *sec);

void e3d_Reset();
short *e3d_NewClip(short *source);

