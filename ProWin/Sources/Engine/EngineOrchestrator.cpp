#include "EngineOrchestrator.h"
#include "InstructionDecoder.h"
#include "InstructionDispatcher.h"
#include <iostream>
#include <chrono>

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
    std::cout << "[ProWin] C++ Engine: Thread started at " << std::hex << entryPoint << std::endl;
    
    setupInitialState(entryPoint);
    
    // Interpreter Loop
    while (m_isRunning) {
        // 1. Fetch
        uint8_t* currentPC = (uint8_t*)m_context.rip;
        
        // 2. Decode
        Instruction inst = InstructionDecoder::decode(currentPC);
        
        // 3. Dispatch (Execute)
        if (!InstructionDispatcher::execute(inst, m_context)) {
            std::cout << "[ProWin] Engine: Graceful termination requested." << std::endl;
            m_isRunning = false;
            break;
        }

        // Throttle for simulation/debugging if needed
        // std::this_thread::sleep_for(std::chrono::microseconds(10));
    }
    
    std::cout << "[ProWin] C++ Engine: Thread terminated" << std::endl;
}

void EngineOrchestrator::setupInitialState(uint64_t entryPoint) {
    m_context.rip = entryPoint;
    // Simulate a basic stack allocation
    m_context.rsp = 0x7FFFFFFF0000; // Common high memory stack address simulation
    std::cout << "[ProWin] C++ Engine: Initialized RIP to " << std::hex << m_context.rip << std::endl;
}

}
