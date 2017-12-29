#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <poll.h>
#include <unistd.h>
#include <sys/ioctl.h>

#include "doomdef.h"
#include "doomstat.h"

#include "m_argv.h"
#include "d_main.h"
#include "d_net.h"
#include "i_video.h"
#include "z_zone.h"
#include "d_player.h"
#include "r_main.h"
#include "p_local.h"

#include "network.h"
#include "sv_cmds.h"

extern int netgame;

static int verbuf = NET_VERSION;

static struct pollfd fds[1];

static int server_socket;
static struct sockaddr_in server_addr =
{
	.sin_family = AF_INET,
};

// messages
static const char msg_serverfull[] = "Server is full.";
static const char msg_badversion[] = "Server is different version.";

char msg_motd[512] = "Welcome to kgsws' multiplayer\nDOOM for switch console.\n\nEnjoy your gameplay.";

// client info & packets
int maxclients = MAXCLIENTS;
int maxplayers = MAXPLAYERS;
clientinfo_t clientinfo[MAXCLIENTS]; // connected clients
int playerclient[MAXPLAYERS]; // client to player table
uint32_t playerupdate[MAXPLAYERS]; // set to force player update early
int clientnum;
clientinfo_t *client;

// next level
int exit_countdown;
extern boolean secretexit;
void G_PlayerFinishLevel(int player);
void G_DoLoadLevel();

//
// system stuff

void main_finish()
{
}

void I_Error (char *error, ...)
{
	va_list vl;
	int ret;
	va_start(vl, error);
	ret = vprintf(error, vl);
	va_end(vl);

	printf("\n");

	main_finish();

	//*((uint8_t*)NULL) = 0;
	exit(-1);
}

void I_InitNetwork()
{
	int i;

	netgame = 1;
	startepisode = 1;
	startmap = 1;
	startskill = 4;

//	sv_freeaim = 1;
//	sv_itemrespawn = TICRATE * 10;
//	sv_weaponrespawn = TICRATE * 60;
//	sw_powerrespawn = TICRATE * 10;
//	sv_superrespawn = TICRATE * 10;

	// open server socket
	server_addr.sin_port = htons(5666);

	server_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if(server_socket < 0)
		I_Error("I_InitNetwork: failed to create server socket");

	if(bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)))
	{
		close(server_socket);
		I_Error("I_InitNetwork: failed to bind server port");
	}

	fds[0].fd = server_socket;
	fds[0].events = POLLIN;

	// prepare all clients
	for(i = 0; i < maxclients; i++)
	{
		clientinfo[i].clientstate = -1;
	}
}

int main(int argc, char **argv)
{
	myargc = argc;
	myargv = argv;

	D_DoomMain();

	main_finish();

	return 0;
}

//
// server functions

// next level
void SV_ExitLevel()
{
	int i;
	static int secret_entry;

	gameaction = ga_nothing;

	if(exit_countdown)
		return;

	// take away cards and stuff
	// freeze everyone
	for(i = 0; i < MAXPLAYERS; i++)
		if(playeringame[i])
		{
			G_PlayerFinishLevel(i);
			if(players[i].mo)
			{
				players[i].mo->reactiontime = -1;
				playerupdate[i] = true;
			}
		}

	// decide next level
	switch(gamemode)
	{
		case shareware:
		case registered:
		case retail:
			if(gamemap == 9)
				gamemap = secret_entry + 1;
			else
			if(secretexit)
			{
				secret_entry = gamemap;
				gamemap = 9;
			} else
			{
				gamemap++;
				if(gamemap == 9)
				{
					gamemap = 1;
					gameepisode++;
				}
				switch(gamemode)
				{
					case shareware:
						gameepisode = 1;
					break;
					case registered:
						if(gameepisode > 3)
							gameepisode = 1;
					break;
					case retail:
						if(gameepisode > 4)
							gameepisode = 1;
					break;
				}
			}
		break;
		case commercial:
			if(secretexit)
			{
				if(gamemap > 30)
					gamemap++;
				else
				{
					secret_entry = gamemap + 1;
					gamemap = 31;
				}
			} else
			{
				gamemap++;
				if(gamemap == 31)
					gamemap = 1;
				else
				if(gamemap > 30)
					gamemap = secret_entry;
			}
		break;
	}
	exit_countdown = gametic + EXIT_DELAY;
	startepisode = gameepisode;
	startmap = gamemap;
}

// find client number from connection
int SV_ClientNumber(struct sockaddr_in *addr)
{
	int i;

	for(i = 0; i < maxclients; i++)
	{
		if(clientinfo[i].clientstate >= 0 && addr->sin_addr.s_addr == clientinfo[i].addr.sin_addr.s_addr && addr->sin_port == clientinfo[i].addr.sin_port)
			return i;
	}
	return -1;
}

