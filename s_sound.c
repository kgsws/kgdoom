#include <stdio.h>
#include <stdlib.h>

#include "doomdef.h"

#include "i_system.h"
#include "i_sound.h"
#include "s_sound.h"

#include "z_zone.h"
#include "m_random.h"
#include "w_wad.h"

#include "doomdef.h"
#include "p_local.h"

#include "doomstat.h"

// Purpose?
const char snd_prefixen[]
= { 'P', 'P', 'A', 'S', 'S', 'S', 'M', 'M', 'M', 'S', 'S', 'S' };

#define S_MAX_VOLUME		127

// when to clip out sounds
// Does not fit the large outdoor areas.
#define S_CLIPPING_DIST		(1200*0x10000)

// Distance tp origin when sounds should be maxed out.
// This should relate to movement clipping resolution
// (see BLOCKMAP handling).
// Originally: (200*0x10000).
#define S_CLOSE_DIST		(160*0x10000)


#define S_ATTENUATOR		((S_CLIPPING_DIST-S_CLOSE_DIST)>>FRACBITS)

// Adjustable by menu.
#define NORM_VOLUME    		snd_MaxVolume

#define NORM_PITCH     		128
#define NORM_PRIORITY		64
#define NORM_SEP		128

#define S_PITCH_PERTURB		1
#define S_STEREO_SWING		(96*0x10000)

// percent attenuation from front to back
#define S_IFRACVOL		30

#define NA			0
#define S_NUMCHANNELS		2


// Current music/sfx card - index useless
//  w/o a reference LUT in a sound module.
extern int snd_MusicDevice;
extern int snd_SfxDevice;
// Config file? Same disclaimer as above.
extern int snd_DesiredMusicDevice;
extern int snd_DesiredSfxDevice;



typedef struct
{
    // [kg] sfx lump
    int lump;

    // origin of sound
    void*	origin;

    // [kg] introducing sound slots
    int slot;

    // [kg] client-server issues, don't stop fresh sounds instantly even at volume 0
    int silentic;
   
} channel_t;


// the set of channels available
static channel_t	channels[NUM_SFX_CHANNELS];

// These are not used, but should be (menu).
// Maximum volume of a sound effect.
// Internal default is max out of 0-15.
int 		snd_SfxVolume = 15;

// Maximum volume of music. Useless so far.
int 		snd_MusicVolume = 15; 



// whether songs are mus_paused
static boolean		mus_paused;	

static int		nextcleanup;



//
// Internals.
//
int
S_getChannel
( void*		origin,
  int	sfxinfo,
  soundslot_t   origin_slot );


int
S_AdjustSoundParams
( mobj_t*	listener,
  mobj_t*	source,
  int*		vol,
  int*		sep,
  int*		pitch );

void S_StopChannel(int cnum);



//
// Initializes sound stuff, including volume
// Sets channels, SFX and music volume,
//  allocates channel buffer, sets S_sfx lookup.
//
void S_Init
( int		sfxVolume,
  int		musicVolume )
{  
  int		i;

//  fprintf( stderr, "S_Init: default sfx volume %d\n", sfxVolume);
#ifndef SERVER
  // Whatever these did with DMX, these are rather dummies now.
  I_SetChannels();
#endif
  S_SetSfxVolume(sfxVolume);
  // No music with Linux - another dummy.
  S_SetMusicVolume(musicVolume);

  // Free all channels for use
  for (i=0 ; i<NUM_SFX_CHANNELS ; i++)
    channels[i].lump = 0;

  // no sounds are playing, and they are not mus_paused
  mus_paused = 0;
}




