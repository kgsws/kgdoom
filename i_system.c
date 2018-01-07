#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <stdarg.h>
#include <sys/time.h>
#include <unistd.h>

#include "doomdef.h"
#include "m_misc.h"
#include "i_video.h"
#include "i_sound.h"

#include "d_net.h"
#include "g_game.h"

#ifdef __GNUG__
#pragma implementation "i_system.h"
#endif
#include "i_system.h"

// doom Malloc heap
int doom_heapsize = 64*1024*1024;
uint8_t doom_heap[64*1024*1024];

void
I_Tactile
( int	on,
  int	off,
  int	total )
{
  // UNUSED.
  on = off = total = 0;
}

ticcmd_t	emptycmd;
ticcmd_t*	I_BaseTiccmd(void)
{
    return &emptycmd;
}


int  I_GetHeapSize (void)
{
    return doom_heapsize;
}

byte* I_ZoneBase (int*	size)
{
	void *ptr;
	*size = doom_heapsize;
/*	ptr = malloc(doom_heapsize);
	if(!ptr)
		I_Error("I_ZoneBase: allocation failed");
	return ptr;*/
	return doom_heap;
}



//
// I_GetTime
// returns time in 1/70th second tics
//
int  I_GetTime (void)
{
#ifdef LINUX
    struct timeval	tp;
    struct timezone	tzp;
    int			newtics;
    static int		basetime=0;

    gettimeofday(&tp, &tzp);
    if (!basetime)
	basetime = tp.tv_sec;
    newtics = (tp.tv_sec-basetime)*TICRATE + tp.tv_usec*TICRATE/1000000;
    return newtics;
#else
	return (int)(svcGetSystemTick() / ((uint64_t)19200000 / (uint64_t)TICRATE));
#endif
}



//
// I_Init
//
void I_Init (void)
{
#ifndef SERVER
    I_InitSound();
    I_InitGraphics();
#endif
}

void I_WaitVBL(int count)
{
/*#ifdef SGI
    sginap(1);                                           
#else
#ifdef SUN
    sleep(0);
#else
    usleep (count * (1000000/70) );                                
#endif
#endif*/
}

