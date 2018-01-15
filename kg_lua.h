// LUA game scripting
// by kgsws

#define STATE_NULL_NEXT	0xFFFFFFFF
#define STATE_ANIMATION	0x80000000
#define STATE_AMASK	0x0000FFFF
#define STATE_OMASK	0x7FFF0000
#define STATE_OFFSHIFT	16

extern int linedef_side;

int L_NoRef();
void L_Init();
void L_StateCall(state_t *st, mobj_t *mo);
statenum_t L_StateFromAlias(mobjinfo_t *info, statenum_t state);

void L_SetupMap();
void L_SpawnPlayer(player_t *pl);
int L_TouchSpecial(mobj_t *special, mobj_t *toucher);
void L_FinishGeneric(generic_plane_t *gp, boolean forced);
boolean L_CrushThing(mobj_t *th, sector_t *sec, int lfunc, int larg);

