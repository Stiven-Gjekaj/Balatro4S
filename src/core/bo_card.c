#include "bo_card.h"

BOCard bo_card_make(int rank, BOSuit suit) {
    BOCard card = {0};
    card.rank = (uint8_t)rank;
    card.suit = (uint8_t)suit;
    return card;
}

int bo_card_nominal(const BOCard *card) {
    if (card->rank == BO_ACE) return 11;
    if (card->rank >= 10) return 10;
    return card->rank;
}

double bo_card_nominal_key(const BOCard *card, int by_suit) {
    static const double kSuitNominal[BO_SUIT_COUNT] = {0.04, 0.03, 0.02, 0.01};
    static const double kFaceNominal[5] = {0, 0.1, 0.2, 0.3, 0.4};
    double face = card->rank > 10 && card->rank <= BO_ACE ? kFaceNominal[card->rank - 10] : 0;
    double mult = by_suit ? 1000 : 1;
    // Get_nominal puts a Stone card under all other cards.
    if (bo_card_is_stone(card)) mult = -1000;
    // Balatro sets unique_val to 1 - ID / 1603301, so a newer card is lower.
    double unique = 1 - card->uid / 1603301.0;
    return bo_card_nominal(card) + kSuitNominal[card->suit] * mult + face + 0.000001 * unique;
}

double bo_card_highest_key(const BOCard *card) {
    return bo_card_nominal_key(card, 0);
}

int bo_card_is_stone(const BOCard *card) {
    return card->enhancement == BO_ENH_STONE;
}

static int is_red(int suit) {
    return suit == BO_HEARTS || suit == BO_DIAMONDS;
}

int bo_card_is_suit_for_flush(const BOCard *card, BOSuit suit, int smeared) {
    if (bo_card_is_stone(card)) return 0;
    if (card->enhancement == BO_ENH_WILD && !card->debuffed) return 1;
    if (smeared && is_red(card->suit) == is_red(suit)) return 1;
    return card->suit == suit;
}

const char *bo_card_name(const BOCard *card, char buffer[4]) {
    static const char kRanks[] = "??23456789TJQKA";
    static const char kSuits[] = "SHCD";
    buffer[0] = card->rank <= BO_ACE ? kRanks[card->rank] : '?';
    buffer[1] = card->suit < BO_SUIT_COUNT ? kSuits[card->suit] : '?';
    buffer[2] = 0;
    return buffer;
}
