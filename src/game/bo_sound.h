// bo_sound: the sounds and the music of the game. It loads the Ogg Vorbis
// files of resources/sounds in Balatro.love on a thread of its own, and
// plays them with bo_mixer and bo_audio.

#ifndef BO_SOUND_H
#define BO_SOUND_H

// Starts the output and the thread that loads the sounds. A sound plays
// only after the thread loads it.
void bo_sound_start(const char *love_path);

// play_sound of Balatro: the pitch per and the volume vol.
void bo_sound_play(const char *code, float per, float vol);

// modulate_sound: the music track desired, as "music1", and G.PITCH_MOD.
void bo_sound_modulate(float dt, const char *desired, float pitch_mod);

// The ambient sounds of the fire and the organ.
void bo_sound_ambient(const char *code, float vol, float per);

// The volumes of the settings, from 0 to 100, as G.SETTINGS.SOUND.
void bo_sound_set_volumes(int volume, int music, int sounds);

// The voices that play now, and the frames that the output rendered.
int bo_sound_voices(void);
long long bo_sound_rendered(void);

// Stops the output in the background, and starts it again.
void bo_sound_set_active(int active);

#endif
