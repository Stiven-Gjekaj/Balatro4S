#include "bo_mixer.h"

#include <math.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>

// The decoder is third party code. Its warnings are not ours to fix.
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Weverything"
#define STB_VORBIS_NO_PUSHDATA_API
#define STB_VORBIS_NO_STDIO
#include "../third_party/stb_vorbis.c"
#pragma clang diagnostic pop

#define BO_MIXER_SOUNDS 128
#define BO_MIXER_VOICES 48
#define BO_STREAM_FRAMES 2048

// The pitch and the volume of each music track, as RESTART_MUSIC sets them.
#define BO_MUSIC_PER 0.7f
#define BO_MUSIC_VOL 0.6f

// A sound that the mixer decodes while it plays. buffer holds the frames
// from first to next, and the decoder is at next.
typedef struct {
    stb_vorbis *vorbis;
    unsigned char *ogg;
    int first, count, next;
    int16_t buffer[BO_STREAM_FRAMES * 2];
} BOStream;

typedef struct {
    char code[32];
    int16_t *samples;
    int channels;
    int frames;
    int rate;
    int music;
    BOStream *stream;
} BOSound;

typedef struct {
    int active;
    int sound;
    double pos;
    float per;
    float vol;      // original_volume.
    float current;  // current_volume of a music track.
} BOVoice;

struct BOMixer {
    pthread_mutex_t lock;
    int rate;
    BOSound sounds[BO_MIXER_SOUNDS];
    int sound_count;
    BOVoice voices[BO_MIXER_VOICES];
    float volume, music, effects;
    float pitch_mod;
    char desired[16];
    float *mix;
    int mix_frames;
};

BOMixer *bo_mixer_new(int rate) {
    BOMixer *mixer = calloc(1, sizeof(BOMixer));
    pthread_mutex_init(&mixer->lock, NULL);
    mixer->rate = rate;
    mixer->volume = 0.5f;
    mixer->music = 1;
    mixer->effects = 1;
    mixer->pitch_mod = 1;
    return mixer;
}

void bo_mixer_free(BOMixer *mixer) {
    if (!mixer) return;
    for (int i = 0; i < mixer->sound_count; i++) {
        BOSound *s = &mixer->sounds[i];
        free(s->samples);
        if (s->stream) {
            stb_vorbis_close(s->stream->vorbis);
            free(s->stream->ogg);
            free(s->stream);
        }
    }
    free(mixer->mix);
    pthread_mutex_destroy(&mixer->lock);
    free(mixer);
}

static int find_sound(BOMixer *mixer, const char *code) {
    for (int i = 0; i < mixer->sound_count; i++) {
        if (strcmp(mixer->sounds[i].code, code) == 0) return i;
    }
    return -1;
}

static int add_sound(BOMixer *mixer, const BOSound *sound) {
    pthread_mutex_lock(&mixer->lock);
    int ok = mixer->sound_count < BO_MIXER_SOUNDS && find_sound(mixer, sound->code) < 0;
    if (ok) mixer->sounds[mixer->sound_count++] = *sound;
    pthread_mutex_unlock(&mixer->lock);
    return ok;
}

int bo_mixer_add_samples(BOMixer *mixer, const char *code, int16_t *samples, int frames, int channels, int rate) {
    BOSound s = {{0}};
    strncpy(s.code, code, sizeof(s.code) - 1);
    s.samples = samples;
    s.frames = frames;
    s.channels = channels == 1 ? 1 : 2;
    s.rate = rate;
    s.music = strstr(code, "music") != NULL;
    return add_sound(mixer, &s);
}

int bo_mixer_add_vorbis(BOMixer *mixer, const char *code, unsigned char *ogg, size_t size, int stream) {
    if (!stream) {
        int channels = 0, rate = 0;
        short *samples = NULL;
        int frames = stb_vorbis_decode_memory(ogg, (int)size, &channels, &rate, &samples);
        if (frames <= 0 || !samples) return 0;
        if (!bo_mixer_add_samples(mixer, code, samples, frames, channels, rate)) {
            free(samples);
            return 0;
        }
        free(ogg);
        return 1;
    }
    int error = 0;
    stb_vorbis *vorbis = stb_vorbis_open_memory(ogg, (int)size, &error, NULL);
    if (!vorbis) return 0;
    stb_vorbis_info info = stb_vorbis_get_info(vorbis);
    BOSound s = {{0}};
    strncpy(s.code, code, sizeof(s.code) - 1);
    s.channels = 2;
    s.frames = (int)stb_vorbis_stream_length_in_samples(vorbis);
    s.rate = (int)info.sample_rate;
    s.music = strstr(code, "music") != NULL;
    s.stream = calloc(1, sizeof(BOStream));
    s.stream->vorbis = vorbis;
    s.stream->ogg = ogg;
    if (!add_sound(mixer, &s)) {
        stb_vorbis_close(vorbis);
        free(s.stream);
        return 0;
    }
    return 1;
}

