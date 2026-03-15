#ifndef StubManager_h
#define StubManager_h

#include <string>
#include <unordered_map>
#include <cstdint>

namespace ProWin {

class StubManager {
public:
    typedef void (*StubFunction)();

    static StubManager& getInstance();

    void registerStub(const std::string& dllName, const std::string& funcName, StubFunction func);
    void* getStub(const std::string& dllName, const std::string& funcName);

private:
    StubManager() {}
    std::unordered_map<std::string, std::unordered_map<std::string, void*>> m_stubs;
};

}

#endif /* StubManager_h */
