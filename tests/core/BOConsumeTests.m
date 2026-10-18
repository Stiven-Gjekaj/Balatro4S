// Tests of the use of the Tarot, Planet, and Spectral cards. Each expected
// value comes from Card:use_consumeable of Balatro 1.0.1o with the same
// cards, with tools/oracle.sh score and a "use" line. tools/fuzz.py --use
// compares many more random uses.

#import "BOTest.h"
#include <stdlib.h>
#include "bo_consume.h"
#include "bo_pool.h"
#include "bo_rules.h"
#include "bo_run.h"

// A run in the middle of a Small Blind, with no deck, as the oracle makes it.
static BORun *make_run(const char *seed, int dollars) {
    BORun *run = calloc(1, sizeof(BORun));
    bo_run_start(run, seed);
    bo_random_start(&run->random, seed);
    run->deck_count = 0;
    run->state = BO_STATE_SELECTING;
    run->blind = BO_BL_SMALL;
    run->dollars = dollars;
    return run;
}

static BOItem *add_joker(BORun *run, BOCenterId center, int edition) {
    BOItem item = bo_pool_make_item(run, center);
    item.edition = (uint8_t)edition;
    bo_run_add_joker(run, &item);
    return &run->jokers[run->joker_count - 1];
}

static void hold(BORun *run, int rank, int suit) {
    run->hand[run->hand_count++] = bo_rules_new_playing_card(run, rank, suit);
}

// Puts the consumable in the last slot and uses it.
static int use(BORun *run, BOCenterId center, uint64_t highlighted) {
    BOItem item = bo_pool_make_item(run, center);
    bo_run_add_consumable(run, &item);
    return bo_run_use_consumable(run, run->consumable_count - 1, highlighted);
}

BO_TEST(test_consume_familiar_makes_enhanced_face_cards_for_hologram) {
    BORun *run = make_run("ABCDEFGH", 10);
    add_joker(run, BO_J_CONSTELLATION, 0);
    add_joker(run, BO_J_GLASS, 0);
    BOItem *hologram = add_joker(run, BO_J_HOLOGRAM, 0);
    hold(run, BO_ACE, BO_SPADES);
    hold(run, BO_KING, BO_HEARTS);
    run->hand[1].enhancement = BO_ENH_GLASS;
    hold(run, 5, BO_DIAMONDS);
    run->hand[2].edition = BO_EDITION_FOIL;
    hold(run, 9, BO_CLUBS);
    run->hand[3].enhancement = BO_ENH_STEEL;
    run->hand[3].seal = BO_SEAL_RED;
    hold(run, 2, BO_HEARTS);
    BO_EXPECT(use(run, BO_C_FAMILIAR, 0));
    // The 5 of Diamonds goes, and three face cards come.
    BO_EQUAL_INT(run->hand_count, 7);
    BO_EQUAL_INT(run->hand[4].rank, BO_KING);
    BO_EQUAL_INT(run->hand[4].suit, BO_CLUBS);
    BO_EQUAL_INT(run->hand[4].enhancement, BO_ENH_STEEL);
    BO_EQUAL_INT(run->hand[5].rank, BO_QUEEN);
    BO_EQUAL_INT(run->hand[5].suit, BO_DIAMONDS);
    BO_EQUAL_INT(run->hand[5].enhancement, BO_ENH_LUCKY);
    BO_EQUAL_INT(run->hand[6].rank, BO_KING);
    BO_EQUAL_INT(run->hand[6].suit, BO_DIAMONDS);
    BO_EQUAL_INT(run->hand[6].enhancement, BO_ENH_WILD);
    BO_EXPECT(hologram->ability.x_mult == 1.75);
    free(run);
}

