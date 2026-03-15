#include "EngineOrchestrator.h"
#include "InstructionDecoder.h"
#include "InstructionDispatcher.h"
#include <cstdio>
#include <chrono>
#include <thread>

namespace ProWin {

EngineOrchestrator::EngineOrchestrator() : m_isRunning(false) {}

EngineOrchestrator::~EngineOrchestrator() {
    stop();
}

EngineOrchestrator& EngineOrchestrator::getInstance() {
    static EngineOrchestrator instance;
    return instance;
}

void EngineOrchestrator::start(uint64_t entryPoint) {
    if (m_isRunning) return;
    
    printf("[ProWin] C++ Engine: Starting thread at 0x%llx\n", entryPoint);
    fflush(stdout);
    
    m_isRunning = true;
    m_engineThread = std::thread(&EngineOrchestrator::engineLoop, this, entryPoint);
}

void EngineOrchestrator::stop() {
    if (!m_isRunning) return;
    
    m_isRunning = false;
    if (m_engineThread.joinable()) {
        m_engineThread.join();
    }
}

void EngineOrchestrator::engineLoop(uint64_t entryPoint) {
    printf("[ProWin] C++ Engine: Thread entered at 0x%llx\n", entryPoint);
    fflush(stdout);
    
    setupInitialState(entryPoint);
    
    // Interpreter Loop
    while (m_isRunning) {
        // 1. Fetch
        uint8_t* currentPC = (uint8_t*)m_context.rip;
        
        // 2. Decode
        Instruction inst = InstructionDecoder::decode(currentPC);
        
        // 3. Dispatch (Execute)
        if (!InstructionDispatcher::execute(inst, m_context)) {
            printf("[ProWin] Engine: Graceful termination requested.\n");
            fflush(stdout);
            m_isRunning = false;
            break;
        }
    }
    
    printf("[ProWin] C++ Engine: Thread terminated\n");
    fflush(stdout);
}

void EngineOrchestrator::setupInitialState(uint64_t entryPoint) {
    m_context.rip = entryPoint;
    // Simulate a basic stack allocation
    m_context.rsp = 0x7FFFFFFF0000; // Common high memory stack address simulation
    printf("[ProWin] C++ Engine: Initialized RIP to 0x%llx\n", m_context.rip);
    fflush(stdout);
}

}
