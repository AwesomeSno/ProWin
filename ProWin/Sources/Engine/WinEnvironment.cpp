#include "WinEnvironment.h"
#include <cstdio>
#include <unistd.h>

namespace ProWin {

bool WinEnvironment::initialize(uint64_t imageBase) {
    // Allocate TEB
    m_tebMemory = (uint8_t*)mmap(NULL, TEB_SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON, -1, 0);
    if (m_tebMemory == MAP_FAILED) {
        printf("[ProWin] WinEnvironment: Failed to allocate TEB\n");
        return false;
    }
    memset(m_tebMemory, 0, TEB_SIZE);
    m_tebBase = (uint64_t)m_tebMemory;

    // Allocate PEB
    m_pebMemory = (uint8_t*)mmap(NULL, PEB_SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON, -1, 0);
    if (m_pebMemory == MAP_FAILED) {
        printf("[ProWin] WinEnvironment: Failed to allocate PEB\n");
        return false;
    }
    memset(m_pebMemory, 0, PEB_SIZE);
    m_pebBase = (uint64_t)m_pebMemory;

    // ===== Initialize TEB =====
    // TEB+0x00: SEH chain (NULL = end of chain)
    *(uint64_t*)(m_tebMemory + 0x00) = 0xFFFFFFFFFFFFFFFFULL;
    
    // TEB+0x08: Stack Base (top of stack)
    *(uint64_t*)(m_tebMemory + 0x08) = 0x7FFFFFFFE000ULL;
    
    // TEB+0x10: Stack Limit (bottom of stack)
    *(uint64_t*)(m_tebMemory + 0x10) = 0x7FFFFFFF0000ULL;
    
    // TEB+0x30: Self-pointer (GS:[0x30] points back to TEB)
    *(uint64_t*)(m_tebMemory + 0x30) = m_tebBase;
    
    // TEB+0x40: ProcessId
    *(uint32_t*)(m_tebMemory + 0x40) = 1337;
    
    // TEB+0x48: ThreadId
    *(uint32_t*)(m_tebMemory + 0x48) = 1;
    
    // TEB+0x60: PEB pointer (GS:[0x60] = PEB)
    *(uint64_t*)(m_tebMemory + 0x60) = m_pebBase;

    // ===== Initialize PEB =====
    // PEB+0x02: BeingDebugged = 0 (not debugged)
    m_pebMemory[0x02] = 0;
    
    // PEB+0x10: ImageBaseAddress
    *(uint64_t*)(m_pebMemory + 0x10) = imageBase;
    
    // PEB+0x18: Ldr (loader data — NULL for now)
    *(uint64_t*)(m_pebMemory + 0x18) = 0;
    
    // PEB+0x20: ProcessParameters (NULL for now)
    *(uint64_t*)(m_pebMemory + 0x20) = 0;
    
    // PEB+0xBC: NumberOfProcessors
    *(uint32_t*)(m_pebMemory + 0xBC) = 8;

    printf("[ProWin] WinEnvironment: TEB at 0x%llx, PEB at 0x%llx\n", m_tebBase, m_pebBase);
    return true;
}

uint64_t WinEnvironment::readGS(uint32_t offset) const {
    if (m_tebMemory == nullptr) return 0;
    if (offset + 8 > TEB_SIZE) return 0;
    return *(uint64_t*)(m_tebMemory + offset);
}

} // namespace ProWin
