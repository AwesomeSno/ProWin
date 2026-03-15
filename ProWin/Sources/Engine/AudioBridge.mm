#import "AudioBridge.h"

@implementation AudioBridge

+ (instancetype)sharedInstance {
    static AudioBridge *instance = nil;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        instance = [[AudioBridge alloc] init];
    });
    return instance;
}

- (void)playSound {
    NSLog(@"[ProWin] AudioBridge: playSound called");
}

- (void)stopSound {
    NSLog(@"[ProWin] AudioBridge: stopSound called");
}

@end