//
// Per level startup code.
// Kills playing sounds at start of level,
//  determines music if any, changes music.
//
void S_Start(void)
{
#ifndef SERVER
  int cnum;
  int mnum;

  // kill all playing sounds at start of level
  //  (trust me - a good idea)
  for (cnum=0 ; cnum<NUM_SFX_CHANNELS ; cnum++)
    if (channels[cnum].lump)
      S_StopChannel(cnum);
  
  // start new music for the level
  mus_paused = 0;
/*
  if (gamemode == commercial)
    mnum = mus_runnin + gamemap - 1;
  else
  {
    int spmus[]=
    {
      // Song - Who? - Where?
      
      mus_e3m4,	// American	e4m1
      mus_e3m2,	// Romero	e4m2
      mus_e3m3,	// Shawn	e4m3
      mus_e1m5,	// American	e4m4
      mus_e2m7,	// Tim 	e4m5
      mus_e2m4,	// Romero	e4m6
      mus_e2m6,	// J.Anderson	e4m7 CHIRON.WAD
      mus_e2m5,	// Shawn	e4m8
      mus_e1m9	// Tim		e4m9
    };
    
    if (gameepisode < 4)
      mnum = mus_e1m1 + (gameepisode-1)*9 + gamemap-1;
    else
      mnum = spmus[gamemap-1];
    }	
  
  // HACK FOR COMMERCIAL
  //  if (commercial && mnum > mus_e3m9)	
  //      mnum -= mus_e3m9;
  
  S_ChangeMusic(mnum, true);
*/  
  nextcleanup = 15;
#endif
}	





void
S_StartSoundAtVolume
( void*		origin_p,
  int		sfx_id,
  int		volume,
  soundslot_t   origin_slot )
{
#ifndef SERVER
  int		rc;
  int		sep;
  int		pitch;
  int		priority;
  int		cnum;

  mobj_t*	origin = (mobj_t *) origin_p;

  // check for bogus sound #
  if(!sfx_id)
    return;
  
    pitch = NORM_PITCH;
    priority = NORM_PRIORITY;

  // Check to see if it is audible,
  //  and if not, modify the params
  if (origin && origin != players[displayplayer].mo)
  {
    rc = S_AdjustSoundParams(players[displayplayer].mo,
			     origin,
			     &volume,
			     &sep,
			     &pitch);
	
    if ( origin->x == players[displayplayer].mo->x
	 && origin->y == players[displayplayer].mo->y)
    {	
      sep 	= NORM_SEP;
    }

    if(!rc && !netgame)
      return;
  }	
  else
  {
    sep = NORM_SEP;
  }
/*
  // hacks to vary the sfx pitches
  if (sfx_id >= sfx_sawup
      && sfx_id <= sfx_sawhit)
  {	
    pitch += 8 - (M_Random()&15);
    
    if (pitch<0)
      pitch = 0;
    else if (pitch>255)
      pitch = 255;
  }
  else if (sfx_id != sfx_itemup
	   && sfx_id != sfx_tink)
  {
    pitch += 16 - (M_Random()&31);
    
    if (pitch<0)
      pitch = 0;
    else if (pitch>255)
      pitch = 255;
  }
*/
  // kill old sound
  S_StopSound(origin, origin_slot);

  // try to find a channel
  cnum = S_getChannel(origin, sfx_id, origin_slot);
  
  if (cnum<0)
    return;

  //
  // This is supposed to handle the loading/caching.
  // For some odd reason, the caching is done nearly
  //  each time the sound is needed?
  //

  // Assigns the handle to one of the channels in the
  //  mix/output buffer.
  I_StartSound(sfx_id,
		volume,
		sep,
		pitch,
		priority,
		cnum);
#endif
}	

void
S_StartSound
( void*		origin,
  int		sfx_id,
  soundslot_t   origin_slot )
{
    S_StartSoundAtVolume(origin, sfx_id, snd_SfxVolume, origin_slot);
}

void S_StopSound(void* origin, soundslot_t origin_slot)
{
#ifndef SERVER
    int cnum;

    for (cnum=0 ; cnum<NUM_SFX_CHANNELS ; cnum++)
    {
	if (channels[cnum].lump && channels[cnum].origin == origin && (origin_slot == SOUND_STOP_ALL || channels[cnum].slot == origin_slot))
	{
	    S_StopChannel(cnum);
	    break;
	}
    }
#endif
}