int bo_mixer_has(BOMixer *mixer, const char *code) {
    pthread_mutex_lock(&mixer->lock);
    int has = find_sound(mixer, code) >= 0;
    pthread_mutex_unlock(&mixer->lock);
    return has;
}

void bo_mixer_set_volumes(BOMixer *mixer, float volume, float music, float sounds) {
    pthread_mutex_lock(&mixer->lock);
    mixer->volume = volume;
    mixer->music = music;
    mixer->effects = sounds;
    pthread_mutex_unlock(&mixer->lock);
}

// Starts a voice of the sound. A stream has one voice: a new play stops
// the old voice and starts at the start again.
static BOVoice *start_voice(BOMixer *mixer, int sound, float per, float vol) {
    BOSound *s = &mixer->sounds[sound];
    BOVoice *free_voice = NULL;
    for (int i = 0; i < BO_MIXER_VOICES; i++) {
        BOVoice *v = &mixer->voices[i];
        if (s->stream && v->active && v->sound == sound) {
            free_voice = v;
            break;
        }
        if (!v->active && !free_voice) free_voice = v;
    }
    if (!free_voice) return NULL;
    BOVoice voice = {1, sound, 0, per, vol, 0};
    *free_voice = voice;
    return free_voice;
}

void bo_mixer_play(BOMixer *mixer, const char *code, float per, float vol) {
    pthread_mutex_lock(&mixer->lock);
    int sound = find_sound(mixer, code);
    if (sound >= 0 && vol * mixer->volume * mixer->effects > 0) start_voice(mixer, sound, per, vol);
    pthread_mutex_unlock(&mixer->lock);
}

static BOVoice *find_voice(BOMixer *mixer, int sound) {
    for (int i = 0; i < BO_MIXER_VOICES; i++) {
        if (mixer->voices[i].active && mixer->voices[i].sound == sound) return &mixer->voices[i];
    }
    return NULL;
}

void bo_mixer_ambient(BOMixer *mixer, const char *code, float vol, float per) {
    pthread_mutex_lock(&mixer->lock);
    int sound = find_sound(mixer, code);
    if (sound >= 0) {
        BOVoice *voice = find_voice(mixer, sound);
        int audible = vol * mixer->volume * mixer->effects > 0;
        if (voice && !audible) {
            voice->active = 0;
        } else if (voice) {
            voice->vol = vol;
        } else if (audible) {
            start_voice(mixer, sound, per, vol);
        }
    }
    pthread_mutex_unlock(&mixer->lock);
}

void bo_mixer_modulate(BOMixer *mixer, float dt, const char *desired, float pitch_mod) {
    pthread_mutex_lock(&mixer->lock);
    mixer->pitch_mod = pitch_mod;
    strncpy(mixer->desired, desired ? desired : "", sizeof(mixer->desired) - 1);
    float t = fminf(1, dt * 3);
    int restart = 0;
    for (int i = 0; i < mixer->sound_count; i++) {
        if (!mixer->sounds[i].music) continue;
        BOVoice *voice = find_voice(mixer, i);
        if (!voice) {
            restart = mixer->desired[0] != 0;
            continue;
        }
        float target = strcmp(mixer->sounds[i].code, mixer->desired) == 0 ? 1 : 0;
        voice->current = target * t + (1 - t) * voice->current;
    }
    if (restart) {
        // RESTART_MUSIC: all the tracks start together. The new source of
        // the track desired starts at full volume, the others at 0.
        for (int i = 0; i < mixer->sound_count; i++) {
            if (!mixer->sounds[i].music) continue;
            BOVoice *voice = start_voice(mixer, i, BO_MUSIC_PER, BO_MUSIC_VOL);
            if (voice) voice->current = strcmp(mixer->sounds[i].code, mixer->desired) == 0 ? 1 : 0;
        }
    }
    pthread_mutex_unlock(&mixer->lock);
}

