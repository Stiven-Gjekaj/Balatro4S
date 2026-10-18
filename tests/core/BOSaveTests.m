// Tests of the save of a run.

#import "BOTest.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "bo_save.h"

static const char *temp_path(void) {
    static char path[256];
    snprintf(path, sizeof(path), "%s/bo-save-%d.b4s", NSTemporaryDirectory().fileSystemRepresentation, getpid());
    return path;
}

BO_TEST(test_save_reads_back_the_same_run) {
    BORun *run = calloc(1, sizeof(BORun));
    bo_run_start_deck(run, "ABCDEFGH", BO_B_GHOST, 3);
    bo_run_select_blind(run);
    run->dollars = 42;
    BOSaveExtra extra = {1};
    const char *path = temp_path();
    BO_EXPECT(bo_save_write(run, &extra, path));
    BORun *loaded = calloc(1, sizeof(BORun));
    BOSaveExtra loadedExtra = {0};
    BO_EXPECT(bo_save_read(loaded, &loadedExtra, path));
    BO_EXPECT(memcmp(run, loaded, sizeof(BORun)) == 0);
    BO_EQUAL_INT(loadedExtra.endless, 1);
    // The run goes on as the first one does: the same draw after a play.
    BOScore a, b;
    BO_EXPECT(bo_run_play(run, 1, &a));
    BO_EXPECT(bo_run_play(loaded, 1, &b));
    BO_EXPECT(memcmp(run, loaded, sizeof(BORun)) == 0);
    bo_save_remove(path);
    BO_EXPECT(!bo_save_read(loaded, NULL, path));
    free(run);
    free(loaded);
}

BO_TEST(test_save_refuses_a_file_of_another_layout_or_a_short_file) {
    BORun *run = calloc(1, sizeof(BORun));
    bo_run_start(run, "ABCDEFGH");
    const char *path = temp_path();
    BO_EXPECT(bo_save_write(run, NULL, path));
    // Change one byte of the layout in the header.
    FILE *file = fopen(path, "r+b");
    fseek(file, 8, SEEK_SET);
    int byte = fgetc(file);
    fseek(file, 8, SEEK_SET);
    fputc(byte ^ 0xff, file);
    fclose(file);
    BORun *loaded = calloc(1, sizeof(BORun));
    loaded->dollars = 7;
    BO_EXPECT(!bo_save_read(loaded, NULL, path));
    BO_EQUAL_INT(loaded->dollars, 7);
    // A file that stops in the middle of the run.
    BO_EXPECT(bo_save_write(run, NULL, path));
    BO_EXPECT(truncate(path, 1000) == 0);
    BO_EXPECT(!bo_save_read(loaded, NULL, path));
    BO_EQUAL_INT(loaded->dollars, 7);
    bo_save_remove(path);
    free(run);
    free(loaded);
}
