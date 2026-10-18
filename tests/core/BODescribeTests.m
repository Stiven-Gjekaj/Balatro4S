// Tests of bo_describe with a small text in the format of en-us.lua.

#import "BOTest.h"
#include <stdlib.h>
#include "bo_describe.h"
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
