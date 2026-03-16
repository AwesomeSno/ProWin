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
    printf("[ProWin] EngineBridge: startEngine called with 0x%llx\n", entryPoint);
    fflush(stdout);
    return ProWin::EngineOrchestrator::getInstance().start(entryPoint);
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

- (void*)getVRAMPointer {
    return ProWin::DisplayManager::getInstance().getVRAM();
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
