// describe.c: prints the text of each card with bo_describe of Balatro4S.
// tools/describe.lua prints the same with the code of Balatro, and
// tools/describe.py compares the two.
// Usage: describe EN_US_LUA SEED
// Each line is: key | line 1 | line 2 ..., with no markup.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bo_describe.h"
#include "bo_pool.h"
#include "bo_run.h"

// Removes the markup of Balatro, as "{C:mult}".
static void plain(const char *in, char *out) {
    int depth = 0;
    for (; *in; in++) {
        if (*in == '{') depth++;
        else if (*in == '}') depth--;
        else if (depth == 0) *out++ = *in;
    }
    *out = 0;
}

int main(int argc, char **argv) {
    if (argc < 3) {
        fprintf(stderr, "usage: describe EN_US_LUA SEED\n");
        return 1;
    }
    FILE *file = fopen(argv[1], "rb");
    if (!file) return 1;
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    rewind(file);
    char *text = malloc(size + 1);
    text[fread(text, 1, size, file)] = 0;
    fclose(file);
    BOLuaValue *root = bo_luatab_parse(text);
    bo_describe_set_text(root);
    BORun *run = calloc(1, sizeof(BORun));
    bo_run_start(run, argv[2]);
    // The keys in the order of strcmp, as table.sort of describe.lua.
    int ids[BO_CENTER_COUNT], count = 0;
    for (int i = 0; i < BO_CENTER_COUNT; i++) {
        int set = bo_center((BOCenterId)i)->set;
        if (set == BO_SET_JOKER || set == BO_SET_TAROT || set == BO_SET_PLANET || set == BO_SET_SPECTRAL ||
            set == BO_SET_VOUCHER || set == BO_SET_BOOSTER) {
            ids[count++] = i;
        }
    }
    for (int a = 0; a < count; a++) {
        for (int b = a + 1; b < count; b++) {
            if (strcmp(bo_center((BOCenterId)ids[a])->key, bo_center((BOCenterId)ids[b])->key) > 0) {
                int t = ids[a];
                ids[a] = ids[b];
                ids[b] = t;
            }
        }
    }
    for (int a = 0; a < count; a++) {
        BOItem item = bo_pool_make_item(run, (BOCenterId)ids[a]);
        BODescription d;
        memset(&d, 0, sizeof(d));
        bo_describe_item(run, &item, &d);
        printf("%s", bo_center((BOCenterId)ids[a])->key);
        for (int i = 0; i < d.line_count; i++) {
            char line[BO_DESCRIBE_LINE_LENGTH];
            plain(d.lines[i], line);
            printf(" | %s", line);
        }
        printf("\n");
    }
    return 0;
}