//
// Stop and resume music, during game PAUSE.
//
void S_PauseSound(void)
{
/*    if (mus_playing && !mus_paused)
    {
	I_PauseSong(mus_playing->handle);
	mus_paused = true;
    }*/
}

void S_ResumeSound(void)
{
/*    if (mus_playing && mus_paused)
    {
	I_ResumeSong(mus_playing->handle);
	mus_paused = false;
    }*/
}

#ifndef SERVER
//
// Updates music & sounds
//
void S_UpdateSounds(void* listener_p)
{
    int		audible;
    int		cnum;
    int		volume;
    int		sep;
    int		pitch;
    int		sfx;
    channel_t*	c;
    
    mobj_t*	listener = (mobj_t*)listener_p;
    
    for (cnum=0 ; cnum<NUM_SFX_CHANNELS ; cnum++)
    {
	c = &channels[cnum];
	sfx = c->lump;

	if (c->lump)
	{
	    if (I_SoundIsPlaying(cnum))
	    {
		// initialize parameters
		volume = snd_SfxVolume;
		pitch = NORM_PITCH;
		sep = NORM_SEP;

		// check non-local sounds for distance clipping
		//  or modify their params
		if (c->origin && listener_p != c->origin)
		{
		    audible = S_AdjustSoundParams(listener,
						  c->origin,
						  &volume,
						  &sep,
						  &pitch);

		    if (!audible && (!netgame || c->silentic < gametic)) // [kg] client-server issues, don't instantly stop inaudible sounds
		    {
			S_StopChannel(cnum);
		    }
		    else
			I_UpdateSoundParams(cnum, volume, sep, pitch);
		}
	    }
	    else
	    {
		// if channel is allocated but sound has stopped,
		//  free it
		S_StopChannel(cnum);
	    }
	}
    }
    // kill music if it is a single-play && finished
    // if (	mus_playing
    //      && !I_QrySongPlaying(mus_playing->handle)
    //      && !mus_paused )
    // S_StopMusic();
}
#endif

void S_SetMusicVolume(int volume)
{
    if (volume < 0 || volume > 127)
    {
	I_Error("Attempt to set music volume at %d",
		volume);
    }    
/*
    I_SetMusicVolume(127);
    I_SetMusicVolume(volume);
    snd_MusicVolume = volume;*/
}



void S_SetSfxVolume(int volume)
{

    if (volume < 0 || volume > 127)
	I_Error("Attempt to set sfx volume at %d", volume);

    snd_SfxVolume = volume * 2;

}

//
// Starts some music with the music id found in sounds.h.
//
void S_StartMusic(int m_id)
{
    S_ChangeMusic(m_id, false);
}

void
S_ChangeMusic
( int			musicnum,
  int			looping )
{
/*    musicinfo_t*	music;
    char		namebuf[9];

    if ( (musicnum <= mus_None)
	 || (musicnum >= NUMMUSIC) )
    {
	return;
//	I_Error("Bad music number %d", musicnum);
    }
    else
	music = &S_music[musicnum];

    if (mus_playing == music)
	return;

    // shutdown old music
    S_StopMusic();

    // get lumpnum if neccessary
    if (!music->lumpnum)
    {
	sprintf(namebuf, "d_%s", music->name);
	music->lumpnum = W_GetNumForName(namebuf);
    }

    // load & register it
    music->data = (void *) W_CacheLumpNum(music->lumpnum);
    music->handle = I_RegisterSong(music->data);

    // play it
    I_PlaySong(music->handle, looping);

    mus_playing = music;*/
}


void S_StopMusic(void)
{
/*    if (mus_playing)
    {
	if (mus_paused)
	    I_ResumeSong(mus_playing->handle);

	I_StopSong(mus_playing->handle);
	I_UnRegisterSong(mus_playing->handle);
	Z_ChangeTag(mus_playing->data, PU_CACHE);
	
	mus_playing->data = 0;
	mus_playing = 0;
    }*/
}


#ifndef SERVER

