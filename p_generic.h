// generic thinkers
// by kgsws

//
// planes

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

//
// lines

typedef struct
{
	fixed_t x, y;
} generic_texscrl_t;

typedef struct
{
	thinker_t thinker;
	void **ga; // pointer to origin (either side)
	side_t *side;
	union
	{
		generic_texscrl_t scroll;
	};
} generic_line_t;

void T_TexScroll(generic_line_t *ga);
generic_line_t *P_TextureScroller(line_t *line, fixed_t x, fixed_t y, int side);

//
// mobj

typedef struct generic_ticker_s
{
	// it's not a thinker
	struct generic_ticker_s *next;
	int id;	// user ID; unique
	int ticrate;
	int curtics;
	int lua_action; // Lua function to call
	int lua_arg; // Lua, optional argument
	void *patch;
} generic_ticker_t;

void P_AddMobjTicker(mobj_t *mo, int id, int ticrate, int action, int arg, int patch);
void P_RemoveMobjTicker(mobj_t *mo, int id);
void P_RemoveMobjTickers(mobj_t *mo);

