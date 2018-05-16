#include "doomdef.h"

#include "stdlib.h"

#include "doomtype.h"
#include "i_system.h"

#ifdef __GNUG__
#pragma implementation "m_fixed.h"
#endif
#include "m_fixed.h"




// Fixme. __USE_C_FIXED__ or something.

fixed_t
FixedMul
( fixed_t	a,
  fixed_t	b )
{
    return ((long long) a * (long long) b) >> FRACBITS;
}



//
// FixedDiv, C version.
//

fixed_t
FixedDiv
( fixed_t	a,
  fixed_t	b )
{
    if ( (abs(a)>>14) >= abs(b))
	return (a^b)<0 ? MININT : MAXINT;
    return FixedDiv2 (a,b);
}



fixed_t
FixedDiv2
( fixed_t	a,
  fixed_t	b )
{
#if 1
    int64_t c;
    c = ((int64_t)a * (1<<16)) / ((int64_t)b);
    return (fixed_t) c;
#else

    double c;

    c = ((double)a) / ((double)b) * FRACUNIT;

    if (c >= 2147483648.0 || c < -2147483648.0)
	I_Error("FixedDiv: divide by zero");
    return (fixed_t) c;
#endif
}

