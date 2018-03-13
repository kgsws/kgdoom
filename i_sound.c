#include "doomdef.h"
#include "z_zone.h"

#include "i_system.h"
#include "i_sound.h"
#include "m_argv.h"
#include "m_misc.h"
#include "w_wad.h"
#include "st_stuff.h"

#ifdef LINUX
#include <SDL/SDL.h>
#endif

// UNIX hack, to be removed.
#ifdef SNDSERV
// Separate sound server process.
FILE*	sndserver=0;
char*	sndserver_filename = "./sndserver ";
#elif SNDINTR

// Update all 30 millisecs, approx. 30fps synchronized.
// Linux resolution is allegedly 10 millisecs,
//  scale is microseconds.
#define SOUND_INTERVAL     500

// Get the interrupt. Set duration in millisecs.
int I_SoundSetTimer( int duration_of_tick );
void I_SoundDelTimer( void );
#else
// None?
#endif


// A quick hack to establish a protocol between
// synchronous mix buffer updates and asynchronous
// audio writes. Probably redundant with gametic.
static int flag = 0;

// The number of internal mixing channels,
//  the samples calculated for each mixing step,
//  the size of the 16bit, 2 hardware channel (stereo)
//  mixing buffer, and the samplerate of the raw data.


// Needed for calling the actual sound output.
#define SAMPLECOUNT		512
// It is 2 for 16bit, and 2 for two channels.
#define BUFMUL                  4
#define MIXBUFFERSIZE		(SAMPLECOUNT*BUFMUL)

#define SAMPLERATE		48000	// Hz
#define SAMPLESIZE		2   	// 16bit

// [kg] sampling rate
uint16_t rates[NUM_SFX_CHANNELS];

// The actual output device.
int	audio_fd;

// The channel step amount...
int	channelstep[NUM_SFX_CHANNELS];


// The channel data pointers, start and end.
unsigned char*	channels[NUM_SFX_CHANNELS];
unsigned char*	channelsend[NUM_SFX_CHANNELS];

// Pitch to stepping lookup, unused.
int		steptable[256];

// Volume lookups.
int		vol_lookup[128*256];

// Hardware left and right channel volume lookup.
int*		channelleftvol_lookup[NUM_SFX_CHANNELS];
int*		channelrightvol_lookup[NUM_SFX_CHANNELS];

#ifndef LINUX
#define num_frames_per_chunk	(SAMPLERATE / 35)
static const size_t chunk_size = ((num_frames_per_chunk * sizeof(uint32_t)) + 0xfff) & ~0xfff;
static uint32_t __attribute__((aligned(0x1000))) chunks[2][chunk_size / sizeof(uint32_t)];
static audio_output_t output;
static audio_output_buffer_t buffers[2];
static handle_t event;
#endif


//
// SFX API
// Note: this was called by S_Init.
// However, whatever they did in the
// old DPMS based DOS version, this
// were simply dummies in the Linux
// version.
// See soundserver initdata().
//
void I_SetChannels()
{
  // Init internal lookups (raw data, mixing buffer, channels).
  // This function sets up internal lookups used during
  //  the mixing process. 
  int		i;
  int		j;
    
//  int*	steptablemid = steptable + 128;
  
  // Okay, reset internal mixing channels to zero.
  /*for (i=0; i<NUM_CHANNELS; i++)
  {
    channels[i] = 0;
  }*/

  // This table provides step widths for pitch parameters.
  // I fail to see that this is currently used.
//  for (i=-128 ; i<128 ; i++)
//    steptablemid[i] = (int)(pow(2.0, (i/64.0))*65536.0);
//    steptablemid[i] = (int)(((i/64.0)*(i/64.0))*65536.0);
  
  
  // Generates volume lookup tables
  //  which also turn the unsigned samples
  //  into signed samples.
  for (i=0 ; i<128 ; i++)
    for (j=0 ; j<256 ; j++)
      vol_lookup[i*256+j] = (i*(j-128)*256)/127;
}	

 
void I_SetSfxVolume(int volume)
{
  // Identical to DOS.
  // Basically, this should propagate
  //  the menu/config file setting
  //  to the state variable used in
  //  the mixing.
  snd_SfxVolume = volume;
}

