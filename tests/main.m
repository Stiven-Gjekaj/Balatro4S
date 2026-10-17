// Runs each registered test and prints one line for each failure.
// The exit status is 0 when all tests pass.

#import "BOTest.h"

#define BO_MAX_TESTS 512

static const char *gNames[BO_MAX_TESTS];
static BOTestFunction gFunctions[BO_MAX_TESTS];
static int gCount;
static int gFailures;
static int gSkipped;
static const char *gCurrent;

void BOTestRegister(const char *name, BOTestFunction function) {
    if (gCount < BO_MAX_TESTS) {
        gNames[gCount] = name;
        gFunctions[gCount] = function;
        gCount++;
    }
}

void BOTestFail(const char *file, int line, NSString *message) {
    gFailures++;
    const char *base = strrchr(file, '/');
    printf("FAIL %s (%s:%d): %s\n", gCurrent, base ? base + 1 : file, line, message.UTF8String);
}

void BOTestSkip(void) {
    gSkipped++;
}

BOOL BOTestWait(BOOL *done, NSTimeInterval seconds) {
    NSDate *until = [NSDate dateWithTimeIntervalSinceNow:seconds];
    while (!*done && [until timeIntervalSinceNow] > 0) {
        [[NSRunLoop currentRunLoop] runMode:NSDefaultRunLoopMode beforeDate:[NSDate dateWithTimeIntervalSinceNow:0.05]];
    }
    return *done;
}

NSString *BOTestTemporaryDirectory(void) {
    NSString *template = [NSTemporaryDirectory() stringByAppendingPathComponent:@"botest.XXXXXX"];
    char path[PATH_MAX];
    strlcpy(path, template.fileSystemRepresentation, sizeof(path));
    if (!mkdtemp(path)) return nil;
    return [[NSFileManager defaultManager] stringWithFileSystemRepresentation:path length:strlen(path)];
}

int main(int argc, char **argv) {
    int failedTests = 0;
    for (int i = 0; i < gCount; i++) {
        if (argc > 1 && !strstr(gNames[i], argv[1])) continue;
        @autoreleasepool {
            int before = gFailures;
            gCurrent = gNames[i];
            gFunctions[i]();
            if (gFailures != before) failedTests++;
        }
    }
    printf("%d tests, %d failed, %d skipped\n", gCount, failedTests, gSkipped);
    return failedTests == 0 ? 0 : 1;
}
