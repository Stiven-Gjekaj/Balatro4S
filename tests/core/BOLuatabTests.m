// Tests of bo_luatab with a small text in the format of en-us.lua.

#import "BOTest.h"
#include "bo_luatab.h"

static const char *kText =
    "return {\n"
    "    descriptions={\n"
    "        Joker={\n"
    "            j_joker={\n"
    "                name=\"Joker\",\n"
    "                text={\n"
    "                    \"{C:red,s:1.1}+#1#{} Mult\",\n"
    "                    'with \\\"quotes\\\"',\n"
    "                },\n"
    "            },\n"
    "        },\n"
    "    },\n"
    "    -- a comment\n"
    "    misc={ value = -2.5, on = true, off = false, [\"odd key\"] = 3, [4] = 'four' },\n"
    "}\n";

BO_TEST(test_luatab_reads_nested_tables_and_lists) {
    BOLuaValue *root = bo_luatab_parse(kText);
    BO_EXPECT(root != NULL);
    BO_EQUAL(@(bo_luatab_string(root, "descriptions.Joker.j_joker.name")), @"Joker");
    BOLuaValue *text = bo_luatab_path(root, "descriptions.Joker.j_joker.text");
    BO_EQUAL_INT(bo_luatab_length(text), 2);
    BO_EQUAL(@(bo_luatab_item(text, 0)->string), @"{C:red,s:1.1}+#1#{} Mult");
    BO_EQUAL(@(bo_luatab_item(text, 1)->string), @"with \"quotes\"");
    BO_EXPECT(bo_luatab_path(root, "misc.value")->number == -2.5);
    BO_EQUAL_INT(bo_luatab_path(root, "misc.on")->number, 1);
    BO_EQUAL_INT(bo_luatab_path(root, "misc.off")->number, 0);
    BO_EQUAL_INT(bo_luatab_get(bo_luatab_get(root, "misc"), "odd key")->number, 3);
    BO_EQUAL(@(bo_luatab_string(root, "misc.4")), @"four");
    BO_EXPECT(bo_luatab_path(root, "descriptions.Tarot") == NULL);
    bo_luatab_free(root);
}

BO_TEST(test_luatab_refuses_text_that_is_not_a_table) {
    BO_EXPECT(bo_luatab_parse("print('hello')") == NULL);
    BO_EXPECT(bo_luatab_parse("return { a = ") == NULL);
}
