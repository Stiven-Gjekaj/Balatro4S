// bo_luatab: reads a Lua table constructor, as the text files of Balatro
// (localization/en-us.lua) hold them: "return { key = "text", list = {...} }".
// It reads tables, strings, numbers, booleans, and comments. It does not
// run Lua code.

#ifndef BO_LUATAB_H
#define BO_LUATAB_H

typedef enum {
    BO_LUA_NIL,
    BO_LUA_TABLE,
    BO_LUA_STRING,
    BO_LUA_NUMBER,
    BO_LUA_BOOLEAN,
} BOLuaType;

typedef struct BOLuaValue BOLuaValue;

struct BOLuaValue {
    BOLuaType type;
    char *string;      // A string value.
    double number;     // A number value, or 1 and 0 for a boolean.
    int count;         // A table: the count of its entries.
    char **keys;       // A table: the key of each entry, or NULL for a list item.
    BOLuaValue **values;
};

// Parses text. Returns NULL when the text is not a table constructor.
BOLuaValue *bo_luatab_parse(const char *text);
void bo_luatab_free(BOLuaValue *value);

// The entry with a key, or NULL.
BOLuaValue *bo_luatab_get(const BOLuaValue *table, const char *key);
// The entry at a dot path, as "descriptions.Joker.j_joker.name".
BOLuaValue *bo_luatab_path(const BOLuaValue *table, const char *path);
// The list item at index (from 0), or NULL.
BOLuaValue *bo_luatab_item(const BOLuaValue *table, int index);
// The count of list items.
int bo_luatab_length(const BOLuaValue *table);
// The string at a path, or NULL.
const char *bo_luatab_string(const BOLuaValue *table, const char *path);

#endif
