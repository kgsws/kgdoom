#ifndef __M_RANDOM__
#define __M_RANDOM__


#include "doomtype.h"



// Returns a number from 0 to 255,
// from a lookup table.
int M_Random (void);

// As M_Random, but used only by the play simulation.
int P_Random (void);

// Fix randoms for demos.
void M_ClearRandom (void);


#endif

