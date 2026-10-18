// Tests of the booster packs. Each expected card comes from Card:open of
// Balatro 1.0.1o with the same seed, with tools/oracle.sh score and an
// "open" line. tools/fuzz.py --pack compares many more random packs.

#import "BOTest.h"
#include <stdlib.h>
#include <string.h>
#include "bo_pack.h"
#include "bo_pool.h"
#include "bo_rules.h"
#include "bo_run.h"

// A run in the shop, with no deck, as the oracle makes it.
static BORun *make_run(const char *seed, int ante, int dollars) {
    BORun *run = calloc(1, sizeof(BORun));
    bo_run_start(run, seed);
    bo_random_start(&run->random, seed);
    run->deck_count = 0;
    run->state = BO_STATE_SHOP;
    run->ante = ante;
    run->dollars = dollars;
    return run;
}

static void add_joker(BORun *run, BOCenterId center) {
    BOItem item = bo_pool_make_item(run, center);
    bo_run_add_joker(run, &item);
}

// Opens a pack as the shop does: the booster card is made, then opened.
static void open_pack(BORun *run, BOCenterId center) {
    BOItem booster = bo_pool_make_item(run, center);
    bo_run_forget_card(run, center);
    BO_EXPECT(bo_pack_open(run, center, booster.cost));
}

static void deck_under(BORun *run, int rank, int suit, int enhancement) {
    memmove(&run->deck[1], run->deck, sizeof(BOCard) * run->deck_count);
    run->deck[0] = bo_rules_new_playing_card(run, rank, suit);
    run->deck[0].enhancement = (uint8_t)enhancement;
    run->deck_count++;
}

BO_TEST(test_pack_arcana_shows_tarots_and_draws_the_hand) {
    BORun *run = make_run("ABCDEFGH", 1, 10);
    add_joker(run, BO_J_HALLUCINATION);
    add_joker(run, BO_J_RED_CARD);
    int ranks[10] = {BO_ACE, BO_KING, 5, 9, 2, 7, BO_JACK, BO_QUEEN, 4, 3};
    int suits[10] = {BO_SPADES, BO_HEARTS, BO_DIAMONDS, BO_CLUBS, BO_HEARTS,
                     BO_SPADES, BO_DIAMONDS, BO_CLUBS, BO_HEARTS, BO_SPADES};
    for (int i = 0; i < 10; i++) deck_under(run, ranks[i], suits[i], 0);
    open_pack(run, BO_P_ARCANA_MEGA_1);
    BO_EQUAL_INT(run->state, BO_STATE_PACK);
    BO_EQUAL_INT(run->pack_choices, 2);
    BO_EQUAL_INT(run->pack_count, 5);
    BOCenterId expected[5] = {BO_C_HEIROPHANT, BO_C_DEVIL, BO_C_HANGED_MAN, BO_C_JUSTICE, BO_C_HERMIT};
    for (int i = 0; i < 5; i++) BO_EQUAL_INT(run->pack_cards[i].item.center, expected[i]);
    BO_EQUAL_INT(run->hand_count, 8);
    // The Hierophant on the Ace, then The Hanged Man on two more cards.
    BO_EXPECT(bo_pack_choose(run, 0, 0x1));
    BO_EQUAL_INT(run->pack_choices, 1);
    BO_EXPECT(bo_pack_choose(run, 1, 0x6));
    BO_EQUAL_INT(run->state, BO_STATE_SHOP);
    BO_EQUAL_INT(run->hand_count, 0);
    BO_EQUAL_INT(run->deck_count, 8);
    BO_EQUAL_INT(run->dollars, 2);
    int bonus = 0;
    for (int i = 0; i < run->deck_count; i++) bonus += run->deck[i].enhancement == BO_ENH_BONUS;
    BO_EQUAL_INT(bonus, 1);
    free(run);
}

BO_TEST(test_pack_standard_adds_its_cards_under_the_deck) {
    BORun *run = make_run("ABCDEFGH", 2, 10);
    add_joker(run, BO_J_HOLOGRAM);
    open_pack(run, BO_P_STANDARD_MEGA_1);
    BO_EQUAL_INT(run->pack_count, 5);
    BO_EQUAL_INT(run->pack_cards[1].card.rank, 6);
    BO_EQUAL_INT(run->pack_cards[1].card.suit, BO_HEARTS);
    BO_EQUAL_INT(run->pack_cards[1].card.edition, BO_EDITION_FOIL);
    BO_EQUAL_INT(run->pack_cards[4].card.rank, BO_QUEEN);
    BO_EQUAL_INT(run->pack_cards[4].card.enhancement, BO_ENH_GLASS);
    BO_EXPECT(bo_pack_choose(run, 1, 0));
    BO_EXPECT(bo_pack_choose(run, 3, 0));
    BO_EQUAL_INT(run->deck_count, 2);
    // The top of the deck is the first card that was taken.
    BO_EQUAL_INT(run->deck[1].rank, 6);
    BO_EQUAL_INT(run->deck[0].rank, BO_QUEEN);
    BO_EXPECT(run->jokers[0].ability.x_mult == 1.5);
    free(run);
}

BO_TEST(test_pack_skip_gives_red_card_its_mult) {
    BORun *run = make_run("KQXP2RLM", 1, 10);
    add_joker(run, BO_J_RED_CARD);
    open_pack(run, BO_P_BUFFOON_NORMAL_1);
    BO_EQUAL_INT(run->pack_count, 2);
    BO_EQUAL_INT(run->pack_cards[0].item.center, BO_J_SUPERNOVA);
    BO_EQUAL_INT(run->pack_cards[0].item.edition, BO_EDITION_HOLO);
    BO_EQUAL_INT(run->pack_cards[1].item.center, BO_J_MISPRINT);
    BO_EXPECT(bo_pack_can_skip(run));
    bo_pack_skip(run);
    BO_EQUAL_INT(run->state, BO_STATE_SHOP);
    BO_EXPECT(run->jokers[0].ability.mult == 3);
    BO_EQUAL_INT(run->dollars, 6);
    free(run);
}

BO_TEST(test_pack_hallucination_makes_tarots_after_the_pack_cards) {
    BORun *run = make_run("HALU1234", 1, 10);
    add_joker(run, BO_J_HALLUCINATION);
    add_joker(run, BO_J_HALLUCINATION);
    add_joker(run, BO_J_HALLUCINATION);
    open_pack(run, BO_P_CELESTIAL_NORMAL_1);
    BOCenterId expected[3] = {BO_C_JUPITER, BO_C_NEPTUNE, BO_C_PLUTO};
    for (int i = 0; i < 3; i++) BO_EQUAL_INT(run->pack_cards[i].item.center, expected[i]);
    BO_EQUAL_INT(run->consumable_count, 2);
    BO_EQUAL_INT(run->consumables[0].center, BO_C_HIGH_PRIESTESS);
    BO_EQUAL_INT(run->consumables[1].center, BO_C_JUSTICE);
    BO_EXPECT(bo_pack_choose(run, 0, 0));
    BO_EQUAL_INT(run->hands[BO_HAND_FLUSH].level, 2);
    free(run);
}