void S_StopChannel(int cnum)
{
    int		i;
    channel_t*	c = &channels[cnum];

    if (c->lump)
    {
	// stop the sound playing
	if (I_SoundIsPlaying(cnum))
	{
	    I_StopSound(cnum);
	}
	// check to see
	//  if other channels are playing the sound
	for (i=0 ; i<NUM_SFX_CHANNELS ; i++)
	{
	    if (cnum != i
		&& c->lump == channels[i].lump)
	    {
		break;
	    }
	}
	c->lump = 0;
    }

}

// [kg] stop everything (that has mobj)
void S_StopSounds()
{
	int i;

	for(i = 0; i < NUM_SFX_CHANNELS; i++)
		if(channels[i].origin)
			S_StopChannel(i);
}

//
// Changes volume, stereo-separation, and pitch variables
//  from the norm of a sound effect to be played.
// If the sound is not audible, returns a 0.
// Otherwise, modifies parameters and returns 1.
//
int
S_AdjustSoundParams
( mobj_t*	listener,
  mobj_t*	source,
  int*		vol,
  int*		sep,
  int*		pitch )
{
    fixed_t	approx_dist;
    fixed_t	adx;
    fixed_t	ady;
    angle_t	angle;

    // [kg] noone to listen?
    if(!listener)
    {
	*vol = 0;
	return 0;
    }

    // calculate the distance to sound origin
    //  and clip it if necessary
    adx = abs(listener->x - source->x);
    ady = abs(listener->y - source->y);

    // From _GG1_ p.428. Appox. eucledian distance fast.
    approx_dist = adx + ady - ((adx < ady ? adx : ady)>>1);
    
    if (approx_dist > S_CLIPPING_DIST)
    {
	*vol = 0;
	return 0;
    }
    
    // angle of source to listener
    angle = R_PointToAngle2(listener->x,
			    listener->y,
			    source->x,
			    source->y);

    if (angle > listener->angle)
	angle = angle - listener->angle;
    else
	angle = angle + (0xffffffff - listener->angle);

    angle >>= ANGLETOFINESHIFT;

    // stereo separation
    *sep = 128 - (FixedMul(S_STEREO_SWING,finesine[angle])>>FRACBITS);

    // volume calculation
    if (approx_dist < S_CLOSE_DIST)
    {
	*vol = snd_SfxVolume;
    }
    else
    {
	// distance effect
	*vol = (snd_SfxVolume
		* ((S_CLIPPING_DIST - approx_dist)>>FRACBITS))
	    / S_ATTENUATOR; 
    }

    return (*vol > 0);
}


//
// S_getChannel :
//   If none available, return -1.  Otherwise channel #.
//
int
S_getChannel
( void*		origin,
  int	sfxinfo,
  soundslot_t   origin_slot )
{
    // channel number to use
    int		cnum;
    
    channel_t*	c;

    // Find an open channel
    for (cnum=0 ; cnum<NUM_SFX_CHANNELS ; cnum++)
    {
	if (!channels[cnum].lump)
	    break;
	else if (origin &&  channels[cnum].origin == origin && channels[cnum].slot == origin_slot)
	{
	    S_StopChannel(cnum);
	    break;
	}
    }

	// TODO: replace oldest
    // None available
/*    if (cnum == NUM_SFX_CHANNELS)
    {
	// Look for lower priority
	for (cnum=0 ; cnum<NUM_SFX_CHANNELS ; cnum++)
	    if (channels[cnum].sfxinfo->priority >= sfxinfo->priority) break;

	if (cnum == NUM_SFX_CHANNELS)
	{
	    // FUCK!  No lower priority.  Sorry, Charlie.    
	    return -1;
	}
	else
	{
	    // Otherwise, kick out lower priority.
	    S_StopChannel(cnum);
	}
    }
*/
    c = &channels[cnum];

    // channel is decided to be cnum.
    c->lump = sfxinfo;
    c->origin = origin;
    c->slot = origin_slot;
    c->silentic = gametic + (TICRATE/2);

    return cnum;
}
#endif

