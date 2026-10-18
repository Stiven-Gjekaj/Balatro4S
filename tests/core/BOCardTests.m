// Tests of bo_card.

#import "BOTest.h"
#include "bo_card.h"

BO_TEST(test_card_gives_the_chips_of_its_rank) {
    BOCard two = bo_card_make(2, BO_HEARTS);
    BOCard ten = bo_card_make(10, BO_HEARTS);
    BOCard king = bo_card_make(BO_KING, BO_HEARTS);
    BOCard ace = bo_card_make(BO_ACE, BO_HEARTS);
    BO_EQUAL_INT(bo_card_nominal(&two), 2);
    BO_EQUAL_INT(bo_card_nominal(&ten), 10);
    BO_EQUAL_INT(bo_card_nominal(&king), 10);
    BO_EQUAL_INT(bo_card_nominal(&ace), 11);
}

BO_TEST(test_card_orders_the_highest_card_by_rank_then_face_then_suit) {
    BOCard ten = bo_card_make(10, BO_SPADES);
    BOCard jack = bo_card_make(BO_JACK, BO_DIAMONDS);
    BOCard kingD = bo_card_make(BO_KING, BO_DIAMONDS);
    BOCard kingS = bo_card_make(BO_KING, BO_SPADES);
    BO_EXPECT(bo_card_highest_key(&jack) > bo_card_highest_key(&ten));
    BO_EXPECT(bo_card_highest_key(&kingS) > bo_card_highest_key(&kingD));
    BOCard stone = bo_card_make(BO_ACE, BO_SPADES);
    stone.enhancement = BO_ENH_STONE;
    BOCard two = bo_card_make(2, BO_DIAMONDS);
    BO_EXPECT(bo_card_highest_key(&two) > bo_card_highest_key(&stone));
}

BO_TEST(test_card_tests_its_suit_for_a_flush) {
    BOCard heart = bo_card_make(5, BO_HEARTS);
    BO_EXPECT(bo_card_is_suit_for_flush(&heart, BO_HEARTS, 0));
    BO_EXPECT(!bo_card_is_suit_for_flush(&heart, BO_DIAMONDS, 0));
    BO_EXPECT(bo_card_is_suit_for_flush(&heart, BO_DIAMONDS, 1));
    BO_EXPECT(!bo_card_is_suit_for_flush(&heart, BO_SPADES, 1));
    heart.enhancement = BO_ENH_STONE;
    BO_EXPECT(!bo_card_is_suit_for_flush(&heart, BO_HEARTS, 0));
}

BO_TEST(test_card_has_a_short_name) {
    char name[4];
    BOCard ace = bo_card_make(BO_ACE, BO_SPADES);
    BOCard ten = bo_card_make(10, BO_DIAMONDS);
    BO_EQUAL(@(bo_card_name(&ace, name)), @"AS");
    BO_EQUAL(@(bo_card_name(&ten, name)), @"TD");
}

BO_TEST(test_card_sorts_by_suit_before_rank_for_a_suit_sort) {
    BOCard twoS = bo_card_make(2, BO_SPADES);
    BOCard aceH = bo_card_make(BO_ACE, BO_HEARTS);
    BO_EXPECT(bo_card_nominal_key(&aceH, 0) > bo_card_nominal_key(&twoS, 0));
    BO_EXPECT(bo_card_nominal_key(&twoS, 1) > bo_card_nominal_key(&aceH, 1));
    // Two cards with the same rank and suit: the older card is higher.
    BOCard old = bo_card_make(9, BO_CLUBS), young = bo_card_make(9, BO_CLUBS);
    old.uid = 1;
    young.uid = 2;
    BO_EXPECT(bo_card_nominal_key(&old, 0) > bo_card_nominal_key(&young, 0));
}
