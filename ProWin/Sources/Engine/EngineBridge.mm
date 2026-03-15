#import "EngineBridge.h"
#include "EngineOrchestrator.h"

@implementation EngineBridge

+ (instancetype)sharedInstance {
    static EngineBridge *instance = nil;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        instance = [[EngineBridge alloc] init];
    });
    return instance;
}

- (void)startEngine:(uint64_t)entryPoint {
    ProWin::EngineOrchestrator::getInstance().start(entryPoint);
}

- (void)stopEngine {
    ProWin::EngineOrchestrator::getInstance().stop();
}

- (BOOL)isEngineRunning {
    return ProWin::EngineOrchestrator::getInstance().isRunning();
}

@end
