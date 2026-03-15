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

// Graphics Stubs
void stub_GDI_GetDC() {
    std::cout << "[ProWin] GDI: GetDC called" << std::endl;
}

void stub_DXGI_CreateFactory() {
    std::cout << "[ProWin] DXGI: CreateDXGIFactory called" << std::endl;
}

// Input Stubs
uint32_t stub_XInputGetState(uint32_t dwUserIndex, void* pState) {
    // Return ERROR_SUCCESS (0) if controller is supposedly connected
    if (dwUserIndex == 0) return 0;
    return 1167; // ERROR_DEVICE_NOT_CONNECTED
}

void initDefaultStubs() {
    StubManager& sm = StubManager::getInstance();
    sm.registerStub("kernel32.dll", "ExitProcess", (StubManager::StubFunction)stub_ExitProcess);
    sm.registerStub("kernel32.dll", "GetModuleHandleA", (StubManager::StubFunction)stub_GetModuleHandleA);
    
    // Graphics
    sm.registerStub("user32.dll", "GetDC", (StubManager::StubFunction)stub_GDI_GetDC);
    sm.registerStub("dxgi.dll", "CreateDXGIFactory", (StubManager::StubFunction)stub_DXGI_CreateFactory);
    
    // Input
    sm.registerStub("xinput1_4.dll", "XInputGetState", (StubManager::StubFunction)stub_XInputGetState);
}

}
