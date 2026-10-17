// shot: save a screenshot of the phone to a PNG file.
// It does not put the image in the Camera Roll.
// Usage: shot OUTPUT.png

#import <UIKit/UIKit.h>

// A private function of UIKit. iOS 6 exports it.
CGImageRef UIGetScreenImage(void);

int main(int argc, char **argv) {
    @autoreleasepool {
        if (argc < 2) {
            fprintf(stderr, "usage: shot OUTPUT.png\n");
            return 2;
        }
        CGImageRef image = UIGetScreenImage();
        if (!image) {
            fprintf(stderr, "shot: no screen image\n");
            return 1;
        }
        NSData *png = UIImagePNGRepresentation([UIImage imageWithCGImage:image]);
        CGImageRelease(image);
        if (![png writeToFile:@(argv[1]) atomically:YES]) {
            fprintf(stderr, "shot: cannot write %s\n", argv[1]);
            return 1;
        }
        printf("%s: %lu bytes\n", argv[1], (unsigned long)png.length);
    }
    return 0;
}
