// generic floor / ceiling movement
// by kgsws

typedef struct
{
	sector_t *sector;
	fixed_t startz;
	fixed_t stopz;
	fixed_t speed;
	fixed_t crushspeed;
	int startpic;
	int stoppic;
	int startsound;
	int stopsound;
	int movesound;
} generic_info_t;

typedef struct
{
	thinker_t thinker;
	generic_info_t info;
	fixed_t speed;
} generic_plane_t;

void T_GenericCeiling(generic_plane_t *gp);
void P_GenericSectorCeiling(sector_t *sec, generic_info_t *info);

void T_GenericFloor(generic_plane_t *gp);
void P_GenericSectorFloor(sector_t *sec, generic_info_t *info);

