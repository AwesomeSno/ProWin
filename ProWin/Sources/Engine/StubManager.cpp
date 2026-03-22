#include "StubManager.h"
#include "CPUContext.h"
#include "InputState.h"
#include "HeapManager.h"
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <ctime>
#include <unistd.h>

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

uint64_t generic_null_stub(CPUContext& ctx) {
    printf("[StubManager] WARNING: Unimplemented function called at RIP=0x%llx\n", ctx.rip);
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

// ============================================================
// KERNEL32.DLL STUBS
// ============================================================

uint64_t stub_ExitProcess(CPUContext& ctx) {
    printf("[ProWin] ExitProcess called with code %llu\n", ctx.rcx);
    return 0; // Will be caught by engine event
}

uint64_t stub_GetModuleHandleA(CPUContext& ctx) {
    uint64_t lpModuleName = ctx.rcx;
    if (lpModuleName == 0) {
        // NULL = return base of current process
        return 0x00400000; // Default ImageBase
    }
    const char* name = (const char*)lpModuleName;
    printf("[ProWin] GetModuleHandleA(\"%s\")\n", name);
    return 0x10000000; // Fake handle
}

uint64_t stub_GetModuleHandleW(CPUContext& ctx) {
    if (ctx.rcx == 0) return 0x00400000;
    return 0x10000000;
}

uint64_t stub_GetProcAddress(CPUContext& ctx) {
    uint64_t hModule = ctx.rcx;
    const char* procName = (const char*)ctx.rdx;
    printf("[ProWin] GetProcAddress(0x%llx, \"%s\")\n", hModule, procName ? procName : "NULL");
    // Return a null stub address
    return (uint64_t)(void*)generic_null_stub;
}

uint64_t stub_LoadLibraryA(CPUContext& ctx) {
    const char* name = (const char*)ctx.rcx;
    printf("[ProWin] LoadLibraryA(\"%s\")\n", name ? name : "NULL");
    return 0x20000000; // Fake module handle
}

uint64_t stub_LoadLibraryW(CPUContext& ctx) {
    printf("[ProWin] LoadLibraryW called\n");
    return 0x20000000;
}

uint64_t stub_FreeLibrary(CPUContext& ctx) {
    return 1; // TRUE
}

uint64_t stub_GetProcessHeap(CPUContext& ctx) {
    return HeapManager::getInstance().getProcessHeap();
}

uint64_t stub_HeapAlloc(CPUContext& ctx) {
    uint64_t hHeap = ctx.rcx;
    uint32_t dwFlags = (uint32_t)ctx.rdx;
    uint64_t dwBytes = ctx.r8;
    return HeapManager::getInstance().heapAlloc(hHeap, dwFlags, dwBytes);
}

uint64_t stub_HeapFree(CPUContext& ctx) {
    uint64_t hHeap = ctx.rcx;
    uint32_t dwFlags = (uint32_t)ctx.rdx;
    uint64_t lpMem = ctx.r8;
    return HeapManager::getInstance().heapFree(hHeap, dwFlags, lpMem) ? 1 : 0;
}

uint64_t stub_VirtualAlloc(CPUContext& ctx) {
    return HeapManager::getInstance().virtualAlloc(ctx.rcx, ctx.rdx, (uint32_t)ctx.r8, (uint32_t)ctx.r9);
}

uint64_t stub_VirtualFree(CPUContext& ctx) {
    return HeapManager::getInstance().virtualFree(ctx.rcx, ctx.rdx, (uint32_t)ctx.r8) ? 1 : 0;
}

uint64_t stub_VirtualProtect(CPUContext& ctx) {
    // For now, always succeed
    return 1;
}

uint64_t stub_GetLastError(CPUContext& ctx) {
    return 0; // ERROR_SUCCESS
}

uint64_t stub_SetLastError(CPUContext& ctx) {
    return 0;
}

uint64_t stub_GetCurrentProcessId(CPUContext& ctx) {
    return 1337; // Fake PID
}

uint64_t stub_GetCurrentThreadId(CPUContext& ctx) {
    return 1; // Fake TID
}

uint64_t stub_GetCurrentProcess(CPUContext& ctx) {
    return 0xFFFFFFFFFFFFFFFFULL; // Pseudo-handle for current process
}

uint64_t stub_QueryPerformanceCounter(CPUContext& ctx) {
    uint64_t* ptr = (uint64_t*)ctx.rcx;
    if (ptr) {
        struct timespec ts;
        clock_gettime(CLOCK_MONOTONIC, &ts);
        *ptr = (uint64_t)ts.tv_sec * 1000000000ULL + (uint64_t)ts.tv_nsec;
    }
    return 1;
}

uint64_t stub_QueryPerformanceFrequency(CPUContext& ctx) {
    uint64_t* ptr = (uint64_t*)ctx.rcx;
    if (ptr) {
        *ptr = 1000000000ULL; // nanosecond precision
    }
    return 1;
}

uint64_t stub_GetTickCount(CPUContext& ctx) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)(ts.tv_sec * 1000 + ts.tv_nsec / 1000000);
}

