// bo_card: a playing card, with the values of card.lua in Balatro 1.0.1o.

#ifndef BO_CARD_H
#define BO_CARD_H

#include <stdint.h>

// The order of the suits is the order in which get_flush tests them.
typedef enum {
    BO_SPADES = 0,
    BO_HEARTS = 1,
    BO_CLUBS = 2,
    BO_DIAMONDS = 3,
    BO_SUIT_COUNT = 4,
} BOSuit;

// The rank is the id of Balatro: 2 to 10, then 11 Jack, 12 Queen,
// 13 King, and 14 Ace.
enum {
    BO_JACK = 11,
    BO_QUEEN = 12,
    BO_KING = 13,
    BO_ACE = 14,
};

typedef enum {
    BO_ENH_NONE = 0,
    BO_ENH_BONUS,
    BO_ENH_MULT,
    BO_ENH_WILD,
    BO_ENH_GLASS,
    BO_ENH_STEEL,
    BO_ENH_STONE,
    BO_ENH_GOLD,
    BO_ENH_LUCKY,
} BOEnhancement;

typedef enum {
    BO_EDITION_NONE = 0,
    BO_EDITION_FOIL,
    BO_EDITION_HOLO,
    BO_EDITION_POLYCHROME,
    BO_EDITION_NEGATIVE,
} BOEdition;

typedef enum {
    BO_SEAL_NONE = 0,
    BO_SEAL_GOLD,
    BO_SEAL_RED,
    BO_SEAL_BLUE,
    BO_SEAL_PURPLE,
} BOSeal;

typedef struct {
    uint8_t rank;
    uint8_t suit;
    uint8_t enhancement;
    uint8_t edition;
    uint8_t seal;
    uint8_t debuffed;
    int16_t perma_bonus;  // The chips that Hiker adds for the rest of the run.
    uint32_t uid;         // The order of creation (sort_id). unique_val follows it.
    // Flags of one play or one round.
    uint8_t vampired;          // Vampire took the enhancement in this play.
    uint8_t lucky_trigger;     // A Lucky Card paid in this play.
    uint8_t shattered;         // A Glass Card broke.
    uint8_t destroyed;
    uint8_t face_down;         // A boss blind turned the card.
    uint8_t played_this_ante;  // For The Pillar.
    uint8_t forced_selection;  // Cerulean Bell.
    uint16_t times_played;     // base.times_played: the plays of the card in this run.
} BOCard;

BOCard bo_card_make(int rank, BOSuit suit);

// The chips of the rank: 2 to 10, 10 for a face card, and 11 for an Ace.
int bo_card_nominal(const BOCard *card);

// get_nominal: the value that get_highest and the sort of the hand
// compare. It is the nominal, then the face, then the suit, then the order
// of creation. With by_suit, the suit comes first, as for "suit desc".
double bo_card_nominal_key(const BOCard *card, int by_suit);
double bo_card_highest_key(const BOCard *card);

int bo_card_is_stone(const BOCard *card);

// The suit test of get_flush: a Stone card has no suit, and a Wild card
// that is not debuffed has each suit. With smeared, Hearts and Diamonds
// are one suit, and Spades and Clubs are one suit.
int bo_card_is_suit_for_flush(const BOCard *card, BOSuit suit, int smeared);

// The short name, for example "AS" or "TH", for the logs and the tests.
const char *bo_card_name(const BOCard *card, char buffer[4]);

#endif
