#include "EngineOrchestrator.h"
#include "InstructionDecoder.h"
#include "InstructionDispatcher.h"
#include "DisplayManager.h"
#include "EngineConstants.h"
#include <cstdio>
#include <chrono>
#include <thread>

namespace ProWin {

EngineOrchestrator::EngineOrchestrator() : m_isRunning(false), m_isPaused(false), m_isLoaded(false) {
    m_snapshot = {0, 0, 0, false, false};
}

EngineOrchestrator::~EngineOrchestrator() {
    stop();
}

EngineOrchestrator& EngineOrchestrator::getInstance() {
    static EngineOrchestrator instance;
    return instance;
}

bool EngineOrchestrator::start(uint64_t entryPoint) {
    if (m_isRunning) return true;
    
    m_errorState.clear();
    try {
        DisplayManager::getInstance().initialize(PROWIN_VRAM_WIDTH, PROWIN_VRAM_HEIGHT);
    } catch (const std::exception& e) {
        printf("[ProWin] EngineOrchestrator: Initialization failed: %s\n", e.what());
        m_errorState = e.what();
        if (m_eventCallback) m_eventCallback("error:" + std::string(e.what()));
        return false;
    }
    
    // Safety: If there's a finished thread from a previous run, we MUST join it 
    // before assigning a new thread to m_engineThread, otherwise C++ will crash.
    if (m_engineThread.joinable()) {
        m_engineThread.join();
    }
    
    printf("[ProWin] C++ Engine: Starting thread at 0x%llx\n", entryPoint);
    fflush(stdout);
    
    m_isRunning = true;
    if (m_eventCallback) m_eventCallback("start");
    m_engineThread = std::thread(&EngineOrchestrator::engineLoop, this, entryPoint);
    return true;
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
        // Handle Pause
        if (m_isPaused) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            updateSnapshot();
            continue;
        }

        // 1. Fetch
        uint8_t* currentPC = (uint8_t*)m_context.rip;
        
        // 2. Decode
        Instruction inst = InstructionDecoder::decode(currentPC);
        
        // 3. Dispatch (Execute)
        if (!InstructionDispatcher::execute(inst, m_context)) {
            printf("[ProWin] Engine: Graceful termination requested.\n");
            fflush(stdout);
            m_isRunning = false;
            if (m_eventCallback) m_eventCallback("halt");
        }

        updateSnapshot();
    }
    
    printf("[ProWin] C++ Engine: Thread terminated\n");
    fflush(stdout);
    if (m_eventCallback) m_eventCallback("finish");
}

void EngineOrchestrator::setupInitialState(uint64_t entryPoint) {
    if (!m_isLoaded) {
        m_context.rip = entryPoint;
        m_isLoaded = true;
    }
    // Simulate a basic stack allocation
    m_context.rsp = 0x7FFFFFFF0000;
    
    // Pass VRAM pointer to RDI so the test binary knows where to write
    m_context.rdi = (uint64_t)DisplayManager::getInstance().getVRAM();
    
    printf("[ProWin] C++ Engine: Initialized RIP to 0x%llx, RDI to 0x%llx\n", 
           m_context.rip, m_context.rdi);
    fflush(stdout);
    updateSnapshot();
}

EngineSnapshot EngineOrchestrator::getSnapshot() {
    std::lock_guard<std::mutex> lock(m_snapshotMutex);
    return m_snapshot;
}

void EngineOrchestrator::updateSnapshot() {
    std::lock_guard<std::mutex> lock(m_snapshotMutex);
    m_snapshot.rax = m_context.rax;
    m_snapshot.rip = m_context.rip;
    m_snapshot.rflags = m_context.rflags;
    m_snapshot.isRunning = m_isRunning;
    m_snapshot.isPaused = m_isPaused;
}

}