uint64_t stub_GetTickCount64(CPUContext& ctx) {
    return stub_GetTickCount(ctx);
}

uint64_t stub_Sleep(CPUContext& ctx) {
    uint32_t ms = (uint32_t)ctx.rcx;
    if (ms > 0) {
        usleep(ms * 1000);
    }
    return 0;
}

uint64_t stub_GetSystemInfo(CPUContext& ctx) {
    // Fill in a minimal SYSTEM_INFO struct
    uint8_t* ptr = (uint8_t*)ctx.rcx;
    if (ptr) {
        memset(ptr, 0, 48);
        *(uint16_t*)(ptr + 0) = 9;  // wProcessorArchitecture = AMD64
        *(uint32_t*)(ptr + 4) = 4096; // dwPageSize
        *(uint32_t*)(ptr + 32) = 8;   // dwNumberOfProcessors
    }
    return 0;
}

uint64_t stub_GetStdHandle(CPUContext& ctx) {
    uint32_t nStdHandle = (uint32_t)ctx.rcx;
    switch (nStdHandle) {
        case 0xFFFFFFF6: return 1; // STD_INPUT
        case 0xFFFFFFF5: return 2; // STD_OUTPUT
        case 0xFFFFFFF4: return 3; // STD_ERROR
    }
    return 0;
}

uint64_t stub_WriteFile(CPUContext& ctx) {
    uint64_t hFile = ctx.rcx;
    const char* buf = (const char*)ctx.rdx;
    uint32_t nBytes = (uint32_t)ctx.r8;
    uint32_t* written = (uint32_t*)ctx.r9;
    
    if (hFile == 2 && buf && nBytes > 0) { // stdout
        fwrite(buf, 1, nBytes, stdout);
        fflush(stdout);
    }
    if (written) *written = nBytes;
    return 1;
}

uint64_t stub_CloseHandle(CPUContext& ctx) {
    return 1; // TRUE
}

uint64_t stub_CreateFileA(CPUContext& ctx) {
    const char* path = (const char*)ctx.rcx;
    printf("[ProWin] CreateFileA(\"%s\")\n", path ? path : "NULL");
    return 0xFFFFFFFFFFFFFFFFULL; // INVALID_HANDLE_VALUE (file not found)
}

uint64_t stub_GetCommandLineA(CPUContext& ctx) {
    static const char* cmdLine = "prowin.exe";
    return (uint64_t)cmdLine;
}

uint64_t stub_GetCommandLineW(CPUContext& ctx) {
    static const wchar_t cmdLine[] = L"prowin.exe";
    return (uint64_t)cmdLine;
}

uint64_t stub_InitializeCriticalSection(CPUContext& ctx) {
    return 0; // Success
}

uint64_t stub_EnterCriticalSection(CPUContext& ctx) {
    return 0;
}

