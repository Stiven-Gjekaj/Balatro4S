// tools/gen_challenges.lua makes this file from challenges.lua. Do not
// change it by hand.

#include "bo_challenge_data.h"

#include <stddef.h>

static const BOChallengeRule kCustom1[] = {{"no_reward", 0, NULL}, {"no_extra_hand_money", 0, NULL}, {"no_interest", 0, NULL}, };
static const BOChallengeCard kJokers1[] = {{"j_egg", NULL, 0, 0}, {"j_egg", NULL, 0, 0}, {"j_egg", NULL, 0, 0}, {"j_egg", NULL, 0, 0}, {"j_egg", NULL, 0, 0}, };
static const char *const kBanned1[] = {"v_seed_money", "v_money_tree", "j_to_the_moon", "j_rocket", "j_golden", "j_satellite", };
static const BOChallengeCard kJokers2[] = {{"j_ride_the_bus", NULL, 1, 0}, {"j_shortcut", NULL, 1, 0}, };
static const BOChallengeDeckCard kDeck2[] = {{'D', '4', NULL, NULL, NULL}, {'D', '5', NULL, NULL, NULL}, {'D', '6', NULL, NULL, NULL}, {'D', '7', NULL, NULL, NULL}, {'D', '8', NULL, NULL, NULL}, {'D', '9', NULL, NULL, NULL}, {'D', 'T', NULL, NULL, NULL}, {'D', 'J', NULL, NULL, NULL}, {'D', 'Q', NULL, NULL, NULL}, {'D', 'K', NULL, NULL, NULL}, {'D', 'J', NULL, NULL, NULL}, {'D', 'Q', NULL, NULL, NULL}, {'D', 'K', NULL, NULL, NULL}, {'C', '4', NULL, NULL, NULL}, {'C', '5', NULL, NULL, NULL}, {'C', '6', NULL, NULL, NULL}, {'C', '7', NULL, NULL, NULL}, {'C', '8', NULL, NULL, NULL}, {'C', '9', NULL, NULL, NULL}, {'C', 'T', NULL, NULL, NULL}, {'C', 'J', NULL, NULL, NULL}, {'C', 'Q', NULL, NULL, NULL}, {'C', 'K', NULL, NULL, NULL}, {'C', 'J', NULL, NULL, NULL}, {'C', 'Q', NULL, NULL, NULL}, {'C', 'K', NULL, NULL, NULL}, {'H', '4', NULL, NULL, NULL}, {'H', '5', NULL, NULL, NULL}, {'H', '6', NULL, NULL, NULL}, {'H', '7', NULL, NULL, NULL}, {'H', '8', NULL, NULL, NULL}, {'H', '9', NULL, NULL, NULL}, {'H', 'T', NULL, NULL, NULL}, {'H', 'J', NULL, NULL, NULL}, {'H', 'Q', NULL, NULL, NULL}, {'H', 'K', NULL, NULL, NULL}, {'H', 'J', NULL, NULL, NULL}, {'H', 'Q', NULL, NULL, NULL}, {'H', 'K', NULL, NULL, NULL}, {'S', '4', NULL, NULL, NULL}, {'S', '5', NULL, NULL, NULL}, {'S', '6', NULL, NULL, NULL}, {'S', '7', NULL, NULL, NULL}, {'S', '8', NULL, NULL, NULL}, {'S', '9', NULL, NULL, NULL}, {'S', 'T', NULL, NULL, NULL}, {'S', 'J', NULL, NULL, NULL}, {'S', 'Q', NULL, NULL, NULL}, {'S', 'K', NULL, NULL, NULL}, {'S', 'J', NULL, NULL, NULL}, {'S', 'Q', NULL, NULL, NULL}, {'S', 'K', NULL, NULL, NULL}, };
static const BOChallengeRule kCustom3[] = {{"chips_dollar_cap", 0, NULL}, };
static const BOChallengeRule kModifiers3[] = {{"dollars", 100, NULL}, };
static const char *const kVouchers3[] = {"v_seed_money", "v_money_tree", };
static const BOChallengeCard kJokers4[] = {{"j_ceremonial", NULL, 1, 1}, };
static const BOChallengeRule kCustom5[] = {{"flipped_cards", 4, NULL}, };
static const BOChallengeRule kCustom6[] = {{"no_extra_hand_money", 0, NULL}, {"no_interest", 0, NULL}, };
static const BOChallengeCard kJokers6[] = {{"j_pareidolia", "negative", 1, 0}, {"j_business", NULL, 1, 0}, };
static const BOChallengeDeckCard kDeck6[] = {{'D', '2', NULL, NULL, NULL}, {'D', '3', NULL, NULL, NULL}, {'D', '4', NULL, NULL, NULL}, {'D', '5', NULL, NULL, NULL}, {'D', '6', NULL, NULL, NULL}, {'D', '7', NULL, NULL, NULL}, {'D', '8', NULL, NULL, NULL}, {'D', '9', NULL, NULL, NULL}, {'C', '2', NULL, NULL, NULL}, {'C', '3', NULL, NULL, NULL}, {'C', '4', NULL, NULL, NULL}, {'C', '5', NULL, NULL, NULL}, {'C', '6', NULL, NULL, NULL}, {'C', '7', NULL, NULL, NULL}, {'C', '8', NULL, NULL, NULL}, {'C', '9', NULL, NULL, NULL}, {'H', '2', NULL, NULL, NULL}, {'H', '3', NULL, NULL, NULL}, {'H', '4', NULL, NULL, NULL}, {'H', '5', NULL, NULL, NULL}, {'H', '6', NULL, NULL, NULL}, {'H', '7', NULL, NULL, NULL}, {'H', '8', NULL, NULL, NULL}, {'H', '9', NULL, NULL, NULL}, {'S', '2', NULL, NULL, NULL}, {'S', '3', NULL, NULL, NULL}, {'S', '4', NULL, NULL, NULL}, {'S', '5', NULL, NULL, NULL}, {'S', '6', NULL, NULL, NULL}, {'S', '7', NULL, NULL, NULL}, {'S', '8', NULL, NULL, NULL}, {'S', '9', NULL, NULL, NULL}, };
static const char *const kBanned6[] = {"bl_plant", };
static const BOChallengeRule kCustom7[] = {{"minus_hand_size_per_X_dollar", 5, NULL}, };
static const BOChallengeRule kModifiers7[] = {{"hand_size", 10, NULL}, };
static const BOChallengeRule kCustom8[] = {{"all_eternal", 0, NULL}, };
static const char *const kBanned8[] = {"j_gros_michel", "j_ice_cream", "j_cavendish", "j_turtle_bean", "j_ramen", "j_diet_cola", "j_selzer", "j_popcorn", "j_mr_bones", "j_invisible", "j_luchador", "bl_final_leaf", };
static const BOChallengeCard kJokers9[] = {{"j_marble", NULL, 1, 0}, };
static const BOChallengeDeckCard kDeck9[] = {{'D', '2', NULL, NULL, NULL}, {'D', '3', NULL, NULL, NULL}, {'D', '4', NULL, NULL, NULL}, {'D', '5', NULL, NULL, NULL}, {'D', '6', NULL, NULL, NULL}, {'D', '7', NULL, NULL, NULL}, {'D', '8', NULL, NULL, NULL}, {'D', '9', NULL, NULL, NULL}, {'D', 'T', NULL, NULL, NULL}, {'D', 'J', "m_stone", NULL, NULL}, {'D', 'Q', "m_stone", NULL, NULL}, {'D', 'K', "m_stone", NULL, NULL}, {'D', 'A', NULL, NULL, NULL}, {'C', '2', NULL, NULL, NULL}, {'C', '3', NULL, NULL, NULL}, {'C', '4', NULL, NULL, NULL}, {'C', '5', NULL, NULL, NULL}, {'C', '6', NULL, NULL, NULL}, {'C', '7', NULL, NULL, NULL}, {'C', '8', NULL, NULL, NULL}, {'C', '9', NULL, NULL, NULL}, {'C', 'T', NULL, NULL, NULL}, {'C', 'J', "m_stone", NULL, NULL}, {'C', 'Q', "m_stone", NULL, NULL}, {'C', 'K', "m_stone", NULL, NULL}, {'C', 'A', NULL, NULL, NULL}, {'H', '2', NULL, NULL, NULL}, {'H', '3', NULL, NULL, NULL}, {'H', '4', NULL, NULL, NULL}, {'H', '5', NULL, NULL, NULL}, {'H', '6', NULL, NULL, NULL}, {'H', '7', NULL, NULL, NULL}, {'H', '8', NULL, NULL, NULL}, {'H', '9', NULL, NULL, NULL}, {'H', 'T', NULL, NULL, NULL}, {'H', 'J', "m_stone", NULL, NULL}, {'H', 'Q', "m_stone", NULL, NULL}, {'H', 'K', "m_stone", NULL, NULL}, {'H', 'A', NULL, NULL, NULL}, {'S', '2', NULL, NULL, NULL}, {'S', '3', NULL, NULL, NULL}, {'S', '4', NULL, NULL, NULL}, {'S', '5', NULL, NULL, NULL}, {'S', '6', NULL, NULL, NULL}, {'S', '7', NULL, NULL, NULL}, {'S', '8', NULL, NULL, NULL}, {'S', '9', NULL, NULL, NULL}, {'S', 'T', NULL, NULL, NULL}, {'S', 'J', "m_stone", NULL, NULL}, {'S', 'Q', "m_stone", NULL, NULL}, {'S', 'K', "m_stone", NULL, NULL}, {'S', 'A', NULL, NULL, NULL}, };
static const BOChallengeRule kCustom10[] = {{"debuff_played_cards", 0, NULL}, };
static const BOChallengeDeckCard kDeck10[] = {{'D', '2', NULL, NULL, "Red"}, {'D', '3', NULL, NULL, "Red"}, {'D', '4', NULL, NULL, "Red"}, {'D', '5', NULL, NULL, "Red"}, {'D', '6', NULL, NULL, "Red"}, {'D', '7', NULL, NULL, "Red"}, {'D', '8', NULL, NULL, "Red"}, {'D', '9', NULL, NULL, "Red"}, {'D', 'T', NULL, NULL, "Red"}, {'D', 'J', NULL, NULL, "Red"}, {'D', 'Q', NULL, NULL, "Red"}, {'D', 'K', NULL, NULL, "Red"}, {'D', 'A', NULL, NULL, "Red"}, {'C', '2', NULL, NULL, "Red"}, {'C', '3', NULL, NULL, "Red"}, {'C', '4', NULL, NULL, "Red"}, {'C', '5', NULL, NULL, "Red"}, {'C', '6', NULL, NULL, "Red"}, {'C', '7', NULL, NULL, "Red"}, {'C', '8', NULL, NULL, "Red"}, {'C', '9', NULL, NULL, "Red"}, {'C', 'T', NULL, NULL, "Red"}, {'C', 'J', NULL, NULL, "Red"}, {'C', 'Q', NULL, NULL, "Red"}, {'C', 'K', NULL, NULL, "Red"}, {'C', 'A', NULL, NULL, "Red"}, {'H', '2', NULL, NULL, "Red"}, {'H', '3', NULL, NULL, "Red"}, {'H', '4', NULL, NULL, "Red"}, {'H', '5', NULL, NULL, "Red"}, {'H', '6', NULL, NULL, "Red"}, {'H', '7', NULL, NULL, "Red"}, {'H', '8', NULL, NULL, "Red"}, {'H', '9', NULL, NULL, "Red"}, {'H', 'T', NULL, NULL, "Red"}, {'H', 'J', NULL, NULL, "Red"}, {'H', 'Q', NULL, NULL, "Red"}, {'H', 'K', NULL, NULL, "Red"}, {'H', 'A', NULL, NULL, "Red"}, {'S', '2', NULL, NULL, "Red"}, {'S', '3', NULL, NULL, "Red"}, {'S', '4', NULL, NULL, "Red"}, {'S', '5', NULL, NULL, "Red"}, {'S', '6', NULL, NULL, "Red"}, {'S', '7', NULL, NULL, "Red"}, {'S', '8', NULL, NULL, "Red"}, {'S', '9', NULL, NULL, "Red"}, {'S', 'T', NULL, NULL, "Red"}, {'S', 'J', NULL, NULL, "Red"}, {'S', 'Q', NULL, NULL, "Red"}, {'S', 'K', NULL, NULL, "Red"}, {'S', 'A', NULL, NULL, "Red"}, };
static const BOChallengeRule kCustom11[] = {{"set_eternal_ante", 4, NULL}, {"set_joker_slots_ante", 4, NULL}, };
static const char *const kBanned11[] = {"bl_final_leaf", };
static const BOChallengeRule kCustom12[] = {{"inflation", 0, NULL}, };
static const BOChallengeCard kJokers12[] = {{"j_credit_card", NULL, 0, 0}, };
static const char *const kBanned12[] = {"v_clearance_sale", "v_liquidation", };
static const BOChallengeRule kCustom13[] = {{"no_shop_jokers", 0, NULL}, };
static const BOChallengeCard kJokers13[] = {{"j_vampire", NULL, 1, 0}, };
static const BOChallengeCard kConsumables13[] = {{"c_empress", NULL, 0, 0}, {"c_emperor", NULL, 0, 0}, };
static const char *const kVouchers13[] = {"v_magic_trick", "v_illusion", };
static const BOChallengeCard kJokers14[] = {{"j_oops", "negative", 1, 0}, {"j_oops", "negative", 1, 0}, };
static const BOChallengeDeckCard kDeck14[] = {{'D', '2', "m_glass", NULL, NULL}, {'D', '3', "m_glass", NULL, NULL}, {'D', '4', "m_glass", NULL, NULL}, {'D', '5', "m_glass", NULL, NULL}, {'D', '6', "m_glass", NULL, NULL}, {'D', '7', "m_glass", NULL, NULL}, {'D', '8', "m_glass", NULL, NULL}, {'D', '9', "m_glass", NULL, NULL}, {'D', 'T', "m_glass", NULL, NULL}, {'D', 'J', "m_glass", NULL, NULL}, {'D', 'Q', "m_glass", NULL, NULL}, {'D', 'K', "m_glass", NULL, NULL}, {'D', 'A', "m_glass", NULL, NULL}, {'C', '2', "m_glass", NULL, NULL}, {'C', '3', "m_glass", NULL, NULL}, {'C', '4', "m_glass", NULL, NULL}, {'C', '5', "m_glass", NULL, NULL}, {'C', '6', "m_glass", NULL, NULL}, {'C', '7', "m_glass", NULL, NULL}, {'C', '8', "m_glass", NULL, NULL}, {'C', '9', "m_glass", NULL, NULL}, {'C', 'T', "m_glass", NULL, NULL}, {'C', 'J', "m_glass", NULL, NULL}, {'C', 'Q', "m_glass", NULL, NULL}, {'C', 'K', "m_glass", NULL, NULL}, {'C', 'A', "m_glass", NULL, NULL}, {'H', '2', "m_glass", NULL, NULL}, {'H', '3', "m_glass", NULL, NULL}, {'H', '4', "m_glass", NULL, NULL}, {'H', '5', "m_glass", NULL, NULL}, {'H', '6', "m_glass", NULL, NULL}, {'H', '7', "m_glass", NULL, NULL}, {'H', '8', "m_glass", NULL, NULL}, {'H', '9', "m_glass", NULL, NULL}, {'H', 'T', "m_glass", NULL, NULL}, {'H', 'J', "m_glass", NULL, NULL}, {'H', 'Q', "m_glass", NULL, NULL}, {'H', 'K', "m_glass", NULL, NULL}, {'H', 'A', "m_glass", NULL, NULL}, {'S', '2', "m_glass", NULL, NULL}, {'S', '3', "m_glass", NULL, NULL}, {'S', '4', "m_glass", NULL, NULL}, {'S', '5', "m_glass", NULL, NULL}, {'S', '6', "m_glass", NULL, NULL}, {'S', '7', "m_glass", NULL, NULL}, {'S', '8', "m_glass", NULL, NULL}, {'S', '9', "m_glass", NULL, NULL}, {'S', 'T', "m_glass", NULL, NULL}, {'S', 'J', "m_glass", NULL, NULL}, {'S', 'Q', "m_glass", NULL, NULL}, {'S', 'K', "m_glass", NULL, NULL}, {'S', 'A', "m_glass", NULL, NULL}, };
static const char *const kBanned14[] = {"c_magician", "c_empress", "c_heirophant", "c_chariot", "c_devil", "c_tower", "c_lovers", "c_incantation", "c_grim", "c_familiar", "p_standard_normal_1", "p_standard_normal_1", "p_standard_normal_2", "p_standard_normal_3", "p_standard_normal_4", "p_standard_jumbo_1", "p_standard_jumbo_2", "p_standard_mega_1", "p_standard_mega_2", "j_marble", "j_vampire", "j_midas_mask", "j_certificate", "v_magic_trick", "v_illusion", "tag_standard", };
static const BOChallengeCard kJokers15[] = {{"j_obelisk", NULL, 1, 0}, {"j_marble", "negative", 1, 0}, };
static const BOChallengeRule kModifiers16[] = {{"hands", 2, NULL}, {"discards", 2, NULL}, {"joker_slots", 4, NULL}, };
static const BOChallengeCard kJokers16[] = {{"j_constellation", NULL, 1, 0}, {"j_rocket", NULL, 1, 0}, };
static const char *const kVouchers16[] = {"v_planet_merchant", "v_planet_tycoon", };
static const char *const kBanned16[] = {"v_grabber", "v_nacho_tong", "j_burglar", };
static const BOChallengeRule kModifiers17[] = {{"hand_size", 5, NULL}, {"joker_slots", 7, NULL}, {"discards", 6, NULL}, };
static const BOChallengeCard kJokers17[] = {{"j_card_sharp", NULL, 0, 0}, {"j_joker", NULL, 0, 0}, };
static const char *const kBanned17[] = {"j_juggler", "j_troubadour", "j_turtle_bean", };
static const BOChallengeRule kCustom18[] = {{"discard_cost", 1, NULL}, };
static const BOChallengeRule kModifiers18[] = {{"hands", 1, NULL}, {"discards", 6, NULL}, {"dollars", 10, NULL}, };
static const BOChallengeCard kJokers18[] = {{"j_credit_card", NULL, 0, 0}, };
static const char *const kBanned18[] = {"v_grabber", "v_nacho_tong", "j_burglar", };
static const BOChallengeRule kCustom19[] = {{"no_reward_specific", 0, "Small"}, {"no_reward_specific", 0, "Big"}, };
static const BOChallengeRule kModifiers19[] = {{"joker_slots", 3, NULL}, };
static const BOChallengeRule kCustom20[] = {{"no_shop_jokers", 0, NULL}, };
static const BOChallengeRule kModifiers20[] = {{"joker_slots", 0, NULL}, };
static const char *const kBanned20[] = {"c_judgement", "c_wraith", "c_soul", "v_antimatter", "p_buffoon_normal_1", "p_buffoon_normal_1", "p_buffoon_normal_2", "p_buffoon_jumbo_1", "p_buffoon_mega_1", "tag_rare", "tag_uncommon", "tag_holo", "tag_polychrome", "tag_negative", "tag_foil", "tag_buffoon", "tag_top_up", "bl_final_acorn", "bl_final_heart", "bl_final_leaf", };

