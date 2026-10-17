// Tests of bo_score. The numbers are the numbers that Balatro shows for
// the same cards with no jokers.

#import "BOTest.h"
#include "bo_score.h"

static BOCard card(int rank, BOSuit suit) {
    return bo_card_make(rank, suit);
}

BO_TEST(test_score_adds_the_chips_of_the_scoring_cards_only) {
    BOScoreContext context;
    bo_score_context_init(&context);
    // A pair of Kings with a 9 and a 4: 10 + 10 + 10 chips, 2 mult.
    BOCard played[] = {card(BO_KING, BO_SPADES), card(9, BO_HEARTS), card(BO_KING, BO_CLUBS), card(4, BO_DIAMONDS)};
    BOScore score;
    bo_score_play(&context, played, 4, NULL, 0, &score);
    BO_EQUAL_INT(score.type, BO_HAND_PAIR);
    BO_EQUAL_INT(score.chips, 30);
    BO_EQUAL_INT(score.mult, 2);
    BO_EQUAL_INT(score.total, 60);
    BO_EQUAL_INT(score.event_count, 2);
    BO_EQUAL_INT(score.events[0].index, 0);
    BO_EQUAL_INT(score.events[1].index, 2);
    BO_EQUAL_INT(score.events[1].chips, 30);
}

BO_TEST(test_score_of_an_ace_flush) {
    BOScoreContext context;
    bo_score_context_init(&context);
    BOCard played[] = {card(BO_ACE, BO_HEARTS), card(BO_KING, BO_HEARTS), card(10, BO_HEARTS), card(5, BO_HEARTS),
                       card(4, BO_HEARTS)};
    BOScore score;
    bo_score_play(&context, played, 5, NULL, 0, &score);
    // 35 + 11 + 10 + 10 + 5 + 4 = 75 chips, times 4 mult.
    BO_EQUAL_INT(score.chips, 75);
    BO_EQUAL_INT(score.total, 300);
}

BO_TEST(test_score_uses_the_level_of_the_hand) {
    BOScoreContext context;
    bo_score_context_init(&context);
    context.levels[BO_HAND_HIGH_CARD] = 2;
    BOCard played[] = {card(7, BO_SPADES)};
    BOScore score;
    bo_score_play(&context, played, 1, NULL, 0, &score);
    // Level 2 High Card: 15 chips and 2 mult, then the 7.
    BO_EQUAL_INT(score.base_chips, 15);
    BO_EQUAL_INT(score.chips, 22);
    BO_EQUAL_INT(score.total, 44);
}

BO_TEST(test_score_gives_a_stone_card_50_chips_and_skips_a_debuffed_card) {
    BOScoreContext context;
    bo_score_context_init(&context);
    BOCard played[] = {card(3, BO_SPADES), card(3, BO_HEARTS), card(BO_ACE, BO_CLUBS)};
    played[2].enhancement = BO_ENH_STONE;
    played[1].debuffed = 1;
    BOScore score;
    bo_score_play(&context, played, 3, NULL, 0, &score);
    // Pair: 10 chips, the first 3, no chips from the debuffed 3, and 50.
    BO_EQUAL_INT(score.type, BO_HAND_PAIR);
    BO_EQUAL_INT(score.chips, 63);
}
