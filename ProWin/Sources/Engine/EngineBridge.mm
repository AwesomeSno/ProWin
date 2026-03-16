#import "EngineBridge.h"
#include "EngineOrchestrator.h"
#include "DisplayManager.h"

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

@end
