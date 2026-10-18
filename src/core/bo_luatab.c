#include "bo_luatab.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    const char *p;
    int failed;
} BOParser;

static void skip_space(BOParser *ps) {
    for (;;) {
        while (*ps->p && isspace((unsigned char)*ps->p)) ps->p++;
        if (ps->p[0] == '-' && ps->p[1] == '-') {
            if (ps->p[2] == '[' && ps->p[3] == '[') {
                const char *end = strstr(ps->p + 4, "]]");
                ps->p = end ? end + 2 : ps->p + strlen(ps->p);
            } else {
                while (*ps->p && *ps->p != '\n') ps->p++;
            }
            continue;
        }
        return;
    }
}

static BOLuaValue *new_value(BOLuaType type) {
    BOLuaValue *v = calloc(1, sizeof(BOLuaValue));
    v->type = type;
    return v;
}

static char *read_string(BOParser *ps) {
    char quote = *ps->p++;
    size_t capacity = 64, length = 0;
    char *out = malloc(capacity);
    while (*ps->p && *ps->p != quote) {
        char c = *ps->p++;
        if (c == '\\' && *ps->p) {
            char e = *ps->p++;
            switch (e) {
            case 'n': c = '\n'; break;
            case 't': c = '\t'; break;
            default: c = e; break;
            }
        }
        if (length + 2 > capacity) {
            capacity *= 2;
            out = realloc(out, capacity);
        }
        out[length++] = c;
    }
    if (*ps->p == quote) {
        ps->p++;
    } else {
        ps->failed = 1;
    }
    out[length] = 0;
    return out;
}

static BOLuaValue *parse_value(BOParser *ps);

static void add_entry(BOLuaValue *table, char *key, BOLuaValue *value) {
    table->keys = realloc(table->keys, sizeof(char *) * (table->count + 1));
    table->values = realloc(table->values, sizeof(BOLuaValue *) * (table->count + 1));
    table->keys[table->count] = key;
    table->values[table->count] = value;
    table->count++;
}

static int is_name_start(char c) {
    return isalpha((unsigned char)c) || c == '_';
}

static BOLuaValue *parse_table(BOParser *ps) {
    ps->p++;  // {
    BOLuaValue *table = new_value(BO_LUA_TABLE);
    for (;;) {
        skip_space(ps);
        if (*ps->p == '}') {
            ps->p++;
            return table;
        }
        if (!*ps->p) {
            ps->failed = 1;
            return table;
        }
        char *key = NULL;
        const char *save = ps->p;
        if (*ps->p == '[') {
            ps->p++;
            skip_space(ps);
            if (*ps->p == '"' || *ps->p == '\'') {
                key = read_string(ps);
            } else {
                BOLuaValue *k = parse_value(ps);
                if (k && k->type == BO_LUA_NUMBER) {
                    key = malloc(32);
                    snprintf(key, 32, "%g", k->number);
                }
                bo_luatab_free(k);
            }
            skip_space(ps);
            if (*ps->p == ']') ps->p++;
            skip_space(ps);
            if (*ps->p == '=') ps->p++;
        } else if (is_name_start(*ps->p)) {
            const char *start = ps->p;
            while (is_name_start(*ps->p) || isdigit((unsigned char)*ps->p)) ps->p++;
            const char *end = ps->p;
            skip_space(ps);
            if (*ps->p == '=' && ps->p[1] != '=') {
                ps->p++;
                key = malloc((size_t)(end - start) + 1);
                memcpy(key, start, (size_t)(end - start));
                key[end - start] = 0;
            } else {
                ps->p = save;
            }
        }
        BOLuaValue *value = parse_value(ps);
        if (ps->failed) {
            free(key);
            bo_luatab_free(value);
            return table;
        }
        add_entry(table, key, value);
        skip_space(ps);
        if (*ps->p == ',' || *ps->p == ';') ps->p++;
    }
}

static BOLuaValue *parse_value(BOParser *ps) {
    skip_space(ps);
    char c = *ps->p;
    if (c == '{') return parse_table(ps);
    if (c == '"' || c == '\'') {
        BOLuaValue *v = new_value(BO_LUA_STRING);
        v->string = read_string(ps);
        return v;
    }
    if (c == '-' || c == '.' || isdigit((unsigned char)c)) {
        char *end = NULL;
        double number = strtod(ps->p, &end);
        if (end == ps->p) {
            ps->failed = 1;
            return NULL;
        }
        ps->p = end;
        BOLuaValue *v = new_value(BO_LUA_NUMBER);
        v->number = number;
        return v;
    }
    if (strncmp(ps->p, "true", 4) == 0) {
        ps->p += 4;
        BOLuaValue *v = new_value(BO_LUA_BOOLEAN);
        v->number = 1;
        return v;
    }
    if (strncmp(ps->p, "false", 5) == 0) {
        ps->p += 5;
        return new_value(BO_LUA_BOOLEAN);
    }
    if (strncmp(ps->p, "nil", 3) == 0) {
        ps->p += 3;
        return new_value(BO_LUA_NIL);
    }
    ps->failed = 1;
    return NULL;
}

BOLuaValue *bo_luatab_parse(const char *text) {
    BOParser ps = {text, 0};
    skip_space(&ps);
    if (strncmp(ps.p, "return", 6) == 0) ps.p += 6;
    skip_space(&ps);
    if (*ps.p != '{') return NULL;
    BOLuaValue *root = parse_value(&ps);
    if (ps.failed) {
        bo_luatab_free(root);
        return NULL;
    }
    return root;
}

void bo_luatab_free(BOLuaValue *value) {
    if (!value) return;
    for (int i = 0; i < value->count; i++) {
        free(value->keys[i]);
        bo_luatab_free(value->values[i]);
    }
    free(value->keys);
    free(value->values);
    free(value->string);
    free(value);
}

BOLuaValue *bo_luatab_get(const BOLuaValue *table, const char *key) {
    if (!table || table->type != BO_LUA_TABLE || !key) return NULL;
    for (int i = 0; i < table->count; i++) {
        if (table->keys[i] && strcmp(table->keys[i], key) == 0) return table->values[i];
    }
    return NULL;
}

BOLuaValue *bo_luatab_path(const BOLuaValue *table, const char *path) {
    char part[128];
    const BOLuaValue *current = table;
    while (current && *path) {
        const char *dot = strchr(path, '.');
        size_t length = dot ? (size_t)(dot - path) : strlen(path);
        if (length >= sizeof(part)) return NULL;
        memcpy(part, path, length);
        part[length] = 0;
        current = bo_luatab_get(current, part);
        path = dot ? dot + 1 : path + length;
    }
    return (BOLuaValue *)current;
}

BOLuaValue *bo_luatab_item(const BOLuaValue *table, int index) {
    if (!table || table->type != BO_LUA_TABLE) return NULL;
    int n = 0;
    for (int i = 0; i < table->count; i++) {
        if (table->keys[i]) continue;
        if (n == index) return table->values[i];
        n++;
    }
    return NULL;
}

int bo_luatab_length(const BOLuaValue *table) {
    if (!table || table->type != BO_LUA_TABLE) return 0;
    int n = 0;
    for (int i = 0; i < table->count; i++) n += table->keys[i] == NULL;
    return n;
}

const char *bo_luatab_string(const BOLuaValue *table, const char *path) {
    BOLuaValue *v = bo_luatab_path(table, path);
    return v && v->type == BO_LUA_STRING ? v->string : NULL;
}
