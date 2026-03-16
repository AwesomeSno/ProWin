#import "EngineBridge.h"
#include "EngineOrchestrator.h"
#include "DisplayManager.h"
#include "InputState.h"

@implementation EngineBridge

+ (instancetype)sharedInstance {
    static EngineBridge *instance = nil;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        instance = [[EngineBridge alloc] init];
    });
    return instance;
}

- (BOOL)startEngine:(uint64_t)entryPoint {
    auto& engine = ProWin::EngineOrchestrator::getInstance();
    
    // Set up Swift-compatible callback
    engine.setEventCallback([self](const std::string& event) {
        if (self.onEngineEvent) {
            NSString *nsEvent = [NSString stringWithUTF8String:event.c_str()];
            dispatch_async(dispatch_get_main_queue(), ^{
                self.onEngineEvent(nsEvent);
            });
        }
    });

    return engine.start(entryPoint);
}

- (NSString *)getErrorState {
    const char* err = ProWin::EngineOrchestrator::getInstance().getErrorState();
    return [NSString stringWithUTF8String:err];
}

- (void)stopEngine {
    ProWin::EngineOrchestrator::getInstance().stop();
}

- (BOOL)isEngineRunning {
    return ProWin::EngineOrchestrator::getInstance().isRunning();
}

- (void)setEntryPoint:(uint64_t)address {
    ProWin::EngineOrchestrator::getInstance().setEntryPoint(address);
}

- (BOOL)isLoaded {
    return ProWin::EngineOrchestrator::getInstance().isLoaded();
}

- (uint64_t)getRegisterRAX {
    return ProWin::EngineOrchestrator::getInstance().getContext().rax;
}

- (EngineSnapshotStruct)getSnapshot {
    ProWin::EngineSnapshot cppSnapshot = ProWin::EngineOrchestrator::getInstance().getSnapshot();
    EngineSnapshotStruct snapshot;
    snapshot.rax = cppSnapshot.rax;
    snapshot.rip = cppSnapshot.rip;
    snapshot.rflags = cppSnapshot.rflags;
    snapshot.isRunning = cppSnapshot.isRunning;
    snapshot.isPaused = cppSnapshot.isPaused;
    return snapshot;
}

- (void)pauseEngine {
    ProWin::EngineOrchestrator::getInstance().pause();
}

- (void)resumeEngine {
    ProWin::EngineOrchestrator::getInstance().resume();
}

- (BOOL)isPaused {
    return ProWin::EngineOrchestrator::getInstance().isPaused();
}

- (void*)getVRAMPointer {
    return ProWin::DisplayManager::getInstance().getVRAM();
}

- (uint64_t)getVRAMSize {
    return ProWin::DisplayManager::getInstance().getVRAMSize();
}

- (id<MTLBuffer>)getVRAMBufferWithDevice:(id<MTLDevice>)device {
    void* ptr = [self getVRAMPointer];
    uint64_t size = [self getVRAMSize];
    if (!ptr || size == 0) return nil;
    
    return [device newBufferWithBytesNoCopy:ptr
                                     length:size
                                    options:MTLResourceStorageModeShared
                                deallocator:nil];
}

- (BOOL)isVRAMValid {
    // Basic check: is engine running AND is there a valid pointer?
    return [self isEngineRunning] && ([self getVRAMPointer] != nil);
}

- (int)getDisplayWidth {
    return ProWin::DisplayManager::getInstance().getWidth();
}

- (int)getDisplayHeight {
    return ProWin::DisplayManager::getInstance().getHeight();
}

- (void)updateInputState:(uint32_t)playerIndex
                 buttons:(uint16_t)buttons
            leftStickX:(int16_t)lx
            leftStickY:(int16_t)ly
           rightStickX:(int16_t)rx
           rightStickY:(int16_t)ry {
    ProWin::WinXInputState state;
    state.buttons = buttons;
    state.leftStickX = lx;
    state.leftStickY = ly;
    state.rightStickX = rx;
    state.rightStickY = ry;
    state.packetNumber++; // Basic increment for now
    ProWin::InputStateManager::getInstance().updateState(playerIndex, state);
}

- (void)playSound:(uint64_t)bufferPtr size:(uint32_t)size {
    // In a real implementation, we'd pass this to AudioManager.shared.
    // For now, let's just log that we reached the bridge.
    printf("[EngineBridge] Audio data received: 0x%llx (size: %u)\n", bufferPtr, size);
    // TODO: Connect to AudioManager.shared in a future update or via delegate.
}

@end
