// tools/gen_challenges.lua makes this file from challenges.lua. Do not
// change it by hand.

#ifndef BO_CHALLENGE_DATA_H
#define BO_CHALLENGE_DATA_H

#include <stdint.h>

// A rule of rules.custom or rules.modifiers: an id, and a number or a text.
typedef struct {
    const char *id;
    double number;
    const char *text;
} BOChallengeRule;

// A joker or a consumable of the start, with its edition and stickers.
typedef struct {
    const char *key;
    const char *edition;
    uint8_t eternal;
    uint8_t pinned;
} BOChallengeCard;

// A card of deck.cards: the letters of its suit and rank, as 'D' and 'T',
// and its enhancement, edition, and seal, or NULL.
typedef struct {
    char suit;
    char rank;
    const char *enhancement;
    const char *edition;
    const char *seal;
} BOChallengeDeckCard;

typedef struct {
    const char *id;
    const char *name;
    const BOChallengeRule *custom;
    uint8_t custom_count;
    const BOChallengeRule *modifiers;
    uint8_t modifier_count;
    const BOChallengeCard *jokers;
    uint8_t joker_count;
    const BOChallengeCard *consumables;
    uint8_t consumable_count;
    const char *const *vouchers;
    uint8_t voucher_count;
    // deck: its type, its cards, and the filters of the standard cards.
    // The filters are the letters of the ranks and suits, as "AK".
    const char *deck_type;
    const BOChallengeDeckCard *deck_cards;
    uint8_t deck_card_count;
    const char *yes_ranks, *no_ranks, *yes_suits, *no_suits;
    const char *deck_enhancement;
    const char *deck_edition;
    uint8_t deck_gold_seal;
    // restrictions: each banned key, with the ids of a banned group.
    const char *const *banned;
    uint8_t banned_count;
} BOChallengeInfo;

#define BO_CHALLENGE_COUNT 20

const BOChallengeInfo *bo_challenge(int index);

#endif
