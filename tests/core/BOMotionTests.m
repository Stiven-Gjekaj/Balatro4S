// Tests of the motion of a card. The fan values come from the hand rule of
// CardArea:align_cards in Balatro 1.0.1o.

#import "BOTest.h"
#include <math.h>
#include "bo_motion.h"

static void run_frames(BOMotion *m, float tx, float ty, float tr, int frames, double *now) {
    for (int i = 0; i < frames; i++) {
        *now += 1.0 / 60;
        bo_motion_step(m, tx, ty, tr, 1, 1.0 / 60, *now);
    }
}

BO_TEST(test_motion_arrives_at_the_target_and_stops) {
    BOMotion m;
    bo_motion_place(&m, 0, 0, 0);
    double now = 0;
    run_frames(&m, 5, -2, 0.1f, 120, &now);
    BO_EXPECT(m.x == 5);
    BO_EXPECT(m.y == -2);
    BO_EXPECT(m.r == 0.1f);
    BO_EXPECT(m.vx == 0);
    BO_EXPECT(m.vr == 0);
}

BO_TEST(test_motion_leans_in_the_direction_that_it_moves) {
    BOMotion m;
    bo_motion_place(&m, 0, 0, 0);
    double now = 0;
    run_frames(&m, 5, 0, 0, 3, &now);
    BO_EXPECT(m.vx > 0);
    BO_EXPECT(m.r > 0.05f);
    bo_motion_place(&m, 0, 0, 0);
    run_frames(&m, -5, 0, 0, 3, &now);
    BO_EXPECT(m.r < -0.05f);
}

BO_TEST(test_motion_speed_has_a_limit) {
    BOMotion m;
    bo_motion_place(&m, 0, 0, 0);
    double now = 1.0 / 60;
    bo_motion_step(&m, 1000, 0, 0, 1, 1.0 / 60, now);
    // max_vel is 70 units each second.
    BO_EXPECT(fabsf(m.vx - 70.0f / 60) < 1e-4f);
}

BO_TEST(test_motion_juice_shakes_and_then_stops) {
    BOMotion m;
    bo_motion_place(&m, 1, 1, 0);
    double now = 10;
    bo_motion_juice(&m, 0.4f, 0.24f, now);
    BO_EXPECT(fabsf(m.scale - 0.76f) < 1e-6f);
    float maxR = 0;
    for (int i = 0; i < 24; i++) {
        now += 1.0 / 60;
        bo_motion_step(&m, 1, 1, 0, 1, 1.0 / 60, now);
        if (fabsf(m.r) > maxR) maxR = fabsf(m.r);
    }
    BO_EXPECT(maxR > 0.1f);
    BO_EXPECT(!m.juicing);
    run_frames(&m, 1, 1, 0, 60, &now);
    BO_EXPECT(fabsf(m.scale - 1) < 1e-3f);
    BO_EXPECT(m.r == 0);
}

BO_TEST(test_motion_fan_of_five_cards) {
    // card.T.r = 0.2*(-#cards/2 - 0.5 + k)/#cards with k from 1.
    BO_EXPECT(fabsf(bo_motion_fan_angle(0, 5, 0.2f) + 0.08f) < 1e-6f);
    BO_EXPECT(fabsf(bo_motion_fan_angle(2, 5, 0.2f)) < 1e-6f);
    BO_EXPECT(fabsf(bo_motion_fan_angle(4, 5, 0.2f) - 0.08f) < 1e-6f);
    // The middle card is 0.2 units higher than the cards at the two ends.
    BO_EXPECT(fabsf(bo_motion_fan_drop(0, 5)) < 1e-6f);
    BO_EXPECT(fabsf(bo_motion_fan_drop(2, 5) + 0.2f) < 1e-6f);
    BO_EXPECT(fabsf(bo_motion_fan_drop(4, 5)) < 1e-6f);
}
