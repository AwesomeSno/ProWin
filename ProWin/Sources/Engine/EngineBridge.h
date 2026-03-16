#ifndef EngineBridge_h
#define EngineBridge_h

#import <Foundation/Foundation.h>
#import <Metal/Metal.h>

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
- (uint64_t)getVRAMSize;
- (id<MTLBuffer>)getVRAMBufferWithDevice:(id<MTLDevice>)device;
- (BOOL)isVRAMValid;
- (int)getDisplayWidth;
- (int)getDisplayHeight;

// Hardware Bridge
- (void)updateInputState:(uint32_t)playerIndex
                 buttons:(uint16_t)buttons
            leftStickX:(int16_t)lx
            leftStickY:(int16_t)ly
           rightStickX:(int16_t)rx
           rightStickY:(int16_t)ry;

- (void)playSound:(uint64_t)bufferPtr size:(uint32_t)size;

@end

#endif /* EngineBridge_h */
