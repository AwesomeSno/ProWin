#include "HeapManager.h"
#include <cstdio>
#include <cstring>
#include <unistd.h>

namespace ProWin {

HeapManager::HeapManager() : m_totalAllocated(0) {}

uint64_t HeapManager::heapAlloc(uint64_t heapHandle, uint32_t flags, uint64_t size) {
    if (size == 0) size = 1; // Windows allows zero-size allocs
    
    std::lock_guard<std::mutex> lock(m_mutex);
    
    // Page-align for mmap
    size_t pageSize = getpagesize();
    size_t allocSize = (size + pageSize - 1) & ~(pageSize - 1);
    
    void* ptr = mmap(NULL, allocSize, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON, -1, 0);
    if (ptr == MAP_FAILED) {
        printf("[ProWin] HeapManager: HeapAlloc failed for %llu bytes\n", (unsigned long long)size);
        return 0;
    }
    
    // HEAP_ZERO_MEMORY (0x08)
    if (flags & 0x08) {
        memset(ptr, 0, allocSize);
    }
    
    uint64_t addr = (uint64_t)ptr;
    m_allocations[addr] = {ptr, allocSize};
    m_totalAllocated += allocSize;
    
    return addr;
}

bool HeapManager::heapFree(uint64_t heapHandle, uint32_t flags, uint64_t ptr) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    auto it = m_allocations.find(ptr);
    if (it == m_allocations.end()) {
        return false;
    }
    
    munmap(it->second.hostPtr, it->second.size);
    m_totalAllocated -= it->second.size;
    m_allocations.erase(it);
    return true;
}

uint64_t HeapManager::virtualAlloc(uint64_t address, uint64_t size, uint32_t allocType, uint32_t protect) {
    if (size == 0) return 0;
    
    std::lock_guard<std::mutex> lock(m_mutex);
    
    size_t pageSize = getpagesize();
    size_t allocSize = (size + pageSize - 1) & ~(pageSize - 1);
    
    int prot = PROT_READ | PROT_WRITE;
    if (protect & 0x10) prot |= PROT_EXEC; // PAGE_EXECUTE
    
    void* hint = address ? (void*)address : NULL;
    int flags = MAP_PRIVATE | MAP_ANON;
    
    void* ptr = mmap(hint, allocSize, prot, flags, -1, 0);
    if (ptr == MAP_FAILED) {
        printf("[ProWin] HeapManager: VirtualAlloc failed for %llu bytes\n", (unsigned long long)size);
        return 0;
    }
    
    uint64_t addr = (uint64_t)ptr;
    m_allocations[addr] = {ptr, allocSize};
    m_totalAllocated += allocSize;
    
    return addr;
}

bool HeapManager::virtualFree(uint64_t address, uint64_t size, uint32_t freeType) {
    // MEM_RELEASE (0x8000)
    if (freeType & 0x8000) {
        return heapFree(0, 0, address);
    }
    return false;
}

} // namespace ProWin
