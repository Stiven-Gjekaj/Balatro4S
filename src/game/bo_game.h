// bo_game: the game loop. The view controller calls it for each frame
// and each touch. The coordinates are pixels, with (0, 0) at the top left.

#ifndef BO_GAME_H
#define BO_GAME_H

enum {
    BO_TOUCH_BEGAN = 0,
    BO_TOUCH_MOVED = 1,
    BO_TOUCH_ENDED = 2,
};

// Opens Balatro.love at love_path, loads the textures, and starts a run.
// Returns 0 on a failure. Then bo_game_error() tells the reason.
int bo_game_start(const char *love_path, int width, int height);
const char *bo_game_error(void);

// Moves the game on by dt seconds and draws one frame.
void bo_game_frame(double dt);

void bo_game_touch(int phase, float x, float y);

#endif
