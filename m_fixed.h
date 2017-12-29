#ifndef __M_FIXED__
#define __M_FIXED__


#ifdef __GNUG__
#pragma interface
#endif

//#ifdef LINUX
#include <inttypes.h>
//#endif

//
// Fixed point, 32bit as 16.16.
//
#define FRACBITS		16
#define FRACUNIT		(1<<FRACBITS)

typedef int32_t fixed_t;

fixed_t FixedMul	(fixed_t a, fixed_t b);
fixed_t FixedDiv	(fixed_t a, fixed_t b);
fixed_t FixedDiv2	(fixed_t a, fixed_t b);



#endif

