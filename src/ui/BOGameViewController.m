#import <OpenGLES/EAGL.h>
#import <OpenGLES/ES2/gl.h>
#import <QuartzCore/QuartzCore.h>
#import "BOGameViewController.h"
#include "bo_game.h"
#include "bo_sound.h"

// The view of the game is a CAEAGLLayer.
@interface BOGLView : UIView
@end

@implementation BOGLView
+ (Class)layerClass {
    return [CAEAGLLayer class];
}
@end

@implementation BOGameViewController {
    EAGLContext *_context;
    GLuint _framebuffer;
    GLuint _colorbuffer;
    GLint _width;
    GLint _height;
    CADisplayLink *_link;
    CFTimeInterval _last;
    BOOL _started;
    UILabel *_message;
}

- (void)loadView {
    BOGLView *view = [[BOGLView alloc] initWithFrame:[UIScreen mainScreen].bounds];
    view.multipleTouchEnabled = NO;
    view.contentScaleFactor = [UIScreen mainScreen].scale;
    CAEAGLLayer *layer = (CAEAGLLayer *)view.layer;
    // A layer that is not opaque goes through the compositor of iOS 6, so
    // that RecTunnelX can read it from the framebuffer. An opaque layer
    // goes to the screen by itself, and the capture is black. The frame
    // rate stays at 60.
    layer.opaque = NO;
    layer.drawableProperties = @{kEAGLDrawablePropertyRetainedBacking: @NO,
                                 kEAGLDrawablePropertyColorFormat: kEAGLColorFormatRGBA8};
    self.view = view;
}

- (void)viewDidLoad {
    [super viewDidLoad];
    _context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES2];
    [EAGLContext setCurrentContext:_context];
    glGenFramebuffers(1, &_framebuffer);
    glGenRenderbuffers(1, &_colorbuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, _framebuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, _colorbuffer);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, _colorbuffer);
}

- (void)viewDidLayoutSubviews {
    [super viewDidLayoutSubviews];
    // The storage follows the size of the layer, which is landscape after
    // the rotation.
    [EAGLContext setCurrentContext:_context];
    glBindRenderbuffer(GL_RENDERBUFFER, _colorbuffer);
    [_context renderbufferStorage:GL_RENDERBUFFER fromDrawable:(CAEAGLLayer *)self.view.layer];
    glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_WIDTH, &_width);
    glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_HEIGHT, &_height);
    if (_width > _height && !_started) [self start];
}

- (void)start {
    _started = YES;
    NSString *love = @"/var/mobile/Media/Balatro4S/Balatro.love";
    if (!bo_game_start(love.fileSystemRepresentation, _width, _height)) {
        [self showMessage:@(bo_game_error())];
        return;
    }
    _link = [CADisplayLink displayLinkWithTarget:self selector:@selector(frame:)];
    [_link addToRunLoop:[NSRunLoop mainRunLoop] forMode:NSRunLoopCommonModes];
}

- (void)showMessage:(NSString *)text {
    _message = [[UILabel alloc] initWithFrame:CGRectInset(self.view.bounds, 24, 24)];
    _message.numberOfLines = 0;
    _message.textAlignment = NSTextAlignmentCenter;
    _message.backgroundColor = [UIColor clearColor];
    _message.textColor = [UIColor whiteColor];
    _message.font = [UIFont boldSystemFontOfSize:15];
    _message.text = text;
    self.view.backgroundColor = [UIColor colorWithRed:0x37 / 255.0 green:0x42 / 255.0 blue:0x44 / 255.0 alpha:1];
    [self.view addSubview:_message];
}

- (void)frame:(CADisplayLink *)link {
    CFTimeInterval now = link.timestamp;
    double dt = _last ? now - _last : 1.0 / 60;
    _last = now;
    [EAGLContext setCurrentContext:_context];
    glBindFramebuffer(GL_FRAMEBUFFER, _framebuffer);
    bo_game_frame(dt > 0.1 ? 0.1 : dt);
    glBindRenderbuffer(GL_RENDERBUFFER, _colorbuffer);
    [_context presentRenderbuffer:GL_RENDERBUFFER];
}

- (void)touch:(NSSet *)touches phase:(int)phase {
    UITouch *touch = [touches anyObject];
    CGPoint point = [touch locationInView:self.view];
    CGFloat scale = self.view.contentScaleFactor;
    bo_game_touch(phase, (float)(point.x * scale), (float)(point.y * scale));
}

- (void)touchesBegan:(NSSet *)touches withEvent:(UIEvent *)event {
    [self touch:touches phase:BO_TOUCH_BEGAN];
}

- (void)touchesMoved:(NSSet *)touches withEvent:(UIEvent *)event {
    [self touch:touches phase:BO_TOUCH_MOVED];
}

- (void)touchesEnded:(NSSet *)touches withEvent:(UIEvent *)event {
    [self touch:touches phase:BO_TOUCH_ENDED];
}

- (void)touchesCancelled:(NSSet *)touches withEvent:(UIEvent *)event {
    [self touch:touches phase:BO_TOUCH_ENDED];
}

- (void)setPaused:(BOOL)paused {
    _link.paused = paused;
    _last = 0;
    bo_sound_set_active(!paused);
}

- (BOOL)prefersStatusBarHidden {
    return YES;
}

- (BOOL)shouldAutorotate {
    return YES;
}

- (UIInterfaceOrientationMask)supportedInterfaceOrientations {
    return UIInterfaceOrientationMaskLandscape;
}

- (UIInterfaceOrientation)preferredInterfaceOrientationForPresentation {
    return UIInterfaceOrientationLandscapeRight;
}

@end
