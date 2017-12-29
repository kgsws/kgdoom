// network buffer functions
// by kgsws
#include <netinet/in.h>
#include "doomdef.h"
#include "doomstat.h"
#include "i_system.h"
#include "d_player.h"
#include "z_zone.h"
#include "network.h"

// common
int net_timeout;

// read
uint8_t recvbuf[MAX_PACKET_LEN];
int recvlen;
int recvpos;

#ifdef SERVER
// write
static uint8_t cmdbuf[MAX_PACKET_LEN];
static int cmdpos;
static int isReliable;
#else
// write
static uint8_t cmdbuf[MAX_PACKET_LEN];
static int cmdpos;
#endif

// empty
static char empty[] = "empty error";

#ifndef SERVER
extern int client_socket;
#endif

//
// packet reading

int NET_GetByte(uint8_t *out)
{
	if(recvlen == recvpos)
	{
		*out = 0;
		return 1;
	}

	*out = recvbuf[recvpos];
	recvpos++;

	return 0;
}

int NET_GetUint16(uint16_t *out)
{
	union
	{
		uint16_t u16;
		uint8_t u8[2];
	} val;

	if(NET_GetByte(&val.u8[0]))
		return 1;
	if(NET_GetByte(&val.u8[1]))
		return 1;

	*out = val.u16;
	return 0;
}

int NET_GetUint32(uint32_t *out)
{
	union
	{
		uint32_t u32;
		uint8_t u8[4];
	} val;

	if(NET_GetByte(&val.u8[0]))
		return 1;
	if(NET_GetByte(&val.u8[1]))
		return 1;
	if(NET_GetByte(&val.u8[2]))
		return 1;
	if(NET_GetByte(&val.u8[3]))
		return 1;

	*out = val.u32;
	return 0;
}

int NET_GetString(char **out)
{
	if(recvlen == recvpos)
	{
		*out = empty;
		return 1;
	}

	*out = (char*)recvbuf + recvpos;

	while(recvbuf[recvpos])
	{
		recvpos++;
		if(recvpos == recvlen)
		{
			recvbuf[recvpos-1] = 0;
			return 1;
		}
	}
	recvpos++;

	return 0;
}

//
// packet writing

#ifdef SERVER
void NET_SetupCommand(uint8_t cmd)
{
	isReliable = 1;
	cmdpos = 1;
	cmdbuf[0] = cmd;
}

void NET_SetupUnreliable(uint8_t cmd)
{
	isReliable = 0;
	cmdpos = 1;
	cmdbuf[0] = cmd;
}
#else
void NET_SetupCommand(uint8_t cmd)
{
//	cmdpos = 1;
//	cmdbuf[0] = cmd;
	cmdbuf[cmdpos] = cmd;
	cmdpos++;
}
#endif

void NET_AddByte(uint8_t out)
{
	cmdbuf[cmdpos] = out;
	cmdpos++;
}

void NET_AddUint16(uint16_t out)
{
	cmdbuf[cmdpos] = out;
	cmdpos++;
	cmdbuf[cmdpos] = out >> 8;
	cmdpos++;
}

void NET_AddUint32(uint32_t out)
{
	cmdbuf[cmdpos] = out;
	cmdpos++;
	cmdbuf[cmdpos] = out >> 8;
	cmdpos++;
	cmdbuf[cmdpos] = out >> 16;
	cmdpos++;
	cmdbuf[cmdpos] = out >> 24;
	cmdpos++;
}

void NET_AddString(const char *str)
{
	int len = strlen(str) + 1;
	memcpy(cmdbuf + cmdpos, str, len);
	cmdpos += len;
}

#ifdef SERVER
void NET_SendCommand(int client)
{
	if(isReliable)
	{
		// use reliable packet buffer
		clientinfo_t *cl = &clientinfo[client];
		outpacket_t *pkt = &cl->out[cl->packet_head % MAX_PACKET_COUNT];

		// move keep-alive
		cl->keep_tic = gametic;

		if(MAX_PACKET_LEN - (pkt->bufwrite - pkt->buffer) < cmdpos)
		{
			// command won't fit here; advace packet
			cl->packet_head++;
			pkt = &cl->out[cl->packet_head % MAX_PACKET_COUNT];
			// reset
			pkt->bufwrite = pkt->buffer;
			// DEBUG
			if(cl->packet_head == cl->packet_tail)
				I_Error("NET_SendCommand: out of packets");
		}

		if(pkt->bufwrite == pkt->buffer)
		{
			// fresh packet; add header
			*(uint32_t*)pkt->buffer = cl->packet_head;
			pkt->bufwrite += sizeof(uint32_t);
		}

		memcpy(pkt->bufwrite, cmdbuf, cmdpos);
		pkt->bufwrite += cmdpos;
	} else
	{
		// use unreliable packet buffer
		if(clientinfo[client].unreliable.pos + cmdpos > MAX_PACKET_LEN)
		{
			// buffer is full, too bad
			// TODO: maybe send out and release some space?
			return;
		}
		memcpy(clientinfo[client].unreliable.buffer + clientinfo[client].unreliable.pos, cmdbuf, cmdpos);
		clientinfo[client].unreliable.pos += cmdpos;
	}
}

void NET_SendCommandAll(int client)
{
	int i;

	for(i = 0; i < maxclients; i++)
	{
		if(clientinfo[i].clientstate < 0)
			continue; // client not connected
		if(i == client)
			continue; // skip this client
		NET_SendCommand(i);
	}
}
#else
extern int keep_alive;
void NET_SendCommand()
{
	if(!cmdpos)
		return;
	keep_alive = gametic + SERVER_CLIENT_KEEPALIVE;
	bsd_send(client_socket, cmdbuf, cmdpos, 0);
	cmdpos = 0;
}
#endif

