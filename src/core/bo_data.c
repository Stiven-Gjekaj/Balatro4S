// tools/gen_data.lua makes this file from game.lua. Do not change it by hand.

#include "bo_data.h"

#include <stddef.h>
#include <string.h>

static const BOConfigValue k5[] = {{BO_F_EXTRA, 4, NULL}, };
static const BOConfigValue k11[] = {{BO_F_EXTRA, 50, NULL}, };
static const BOConfigValue k12[] = {{BO_F_EXTRA, 10, NULL}, };
static const BOConfigValue k13[] = {{BO_F_EXTRA, 1.5, NULL}, };
static const BOConfigValue k14[] = {{BO_F_EXTRA, 1, NULL}, };
static const BOConfigValue k16[] = {{BO_F_EXTRA_S_MULT, 3, NULL}, {BO_F_EXTRA_SUIT, 0, "Diamonds"}, };
static const BOConfigValue k17[] = {{BO_F_EXTRA_S_MULT, 3, NULL}, {BO_F_EXTRA_SUIT, 0, "Hearts"}, };
static const BOConfigValue k18[] = {{BO_F_EXTRA_S_MULT, 3, NULL}, {BO_F_EXTRA_SUIT, 0, "Spades"}, };
static const BOConfigValue k19[] = {{BO_F_EXTRA_S_MULT, 3, NULL}, {BO_F_EXTRA_SUIT, 0, "Clubs"}, };
static const BOConfigValue k30[] = {{BO_F_EXTRA_MULT, 20, NULL}, {BO_F_EXTRA_SIZE, 3, NULL}, };
static const BOConfigValue k33[] = {{BO_F_EXTRA, 1, NULL}, };
static const BOConfigValue k34[] = {{BO_F_EXTRA, 20, NULL}, };
static const BOConfigValue k36[] = {{BO_F_EXTRA, 30, NULL}, };
static const BOConfigValue k37[] = {{BO_F_EXTRA_D_REMAINING, 0, NULL}, {BO_F_EXTRA_MULT, 15, NULL}, };
static const BOConfigValue k38[] = {{BO_F_EXTRA, 1, NULL}, };
static const BOConfigValue k39[] = {{BO_F_EXTRA_XMULT, 4, NULL}, {BO_F_EXTRA_EVERY, 5, NULL}, {BO_F_EXTRA_REMAINING, 0, "5 remaining"}, };
static const BOConfigValue k40[] = {{BO_F_EXTRA, 4, NULL}, };
static const BOConfigValue k41[] = {{BO_F_EXTRA_MAX, 23, NULL}, {BO_F_EXTRA_MIN, 0, NULL}, };
static const BOConfigValue k42[] = {{BO_F_EXTRA, 1, NULL}, };
static const BOConfigValue k44[] = {{BO_F_EXTRA, 1, NULL}, };
static const BOConfigValue k45[] = {{BO_F_EXTRA, 8, NULL}, };
static const BOConfigValue k46[] = {{BO_F_EXTRA, 0.20000000000000001, NULL}, };
static const BOConfigValue k47[] = {{BO_F_EXTRA, 30, NULL}, };
static const BOConfigValue k48[] = {{BO_F_EXTRA, 3, NULL}, };
static const BOConfigValue k49[] = {{BO_F_EXTRA, 2, NULL}, };
static const BOConfigValue k50[] = {{BO_F_EXTRA, 1, NULL}, };
static const BOConfigValue k52[] = {{BO_F_EXTRA_MULT, 15, NULL}, {BO_F_EXTRA_ODDS, 6, NULL}, };
static const BOConfigValue k53[] = {{BO_F_EXTRA, 4, NULL}, };
static const BOConfigValue k54[] = {{BO_F_EXTRA, 31, NULL}, };
static const BOConfigValue k55[] = {{BO_F_EXTRA_CHIPS, 20, NULL}, {BO_F_EXTRA_MULT, 4, NULL}, };
static const BOConfigValue k56[] = {{BO_F_EXTRA, 2, NULL}, };
static const BOConfigValue k57[] = {{BO_F_EXTRA, 1, NULL}, };
static const BOConfigValue k58[] = {{BO_F_EXTRA, 1, NULL}, };
static const BOConfigValue k59[] = {{BO_F_EXTRA, 4, NULL}, };
static const BOConfigValue k60[] = {{BO_F_EXTRA, 3, NULL}, };
static const BOConfigValue k61[] = {{BO_F_EXTRA, 3, NULL}, };
static const BOConfigValue k62[] = {{BO_F_EXTRA, 3, NULL}, };
static const BOConfigValue k63[] = {{BO_F_EXTRA_CHIP_MOD, 15, NULL}, {BO_F_EXTRA_CHIPS, 0, NULL}, };
static const BOConfigValue k64[] = {{BO_F_EXTRA_CHIP_MOD, 5, NULL}, {BO_F_EXTRA_CHIPS, 100, NULL}, };
static const BOConfigValue k67[] = {{BO_F_EXTRA, 2, NULL}, };
static const BOConfigValue k69[] = {{BO_F_EXTRA, 0.10000000000000001, NULL}, };
static const BOConfigValue k70[] = {{BO_F_EXTRA, 5, NULL}, };
static const BOConfigValue k71[] = {{BO_F_EXTRA_DOLLARS, 5, NULL}, {BO_F_EXTRA_FACES, 3, NULL}, };
static const BOConfigValue k72[] = {{BO_F_EXTRA_DISCARD_SUB, 1, NULL}, {BO_F_EXTRA_HAND_ADD, 1, NULL}, };
static const BOConfigValue k74[] = {{BO_F_EXTRA_DOLLARS, 4, NULL}, {BO_F_EXTRA_POKER_HAND, 0, "High Card"}, };
static const BOConfigValue k75[] = {{BO_F_EXTRA_XMULT, 3, NULL}, {BO_F_EXTRA_ODDS, 1000, NULL}, };
static const BOConfigValue k76[] = {{BO_F_EXTRA_XMULT, 3, NULL}, };
static const BOConfigValue k77[] = {{BO_F_EXTRA, 3, NULL}, };
static const BOConfigValue k78[] = {{BO_F_EXTRA, 0.5, NULL}, };
static const BOConfigValue k79[] = {{BO_F_EXTRA_CHIP_MOD, 4, NULL}, {BO_F_EXTRA_CHIPS, 0, NULL}, };
static const BOConfigValue k80[] = {{BO_F_EXTRA_POKER_HAND, 0, "Straight Flush"}, };
static const BOConfigValue k81[] = {{BO_F_EXTRA, 2, NULL}, };
static const BOConfigValue k82[] = {{BO_F_EXTRA, 0.10000000000000001, NULL}, };
static const BOConfigValue k84[] = {{BO_F_EXTRA, 0.25, NULL}, };
static const BOConfigValue k85[] = {{BO_F_EXTRA, 4, NULL}, };
static const BOConfigValue k86[] = {{BO_F_EXTRA, 1.5, NULL}, };
static const BOConfigValue k87[] = {{BO_F_EXTRA, 1, NULL}, };
static const BOConfigValue k88[] = {{BO_F_EXTRA_DOLLARS, 1, NULL}, {BO_F_EXTRA_INCREASE, 2, NULL}, };
static const BOConfigValue k89[] = {{BO_F_EXTRA, 0.20000000000000001, NULL}, };
static const BOConfigValue k92[] = {{BO_F_EXTRA, 2, NULL}, };
static const BOConfigValue k93[] = {{BO_F_EXTRA, 1, NULL}, };
static const BOConfigValue k94[] = {{BO_F_EXTRA_H_MOD, 1, NULL}, {BO_F_EXTRA_H_SIZE, 5, NULL}, };
static const BOConfigValue k95[] = {{BO_F_EXTRA, 4, NULL}, };
static const BOConfigValue k96[] = {{BO_F_EXTRA_DOLLARS, 1, NULL}, {BO_F_EXTRA_ODDS, 2, NULL}, };
static const BOConfigValue k97[] = {{BO_F_EXTRA, 5, NULL}, };
static const BOConfigValue k98[] = {{BO_F_EXTRA, 1, NULL}, };
static const BOConfigValue k99[] = {{BO_F_EXTRA, 2, NULL}, };
static const BOConfigValue k100[] = {{BO_F_EXTRA, 1, NULL}, };
static const BOConfigValue k103[] = {{BO_F_EXTRA, 25, NULL}, };
static const BOConfigValue k104[] = {{BO_F_EXTRA, 4, NULL}, };
static const BOConfigValue k105[] = {{BO_F_EXTRA, 0.25, NULL}, };
static const BOConfigValue k106[] = {{BO_F_EXTRA, 1.5, NULL}, };
static const BOConfigValue k107[] = {{BO_F_EXTRA, 2, NULL}, };
static const BOConfigValue k109[] = {{BO_F_EXTRA, 3, NULL}, };
static const BOConfigValue k110[] = {{BO_F_EXTRA, 2, NULL}, };
static const BOConfigValue k111[] = {{BO_F_EXTRA, 4, NULL}, };
static const BOConfigValue k112[] = {{BO_F_EXTRA, 2, NULL}, };
static const BOConfigValue k113[] = {{BO_F_EXTRA, 1.5, NULL}, };
static const BOConfigValue k114[] = {{BO_F_EXTRA, 0.01, NULL}, };
static const BOConfigValue k115[] = {{BO_F_EXTRA_CHIPS, 10, NULL}, {BO_F_EXTRA_MULT, 4, NULL}, };
static const BOConfigValue k116[] = {{BO_F_EXTRA, 10, NULL}, };
static const BOConfigValue k117[] = {{BO_F_EXTRA_CHIP_MOD, 3, NULL}, {BO_F_EXTRA_CHIPS, 0, NULL}, };
static const BOConfigValue k118[] = {{BO_F_EXTRA, 5, NULL}, };
static const BOConfigValue k119[] = {{BO_F_EXTRA, 0.25, NULL}, };
static const BOConfigValue k120[] = {{BO_F_EXTRA, 4, NULL}, };
static const BOConfigValue k122[] = {{BO_F_EXTRA, 3, NULL}, };
static const BOConfigValue k123[] = {{BO_F_EXTRA, 1, NULL}, };
static const BOConfigValue k125[] = {{BO_F_EXTRA_H_PLAYS, -1, NULL}, {BO_F_EXTRA_H_SIZE, 2, NULL}, };
static const BOConfigValue k128[] = {{BO_F_EXTRA, 0.25, NULL}, };
static const BOConfigValue k129[] = {{BO_F_EXTRA, 2, NULL}, };
static const BOConfigValue k130[] = {{BO_F_EXTRA, 1, NULL}, };
static const BOConfigValue k131[] = {{BO_F_EXTRA_XMULT, 1.5, NULL}, {BO_F_EXTRA_ODDS, 2, NULL}, };
static const BOConfigValue k132[] = {{BO_F_EXTRA, 50, NULL}, };
static const BOConfigValue k133[] = {{BO_F_EXTRA, 7, NULL}, };
static const BOConfigValue k134[] = {{BO_F_EXTRA, 0.75, NULL}, };
static const BOConfigValue k136[] = {{BO_F_EXTRA, 3, NULL}, };
static const BOConfigValue k138[] = {{BO_F_EXTRA_CHIP_MOD, 8, NULL}, {BO_F_EXTRA_CHIPS, 0, NULL}, };
static const BOConfigValue k141[] = {{BO_F_EXTRA, 2, NULL}, };
static const BOConfigValue k142[] = {{BO_F_EXTRA, 2, NULL}, };
static const BOConfigValue k143[] = {{BO_F_EXTRA, 8, NULL}, };
static const BOConfigValue k144[] = {{BO_F_EXTRA, 0.5, NULL}, };
static const BOConfigValue k150[] = {{BO_F_EXTRA_CHIP_MOD, 250, NULL}, {BO_F_EXTRA_H_SIZE, 2, NULL}, };
static const BOConfigValue k151[] = {{BO_F_EXTRA, 2, NULL}, };
static const BOConfigValue k153[] = {{BO_F_EXTRA, 1, NULL}, };
static const BOConfigValue k154[] = {{BO_F_EXTRA, 13, NULL}, };
static const BOConfigValue k155[] = {{BO_F_EXTRA, 3, NULL}, };
static const BOConfigValue k158[] = {{BO_F_EXTRA, 4, NULL}, };
static const BOConfigValue k159[] = {{BO_F_EXTRA_DOLLARS, 5, NULL}, {BO_F_EXTRA_MULT, 2, NULL}, };
static const BOConfigValue k160[] = {{BO_F_EXTRA, 1, NULL}, };
static const BOConfigValue k161[] = {{BO_F_EXTRA, 2, NULL}, };
static const BOConfigValue k162[] = {{BO_F_EXTRA_DISCARDS, 23, NULL}, {BO_F_EXTRA_XMULT_LC, 1, NULL}, };
static const BOConfigValue k166[] = {{BO_F_MAX_HIGHLIGHTED, 2, NULL}, {BO_F_MOD_CONV, 0, "m_lucky"}, };
static const BOConfigValue k167[] = {{BO_F_PLANETS, 2, NULL}, };
static const BOConfigValue k168[] = {{BO_F_MAX_HIGHLIGHTED, 2, NULL}, {BO_F_MOD_CONV, 0, "m_mult"}, };
static const BOConfigValue k169[] = {{BO_F_TAROTS, 2, NULL}, };
static const BOConfigValue k170[] = {{BO_F_MAX_HIGHLIGHTED, 2, NULL}, {BO_F_MOD_CONV, 0, "m_bonus"}, };
static const BOConfigValue k171[] = {{BO_F_MAX_HIGHLIGHTED, 1, NULL}, {BO_F_MOD_CONV, 0, "m_wild"}, };
static const BOConfigValue k172[] = {{BO_F_MAX_HIGHLIGHTED, 1, NULL}, {BO_F_MOD_CONV, 0, "m_steel"}, };
static const BOConfigValue k173[] = {{BO_F_MAX_HIGHLIGHTED, 1, NULL}, {BO_F_MOD_CONV, 0, "m_glass"}, };
static const BOConfigValue k174[] = {{BO_F_EXTRA, 20, NULL}, };
static const BOConfigValue k175[] = {{BO_F_EXTRA, 4, NULL}, };
static const BOConfigValue k176[] = {{BO_F_MAX_HIGHLIGHTED, 2, NULL}, {BO_F_MOD_CONV, 0, "up_rank"}, };
static const BOConfigValue k177[] = {{BO_F_MAX_HIGHLIGHTED, 2, NULL}, {BO_F_REMOVE_CARD, 1, NULL}, };
static const BOConfigValue k178[] = {{BO_F_MAX_HIGHLIGHTED, 2, NULL}, {BO_F_MIN_HIGHLIGHTED, 2, NULL}, {BO_F_MOD_CONV, 0, "card"}, };
static const BOConfigValue k179[] = {{BO_F_EXTRA, 50, NULL}, };
static const BOConfigValue k180[] = {{BO_F_MAX_HIGHLIGHTED, 1, NULL}, {BO_F_MOD_CONV, 0, "m_gold"}, };
static const BOConfigValue k181[] = {{BO_F_MAX_HIGHLIGHTED, 1, NULL}, {BO_F_MOD_CONV, 0, "m_stone"}, };
static const BOConfigValue k182[] = {{BO_F_MAX_HIGHLIGHTED, 3, NULL}, {BO_F_SUIT_CONV, 0, "Diamonds"}, };
static const BOConfigValue k183[] = {{BO_F_MAX_HIGHLIGHTED, 3, NULL}, {BO_F_SUIT_CONV, 0, "Clubs"}, };
static const BOConfigValue k184[] = {{BO_F_MAX_HIGHLIGHTED, 3, NULL}, {BO_F_SUIT_CONV, 0, "Hearts"}, };
static const BOConfigValue k186[] = {{BO_F_MAX_HIGHLIGHTED, 3, NULL}, {BO_F_SUIT_CONV, 0, "Spades"}, };
static const BOConfigValue k187[] = {{BO_F_HAND_TYPE, 0, "Pair"}, };
static const BOConfigValue k188[] = {{BO_F_HAND_TYPE, 0, "Three of a Kind"}, };
static const BOConfigValue k189[] = {{BO_F_HAND_TYPE, 0, "Full House"}, };
static const BOConfigValue k190[] = {{BO_F_HAND_TYPE, 0, "Four of a Kind"}, };
static const BOConfigValue k191[] = {{BO_F_HAND_TYPE, 0, "Flush"}, };
static const BOConfigValue k192[] = {{BO_F_HAND_TYPE, 0, "Straight"}, };
static const BOConfigValue k193[] = {{BO_F_HAND_TYPE, 0, "Two Pair"}, };
static const BOConfigValue k194[] = {{BO_F_HAND_TYPE, 0, "Straight Flush"}, };
static const BOConfigValue k195[] = {{BO_F_HAND_TYPE, 0, "High Card"}, };
static const BOConfigValue k196[] = {{BO_F_HAND_TYPE, 0, "Five of a Kind"}, {BO_F_SOFTLOCK, 1, NULL}, };
static const BOConfigValue k197[] = {{BO_F_HAND_TYPE, 0, "Flush House"}, {BO_F_SOFTLOCK, 1, NULL}, };
static const BOConfigValue k198[] = {{BO_F_HAND_TYPE, 0, "Flush Five"}, {BO_F_SOFTLOCK, 1, NULL}, };
static const BOConfigValue k199[] = {{BO_F_EXTRA, 3, NULL}, {BO_F_REMOVE_CARD, 1, NULL}, };
static const BOConfigValue k200[] = {{BO_F_EXTRA, 2, NULL}, {BO_F_REMOVE_CARD, 1, NULL}, };
static const BOConfigValue k201[] = {{BO_F_EXTRA, 4, NULL}, {BO_F_REMOVE_CARD, 1, NULL}, };
static const BOConfigValue k202[] = {{BO_F_EXTRA, 0, "Gold"}, {BO_F_MAX_HIGHLIGHTED, 1, NULL}, };
static const BOConfigValue k208[] = {{BO_F_EXTRA_DESTROY, 5, NULL}, {BO_F_EXTRA_DOLLARS, 20, NULL}, {BO_F_REMOVE_CARD, 1, NULL}, };
static const BOConfigValue k209[] = {{BO_F_EXTRA, 2, NULL}, };
static const BOConfigValue k210[] = {{BO_F_EXTRA, 0, "Red"}, {BO_F_MAX_HIGHLIGHTED, 1, NULL}, };
static const BOConfigValue k211[] = {{BO_F_EXTRA, 2, NULL}, };
static const BOConfigValue k212[] = {{BO_F_EXTRA, 0, "Blue"}, {BO_F_MAX_HIGHLIGHTED, 1, NULL}, };
static const BOConfigValue k213[] = {{BO_F_EXTRA, 0, "Purple"}, {BO_F_MAX_HIGHLIGHTED, 1, NULL}, };
static const BOConfigValue k214[] = {{BO_F_EXTRA, 2, NULL}, {BO_F_MAX_HIGHLIGHTED, 1, NULL}, };
static const BOConfigValue k218[] = {{BO_F_REQUIRES_1, 0, "v_overstock_norm"}, };
static const BOConfigValue k219[] = {{BO_F_EXTRA, 25, NULL}, };
static const BOConfigValue k220[] = {{BO_F_EXTRA, 50, NULL}, {BO_F_REQUIRES_1, 0, "v_clearance_sale"}, };
static const BOConfigValue k221[] = {{BO_F_EXTRA, 2, NULL}, };
static const BOConfigValue k222[] = {{BO_F_EXTRA, 4, NULL}, {BO_F_REQUIRES_1, 0, "v_hone"}, };
static const BOConfigValue k223[] = {{BO_F_EXTRA, 2, NULL}, };
static const BOConfigValue k224[] = {{BO_F_EXTRA, 2, NULL}, {BO_F_REQUIRES_1, 0, "v_reroll_surplus"}, };
static const BOConfigValue k225[] = {{BO_F_EXTRA, 3, NULL}, };
static const BOConfigValue k226[] = {{BO_F_EXTRA, 4, NULL}, {BO_F_REQUIRES_1, 0, "v_crystal_ball"}, };
static const BOConfigValue k227[] = {{BO_F_EXTRA, 3, NULL}, };
static const BOConfigValue k228[] = {{BO_F_EXTRA, 1.5, NULL}, {BO_F_REQUIRES_1, 0, "v_telescope"}, };
static const BOConfigValue k229[] = {{BO_F_EXTRA, 1, NULL}, };
static const BOConfigValue k230[] = {{BO_F_EXTRA, 1, NULL}, {BO_F_REQUIRES_1, 0, "v_grabber"}, };
static const BOConfigValue k231[] = {{BO_F_EXTRA, 1, NULL}, };
static const BOConfigValue k232[] = {{BO_F_EXTRA, 1, NULL}, {BO_F_REQUIRES_1, 0, "v_wasteful"}, };
static const BOConfigValue k233[] = {{BO_F_EXTRA, 2.3999999999999999, NULL}, {BO_F_EXTRA_DISP, 2, NULL}, };
static const BOConfigValue k234[] = {{BO_F_EXTRA, 8, NULL}, {BO_F_EXTRA_DISP, 4, NULL}, {BO_F_REQUIRES_1, 0, "v_tarot_merchant"}, };
static const BOConfigValue k235[] = {{BO_F_EXTRA, 2.3999999999999999, NULL}, {BO_F_EXTRA_DISP, 2, NULL}, };
static const BOConfigValue k236[] = {{BO_F_EXTRA, 8, NULL}, {BO_F_EXTRA_DISP, 4, NULL}, {BO_F_REQUIRES_1, 0, "v_planet_merchant"}, };
static const BOConfigValue k237[] = {{BO_F_EXTRA, 50, NULL}, };
static const BOConfigValue k238[] = {{BO_F_EXTRA, 100, NULL}, {BO_F_REQUIRES_1, 0, "v_seed_money"}, };
static const BOConfigValue k239[] = {{BO_F_EXTRA, 5, NULL}, };
static const BOConfigValue k240[] = {{BO_F_EXTRA, 15, NULL}, {BO_F_REQUIRES_1, 0, "v_blank"}, };
static const BOConfigValue k241[] = {{BO_F_EXTRA, 4, NULL}, };
static const BOConfigValue k242[] = {{BO_F_EXTRA, 4, NULL}, {BO_F_REQUIRES_1, 0, "v_magic_trick"}, };
static const BOConfigValue k243[] = {{BO_F_EXTRA, 1, NULL}, };
static const BOConfigValue k244[] = {{BO_F_EXTRA, 1, NULL}, {BO_F_REQUIRES_1, 0, "v_hieroglyph"}, };
static const BOConfigValue k245[] = {{BO_F_EXTRA, 10, NULL}, };
static const BOConfigValue k246[] = {{BO_F_EXTRA, 10, NULL}, {BO_F_REQUIRES_1, 0, "v_directors_cut"}, };
static const BOConfigValue k247[] = {{BO_F_EXTRA, 1, NULL}, };
static const BOConfigValue k248[] = {{BO_F_EXTRA, 1, NULL}, {BO_F_REQUIRES_1, 0, "v_paint_brush"}, };
static const BOConfigValue k249[] = {{BO_F_DISCARDS, 1, NULL}, };
static const BOConfigValue k250[] = {{BO_F_HANDS, 1, NULL}, };
static const BOConfigValue k251[] = {{BO_F_DOLLARS, 10, NULL}, };
static const BOConfigValue k252[] = {{BO_F_EXTRA_DISCARD_BONUS, 1, NULL}, {BO_F_EXTRA_HAND_BONUS, 2, NULL}, {BO_F_NO_INTEREST, 1, NULL}, };
static const BOConfigValue k253[] = {{BO_F_HANDS, -1, NULL}, {BO_F_JOKER_SLOT, 1, NULL}, };
static const BOConfigValue k254[] = {{BO_F_CONSUMABLES_1, 0, "c_fool"}, {BO_F_CONSUMABLES_2, 0, "c_fool"}, {BO_F_VOUCHER, 0, "v_crystal_ball"}, };
static const BOConfigValue k255[] = {{BO_F_CONSUMABLE_SLOT, -1, NULL}, {BO_F_VOUCHER, 0, "v_telescope"}, };
static const BOConfigValue k256[] = {{BO_F_CONSUMABLES_1, 0, "c_hex"}, {BO_F_SPECTRAL_RATE, 2, NULL}, };
static const BOConfigValue k257[] = {{BO_F_REMOVE_FACES, 1, NULL}, };
static const BOConfigValue k259[] = {{BO_F_VOUCHERS_1, 0, "v_tarot_merchant"}, {BO_F_VOUCHERS_2, 0, "v_planet_merchant"}, {BO_F_VOUCHERS_3, 0, "v_overstock_norm"}, };
static const BOConfigValue k260[] = {{BO_F_HAND_SIZE, 2, NULL}, {BO_F_JOKER_SLOT, -1, NULL}, };
static const BOConfigValue k262[] = {{BO_F_ANTE_SCALING, 2, NULL}, };
static const BOConfigValue k263[] = {{BO_F_RANDOMIZE_RANK_SUIT, 1, NULL}, };
static const BOConfigValue k265[] = {{BO_F_CHOOSE, 1, NULL}, {BO_F_EXTRA, 3, NULL}, };
static const BOConfigValue k266[] = {{BO_F_CHOOSE, 1, NULL}, {BO_F_EXTRA, 3, NULL}, };
static const BOConfigValue k267[] = {{BO_F_CHOOSE, 1, NULL}, {BO_F_EXTRA, 3, NULL}, };
static const BOConfigValue k268[] = {{BO_F_CHOOSE, 1, NULL}, {BO_F_EXTRA, 3, NULL}, };
static const BOConfigValue k269[] = {{BO_F_CHOOSE, 1, NULL}, {BO_F_EXTRA, 5, NULL}, };
static const BOConfigValue k270[] = {{BO_F_CHOOSE, 1, NULL}, {BO_F_EXTRA, 5, NULL}, };
static const BOConfigValue k271[] = {{BO_F_CHOOSE, 2, NULL}, {BO_F_EXTRA, 5, NULL}, };
static const BOConfigValue k272[] = {{BO_F_CHOOSE, 2, NULL}, {BO_F_EXTRA, 5, NULL}, };
static const BOConfigValue k273[] = {{BO_F_CHOOSE, 1, NULL}, {BO_F_EXTRA, 3, NULL}, };
static const BOConfigValue k274[] = {{BO_F_CHOOSE, 1, NULL}, {BO_F_EXTRA, 3, NULL}, };
static const BOConfigValue k275[] = {{BO_F_CHOOSE, 1, NULL}, {BO_F_EXTRA, 3, NULL}, };
static const BOConfigValue k276[] = {{BO_F_CHOOSE, 1, NULL}, {BO_F_EXTRA, 3, NULL}, };
static const BOConfigValue k277[] = {{BO_F_CHOOSE, 1, NULL}, {BO_F_EXTRA, 5, NULL}, };
static const BOConfigValue k278[] = {{BO_F_CHOOSE, 1, NULL}, {BO_F_EXTRA, 5, NULL}, };
static const BOConfigValue k279[] = {{BO_F_CHOOSE, 2, NULL}, {BO_F_EXTRA, 5, NULL}, };
static const BOConfigValue k280[] = {{BO_F_CHOOSE, 2, NULL}, {BO_F_EXTRA, 5, NULL}, };
static const BOConfigValue k281[] = {{BO_F_CHOOSE, 1, NULL}, {BO_F_EXTRA, 3, NULL}, };
static const BOConfigValue k282[] = {{BO_F_CHOOSE, 1, NULL}, {BO_F_EXTRA, 3, NULL}, };
static const BOConfigValue k283[] = {{BO_F_CHOOSE, 1, NULL}, {BO_F_EXTRA, 3, NULL}, };
static const BOConfigValue k284[] = {{BO_F_CHOOSE, 1, NULL}, {BO_F_EXTRA, 3, NULL}, };
static const BOConfigValue k285[] = {{BO_F_CHOOSE, 1, NULL}, {BO_F_EXTRA, 5, NULL}, };
static const BOConfigValue k286[] = {{BO_F_CHOOSE, 1, NULL}, {BO_F_EXTRA, 5, NULL}, };
static const BOConfigValue k287[] = {{BO_F_CHOOSE, 2, NULL}, {BO_F_EXTRA, 5, NULL}, };
static const BOConfigValue k288[] = {{BO_F_CHOOSE, 2, NULL}, {BO_F_EXTRA, 5, NULL}, };
static const BOConfigValue k289[] = {{BO_F_CHOOSE, 1, NULL}, {BO_F_EXTRA, 2, NULL}, };
static const BOConfigValue k290[] = {{BO_F_CHOOSE, 1, NULL}, {BO_F_EXTRA, 2, NULL}, };
static const BOConfigValue k291[] = {{BO_F_CHOOSE, 1, NULL}, {BO_F_EXTRA, 4, NULL}, };
static const BOConfigValue k292[] = {{BO_F_CHOOSE, 2, NULL}, {BO_F_EXTRA, 4, NULL}, };
static const BOConfigValue k293[] = {{BO_F_CHOOSE, 1, NULL}, {BO_F_EXTRA, 2, NULL}, };
static const BOConfigValue k294[] = {{BO_F_CHOOSE, 1, NULL}, {BO_F_EXTRA, 2, NULL}, };
static const BOConfigValue k295[] = {{BO_F_CHOOSE, 1, NULL}, {BO_F_EXTRA, 4, NULL}, };
static const BOConfigValue k296[] = {{BO_F_CHOOSE, 2, NULL}, {BO_F_EXTRA, 4, NULL}, };
static const BOConfigValue k298[] = {{BO_F_ODDS, 3, NULL}, {BO_F_REQUIRES, 0, "j_blueprint"}, };
static const BOConfigValue k299[] = {{BO_F_EDITION, 0, "negative"}, {BO_F_ODDS, 5, NULL}, {BO_F_REQUIRES, 0, "e_negative"}, };
static const BOConfigValue k300[] = {{BO_F_EDITION, 0, "foil"}, {BO_F_ODDS, 2, NULL}, {BO_F_REQUIRES, 0, "e_foil"}, };
static const BOConfigValue k301[] = {{BO_F_EDITION, 0, "holo"}, {BO_F_ODDS, 3, NULL}, {BO_F_REQUIRES, 0, "e_holo"}, };
static const BOConfigValue k302[] = {{BO_F_EDITION, 0, "polychrome"}, {BO_F_ODDS, 4, NULL}, {BO_F_REQUIRES, 0, "e_polychrome"}, };
static const BOConfigValue k303[] = {{BO_F_DOLLARS, 25, NULL}, };
static const BOConfigValue k310[] = {{BO_F_DOLLARS_PER_HAND, 1, NULL}, };
static const BOConfigValue k311[] = {{BO_F_DOLLARS_PER_DISCARD, 1, NULL}, };
static const BOConfigValue k317[] = {{BO_F_SPAWN_JOKERS, 2, NULL}, };
static const BOConfigValue k318[] = {{BO_F_SKIP_BONUS, 5, NULL}, };
static const BOConfigValue k319[] = {{BO_F_LEVELS, 3, NULL}, };
static const BOConfigValue k320[] = {{BO_F_MAX, 40, NULL}, };

