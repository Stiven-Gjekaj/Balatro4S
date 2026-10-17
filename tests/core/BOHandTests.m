// Tests of bo_hand. Each case comes from the rules of evaluate_poker_hand
// in Balatro 1.0.1o.

#import "BOTest.h"
#include <string.h>
#include "bo_hand.h"

// Parses cards such as "AS KH TD 9C 2S". A "*" after a card makes it a
// Stone card, and a "w" makes it a Wild card.
static int parse(const char *text, BOCard *cards) {
    static const char kRanks[] = "23456789TJQKA";
    static const char kSuits[] = "SHCD";
    int count = 0;
    for (const char *p = text; *p;) {
        while (*p == ' ') p++;
        if (!*p) break;
        int rank = (int)(strchr(kRanks, p[0]) - kRanks) + 2;
        BOSuit suit = (BOSuit)(strchr(kSuits, p[1]) - kSuits);
        cards[count] = bo_card_make(rank, suit);
        cards[count].uid = (uint32_t)count;
        p += 2;
        if (*p == '*') cards[count].enhancement = BO_ENH_STONE, p++;
        if (*p == 'w') cards[count].enhancement = BO_ENH_WILD, p++;
        count++;
    }
    return count;
}

static BOHandResult evaluate(const char *text, const BOHandRules *rules) {
    BOCard cards[BO_MAX_PLAY];
    int count = parse(text, cards);
    return bo_hand_evaluate(cards, count, rules);
}

static BOHandType type_of(const char *text) {
    return evaluate(text, NULL).type;
}

BO_TEST(test_hand_finds_each_type) {
    BO_EQUAL_INT(type_of("AS"), BO_HAND_HIGH_CARD);
    BO_EQUAL_INT(type_of("9S 9D KH"), BO_HAND_PAIR);
    BO_EQUAL_INT(type_of("AH AD 4H 4C QC"), BO_HAND_TWO_PAIR);
    BO_EQUAL_INT(type_of("TS TC TD 6H 5D"), BO_HAND_THREE_OF_A_KIND);
    BO_EQUAL_INT(type_of("JD TC 9C 8S 7H"), BO_HAND_STRAIGHT);
    BO_EQUAL_INT(type_of("AH KH TH 5H 4H"), BO_HAND_FLUSH);
    BO_EQUAL_INT(type_of("KH KC KD 2S 2D"), BO_HAND_FULL_HOUSE);
    BO_EQUAL_INT(type_of("JS JH JC JD 3C"), BO_HAND_FOUR_OF_A_KIND);
    BO_EQUAL_INT(type_of("QS JS TS 9S 8S"), BO_HAND_STRAIGHT_FLUSH);
    BO_EQUAL_INT(type_of("AS AH AH AC AD"), BO_HAND_FIVE_OF_A_KIND);
    BO_EQUAL_INT(type_of("7D 7D 7D 4D 4D"), BO_HAND_FLUSH_HOUSE);
    BO_EQUAL_INT(type_of("AS AS AS AS AS"), BO_HAND_FLUSH_FIVE);
}

BO_TEST(test_hand_scores_only_the_cards_of_the_hand) {
    BO_EQUAL_INT(evaluate("KS 9S 9D 6H 3D", NULL).scoring, 0x06);
    BO_EQUAL_INT(evaluate("AH AD QC 4H 4C", NULL).scoring, 0x1b);
    BO_EQUAL_INT(evaluate("TS TC TD 6H 5D", NULL).scoring, 0x07);
    BO_EQUAL_INT(evaluate("JS JH JC JD 3C", NULL).scoring, 0x0f);
    // High Card scores only the highest card. An Ace beats a King.
    BO_EQUAL_INT(evaluate("3D KC AS 9D", NULL).scoring, 0x04);
    // Between two Kings, Spades is the highest suit.
    BO_EQUAL_INT(evaluate("KD KS", NULL).type, BO_HAND_PAIR);
    BO_EQUAL_INT(evaluate("KD QS", NULL).scoring, 0x01);
    BO_EQUAL_INT(evaluate("JH QD", NULL).scoring, 0x02);
}