// MUSIC API - dummy. Some code from DOS version.
void I_SetMusicVolume(int volume)
{
  // Internal state variable.
  snd_MusicVolume = volume;
  // Now set volume on output device.
  // Whatever( snd_MusciVolume );
}

//
// Starting a sound means adding it
//  to the current list of active sounds
//  in the internal channels.
// As the SFX info struct contains
//  e.g. a pointer to the raw data,
//  it is ignored.
// As our sound handling does not handle
//  priority, it is ignored.
// Pitching (that is, increased speed of playback)
//  is set, but currently not used by mixing.
//
int
I_StartSound
( int		sfxid,
  int		volume,
  int		seperation,
  int		pitch,
  int		priority,
  int slot )
{
    // set slot sound
    int		i;
    int		rc = -1;
    
    int		oldest = gametic;
    int		oldestnum = 0;

    int		rightvol;
    int		leftvol;

    // Okay, in the less recent channel,
    //  we will handle the new SFX.
    channels[slot] = (unsigned char *)W_CacheLumpNum(sfxid);
    // Set pointer to end of raw data.
    channelsend[slot] = channels[slot] + W_LumpLength(sfxid) - 32;
    // [kg] handle multiple rates
    rates[slot] = *(uint16_t*)(channels[slot] + 2);
    // [kg] reset rate ticker
    channelstep[slot] = 0;

    // Set pointer to raw data.
    channels[slot] += 16;

    // [kg] bogus sound check
    if(channelsend[slot] < channels[slot])
	channelsend[slot] = channels[slot];

    // Separation, that is, orientation/stereo.
    //  range is: 1 - 256
    seperation += 1;

    // Per left/right channel.
    //  x^2 seperation,
    //  adjust volume properly.
    leftvol =
	volume - ((volume*seperation*seperation) >> 16); ///(256*256);
    seperation = seperation - 257;
    rightvol =
	volume - ((volume*seperation*seperation) >> 16);	

    // Sanity check, clamp volume.
    if(rightvol < 0)
	rightvol = 0;
    if(rightvol > 127)
	rightvol = 127;
    if(leftvol < 0)
	leftvol = 0;
    if(leftvol > 127)
	leftvol = 127;

    // Get the proper lookup table piece
    //  for this volume level???
    channelleftvol_lookup[slot] = &vol_lookup[leftvol*256];
    channelrightvol_lookup[slot] = &vol_lookup[rightvol*256];

    return slot;
}



void I_StopSound(int slot)
{
	channels[slot] = 0;
}


int I_SoundIsPlaying(int slot)
{
	return !!channels[slot];
}