uint64_t stub_LeaveCriticalSection(CPUContext& ctx) {
    return 0;
}

uint64_t stub_DeleteCriticalSection(CPUContext& ctx) {
    return 0;
}

// ============================================================
// USER32.DLL STUBS
// ============================================================

uint64_t stub_MessageBoxA(CPUContext& ctx) {
    const char* text = (const char*)ctx.rdx;
    const char* caption = (const char*)ctx.r8;
    printf("[ProWin] MessageBoxA: \"%s\" - \"%s\"\n",
           caption ? caption : "(null)", text ? text : "(null)");
    return 1; // IDOK
}

uint64_t stub_GetDC(CPUContext& ctx) {
    return 0xDC001;
}

uint64_t stub_ReleaseDC(CPUContext& ctx) {
    return 1;
}

uint64_t stub_CreateWindowExA(CPUContext& ctx) {
    printf("[ProWin] CreateWindowExA called\n");
    return 0x00D00001; // Fake HWND
}

uint64_t stub_ShowWindow(CPUContext& ctx) {
    return 1; // TRUE
}

uint64_t stub_UpdateWindow(CPUContext& ctx) {
    return 1;
}

uint64_t stub_PeekMessageA(CPUContext& ctx) {
    return 0; // No messages
}

uint64_t stub_DispatchMessageA(CPUContext& ctx) {
    return 0;
}

uint64_t stub_TranslateMessage(CPUContext& ctx) {
    return 0;
}

uint64_t stub_DefWindowProcA(CPUContext& ctx) {
    return 0;
}

uint64_t stub_RegisterClassExA(CPUContext& ctx) {
    return 1; // Success atom
}

uint64_t stub_GetSystemMetrics(CPUContext& ctx) {
    uint32_t index = (uint32_t)ctx.rcx;
    switch (index) {
        case 0: return 800;  // SM_CXSCREEN
        case 1: return 600;  // SM_CYSCREEN
        default: return 0;
    }
}

// ============================================================
// GDI32.DLL STUBS
// ============================================================

uint64_t stub_SelectObject(CPUContext& ctx) {
    return 0x000B0001; // Fake previous object
}

uint64_t stub_DeleteObject(CPUContext& ctx) {
    return 1;
}

// ============================================================
// DXGI / D3D STUBS (Gaming-critical)
// ============================================================

uint64_t stub_CreateDXGIFactory(CPUContext& ctx) {
    printf("[ProWin] DXGI: CreateDXGIFactory called\n");
    return 0; // S_OK
}

uint64_t stub_D3D11CreateDevice(CPUContext& ctx) {
    printf("[ProWin] D3D11: D3D11CreateDevice called\n");
    return 0; // S_OK
}

uint64_t stub_D3D11CreateDeviceAndSwapChain(CPUContext& ctx) {
    printf("[ProWin] D3D11: D3D11CreateDeviceAndSwapChain called\n");
    return 0; // S_OK
}

// ============================================================
// XINPUT STUBS (Gaming-critical)
// ============================================================

struct XINPUT_STATE {
    uint32_t dwPacketNumber;
    uint8_t  Gamepad[12];
};

uint64_t stub_XInputGetState(CPUContext& ctx) {
    uint32_t dwUserIndex = (uint32_t)ctx.rcx;
    uint64_t pStatePtr = ctx.rdx;

    if (pStatePtr != 0) {
        WinXInputState realState = InputStateManager::getInstance().getState(dwUserIndex);
        memcpy((void*)pStatePtr, &realState, sizeof(WinXInputState));
    }
    
    return 0; // ERROR_SUCCESS
}

uint64_t stub_XInputSetState(CPUContext& ctx) {
    return 0; // ERROR_SUCCESS (vibration — ignore for now)
}

