// Tests of the mixer: the pitch and the volume of a voice, and the fade
// of the music tracks, as engine/sound_manager.lua of Balatro 1.0.1o.

#import "BOTest.h"
#include <stdlib.h>
#include "bo_mixer.h"

// A sound of frames stereo frames: the left value is the frame index, and
// the right value is the negative index.
static int16_t *ramp(int frames) {
    int16_t *samples = malloc(sizeof(int16_t) * 2 * frames);
    for (int i = 0; i < frames; i++) {
        samples[i * 2] = (int16_t)i;
        samples[i * 2 + 1] = (int16_t)-i;
    }
    return samples;
}

static int16_t *constant(int frames, int16_t value) {
    int16_t *samples = malloc(sizeof(int16_t) * 2 * frames);
    for (int i = 0; i < frames * 2; i++) samples[i] = value;
    return samples;
}

BO_TEST(test_mixer_plays_a_sound_at_full_volume) {
    BOMixer *m = bo_mixer_new(100);
    bo_mixer_set_volumes(m, 1, 1, 1);
    BO_EXPECT(bo_mixer_add_samples(m, "card1", ramp(10), 10, 2, 100));
    bo_mixer_play(m, "card1", 1, 1);
    BO_EQUAL_INT(bo_mixer_voices(m), 1);
    int16_t out[24];
    bo_mixer_render(m, out, 12);
    for (int i = 0; i < 10; i++) {
        BO_EQUAL_INT(out[i * 2], i);
        BO_EQUAL_INT(out[i * 2 + 1], -i);
    }
    BO_EQUAL_INT(out[20], 0);
    BO_EQUAL_INT(bo_mixer_voices(m), 0);
    bo_mixer_free(m);
}

BO_TEST(test_mixer_pitch_changes_the_speed_and_volume_the_level) {
    BOMixer *m = bo_mixer_new(100);
    // The default volume is 0.5, as SOUND.volume 50.
    bo_mixer_add_samples(m, "chips1", ramp(100), 100, 2, 100);
    bo_mixer_play(m, "chips1", 2, 0.5f);
    int16_t out[20];
    bo_mixer_render(m, out, 10);
    // Pitch 2 reads each second frame. The gain is 0.5 * 0.5.
    BO_EQUAL_INT(out[2 * 2], 1);
    BO_EQUAL_INT(out[8 * 2], 4);
    // Pitch 0.5 reads each frame two times, with the value between them.
    bo_mixer_stop(m);
    bo_mixer_set_volumes(m, 1, 1, 1);
    bo_mixer_play(m, "chips1", 0.5f, 1);
    bo_mixer_render(m, out, 10);
    BO_EQUAL_INT(out[6 * 2], 3);
    BO_EQUAL_INT(out[7 * 2], 4);  // 3.5, rounded to even.
    bo_mixer_free(m);
}

BO_TEST(test_mixer_plays_mono_in_both_channels_and_clamps_the_sum) {
    BOMixer *m = bo_mixer_new(100);
    bo_mixer_set_volumes(m, 1, 1, 1);
    int16_t *mono = malloc(sizeof(int16_t) * 4);
    for (int i = 0; i < 4; i++) mono[i] = 30000;
    bo_mixer_add_samples(m, "button", mono, 4, 1, 100);
    bo_mixer_play(m, "button", 1, 1);
    bo_mixer_play(m, "button", 1, 1);
    BO_EQUAL_INT(bo_mixer_voices(m), 2);
    int16_t out[8];
    bo_mixer_render(m, out, 4);
    BO_EQUAL_INT(out[0], 32767);
    BO_EQUAL_INT(out[1], 32767);
    bo_mixer_free(m);
}