static const char *kFieldNames[BO_FIELD_COUNT] = {
    "ante_scaling",
    "choose",
    "consumable_slot",
    "consumables_1",
    "consumables_2",
    "discards",
    "dollars",
    "dollars_per_discard",
    "dollars_per_hand",
    "edition",
    "extra",
    "extra_Xmult",
    "extra_chip_mod",
    "extra_chips",
    "extra_d_remaining",
    "extra_destroy",
    "extra_discard_bonus",
    "extra_discard_sub",
    "extra_discards",
    "extra_disp",
    "extra_dollars",
    "extra_every",
    "extra_faces",
    "extra_h_mod",
    "extra_h_plays",
    "extra_h_size",
    "extra_hand_add",
    "extra_hand_bonus",
    "extra_increase",
    "extra_max",
    "extra_min",
    "extra_mult",
    "extra_odds",
    "extra_poker_hand",
    "extra_remaining",
    "extra_s_mult",
    "extra_size",
    "extra_suit",
    "extra_xmult",
    "hand_size",
    "hand_type",
    "hands",
    "joker_slot",
    "levels",
    "max",
    "max_highlighted",
    "min_highlighted",
    "mod_conv",
    "no_interest",
    "odds",
    "planets",
    "randomize_rank_suit",
    "remove_card",
    "remove_faces",
    "requires",
    "requires_1",
    "skip_bonus",
    "softlock",
    "spawn_jokers",
    "spectral_rate",
    "suit_conv",
    "tarots",
    "voucher",
    "vouchers_1",
    "vouchers_2",
    "vouchers_3",
};

