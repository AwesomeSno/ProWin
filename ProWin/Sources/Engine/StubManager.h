#ifndef StubManager_h
#define StubManager_h

#include <string>
#include <unordered_map>
#include <cstdint>

namespace ProWin {

struct CPUContext;

class StubManager {
public:
    typedef uint64_t (*StubFunction)(CPUContext& ctx);

    static StubManager& getInstance();

    void registerStub(const std::string& dllName, const std::string& funcName, StubFunction func);
    void* getStub(const std::string& dllName, const std::string& funcName);
    void* getOrCreateStub(const std::string& dllName, const std::string& funcName);
    uint64_t callStub(const std::string& dllName, const std::string& funcName, CPUContext& ctx);

private:
    StubManager() {}
    std::unordered_map<std::string, std::unordered_map<std::string, void*>> m_stubs;
};

}

#endif /* StubManager_h */
