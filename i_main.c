#include "doomdef.h"
#include "doomstat.h"

#include "m_argv.h"
#include "d_main.h"
#include "d_net.h"
#include "i_video.h"
#include "z_zone.h"
#include "g_game.h"
#include "i_system.h"
#include "i_sound.h"

#include <setjmp.h>

#ifdef LINUX
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#else
#include <sys/fcntl.h>
#endif

#include "network.h"
#include "cl_cmds.h"

#include "kg_record.h"

#include "t_text.h"

// for exit
jmp_buf exitenv;

extern int	key_use;

static struct sockaddr_in client_addr =
{
	.sin_family = AF_INET,
};
int client_socket;

int keep_alive;
int last_packet_tic;

void main_finish()
{
	if(netgame)
		CL_Disconnect();
	I_ShutdownSound();
	I_ShutdownGraphics();
#ifndef LINUX
	display_finalize();
	vi_finalize();
	gpu_finalize();
	hid_finalize();
	bsd_finalize();
	sm_finalize();
#endif
}

extern fixed_t viewx;
extern fixed_t viewy;
extern fixed_t viewz;

void I_Error (char *error, ...)
{
	va_list vl;
	int ret;

#ifdef VIDEO_STDOUT
	T_Enable(1);
	T_Colors(9, 0);
	printf("-= ERROR =-\n");
	T_Colors(15, 0);
#endif

	va_start(vl, error);
	ret = vprintf(error, vl);
	va_end(vl);

	printf("\n");

#ifdef VIDEO_STDOUT
#ifdef LINUX
	fflush(stdout);
	sleep(5);
#else
	svcSleepThread(5000000000UL);
#endif
#endif

//	printf("\nAt %ix%ix%i\n", viewx / FRACUNIT, viewy / FRACUNIT, viewz / FRACUNIT);

//	I_FinishUpdate();
//	*((uint8_t*)1) = 1;

	longjmp(exitenv, 2);
}

//
// I_Quit
//
void I_Quit (void)
{
	longjmp(exitenv, 1);
}

int main(int argc, char **argv)
{
	int ret;

	myargc = argc;
	myargv = argv;

#ifndef LINUX
	result_t r;

	if((r = sm_init()) != RESULT_OK)
	{
		printf("- failed to init sm: 0x%x\n", r);
		return 1;
	}

	if((r = bsd_init()) != RESULT_OK)
	{
		printf("- failed to init bsd: 0x%x, %d\n", r, bsd_errno);
		sm_finalize();
		return 2;
	}

	if((r = hid_init()) != RESULT_OK)
	{
		printf("- failed to init hid: 0x%x\n", r);
		bsd_finalize();
		sm_finalize();
		return 3;
	}
#endif

	srand(time(NULL));

	ret = setjmp(exitenv);
	if(ret)
	{
		main_finish();
		return ret - 1;
	}

	D_DoomMain();
	return 0;
}

//
// client functions

void I_InitNetwork()
{
	int i;

	// setup players for Lua
	for(i = 0; i < MAXPLAYERS; i++)
		players[i].think.lua_type = TT_PLAYER;

	{
		// singleplayer game
		consoleplayer = 0;
		playercount = 1;
		// init save / recording
		rec_init();
	}
	displayplayer = consoleplayer;
	playeringame[consoleplayer] = true;
}

void I_NetUpdate()
{

}

