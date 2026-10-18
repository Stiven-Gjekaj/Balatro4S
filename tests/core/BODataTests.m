// Tests of bo_data, the tables that tools/gen_data.lua makes.

#import "BOTest.h"
#include "bo_data.h"

BO_TEST(test_data_has_each_card_of_balatro) {
    int counts[BO_SET_TAG + 1] = {0};
    for (int i = 0; i < BO_CENTER_COUNT; i++) counts[bo_center((BOCenterId)i)->set]++;
    BO_EQUAL_INT(counts[BO_SET_JOKER], 150);
    BO_EQUAL_INT(counts[BO_SET_TAROT], 22);
    BO_EQUAL_INT(counts[BO_SET_PLANET], 12);
    BO_EQUAL_INT(counts[BO_SET_SPECTRAL], 18);
    BO_EQUAL_INT(counts[BO_SET_VOUCHER], 32);
    BO_EQUAL_INT(counts[BO_SET_BOOSTER], 32);
    BO_EQUAL_INT(counts[BO_SET_TAG], 24);
    BO_EQUAL_INT(counts[BO_SET_ENHANCED], 8);
}

BO_TEST(test_data_reads_the_config_of_a_card) {
    const BOCenterInfo *greedy = bo_center(BO_J_GREEDY_JOKER);
    BO_EQUAL(@(greedy->key), @"j_greedy_joker");
    BO_EQUAL_INT(greedy->cost, 5);
    BO_EQUAL_INT(bo_center_number(greedy, BO_F_EXTRA_S_MULT, NULL), 3);
    BO_EQUAL(@(bo_center_text(greedy, BO_F_EXTRA_SUIT)), @"Diamonds");
    int has = 1;
    bo_center_number(greedy, BO_F_EXTRA_ODDS, &has);
    BO_EXPECT(!has);
    BO_EQUAL_INT(bo_center(BO_J_JOKER)->mult, 4);
    BO_EQUAL(@(bo_center(BO_J_JOLLY)->hand_type), @"Pair");
    BO_EQUAL_INT(bo_center_by_key("j_blueprint"), BO_J_BLUEPRINT);
    BO_EQUAL_INT(bo_center_by_key("j_nothing"), BO_CENTER_NONE);
}

BO_TEST(test_data_keeps_the_jokers_in_their_order) {
    // The pools of Balatro sort the jokers by their order field.
    int last = 0;
    for (int i = 0; i < BO_CENTER_COUNT; i++) {
        const BOCenterInfo *c = bo_center((BOCenterId)i);
        if (c->set != BO_SET_JOKER) continue;
        BO_EXPECT(c->order > last);
        last = c->order;
    }
    BO_EQUAL_INT(last, 150);
}

BO_TEST(test_data_has_the_unlock_conditions) {
    const BOCenterInfo *magic = bo_center(BO_B_MAGIC);
    BO_EQUAL(@(magic->unlock_type), @"win_deck");
    BO_EQUAL(@(magic->unlock_text), @"b_red");
    const BOCenterInfo *glass = bo_center(BO_J_GLASS);
    BO_EQUAL(@(glass->unlock_type), @"modify_deck");
    BO_EXPECT(glass->unlock_number == 5);
    BO_EQUAL(@(glass->unlock_text), @"m_glass");
    BO_EQUAL_INT(glass->unlock_kind, 2);
    const BOCenterInfo *zodiac = bo_center(BO_B_ZODIAC);
    BO_EQUAL(@(zodiac->unlock_type), @"win_stake");
    BO_EXPECT(zodiac->unlock_number == 2);
    BO_EXPECT(bo_center(BO_J_JOKER)->unlock_type == NULL);
}

BO_TEST(test_data_has_the_cards_that_start_discovered) {
    int count = 0;
    for (int i = 0; i < BO_CENTER_COUNT; i++) count += bo_center((BOCenterId)i)->discovered;
    BO_EQUAL_INT(count, 2);
    BO_EXPECT(bo_center(BO_J_JOKER)->discovered);
    BO_EXPECT(bo_center(BO_B_RED)->discovered);
}
