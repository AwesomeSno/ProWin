#ifndef EngineBridge_h
#define EngineBridge_h

#import <Foundation/Foundation.h>

@interface EngineBridge : NSObject

+ (instancetype)sharedInstance;
- (BOOL)startEngine:(uint64_t)entryPoint;
- (void)stopEngine;
- (BOOL)isEngineRunning;
- (void)setEntryPoint:(uint64_t)address;
- (BOOL)isLoaded;
- (NSString *)getErrorState;
- (uint64_t)getRegisterRAX;
- (void*)getVRAMPointer;
- (int)getDisplayWidth;
- (int)getDisplayHeight;

@end

#endif /* EngineBridge_h */
