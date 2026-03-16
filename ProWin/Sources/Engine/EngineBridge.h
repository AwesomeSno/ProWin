#ifndef EngineBridge_h
#define EngineBridge_h

#import <Foundation/Foundation.h>

@interface EngineBridge : NSObject

+ (instancetype)sharedInstance;
- (void)startEngine:(uint64_t)entryPoint;
- (void)stopEngine;
- (BOOL)isEngineRunning;
- (uint64_t)getRegisterRAX;
- (void*)getVRAMPointer;
- (int)getDisplayWidth;
- (int)getDisplayHeight;

@end

#endif /* EngineBridge_h */
