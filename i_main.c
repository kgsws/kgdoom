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

#ifndef LINUX
//#define NET_STDOUT
#endif

// for exit
jmp_buf exitenv;

extern int	key_use;

#ifdef NET_STDOUT
static FILE net_stdout;
static int net_socket;
static struct sockaddr_in server_addr =
{
	.sin_family = AF_INET,
	.sin_port = 0xAF0B,
	.sin_addr = {0x2F01A8C0}
};
#endif

static struct sockaddr_in client_addr =
{
	.sin_family = AF_INET,
};
int client_socket;

int keep_alive;
int last_packet_tic;

#ifdef NET_STDOUT
static int stdout_net(struct _reent *reent, void *v, const char *ptr, int len)
{
	if(len > 0)
		bsd_send(net_socket, ptr, len, 0);
	return len;
}
#endif

void main_finish()
{
	if(netgame)
		CL_Disconnect();
	I_ShutdownSound();
	I_ShutdownGraphics();
#ifndef LINUX
#ifdef NET_STDOUT
	bsd_close(net_socket);
#endif
	display_finalize();
	vi_finalize();
	am_finalize();
	gpu_finalize();
	hid_finalize();
#ifdef NET_STDOUT
	bsd_finalize();
#endif
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

	printf("-= kgDoom =-\n");

	myargc = argc;
	myargv = argv;

#ifndef LINUX
	result_t r;

	if((r = sm_init()) != RESULT_OK)
	{
		printf("- failed to init sm: 0x%x\n", r);
		return 1;
	}

#ifdef NET_STDOUT
	if((r = bsd_init()) != RESULT_OK)
	{
		printf("- failed to init bsd: 0x%x, %d\n", r, bsd_errno);
		sm_finalize();
		return 2;
	}
#endif

	if((r = hid_init()) != RESULT_OK)
	{
		printf("- failed to init hid: 0x%x\n", r);
#ifdef NET_STDOUT
		bsd_finalize();
#endif
		sm_finalize();
		return 3;
	}
#endif

#ifdef NET_STDOUT
	{
		net_socket = bsd_socket(2, 1, 6); // AF_INET, SOCK_STREAM, PROTO_TCP
		if(net_socket >= 0)
		{
			if(bsd_connect(net_socket, (struct sockaddr*) &server_addr, sizeof(server_addr)) < 0)
				bsd_close(net_socket);
			else
			{
				T_SetStdout(stdout_net);
				bsd_send(net_socket, "Connected ...\n", 14, 0);
				printf("Net stdout activated ...\n");
			}
		}
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

