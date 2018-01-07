// LUA game scripting
// by kgsws

#define STATE_ANIMATION	0x80000000
#define STATE_AMASK	0x0000FFFF
#define STATE_OMASK	0x7FFF0000
#define STATE_OFFSHIFT	16

void L_Init();
void L_StateCall(state_t *st, mobj_t *mo);
statenum_t L_StateFromAlias(mobj_t *mo, statenum_t state);

