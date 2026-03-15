#import <Foundation/Foundation.h>

@interface GraphicsBridge : NSObject

+ (instancetype)sharedInstance;
- (void)presentFrame;
- (void*)getMetalDevice;

@end