BO_TEST(test_hand_uses_the_highest_pair_of_two_pair_hands) {
    // Two Pair holds both pairs. The Pair bit is also set.
    BOHandResult result = evaluate("4H AH 4C AD 9S", NULL);
    BO_EQUAL_INT(result.type, BO_HAND_TWO_PAIR);
    BO_EQUAL_INT(result.scoring, 0x0f);
    BO_EXPECT(bo_hand_contains(&result, BO_HAND_PAIR));
    BO_EXPECT(!bo_hand_contains(&result, BO_HAND_THREE_OF_A_KIND));
}

BO_TEST(test_hand_reads_an_ace_as_low_or_high_but_not_both) {
    BO_EQUAL_INT(type_of("AS 2D 3H 4C 5S"), BO_HAND_STRAIGHT);
    BO_EQUAL_INT(type_of("AS KD QH JC TS"), BO_HAND_STRAIGHT);
    BO_EQUAL_INT(type_of("QS KD AH 2C 3S"), BO_HAND_HIGH_CARD);
}

BO_TEST(test_hand_marks_a_royal_flush) {
    BOHandResult royal = evaluate("AS KS QS JS TS", NULL);
    BO_EQUAL_INT(royal.type, BO_HAND_STRAIGHT_FLUSH);
    BO_EXPECT(royal.royal);
    BO_EQUAL(@(bo_hand_name(royal.type, royal.royal)), @"Royal Flush");
    BOHandResult low = evaluate("9S KS QS JS TS", NULL);
    BO_EXPECT(!low.royal);
    BO_EQUAL(@(bo_hand_name(low.type, low.royal)), @"Straight Flush");
}

BO_TEST(test_hand_fills_the_table_of_contained_hands) {
    BOHandResult five = evaluate("AS AH AH AC AD", NULL);
    BO_EXPECT(bo_hand_contains(&five, BO_HAND_FOUR_OF_A_KIND));
    BO_EXPECT(bo_hand_contains(&five, BO_HAND_THREE_OF_A_KIND));
    BO_EXPECT(bo_hand_contains(&five, BO_HAND_PAIR));
    BO_EXPECT(!bo_hand_contains(&five, BO_HAND_TWO_PAIR));
    BO_EXPECT(bo_hand_contains(&five, BO_HAND_HIGH_CARD));

    BOHandResult full = evaluate("KH KC KD 2S 2D", NULL);
    BO_EXPECT(bo_hand_contains(&full, BO_HAND_TWO_PAIR));
    BO_EXPECT(bo_hand_contains(&full, BO_HAND_THREE_OF_A_KIND));
    BO_EXPECT(bo_hand_contains(&full, BO_HAND_PAIR));
    BO_EXPECT(!bo_hand_contains(&full, BO_HAND_FLUSH));

    BOHandResult flushHouse = evaluate("7D 7D 7D 4D 4D", NULL);
    BO_EXPECT(bo_hand_contains(&flushHouse, BO_HAND_FULL_HOUSE));
    BO_EXPECT(bo_hand_contains(&flushHouse, BO_HAND_FLUSH));
    BO_EXPECT(!bo_hand_contains(&flushHouse, BO_HAND_FIVE_OF_A_KIND));
}

BO_TEST(test_hand_needs_five_cards_for_a_flush_or_a_straight) {
    BO_EQUAL_INT(type_of("AH KH TH 5H"), BO_HAND_HIGH_CARD);
    BO_EQUAL_INT(type_of("5S 6D 7H 8C"), BO_HAND_HIGH_CARD);
}

