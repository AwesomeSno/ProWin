#include "StubManager.h"
#include "CPUContext.h"
#include <iostream>
#include <cstring>

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

uint64_t generic_null_stub(CPUContext& ctx) {
    printf("[StubManager] ERROR: Unimplemented function called!\n");
    return 0;
}

void* StubManager::getOrCreateStub(const std::string& dllName, const std::string& funcName) {
    void* stub = getStub(dllName, funcName);
    if (stub) return stub;

    printf("[StubManager] WARNING: No stub for %s!%s — using null stub\n", dllName.c_str(), funcName.c_str());
    
    return (void*)generic_null_stub;
}

uint64_t StubManager::callStub(const std::string& dllName, const std::string& funcName, CPUContext& ctx) {
    StubFunction stub = (StubFunction)getStub(dllName, funcName);
    if (!stub) {
        stub = (StubFunction)getOrCreateStub(dllName, funcName);
    }
    uint64_t result = stub(ctx);
    ctx.rax = result;
    return result;
}

// Example Stubs
uint64_t stub_ExitProcess(CPUContext& ctx) {
    std::cout << "[ProWin] ExitProcess called!" << std::endl;
    exit(0);
    return 0;
}

uint64_t stub_GetModuleHandleA(CPUContext& ctx) {
    uint64_t lpModuleName = ctx.rcx;
    std::cout << "[ProWin] GetModuleHandleA called! ModuleName pointer: 0x" << std::hex << lpModuleName << std::dec << std::endl;
    return 0x12340000; // Fake handle
}

// Graphics Stubs
uint64_t stub_GDI_GetDC(CPUContext& ctx) {
    uint64_t hWnd = ctx.rcx;
    std::cout << "[ProWin] GDI: GetDC called for hWnd: 0x" << std::hex << hWnd << std::dec << std::endl;
    return 0xDC001; // Fake DC
}

uint64_t stub_DXGI_CreateFactory(CPUContext& ctx) {
    std::cout << "[ProWin] DXGI: CreateDXGIFactory called" << std::endl;
    return 0; // S_OK
}

// Input Stubs
struct XINPUT_STATE {
    uint32_t dwPacketNumber;
    uint8_t  Gamepad[12]; // Placeholder size
};

uint64_t stub_XInputGetState(CPUContext& ctx) {
    uint32_t dwUserIndex = (uint32_t)ctx.rcx;
    uint64_t pStatePtr = ctx.rdx;

    printf("[StubManager] XInputGetState called for user %u\n", dwUserIndex);

    if (pStatePtr != 0) {
        memset((void*)pStatePtr, 0, sizeof(XINPUT_STATE));
    }
    
    return 0; // ERROR_SUCCESS
}

// Audio Stubs
uint64_t stub_XAudio2Create(CPUContext& ctx) {
    std::cout << "[ProWin] XAudio2: XAudio2Create called" << std::endl;
    return 0; // S_OK
}

uint64_t stub_DirectSoundCreate(CPUContext& ctx) {
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
