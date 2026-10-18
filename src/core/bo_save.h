// bo_save: writes a run to a file and reads it back, as save_run and
// Game:start_run with a saveTable do in Balatro.
//
// The file holds a header and the bytes of BORun. A file from a build with
// another layout of BORun does not load: the header holds a fingerprint of
// the layout, and the run starts again.

#ifndef BO_SAVE_H
#define BO_SAVE_H

#include "bo_run.h"

// The values of the game that are not in BORun.
typedef struct {
    int endless;  // The player goes on after the win.
} BOSaveExtra;

// The fingerprint of the layout of BORun in this build.
uint32_t bo_save_layout(void);

// Writes run and extra to path. It writes a new file and then renames it,
// so that a stop in the middle keeps the old file. Returns 1 on success.
int bo_save_write(const BORun *run, const BOSaveExtra *extra, const char *path);

// Reads the run of path into run and extra. Returns 0 when there is no
// file, or when the file is not a save of this layout. Then run does not
// change.
int bo_save_read(BORun *run, BOSaveExtra *extra, const char *path);

// Removes the save, as Balatro does at the end of a run.
void bo_save_remove(const char *path);

#endif