// find free slot and add client
int SV_FindFreeSlot(struct sockaddr_in *addr)
{
	int i, j;

	for(i = 0; i < maxclients; i++)
	{
		if(clientinfo[i].clientstate < 0)
		{
			clientinfo[i].addr = *addr;
			clientinfo[i].player = NULL;
			clientinfo[i].last_tic = gametic;
			clientinfo[i].packet_head = 0;
			clientinfo[i].packet_tail = 0;
			clientinfo[i].clientstate = 0;
			clientinfo[i].gametic = 0;
			// clear packets
			clientinfo[i].unreliable.pos = sizeof(uint32_t) * 2;
			*(uint32_t*)clientinfo[i].unreliable.buffer = 0xFFFFFFFF; // unreliable header (1st part)
			for(j = 0; j < MAX_PACKET_COUNT; j++)
				clientinfo[i].out[j].bufwrite = clientinfo[i].out[j].buffer;
			return i;
		}
	}
	return -1;
}

// release player from client slot
void SV_DisconnectPlayer(int cl)
{
	player_t *pl = clientinfo[cl].player;
	if(pl)
	{
		int plnum = pl - players;

		playeringame[plnum] = false;

		if(pl->mo)
		{
			pl->mo->player = NULL;
			P_DamageMobj(pl->mo, pl->mo, pl->mo, INSTANTKILL);
		}
		pl->mo = NULL;

		clientinfo[cl].player = NULL;
		if(clientinfo[cl].clientstate > 0)
			clientinfo[cl].clientstate = 1;

		// tell clients
		SV_SpawnPlayer(plnum);

		playercount--;
	}
}

// release client slot; free player if playing
void SV_DisconnectClient(int cl)
{
	printf("- disconnected client %i (%i.%i.%i.%i:%i)\n", cl, clientinfo[cl].addr.sin_addr.s_addr & 0xFF, (clientinfo[cl].addr.sin_addr.s_addr >> 8) & 0xFF, (clientinfo[cl].addr.sin_addr.s_addr >> 16) & 0xFF, clientinfo[cl].addr.sin_addr.s_addr >> 24, (int)ntohs(clientinfo[cl].addr.sin_port));
	clientinfo[cl].addr.sin_addr.s_addr = 0;
	clientinfo[cl].clientstate = -1;
	clientinfo[cl].packet_head = 0;
	clientinfo[cl].packet_tail = 0;
	clientinfo[cl].unreliable.pos = 0;
	// remove player
	SV_DisconnectPlayer(cl);
}

// disconnect everyone on level change
void SV_LevelChange()
{
	int cl;
	player_t *pl;

	for(cl = 0; cl < maxclients; cl++)
	{
		if(clientinfo[cl].clientstate < 0)
			continue;
		pl = clientinfo[cl].player;
		clientinfo[cl].clientstate = 0;
		if(pl)
		{
			// remove mobj
			if(pl->mo)
			{
				pl->mo->player = NULL;
				pl->mo = NULL;
			}
			// tell clients
			SV_SpawnPlayer(pl - players);
			// prepare to respawn
			pl->playerstate = PST_RESPAWN;
		}
	}

	// now make each client to reconnect
	for(cl = 0; cl < maxclients; cl++)
	{
		if(clientinfo[cl].clientstate < 0)
			continue;
		clientnum = cl;
		SV_CmdConnected();
	}
}

// parse client buffer
void SV_ParseClient()
{
	uint32_t tmp;
	uint8_t cmd;

	while(!NET_GetByte(&cmd))
	{
		switch(cmd)
		{
			case CLM_CONNECT:
				// read out version again
				NET_GetUint32(&tmp);
				// resend all packets
				client->packet_tail = client->packet_head - (MAX_PACKET_COUNT-1);
				if(client->packet_tail < 0)
					client->packet_tail = 0;
			break;
			case CLM_DISCONNECT:
				// disconnect client
				SV_DisconnectClient(clientnum);
				return;
			break;
			case CLM_PACKET_LOSS:
				if(SV_CmdPacketLoss())
				{
					SV_DisconnectClient(clientnum);
					return;
				}
			break;
			case CLM_LOADED:
				// read out checksum
				NET_GetUint32(&tmp);
				// full level status update
				if(client->clientstate)
				{
					// resend all packets
					client->packet_tail = client->packet_head - (MAX_PACKET_COUNT-1);
					if(client->packet_tail < 0)
						client->packet_tail = 0;
					break;
				}
				client->clientstate = 1;
				SV_CmdLoaded(tmp);
			break;
			case CLM_KEEP_ALIVE:
				// do nothing
			break;
			case CLM_JOIN:
				SV_CmdJoin();
			break;
			case CLM_TICK:
				SV_CmdTick();
				client->move_tic = gametic;
			break;
			default:
				printf("- unknown client command %i\n", cmd);
			break;
		}
	}
	client->last_tic = gametic;
}

// poll for packets
void I_NetworkPoll()
{
	poll(fds, sizeof(fds) / sizeof(struct pollfd), 1000 / TICRATE);
}

