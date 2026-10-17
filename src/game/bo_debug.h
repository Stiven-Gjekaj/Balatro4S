// bo_debug: lets a script on the Mac drive the game over SSH.
// A debug build reads /tmp/Balatro4S.cmd two times each second, does each
// line in it, and deletes the file. The commands are:
//   tap X Y    a tap at the pixel (X, Y) of the landscape screen

#ifndef BO_DEBUG_H
#define BO_DEBUG_H

void bo_debug_poll(double dt);

#endif
