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

#ifndef LINUX

static FILE http_stdout;
static const char http_get_template[] = "GET /files/%s HTTP/1.1\r\nHost: pegaswitch.local\r\nUser-Agent: kgDOOM\r\nAccept-Encoding: none\r\nConnection: close\r\n\r\n";

static struct sockaddr_in server_addr =
{
	.sin_family = AF_INET,
	.sin_port = htons(80)
};

static int http_socket;

static int stdout_http(struct _reent *reent, void *v, const char *ptr, int len)
{
	bsd_send(http_socket, ptr, len, 0);
	return len;
}

static int parse_header(char *buf, int len, int *offset, int *got)
{
	char *ptr = buf;
	char *pptr = buf;
	int ret;
	int state = 0;
	int content_length = 0;

	while(len)
	{
		// get some bytes
		ret = bsd_recv(http_socket, ptr, len, 0);
		if(ret <= 0)
			return -1;
		ptr += ret;
		// parse line(s)
		while(1)
		{
			char *tptr = pptr;
			char *eptr = pptr + ret;
			// get line end
			while(tptr < eptr)
			{
				if(*tptr == '\n')
				{
					if(tptr - pptr <= 1)
					{
						// empty line, header ending
						if(state)
						{
							*offset = (int)((tptr + 1) - buf);
							*got = (int)((ptr - buf) - *offset);
							return content_length;
						} else
							return -2;
					}
					// got one line, parse it
					if(state)
					{
						if(!strncmp(pptr, "Content-Length:", 15))
							sscanf(pptr + 15, "%d", &content_length);
					} else
					{
						int v1, v2, res;
						// HTTP response
						state = 1;
						if(sscanf(pptr, "HTTP/%d.%d %d", &v1, &v2, &res) != 3 || !res)
							return -1;
						if(res != 200)
							return -res;
					}
					// go next
					pptr = tptr + 1;
					break;
				}
				tptr++;
			}
			if(tptr == pptr)
				// no more lines left
				break;
		}
		// go next
		len -= ret;
	}
	
	return -1;
}

int http_get_file(const char *path, void **buff)
{
	char temp[1024];
	int ret, offs, got;
	void *ptr;
	int size;

	http_socket = bsd_socket(2, 1, 6); // AF_INET, SOCK_STREAM, PROTO_TCP
	if(http_socket < 0)
		return -1;

	if(bsd_connect(http_socket, (struct sockaddr*) &server_addr, sizeof(server_addr)) < 0)
	{
		bsd_close(http_socket);
		return -2;
	}

	// make a request
	fprintf(&http_stdout, http_get_template, path);

	// get an answer
	ret = parse_header(temp, sizeof(temp), &offs, &got);
	// load it now
	if(ret > 0)
	{
		printf("- HTTP file size: %iB\n", ret);
		*buff = Z_Malloc(ret, PU_STATIC, NULL);
		ptr = *buff;
		size = ret;
		if(got)
		{
			memcpy(ptr, temp + offs, got);
			ptr += got;
			size -= got;
		}
		while(size)
		{
			got = bsd_recv(http_socket, ptr, size, 0);
			if(got <= 0)
			{
				bsd_close(http_socket);
				printf("- read error\n");
				return -4;
			}
			size -= got;
			ptr += got;
		}
		printf("- file loaded\n");
	}

	bsd_close(http_socket);
	return ret;
}

char *test_argv[] =
{
	"doom",
	"-warp",
	"1",
	"2",
	NULL
};

#endif

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
#ifdef VIDEO_STDOUT
	T_Enable(1);
	T_Colors(9, 0);
	printf("-= ERROR =-\n");
	T_Colors(15, 0);
#endif

	va_list vl;
	int ret;
	va_start(vl, error);
	ret = vprintf(error, vl);
	va_end(vl);

	printf("\n");

#ifdef VIDEO_STDOUT
	sleep(5);
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

	// prepare HTTP stdout
	if(libtransistor_context.workstation_addr)
		server_addr.sin_addr.s_addr = libtransistor_context.workstation_addr;
	else
	{
		server_addr.sin_addr.s_addr = make_ip(192,168,1,47);
		server_addr.sin_port = htons(8001);
		myargv = test_argv;
		myargc = (sizeof(test_argv) / sizeof(char*)) - 1;
	}
	http_stdout._write = stdout_http;
	http_stdout._flags = __SWR | __SNBF;
	http_stdout._bf._base = (void*)1;
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
	}
	displayplayer = consoleplayer;
	playeringame[consoleplayer] = true;
}

void I_NetUpdate()
{

}