//
// This function loops all active (internal) sound
//  channels, retrieves a given number of samples
//  from the raw sound data, modifies it according
//  to the current (internal) channel parameters,
//  mixes the per channel samples into the global
//  mixbuffer, clamping it to the allowed range,
//  and sets up everything for transferring the
//  contents of the mixbuffer to the (two)
//  hardware channels (left and right, that is).
//
// This function currently supports only 16bit.
//
#ifdef LINUX
void SND_Mix(void *unused, int16_t *mixbuffer, int len)
{
	int samples = len / BUFMUL;
#else
void SND_Mix(void *unused, int16_t *mixbuffer, int samples)
{
#endif
  // Mix current sound data.
  // Data, from raw sound, for right and left.
  register unsigned int	sample;
  register int		dl;
  register int		dr;
  
  // Pointers in global mixbuffer, left, right, end.
  signed short*		leftout;
  signed short*		rightout;
  signed short*		leftend;
  // Step in mixbuffer, left and right, thus two.
  int				step;

  // Mixing channel index.
  int				chan;
    
    // Left and right channel
    //  are in global mixbuffer, alternating.
    leftout = mixbuffer;
    rightout = mixbuffer+1;

    // Determine end, for left channel only
    //  (right channel is implicit).
    leftend = mixbuffer + samples*2;

    // [kg] normal or half speed
    if(in_weapon_menu || sv_slowmo)
	step = 1; // half
    else
	step = 0; // normal

    // Mix sounds into the mixing buffer.
    // Loop over step*SAMPLECOUNT,
    //  that is 512 values for two channels.
    while (leftout != leftend)
    {
	// Reset left/right value. 
	dl = 0;
	dr = 0;

	// Love thy L2 chache - made this a loop.
	// Now more channels could be set at compile time
	//  as well. Thus loop those  channels.
	for ( chan = 0; chan < NUM_SFX_CHANNELS; chan++ )
	{
	    // Check channel, if active.
	    if (channels[ chan ])
	    {
		// Get the raw data from the channel. 
		sample = *channels[ chan ];
		// Add left and right part
		//  for this channel (sound)
		//  to the current data.
		// Adjust volume accordingly.

		dl += channelleftvol_lookup[ chan ][sample] * 2;
		dr += channelrightvol_lookup[ chan ][sample] * 2;

		channelstep[chan] += rates[chan] >> step;
		if(channelstep[chan] >= SAMPLERATE)
		{
			channelstep[chan] -= SAMPLERATE;
			channels[chan]++;
		}

		// Check whether we are done.
		if (channels[ chan ] >= channelsend[ chan ])
		    channels[ chan ] = 0;
	    }
	}
	
	// Clamp to range. Left hardware channel.
	// Has been char instead of short.
	// if (dl > 127) *leftout = 127;
	// else if (dl < -128) *leftout = -128;
	// else *leftout = dl;

	if (dl > 0x7fff)
	    *leftout = 0x7fff;
	else if (dl < -0x8000)
	    *leftout = -0x8000;
	else
	    *leftout = dl;

	// Same for right hardware channel.
	if (dr > 0x7fff)
	    *rightout = 0x7fff;
	else if (dr < -0x8000)
	    *rightout = -0x8000;
	else
	    *rightout = dr;

	// Increment current pointers in mixbuffer.
	leftout += 2;
	rightout += 2;
    }
}

void
I_UpdateSoundParams
( int	slot,
  int	volume,
  int	seperation,
  int	pitch)
{
    int		rightvol;
    int		leftvol;

    // Separation, that is, orientation/stereo.
    //  range is: 1 - 256
    seperation += 1;

    // Per left/right channel.
    //  x^2 seperation,
    //  adjust volume properly.
    leftvol =
	volume - ((volume*seperation*seperation) >> 16); ///(256*256);
    seperation = seperation - 257;
    rightvol =
	volume - ((volume*seperation*seperation) >> 16);	

    // Sanity check, clamp volume.
    if (rightvol < 0 || rightvol > 127)
	I_Error("rightvol out of bounds");
    
    if (leftvol < 0 || leftvol > 127)
	I_Error("leftvol out of bounds");
    
    // Get the proper lookup table piece
    //  for this volume level???
    channelleftvol_lookup[slot] = &vol_lookup[leftvol*256];
    channelrightvol_lookup[slot] = &vol_lookup[rightvol*256];
}

void I_ShutdownSound(void)
{
#ifndef LINUX
	audio_ipc_output_stop(&output);
	audio_ipc_output_close(&output);
	audio_ipc_finalize();
#endif
}

#ifndef LINUX
void I_UpdateSound()
{
	audio_output_buffer_t *released;
	uint32_t num;
	result_t r;

	r = svcWaitSynchronization(&num, &event, 1, 0);
	if(r)
		return;
	svcResetSignal(event);

	while(1)
	{
		r = audio_ipc_output_get_released_buffer(&output, &num, &released);
		if(r)
			I_Error("I_UpdateSound: audio_ipc_output_get_released_buffer failed 0x%08X", r);

		if(!released)
			break;

		SND_Mix(NULL, released->sample_data, num_frames_per_chunk);
		released->data_size = num_frames_per_chunk * sizeof(uint32_t);

		r = audio_ipc_output_append_buffer(&output, released);
		if(r)
			I_Error("I_UpdateSound: audio_ipc_output_append_buffer failed 0x%08X", r);
	}
}
#endif

void
I_InitSound()
{ 
#ifdef LINUX
	SDL_AudioSpec fmt;

	fmt.freq = SAMPLERATE;
	fmt.format = AUDIO_S16;
	fmt.channels = 2;
	fmt.samples = SAMPLECOUNT;
	fmt.callback = (void*)SND_Mix;
	fmt.userdata = NULL;

	if(SDL_OpenAudio(&fmt, NULL) < 0)
	{
		printf("SOUND: audio open failed\n");
		return;
	}

	SDL_PauseAudio(0);
#else
	result_t r;
	int i;
	char names[8][0x20];
	uint32_t num_names;

	r = audio_ipc_init();
	if(r)
		I_Error("I_InitSound: audio_ipc_init failed 0x%08X", r);
	r = audio_ipc_list_outputs(&names[0], 8, &num_names);
	if(r)
		I_Error("I_InitSound: audio_ipc_list_outputs failed 0x%08X", r);
	printf("I_InitSound: got %i output(s)\n", num_names);
	r = audio_ipc_open_output(names[0], &output);
	if(r)
		I_Error("I_InitSound: audio_ipc_open_output failed 0x%08X", r);

	if(output.sample_rate != SAMPLERATE || output.sample_format != PCM_INT16 || output.num_channels != 2)
		I_Error("I_InitSound: rate %i channels %i format %i is not supported\n", output.sample_rate, output.num_channels, output.sample_format);

	r = audio_ipc_output_register_buffer_event(&output, &event);
	if(r)
		I_Error("I_InitSound: audio_ipc_output_register_buffer_event failed 0x%08X", r);

	for(int i = 0; i < 2; i++)
	{
		buffers[i].ptr = &buffers[i].sample_data;
		buffers[i].sample_data = chunks[i];
		buffers[i].buffer_size = sizeof(chunks[i]);
		buffers[i].data_size = num_frames_per_chunk * sizeof(uint32_t);
		buffers[i].unknown = 0;

		SND_Mix(NULL, buffers[i].sample_data, num_frames_per_chunk);

		r = audio_ipc_output_append_buffer(&output, &buffers[i]);
		if(r)
			I_Error("I_InitSound: audio_ipc_output_append_buffer %i failed 0x%08X", i, r);
	}

	r = audio_ipc_output_start(&output);
	if(r)
		I_Error("I_InitSound: audio_ipc_output_start failed 0x%08X", r);
#endif
}


//
// MUSIC API.
// Still no music done.
// Remains. Dummies.
//
void I_InitMusic(void)		{ }
void I_ShutdownMusic(void)	{ }

static int	looping=0;
static int	musicdies=-1;

void I_PlaySong(int handle, int looping)
{
  // UNUSED.
  handle = looping = 0;
  musicdies = gametic + TICRATE*30;
}

void I_PauseSong (int handle)
{
  // UNUSED.
  handle = 0;
}

void I_ResumeSong (int handle)
{
  // UNUSED.
  handle = 0;
}

void I_StopSong(int handle)
{
  // UNUSED.
  handle = 0;
  
  looping = 0;
  musicdies = 0;
}

void I_UnRegisterSong(int handle)
{
  // UNUSED.
  handle = 0;
}

int I_RegisterSong(void* data)
{
  // UNUSED.
  data = NULL;
  
  return 1;
}

// Is the song playing?
int I_QrySongPlaying(int handle)
{
  // UNUSED.
  handle = 0;
  return looping || musicdies > gametic;
}

