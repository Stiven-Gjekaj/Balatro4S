#import "BOAppDelegate.h"
#import "BOGameViewController.h"

@implementation BOAppDelegate {
    BOGameViewController *_game;
}

- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)options {
    [application setStatusBarHidden:YES];
    // A card game has no touch for a long time while the player thinks.
    // The screen stays on while the game is in front.
    application.idleTimerDisabled = YES;
    self.window = [[UIWindow alloc] initWithFrame:[UIScreen mainScreen].bounds];
    _game = [[BOGameViewController alloc] init];
    self.window.rootViewController = _game;
    [self.window makeKeyAndVisible];
    return YES;
}

- (void)applicationWillResignActive:(UIApplication *)application {
    // iOS stops the app when it draws with OpenGL in the background.
    [_game setPaused:YES];
}

- (void)applicationDidBecomeActive:(UIApplication *)application {
    [_game setPaused:NO];
}

@end
