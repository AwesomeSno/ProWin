#ifndef AudioBridge_h
#define AudioBridge_h

#import <Foundation/Foundation.h>

@interface AudioBridge : NSObject

+ (instancetype)sharedInstance;
- (void)playSound;
- (void)stopSound;

@end

#endif /* AudioBridge_h */
