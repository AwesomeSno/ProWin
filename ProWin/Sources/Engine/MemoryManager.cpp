#include "MemoryManager.h"
#include <unistd.h>

namespace ProWin {

void* MemoryManager::reserve(uint64_t preferredAddress, size_t size, bool shared) {
    // Round size to page size
    size_t pageSize = getpagesize();
    size = (size + pageSize - 1) & ~(pageSize - 1);
    
    int flags = (shared ? MAP_SHARED : MAP_PRIVATE) | MAP_ANON;
    
    // Attempt to map at the preferred address first
    void* addr = mmap((void*)preferredAddress, size, PROT_NONE, flags, -1, 0);
    
    if (addr == MAP_FAILED) {
        // Fallback: let the OS choose any available space
        addr = mmap(NULL, size, PROT_NONE, flags, -1, 0);
    }
    
    return addr == MAP_FAILED ? nullptr : addr;
}

bool MemoryManager::commit(void* address, size_t size, int prot) {
    size_t pageSize = getpagesize();
    uintptr_t start = (uintptr_t)address;
    uintptr_t alignedStart = start & ~(pageSize - 1);
    size_t alignedSize = (start + size - alignedStart + pageSize - 1) & ~(pageSize - 1);
    
    if (mprotect((void*)alignedStart, alignedSize, prot) != 0) {
        perror("[ProWin] MemoryManager: mprotect failed");
        return false;
    }
    return true;
}

bool MemoryManager::protect(void* address, size_t size, int prot) {
    return mprotect(address, size, prot) == 0;
}

void MemoryManager::release(void* address, size_t size) {
    munmap(address, size);
}

}
