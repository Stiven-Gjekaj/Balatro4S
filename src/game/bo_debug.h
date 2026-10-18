// bo_debug: lets a script on the Mac drive the game over SSH.
// A debug build looks at /tmp/Balatro4S.cmd two times each second. When
// the file changed, the app does each line in it. The commands are:
//   tap X Y      a tap at the pixel (X, Y) of the landscape screen
//   target N     sets the chip target of the blind that is on to N
//   dollars N    sets the money to N

#ifndef BO_DEBUG_H
#define BO_DEBUG_H

void bo_debug_poll(double dt);

#endif
