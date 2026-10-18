#include "bo_motion.h"

#include <math.h>
#include <string.h>

void bo_motion_place(BOMotion *m, float x, float y, float r) {
    memset(m, 0, sizeof(*m));
    m->x = x;
    m->y = y;
    m->r = r;
    m->scale = 1;
}

// move_juice: the shake gets smaller to 0 at end_time.
static void move_juice(BOMotion *m, double now) {
    if (!m->juicing) return;
    if (m->juice_end < now) {
        m->juicing = 0;
        m->juice_scale = m->juice_r = 0;
        return;
    }
    double t = now - m->juice_start;
    double left = (m->juice_end - now) / (m->juice_end - m->juice_start);
    if (left < 0) left = 0;
    m->juice_scale = (float)(m->juice_scale_amt * sin(50.8 * t) * left * left * left);
    m->juice_r = (float)(m->juice_r_amt * sin(40.8 * t) * left * left);
}

void bo_motion_step(BOMotion *m, float tx, float ty, float tr, float tscale, double dt, double now) {
    if (dt <= 0) return;
    // The constants of Game:update.
    float expXY = (float)exp(-50 * dt), expScale = (float)exp(-60 * dt), expR = (float)exp(-190 * dt);
    float maxVel = (float)(70 * dt);
    move_juice(m, now);
    // move_xy
    m->vx = expXY * m->vx + (1 - expXY) * (tx - m->x) * 35 * (float)dt;
    m->vy = expXY * m->vy + (1 - expXY) * (ty - m->y) * 35 * (float)dt;
    float speed2 = m->vx * m->vx + m->vy * m->vy;
    if (speed2 > maxVel * maxVel) {
        float speed = sqrtf(speed2);
        m->vx = maxVel * m->vx / speed;
        m->vy = maxVel * m->vy / speed;
    }
    m->x += m->vx;
    m->y += m->vy;
    if (fabsf(m->x - tx) < 0.01f && fabsf(m->vx) < 0.01f) m->x = tx, m->vx = 0;
    if (fabsf(m->y - ty) < 0.01f && fabsf(m->vy) < 0.01f) m->y = ty, m->vy = 0;
    // move_r: the card leans in the direction that it moves.
    float desR = tr + 0.015f * m->vx / (float)dt + m->juice_r * 2;
    m->vr = expR * m->vr + (1 - expR) * (desR - m->r);
    m->r += m->vr;
    if (fabsf(m->r - tr) < 0.001f && fabsf(m->vr) < 0.001f) m->r = tr, m->vr = 0;
    // move_scale
    float desScale = tscale + m->juice_scale;
    m->vscale = expScale * m->vscale + (1 - expScale) * (desScale - m->scale);
    m->scale += m->vscale;
}

void bo_motion_juice(BOMotion *m, float amount, float rot_amt, double now) {
    m->juicing = 1;
    m->juice_scale = 0;
    m->juice_r = 0;
    m->juice_scale_amt = amount;
    m->juice_r_amt = rot_amt;
    m->juice_start = now;
    m->juice_end = now + 0.4;
    m->scale = 1 - 0.6f * amount;
}

float bo_motion_fan_angle(int k, int count, float spread) {
    if (count <= 0) return 0;
    return spread * (-count / 2.0f - 0.5f + (k + 1)) / count;
}

float bo_motion_fan_drop(int k, int count) {
    if (count <= 0) return 0;
    return fabsf(0.5f * (-count / 2.0f + (k + 1) - 0.5f) / count) - 0.2f;
}