static const BOChallengeInfo kChallenges[BO_CHALLENGE_COUNT] = {
    {"c_omelette_1", "The Omelette", kCustom1, 3, NULL, 0, kJokers1, 5, NULL, 0, NULL, 0, "Challenge Deck", NULL, 0, NULL, NULL, NULL, NULL, NULL, NULL, 0, kBanned1, 6},
    {"c_city_1", "15 Minute City", NULL, 0, NULL, 0, kJokers2, 2, NULL, 0, NULL, 0, "Challenge Deck", kDeck2, 52, NULL, NULL, NULL, NULL, NULL, NULL, 0, NULL, 0},
    {"c_rich_1", "Rich get Richer", kCustom3, 1, kModifiers3, 1, NULL, 0, NULL, 0, kVouchers3, 2, "Challenge Deck", NULL, 0, NULL, NULL, NULL, NULL, NULL, NULL, 0, NULL, 0},
    {"c_knife_1", "On a Knife's Edge", NULL, 0, NULL, 0, kJokers4, 1, NULL, 0, NULL, 0, "Challenge Deck", NULL, 0, NULL, NULL, NULL, NULL, NULL, NULL, 0, NULL, 0},
    {"c_xray_1", "X-ray Vision", kCustom5, 1, NULL, 0, NULL, 0, NULL, 0, NULL, 0, "Challenge Deck", NULL, 0, NULL, NULL, NULL, NULL, NULL, NULL, 0, NULL, 0},
    {"c_mad_world_1", "Mad World", kCustom6, 2, NULL, 0, kJokers6, 2, NULL, 0, NULL, 0, "Challenge Deck", kDeck6, 32, NULL, NULL, NULL, NULL, NULL, NULL, 0, kBanned6, 1},
    {"c_luxury_1", "Luxury Tax", kCustom7, 1, kModifiers7, 1, NULL, 0, NULL, 0, NULL, 0, "Challenge Deck", NULL, 0, NULL, NULL, NULL, NULL, NULL, NULL, 0, NULL, 0},
    {"c_non_perishable_1", "Non-Perishable", kCustom8, 1, NULL, 0, NULL, 0, NULL, 0, NULL, 0, "Challenge Deck", NULL, 0, NULL, NULL, NULL, NULL, NULL, NULL, 0, kBanned8, 12},
    {"c_medusa_1", "Medusa", NULL, 0, NULL, 0, kJokers9, 1, NULL, 0, NULL, 0, "Challenge Deck", kDeck9, 52, NULL, NULL, NULL, NULL, NULL, NULL, 0, NULL, 0},
    {"c_double_nothing_1", "Double or Nothing", kCustom10, 1, NULL, 0, NULL, 0, NULL, 0, NULL, 0, "Challenge Deck", kDeck10, 52, NULL, NULL, NULL, NULL, NULL, NULL, 0, NULL, 0},
    {"c_typecast_1", "Typecast", kCustom11, 2, NULL, 0, NULL, 0, NULL, 0, NULL, 0, "Challenge Deck", NULL, 0, NULL, NULL, NULL, NULL, NULL, NULL, 0, kBanned11, 1},
    {"c_inflation_1", "Inflation", kCustom12, 1, NULL, 0, kJokers12, 1, NULL, 0, NULL, 0, "Challenge Deck", NULL, 0, NULL, NULL, NULL, NULL, NULL, NULL, 0, kBanned12, 2},
    {"c_bram_poker_1", "Bram Poker", kCustom13, 1, NULL, 0, kJokers13, 1, kConsumables13, 2, kVouchers13, 2, "Challenge Deck", NULL, 0, NULL, NULL, NULL, NULL, NULL, NULL, 0, NULL, 0},
    {"c_fragile_1", "Fragile", NULL, 0, NULL, 0, kJokers14, 2, NULL, 0, NULL, 0, "Challenge Deck", kDeck14, 52, NULL, NULL, NULL, NULL, NULL, NULL, 0, kBanned14, 26},
    {"c_monolith_1", "Monolith", NULL, 0, NULL, 0, kJokers15, 2, NULL, 0, NULL, 0, "Challenge Deck", NULL, 0, NULL, NULL, NULL, NULL, NULL, NULL, 0, NULL, 0},
    {"c_blast_off_1", "Blast Off", NULL, 0, kModifiers16, 3, kJokers16, 2, NULL, 0, kVouchers16, 2, "Challenge Deck", NULL, 0, NULL, NULL, NULL, NULL, NULL, NULL, 0, kBanned16, 3},
    {"c_five_card_1", "Five-Card Draw", NULL, 0, kModifiers17, 3, kJokers17, 2, NULL, 0, NULL, 0, "Challenge Deck", NULL, 0, NULL, NULL, NULL, NULL, NULL, NULL, 0, kBanned17, 3},
    {"c_golden_needle_1", "Golden Needle", kCustom18, 1, kModifiers18, 3, kJokers18, 1, NULL, 0, NULL, 0, "Challenge Deck", NULL, 0, NULL, NULL, NULL, NULL, NULL, NULL, 0, kBanned18, 3},
    {"c_cruelty_1", "Cruelty", kCustom19, 2, kModifiers19, 1, NULL, 0, NULL, 0, NULL, 0, "Challenge Deck", NULL, 0, NULL, NULL, NULL, NULL, NULL, NULL, 0, NULL, 0},
    {"c_jokerless_1", "Jokerless", kCustom20, 1, kModifiers20, 1, NULL, 0, NULL, 0, NULL, 0, "Challenge Deck", NULL, 0, NULL, NULL, NULL, NULL, NULL, NULL, 0, kBanned20, 20},
};

const BOChallengeInfo *bo_challenge(int index) {
    if (index < 0 || index >= BO_CHALLENGE_COUNT) return NULL;
    return &kChallenges[index];
}