void bo_mixer_stop(BOMixer *mixer) {
    pthread_mutex_lock(&mixer->lock);
    for (int i = 0; i < BO_MIXER_VOICES; i++) mixer->voices[i].active = 0;
    pthread_mutex_unlock(&mixer->lock);
}

int bo_mixer_voices(BOMixer *mixer) {
    pthread_mutex_lock(&mixer->lock);
    int count = 0;
    for (int i = 0; i < BO_MIXER_VOICES; i++) count += mixer->voices[i].active;
    pthread_mutex_unlock(&mixer->lock);
    return count;
}

// Puts the stereo frame index of the stream into out. Returns 0 at the
// end of the stream. A jump back, or far forward, seeks the decoder.
static int stream_frame(BOSound *s, int index, int16_t out[2]) {
    BOStream *st = s->stream;
    if (index >= s->frames) return 0;
    if (index < st->first || index >= st->next + BO_STREAM_FRAMES * 4) {
        if (!stb_vorbis_seek_frame(st->vorbis, (unsigned)index)) return 0;
        st->first = st->next = index;
        st->count = 0;
    }
    while (index >= st->first + st->count) {
        int got = stb_vorbis_get_samples_short_interleaved(st->vorbis, 2, st->buffer, BO_STREAM_FRAMES * 2);
        if (got <= 0) {
            s->frames = st->next;
            return 0;
        }
        st->first = st->next;
        st->count = got;
        st->next += got;
    }
    out[0] = st->buffer[(index - st->first) * 2];
    out[1] = st->buffer[(index - st->first) * 2 + 1];
    return 1;
}

static int sound_frame(BOSound *s, int index, int16_t out[2]) {
    if (s->stream) return stream_frame(s, index, out);
    if (index >= s->frames) return 0;
    if (s->channels == 1) {
        out[0] = out[1] = s->samples[index];
    } else {
        out[0] = s->samples[index * 2];
        out[1] = s->samples[index * 2 + 1];
    }
    return 1;
}

static void render_voice(BOMixer *mixer, BOVoice *voice, float *mix, int frames) {
    BOSound *s = &mixer->sounds[voice->sound];
    float gain = s->music ? voice->current * voice->vol * mixer->volume * mixer->music
                          : voice->vol * mixer->volume * mixer->effects;
    float pitch = s->music ? voice->per * mixer->pitch_mod : voice->per;
    double step = (double)s->rate / mixer->rate * pitch;
    if (gain * 32768 < 0.5f) {
        // A voice that nobody can hear moves on and decodes nothing.
        voice->pos += step * frames;
        if (voice->pos >= s->frames) voice->active = 0;
        return;
    }
    for (int f = 0; f < frames; f++) {
        int index = (int)voice->pos;
        int16_t a[2], b[2];
        if (!sound_frame(s, index, a)) {
            voice->active = 0;
            return;
        }
        if (!sound_frame(s, index + 1, b)) memcpy(b, a, sizeof(b));
        float frac = (float)(voice->pos - index);
        mix[f * 2] += gain * (a[0] + (b[0] - a[0]) * frac);
        mix[f * 2 + 1] += gain * (a[1] + (b[1] - a[1]) * frac);
        voice->pos += step;
    }
}

void bo_mixer_render(BOMixer *mixer, int16_t *out, int frames) {
    pthread_mutex_lock(&mixer->lock);
    if (mixer->mix_frames < frames) {
        free(mixer->mix);
        mixer->mix = malloc(sizeof(float) * 2 * frames);
        mixer->mix_frames = frames;
    }
    float *mix = mixer->mix;
    memset(mix, 0, sizeof(float) * 2 * frames);
    for (int i = 0; i < BO_MIXER_VOICES; i++) {
        if (mixer->voices[i].active) render_voice(mixer, &mixer->voices[i], mix, frames);
    }
    pthread_mutex_unlock(&mixer->lock);
    for (int i = 0; i < frames * 2; i++) {
        float v = mix[i];
        out[i] = (int16_t)(v > 32767 ? 32767 : v < -32768 ? -32768 : lrintf(v));
    }
}
