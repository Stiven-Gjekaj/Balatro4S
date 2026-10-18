// Tests of the challenges. The expected values come from challenges.lua
// and Game:start_run of Balatro 1.0.1o.

#import "BOTest.h"
#include <string.h>
#include "bo_challenge_data.h"

BO_TEST(test_challenge_data_has_the_20_challenges) {
    BO_EQUAL_INT(BO_CHALLENGE_COUNT, 20);
    const BOChallengeInfo *omelette = bo_challenge(0);
    BO_EQUAL(@(omelette->id), @"c_omelette_1");
    BO_EQUAL_INT(omelette->joker_count, 5);
    BO_EQUAL(@(omelette->jokers[0].key), @"j_egg");
    BO_EQUAL_INT(omelette->custom_count, 3);
    BO_EQUAL(@(omelette->custom[0].id), @"no_reward");
    // Fragile bans the Standard packs with the ids of the group.
    const BOChallengeInfo *fragile = bo_challenge(13);
    BO_EQUAL(@(fragile->name), @"Fragile");
    BO_EQUAL_INT(fragile->banned_count, 26);
    BO_EQUAL_INT(fragile->deck_card_count, 52);
    BO_EXPECT(fragile->jokers[0].eternal);
    BO_EQUAL(@(fragile->jokers[0].edition), @"negative");
    const BOChallengeInfo *luxury = bo_challenge(6);
    BO_EQUAL(@(luxury->custom[0].id), @"minus_hand_size_per_X_dollar");
    BO_EXPECT(luxury->custom[0].number == 5);
    BO_EXPECT(bo_challenge(20) == NULL);
}
