// Tests of bo_describe with a small text in the format of en-us.lua.

#import "BOTest.h"
#include <stdlib.h>
#include "bo_describe.h"
#include "bo_profile.h"
#include "bo_pool.h"
#include "bo_run.h"

static const char *kText =
    "return { descriptions = { Joker = {\n"
    "  j_greedy_joker = { name = 'Greedy Joker', text = { 'Played cards with', '{C:diamonds}#2#{} suit give',"
    "                     '{C:mult}+#1#{} Mult when scored' } },\n"
    "  j_scholar = { name = 'Scholar', text = { '{C:chips}+#2#{} Chips and {C:mult}+#1#{} Mult' } },\n"
    "} }, misc = { suits_singular = { Diamonds = 'Diamond' } } }";

BO_TEST(test_describe_fills_the_variables_of_a_joker) {
    BOLuaValue *text = bo_luatab_parse(kText);
    bo_describe_set_text(text);
    BORun *run = calloc(1, sizeof(BORun));
    bo_run_start(run, "ABCDEFGH");
    BOItem greedy = bo_pool_make_item(run, BO_J_GREEDY_JOKER);
    BODescription d;
    BO_EXPECT(bo_describe_item(run, &greedy, &d));
    BO_EQUAL(@(d.name), @"Greedy Joker");
    BO_EQUAL_INT(d.line_count, 3);
    BO_EQUAL(@(d.lines[1]), @"{C:diamonds}Diamond{} suit give");
    BO_EQUAL(@(d.lines[2]), @"{C:mult}+3{} Mult when scored");
    BOItem scholar = bo_pool_make_item(run, BO_J_SCHOLAR);
    bo_describe_item(run, &scholar, &d);
    BO_EQUAL(@(d.lines[0]), @"{C:chips}+20{} Chips and {C:mult}+4{} Mult");
    // A card with no text keeps its name.
    BOItem joker = bo_pool_make_item(run, BO_J_JOKER);
    BO_EXPECT(!bo_describe_item(run, &joker, &d));
    BO_EQUAL(@(d.name), @"Joker");
    bo_describe_set_text(NULL);
    bo_luatab_free(text);
    free(run);
}

BO_TEST(test_describe_writes_numbers_as_lua_does) {
    char text[32];
    bo_describe_number(4, text, sizeof(text));
    BO_EQUAL(@(text), @"4");
    bo_describe_number(1.5, text, sizeof(text));
    BO_EQUAL(@(text), @"1.5");
    bo_describe_number(0.1 + 0.2, text, sizeof(text));
    BO_EQUAL(@(text), @"0.3");
}

BO_TEST(test_describe_fills_the_variables_of_tarot_planet_and_pack_cards) {
    BOLuaValue *text = bo_luatab_parse(
        "return { descriptions = {\n"
        "  Tarot = { c_magician = { name = 'The Magician', text = { 'Enhances {C:attention}#1#{}', 'to {C:attention}#2#' } },\n"
        "            c_fool = { name = 'The Fool', text = { 'Last: #1#' } } },\n"
        "  Planet = { c_pluto = { name = 'Pluto', text = { 'lvl.#1# #2#', '+#3# Mult and +#4# chips' } } },\n"
        "  Enhanced = { m_lucky = { name = 'Lucky Card', text = {} } },\n"
        "  Other = { p_arcana_normal = { name = 'Arcana Pack', text = { 'Choose #1# of up to #2#' } } },\n"
        "}, misc = { dictionary = { k_none = 'None' }, poker_hands = { ['High Card'] = 'High Card' } } }");
    bo_describe_set_text(text);
    BORun *run = calloc(1, sizeof(BORun));
    bo_run_start(run, "ABCDEFGH");
    BODescription d;
    BOItem magician = bo_pool_make_item(run, BO_C_MAGICIAN);
    BO_EXPECT(bo_describe_item(run, &magician, &d));
    BO_EQUAL(@(d.lines[0]), @"Enhances {C:attention}2{}");
    BO_EQUAL(@(d.lines[1]), @"to {C:attention}Lucky Card");
    BOItem fool = bo_pool_make_item(run, BO_C_FOOL);
    bo_describe_item(run, &fool, &d);
    BO_EQUAL(@(d.lines[0]), @"Last: None");
    BOItem pluto = bo_pool_make_item(run, BO_C_PLUTO);
    bo_run_level_up_hand(run, BO_HAND_HIGH_CARD, 2);
    bo_describe_item(run, &pluto, &d);
    BO_EQUAL(@(d.lines[0]), @"lvl.3 High Card");
    BO_EQUAL(@(d.lines[1]), @"+1 Mult and +10 chips");
    BOItem pack = bo_pool_make_item(run, BO_P_ARCANA_NORMAL_2);
    bo_describe_item(run, &pack, &d);
    BO_EQUAL(@(d.name), @"Arcana Pack");
    BO_EQUAL(@(d.lines[0]), @"Choose 1 of up to 3");
    bo_describe_set_text(NULL);
    bo_luatab_free(text);
    free(run);
}

