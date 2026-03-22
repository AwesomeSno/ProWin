#import <Foundation/Foundation.h>

@interface AudioBridge : NSObject

+ (instancetype)sharedInstance;

/// Queue raw PCM float data for playback
- (void)playPCMData:(const void *)data length:(NSUInteger)length channels:(NSUInteger)channels sampleRate:(double)sampleRate;

/// Legacy API
- (void)playSound;
- (void)stopSound;

@end
