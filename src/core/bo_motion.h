// bo_motion: the motion of a card on the screen, as Moveable:move_xy,
// move_r, move_scale, juice_up, and move_juice of Balatro 1.0.1o.
//
// The values are in the units of the game: a card is 2.4*35/41 units wide.
// The caller changes them to pixels.

#ifndef BO_MOTION_H
#define BO_MOTION_H

// The width of a card in the units of the game: G.CARD_W.
#define BO_MOTION_CARD_W (2.4 * 35 / 41)

typedef struct {
    // VT: the place that shows. T: the place that the card goes to.
    float x, y, r, scale;
    float vx, vy, vr, vscale;
    // The juice: a short shake of the size and the angle.
    int juicing;
    float juice_scale, juice_r;
    float juice_scale_amt, juice_r_amt;
    double juice_start, juice_end;
} BOMotion;

// The card shows at its target with no motion.
void bo_motion_place(BOMotion *m, float x, float y, float r);

// Moves the card one frame of dt seconds to the target. now is the time of
// the game, as G.TIMERS.REAL.
void bo_motion_step(BOMotion *m, float tx, float ty, float tr, float tscale, double dt, double now);

// juice_up(amount, rot_amt): the card shakes for 0.4 seconds.
void bo_motion_juice(BOMotion *m, float amount, float rot_amt, double now);

// align_cards of a hand: the angle and the drop of card k (0 first) of
// count cards. The sway that moves with the time is not in these values.
float bo_motion_fan_angle(int k, int count, float spread);
float bo_motion_fan_drop(int k, int count);

#endif
