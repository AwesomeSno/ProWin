#ifndef EngineOrchestrator_h
#define EngineOrchestrator_h

#include <cstdint>
#include <atomic>
#include <thread>

namespace ProWin {

class EngineOrchestrator {
public:
    static EngineOrchestrator& getInstance();

    void start(uint64_t entryPoint);
    void stop();
    bool isRunning() const { return m_isRunning; }

private:
    EngineOrchestrator();
    ~EngineOrchestrator();

    void engineLoop(uint64_t entryPoint);

    std::atomic<bool> m_isRunning;
    std::thread m_engineThread;
};

}

#endif /* EngineOrchestrator_h */
