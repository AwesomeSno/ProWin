#ifndef EngineOrchestrator_h
#define EngineOrchestrator_h

#include <cstdint>
#include <atomic>
#include <thread>
#include <string>
#include "CPUContext.h"

namespace ProWin {

class EngineOrchestrator {
public:
    static EngineOrchestrator& getInstance();

    bool start(uint64_t entryPoint);
    void stop();
    bool isRunning() const { return m_isRunning; }
    bool isLoaded() const { return m_isLoaded; }
    void setEntryPoint(uint64_t address) { 
        m_context.rip = address; 
        m_isLoaded = true;
    }
    CPUContext& getContext() { return m_context; }
    const char* getErrorState() const { return m_errorState.c_str(); }

private:
    EngineOrchestrator();
    ~EngineOrchestrator();

    void engineLoop(uint64_t entryPoint);
    void setupInitialState(uint64_t entryPoint);

    std::atomic<bool> m_isRunning;
    bool m_isLoaded;
    std::thread m_engineThread;
    CPUContext m_context;
    std::string m_errorState;
};

}

#endif /* EngineOrchestrator_h */