void I_NetUpdate()
{
	struct sockaddr_in addr;
	int length, cl;
	socklen_t fl = sizeof(addr);

	// read all packets
	while(1)
	{
		length = 0;
		ioctl(server_socket, FIONREAD, &length);
		if(length <= 0)
			break;
		if(length > MAX_PACKET_LEN) // should never happen with normal clients
			length = MAX_PACKET_LEN;
		recvlen = recvfrom(server_socket, recvbuf, length, 0, (struct sockaddr*)&addr, &fl);
		cl = SV_ClientNumber(&addr);
		if(cl < 0)
		{
			// new client
			// there is only one message type allowed
			if(recvbuf[0] == CLM_CONNECT)
			{
				if(!memcmp(recvbuf + 1, &verbuf, sizeof(verbuf)))
				{
					cl = SV_FindFreeSlot(&addr);
					if(cl < 0)
					{
						// no free slot; tell client
						memset(recvbuf, 0, sizeof(uint32_t)); // packet number
						recvbuf[sizeof(uint32_t)] = SVM_DISCONNECT; // packet type
						strcpy((char*)recvbuf + sizeof(uint32_t) + 1, msg_serverfull); // packet data
						sendto(server_socket, recvbuf, sizeof(uint32_t) + 1 + sizeof(msg_serverfull), 0, (struct sockaddr*)&addr, fl);
					} else
					{
						printf("- connected client %i (%i.%i.%i.%i:%i)\n", cl, addr.sin_addr.s_addr & 0xFF, (addr.sin_addr.s_addr >> 8) & 0xFF, (addr.sin_addr.s_addr >> 16) & 0xFF, addr.sin_addr.s_addr >> 24, (int)ntohs(addr.sin_port));
						// connected; confirm connection
						clientnum = cl;
						SV_CmdConnected();
					}
				} else
				{
					// invalid version; tell client
					memset(recvbuf, 0, sizeof(uint32_t)); // packet number
					recvbuf[sizeof(uint32_t)] = SVM_DISCONNECT; // packet type
					strcpy((char*)recvbuf + sizeof(uint32_t) + 1, msg_badversion); // packet data
					sendto(server_socket, recvbuf, sizeof(uint32_t) + 1 + sizeof(msg_badversion), 0, (struct sockaddr*)&addr, fl);
				}
			}
		} else
		{
			// existing client; parse packet
			recvpos = 0;

			clientnum = cl;
			client = &clientinfo[cl];

			SV_ParseClient();
		}
	}

	// process timeouts, send keep-alive, update players
	for(cl = 0; cl < maxclients; cl++)
	{
		clientinfo_t *client = &clientinfo[cl];
		if(client->clientstate >= 0)
		{
			// check for timeouts
			if(client->last_tic + SERVER_CLIENT_TIMEOUT < gametic)
			{
				SV_DisconnectClient(cl);
				continue;
			}
			// maybe update (single) player
			if(client->player && (!((gametic^cl) & 3) || playerupdate[client->player - players]) )
			{
				SV_UpdateLocalPlayer(cl);
			}
			// update all other players
			SV_UpdateOtherPlayers(cl);
			// check for keep-alive
			if(client->keep_tic + SERVER_CLIENT_KEEPALIVE < gametic)
			{
				// do not send anything if there is unrelable data
				if(client->unreliable.pos <= sizeof(uint32_t) * 2)
					SV_KeepAlive(cl);
			}
		}
	}

	// send all packets
	for(cl = 0; cl < maxclients; cl++)
	{
		outpacket_t *pkt;
		clientinfo_t *client = &clientinfo[cl];
		// check
		if(client->clientstate < 0)
			continue;
		// send all filled packets
		while(client->packet_head != client->packet_tail)
		{
			pkt = &client->out[client->packet_tail % MAX_PACKET_COUNT];
			client->packet_tail++;
			if(pkt->bufwrite != pkt->buffer)
				sendto(server_socket, pkt->buffer, pkt->bufwrite - pkt->buffer, 0, (struct sockaddr*)&client->addr, sizeof(struct sockaddr_in));
		}
		// maybe send current packet
		pkt = &client->out[client->packet_head % MAX_PACKET_COUNT];
		if(pkt->buffer != pkt->bufwrite)
		{
			sendto(server_socket, pkt->buffer, pkt->bufwrite - pkt->buffer, 0, (struct sockaddr*)&client->addr, sizeof(struct sockaddr_in));
			// clear next one
			client->packet_head++;
			client->packet_tail = client->packet_head;
			pkt = &client->out[client->packet_head % MAX_PACKET_COUNT];
			pkt->bufwrite = pkt->buffer;
		}
		// send unreliable packet
		if(client->unreliable.pos > sizeof(uint32_t) * 2)
		{
			if(client->clientstate > 0)
			{
				// add current reliable packet number here
				*((uint32_t*)(client->unreliable.buffer + sizeof(uint32_t))) = client->packet_head - 1;
				// only send data to fully connected clients
				sendto(server_socket, client->unreliable.buffer, client->unreliable.pos, 0, (struct sockaddr*)&client->addr, sizeof(struct sockaddr_in));
			}
			client->unreliable.pos = sizeof(uint32_t) * 2;
			client->keep_tic = gametic;
		}
	}

	// do next level
	if(exit_countdown)
	{
		if(exit_countdown < gametic)
		{
			SV_LevelChange();
			exit_countdown = 0;
			G_DoLoadLevel();
		}
	}
}