BO_TEST(test_describe_writes_the_text_of_a_boss) {
    BOLuaValue *text = bo_luatab_parse(
        "return { descriptions = { Blind = { bl_ox = { name = 'The Ox', text = { 'Playing a #1#', 'sets money to $0' } } } },\n"
        "misc = { poker_hands = { ['Two Pair'] = 'Two Pair' } } }");
    bo_describe_set_text(text);
    BORun *run = calloc(1, sizeof(BORun));
    bo_run_start(run, "ABCDEFGH");
    run->most_played_hand = BO_HAND_TWO_PAIR;
    BODescription d;
    BO_EXPECT(bo_describe_blind(run, BO_BL_OX, &d));
    BO_EQUAL(@(d.name), @"The Ox");
    BO_EQUAL(@(d.lines[0]), @"Playing a Two Pair");
    BO_EQUAL(@(d.lines[1]), @"sets money to $0");
    bo_describe_set_text(NULL);
    bo_luatab_free(text);
    free(run);
}

BO_TEST(test_describe_writes_the_text_of_a_tag) {
    BOLuaValue *text = bo_luatab_parse(
        "return { descriptions = { Tag = { tag_skip = { name = 'Skip Tag', text = { '$#1# per skip', '(Will give $#2#)' } } } } }");
    bo_describe_set_text(text);
    BORun *run = calloc(1, sizeof(BORun));
    bo_run_start(run, "ABCDEFGH");
    run->skips = 2;
    BODescription d;
    BO_EXPECT(bo_describe_tag(run, BO_TAG_SKIP, BO_HAND_HIGH_CARD, &d));
    BO_EQUAL(@(d.name), @"Skip Tag");
    BO_EQUAL(@(d.lines[0]), @"$5 per skip");
    // The skip of this blind counts too.
    BO_EQUAL(@(d.lines[1]), @"(Will give $15)");
    bo_describe_set_text(NULL);
    bo_luatab_free(text);
    free(run);
}

BO_TEST(test_describe_writes_the_text_of_a_deck) {
    BOLuaValue *text = bo_luatab_parse(
        "return { descriptions = { Back = {\n"
        "  b_magic = { name = 'Magic Deck', text = { 'Start run with the', '#1# voucher', '2 copies of #2#' } },\n"
        "  b_painted = { name = 'Painted Deck', text = { '+#1# hand size,', '#2# Joker slot' } } } } }");
    bo_describe_set_text(text);
    BOItem magic = {.center = BO_B_MAGIC};
    BODescription d;
    BO_EXPECT(bo_describe_item(NULL, &magic, &d));
    BO_EQUAL(@(d.name), @"Magic Deck");
    BO_EQUAL(@(d.lines[1]), @"Crystal Ball voucher");
    BO_EQUAL(@(d.lines[2]), @"2 copies of The Fool");
    BOItem painted = {.center = BO_B_PAINTED};
    BO_EXPECT(bo_describe_item(NULL, &painted, &d));
    BO_EQUAL(@(d.lines[0]), @"+2 hand size,");
    BO_EQUAL(@(d.lines[1]), @"-1 Joker slot");
    bo_describe_set_text(NULL);
    bo_luatab_free(text);
}

BO_TEST(test_describe_writes_the_condition_of_a_locked_deck) {
    BOLuaValue *text = bo_luatab_parse(
        "return { descriptions = {\n"
        "  Other = { deck_locked_win = { name = 'Locked', text = { 'Win a run with', '{C:attention}#1#{}' } },\n"
        "            deck_locked_stake = { name = 'Locked', text = { 'on at least', '{V:1}#1#{} difficulty' } } },\n"
        "  Back = { b_red = { name = 'Red Deck', text = {} } },\n"
        "  Stake = { stake_green = { name = 'Green Stake', text = {} } } },\n"
        "  misc = { dictionary = { k_unknown = 'Unknown' } } }");
    bo_describe_set_text(text);
    BODescription d;
    BO_EXPECT(bo_describe_locked_back(BO_B_MAGIC, &d));
    BO_EQUAL(@(d.name), @"Locked");
    BO_EQUAL(@(d.lines[1]), @"{C:attention}Red Deck{}");
    BO_EXPECT(bo_describe_locked_back(BO_B_PAINTED, &d));
    BO_EQUAL(@(d.lines[1]), @"{V:1}Green Stake{} difficulty");
    bo_describe_set_text(NULL);
    bo_luatab_free(text);
}

