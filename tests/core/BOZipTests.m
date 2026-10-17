// Tests of bo_zip. Each test writes its own archive.

#import "BOTest.h"
#include "bo_zip.h"
#include <zlib.h>

static void put16(NSMutableData *data, uint16_t value) {
    uint8_t bytes[2] = {value & 0xff, value >> 8};
    [data appendBytes:bytes length:2];
}

static void put32(NSMutableData *data, uint32_t value) {
    put16(data, value & 0xffff);
    put16(data, value >> 16);
}

static NSData *rawDeflate(NSData *input) {
    z_stream stream = {0};
    deflateInit2(&stream, Z_BEST_COMPRESSION, Z_DEFLATED, -MAX_WBITS, 8, Z_DEFAULT_STRATEGY);
    NSMutableData *out = [NSMutableData dataWithLength:deflateBound(&stream, input.length)];
    stream.next_in = (Bytef *)input.bytes;
    stream.avail_in = (uInt)input.length;
    stream.next_out = out.mutableBytes;
    stream.avail_out = (uInt)out.length;
    deflate(&stream, Z_FINISH);
    out.length = stream.total_out;
    deflateEnd(&stream);
    return out;
}

// Writes a zip archive with one entry for each name. An entry is deflated
// when its name ends in ".z", and stored otherwise.
static NSString *writeArchive(NSArray *names, NSArray *contents, NSString *comment) {
    NSMutableData *zip = [NSMutableData data];
    NSMutableData *directory = [NSMutableData data];
    for (NSUInteger i = 0; i < names.count; i++) {
        NSData *name = [names[i] dataUsingEncoding:NSUTF8StringEncoding];
        NSData *plain = [contents[i] dataUsingEncoding:NSUTF8StringEncoding];
        BOOL deflated = [names[i] hasSuffix:@".z"];
        NSData *packed = deflated ? rawDeflate(plain) : plain;
        uint32_t crc = (uint32_t)crc32(0, plain.bytes, (uInt)plain.length);
        uint32_t offset = (uint32_t)zip.length;

        put32(zip, 0x04034b50);
        put16(zip, 20); put16(zip, 0); put16(zip, deflated ? 8 : 0); put16(zip, 0); put16(zip, 0);
        put32(zip, crc); put32(zip, (uint32_t)packed.length); put32(zip, (uint32_t)plain.length);
        put16(zip, (uint16_t)name.length);
        put16(zip, 4);  // An extra field, so that the reader must skip it.
        [zip appendData:name];
        put32(zip, 0xdeadbeef);
        [zip appendData:packed];

        put32(directory, 0x02014b50);
        put16(directory, 20); put16(directory, 20); put16(directory, 0); put16(directory, deflated ? 8 : 0);
        put16(directory, 0); put16(directory, 0);
        put32(directory, crc); put32(directory, (uint32_t)packed.length); put32(directory, (uint32_t)plain.length);
        put16(directory, (uint16_t)name.length); put16(directory, 0); put16(directory, 0);
        put16(directory, 0); put16(directory, 0); put32(directory, 0);
        put32(directory, offset);
        [directory appendData:name];
    }
    uint32_t directoryOffset = (uint32_t)zip.length;
    [zip appendData:directory];
    NSData *commentData = [comment dataUsingEncoding:NSUTF8StringEncoding];
    put32(zip, 0x06054b50);
    put16(zip, 0); put16(zip, 0);
    put16(zip, (uint16_t)names.count); put16(zip, (uint16_t)names.count);
    put32(zip, (uint32_t)directory.length); put32(zip, directoryOffset);
    put16(zip, (uint16_t)commentData.length);
    [zip appendData:commentData];

    NSString *path = [BOTestTemporaryDirectory() stringByAppendingPathComponent:@"test.love"];
    [zip writeToFile:path atomically:NO];
    return path;
}

BO_TEST(test_zip_reads_stored_and_deflated_entries) {
    NSString *long_text = [@"" stringByPaddingToLength:5000 withString:@"Jimbo " startingAtIndex:0];
    NSString *path = writeArchive(@[@"version.jkr", @"resources/a.z"], @[@"1.0.1o-FULL", long_text], @"");
    BOZip *zip = bo_zip_open(path.fileSystemRepresentation);
    BO_EXPECT(zip != NULL);
    BO_EQUAL_INT(bo_zip_count(zip), 2);
    BO_EQUAL(@(bo_zip_name(zip, 1)), @"resources/a.z");

    size_t size = 0;
    unsigned char *stored = bo_zip_read(zip, "version.jkr", &size);
    BO_EQUAL_INT(size, 11);
    BO_EQUAL(@((char *)stored), @"1.0.1o-FULL");
    free(stored);

    unsigned char *inflated = bo_zip_read(zip, "resources/a.z", &size);
    BO_EQUAL_INT(size, 5000);
    BO_EQUAL(@((char *)inflated), long_text);
    free(inflated);
    bo_zip_close(zip);
}

BO_TEST(test_zip_finds_the_end_record_after_a_comment) {
    NSString *path = writeArchive(@[@"main.lua"], @[@"love"], @"a comment with PK in it");
    BOZip *zip = bo_zip_open(path.fileSystemRepresentation);
    unsigned char *data = bo_zip_read(zip, "main.lua", NULL);
    BO_EQUAL(@((char *)data), @"love");
    free(data);
    bo_zip_close(zip);
}

BO_TEST(test_zip_gives_null_for_a_missing_entry_or_a_file_that_is_not_a_zip) {
    NSString *path = writeArchive(@[@"main.lua"], @[@"love"], @"");
    BOZip *zip = bo_zip_open(path.fileSystemRepresentation);
    BO_EXPECT(bo_zip_read(zip, "main", NULL) == NULL);
    BO_EXPECT(bo_zip_name(zip, 1) == NULL);
    bo_zip_close(zip);

    NSString *text = [BOTestTemporaryDirectory() stringByAppendingPathComponent:@"not.zip"];
    [@"just some text, no archive" writeToFile:text atomically:NO encoding:NSUTF8StringEncoding error:NULL];
    BO_EXPECT(bo_zip_open(text.fileSystemRepresentation) == NULL);
    BO_EXPECT(bo_zip_open("/no/such/file.love") == NULL);
}
