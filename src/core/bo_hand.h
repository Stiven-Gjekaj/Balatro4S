// bo_hand: finds the poker hand of the played cards, as
// evaluate_poker_hand and get_poker_hand_info do in Balatro 1.0.1o.

#ifndef BO_HAND_H
#define BO_HAND_H

#include <stdint.h>
#include "bo_card.h"

// The order is the "order" field of G.GAME.hands. A lower value is a
// better hand.
typedef enum {
    BO_HAND_FLUSH_FIVE = 0,
    BO_HAND_FLUSH_HOUSE,
    BO_HAND_FIVE_OF_A_KIND,
    BO_HAND_STRAIGHT_FLUSH,
    BO_HAND_FOUR_OF_A_KIND,
    BO_HAND_FULL_HOUSE,
    BO_HAND_FLUSH,
    BO_HAND_STRAIGHT,
    BO_HAND_THREE_OF_A_KIND,
    BO_HAND_TWO_PAIR,
    BO_HAND_PAIR,
    BO_HAND_HIGH_CARD,
    BO_HAND_COUNT,
    BO_HAND_NONE = -1,
} BOHandType;

#define BO_MAX_PLAY 5

// The jokers that change the rules of a hand.
typedef struct {
    int four_fingers;  // A flush or a straight needs only 4 cards.
    int shortcut;      // A straight can skip one rank.
    int smeared;       // Hearts and Diamonds are one suit, and so are Spades and Clubs.
} BOHandRules;

typedef struct {
    BOHandType type;
    // A bit for each hand that the cards contain, as the poker_hands table
    // that the jokers read. For example, a Full House also contains a
    // Pair and a Three of a Kind.
    uint16_t contains;
    // A bit for each played card that the hand uses, in the order of play.
    // Stone cards are not in it. bo_hand_scoring_cards adds them.
    uint8_t scoring;
    int royal;  // A Straight Flush with no card under 10.
} BOHandResult;

// Finds the hand of count cards (1 to 5). The ranks of Stone cards are
// unique, so a Stone card never makes a pair.
BOHandResult bo_hand_evaluate(const BOCard *cards, int count, const BOHandRules *rules);

// The cards that score: the cards of the hand, and each Stone card. With
// splash, each played card scores.
uint8_t bo_hand_scoring_cards(const BOHandResult *result, const BOCard *cards, int count, int splash);

int bo_hand_contains(const BOHandResult *result, BOHandType type);

// The name that Balatro shows, for example "Royal Flush" or "Two Pair".
const char *bo_hand_name(BOHandType type, int royal);

typedef struct {
    int chips;
    int mult;
    int level_chips;  // The chips that one level adds.
    int level_mult;
} BOHandBase;

// The base values of each hand at level 1, from game.lua.
const BOHandBase *bo_hand_base(BOHandType type);

#endif
