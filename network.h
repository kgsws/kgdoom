// clinet / server network
// by kgsws

#define NET_VERSION	0x7B2C0000

#define MAXCLIENTS	64	// players and spectators
#define MAX_PACKET_LEN	1024

// packet buffer count for each client
// used to resend any lost packets
#define MAX_PACKET_COUNT	64

// timeout to client drop
#define SERVER_CLIENT_TIMEOUT	(TICRATE*30)
// periodic keep-alive packet if there are no other data
#define SERVER_CLIENT_KEEPALIVE	(TICRATE*5)
// time to consider player lagging
#define SERVER_PLAYER_LAGGING	(TICRATE+(TICRATE/2))
// serverside exit delay
#define EXIT_DELAY	(TICRATE)

// MOTD time
#define CLIENT_MOTDTIME	(TICRATE*10)

// network protocol verision
#define NET_VERSION	0x7B2C0000

#define make_ip(a,b,c,d)	((a) | ((b) << 8) | ((c) << 16) | ((d) << 24))

#ifdef LINUX
#define bsd_send	send
#define bsd_recv	recv
#define bsd_socket	socket
#define bsd_connect	connect
#endif

#ifdef SERVER
typedef struct
{
	uint8_t buffer[MAX_PACKET_LEN];
	uint8_t *bufwrite;
} outpacket_t;

typedef struct
{
	uint8_t buffer[MAX_PACKET_LEN];
	int pos;
} unreliable_t;

typedef struct
{
	// packets
	outpacket_t out[MAX_PACKET_COUNT];
	unreliable_t unreliable;
	int packet_head;
	int packet_tail;
	// connection
	struct sockaddr_in addr;
	int clientstate;
	// game
	player_t *player;
	int last_tic;
	int keep_tic;
	int move_tic;
	int gametic;
} clientinfo_t;
#endif

// client to server
enum
{
	CLM_CONNECT,
	CLM_DISCONNECT,
	CLM_LOADED,
	CLM_PACKET_LOSS,
	CLM_KEEP_ALIVE,
	CLM_JOIN,
	CLM_TICK,
};

// server to client
enum
{
	SVM_CONNECT,
	SVM_DISCONNECT,
	SVM_KEEP_ALIVE,
	SVM_CHANGE_SECTOR,
	SVM_CHANGE_LSIDE,
	SVM_SPAWN_MOBJ,
	SVM_SPAWN_PLAYER,
	SVM_CEILING,
	SVM_FLOOR,
	SVM_PLAYER_INFO,
	SVM_UPDATE_MOBJ,
	SVM_REMOVE_MOBJ,
	SVM_PLAYER_PICKUP,
	SVM_PLAYER_INVENTORY,
	SVM_PLAYER_MESSAGE,
	SVM_SOUND,
};

// flags for MOBJ updates
#define SV_MOBJF_X	0x00000001
#define SV_MOBJF_Y	0x00000002
#define SV_MOBJF_Z	0x00000004
#define SV_MOBJF_ANGLE	0x00000008
#define SV_MOBJF_STATE	0x00000010
// MOBJ spawn/update CMD flags
#define SV_MOBJF_RADIUS	0x00000020
#define SV_MOBJF_HEIGHT	0x00000040
#define SV_MOBJF_FLAGS	0x00000080
#define SV_MOBJF_HEALTH	0x00000100
#define SV_MOBJF_FLOORZ	0x00000200
#define SV_MOBJF_CEILZ	0x00000400
#define SV_MOBJF_MOMX	0x00000800
#define SV_MOBJF_MOMY	0x00001000
#define SV_MOBJF_MOMZ	0x00002000
#define SV_MOBJF_PITCH	0x00004000
#define SV_MOBJF_TARGET	0x00008000
#define SV_MOBJF_SOURCE	0x00010000
#define SV_MOBJF_MDIR	0x00020000
// special slot for players mobj
#define SV_MOBJF_PLAYER 0x08000000
// automatic sound slot playback (7 slots; 0 = no sound)
#define SV_MOBJF_SOUNDMASK	0x70000000
#define SV_MOBJF_SOUND_SEE	0x10000000
#define SV_MOBJF_SOUND_ATTACK	0x20000000
#define SV_MOBJF_SOUND_PAIN	0x30000000
#define SV_MOBJF_SOUND_DEATH	0x40000000
#define SV_MOBJF_SOUND_ACTIVE	0x50000000
// serverside only; autodetect some changes
#define SV_MOBJF_AUTO	0x80000000
// combined flags
#define SV_MOBJF_POSITION	(SV_MOBJF_X|SV_MOBJF_Y|SV_MOBJF_Z)
#define SV_MOBJF_MOMENTNUM	(SV_MOBJF_MOMX|SV_MOBJF_MOMY|SV_MOBJF_MOMZ)

