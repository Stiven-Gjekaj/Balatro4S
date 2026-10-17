#include "bo_hand.h"

#include <string.h>

static const BOHandBase kBase[BO_HAND_COUNT] = {
    {160, 16, 50, 3},  // Flush Five
    {140, 14, 40, 4},  // Flush House
    {120, 12, 35, 3},  // Five of a Kind
    {100, 8, 40, 4},   // Straight Flush
    {60, 7, 30, 3},    // Four of a Kind
    {40, 4, 25, 2},    // Full House
    {35, 4, 15, 2},    // Flush
    {30, 4, 30, 3},    // Straight
    {30, 3, 20, 2},    // Three of a Kind
    {20, 2, 20, 1},    // Two Pair
    {10, 2, 15, 1},    // Pair
    {5, 1, 10, 1},     // High Card
};

static const char *kNames[BO_HAND_COUNT] = {
    "Flush Five", "Flush House", "Five of a Kind", "Straight Flush", "Four of a Kind", "Full House",
    "Flush", "Straight", "Three of a Kind", "Two Pair", "Pair", "High Card",
};

const BOHandBase *bo_hand_base(BOHandType type) {
    if (type < 0 || type >= BO_HAND_COUNT) return NULL;
    return &kBase[type];
}

const char *bo_hand_name(BOHandType type, int royal) {
    if (type == BO_HAND_STRAIGHT_FLUSH && royal) return "Royal Flush";
    if (type < 0 || type >= BO_HAND_COUNT) return "";
    return kNames[type];
}

int bo_hand_contains(const BOHandResult *result, BOHandType type) {
    return type >= 0 && type < BO_HAND_COUNT && (result->contains >> type) & 1;
}

// The id of a card for the groups and the straights. Balatro gives a
// Stone card a random negative id. Here each Stone card gets its own
// negative id, so it never matches another card.
static int card_id(const BOCard *cards, int index) {
    return bo_card_is_stone(&cards[index]) ? -1 - index : cards[index].rank;
}

// get_X_same: the groups of exactly num cards of one rank, the highest
// rank first. Each group is a bit mask of cards.
static int groups_of(const BOCard *cards, int count, int num, uint8_t out[BO_MAX_PLAY]) {
    uint8_t byRank[15] = {0};
    for (int i = count - 1; i >= 0; i--) {
        int id = card_id(cards, i);
        uint8_t group = 0;
        int size = 0;
        for (int j = 0; j < count; j++) {
            if (card_id(cards, j) == id) {
                group |= (uint8_t)(1 << j);
                size++;
            }
        }
        if (size == num && id >= 2) byRank[id] = group;
    }
    int found = 0;
    for (int id = 14; id >= 2; id--) {
        if (byRank[id]) out[found++] = byRank[id];
    }
    return found;
}

static uint8_t flush_of(const BOCard *cards, int count, const BOHandRules *rules) {
    int need = rules->four_fingers ? 4 : 5;
    if (count > 5 || count < need) return 0;
    for (int suit = 0; suit < BO_SUIT_COUNT; suit++) {
        uint8_t mask = 0;
        int matches = 0;
        for (int i = 0; i < count; i++) {
            if (bo_card_is_suit_for_flush(&cards[i], (BOSuit)suit, rules->smeared)) {
                mask |= (uint8_t)(1 << i);
                matches++;
            }
        }
        if (matches >= need) return mask;
    }
    return 0;
}

// get_straight. The loop goes from j = 1 (an Ace as a low card) to 14.
static uint8_t straight_of(const BOCard *cards, int count, const BOHandRules *rules) {
    int need = rules->four_fingers ? 4 : 5;
    if (count > 5 || count < need) return 0;
    uint8_t byRank[15] = {0};
    for (int i = 0; i < count; i++) {
        int id = card_id(cards, i);
        if (id > 1 && id < 15) byRank[id] |= (uint8_t)(1 << i);
    }
    uint8_t cardsSoFar = 0;
    int length = 0;
    int straight = 0;
    int skipped = 0;
    for (int j = 1; j <= 14; j++) {
        uint8_t here = byRank[j == 1 ? 14 : j];
        if (here) {
            length++;
            skipped = 0;
            cardsSoFar |= here;
        } else if (rules->shortcut && !skipped && j != 14) {
            skipped = 1;
        } else {
            length = 0;
            skipped = 0;
            if (!straight) cardsSoFar = 0;
            if (straight) break;
        }
        if (length >= need) straight = 1;
    }
    return straight ? cardsSoFar : 0;
}

