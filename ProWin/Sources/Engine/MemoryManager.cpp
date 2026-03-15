#include "MemoryManager.h"
#include <unistd.h>
#include <iostream>

namespace ProWin {

void* MemoryManager::reserve(uint64_t preferredAddress, size_t size) {
    // Round size to page size
    size_t pageSize = getpagesize();
    size = (size + pageSize - 1) & ~(pageSize - 1);
    
    // Attempt to map at the preferred address first
    void* addr = mmap((void*)preferredAddress, size, PROT_NONE, MAP_PRIVATE | MAP_ANON, -1, 0);
    
    if (addr == MAP_FAILED) {
        // Fallback: let the OS choose any available space
        addr = mmap(NULL, size, PROT_NONE, MAP_PRIVATE | MAP_ANON, -1, 0);
    }
    
    return addr == MAP_FAILED ? nullptr : addr;
}

bool MemoryManager::commit(void* address, size_t size, int prot) {
    // On macOS/POSIX, mmap already "commits" the virtual address range.
    // However, we can use mprotect to set the desired permissions.
    return mprotect(address, size, prot) == 0;
}

bool MemoryManager::protect(void* address, size_t size, int prot) {
    return mprotect(address, size, prot) == 0;
}

void MemoryManager::release(void* address, size_t size) {
    munmap(address, size);
}

}