BO_TEST(test_mixer_ignores_an_unknown_sound_and_bad_bytes) {
    BOMixer *m = bo_mixer_new(100);
    bo_mixer_play(m, "nothing", 1, 1);
    BO_EQUAL_INT(bo_mixer_voices(m), 0);
    unsigned char junk[64] = {1, 2, 3};
    BO_EXPECT(!bo_mixer_add_vorbis(m, "junk", junk, sizeof(junk), 0));
    BO_EXPECT(!bo_mixer_add_vorbis(m, "junk", junk, sizeof(junk), 1));
    BO_EXPECT(!bo_mixer_has(m, "junk"));
    // The volume of the sounds at 0 plays nothing.
    bo_mixer_add_samples(m, "card1", ramp(10), 10, 2, 100);
    bo_mixer_set_volumes(m, 1, 1, 0);
    bo_mixer_play(m, "card1", 1, 1);
    BO_EQUAL_INT(bo_mixer_voices(m), 0);
    bo_mixer_free(m);
}

BO_TEST(test_mixer_starts_all_tracks_and_fades_to_the_desired_one) {
    BOMixer *m = bo_mixer_new(1000);
    bo_mixer_set_volumes(m, 1, 1, 1);
    bo_mixer_add_samples(m, "music1", constant(100000, 1000), 100000, 2, 1000);
    bo_mixer_add_samples(m, "music2", constant(100000, 10000), 100000, 2, 1000);
    // No track is desired: no track starts.
    bo_mixer_modulate(m, 0.1f, "", 1);
    BO_EQUAL_INT(bo_mixer_voices(m), 0);
    bo_mixer_modulate(m, 0.1f, "music1", 1);
    BO_EQUAL_INT(bo_mixer_voices(m), 2);
    int16_t out[20];
    bo_mixer_render(m, out, 10);
    // Only music1 sounds, at the volume 0.6 of RESTART_MUSIC.
    BO_EQUAL_INT(out[0], 600);
    // Each modulate moves the volume by dt * 3 to the desired track.
    bo_mixer_modulate(m, 0.1f, "music2", 1);
    bo_mixer_render(m, out, 10);
    BO_EQUAL_INT(out[0], 420 + 1800);
    for (int i = 0; i < 40; i++) bo_mixer_modulate(m, 0.1f, "music2", 1);
    bo_mixer_render(m, out, 10);
    BO_EQUAL_INT(out[0], 6000);
    bo_mixer_free(m);
}

BO_TEST(test_mixer_plays_music_at_pitch_0_7_and_restarts_it_at_the_end) {
    BOMixer *m = bo_mixer_new(100);
    bo_mixer_set_volumes(m, 1, 1, 1);
    bo_mixer_add_samples(m, "music1", ramp(70), 70, 2, 100);
    bo_mixer_modulate(m, 0.01f, "music1", 1);
    int16_t out[200];
    bo_mixer_render(m, out, 100);
    // Frame 10 reads the sound at 7, with the gain 0.6.
    BO_EQUAL_INT(out[10 * 2], 4);  // 4.2.
    BO_EQUAL_INT(bo_mixer_voices(m), 0);
    bo_mixer_modulate(m, 0.01f, "music1", 1);
    BO_EQUAL_INT(bo_mixer_voices(m), 1);
    bo_mixer_free(m);
}

BO_TEST(test_mixer_ambient_starts_changes_and_stops_a_sound) {
    BOMixer *m = bo_mixer_new(100);
    bo_mixer_set_volumes(m, 1, 1, 1);
    bo_mixer_add_samples(m, "ambientOrgan1", constant(1000, 1000), 1000, 2, 100);
    bo_mixer_ambient(m, "ambientOrgan1", 0, 0.7f);
    BO_EQUAL_INT(bo_mixer_voices(m), 0);
    bo_mixer_ambient(m, "ambientOrgan1", 0.5f, 0.7f);
    bo_mixer_ambient(m, "ambientOrgan1", 0.25f, 0.7f);
    BO_EQUAL_INT(bo_mixer_voices(m), 1);
    int16_t out[4];
    bo_mixer_render(m, out, 2);
    BO_EQUAL_INT(out[0], 250);
    bo_mixer_ambient(m, "ambientOrgan1", 0, 0.7f);
    BO_EQUAL_INT(bo_mixer_voices(m), 0);
    bo_mixer_free(m);
}
