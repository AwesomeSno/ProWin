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

void null_stub_handler(const std::string& dll, const std::string& func) {
    printf("[StubManager] FATAL: Call to unimplemented stub %s!%s\n", dll.c_str(), func.c_str());
    // In a real engine we might throw or halt. For now, just log.
}

void generic_null_stub() {
    printf("[StubManager] ERROR: Unimplemented function called!\n");
}

void* StubManager::getOrCreateStub(const std::string& dllName, const std::string& funcName) {
    void* stub = getStub(dllName, funcName);
    if (stub) return stub;

    printf("[StubManager] WARNING: No stub for %s!%s — using null stub\n", dllName.c_str(), funcName.c_str());
    
    return (void*)generic_null_stub;
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

// Audio Stubs
uint32_t stub_XAudio2Create(void** ppXAudio2, uint32_t Flags, uint32_t XAudio2Processor) {
    std::cout << "[ProWin] XAudio2: XAudio2Create called" << std::endl;
    return 0; // S_OK
}

uint32_t stub_DirectSoundCreate(const void* pcGuidDevice, void** ppDS, void* pUnkOuter) {
    std::cout << "[ProWin] DirectSound: DirectSoundCreate called" << std::endl;
    return 0; // DS_OK
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

    // Audio
    sm.registerStub("xaudio2_9.dll", "XAudio2Create", (StubManager::StubFunction)stub_XAudio2Create);
    sm.registerStub("dsound.dll", "DirectSoundCreate", (StubManager::StubFunction)stub_DirectSoundCreate);
}

}
