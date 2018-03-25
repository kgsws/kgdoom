// singleplayer game recording
// also used as game saves
// by kgsws

#define RECORDING_SIZE	(4*1024*1024)
#define RECORDING_HEAD	0x6472646b
#define RECORDING_VER	0x00000001

extern int rec_is_playback;

void rec_init();
void rec_reset();
void rec_ticcmd(ticcmd_t *cmd);
void rec_save(const char *path, const char *title);
void rec_load(const char *path, int type);
void rec_get_ticcmd(ticcmd_t *cmd);

