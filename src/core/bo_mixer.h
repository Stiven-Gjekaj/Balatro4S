// bo_mixer: mixes the sounds and the music of the game into one stereo
// stream, as the sound thread of Balatro 1.0.1o (engine/sound_manager.lua)
// does with the sources of LOVE.
//
// A sound is a set of samples in memory, or a stream that the mixer
// decodes from Ogg Vorbis while it plays. Each play makes a voice with a
// pitch and a volume. The pitch changes the speed of the samples, as
// Source:setPitch does. The five music tracks play at the same time, and
// the mixer fades each one in or out, as MODULATE does.
//
// The game thread calls the play and modulate functions. The audio thread
// calls bo_mixer_render. A lock in the mixer keeps the two apart.

#ifndef BO_MIXER_H
#define BO_MIXER_H

#include <stddef.h>
#include <stdint.h>

typedef struct BOMixer BOMixer;

// A new mixer that renders stereo samples at rate samples per second.
BOMixer *bo_mixer_new(int rate);
void bo_mixer_free(BOMixer *mixer);

// Adds a sound with the name code, as "card1", from samples in memory.
// samples holds frames * channels values, with 1 or 2 channels. The mixer
// keeps the buffer and frees it. Returns 0 when the mixer is full.
int bo_mixer_add_samples(BOMixer *mixer, const char *code, int16_t *samples, int frames, int channels, int rate);

// Adds a sound from the bytes of an Ogg Vorbis file. The mixer keeps the
// bytes and frees them. With stream at 1, the mixer decodes the sound while
// it plays, and the sound has one voice at a time. With stream at 0, the
// mixer decodes all of the sound now. Returns 0 when the bytes are not
// Ogg Vorbis or the mixer is full; then the caller keeps the bytes.
int bo_mixer_add_vorbis(BOMixer *mixer, const char *code, unsigned char *ogg, size_t size, int stream);

// Returns 1 when the mixer has the sound with the name code.
int bo_mixer_has(BOMixer *mixer, const char *code);

// The volumes of the settings, from 0 to 1: all audio, the music, and the
// sounds of the game. SOUND.volume of Balatro starts at 50, so all audio
// starts at 0.5.
void bo_mixer_set_volumes(BOMixer *mixer, float volume, float music, float sounds);

// play_sound: plays the sound code with the pitch per and the volume vol.
// It does nothing when the mixer does not have the sound.
void bo_mixer_play(BOMixer *mixer, const char *code, float per, float vol);

// AMBIENT, for the fire and the organ: starts the sound, or sets the
// volume of the voice that plays it. When the sound ends, the next call
// starts it again. A volume of 0 stops it.
void bo_mixer_ambient(BOMixer *mixer, const char *code, float vol, float per);

// modulate_sound: fades the music track desired in, as "music1", and the
// other tracks out, at the speed of dt. NULL or "" stops no track and
// starts none. pitch_mod is G.PITCH_MOD. When no track plays, or a track
// is at its end, all the tracks start again from the start.
void bo_mixer_modulate(BOMixer *mixer, float dt, const char *desired, float pitch_mod);

// Stops all the voices, as STOP_AUDIO does.
void bo_mixer_stop(BOMixer *mixer);

// The voices that play now.
int bo_mixer_voices(BOMixer *mixer);

// Writes frames stereo frames, left and right, into out.
void bo_mixer_render(BOMixer *mixer, int16_t *out, int frames);

#endif