uint64_t stub_XInputGetCapabilities(CPUContext& ctx) {
    uint32_t dwUserIndex = (uint32_t)ctx.rcx;
    if (dwUserIndex > 3) return 1167; // ERROR_DEVICE_NOT_CONNECTED
    
    uint8_t* caps = (uint8_t*)ctx.r8;
    if (caps) {
        memset(caps, 0, 20);
        *(uint8_t*)(caps + 0) = 1; // Type = GAMEPAD
        *(uint8_t*)(caps + 1) = 1; // SubType = GAMEPAD
    }
    return 0; // ERROR_SUCCESS
}

// ============================================================
// XAUDIO2 / DIRECTSOUND STUBS (Audio)
// ============================================================

uint64_t stub_XAudio2Create(CPUContext& ctx) {
    printf("[ProWin] XAudio2Create called — bridged to AVAudioEngine\n");
    return 0;
}

uint64_t stub_DirectSoundCreate(CPUContext& ctx) {
    printf("[ProWin] DirectSoundCreate called — bridged to AVAudioEngine\n");
    return 0;
}

// ============================================================
// NTDLL STUBS
// ============================================================

uint64_t stub_RtlInitializeCriticalSection(CPUContext& ctx) {
    return 0;
}

uint64_t stub_RtlEnterCriticalSection(CPUContext& ctx) {
    return 0;
}

uint64_t stub_RtlLeaveCriticalSection(CPUContext& ctx) {
    return 0;
}

uint64_t stub_NtQuerySystemInformation(CPUContext& ctx) {
    return 0; // STATUS_SUCCESS
}

// ============================================================
// REGISTRATION
// ============================================================

