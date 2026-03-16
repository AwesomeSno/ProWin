#ifndef EngineBridge_h
#define EngineBridge_h

#import <Foundation/Foundation.h>
#import <Metal/Metal.h>

typedef struct {
    uint64_t rax;
    uint64_t rip;
    uint32_t rflags;
    BOOL isRunning;
    BOOL isPaused;
} EngineSnapshotStruct;

@interface EngineBridge : NSObject

+ (instancetype)sharedInstance;
- (BOOL)startEngine:(uint64_t)entryPoint;
- (void)stopEngine;
- (BOOL)isEngineRunning;
- (void)setEntryPoint:(uint64_t)address;
- (BOOL)isLoaded;
- (NSString *)getErrorState;
- (uint64_t)getRegisterRAX;
- (EngineSnapshotStruct)getSnapshot;
- (void)pauseEngine;
- (void)resumeEngine;
- (BOOL)isPaused;
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

@property (nonatomic, copy) void (^onEngineEvent)(NSString *event);

@end

#endif /* EngineBridge_h */
