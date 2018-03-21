#ifndef __M_RANDOM__
#define __M_RANDOM__


#include "doomtype.h"

extern int rndindex;
extern int prndindex;
extern uint32_t rand_m_z;
extern uint32_t rand_m_w;

// Returns a number from 0 to 255,
// from a lookup table.
int M_Random (void);

// As M_Random, but used only by the play simulation.
int P_Random (void);

// [kg] bigger random
uint32_t F_Random();

// Fix randoms for demos.
void M_ClearRandom (void);


#endif