BO_TEST(test_hand_obeys_four_fingers_and_shortcut) {
    BOHandRules fingers = {1, 0, 0};
    BOHandResult flush = evaluate("AH KH TH 5H 2S", &fingers);
    BO_EQUAL_INT(flush.type, BO_HAND_FLUSH);
    BO_EQUAL_INT(flush.scoring, 0x0f);
    BOHandResult straight = evaluate("5S 6D 7H 8C KC", &fingers);
    BO_EQUAL_INT(straight.type, BO_HAND_STRAIGHT);
    BO_EQUAL_INT(straight.scoring, 0x0f);

    BOHandRules shortcut = {0, 1, 0};
    BO_EQUAL_INT(evaluate("2S 4D 6H 8C TC", &shortcut).type, BO_HAND_STRAIGHT);
    BO_EQUAL_INT(evaluate("2S 4D 6H 8C QC", &shortcut).type, BO_HAND_HIGH_CARD);
    // A skip cannot be two ranks in a row.
    BO_EQUAL_INT(evaluate("2S 3D 4H 5C 8C", &shortcut).type, BO_HAND_HIGH_CARD);
}

BO_TEST(test_hand_obeys_smeared_joker) {
    BOHandRules smeared = {0, 0, 1};
    BO_EQUAL_INT(evaluate("AH KD TH 5D 4H", &smeared).type, BO_HAND_FLUSH);
    BO_EQUAL_INT(evaluate("AH KD TH 5D 4S", &smeared).type, BO_HAND_HIGH_CARD);
}

BO_TEST(test_hand_treats_wild_and_stone_cards_as_balatro_does) {
    // A Wild card is each suit for a flush.
    BO_EQUAL_INT(type_of("AH KH THw 5S 4H"), BO_HAND_HIGH_CARD);
    BO_EQUAL_INT(type_of("AH KH TSw 5H 4H"), BO_HAND_FLUSH);
    // A debuffed Wild card keeps its suit for a flush.
    BOCard cards[BO_MAX_PLAY];
    int count = parse("AH KH TSw 5H 4H", cards);
    cards[2].debuffed = 1;
    BO_EQUAL_INT(bo_hand_evaluate(cards, count, NULL).type, BO_HAND_HIGH_CARD);

    // A Stone card never pairs and never makes a flush, but it scores.
    BOHandResult stones = evaluate("AS* AS* 3H", NULL);
    BO_EQUAL_INT(stones.type, BO_HAND_HIGH_CARD);
    BO_EQUAL_INT(stones.scoring, 0x04);
    count = parse("AS* AS* 3H", cards);
    BO_EQUAL_INT(bo_hand_scoring_cards(&stones, cards, count, 0), 0x07);
}

BO_TEST(test_hand_scores_each_card_with_splash) {
    BOCard cards[BO_MAX_PLAY];
    int count = parse("KS 9S 9D 6H 3D", cards);
    BOHandResult result = bo_hand_evaluate(cards, count, NULL);
    BO_EQUAL_INT(bo_hand_scoring_cards(&result, cards, count, 1), 0x1f);
}

BO_TEST(test_hand_has_the_base_values_of_balatro) {
    BO_EQUAL_INT(bo_hand_base(BO_HAND_HIGH_CARD)->chips, 5);
    BO_EQUAL_INT(bo_hand_base(BO_HAND_FLUSH)->chips, 35);
    BO_EQUAL_INT(bo_hand_base(BO_HAND_FLUSH)->mult, 4);
    BO_EQUAL_INT(bo_hand_base(BO_HAND_STRAIGHT)->level_chips, 30);
    BO_EQUAL_INT(bo_hand_base(BO_HAND_FLUSH_FIVE)->mult, 16);
    BO_EXPECT(bo_hand_base(BO_HAND_NONE) == NULL);
}

BO_TEST(test_hand_levels_add_chips_and_mult_with_limits) {
    BO_EQUAL_INT(bo_hand_level_chips(BO_HAND_PAIR, 1), 10);
    BO_EQUAL_INT(bo_hand_level_mult(BO_HAND_PAIR, 1), 2);
    BO_EQUAL_INT(bo_hand_level_chips(BO_HAND_PAIR, 3), 40);
    BO_EQUAL_INT(bo_hand_level_mult(BO_HAND_PAIR, 3), 4);
    // The Arm lowers a level. A level of 0 keeps the mult at 1 or more.
    BO_EQUAL_INT(bo_hand_level_chips(BO_HAND_HIGH_CARD, 0), 0);
    BO_EQUAL_INT(bo_hand_level_mult(BO_HAND_HIGH_CARD, 0), 1);
}
