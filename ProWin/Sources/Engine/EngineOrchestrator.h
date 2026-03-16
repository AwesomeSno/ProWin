#ifndef EngineOrchestrator_h
#define EngineOrchestrator_h

#include <cstdint>
#include <atomic>
#include <thread>
#include <string>
#include <functional>
#include "CPUContext.h"
#include <mutex>

namespace ProWin {

struct EngineSnapshot {
    uint64_t rax;
    uint64_t rip;
    uint32_t rflags;
    bool isRunning;
    bool isPaused;
};

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
    
    void setEventCallback(std::function<void(const std::string&)> cb) { m_eventCallback = cb; }

    void pause() { m_isPaused = true; }
    void resume() { m_isPaused = false; }
    bool isPaused() const { return m_isPaused; }

    EngineSnapshot getSnapshot();
    ~EngineOrchestrator();

private:
    EngineOrchestrator();

    void engineLoop(uint64_t entryPoint);
    void setupInitialState(uint64_t entryPoint);

    std::atomic<bool> m_isRunning;
    std::atomic<bool> m_isPaused;
    bool m_isLoaded;
    std::thread m_engineThread;
    CPUContext m_context;
    std::string m_errorState;

    std::mutex m_snapshotMutex;
    EngineSnapshot m_snapshot;
    void updateSnapshot();

    std::function<void(const std::string&)> m_eventCallback;
};

}

#endif /* EngineOrchestrator_h */
