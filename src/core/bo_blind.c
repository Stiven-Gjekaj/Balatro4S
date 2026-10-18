#include "bo_blind.h"

#include <math.h>
#include <stddef.h>

#pragma STDC FP_CONTRACT OFF

static const BOBlindInfo kBlinds[BO_BLIND_COUNT] = {
    [BO_BL_ARM] = {"bl_arm", "The Arm", 8, 5, 2, 11, 1, 2, 0, 0x6865f3},
    [BO_BL_BIG] = {"bl_big", "Big Blind", 2, 4, 1.5, 1, 0, 0, 0, 0x50846e},
    [BO_BL_CLUB] = {"bl_club", "The Club", 9, 5, 2, 4, 1, 1, 0, 0xb9cb92},
    [BO_BL_EYE] = {"bl_eye", "The Eye", 16, 5, 2, 17, 1, 3, 0, 0x4b71e4},
    [BO_BL_FINAL_ACORN] = {"bl_final_acorn", "Amber Acorn", 26, 8, 2, 27, 1, 10, 1, 0xfda200},
    [BO_BL_FINAL_BELL] = {"bl_final_bell", "Cerulean Bell", 30, 8, 2, 26, 1, 10, 1, 0x009cfd},
    [BO_BL_FINAL_HEART] = {"bl_final_heart", "Crimson Heart", 29, 8, 2, 25, 1, 10, 1, 0xac3232},
    [BO_BL_FINAL_LEAF] = {"bl_final_leaf", "Verdant Leaf", 27, 8, 2, 28, 1, 10, 1, 0x56a786},
    [BO_BL_FINAL_VESSEL] = {"bl_final_vessel", "Violet Vessel", 28, 8, 6, 29, 1, 10, 1, 0x8a71e1},
    [BO_BL_FISH] = {"bl_fish", "The Fish", 10, 5, 2, 5, 1, 2, 0, 0x3e85bd},
    [BO_BL_FLINT] = {"bl_flint", "The Flint", 24, 5, 2, 24, 1, 2, 0, 0xe56a2f},
    [BO_BL_GOAD] = {"bl_goad", "The Goad", 12, 5, 2, 13, 1, 1, 0, 0xb95c96},
    [BO_BL_HEAD] = {"bl_head", "The Head", 22, 5, 2, 21, 1, 1, 0, 0xac9db4},
    [BO_BL_HOOK] = {"bl_hook", "The Hook", 3, 5, 2, 7, 1, 1, 0, 0xa84024},
    [BO_BL_HOUSE] = {"bl_house", "The House", 5, 5, 2, 3, 1, 2, 0, 0x5186a8},
    [BO_BL_MANACLE] = {"bl_manacle", "The Manacle", 15, 5, 2, 8, 1, 1, 0, 0x575757},
    [BO_BL_MARK] = {"bl_mark", "The Mark", 25, 5, 2, 23, 1, 2, 0, 0x6a3847},
    [BO_BL_MOUTH] = {"bl_mouth", "The Mouth", 17, 5, 2, 18, 1, 2, 0, 0xae718e},
    [BO_BL_NEEDLE] = {"bl_needle", "The Needle", 21, 5, 1, 20, 1, 2, 0, 0x5c6e31},
    [BO_BL_OX] = {"bl_ox", "The Ox", 4, 5, 2, 2, 1, 6, 0, 0xb95b08},
    [BO_BL_PILLAR] = {"bl_pillar", "The Pillar", 20, 5, 2, 16, 1, 1, 0, 0x7e6752},
    [BO_BL_PLANT] = {"bl_plant", "The Plant", 18, 5, 2, 19, 1, 4, 0, 0x709284},
    [BO_BL_PSYCHIC] = {"bl_psychic", "The Psychic", 11, 5, 2, 12, 1, 1, 0, 0xefc03c},
    [BO_BL_SERPENT] = {"bl_serpent", "The Serpent", 19, 5, 2, 15, 1, 5, 0, 0x439a4f},
    [BO_BL_SMALL] = {"bl_small", "Small Blind", 1, 3, 1, 0, 0, 0, 0, 0x50846e},
    [BO_BL_TOOTH] = {"bl_tooth", "The Tooth", 23, 5, 2, 22, 1, 3, 0, 0xb52d2d},
    [BO_BL_WALL] = {"bl_wall", "The Wall", 6, 5, 4, 9, 1, 2, 0, 0x8a59a5},
    [BO_BL_WATER] = {"bl_water", "The Water", 13, 5, 2, 14, 1, 2, 0, 0xc6e0eb},
    [BO_BL_WHEEL] = {"bl_wheel", "The Wheel", 7, 5, 2, 10, 1, 2, 0, 0x50bf7c},
    [BO_BL_WINDOW] = {"bl_window", "The Window", 14, 5, 2, 6, 1, 1, 0, 0xa9a295},
};

const BOBlindInfo *bo_blind_info(BOBlindId id) {
    if (id < 0 || id >= BO_BLIND_COUNT) return NULL;
    return &kBlinds[id];
}

double bo_blind_amount(int ante, int scaling) {
    static const double kAmounts[3][8] = {
        {300, 800, 2000, 5000, 11000, 20000, 35000, 50000},
        {300, 900, 2600, 8000, 20000, 36000, 60000, 100000},
        {300, 1000, 3200, 9000, 25000, 60000, 110000, 200000},
    };
    if (scaling < 1 || scaling > 3) scaling = 1;
    const double *amounts = kAmounts[scaling - 1];
    if (ante < 1) return 100;
    if (ante <= 8) return amounts[ante - 1];
    // The endless antes after 8.
    double k = 0.75, a = amounts[7], b = 1.6, c = ante - 8, d = 1 + 0.2 * (ante - 8);
    double amount = floor(a * pow(b + pow(k * c, d), c));
    double step = pow(10, floor(log10(amount) - 1));
    return amount - (amount - floor(amount / step) * step);
}

BOBlindId bo_blind_new_boss(BORandom *random, int ante, int win_ante, int used[BO_BLIND_COUNT],
                            const unsigned char banned[BO_BLIND_COUNT]) {
    int eligible[BO_BLIND_COUNT] = {0};
    int lowAnte = ante > 1 ? ante : 1;
    for (int i = 0; i < BO_BLIND_COUNT; i++) {
        const BOBlindInfo *b = &kBlinds[i];
        if (!b->boss || (banned && banned[i])) continue;
        if (!b->showdown) {
            eligible[i] = b->min_ante <= lowAnte && (lowAnte % win_ante != 0 || ante < 2);
        } else {
            eligible[i] = ante % win_ante == 0 && ante >= 2;
        }
    }
    int minUse = 100;
    for (int i = 0; i < BO_BLIND_COUNT; i++) {
        if (eligible[i] && used[i] <= minUse) minUse = used[i];
    }
    // The keys of the table are in alphabetical order, as the sort of
    // pseudorandom_element puts them.
    BOBlindId keys[BO_BLIND_COUNT];
    int count = 0;
    for (int i = 0; i < BO_BLIND_COUNT; i++) {
        if (eligible[i] && used[i] <= minUse) keys[count++] = (BOBlindId)i;
    }
    if (count == 0) return BO_BL_SMALL;
    BOBlindId boss = keys[bo_random_index(random, bo_pseudoseed(random, "boss"), count)];
    used[boss]++;
    return boss;
}