const char *bo_field_name(BOField field) {
    return field < BO_FIELD_COUNT ? kFieldNames[field] : "";
}

static const BOCenterInfo kCenters[BO_CENTER_COUNT] = {
    {"c_base", "Default Base", BO_SET_DEFAULT, 0, 0, 0, 1, 0, -1, -1, 1, 0, 0, 0, 0, 0, 0, 0, 0, NULL, "Base", NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, NULL},
    {"m_bonus", "Bonus", BO_SET_ENHANCED, 2, 0, 0, 1, 1, -1, -1, 1, 0, 0, 0, 0, 0, 0, 0, 0, NULL, "Bonus Card", NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 30, 0, 0, NULL},
    {"m_mult", "Mult", BO_SET_ENHANCED, 3, 0, 0, 2, 1, -1, -1, 1, 0, 0, 0, 0, 0, 0, 0, 0, NULL, "Mult Card", NULL, NULL, NULL, NULL, 4, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, NULL},
    {"m_wild", "Wild Card", BO_SET_ENHANCED, 4, 0, 0, 3, 1, -1, -1, 1, 0, 0, 0, 0, 0, 0, 0, 0, NULL, "Wild Card", NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, NULL},
    {"m_glass", "Glass Card", BO_SET_ENHANCED, 5, 0, 0, 5, 1, -1, -1, 1, 0, 0, 0, 0, 0, 0, 0, 0, NULL, "Glass Card", NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0, 4, 1, k5},
    {"m_steel", "Steel Card", BO_SET_ENHANCED, 6, 0, 0, 6, 1, -1, -1, 1, 0, 0, 0, 0, 0, 0, 0, 0, NULL, "Steel Card", NULL, NULL, NULL, NULL, 0, 0, 1.5, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, NULL},
    {"m_stone", "Stone Card", BO_SET_ENHANCED, 7, 0, 0, 5, 0, -1, -1, 1, 0, 0, 0, 0, 0, 0, 0, 0, NULL, "Stone Card", NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 50, 0, 0, NULL},
    {"m_gold", "Gold Card", BO_SET_ENHANCED, 8, 0, 0, 6, 0, -1, -1, 1, 0, 0, 0, 0, 0, 0, 0, 0, NULL, "Gold Card", NULL, NULL, NULL, NULL, 0, 0, 0, 3, 0, 0, 0, 1, 0, 0, 0, 0, 0, NULL},
    {"m_lucky", "Lucky Card", BO_SET_ENHANCED, 9, 0, 0, 4, 1, -1, -1, 1, 0, 0, 0, 0, 0, 0, 0, 0, NULL, "Lucky Card", NULL, NULL, NULL, NULL, 20, 0, 0, 0, 20, 0, 0, 1, 0, 0, 0, 0, 0, NULL},
    {"e_base", "Base", BO_SET_EDITION, 1, 0, 0, 0, 0, -1, -1, 1, 0, 0, 0, 0, 0, 0, 0, 0, NULL, NULL, NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, NULL},
    {"e_foil", "Foil", BO_SET_EDITION, 2, 0, 0, 0, 0, -1, -1, 1, 0, 0, 0, 0, 0, 0, 0, 0, NULL, NULL, NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 50, 1, k11},
    {"e_holo", "Holographic", BO_SET_EDITION, 3, 0, 0, 0, 0, -1, -1, 1, 0, 0, 0, 0, 0, 0, 0, 0, NULL, NULL, NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 10, 1, k12},
    {"e_polychrome", "Polychrome", BO_SET_EDITION, 4, 0, 0, 0, 0, -1, -1, 1, 0, 0, 0, 0, 0, 0, 0, 0, NULL, NULL, NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1.5, 1, k13},
    {"e_negative", "Negative", BO_SET_EDITION, 5, 0, 0, 0, 0, -1, -1, 1, 0, 0, 0, 0, 0, 0, 0, 0, NULL, NULL, NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 1, k14},
    {"j_joker", "Joker", BO_SET_JOKER, 1, 1, 2, 0, 0, -1, -1, 1, 1, 1, 1, 0, 0, 0, 0, 0, NULL, "Mult", NULL, NULL, NULL, NULL, 4, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, NULL},
    {"j_greedy_joker", "Greedy Joker", BO_SET_JOKER, 2, 1, 5, 6, 1, -1, -1, 1, 1, 1, 1, 0, 0, 0, 0, 0, NULL, "Suit Mult", NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 2, k16},
    {"j_lusty_joker", "Lusty Joker", BO_SET_JOKER, 3, 1, 5, 7, 1, -1, -1, 1, 1, 1, 1, 0, 0, 0, 0, 0, NULL, "Suit Mult", NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 2, k17},
    {"j_wrathful_joker", "Wrathful Joker", BO_SET_JOKER, 4, 1, 5, 8, 1, -1, -1, 1, 1, 1, 1, 0, 0, 0, 0, 0, NULL, "Suit Mult", NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 2, k18},
    {"j_gluttenous_joker", "Gluttonous Joker", BO_SET_JOKER, 5, 1, 5, 9, 1, -1, -1, 1, 1, 1, 1, 0, 0, 0, 0, 0, NULL, "Suit Mult", NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 2, k19},
    {"j_jolly", "Jolly Joker", BO_SET_JOKER, 6, 1, 3, 2, 0, -1, -1, 1, 1, 1, 1, 0, 0, 0, 0, 0, NULL, "Type Mult", NULL, NULL, NULL, "Pair", 0, 0, 0, 0, 0, 8, 0, 1, 0, 0, 0, 0, 0, NULL},
    {"j_zany", "Zany Joker", BO_SET_JOKER, 7, 1, 4, 3, 0, -1, -1, 1, 1, 1, 1, 0, 0, 0, 0, 0, NULL, "Type Mult", NULL, NULL, NULL, "Three of a Kind", 0, 0, 0, 0, 0, 12, 0, 1, 0, 0, 0, 0, 0, NULL},
    {"j_mad", "Mad Joker", BO_SET_JOKER, 8, 1, 4, 4, 0, -1, -1, 1, 1, 1, 1, 0, 0, 0, 0, 0, NULL, "Type Mult", NULL, NULL, NULL, "Two Pair", 0, 0, 0, 0, 0, 10, 0, 1, 0, 0, 0, 0, 0, NULL},
    {"j_crazy", "Crazy Joker", BO_SET_JOKER, 9, 1, 4, 5, 0, -1, -1, 1, 1, 1, 1, 0, 0, 0, 0, 0, NULL, "Type Mult", NULL, NULL, NULL, "Straight", 0, 0, 0, 0, 0, 12, 0, 1, 0, 0, 0, 0, 0, NULL},
    {"j_droll", "Droll Joker", BO_SET_JOKER, 10, 1, 4, 6, 0, -1, -1, 1, 1, 1, 1, 0, 0, 0, 0, 0, NULL, "Type Mult", NULL, NULL, NULL, "Flush", 0, 0, 0, 0, 0, 10, 0, 1, 0, 0, 0, 0, 0, NULL},
    {"j_sly", "Sly Joker", BO_SET_JOKER, 11, 1, 3, 0, 14, -1, -1, 1, 1, 1, 1, 0, 0, 0, 0, 0, NULL, NULL, NULL, NULL, NULL, "Pair", 0, 0, 0, 0, 0, 0, 50, 1, 0, 0, 0, 0, 0, NULL},
    {"j_wily", "Wily Joker", BO_SET_JOKER, 12, 1, 4, 1, 14, -1, -1, 1, 1, 1, 1, 0, 0, 0, 0, 0, NULL, NULL, NULL, NULL, NULL, "Three of a Kind", 0, 0, 0, 0, 0, 0, 100, 1, 0, 0, 0, 0, 0, NULL},
    {"j_clever", "Clever Joker", BO_SET_JOKER, 13, 1, 4, 2, 14, -1, -1, 1, 1, 1, 1, 0, 0, 0, 0, 0, NULL, NULL, NULL, NULL, NULL, "Two Pair", 0, 0, 0, 0, 0, 0, 80, 1, 0, 0, 0, 0, 0, NULL},
    {"j_devious", "Devious Joker", BO_SET_JOKER, 14, 1, 4, 3, 14, -1, -1, 1, 1, 1, 1, 0, 0, 0, 0, 0, NULL, NULL, NULL, NULL, NULL, "Straight", 0, 0, 0, 0, 0, 0, 100, 1, 0, 0, 0, 0, 0, NULL},
    {"j_crafty", "Crafty Joker", BO_SET_JOKER, 15, 1, 4, 4, 14, -1, -1, 1, 1, 1, 1, 0, 0, 0, 0, 0, NULL, NULL, NULL, NULL, NULL, "Flush", 0, 0, 0, 0, 0, 0, 80, 1, 0, 0, 0, 0, 0, NULL},
    {"j_half", "Half Joker", BO_SET_JOKER, 16, 1, 5, 7, 0, -1, -1, 1, 1, 1, 1, 0, 0, 0, 0, 0, NULL, "Hand Size Mult", NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 2, k30},
    {"j_stencil", "Joker Stencil", BO_SET_JOKER, 17, 2, 8, 2, 5, -1, -1, 1, 1, 1, 1, 0, 0, 0, 0, 0, NULL, "Hand Size Mult", NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, NULL},
    {"j_four_fingers", "Four Fingers", BO_SET_JOKER, 18, 2, 7, 6, 6, -1, -1, 1, 0, 1, 1, 0, 0, 0, 0, 0, NULL, "", NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, NULL},
    {"j_mime", "Mime", BO_SET_JOKER, 19, 2, 5, 4, 1, -1, -1, 1, 1, 1, 1, 0, 0, 0, 0, 0, NULL, "Hand card double", NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 1, k33},
    {"j_credit_card", "Credit Card", BO_SET_JOKER, 20, 1, 1, 5, 1, -1, -1, 1, 0, 1, 1, 0, 0, 0, 0, 0, NULL, "Credit", NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 20, 1, k34},
    {"j_ceremonial", "Ceremonial Dagger", BO_SET_JOKER, 21, 2, 6, 5, 5, -1, -1, 1, 1, 1, 0, 0, 0, 0, 0, 0, NULL, "", NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, NULL},
    {"j_banner", "Banner", BO_SET_JOKER, 22, 1, 5, 1, 2, -1, -1, 1, 1, 1, 1, 0, 0, 0, 0, 0, NULL, "Discard Chips", NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 30, 1, k36},
    {"j_mystic_summit", "Mystic Summit", BO_SET_JOKER, 23, 1, 5, 2, 2, -1, -1, 1, 1, 1, 1, 0, 0, 0, 0, 0, NULL, "No Discard Mult", NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 2, k37},
    {"j_marble", "Marble Joker", BO_SET_JOKER, 24, 2, 6, 3, 2, -1, -1, 1, 1, 1, 1, 0, 0, 0, 0, 0, NULL, "Stone card hands", NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 1, k38},
    {"j_loyalty_card", "Loyalty Card", BO_SET_JOKER, 25, 2, 5, 4, 2, -1, -1, 1, 1, 1, 1, 0, 0, 0, 0, 0, NULL, "1 in 10 mult", NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 3, k39},
    {"j_8_ball", "8 Ball", BO_SET_JOKER, 26, 1, 5, 0, 5, -1, -1, 1, 1, 1, 1, 0, 0, 0, 0, 0, NULL, "Spawn Tarot", NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 4, 1, k40},
    {"j_misprint", "Misprint", BO_SET_JOKER, 27, 1, 4, 6, 2, -1, -1, 1, 1, 1, 1, 0, 0, 0, 0, 0, NULL, "Random Mult", NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 2, k41},
    {"j_dusk", "Dusk", BO_SET_JOKER, 28, 2, 5, 4, 7, -1, -1, 1, 1, 1, 1, 0, 0, 0, 0, 0, NULL, "", NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 1, k42},
    {"j_raised_fist", "Raised Fist", BO_SET_JOKER, 29, 1, 5, 8, 2, -1, -1, 1, 1, 1, 1, 0, 0, 0, 0, 0, NULL, "Socialized Mult", NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, NULL},
    {"j_chaos", "Chaos the Clown", BO_SET_JOKER, 30, 1, 4, 1, 0, -1, -1, 1, 0, 1, 1, 0, 0, 0, 0, 0, NULL, "Bonus Rerolls", NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 1, k44},
    {"j_fibonacci", "Fibonacci", BO_SET_JOKER, 31, 2, 8, 1, 5, -1, -1, 1, 1, 1, 1, 0, 0, 0, 0, 0, NULL, "Card Mult", NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 8, 1, k45},
    {"j_steel_joker", "Steel Joker", BO_SET_JOKER, 32, 2, 7, 7, 2, -1, -1, 1, 1, 1, 1, 0, 0, 0, 0, 0, NULL, "Steel Card Buff", "m_steel", NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0.20000000000000001, 1, k46},
    {"j_scary_face", "Scary Face", BO_SET_JOKER, 33, 1, 4, 2, 3, -1, -1, 1, 1, 1, 1, 0, 0, 0, 0, 0, NULL, "Scary Face Cards", NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 30, 1, k47},
    {"j_abstract", "Abstract Joker", BO_SET_JOKER, 34, 1, 4, 3, 3, -1, -1, 1, 1, 1, 1, 0, 0, 0, 0, 0, NULL, "Joker Mult", NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 3, 1, k48},
    {"j_delayed_grat", "Delayed Gratification", BO_SET_JOKER, 35, 1, 4, 4, 3, -1, -1, 1, 0, 1, 1, 0, 0, 0, 0, 0, NULL, "Discard dollars", NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 2, 1, k49},
    {"j_hack", "Hack", BO_SET_JOKER, 36, 2, 6, 5, 2, -1, -1, 1, 1, 1, 1, 0, 0, 0, 0, 0, NULL, "Low Card double", NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 1, k50},
    {"j_pareidolia", "Pareidolia", BO_SET_JOKER, 37, 2, 5, 6, 3, -1, -1, 1, 0, 1, 1, 0, 0, 0, 0, 0, NULL, "All face cards", NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, NULL},
    {"j_gros_michel", "Gros Michel", BO_SET_JOKER, 38, 1, 5, 7, 6, -1, -1, 1, 1, 0, 1, 0, 0, 0, 0, 0, NULL, "", NULL, "gros_michel_extinct", NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 2, k52},
    {"j_even_steven", "Even Steven", BO_SET_JOKER, 39, 1, 4, 8, 3, -1, -1, 1, 1, 1, 1, 0, 0, 0, 0, 0, NULL, "Even Card Buff", NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 4, 1, k53},
    {"j_odd_todd", "Odd Todd", BO_SET_JOKER, 40, 1, 4, 9, 3, -1, -1, 1, 1, 1, 1, 0, 0, 0, 0, 0, NULL, "Odd Card Buff", NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 31, 1, k54},
    {"j_scholar", "Scholar", BO_SET_JOKER, 41, 1, 4, 0, 4, -1, -1, 1, 1, 1, 1, 0, 0, 0, 0, 0, NULL, "Ace Buff", NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 2, k55},
    {"j_business", "Business Card", BO_SET_JOKER, 42, 1, 4, 1, 4, -1, -1, 1, 1, 1, 1, 0, 0, 0, 0, 0, NULL, "Face Card dollar Chance", NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 2, 1, k56},
    {"j_supernova", "Supernova", BO_SET_JOKER, 43, 1, 5, 2, 4, -1, -1, 1, 1, 1, 1, 0, 0, 0, 0, 0, NULL, "Hand played mult", NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 1, k57},
    {"j_ride_the_bus", "Ride the Bus", BO_SET_JOKER, 44, 1, 6, 1, 6, -1, -1, 1, 1, 1, 0, 0, 0, 0, 0, 0, NULL, "", NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 1, k58},
    {"j_space", "Space Joker", BO_SET_JOKER, 45, 2, 5, 3, 5, -1, -1, 1, 1, 1, 1, 0, 0, 0, 0, 0, NULL, "Upgrade Hand chance", NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 4, 1, k59},
    {"j_egg", "Egg", BO_SET_JOKER, 46, 1, 4, 0, 10, -1, -1, 1, 0, 1, 1, 0, 0, 0, 0, 0, NULL, NULL, NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 3, 1, k60},
    {"j_burglar", "Burglar", BO_SET_JOKER, 47, 2, 6, 1, 10, -1, -1, 1, 1, 1, 1, 0, 0, 0, 0, 0, NULL, NULL, NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 3, 1, k61},
    {"j_blackboard", "Blackboard", BO_SET_JOKER, 48, 2, 6, 2, 10, -1, -1, 1, 1, 1, 1, 0, 0, 0, 0, 0, NULL, NULL, NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 3, 1, k62},
    {"j_runner", "Runner", BO_SET_JOKER, 49, 1, 5, 3, 10, -1, -1, 1, 1, 1, 0, 0, 0, 0, 0, 0, NULL, NULL, NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 2, k63},
    {"j_ice_cream", "Ice Cream", BO_SET_JOKER, 50, 1, 5, 4, 10, -1, -1, 1, 1, 0, 1, 0, 0, 0, 0, 0, NULL, NULL, NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 2, k64},
    {"j_dna", "DNA", BO_SET_JOKER, 51, 3, 8, 5, 10, -1, -1, 1, 1, 1, 1, 0, 0, 0, 0, 0, NULL, NULL, NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, NULL},
    {"j_splash", "Splash", BO_SET_JOKER, 52, 1, 3, 6, 10, -1, -1, 1, 0, 1, 1, 0, 0, 0, 0, 0, NULL, NULL, NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, NULL},
    {"j_blue_joker", "Blue Joker", BO_SET_JOKER, 53, 1, 5, 7, 10, -1, -1, 1, 1, 1, 1, 0, 0, 0, 0, 0, NULL, NULL, NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 2, 1, k67},
    {"j_sixth_sense", "Sixth Sense", BO_SET_JOKER, 54, 2, 6, 8, 10, -1, -1, 1, 0, 1, 1, 0, 0, 0, 0, 0, NULL, NULL, NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, NULL},
    {"j_constellation", "Constellation", BO_SET_JOKER, 55, 2, 6, 9, 10, -1, -1, 1, 1, 1, 0, 0, 0, 0, 0, 0, NULL, NULL, NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0.10000000000000001, 1, k69},
    {"j_hiker", "Hiker", BO_SET_JOKER, 56, 2, 5, 0, 11, -1, -1, 1, 1, 1, 1, 0, 0, 0, 0, 0, NULL, NULL, NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 5, 1, k70},
    {"j_faceless", "Faceless Joker", BO_SET_JOKER, 57, 1, 4, 1, 11, -1, -1, 1, 1, 1, 1, 0, 0, 0, 0, 0, NULL, NULL, NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 2, k71},
    {"j_green_joker", "Green Joker", BO_SET_JOKER, 58, 1, 4, 2, 11, -1, -1, 1, 1, 1, 0, 0, 0, 0, 0, 0, NULL, NULL, NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 2, k72},
    {"j_superposition", "Superposition", BO_SET_JOKER, 59, 1, 4, 3, 11, -1, -1, 1, 1, 1, 1, 0, 0, 0, 0, 0, NULL, NULL, NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, NULL},
    {"j_todo_list", "To Do List", BO_SET_JOKER, 60, 1, 4, 4, 11, -1, -1, 1, 1, 1, 1, 0, 0, 0, 0, 0, NULL, NULL, NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 2, k74},
    {"j_cavendish", "Cavendish", BO_SET_JOKER, 61, 1, 4, 5, 11, -1, -1, 1, 1, 0, 1, 0, 0, 0, 0, 0, NULL, NULL, NULL, NULL, "gros_michel_extinct", NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 2, k75},
    {"j_card_sharp", "Card Sharp", BO_SET_JOKER, 62, 2, 6, 6, 11, -1, -1, 1, 1, 1, 1, 0, 0, 0, 0, 0, NULL, NULL, NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, k76},
    {"j_red_card", "Red Card", BO_SET_JOKER, 63, 1, 5, 7, 11, -1, -1, 1, 1, 1, 0, 0, 0, 0, 0, 0, NULL, NULL, NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 3, 1, k77},
    {"j_madness", "Madness", BO_SET_JOKER, 64, 2, 7, 8, 11, -1, -1, 1, 1, 1, 0, 0, 0, 0, 0, 0, NULL, NULL, NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0.5, 1, k78},
    {"j_square", "Square Joker", BO_SET_JOKER, 65, 1, 4, 9, 11, -1, -1, 1, 1, 1, 0, 0, 0, 0, 0, 0, NULL, NULL, NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 2, k79},
    {"j_seance", "Seance", BO_SET_JOKER, 66, 2, 6, 0, 12, -1, -1, 1, 1, 1, 1, 0, 0, 0, 0, 0, NULL, NULL, NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, k80},
    {"j_riff_raff", "Riff-raff", BO_SET_JOKER, 67, 1, 6, 1, 12, -1, -1, 1, 1, 1, 1, 0, 0, 0, 0, 0, NULL, NULL, NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 2, 1, k81},
    {"j_vampire", "Vampire", BO_SET_JOKER, 68, 2, 7, 2, 12, -1, -1, 1, 1, 1, 0, 0, 0, 0, 0, 0, NULL, NULL, NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0.10000000000000001, 1, k82},
    {"j_shortcut", "Shortcut", BO_SET_JOKER, 69, 2, 7, 3, 12, -1, -1, 1, 0, 1, 1, 0, 0, 0, 0, 0, NULL, NULL, NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, NULL},
    {"j_hologram", "Hologram", BO_SET_JOKER, 70, 2, 7, 4, 12, 2, 9, 1, 1, 1, 0, 0, 0, 0, 0, 0, NULL, NULL, NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0.25, 1, k84},
    {"j_vagabond", "Vagabond", BO_SET_JOKER, 71, 3, 8, 5, 12, -1, -1, 1, 1, 1, 1, 0, 0, 0, 0, 0, NULL, NULL, NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 4, 1, k85},
    {"j_baron", "Baron", BO_SET_JOKER, 72, 3, 8, 6, 12, -1, -1, 1, 1, 1, 1, 0, 0, 0, 0, 0, NULL, NULL, NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1.5, 1, k86},
    {"j_cloud_9", "Cloud 9", BO_SET_JOKER, 73, 2, 7, 7, 12, -1, -1, 1, 0, 1, 1, 0, 0, 0, 0, 0, NULL, NULL, NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 1, k87},
    {"j_rocket", "Rocket", BO_SET_JOKER, 74, 2, 6, 8, 12, -1, -1, 1, 0, 1, 0, 0, 0, 0, 0, 0, NULL, NULL, NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 2, k88},
    {"j_obelisk", "Obelisk", BO_SET_JOKER, 75, 3, 8, 9, 12, -1, -1, 1, 1, 1, 0, 0, 0, 0, 0, 0, NULL, NULL, NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0.20000000000000001, 1, k89},
    {"j_midas_mask", "Midas Mask", BO_SET_JOKER, 76, 2, 7, 0, 13, -1, -1, 1, 0, 1, 1, 0, 0, 0, 0, 0, NULL, NULL, NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, NULL},
    {"j_luchador", "Luchador", BO_SET_JOKER, 77, 2, 5, 1, 13, -1, -1, 1, 1, 0, 1, 0, 0, 0, 0, 0, NULL, NULL, NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, NULL},
    {"j_photograph", "Photograph", BO_SET_JOKER, 78, 1, 5, 2, 13, -1, -1, 1, 1, 1, 1, 0, 0, 0, 0, 0, NULL, NULL, NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 2, 1, k92},
    {"j_gift", "Gift Card", BO_SET_JOKER, 79, 2, 6, 3, 13, -1, -1, 1, 0, 1, 1, 0, 0, 0, 0, 0, NULL, NULL, NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 1, k93},
    {"j_turtle_bean", "Turtle Bean", BO_SET_JOKER, 80, 2, 6, 4, 13, -1, -1, 1, 0, 0, 1, 0, 0, 0, 0, 0, NULL, NULL, NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 2, k94},
    {"j_erosion", "Erosion", BO_SET_JOKER, 81, 2, 6, 5, 13, -1, -1, 1, 1, 1, 1, 0, 0, 0, 0, 0, NULL, NULL, NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 4, 1, k95},
    {"j_reserved_parking", "Reserved Parking", BO_SET_JOKER, 82, 1, 6, 6, 13, -1, -1, 1, 1, 1, 1, 0, 0, 0, 0, 0, NULL, NULL, NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 2, k96},
    {"j_mail", "Mail-In Rebate", BO_SET_JOKER, 83, 1, 4, 7, 13, -1, -1, 1, 1, 1, 1, 0, 0, 0, 0, 0, NULL, NULL, NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 5, 1, k97},
    {"j_to_the_moon", "To the Moon", BO_SET_JOKER, 84, 2, 5, 8, 13, -1, -1, 1, 0, 1, 1, 0, 0, 0, 0, 0, NULL, NULL, NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 1, k98},
    {"j_hallucination", "Hallucination", BO_SET_JOKER, 85, 1, 4, 9, 13, -1, -1, 1, 1, 1, 1, 0, 0, 0, 0, 0, NULL, NULL, NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 2, 1, k99},
    {"j_fortune_teller", "Fortune Teller", BO_SET_JOKER, 86, 1, 6, 7, 5, -1, -1, 1, 1, 1, 1, 0, 0, 0, 0, 0, NULL, "", NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 1, k100},
    {"j_juggler", "Juggler", BO_SET_JOKER, 87, 1, 4, 0, 1, -1, -1, 1, 0, 1, 1, 0, 0, 0, 0, 0, NULL, "Hand Size", NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, NULL},
    {"j_drunkard", "Drunkard", BO_SET_JOKER, 88, 1, 4, 1, 1, -1, -1, 1, 0, 1, 1, 0, 0, 0, 0, 0, NULL, "Discard Size", NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, NULL},
    {"j_stone", "Stone Joker", BO_SET_JOKER, 89, 2, 6, 9, 0, -1, -1, 1, 1, 1, 1, 0, 0, 0, 0, 0, NULL, "Stone Card Buff", "m_stone", NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 25, 1, k103},
    {"j_golden", "Golden Joker", BO_SET_JOKER, 90, 1, 6, 9, 2, -1, -1, 1, 0, 1, 1, 0, 0, 0, 0, 0, NULL, "Bonus dollars", NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 4, 1, k104},
    {"j_lucky_cat", "Lucky Cat", BO_SET_JOKER, 91, 2, 6, 5, 14, -1, -1, 1, 1, 1, 0, 0, 0, 0, 0, 0, NULL, NULL, "m_lucky", NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0.25, 1, k105},
    {"j_baseball", "Baseball Card", BO_SET_JOKER, 92, 3, 8, 6, 14, -1, -1, 1, 1, 1, 1, 0, 0, 0, 0, 0, NULL, NULL, NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1.5, 1, k106},
    {"j_bull", "Bull", BO_SET_JOKER, 93, 2, 6, 7, 14, -1, -1, 1, 1, 1, 1, 0, 0, 0, 0, 0, NULL, NULL, NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 2, 1, k107},
    {"j_diet_cola", "Diet Cola", BO_SET_JOKER, 94, 2, 6, 8, 14, -1, -1, 1, 1, 0, 1, 0, 0, 0, 0, 0, NULL, NULL, NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, NULL},
    {"j_trading", "Trading Card", BO_SET_JOKER, 95, 2, 6, 9, 14, -1, -1, 1, 0, 1, 1, 0, 0, 0, 0, 0, NULL, NULL, NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 3, 1, k109},
    {"j_flash", "Flash Card", BO_SET_JOKER, 96, 2, 5, 0, 15, -1, -1, 1, 1, 1, 0, 0, 0, 0, 0, 0, NULL, NULL, NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 2, 1, k110},
    {"j_popcorn", "Popcorn", BO_SET_JOKER, 97, 1, 5, 1, 15, -1, -1, 1, 1, 0, 1, 0, 0, 0, 0, 0, NULL, NULL, NULL, NULL, NULL, NULL, 20, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 4, 1, k111},
    {"j_trousers", "Spare Trousers", BO_SET_JOKER, 98, 2, 6, 4, 15, -1, -1, 1, 1, 1, 0, 0, 0, 0, 0, 0, NULL, NULL, NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 2, 1, k112},
    {"j_ancient", "Ancient Joker", BO_SET_JOKER, 99, 3, 8, 7, 15, -1, -1, 1, 1, 1, 1, 0, 0, 0, 0, 0, NULL, NULL, NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1.5, 1, k113},
    {"j_ramen", "Ramen", BO_SET_JOKER, 100, 2, 6, 2, 15, -1, -1, 1, 1, 0, 1, 0, 0, 0, 0, 0, NULL, NULL, NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0.01, 1, k114},
    {"j_walkie_talkie", "Walkie Talkie", BO_SET_JOKER, 101, 1, 4, 8, 15, -1, -1, 1, 1, 1, 1, 0, 0, 0, 0, 0, NULL, NULL, NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 2, k115},
    {"j_selzer", "Seltzer", BO_SET_JOKER, 102, 2, 6, 3, 15, -1, -1, 1, 1, 0, 1, 0, 0, 0, 0, 0, NULL, NULL, NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 10, 1, k116},
    {"j_castle", "Castle", BO_SET_JOKER, 103, 2, 6, 9, 15, -1, -1, 1, 1, 1, 0, 0, 0, 0, 0, 0, NULL, NULL, NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 2, k117},
    {"j_smiley", "Smiley Face", BO_SET_JOKER, 104, 1, 4, 6, 15, -1, -1, 1, 1, 1, 1, 0, 0, 0, 0, 0, NULL, NULL, NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 5, 1, k118},
    {"j_campfire", "Campfire", BO_SET_JOKER, 105, 3, 9, 5, 15, -1, -1, 1, 1, 1, 1, 0, 0, 0, 0, 0, NULL, NULL, NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0.25, 1, k119},
    {"j_ticket", "Golden Ticket", BO_SET_JOKER, 106, 1, 5, 5, 3, -1, -1, 0, 1, 1, 1, 0, 0, 0, 0, 0, NULL, "dollars for Gold cards", "m_gold", NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 4, 1, k120},
    {"j_mr_bones", "Mr. Bones", BO_SET_JOKER, 107, 2, 5, 3, 4, -1, -1, 0, 0, 0, 1, 0, 0, 0, 0, 0, NULL, "Prevent Death", NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, NULL},
    {"j_acrobat", "Acrobat", BO_SET_JOKER, 108, 2, 6, 2, 1, -1, -1, 0, 1, 1, 1, 0, 0, 0, 0, 0, NULL, "Shop size", NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 3, 1, k122},
    {"j_sock_and_buskin", "Sock and Buskin", BO_SET_JOKER, 109, 2, 6, 3, 1, -1, -1, 0, 1, 1, 1, 0, 0, 0, 0, 0, NULL, "Face card double", NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 1, k123},
    {"j_swashbuckler", "Swashbuckler", BO_SET_JOKER, 110, 1, 4, 9, 5, -1, -1, 0, 1, 1, 1, 0, 0, 0, 0, 0, NULL, "Set Mult", NULL, NULL, NULL, NULL, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, NULL},
    {"j_troubadour", "Troubadour", BO_SET_JOKER, 111, 2, 6, 0, 2, -1, -1, 0, 0, 1, 1, 0, 0, 0, 0, 0, NULL, "Hand Size, Plays", NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 2, k125},
    {"j_certificate", "Certificate", BO_SET_JOKER, 112, 2, 6, 8, 8, -1, -1, 0, 1, 1, 1, 0, 0, 0, 0, 0, NULL, "", NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, NULL},
    {"j_smeared", "Smeared Joker", BO_SET_JOKER, 113, 2, 7, 4, 6, -1, -1, 0, 0, 1, 1, 0, 0, 0, 0, 0, NULL, "", NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, NULL},
    {"j_throwback", "Throwback", BO_SET_JOKER, 114, 2, 6, 5, 7, -1, -1, 0, 1, 1, 1, 0, 0, 0, 0, 0, NULL, "", NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0.25, 1, k128},
    {"j_hanging_chad", "Hanging Chad", BO_SET_JOKER, 115, 1, 4, 9, 6, -1, -1, 0, 1, 1, 1, 0, 0, 0, 0, 0, NULL, "", NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 2, 1, k129},
    {"j_rough_gem", "Rough Gem", BO_SET_JOKER, 116, 2, 7, 9, 7, -1, -1, 0, 1, 1, 1, 0, 0, 0, 0, 0, NULL, "", NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 1, k130},
    {"j_bloodstone", "Bloodstone", BO_SET_JOKER, 117, 2, 7, 0, 8, -1, -1, 0, 1, 1, 1, 0, 0, 0, 0, 0, NULL, "", NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 2, k131},
    {"j_arrowhead", "Arrowhead", BO_SET_JOKER, 118, 2, 7, 1, 8, -1, -1, 0, 1, 1, 1, 0, 0, 0, 0, 0, NULL, "", NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 50, 1, k132},
    {"j_onyx_agate", "Onyx Agate", BO_SET_JOKER, 119, 2, 7, 2, 8, -1, -1, 0, 1, 1, 1, 0, 0, 0, 0, 0, NULL, "", NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 7, 1, k133},
    {"j_glass", "Glass Joker", BO_SET_JOKER, 120, 2, 6, 1, 3, -1, -1, 0, 1, 1, 0, 0, 0, 0, 0, 0, NULL, "Glass Card", "m_glass", NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0.75, 1, k134},
    {"j_ring_master", "Showman", BO_SET_JOKER, 121, 2, 5, 6, 5, -1, -1, 0, 0, 1, 1, 0, 0, 0, 0, 0, NULL, "", NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, NULL},
    {"j_flower_pot", "Flower Pot", BO_SET_JOKER, 122, 2, 6, 0, 6, -1, -1, 0, 1, 1, 1, 0, 0, 0, 0, 0, NULL, "", NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 3, 1, k136},
    {"j_blueprint", "Blueprint", BO_SET_JOKER, 123, 3, 10, 0, 3, -1, -1, 0, 1, 1, 1, 0, 0, 0, 0, 0, NULL, "Copycat", NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, NULL},
    {"j_wee", "Wee Joker", BO_SET_JOKER, 124, 3, 8, 0, 0, -1, -1, 0, 1, 1, 0, 0, 0, 0, 0, 0, NULL, "", NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 2, k138},
    {"j_merry_andy", "Merry Andy", BO_SET_JOKER, 125, 2, 7, 8, 0, -1, -1, 0, 0, 1, 1, 0, 0, 0, 0, 0, NULL, "", NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, -1, 3, 0, 0, 0, NULL},
    {"j_oops", "Oops! All 6s", BO_SET_JOKER, 126, 2, 4, 5, 6, -1, -1, 0, 0, 1, 1, 0, 0, 0, 0, 0, NULL, "", NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, NULL},
    {"j_idol", "The Idol", BO_SET_JOKER, 127, 2, 6, 6, 7, -1, -1, 0, 1, 1, 1, 0, 0, 0, 0, 0, NULL, "", NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 2, 1, k141},
    {"j_seeing_double", "Seeing Double", BO_SET_JOKER, 128, 2, 6, 4, 4, -1, -1, 0, 1, 1, 1, 0, 0, 0, 0, 0, NULL, "X1.5 Mult club 7", NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 2, 1, k142},
    {"j_matador", "Matador", BO_SET_JOKER, 129, 2, 7, 4, 5, -1, -1, 0, 1, 1, 1, 0, 0, 0, 0, 0, NULL, "", NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 8, 1, k143},
    {"j_hit_the_road", "Hit the Road", BO_SET_JOKER, 130, 3, 8, 8, 5, -1, -1, 0, 1, 1, 1, 0, 0, 0, 0, 0, NULL, "Jack Discard Effect", NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0.5, 1, k144},
    {"j_duo", "The Duo", BO_SET_JOKER, 131, 3, 8, 5, 4, -1, -1, 0, 1, 1, 1, 0, 0, 0, 0, 0, NULL, "X1.5 Mult", NULL, NULL, NULL, "Pair", 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, NULL},
    {"j_trio", "The Trio", BO_SET_JOKER, 132, 3, 8, 6, 4, -1, -1, 0, 1, 1, 1, 0, 0, 0, 0, 0, NULL, "X2 Mult", NULL, NULL, NULL, "Three of a Kind", 0, 0, 0, 0, 0, 0, 0, 3, 0, 0, 0, 0, 0, NULL},
    {"j_family", "The Family", BO_SET_JOKER, 133, 3, 8, 7, 4, -1, -1, 0, 1, 1, 1, 0, 0, 0, 0, 0, NULL, "X3 Mult", NULL, NULL, NULL, "Four of a Kind", 0, 0, 0, 0, 0, 0, 0, 4, 0, 0, 0, 0, 0, NULL},
    {"j_order", "The Order", BO_SET_JOKER, 134, 3, 8, 8, 4, -1, -1, 0, 1, 1, 1, 0, 0, 0, 0, 0, NULL, "X3 Mult", NULL, NULL, NULL, "Straight", 0, 0, 0, 0, 0, 0, 0, 3, 0, 0, 0, 0, 0, NULL},
    {"j_tribe", "The Tribe", BO_SET_JOKER, 135, 3, 8, 9, 4, -1, -1, 0, 1, 1, 1, 0, 0, 0, 0, 0, NULL, "X3 Mult", NULL, NULL, NULL, "Flush", 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, NULL},
    {"j_stuntman", "Stuntman", BO_SET_JOKER, 136, 3, 7, 8, 6, -1, -1, 0, 1, 1, 1, 0, 0, 0, 0, 0, NULL, "", NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 2, k150},
    {"j_invisible", "Invisible Joker", BO_SET_JOKER, 137, 3, 8, 1, 7, -1, -1, 0, 0, 0, 1, 0, 0, 0, 0, 0, NULL, "", NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 2, 1, k151},
    {"j_brainstorm", "Brainstorm", BO_SET_JOKER, 138, 3, 10, 7, 7, -1, -1, 0, 1, 1, 1, 0, 0, 0, 0, 0, NULL, "Copycat", NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, NULL},
    {"j_satellite", "Satellite", BO_SET_JOKER, 139, 2, 6, 8, 7, -1, -1, 0, 0, 1, 1, 0, 0, 0, 0, 0, NULL, "", NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 1, k153},
    {"j_shoot_the_moon", "Shoot the Moon", BO_SET_JOKER, 140, 1, 5, 2, 6, -1, -1, 0, 1, 1, 1, 0, 0, 0, 0, 0, NULL, "", NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 13, 1, k154},
    {"j_drivers_license", "Driver's License", BO_SET_JOKER, 141, 3, 7, 0, 7, -1, -1, 0, 1, 1, 1, 0, 0, 0, 0, 0, NULL, "", NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 3, 1, k155},
    {"j_cartomancer", "Cartomancer", BO_SET_JOKER, 142, 2, 6, 7, 3, -1, -1, 0, 1, 1, 1, 0, 0, 0, 0, 0, NULL, "Tarot Buff", NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, NULL},
    {"j_astronomer", "Astronomer", BO_SET_JOKER, 143, 2, 8, 2, 7, -1, -1, 0, 0, 1, 1, 0, 0, 0, 0, 0, NULL, "", NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, NULL},
    {"j_burnt", "Burnt Joker", BO_SET_JOKER, 144, 3, 8, 3, 7, -1, -1, 0, 1, 1, 1, 0, 0, 0, 0, 0, NULL, "", NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 4, 1, k158},
    {"j_bootstraps", "Bootstraps", BO_SET_JOKER, 145, 2, 7, 9, 8, -1, -1, 0, 1, 1, 1, 0, 0, 0, 0, 0, NULL, "", NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 2, k159},
    {"j_caino", "Caino", BO_SET_JOKER, 146, 4, 20, 3, 8, 3, 9, 0, 1, 1, 1, 0, 0, 0, 0, 0, NULL, "", NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 1, k160},
    {"j_triboulet", "Triboulet", BO_SET_JOKER, 147, 4, 20, 4, 8, 4, 9, 0, 1, 1, 1, 0, 0, 0, 0, 0, NULL, "", NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 2, 1, k161},
    {"j_yorick", "Yorick", BO_SET_JOKER, 148, 4, 20, 5, 8, 5, 9, 0, 1, 1, 1, 0, 0, 0, 0, 0, NULL, "", NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 2, k162},
    {"j_chicot", "Chicot", BO_SET_JOKER, 149, 4, 20, 6, 8, 6, 9, 0, 0, 1, 1, 0, 0, 0, 0, 0, NULL, "", NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, NULL},
    {"j_perkeo", "Perkeo", BO_SET_JOKER, 150, 4, 20, 7, 8, 7, 9, 0, 1, 1, 1, 0, 0, 0, 0, 0, NULL, "", NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, NULL},
    {"c_fool", "The Fool", BO_SET_TAROT, 1, 0, 3, 0, 0, -1, -1, 1, 0, 0, 0, 0, 1, 0, 0, 0, NULL, "Disable Blind Effect", NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, NULL},
    {"c_magician", "The Magician", BO_SET_TAROT, 2, 0, 3, 1, 0, -1, -1, 1, 0, 0, 0, 0, 1, 0, 0, 0, NULL, "Enhance", NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 2, k166},
    {"c_high_priestess", "The High Priestess", BO_SET_TAROT, 3, 0, 3, 2, 0, -1, -1, 1, 0, 0, 0, 0, 1, 0, 0, 0, NULL, "Round Bonus", NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, k167},
    {"c_empress", "The Empress", BO_SET_TAROT, 4, 0, 3, 3, 0, -1, -1, 1, 0, 0, 0, 0, 1, 0, 0, 0, NULL, "Enhance", NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 2, k168},
    {"c_emperor", "The Emperor", BO_SET_TAROT, 5, 0, 3, 4, 0, -1, -1, 1, 0, 0, 0, 0, 1, 0, 0, 0, NULL, "Round Bonus", NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, k169},
    {"c_heirophant", "The Hierophant", BO_SET_TAROT, 6, 0, 3, 5, 0, -1, -1, 1, 0, 0, 0, 0, 1, 0, 0, 0, NULL, "Enhance", NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 2, k170},
    {"c_lovers", "The Lovers", BO_SET_TAROT, 7, 0, 3, 6, 0, -1, -1, 1, 0, 0, 0, 0, 1, 0, 0, 0, NULL, "Enhance", NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 2, k171},
    {"c_chariot", "The Chariot", BO_SET_TAROT, 8, 0, 3, 7, 0, -1, -1, 1, 0, 0, 0, 0, 1, 0, 0, 0, NULL, "Enhance", NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 2, k172},
    {"c_justice", "Justice", BO_SET_TAROT, 9, 0, 3, 8, 0, -1, -1, 1, 0, 0, 0, 0, 1, 0, 0, 0, NULL, "Enhance", NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 2, k173},
    {"c_hermit", "The Hermit", BO_SET_TAROT, 10, 0, 3, 9, 0, -1, -1, 1, 0, 0, 0, 0, 1, 0, 0, 0, NULL, "Dollar Doubler", NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 20, 1, k174},
    {"c_wheel_of_fortune", "The Wheel of Fortune", BO_SET_TAROT, 11, 0, 3, 0, 1, -1, -1, 1, 0, 0, 0, 0, 1, 0, 0, 0, NULL, "Round Bonus", NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 4, 1, k175},
    {"c_strength", "Strength", BO_SET_TAROT, 12, 0, 3, 1, 1, -1, -1, 1, 0, 0, 0, 0, 1, 0, 0, 0, NULL, "Round Bonus", NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 2, k176},
    {"c_hanged_man", "The Hanged Man", BO_SET_TAROT, 13, 0, 3, 2, 1, -1, -1, 1, 0, 0, 0, 0, 1, 0, 0, 0, NULL, "Card Removal", NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 2, k177},
    {"c_death", "Death", BO_SET_TAROT, 14, 0, 3, 3, 1, -1, -1, 1, 0, 0, 0, 0, 1, 0, 0, 0, NULL, "Card Conversion", NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 3, k178},
    {"c_temperance", "Temperance", BO_SET_TAROT, 15, 0, 3, 4, 1, -1, -1, 1, 0, 0, 0, 0, 1, 0, 0, 0, NULL, "Joker Payout", NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 50, 1, k179},
    {"c_devil", "The Devil", BO_SET_TAROT, 16, 0, 3, 5, 1, -1, -1, 1, 0, 0, 0, 0, 1, 0, 0, 0, NULL, "Enhance", NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 2, k180},
    {"c_tower", "The Tower", BO_SET_TAROT, 17, 0, 3, 6, 1, -1, -1, 1, 0, 0, 0, 0, 1, 0, 0, 0, NULL, "Enhance", NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 2, k181},
    {"c_star", "The Star", BO_SET_TAROT, 18, 0, 3, 7, 1, -1, -1, 1, 0, 0, 0, 0, 1, 0, 0, 0, NULL, "Suit Conversion", NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 2, k182},
    {"c_moon", "The Moon", BO_SET_TAROT, 19, 0, 3, 8, 1, -1, -1, 1, 0, 0, 0, 0, 1, 0, 0, 0, NULL, "Suit Conversion", NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 2, k183},
    {"c_sun", "The Sun", BO_SET_TAROT, 20, 0, 3, 9, 1, -1, -1, 1, 0, 0, 0, 0, 1, 0, 0, 0, NULL, "Suit Conversion", NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 2, k184},
    {"c_judgement", "Judgement", BO_SET_TAROT, 21, 0, 3, 0, 2, -1, -1, 1, 0, 0, 0, 0, 1, 0, 0, 0, NULL, "Random Joker", NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, NULL},
    {"c_world", "The World", BO_SET_TAROT, 22, 0, 3, 1, 2, -1, -1, 1, 0, 0, 0, 0, 1, 0, 0, 0, NULL, "Suit Conversion", NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 2, k186},
    {"c_mercury", "Mercury", BO_SET_PLANET, 1, 0, 3, 0, 3, -1, -1, 1, 0, 0, 0, 0, 1, 0, 0, 0, NULL, "Hand Upgrade", NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, k187},
    {"c_venus", "Venus", BO_SET_PLANET, 2, 0, 3, 1, 3, -1, -1, 1, 0, 0, 0, 0, 1, 0, 0, 0, NULL, "Hand Upgrade", NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, k188},
    {"c_earth", "Earth", BO_SET_PLANET, 3, 0, 3, 2, 3, -1, -1, 1, 0, 0, 0, 0, 1, 0, 0, 0, NULL, "Hand Upgrade", NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, k189},
    {"c_mars", "Mars", BO_SET_PLANET, 4, 0, 3, 3, 3, -1, -1, 1, 0, 0, 0, 0, 1, 0, 0, 0, NULL, "Hand Upgrade", NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, k190},
    {"c_jupiter", "Jupiter", BO_SET_PLANET, 5, 0, 3, 4, 3, -1, -1, 1, 0, 0, 0, 0, 1, 0, 0, 0, NULL, "Hand Upgrade", NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, k191},
    {"c_saturn", "Saturn", BO_SET_PLANET, 6, 0, 3, 5, 3, -1, -1, 1, 0, 0, 0, 0, 1, 0, 0, 0, NULL, "Hand Upgrade", NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, k192},
    {"c_uranus", "Uranus", BO_SET_PLANET, 7, 0, 3, 6, 3, -1, -1, 1, 0, 0, 0, 0, 1, 0, 0, 0, NULL, "Hand Upgrade", NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, k193},
    {"c_neptune", "Neptune", BO_SET_PLANET, 8, 0, 3, 7, 3, -1, -1, 1, 0, 0, 0, 0, 1, 0, 0, 0, NULL, "Hand Upgrade", NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, k194},
    {"c_pluto", "Pluto", BO_SET_PLANET, 9, 0, 3, 8, 3, -1, -1, 1, 0, 0, 0, 0, 1, 0, 0, 0, NULL, "Hand Upgrade", NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, k195},
    {"c_planet_x", "Planet X", BO_SET_PLANET, 10, 0, 3, 9, 2, -1, -1, 1, 0, 0, 0, 0, 1, 0, 0, 0, NULL, "Hand Upgrade", NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 2, k196},
    {"c_ceres", "Ceres", BO_SET_PLANET, 11, 0, 3, 8, 2, -1, -1, 1, 0, 0, 0, 0, 1, 0, 0, 0, NULL, "Hand Upgrade", NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 2, k197},
    {"c_eris", "Eris", BO_SET_PLANET, 12, 0, 3, 3, 2, -1, -1, 1, 0, 0, 0, 0, 1, 0, 0, 0, NULL, "Hand Upgrade", NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 2, k198},
    {"c_familiar", "Familiar", BO_SET_SPECTRAL, 1, 0, 4, 0, 4, -1, -1, 1, 0, 0, 0, 0, 1, 0, 0, 0, NULL, NULL, NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 3, 2, k199},
    {"c_grim", "Grim", BO_SET_SPECTRAL, 2, 0, 4, 1, 4, -1, -1, 1, 0, 0, 0, 0, 1, 0, 0, 0, NULL, NULL, NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 2, 2, k200},
    {"c_incantation", "Incantation", BO_SET_SPECTRAL, 3, 0, 4, 2, 4, -1, -1, 1, 0, 0, 0, 0, 1, 0, 0, 0, NULL, NULL, NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 4, 2, k201},
    {"c_talisman", "Talisman", BO_SET_SPECTRAL, 4, 0, 4, 3, 4, -1, -1, 1, 0, 0, 0, 0, 1, 0, 0, 0, NULL, NULL, NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 2, k202},
    {"c_aura", "Aura", BO_SET_SPECTRAL, 5, 0, 4, 4, 4, -1, -1, 1, 0, 0, 0, 0, 1, 0, 0, 0, NULL, NULL, NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, NULL},
    {"c_wraith", "Wraith", BO_SET_SPECTRAL, 6, 0, 4, 5, 4, -1, -1, 1, 0, 0, 0, 0, 1, 0, 0, 0, NULL, NULL, NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, NULL},
    {"c_sigil", "Sigil", BO_SET_SPECTRAL, 7, 0, 4, 6, 4, -1, -1, 1, 0, 0, 0, 0, 1, 0, 0, 0, NULL, NULL, NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, NULL},
    {"c_ouija", "Ouija", BO_SET_SPECTRAL, 8, 0, 4, 7, 4, -1, -1, 1, 0, 0, 0, 0, 1, 0, 0, 0, NULL, NULL, NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, NULL},
    {"c_ectoplasm", "Ectoplasm", BO_SET_SPECTRAL, 9, 0, 4, 8, 4, -1, -1, 1, 0, 0, 0, 0, 1, 0, 0, 0, NULL, NULL, NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, NULL},
    {"c_immolate", "Immolate", BO_SET_SPECTRAL, 10, 0, 4, 9, 4, -1, -1, 1, 0, 0, 0, 0, 1, 0, 0, 0, NULL, NULL, NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 3, k208},
    {"c_ankh", "Ankh", BO_SET_SPECTRAL, 11, 0, 4, 0, 5, -1, -1, 1, 0, 0, 0, 0, 1, 0, 0, 0, NULL, NULL, NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 2, 1, k209},
    {"c_deja_vu", "Deja Vu", BO_SET_SPECTRAL, 12, 0, 4, 1, 5, -1, -1, 1, 0, 0, 0, 0, 1, 0, 0, 0, NULL, NULL, NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 2, k210},
    {"c_hex", "Hex", BO_SET_SPECTRAL, 13, 0, 4, 2, 5, -1, -1, 1, 0, 0, 0, 0, 1, 0, 0, 0, NULL, NULL, NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 2, 1, k211},
    {"c_trance", "Trance", BO_SET_SPECTRAL, 14, 0, 4, 3, 5, -1, -1, 1, 0, 0, 0, 0, 1, 0, 0, 0, NULL, NULL, NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 2, k212},
    {"c_medium", "Medium", BO_SET_SPECTRAL, 15, 0, 4, 4, 5, -1, -1, 1, 0, 0, 0, 0, 1, 0, 0, 0, NULL, NULL, NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 2, k213},
    {"c_cryptid", "Cryptid", BO_SET_SPECTRAL, 16, 0, 4, 5, 5, -1, -1, 1, 0, 0, 0, 0, 1, 0, 0, 0, NULL, NULL, NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 2, 2, k214},
    {"c_soul", "The Soul", BO_SET_SPECTRAL, 17, 0, 4, 2, 2, -1, -1, 1, 0, 0, 0, 0, 1, 1, 0, 0, NULL, "Unlocker", NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, NULL},
    {"c_black_hole", "Black Hole", BO_SET_SPECTRAL, 18, 0, 4, 9, 3, -1, -1, 1, 0, 0, 0, 0, 1, 1, 0, 0, NULL, NULL, NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, NULL},
    {"v_overstock_norm", "Overstock", BO_SET_VOUCHER, 1, 0, 10, 0, 0, -1, -1, 1, 0, 0, 0, 0, 0, 0, 0, 0, NULL, NULL, NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, NULL},
    {"v_overstock_plus", "Overstock Plus", BO_SET_VOUCHER, 2, 0, 10, 0, 1, -1, -1, 0, 0, 0, 0, 0, 0, 0, 0, 0, NULL, NULL, NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, k218},
    {"v_clearance_sale", "Clearance Sale", BO_SET_VOUCHER, 3, 0, 10, 3, 0, -1, -1, 1, 0, 0, 0, 0, 0, 0, 0, 0, NULL, NULL, NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 25, 1, k219},
    {"v_liquidation", "Liquidation", BO_SET_VOUCHER, 4, 0, 10, 3, 1, -1, -1, 0, 0, 0, 0, 0, 0, 0, 0, 0, NULL, NULL, NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 50, 2, k220},
    {"v_hone", "Hone", BO_SET_VOUCHER, 5, 0, 10, 4, 0, -1, -1, 1, 0, 0, 0, 0, 0, 0, 0, 0, NULL, NULL, NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 2, 1, k221},
    {"v_glow_up", "Glow Up", BO_SET_VOUCHER, 6, 0, 10, 4, 1, -1, -1, 0, 0, 0, 0, 0, 0, 0, 0, 0, NULL, NULL, NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 4, 2, k222},
    {"v_reroll_surplus", "Reroll Surplus", BO_SET_VOUCHER, 7, 0, 10, 0, 2, -1, -1, 1, 0, 0, 0, 0, 0, 0, 0, 0, NULL, NULL, NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 2, 1, k223},
    {"v_reroll_glut", "Reroll Glut", BO_SET_VOUCHER, 8, 0, 10, 0, 3, -1, -1, 0, 0, 0, 0, 0, 0, 0, 0, 0, NULL, NULL, NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 2, 2, k224},
    {"v_crystal_ball", "Crystal Ball", BO_SET_VOUCHER, 9, 0, 10, 2, 2, -1, -1, 1, 0, 0, 0, 0, 0, 0, 0, 0, NULL, NULL, NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 3, 1, k225},
    {"v_omen_globe", "Omen Globe", BO_SET_VOUCHER, 10, 0, 10, 2, 3, -1, -1, 0, 0, 0, 0, 0, 0, 0, 0, 0, NULL, NULL, NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 4, 2, k226},
    {"v_telescope", "Telescope", BO_SET_VOUCHER, 11, 0, 10, 3, 2, -1, -1, 1, 0, 0, 0, 0, 0, 0, 0, 0, NULL, NULL, NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 3, 1, k227},
    {"v_observatory", "Observatory", BO_SET_VOUCHER, 12, 0, 10, 3, 3, -1, -1, 0, 0, 0, 0, 0, 0, 0, 0, 0, NULL, NULL, NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1.5, 2, k228},
    {"v_grabber", "Grabber", BO_SET_VOUCHER, 13, 0, 10, 5, 0, -1, -1, 1, 0, 0, 0, 0, 0, 0, 0, 0, NULL, NULL, NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 1, k229},
    {"v_nacho_tong", "Nacho Tong", BO_SET_VOUCHER, 14, 0, 10, 5, 1, -1, -1, 0, 0, 0, 0, 0, 0, 0, 0, 0, NULL, NULL, NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 2, k230},
    {"v_wasteful", "Wasteful", BO_SET_VOUCHER, 15, 0, 10, 6, 0, -1, -1, 1, 0, 0, 0, 0, 0, 0, 0, 0, NULL, NULL, NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 1, k231},
    {"v_recyclomancy", "Recyclomancy", BO_SET_VOUCHER, 16, 0, 10, 6, 1, -1, -1, 0, 0, 0, 0, 0, 0, 0, 0, 0, NULL, NULL, NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 2, k232},
    {"v_tarot_merchant", "Tarot Merchant", BO_SET_VOUCHER, 17, 0, 10, 1, 0, -1, -1, 1, 0, 0, 0, 0, 0, 0, 0, 0, NULL, NULL, NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 2.3999999999999999, 2, k233},
    {"v_tarot_tycoon", "Tarot Tycoon", BO_SET_VOUCHER, 18, 0, 10, 1, 1, -1, -1, 0, 0, 0, 0, 0, 0, 0, 0, 0, NULL, NULL, NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 8, 3, k234},
    {"v_planet_merchant", "Planet Merchant", BO_SET_VOUCHER, 19, 0, 10, 2, 0, -1, -1, 1, 0, 0, 0, 0, 0, 0, 0, 0, NULL, NULL, NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 2.3999999999999999, 2, k235},
    {"v_planet_tycoon", "Planet Tycoon", BO_SET_VOUCHER, 20, 0, 10, 2, 1, -1, -1, 0, 0, 0, 0, 0, 0, 0, 0, 0, NULL, NULL, NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 8, 3, k236},
    {"v_seed_money", "Seed Money", BO_SET_VOUCHER, 21, 0, 10, 1, 2, -1, -1, 1, 0, 0, 0, 0, 0, 0, 0, 0, NULL, NULL, NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 50, 1, k237},
    {"v_money_tree", "Money Tree", BO_SET_VOUCHER, 22, 0, 10, 1, 3, -1, -1, 0, 0, 0, 0, 0, 0, 0, 0, 0, NULL, NULL, NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 100, 2, k238},
    {"v_blank", "Blank", BO_SET_VOUCHER, 23, 0, 10, 7, 0, -1, -1, 1, 0, 0, 0, 0, 0, 0, 0, 0, NULL, NULL, NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 5, 1, k239},
    {"v_antimatter", "Antimatter", BO_SET_VOUCHER, 24, 0, 10, 7, 1, -1, -1, 0, 0, 0, 0, 0, 0, 0, 0, 0, NULL, NULL, NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 15, 2, k240},
    {"v_magic_trick", "Magic Trick", BO_SET_VOUCHER, 25, 0, 10, 4, 2, -1, -1, 1, 0, 0, 0, 0, 0, 0, 0, 0, NULL, NULL, NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 4, 1, k241},
    {"v_illusion", "Illusion", BO_SET_VOUCHER, 26, 0, 10, 4, 3, -1, -1, 0, 0, 0, 0, 0, 0, 0, 0, 0, NULL, NULL, NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 4, 2, k242},
    {"v_hieroglyph", "Hieroglyph", BO_SET_VOUCHER, 27, 0, 10, 5, 2, -1, -1, 1, 0, 0, 0, 0, 0, 0, 0, 0, NULL, NULL, NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 1, k243},
    {"v_petroglyph", "Petroglyph", BO_SET_VOUCHER, 28, 0, 10, 5, 3, -1, -1, 0, 0, 0, 0, 0, 0, 0, 0, 0, NULL, NULL, NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 2, k244},
    {"v_directors_cut", "Director's Cut", BO_SET_VOUCHER, 29, 0, 10, 6, 2, -1, -1, 1, 0, 0, 0, 0, 0, 0, 0, 0, NULL, NULL, NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 10, 1, k245},
    {"v_retcon", "Retcon", BO_SET_VOUCHER, 30, 0, 10, 6, 3, -1, -1, 0, 0, 0, 0, 0, 0, 0, 0, 0, NULL, NULL, NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 10, 2, k246},
    {"v_paint_brush", "Paint Brush", BO_SET_VOUCHER, 31, 0, 10, 7, 2, -1, -1, 1, 0, 0, 0, 0, 0, 0, 0, 0, NULL, NULL, NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 1, k247},
    {"v_palette", "Palette", BO_SET_VOUCHER, 32, 0, 10, 7, 3, -1, -1, 0, 0, 0, 0, 0, 0, 0, 0, 0, NULL, NULL, NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 2, k248},
    {"b_red", "Red Deck", BO_SET_BACK, 1, 0, 0, 0, 0, -1, -1, 1, 0, 0, 0, 0, 0, 0, 0, 0, NULL, NULL, NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, k249},
    {"b_blue", "Blue Deck", BO_SET_BACK, 2, 0, 0, 0, 2, -1, -1, 0, 0, 0, 0, 0, 0, 0, 0, 0, NULL, NULL, NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, k250},
    {"b_yellow", "Yellow Deck", BO_SET_BACK, 3, 0, 0, 1, 2, -1, -1, 0, 0, 0, 0, 0, 0, 0, 0, 0, NULL, NULL, NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, k251},
    {"b_green", "Green Deck", BO_SET_BACK, 4, 0, 0, 2, 2, -1, -1, 0, 0, 0, 0, 0, 0, 0, 0, 0, NULL, NULL, NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 3, k252},
    {"b_black", "Black Deck", BO_SET_BACK, 5, 0, 0, 3, 2, -1, -1, 0, 0, 0, 0, 0, 0, 0, 0, 0, NULL, NULL, NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 2, k253},
    {"b_magic", "Magic Deck", BO_SET_BACK, 6, 0, 0, 0, 3, -1, -1, 0, 0, 0, 0, 0, 0, 0, 0, 0, NULL, NULL, NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 3, k254},
    {"b_nebula", "Nebula Deck", BO_SET_BACK, 7, 0, 0, 3, 0, -1, -1, 0, 0, 0, 0, 0, 0, 0, 0, 0, NULL, NULL, NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 2, k255},
    {"b_ghost", "Ghost Deck", BO_SET_BACK, 8, 0, 0, 6, 2, -1, -1, 0, 0, 0, 0, 0, 0, 0, 0, 0, NULL, NULL, NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 2, k256},
    {"b_abandoned", "Abandoned Deck", BO_SET_BACK, 9, 0, 0, 3, 3, -1, -1, 0, 0, 0, 0, 0, 0, 0, 0, 0, NULL, NULL, NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, k257},
    {"b_checkered", "Checkered Deck", BO_SET_BACK, 10, 0, 0, 1, 3, -1, -1, 0, 0, 0, 0, 0, 0, 0, 0, 0, NULL, NULL, NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, NULL},
    {"b_zodiac", "Zodiac Deck", BO_SET_BACK, 11, 0, 0, 3, 4, -1, -1, 0, 0, 0, 0, 0, 0, 0, 0, 0, NULL, NULL, NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 3, k259},
    {"b_painted", "Painted Deck", BO_SET_BACK, 12, 0, 0, 4, 3, -1, -1, 0, 0, 0, 0, 0, 0, 0, 0, 0, NULL, NULL, NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 2, k260},
    {"b_anaglyph", "Anaglyph Deck", BO_SET_BACK, 13, 0, 0, 2, 4, -1, -1, 0, 0, 0, 0, 0, 0, 0, 0, 0, NULL, NULL, NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, NULL},
    {"b_plasma", "Plasma Deck", BO_SET_BACK, 14, 0, 0, 4, 2, -1, -1, 0, 0, 0, 0, 0, 0, 0, 0, 0, NULL, NULL, NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, k262},
    {"b_erratic", "Erratic Deck", BO_SET_BACK, 15, 0, 0, 2, 3, -1, -1, 0, 0, 0, 0, 0, 0, 0, 0, 0, NULL, NULL, NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, k263},
    {"b_challenge", "Challenge Deck", BO_SET_BACK, 16, 0, 0, 0, 4, -1, -1, 1, 0, 0, 0, 0, 0, 0, 1, 0, NULL, NULL, NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, NULL},
    {"p_arcana_normal_1", "Arcana Pack", BO_SET_BOOSTER, 1, 0, 4, 0, 0, -1, -1, 1, 0, 0, 0, 0, 0, 0, 0, 1, "Arcana", NULL, NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 3, 2, k265},
    {"p_arcana_normal_2", "Arcana Pack", BO_SET_BOOSTER, 2, 0, 4, 1, 0, -1, -1, 1, 0, 0, 0, 0, 0, 0, 0, 1, "Arcana", NULL, NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 3, 2, k266},
    {"p_arcana_normal_3", "Arcana Pack", BO_SET_BOOSTER, 3, 0, 4, 2, 0, -1, -1, 1, 0, 0, 0, 0, 0, 0, 0, 1, "Arcana", NULL, NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 3, 2, k267},
    {"p_arcana_normal_4", "Arcana Pack", BO_SET_BOOSTER, 4, 0, 4, 3, 0, -1, -1, 1, 0, 0, 0, 0, 0, 0, 0, 1, "Arcana", NULL, NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 3, 2, k268},
    {"p_arcana_jumbo_1", "Jumbo Arcana Pack", BO_SET_BOOSTER, 5, 0, 6, 0, 2, -1, -1, 1, 0, 0, 0, 0, 0, 0, 0, 1, "Arcana", NULL, NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 5, 2, k269},
    {"p_arcana_jumbo_2", "Jumbo Arcana Pack", BO_SET_BOOSTER, 6, 0, 6, 1, 2, -1, -1, 1, 0, 0, 0, 0, 0, 0, 0, 1, "Arcana", NULL, NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 5, 2, k270},
    {"p_arcana_mega_1", "Mega Arcana Pack", BO_SET_BOOSTER, 7, 0, 8, 2, 2, -1, -1, 1, 0, 0, 0, 0, 0, 0, 0, 0.25, "Arcana", NULL, NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 5, 2, k271},
    {"p_arcana_mega_2", "Mega Arcana Pack", BO_SET_BOOSTER, 8, 0, 8, 3, 2, -1, -1, 1, 0, 0, 0, 0, 0, 0, 0, 0.25, "Arcana", NULL, NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 5, 2, k272},
    {"p_celestial_normal_1", "Celestial Pack", BO_SET_BOOSTER, 9, 0, 4, 0, 1, -1, -1, 1, 0, 0, 0, 0, 0, 0, 0, 1, "Celestial", NULL, NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 3, 2, k273},
    {"p_celestial_normal_2", "Celestial Pack", BO_SET_BOOSTER, 10, 0, 4, 1, 1, -1, -1, 1, 0, 0, 0, 0, 0, 0, 0, 1, "Celestial", NULL, NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 3, 2, k274},
    {"p_celestial_normal_3", "Celestial Pack", BO_SET_BOOSTER, 11, 0, 4, 2, 1, -1, -1, 1, 0, 0, 0, 0, 0, 0, 0, 1, "Celestial", NULL, NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 3, 2, k275},
    {"p_celestial_normal_4", "Celestial Pack", BO_SET_BOOSTER, 12, 0, 4, 3, 1, -1, -1, 1, 0, 0, 0, 0, 0, 0, 0, 1, "Celestial", NULL, NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 3, 2, k276},
    {"p_celestial_jumbo_1", "Jumbo Celestial Pack", BO_SET_BOOSTER, 13, 0, 6, 0, 3, -1, -1, 1, 0, 0, 0, 0, 0, 0, 0, 1, "Celestial", NULL, NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 5, 2, k277},
    {"p_celestial_jumbo_2", "Jumbo Celestial Pack", BO_SET_BOOSTER, 14, 0, 6, 1, 3, -1, -1, 1, 0, 0, 0, 0, 0, 0, 0, 1, "Celestial", NULL, NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 5, 2, k278},
    {"p_celestial_mega_1", "Mega Celestial Pack", BO_SET_BOOSTER, 15, 0, 8, 2, 3, -1, -1, 1, 0, 0, 0, 0, 0, 0, 0, 0.25, "Celestial", NULL, NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 5, 2, k279},
    {"p_celestial_mega_2", "Mega Celestial Pack", BO_SET_BOOSTER, 16, 0, 8, 3, 3, -1, -1, 1, 0, 0, 0, 0, 0, 0, 0, 0.25, "Celestial", NULL, NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 5, 2, k280},
    {"p_standard_normal_1", "Standard Pack", BO_SET_BOOSTER, 17, 0, 4, 0, 6, -1, -1, 1, 0, 0, 0, 0, 0, 0, 0, 1, "Standard", NULL, NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 3, 2, k281},
    {"p_standard_normal_2", "Standard Pack", BO_SET_BOOSTER, 18, 0, 4, 1, 6, -1, -1, 1, 0, 0, 0, 0, 0, 0, 0, 1, "Standard", NULL, NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 3, 2, k282},
    {"p_standard_normal_3", "Standard Pack", BO_SET_BOOSTER, 19, 0, 4, 2, 6, -1, -1, 1, 0, 0, 0, 0, 0, 0, 0, 1, "Standard", NULL, NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 3, 2, k283},
    {"p_standard_normal_4", "Standard Pack", BO_SET_BOOSTER, 20, 0, 4, 3, 6, -1, -1, 1, 0, 0, 0, 0, 0, 0, 0, 1, "Standard", NULL, NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 3, 2, k284},
    {"p_standard_jumbo_1", "Jumbo Standard Pack", BO_SET_BOOSTER, 21, 0, 6, 0, 7, -1, -1, 1, 0, 0, 0, 0, 0, 0, 0, 1, "Standard", NULL, NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 5, 2, k285},
    {"p_standard_jumbo_2", "Jumbo Standard Pack", BO_SET_BOOSTER, 22, 0, 6, 1, 7, -1, -1, 1, 0, 0, 0, 0, 0, 0, 0, 1, "Standard", NULL, NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 5, 2, k286},
    {"p_standard_mega_1", "Mega Standard Pack", BO_SET_BOOSTER, 23, 0, 8, 2, 7, -1, -1, 1, 0, 0, 0, 0, 0, 0, 0, 0.25, "Standard", NULL, NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 5, 2, k287},
    {"p_standard_mega_2", "Mega Standard Pack", BO_SET_BOOSTER, 24, 0, 8, 3, 7, -1, -1, 1, 0, 0, 0, 0, 0, 0, 0, 0.25, "Standard", NULL, NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 5, 2, k288},
    {"p_buffoon_normal_1", "Buffoon Pack", BO_SET_BOOSTER, 25, 0, 4, 0, 8, -1, -1, 1, 0, 0, 0, 0, 0, 0, 0, 0.59999999999999998, "Buffoon", NULL, NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 2, 2, k289},
    {"p_buffoon_normal_2", "Buffoon Pack", BO_SET_BOOSTER, 26, 0, 4, 1, 8, -1, -1, 1, 0, 0, 0, 0, 0, 0, 0, 0.59999999999999998, "Buffoon", NULL, NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 2, 2, k290},
    {"p_buffoon_jumbo_1", "Jumbo Buffoon Pack", BO_SET_BOOSTER, 27, 0, 6, 2, 8, -1, -1, 1, 0, 0, 0, 0, 0, 0, 0, 0.59999999999999998, "Buffoon", NULL, NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 4, 2, k291},
    {"p_buffoon_mega_1", "Mega Buffoon Pack", BO_SET_BOOSTER, 28, 0, 8, 3, 8, -1, -1, 1, 0, 0, 0, 0, 0, 0, 0, 0.14999999999999999, "Buffoon", NULL, NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 4, 2, k292},
    {"p_spectral_normal_1", "Spectral Pack", BO_SET_BOOSTER, 29, 0, 4, 0, 4, -1, -1, 1, 0, 0, 0, 0, 0, 0, 0, 0.29999999999999999, "Spectral", NULL, NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 2, 2, k293},
    {"p_spectral_normal_2", "Spectral Pack", BO_SET_BOOSTER, 30, 0, 4, 1, 4, -1, -1, 1, 0, 0, 0, 0, 0, 0, 0, 0.29999999999999999, "Spectral", NULL, NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 2, 2, k294},
    {"p_spectral_jumbo_1", "Jumbo Spectral Pack", BO_SET_BOOSTER, 31, 0, 6, 2, 4, -1, -1, 1, 0, 0, 0, 0, 0, 0, 0, 0.29999999999999999, "Spectral", NULL, NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 4, 2, k295},
    {"p_spectral_mega_1", "Mega Spectral Pack", BO_SET_BOOSTER, 32, 0, 8, 3, 4, -1, -1, 1, 0, 0, 0, 0, 0, 0, 0, 0.070000000000000007, "Spectral", NULL, NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 4, 2, k296},
    {"tag_uncommon", "Uncommon Tag", BO_SET_TAG, 1, 0, 0, 0, 0, -1, -1, 1, 0, 0, 0, 0, 0, 0, 0, 0, NULL, NULL, NULL, NULL, NULL, "store_joker_create", 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, NULL},
    {"tag_rare", "Rare Tag", BO_SET_TAG, 2, 0, 0, 1, 0, -1, -1, 1, 0, 0, 0, 0, 0, 0, 0, 0, NULL, NULL, NULL, NULL, NULL, "store_joker_create", 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 2, k298},
    {"tag_negative", "Negative Tag", BO_SET_TAG, 3, 0, 0, 2, 0, -1, -1, 1, 0, 0, 0, 2, 0, 0, 0, 0, NULL, NULL, NULL, NULL, NULL, "store_joker_modify", 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 3, k299},
    {"tag_foil", "Foil Tag", BO_SET_TAG, 4, 0, 0, 3, 0, -1, -1, 1, 0, 0, 0, 0, 0, 0, 0, 0, NULL, NULL, NULL, NULL, NULL, "store_joker_modify", 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 3, k300},
    {"tag_holo", "Holographic Tag", BO_SET_TAG, 5, 0, 0, 0, 1, -1, -1, 1, 0, 0, 0, 0, 0, 0, 0, 0, NULL, NULL, NULL, NULL, NULL, "store_joker_modify", 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 3, k301},
    {"tag_polychrome", "Polychrome Tag", BO_SET_TAG, 6, 0, 0, 1, 1, -1, -1, 1, 0, 0, 0, 0, 0, 0, 0, 0, NULL, NULL, NULL, NULL, NULL, "store_joker_modify", 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 3, k302},
    {"tag_investment", "Investment Tag", BO_SET_TAG, 7, 0, 0, 2, 1, -1, -1, 1, 0, 0, 0, 0, 0, 0, 0, 0, NULL, NULL, NULL, NULL, NULL, "eval", 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, k303},
    {"tag_voucher", "Voucher Tag", BO_SET_TAG, 8, 0, 0, 3, 1, -1, -1, 1, 0, 0, 0, 0, 0, 0, 0, 0, NULL, NULL, NULL, NULL, NULL, "voucher_add", 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, NULL},
    {"tag_boss", "Boss Tag", BO_SET_TAG, 9, 0, 0, 0, 2, -1, -1, 1, 0, 0, 0, 0, 0, 0, 0, 0, NULL, NULL, NULL, NULL, NULL, "new_blind_choice", 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, NULL},
    {"tag_standard", "Standard Tag", BO_SET_TAG, 10, 0, 0, 1, 2, -1, -1, 1, 0, 0, 0, 2, 0, 0, 0, 0, NULL, NULL, NULL, NULL, NULL, "new_blind_choice", 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, NULL},
    {"tag_charm", "Charm Tag", BO_SET_TAG, 11, 0, 0, 2, 2, -1, -1, 1, 0, 0, 0, 0, 0, 0, 0, 0, NULL, NULL, NULL, NULL, NULL, "new_blind_choice", 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, NULL},
    {"tag_meteor", "Meteor Tag", BO_SET_TAG, 12, 0, 0, 3, 2, -1, -1, 1, 0, 0, 0, 2, 0, 0, 0, 0, NULL, NULL, NULL, NULL, NULL, "new_blind_choice", 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, NULL},
    {"tag_buffoon", "Buffoon Tag", BO_SET_TAG, 13, 0, 0, 4, 2, -1, -1, 1, 0, 0, 0, 2, 0, 0, 0, 0, NULL, NULL, NULL, NULL, NULL, "new_blind_choice", 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, NULL},
    {"tag_handy", "Handy Tag", BO_SET_TAG, 14, 0, 0, 1, 3, -1, -1, 1, 0, 0, 0, 2, 0, 0, 0, 0, NULL, NULL, NULL, NULL, NULL, "immediate", 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, k310},
    {"tag_garbage", "Garbage Tag", BO_SET_TAG, 15, 0, 0, 2, 3, -1, -1, 1, 0, 0, 0, 2, 0, 0, 0, 0, NULL, NULL, NULL, NULL, NULL, "immediate", 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, k311},
    {"tag_ethereal", "Ethereal Tag", BO_SET_TAG, 16, 0, 0, 3, 3, -1, -1, 1, 0, 0, 0, 2, 0, 0, 0, 0, NULL, NULL, NULL, NULL, NULL, "new_blind_choice", 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, NULL},
    {"tag_coupon", "Coupon Tag", BO_SET_TAG, 17, 0, 0, 4, 0, -1, -1, 1, 0, 0, 0, 0, 0, 0, 0, 0, NULL, NULL, NULL, NULL, NULL, "shop_final_pass", 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, NULL},
    {"tag_double", "Double Tag", BO_SET_TAG, 18, 0, 0, 5, 0, -1, -1, 1, 0, 0, 0, 0, 0, 0, 0, 0, NULL, NULL, NULL, NULL, NULL, "tag_add", 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, NULL},
    {"tag_juggle", "Juggle Tag", BO_SET_TAG, 19, 0, 0, 5, 1, -1, -1, 1, 0, 0, 0, 0, 0, 0, 0, 0, NULL, NULL, NULL, NULL, NULL, "round_start_bonus", 0, 0, 0, 0, 0, 0, 0, 1, 3, 0, 0, 0, 0, NULL},
    {"tag_d_six", "D6 Tag", BO_SET_TAG, 20, 0, 0, 5, 3, -1, -1, 1, 0, 0, 0, 0, 0, 0, 0, 0, NULL, NULL, NULL, NULL, NULL, "shop_start", 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, NULL},
    {"tag_top_up", "Top-up Tag", BO_SET_TAG, 21, 0, 0, 4, 1, -1, -1, 1, 0, 0, 0, 2, 0, 0, 0, 0, NULL, NULL, NULL, NULL, NULL, "immediate", 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, k317},
    {"tag_skip", "Skip Tag", BO_SET_TAG, 22, 0, 0, 0, 3, -1, -1, 1, 0, 0, 0, 0, 0, 0, 0, 0, NULL, NULL, NULL, NULL, NULL, "immediate", 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, k318},
    {"tag_orbital", "Orbital Tag", BO_SET_TAG, 23, 0, 0, 5, 2, -1, -1, 1, 0, 0, 0, 2, 0, 0, 0, 0, NULL, NULL, NULL, NULL, NULL, "immediate", 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, k319},
    {"tag_economy", "Economy Tag", BO_SET_TAG, 24, 0, 0, 4, 3, -1, -1, 1, 0, 0, 0, 0, 0, 0, 0, 0, NULL, NULL, NULL, NULL, NULL, "immediate", 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, k320},
};

const BOCenterInfo *bo_center(BOCenterId id) {
    if (id < 0 || id >= BO_CENTER_COUNT) return NULL;
    return &kCenters[id];
}

BOCenterId bo_center_by_key(const char *key) {
    for (int i = 0; i < BO_CENTER_COUNT; i++) {
        if (strcmp(kCenters[i].key, key) == 0) return (BOCenterId)i;
    }
    return BO_CENTER_NONE;
}

double bo_center_number(const BOCenterInfo *center, BOField field, int *has) {
    for (int i = 0; i < center->config_count; i++) {
        if (center->config[i].field == field) {
            if (has) *has = 1;
            return center->config[i].number;
        }
    }
    if (has) *has = 0;
    return 0;
}

const char *bo_center_text(const BOCenterInfo *center, BOField field) {
    for (int i = 0; i < center->config_count; i++) {
        if (center->config[i].field == field) return center->config[i].text;
    }
    return NULL;
}