BO_TEST(test_consume_death_copies_the_card_on_the_right) {
    BORun *run = make_run("ABCDEFGH", 4);
    hold(run, BO_ACE, BO_SPADES);
    hold(run, BO_KING, BO_HEARTS);
    hold(run, 5, BO_DIAMONDS);
    hold(run, 9, BO_CLUBS);
    uint32_t uid = run->hand[1].uid;
    BO_EXPECT(use(run, BO_C_DEATH, 0x6));
    BO_EQUAL_INT(run->hand[1].rank, 5);
    BO_EQUAL_INT(run->hand[1].suit, BO_DIAMONDS);
    BO_EQUAL_INT(run->hand[1].uid, uid);
    // Death needs two cards.
    BO_EXPECT(!use(run, BO_C_DEATH, 0x1));
    free(run);
}

BO_TEST(test_consume_hex_keeps_the_eternal_joker_and_draws_for_merry_andy) {
    BORun *run = make_run("KQXP2RLM", 10);
    add_joker(run, BO_J_MERRY_ANDY, 0);
    add_joker(run, BO_J_JOKER, BO_EDITION_FOIL);
    add_joker(run, BO_J_EGG, 0)->eternal = 1;
    add_joker(run, BO_J_CONSTELLATION, 0);
    for (int i = 0; i < 4; i++) run->deck[run->deck_count++] = bo_rules_new_playing_card(run, 2, BO_CLUBS);
    hold(run, BO_ACE, BO_SPADES);
    hold(run, BO_KING, BO_HEARTS);
    BO_EXPECT(use(run, BO_C_HEX, 0));
    BO_EQUAL_INT(run->joker_count, 2);
    BO_EQUAL_INT(run->jokers[0].center, BO_J_MERRY_ANDY);
    BO_EQUAL_INT(run->jokers[0].edition, BO_EDITION_POLYCHROME);
    BO_EQUAL_INT(run->jokers[1].center, BO_J_EGG);
    BO_EQUAL_INT(run->hand_size, 7);
    BO_EQUAL_INT(run->hand_count, 2);
    free(run);
}

BO_TEST(test_consume_fool_makes_the_last_card_and_needs_one) {
    BORun *run = make_run("ABCDEFGH", 4);
    BOItem fool = bo_pool_make_item(run, BO_C_FOOL);
    BO_EXPECT(!bo_consume_can_use(run, &fool, 0, BO_USE_FROM_CONSUMABLES));
    run->last_tarot_planet = BO_C_JUPITER;
    BO_EXPECT(use(run, BO_C_FOOL, 0));
    BO_EQUAL_INT(run->consumable_count, 1);
    BO_EQUAL_INT(run->consumables[0].center, BO_C_JUPITER);
    BO_EQUAL_INT(run->last_tarot_planet, BO_C_FOOL);
    free(run);
}

BO_TEST(test_consume_immolate_destroys_five_cards_for_twenty_dollars) {
    BORun *run = make_run("ABCDEFGH", 3);
    add_joker(run, BO_J_CAINO, 0);
    hold(run, BO_ACE, BO_SPADES);
    hold(run, BO_KING, BO_HEARTS);
    hold(run, BO_QUEEN, BO_DIAMONDS);
    hold(run, 9, BO_CLUBS);
    hold(run, 4, BO_SPADES);
    hold(run, 7, BO_HEARTS);
    hold(run, BO_JACK, BO_DIAMONDS);
    BO_EXPECT(use(run, BO_C_IMMOLATE, 0));
    BO_EQUAL_INT(run->hand_count, 2);
    BO_EQUAL_INT(run->hand[0].rank, BO_QUEEN);
    BO_EQUAL_INT(run->hand[1].rank, 7);
    BO_EQUAL_INT(run->dollars, 23);
    free(run);
}

BO_TEST(test_consume_ectoplasm_makes_a_negative_joker_and_takes_hand_size) {
    BORun *run = make_run("ZZZZ1234", 4);
    add_joker(run, BO_J_JOKER, 0);
    add_joker(run, BO_J_BLUEPRINT, BO_EDITION_HOLO);
    BO_EXPECT(use(run, BO_C_ECTOPLASM, 0));
    BO_EQUAL_INT(run->jokers[0].edition, BO_EDITION_NEGATIVE);
    BO_EQUAL_INT(run->joker_slots, 6);
    BO_EQUAL_INT(run->hand_size, 7);
    BO_EQUAL_INT(run->ecto_minus, 2);
    free(run);
}
