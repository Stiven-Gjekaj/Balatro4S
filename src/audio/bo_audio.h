// bo_audio: sends the stereo stream of a mixer to the speaker of the
// phone with the RemoteIO audio unit.

#ifndef BO_AUDIO_H
#define BO_AUDIO_H

#include "bo_mixer.h"

#define BO_AUDIO_RATE 44100

// Starts the audio session and the output. The audio unit calls
// bo_mixer_render on its own thread. The session is "ambient sound", so
// the ring/silent switch mutes the game and the music of other apps goes
// on. Returns 0 when the output does not start.
int bo_audio_start(BOMixer *mixer);

// The frames that the output rendered since the start.
long long bo_audio_rendered(void);

// Stops the output when the app goes to the background, and starts it
// again when the app comes back.
void bo_audio_set_active(int active);

#endif