BOHandResult bo_hand_evaluate(const BOCard *cards, int count, const BOHandRules *rules) {
    static const BOHandRules kNoRules = {0, 0, 0};
    if (!rules) rules = &kNoRules;
    BOHandResult result;
    memset(&result, 0, sizeof(result));
    result.type = BO_HAND_NONE;
    if (count <= 0) return result;
    if (count > BO_MAX_PLAY) count = BO_MAX_PLAY;

    uint8_t fives[BO_MAX_PLAY], fours[BO_MAX_PLAY], threes[BO_MAX_PLAY], twos[BO_MAX_PLAY];
    int n5 = groups_of(cards, count, 5, fives);
    int n4 = groups_of(cards, count, 4, fours);
    int n3 = groups_of(cards, count, 3, threes);
    int n2 = groups_of(cards, count, 2, twos);
    uint8_t flush = flush_of(cards, count, rules);
    uint8_t straight = straight_of(cards, count, rules);

    // get_highest: the one card with the highest nominal.
    int highest = 0;
    for (int i = 1; i < count; i++) {
        if (bo_card_highest_key(&cards[i]) > bo_card_highest_key(&cards[highest])) highest = i;
    }

    uint8_t hands[BO_HAND_COUNT] = {0};
    if (n5 && flush) hands[BO_HAND_FLUSH_FIVE] = fives[0];
    if (n3 && n2 && flush) hands[BO_HAND_FLUSH_HOUSE] = threes[0] | twos[0];
    if (n5) hands[BO_HAND_FIVE_OF_A_KIND] = fives[0];
    if (flush && straight) hands[BO_HAND_STRAIGHT_FLUSH] = flush | straight;
    if (n4) hands[BO_HAND_FOUR_OF_A_KIND] = fours[0];
    if (n3 && n2) hands[BO_HAND_FULL_HOUSE] = threes[0] | twos[0];
    if (flush) hands[BO_HAND_FLUSH] = flush;
    if (straight) hands[BO_HAND_STRAIGHT] = straight;
    if (n3) hands[BO_HAND_THREE_OF_A_KIND] = threes[0];
    if (n2 == 2) hands[BO_HAND_TWO_PAIR] = twos[0] | twos[1];
    else if (n3 == 1 && n2 == 1) hands[BO_HAND_TWO_PAIR] = twos[0] | threes[0];
    if (n2) hands[BO_HAND_PAIR] = twos[0];
    hands[BO_HAND_HIGH_CARD] = (uint8_t)(1 << highest);

    for (int type = 0; type < BO_HAND_COUNT; type++) {
        if (hands[type] && result.type == BO_HAND_NONE) {
            result.type = (BOHandType)type;
            result.scoring = hands[type];
        }
    }

    // The table that the jokers read has the smaller hands inside the
    // bigger ones. A Five of a Kind also holds a Four of a Kind, and so on.
    // Only the "contains" bits use this. The scoring cards do not.
    if (hands[BO_HAND_FIVE_OF_A_KIND]) hands[BO_HAND_FOUR_OF_A_KIND] = hands[BO_HAND_FIVE_OF_A_KIND];
    if (hands[BO_HAND_FOUR_OF_A_KIND]) hands[BO_HAND_THREE_OF_A_KIND] = hands[BO_HAND_FOUR_OF_A_KIND];
    if (hands[BO_HAND_THREE_OF_A_KIND]) hands[BO_HAND_PAIR] = hands[BO_HAND_THREE_OF_A_KIND];
    for (int type = 0; type < BO_HAND_COUNT; type++) {
        if (hands[type]) result.contains |= (uint16_t)(1 << type);
    }

    if (result.type == BO_HAND_STRAIGHT_FLUSH) {
        int lowest = 10;
        for (int i = 0; i < count; i++) {
            if ((result.scoring >> i) & 1 && card_id(cards, i) < lowest) lowest = card_id(cards, i);
        }
        result.royal = lowest >= 10;
    }
    return result;
}

uint8_t bo_hand_scoring_cards(const BOHandResult *result, const BOCard *cards, int count, int splash) {
    if (count > BO_MAX_PLAY) count = BO_MAX_PLAY;
    uint8_t mask = result->scoring;
    for (int i = 0; i < count; i++) {
        if (splash || bo_card_is_stone(&cards[i])) mask |= (uint8_t)(1 << i);
    }
    return mask;
}