void initDefaultStubs() {
    StubManager& sm = StubManager::getInstance();
    
    // kernel32.dll
    sm.registerStub("kernel32.dll", "ExitProcess", stub_ExitProcess);
    sm.registerStub("kernel32.dll", "GetModuleHandleA", stub_GetModuleHandleA);
    sm.registerStub("kernel32.dll", "GetModuleHandleW", stub_GetModuleHandleW);
    sm.registerStub("kernel32.dll", "GetProcAddress", stub_GetProcAddress);
    sm.registerStub("kernel32.dll", "LoadLibraryA", stub_LoadLibraryA);
    sm.registerStub("kernel32.dll", "LoadLibraryW", stub_LoadLibraryW);
    sm.registerStub("kernel32.dll", "FreeLibrary", stub_FreeLibrary);
    sm.registerStub("kernel32.dll", "GetProcessHeap", stub_GetProcessHeap);
    sm.registerStub("kernel32.dll", "HeapAlloc", stub_HeapAlloc);
    sm.registerStub("kernel32.dll", "HeapFree", stub_HeapFree);
    sm.registerStub("kernel32.dll", "VirtualAlloc", stub_VirtualAlloc);
    sm.registerStub("kernel32.dll", "VirtualFree", stub_VirtualFree);
    sm.registerStub("kernel32.dll", "VirtualProtect", stub_VirtualProtect);
    sm.registerStub("kernel32.dll", "GetLastError", stub_GetLastError);
    sm.registerStub("kernel32.dll", "SetLastError", stub_SetLastError);
    sm.registerStub("kernel32.dll", "GetCurrentProcessId", stub_GetCurrentProcessId);
    sm.registerStub("kernel32.dll", "GetCurrentThreadId", stub_GetCurrentThreadId);
    sm.registerStub("kernel32.dll", "GetCurrentProcess", stub_GetCurrentProcess);
    sm.registerStub("kernel32.dll", "QueryPerformanceCounter", stub_QueryPerformanceCounter);
    sm.registerStub("kernel32.dll", "QueryPerformanceFrequency", stub_QueryPerformanceFrequency);
    sm.registerStub("kernel32.dll", "GetTickCount", stub_GetTickCount);
    sm.registerStub("kernel32.dll", "GetTickCount64", stub_GetTickCount64);
    sm.registerStub("kernel32.dll", "Sleep", stub_Sleep);
    sm.registerStub("kernel32.dll", "GetSystemInfo", stub_GetSystemInfo);
    sm.registerStub("kernel32.dll", "GetStdHandle", stub_GetStdHandle);
    sm.registerStub("kernel32.dll", "WriteFile", stub_WriteFile);
    sm.registerStub("kernel32.dll", "CloseHandle", stub_CloseHandle);
    sm.registerStub("kernel32.dll", "CreateFileA", stub_CreateFileA);
    sm.registerStub("kernel32.dll", "GetCommandLineA", stub_GetCommandLineA);
    sm.registerStub("kernel32.dll", "GetCommandLineW", stub_GetCommandLineW);
    sm.registerStub("kernel32.dll", "InitializeCriticalSection", stub_InitializeCriticalSection);
    sm.registerStub("kernel32.dll", "EnterCriticalSection", stub_EnterCriticalSection);
    sm.registerStub("kernel32.dll", "LeaveCriticalSection", stub_LeaveCriticalSection);
    sm.registerStub("kernel32.dll", "DeleteCriticalSection", stub_DeleteCriticalSection);
    
    // user32.dll
    sm.registerStub("user32.dll", "MessageBoxA", stub_MessageBoxA);
    sm.registerStub("user32.dll", "GetDC", stub_GetDC);
    sm.registerStub("user32.dll", "ReleaseDC", stub_ReleaseDC);
    sm.registerStub("user32.dll", "CreateWindowExA", stub_CreateWindowExA);
    sm.registerStub("user32.dll", "ShowWindow", stub_ShowWindow);
    sm.registerStub("user32.dll", "UpdateWindow", stub_UpdateWindow);
    sm.registerStub("user32.dll", "PeekMessageA", stub_PeekMessageA);
    sm.registerStub("user32.dll", "DispatchMessageA", stub_DispatchMessageA);
    sm.registerStub("user32.dll", "TranslateMessage", stub_TranslateMessage);
    sm.registerStub("user32.dll", "DefWindowProcA", stub_DefWindowProcA);
    sm.registerStub("user32.dll", "RegisterClassExA", stub_RegisterClassExA);
    sm.registerStub("user32.dll", "GetSystemMetrics", stub_GetSystemMetrics);
    
    // gdi32.dll
    sm.registerStub("gdi32.dll", "SelectObject", stub_SelectObject);
    sm.registerStub("gdi32.dll", "DeleteObject", stub_DeleteObject);
    
    // dxgi.dll
    sm.registerStub("dxgi.dll", "CreateDXGIFactory", stub_CreateDXGIFactory);
    
    // d3d11.dll
    sm.registerStub("d3d11.dll", "D3D11CreateDevice", stub_D3D11CreateDevice);
    sm.registerStub("d3d11.dll", "D3D11CreateDeviceAndSwapChain", stub_D3D11CreateDeviceAndSwapChain);
    
    // xinput1_4.dll
    sm.registerStub("xinput1_4.dll", "XInputGetState", stub_XInputGetState);
    sm.registerStub("xinput1_4.dll", "XInputSetState", stub_XInputSetState);
    sm.registerStub("xinput1_4.dll", "XInputGetCapabilities", stub_XInputGetCapabilities);
    
    // xaudio2_9.dll
    sm.registerStub("xaudio2_9.dll", "XAudio2Create", stub_XAudio2Create);
    
    // dsound.dll
    sm.registerStub("dsound.dll", "DirectSoundCreate", stub_DirectSoundCreate);
    
    // ntdll.dll
    sm.registerStub("ntdll.dll", "RtlInitializeCriticalSection", stub_RtlInitializeCriticalSection);
    sm.registerStub("ntdll.dll", "RtlEnterCriticalSection", stub_RtlEnterCriticalSection);
    sm.registerStub("ntdll.dll", "RtlLeaveCriticalSection", stub_RtlLeaveCriticalSection);
    sm.registerStub("ntdll.dll", "NtQuerySystemInformation", stub_NtQuerySystemInformation);
    
    printf("[ProWin] StubManager: Registered %d DLLs with production stubs\n", 7);
}

}
