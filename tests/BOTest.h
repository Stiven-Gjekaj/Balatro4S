// BOTest: a small test runner that operates on macOS and on iOS 6.
// Each test is a function that BO_TEST makes and registers at load time.

#import <Foundation/Foundation.h>

typedef void (*BOTestFunction)(void);

void BOTestRegister(const char *name, BOTestFunction function);
void BOTestFail(const char *file, int line, NSString *message);
void BOTestSkip(void);

#define BO_TEST(name)                                                         \
    static void name(void);                                                   \
    __attribute__((constructor)) static void name##_register(void) {          \
        BOTestRegister(#name, name);                                          \
    }                                                                         \
    static void name(void)

#define BO_EXPECT(condition)                                                  \
    do {                                                                      \
        if (!(condition)) BOTestFail(__FILE__, __LINE__, @"expected: " #condition); \
    } while (0)

#define BO_EQUAL(actual, expected)                                            \
    do {                                                                      \
        id bo_a = (actual), bo_e = (expected);                                \
        if (!(bo_a == bo_e || [bo_a isEqual:bo_e]))                           \
            BOTestFail(__FILE__, __LINE__, [NSString stringWithFormat:        \
                @"%s is <%@>, expected <%@>", #actual, bo_a, bo_e]);           \
    } while (0)

#define BO_EQUAL_INT(actual, expected)                                        \
    do {                                                                      \
        long long bo_a = (long long)(actual), bo_e = (long long)(expected);   \
        if (bo_a != bo_e)                                                     \
            BOTestFail(__FILE__, __LINE__, [NSString stringWithFormat:        \
                @"%s is %lld, expected %lld", #actual, bo_a, bo_e]);           \
    } while (0)

// Runs the run loop until done is YES or the time is over.
BOOL BOTestWait(BOOL *done, NSTimeInterval seconds);

// Makes a new empty directory for one test and returns its path.
NSString *BOTestTemporaryDirectory(void);
