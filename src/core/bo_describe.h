// bo_describe: the text of a card, as Card:generate_UIBox_ability_table
// and localize of Balatro 1.0.1o make it. The words come from
// localization/en-us.lua of the Balatro.love of the owner. The numbers
// come from the card.

#ifndef BO_DESCRIBE_H
#define BO_DESCRIBE_H

#include "bo_item.h"
#include "bo_luatab.h"

struct BORun;

#define BO_DESCRIBE_LINES 8
#define BO_DESCRIBE_LINE_LENGTH 160

typedef struct {
    char name[64];
    int line_count;
    // Each line keeps the markup of Balatro, as "{C:mult}+4{} Mult".
    char lines[BO_DESCRIBE_LINES][BO_DESCRIBE_LINE_LENGTH];
} BODescription;

// The parsed text file. bo_describe_* return 0 with no text.
void bo_describe_set_text(BOLuaValue *root);

// The text of a joker, a consumable, a voucher, or a pack.
int bo_describe_item(const struct BORun *run, const BOItem *item, BODescription *out);

// A word of the dictionary of misc, as "k_mult", or the key itself.
const char *bo_describe_word(const char *key);

// The value of a description variable as Lua writes a number.
void bo_describe_number(double value, char *out, int size);

#endif
