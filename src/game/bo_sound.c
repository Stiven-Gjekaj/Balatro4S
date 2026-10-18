#include "bo_sound.h"

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include "bo_audio.h"
#include "bo_mixer.h"
#include "bo_zip.h"

static BOMixer *gMixer;
static char gLovePath[1024];

static double now_seconds(void) {
    struct timeval now;
    gettimeofday(&now, NULL);
    return now.tv_sec + now.tv_usec / 1e6;
}

// Balatro streams the music and the ambient sounds. The two long sounds of
// the splash screen stream too, so that their samples use no memory.
static int streams(const char *code) {
    return strstr(code, "music") || strstr(code, "ambient") || strcmp(code, "introPad1") == 0 ||
           strcmp(code, "splash_buildup") == 0 || strcmp(code, "whoosh_long") == 0;
}

static void load_pass(BOZip *zip, int streamPass, int *loaded) {
    const char *prefix = "resources/sounds/";
    size_t prefixLength = strlen(prefix);
    for (int i = 0; i < bo_zip_count(zip); i++) {
        const char *name = bo_zip_name(zip, i);
        size_t length = strlen(name);
        if (strncmp(name, prefix, prefixLength) != 0 || length < prefixLength + 5 ||
            strcmp(name + length - 4, ".ogg") != 0) {
            continue;
        }
        char code[32];
        snprintf(code, sizeof(code), "%.*s", (int)(length - prefixLength - 4), name + prefixLength);
        if (streams(code) != streamPass) continue;
        size_t size = 0;
        unsigned char *ogg = bo_zip_read(zip, name, &size);
        if (!ogg) continue;
        if (bo_mixer_add_vorbis(gMixer, code, ogg, size, streamPass)) {
            (*loaded)++;
        } else {
            free(ogg);
        }
    }
}

static void *load_sounds(void *unused) {
    double start = now_seconds();
    BOZip *zip = bo_zip_open(gLovePath);
    if (!zip) return NULL;
    int loaded = 0;
    // The music first: a stream only reads its header.
    load_pass(zip, 1, &loaded);
    load_pass(zip, 0, &loaded);
    bo_zip_close(zip);
    fprintf(stderr, "sound: %d sounds in %.2f s\n", loaded, now_seconds() - start);
    return NULL;
}

void bo_sound_start(const char *love_path) {
    if (gMixer) return;
    gMixer = bo_mixer_new(BO_AUDIO_RATE);
    snprintf(gLovePath, sizeof(gLovePath), "%s", love_path);
    if (!bo_audio_start(gMixer)) fprintf(stderr, "sound: no output\n");
    pthread_t thread;
    if (pthread_create(&thread, NULL, load_sounds, NULL) == 0) pthread_detach(thread);
}

void bo_sound_play(const char *code, float per, float vol) {
    if (gMixer) bo_mixer_play(gMixer, code, per, vol);
}

void bo_sound_modulate(float dt, const char *desired, float pitch_mod) {
    if (gMixer) bo_mixer_modulate(gMixer, dt, desired, pitch_mod);
}

void bo_sound_ambient(const char *code, float vol, float per) {
    if (gMixer) bo_mixer_ambient(gMixer, code, vol, per);
}

void bo_sound_set_volumes(int volume, int music, int sounds) {
    if (gMixer) bo_mixer_set_volumes(gMixer, volume / 100.0f, music / 100.0f, sounds / 100.0f);
}

int bo_sound_voices(void) {
    return gMixer ? bo_mixer_voices(gMixer) : 0;
}

long long bo_sound_rendered(void) {
    return bo_audio_rendered();
}

void bo_sound_set_active(int active) {
    bo_audio_set_active(active);
}
