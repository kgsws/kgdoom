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
	sector_t *sector;
	int ticrate;
	int curtics;	
} generic_call_t;

typedef struct
{
	thinker_t thinker;
	void **gp; // pointer to origin (floor / ceiling / custom) in sector
	union {
		generic_info_t info;
		generic_call_t call;
	};
	fixed_t speed;
	int lua_action; // Lua function to call on stop
	int lua_arg; // Lua, optional argument
	int lua_crush; // Lua function to call on crush
} generic_plane_t;

extern generic_plane_t *crush_gp;

void T_GenericCeiling(generic_plane_t *gp);
generic_plane_t *P_GenericSectorCeiling(generic_info_t *info);

void T_GenericFloor(generic_plane_t *gp);
generic_plane_t *P_GenericSectorFloor(generic_info_t *info);

void T_GenericCaller(generic_plane_t *gp);
generic_plane_t *P_GenericSectorCaller(generic_call_t *info, int dest);

