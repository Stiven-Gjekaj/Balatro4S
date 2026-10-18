// bo_blind: the blinds of Balatro 1.0.1o (P_BLINDS in game.lua), the chip
// targets of each ante, and the choice of the boss.

#ifndef BO_BLIND_H
#define BO_BLIND_H

#include "bo_random.h"

// The blinds in the alphabetical order of their keys. pseudorandom_element
// sorts the keys in this order before it picks one.
typedef enum {
    BO_BL_ARM,
    BO_BL_BIG,
    BO_BL_CLUB,
    BO_BL_EYE,
    BO_BL_FINAL_ACORN,
    BO_BL_FINAL_BELL,
    BO_BL_FINAL_HEART,
    BO_BL_FINAL_LEAF,
    BO_BL_FINAL_VESSEL,
    BO_BL_FISH,
    BO_BL_FLINT,
    BO_BL_GOAD,
    BO_BL_HEAD,
    BO_BL_HOOK,
    BO_BL_HOUSE,
    BO_BL_MANACLE,
    BO_BL_MARK,
    BO_BL_MOUTH,
    BO_BL_NEEDLE,
    BO_BL_OX,
    BO_BL_PILLAR,
    BO_BL_PLANT,
    BO_BL_PSYCHIC,
    BO_BL_SERPENT,
    BO_BL_SMALL,
    BO_BL_TOOTH,
    BO_BL_WALL,
    BO_BL_WATER,
    BO_BL_WHEEL,
    BO_BL_WINDOW,
    BO_BLIND_COUNT,
} BOBlindId;

typedef struct {
    const char *key;
    const char *name;
    int order;        // The place in the collection.
    int dollars;      // The reward.
    double mult;      // The chip target is this times the amount of the ante.
    int sprite_row;   // The row in BlindChips.png.
    int boss;         // 1 for a boss blind.
    int min_ante;     // The first ante where the boss can come.
    int showdown;     // 1 for a final boss, which comes only at ante 8, 16, ...
    unsigned int colour;
} BOBlindInfo;

const BOBlindInfo *bo_blind_info(BOBlindId id);

// get_blind_amount: the base chip target of an ante. scaling is 1, 2, or 3
// (from the stakes).
double bo_blind_amount(int ante, int scaling);

// get_new_boss: picks the boss of an ante from the bosses that were used the
// fewest times. used counts each boss, and the function adds the pick to it.
// banned marks the blinds that a challenge bans, or is NULL.
BOBlindId bo_blind_new_boss(BORandom *random, int ante, int win_ante, int used[BO_BLIND_COUNT],
                            const unsigned char banned[BO_BLIND_COUNT]);

#endif