// sector update flags
#define SV_SECTF_FLOORZ		0x0001
#define SV_SECTF_CEILINGZ	0x0002
#define SV_SECTF_FLOORPIC	0x0004
#define SV_SECTF_CEILINGPIC	0x0008
#define SV_SECTF_LIGHTLEVEL	0x0010
#define SV_SECTF_SPECIAL	0x0020
#define SV_SECTF_TAG		0x0040
#define SV_SECTF_USE_TAG	0x4000
#define SV_SECTF_REMOVE_ACTION	0x8000

// sidedef update flags
#define SV_SIDEF_OFFSETX	0x01
#define SV_SIDEF_OFFSETY	0x02
#define SV_SIDEF_TEX_TOP	0x04
#define SV_SIDEF_TEX_MID	0x08
#define SV_SIDEF_TEX_BOT	0x10

// generic plane movement flags
#define SV_MOVEF_STARTPIC	0x0001
#define SV_MOVEF_STOPPIC	0x0002
#define SV_MOVEF_STARTSOUND	0x0004
#define SV_MOVEF_STOPSOUND	0x0008
#define SV_MOVEF_MOVESOUND	0x0010
// use tag instead of sector (NYI)
#define SV_MOVEF_USE_TAG	0x8000

// sound origin types
#define SV_SOUNDO_MOBJ		0
#define SV_SOUNDO_SECTOR	1
#define SV_SOUNDO_LINE		2

// player update info
typedef struct
{
	uint8_t info;
	uint8_t weapon;
} player_extra_t;
#define SV_PLAYI_USE	0x01
#define SV_PLAYI_ATTACK	0x02
#define SV_PLAYI_MOVING	0x40
#define SV_PLAYI_LAG	0x80

// local player state info
#define SV_PLAYL_MASK	0x0F
#define SV_PLAYL_TELEP	0x40
#define SV_PLAYL_LOCAL	0x80

//
// externs

extern int net_timeout;

extern uint8_t recvbuf[MAX_PACKET_LEN];
extern int recvlen;
extern int recvpos;

#ifdef SERVER
extern int maxclients;
extern int maxplayers;
extern clientinfo_t clientinfo[MAXCLIENTS];
extern int playerclient[MAXPLAYERS];
extern uint32_t playerupdate[MAXPLAYERS];
extern int clientnum;
extern clientinfo_t *client;

int exit_countdown;

extern char msg_motd[512];
#endif

int NET_GetByte(uint8_t *out);
int NET_GetUint16(uint16_t *out);
int NET_GetUint32(uint32_t *out);
int NET_GetString(char **out);

#ifdef SERVER
void NET_SetupUnreliable(uint8_t cmd);
#endif
void NET_SetupCommand(uint8_t cmd);
void NET_AddByte(uint8_t out);
void NET_AddUint16(uint16_t out);
void NET_AddUint32(uint32_t out);
void NET_AddString(const char *str);
#ifdef SERVER
void NET_UpdateUnreliable(int client);
void NET_SendCommand(int client);
void NET_SendCommandAll(int client);
#else
void NET_SendCommand();
#endif

