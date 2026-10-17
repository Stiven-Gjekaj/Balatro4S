#include "bo_zip.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <zlib.h>

typedef struct {
    char *name;
    uint16_t method;
    uint32_t compressed;
    uint32_t size;
    uint32_t offset;
} BOZipEntry;

struct BOZip {
    FILE *file;
    BOZipEntry *entries;
    int count;
};

static uint16_t read16(const unsigned char *p) {
    return (uint16_t)(p[0] | p[1] << 8);
}

static uint32_t read32(const unsigned char *p) {
    return (uint32_t)p[0] | (uint32_t)p[1] << 8 | (uint32_t)p[2] << 16 | (uint32_t)p[3] << 24;
}

// Finds the end of central directory record. It is in the last
// 22 + 65535 bytes, because the comment after it is at most 65535 bytes.
static long find_end_record(FILE *file, unsigned char record[22]) {
    if (fseek(file, 0, SEEK_END) != 0) return -1;
    long length = ftell(file);
    if (length < 22) return -1;
    long span = length < 22 + 65535 ? length : 22 + 65535;
    unsigned char *tail = malloc((size_t)span);
    if (!tail) return -1;
    long found = -1;
    if (fseek(file, length - span, SEEK_SET) == 0 && fread(tail, 1, (size_t)span, file) == (size_t)span) {
        for (long i = span - 22; i >= 0; i--) {
            if (read32(tail + i) == 0x06054b50) {
                memcpy(record, tail + i, 22);
                found = length - span + i;
                break;
            }
        }
    }
    free(tail);
    return found;
}

BOZip *bo_zip_open(const char *path) {
    FILE *file = fopen(path, "rb");
    if (!file) return NULL;
    unsigned char end[22];
    if (find_end_record(file, end) < 0) {
        fclose(file);
        return NULL;
    }
    int count = read16(end + 10);
    uint32_t directorySize = read32(end + 12);
    uint32_t directoryOffset = read32(end + 16);

    unsigned char *directory = malloc(directorySize ? directorySize : 1);
    BOZip *zip = calloc(1, sizeof(BOZip));
    if (zip) zip->entries = calloc(count ? (size_t)count : 1, sizeof(BOZipEntry));
    if (!directory || !zip || !zip->entries || fseek(file, directoryOffset, SEEK_SET) != 0 ||
        fread(directory, 1, directorySize, file) != directorySize) {
        free(directory);
        if (zip) free(zip->entries);
        free(zip);
        fclose(file);
        return NULL;
    }
    zip->file = file;

    uint32_t at = 0;
    for (int i = 0; i < count; i++) {
        if (at + 46 > directorySize || read32(directory + at) != 0x02014b50) break;
        const unsigned char *p = directory + at;
        uint16_t nameLength = read16(p + 28);
        uint32_t next = at + 46 + nameLength + read16(p + 30) + read16(p + 32);
        if (next > directorySize) break;
        BOZipEntry *entry = &zip->entries[zip->count];
        entry->name = malloc(nameLength + 1u);
        if (!entry->name) break;
        memcpy(entry->name, p + 46, nameLength);
        entry->name[nameLength] = 0;
        entry->method = read16(p + 10);
        entry->compressed = read32(p + 20);
        entry->size = read32(p + 24);
        entry->offset = read32(p + 42);
        zip->count++;
        at = next;
    }
    free(directory);
    return zip;
}

void bo_zip_close(BOZip *zip) {
    if (!zip) return;
    for (int i = 0; i < zip->count; i++) free(zip->entries[i].name);
    free(zip->entries);
    fclose(zip->file);
    free(zip);
}

int bo_zip_count(const BOZip *zip) {
    return zip ? zip->count : 0;
}

const char *bo_zip_name(const BOZip *zip, int index) {
    if (!zip || index < 0 || index >= zip->count) return NULL;
    return zip->entries[index].name;
}

static unsigned char *inflate_all(const unsigned char *in, uint32_t inSize, uint32_t outSize) {
    unsigned char *out = malloc(outSize + 1u);
    if (!out) return NULL;
    z_stream stream;
    memset(&stream, 0, sizeof(stream));
    // A negative window size reads raw deflate data with no zlib header.
    if (inflateInit2(&stream, -MAX_WBITS) != Z_OK) {
        free(out);
        return NULL;
    }
    stream.next_in = (Bytef *)in;
    stream.avail_in = inSize;
    stream.next_out = out;
    stream.avail_out = outSize;
    int status = inflate(&stream, Z_FINISH);
    uLong produced = stream.total_out;
    inflateEnd(&stream);
    if (status != Z_STREAM_END || produced != outSize) {
        free(out);
        return NULL;
    }
    return out;
}

unsigned char *bo_zip_read(BOZip *zip, const char *name, size_t *size) {
    if (!zip || !name) return NULL;
    BOZipEntry *entry = NULL;
    for (int i = 0; i < zip->count; i++) {
        if (strcmp(zip->entries[i].name, name) == 0) {
            entry = &zip->entries[i];
            break;
        }
    }
    if (!entry || (entry->method != 0 && entry->method != 8)) return NULL;

    unsigned char local[30];
    if (fseek(zip->file, entry->offset, SEEK_SET) != 0 || fread(local, 1, 30, zip->file) != 30 ||
        read32(local) != 0x04034b50) {
        return NULL;
    }
    long data = (long)entry->offset + 30 + read16(local + 26) + read16(local + 28);
    unsigned char *packed = malloc(entry->compressed + 1u);
    if (!packed) return NULL;
    if (fseek(zip->file, data, SEEK_SET) != 0 ||
        fread(packed, 1, entry->compressed, zip->file) != entry->compressed) {
        free(packed);
        return NULL;
    }

    unsigned char *out;
    if (entry->method == 0) {
        if (entry->compressed != entry->size) {
            free(packed);
            return NULL;
        }
        out = packed;
    } else {
        out = inflate_all(packed, entry->compressed, entry->size);
        free(packed);
        if (!out) return NULL;
    }
    out[entry->size] = 0;
    if (size) *size = entry->size;
    return out;
}