BO_TEST(test_describe_writes_the_unlock_and_undiscovered_texts) {
    BOLuaValue *text = bo_luatab_parse(
        "return { descriptions = {\n"
        "  Joker = { j_burnt = { name = 'Burnt Joker', text = {}, unlock = { 'Sell #1# cards', '(#2#)' } },\n"
        "            j_rough_gem = { name = 'Rough Gem', text = {}, unlock = { '#1# #2# cards' } },\n"
        "            j_idol = { name = 'The Idol', text = {}, unlock = { 'Score #1# chips' } } },\n"
        "  Other = { undiscovered_tarot = { name = 'Not Discovered', text = { 'Use this card' } } } },\n"
        "  misc = { dictionary = { k_locked = 'Locked' }, suits_singular = { Diamonds = 'Diamond' } } }");
    bo_describe_set_text(text);
    BOProfile *p = calloc(1, sizeof(BOProfile));
    bo_profile_init(p);
    p->stats[BO_STAT_CARDS_SOLD] = 7;
    bo_profile_set_active(p);
    BODescription d;
    BO_EXPECT(bo_describe_locked(BO_J_BURNT, &d));
    BO_EQUAL(@(d.name), @"Locked");
    BO_EQUAL(@(d.lines[0]), @"Sell 50 cards");
    BO_EQUAL(@(d.lines[1]), @"(7)");
    BO_EXPECT(bo_describe_locked(BO_J_ROUGH_GEM, &d));
    BO_EQUAL(@(d.lines[0]), @"30 Diamond cards");
    BO_EXPECT(bo_describe_locked(BO_J_IDOL, &d));
    BO_EQUAL(@(d.lines[0]), @"Score 1,000,000 chips");
    BO_EXPECT(bo_describe_undiscovered(BO_C_FOOL, &d));
    BO_EQUAL(@(d.name), @"Not Discovered");
    BO_EQUAL(@(d.lines[0]), @"Use this card");
    bo_profile_set_active(NULL);
    free(p);
    bo_describe_set_text(NULL);
    bo_luatab_free(text);
}

BO_TEST(test_describe_writes_the_rules_of_a_challenge) {
    BOLuaValue *text = bo_luatab_parse(
        "return { misc = { challenge_names = { c_golden_needle_1 = 'Golden Needle' },\n"
        "  v_text = { ch_c_discard_cost = { 'Discards each cost {C:money}$#1#{}' },\n"
        "             ch_m_hands = { '{C:blue}#1#{} hands per round' } } } }");
    bo_describe_set_text(text);
    BODescription d;
    BO_EXPECT(bo_describe_challenge(17, &d));
    BO_EQUAL(@(d.name), @"Golden Needle");
    BO_EQUAL(@(d.lines[0]), @"Discards each cost {C:money}$1{}");
    BO_EQUAL(@(d.lines[1]), @"{C:blue}1{} hands per round");
    bo_describe_set_text(NULL);
    bo_luatab_free(text);
}

BO_TEST(test_describe_removes_the_spaces_in_a_box) {
    // loc_parse_string removes the spaces of a part with an X control.
    BOLuaValue *text = bo_luatab_parse(
        "return { descriptions = { Joker = {\n"
        "  j_duo = { name = 'The Duo', text = { '{X:mult,C:white} X#1# {} Mult if played', 'a {C:attention}#2#' } },\n"
        "} } }");
    bo_describe_set_text(text);
    BORun *run = calloc(1, sizeof(BORun));
    bo_run_start(run, "ABCDEFGH");
    BOItem duo = bo_pool_make_item(run, BO_J_DUO);
    BODescription d;
    BO_EXPECT(bo_describe_item(run, &duo, &d));
    BO_EQUAL(@(d.lines[0]), @"{X:mult,C:white}X2{} Mult if played");
    BO_EQUAL(@(d.lines[1]), @"a {C:attention}Pair");
    bo_describe_set_text(NULL);
    bo_luatab_free(text);
    free(run);
}

BO_TEST(test_describe_adds_the_last_card_of_the_fool) {
    BOLuaValue *text = bo_luatab_parse(
        "return { descriptions = { Tarot = {\n"
        "  c_fool = { name = 'The Fool', text = { 'Creates the last' } },\n"
        "  c_moon = { name = 'The Moon', text = { 'x' } },\n"
        "} },\n"
        "misc = { dictionary = { k_none = 'None' } } }");
    bo_describe_set_text(text);
    BORun *run = calloc(1, sizeof(BORun));
    bo_run_start(run, "ABCDEFGH");
    BOItem fool = bo_pool_make_item(run, BO_C_FOOL);
    BODescription d;
    run->last_tarot_planet = BO_CENTER_NONE;
    BO_EXPECT(bo_describe_item(run, &fool, &d));
    BO_EQUAL_INT(d.line_count, 2);
    BO_EQUAL(@(d.lines[1]), @"{X:red,C:white}None{}");
    run->last_tarot_planet = BO_C_MOON;
    bo_describe_item(run, &fool, &d);
    BO_EQUAL(@(d.lines[1]), @"{X:green,C:white}The Moon{}");
    bo_describe_set_text(NULL);
    bo_luatab_free(text);
    free(run);
}
