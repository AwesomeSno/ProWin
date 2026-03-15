#include "StubManager.h"
#include <iostream>

namespace ProWin {

StubManager& StubManager::getInstance() {
    static StubManager instance;
    return instance;
}

void StubManager::registerStub(const std::string& dllName, const std::string& funcName, StubFunction func) {
    m_stubs[dllName][funcName] = (void*)func;
}

void* StubManager::getStub(const std::string& dllName, const std::string& funcName) {
    auto dllIt = m_stubs.find(dllName);
    if (dllIt != m_stubs.end()) {
        auto funcIt = dllIt->second.find(funcName);
        if (funcIt != dllIt->second.end()) {
            return funcIt->second;
        }
    }
    return nullptr;
}

// Example Stubs
void stub_ExitProcess() {
    std::cout << "[ProWin] ExitProcess called!" << std::endl;
    exit(0);
}

void stub_GetModuleHandleA() {
    std::cout << "[ProWin] GetModuleHandleA called!" << std::endl;
}

void initDefaultStubs() {
    StubManager::getInstance().registerStub("kernel32.dll", "ExitProcess", (StubManager::StubFunction)stub_ExitProcess);
    StubManager::getInstance().registerStub("kernel32.dll", "GetModuleHandleA", (StubManager::StubFunction)stub_GetModuleHandleA);
}

}
