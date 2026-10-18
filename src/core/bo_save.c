#include "bo_save.h"

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Change this number when the meaning of a field changes but its layout
// does not.
#define BO_SAVE_FORMAT 1

typedef struct {
    char magic[4];  // "B4SR": a run of Balatro4S.
    uint32_t format;
    uint32_t layout;
    uint32_t size;
    BOSaveExtra extra;
} BOSaveHeader;

// FNV-1a over the sizes and the places of the parts of BORun. A new field,
// a removed field, or a field with another size changes it.
static uint32_t mix(uint32_t hash, uint32_t value) {
    for (int i = 0; i < 4; i++) {
        hash ^= (value >> (i * 8)) & 0xff;
        hash *= 16777619u;
    }
    return hash;
}

uint32_t bo_save_layout(void) {
    uint32_t h = 2166136261u;
    h = mix(h, sizeof(BORun));
    h = mix(h, sizeof(BOCard));
    h = mix(h, sizeof(BOItem));
    h = mix(h, sizeof(BORandom));
    h = mix(h, sizeof(BOTag));
    h = mix(h, sizeof(BOSaveExtra));
    h = mix(h, BO_CENTER_COUNT);
    h = mix(h, offsetof(BORun, random));
    h = mix(h, offsetof(BORun, state));
    h = mix(h, offsetof(BORun, deck));
    h = mix(h, offsetof(BORun, hand));
    h = mix(h, offsetof(BORun, jokers));
    h = mix(h, offsetof(BORun, consumables));
    h = mix(h, offsetof(BORun, tags));
    return h;
}

int bo_save_write(const BORun *run, const BOSaveExtra *extra, const char *path) {
    char temp[1024];
    if (snprintf(temp, sizeof(temp), "%s.new", path) >= (int)sizeof(temp)) return 0;
    FILE *file = fopen(temp, "wb");
    if (!file) return 0;
    BOSaveHeader header;
    memset(&header, 0, sizeof(header));
    memcpy(header.magic, "B4SR", 4);
    header.format = BO_SAVE_FORMAT;
    header.layout = bo_save_layout();
    header.size = sizeof(BORun);
    if (extra) header.extra = *extra;
    int ok = fwrite(&header, sizeof(header), 1, file) == 1 && fwrite(run, sizeof(BORun), 1, file) == 1;
    ok = fclose(file) == 0 && ok;
    if (!ok || rename(temp, path) != 0) {
        remove(temp);
        return 0;
    }
    return 1;
}

int bo_save_read(BORun *run, BOSaveExtra *extra, const char *path) {
    FILE *file = fopen(path, "rb");
    if (!file) return 0;
    BOSaveHeader header;
    BORun *loaded = malloc(sizeof(BORun));
    int ok = loaded && fread(&header, sizeof(header), 1, file) == 1 && memcmp(header.magic, "B4SR", 4) == 0 &&
             header.format == BO_SAVE_FORMAT && header.layout == bo_save_layout() &&
             header.size == sizeof(BORun) && fread(loaded, sizeof(BORun), 1, file) == 1;
    // The file must end after the run.
    if (ok) ok = fgetc(file) == EOF;
    fclose(file);
    if (ok) {
        memcpy(run, loaded, sizeof(BORun));
        if (extra) *extra = header.extra;
    }
    free(loaded);
    return ok;
}

void bo_save_remove(const char *path) {
    remove(path);
}
