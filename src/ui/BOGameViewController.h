#import <UIKit/UIKit.h>

// Shows the game in an OpenGL ES 2.0 view, in landscape, and sends each
// frame and each touch to bo_game.
@interface BOGameViewController : UIViewController

// Stops the frames while the app is in the background.
- (void)setPaused:(BOOL)paused;

@end
