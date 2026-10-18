#include "bo_debug.h"

#ifdef BO_DEBUG

#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include "bo_game.h"

static const char *kCommands = "/tmp/Balatro4S.cmd";
static double gWait;
static struct timespec gLastChange;

void bo_debug_poll(double dt) {
    gWait -= dt;
    if (gWait > 0) return;
    gWait = 0.5;
    // A file that root writes over SSH cannot be deleted by the app, which
    // runs as mobile. So the app does a file only when its time changes.
    struct stat info;
    if (stat(kCommands, &info) != 0) return;
    if (info.st_mtimespec.tv_sec == gLastChange.tv_sec && info.st_mtimespec.tv_nsec == gLastChange.tv_nsec) return;
    gLastChange = info.st_mtimespec;
    FILE *file = fopen(kCommands, "r");
    if (!file) return;
    char line[256];
    while (fgets(line, sizeof(line), file)) {
        float x, y, x2, y2;
        if (sscanf(line, "drag %f %f %f %f", &x, &y, &x2, &y2) == 4) {
            // A drag in eight steps, as a finger moves.
            bo_game_touch(BO_TOUCH_BEGAN, x, y);
            for (int i = 1; i <= 8; i++) {
                bo_game_touch(BO_TOUCH_MOVED, x + (x2 - x) * i / 8, y + (y2 - y) * i / 8);
            }
            bo_game_touch(BO_TOUCH_ENDED, x2, y2);
            fprintf(stderr, "debug: drag %.0f %.0f to %.0f %.0f\n", x, y, x2, y2);
        } else if (sscanf(line, "tap %f %f", &x, &y) == 2) {
            bo_game_touch(BO_TOUCH_BEGAN, x, y);
            bo_game_touch(BO_TOUCH_ENDED, x, y);
            fprintf(stderr, "debug: tap %.0f %.0f\n", x, y);
        } else {
            bo_game_debug(line);
        }
    }
    fclose(file);
    remove(kCommands);
}

#else

void bo_debug_poll(double dt) {
    (void)dt;
}

#endif
