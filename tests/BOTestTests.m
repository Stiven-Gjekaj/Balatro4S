// Tests of the test runner itself.

#import "BOTest.h"

BO_TEST(test_runner_compares_equal_objects) {
    BO_EQUAL(([NSString stringWithFormat:@"%d", 42]), @"42");
    BO_EQUAL_INT(6 * 7, 42);
    BO_EXPECT(YES);
}

BO_TEST(test_runner_makes_an_empty_temporary_directory) {
    NSString *first = BOTestTemporaryDirectory();
    NSString *second = BOTestTemporaryDirectory();
    BOOL isDirectory = NO;
    BO_EXPECT([[NSFileManager defaultManager] fileExistsAtPath:first isDirectory:&isDirectory] && isDirectory);
    BO_EXPECT(![first isEqualToString:second]);
    BO_EQUAL_INT([[NSFileManager defaultManager] contentsOfDirectoryAtPath:first error:NULL].count, 0);
}
