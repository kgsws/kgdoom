#ifndef __P_SETUP__
#define __P_SETUP__


#ifdef __GNUG__
#pragma interface
#endif


// NOT called by W_Ticker. Fixme.
void
P_SetupLevel
( int		episode,
  int		map,
  int		playermask,
  skill_t	skill);

// Called by startup code.
void P_Init (void);

#endif

