#ifndef MemoryManager_h
#define MemoryManager_h

#include <cstdint>
#include <vector>
#include <sys/mman.h>

namespace ProWin {

class MemoryManager {
public:
    struct MemoryRegion {
        uint64_t address;
        size_t size;
        uint32_t protection; // PROT_READ, etc.
    };

    static void* reserve(uint64_t preferredAddress, size_t size);
    static bool commit(void* address, size_t size, int prot);
    static bool protect(void* address, size_t size, int prot);
    static void release(void* address, size_t size);
};

}

#endif /* MemoryManager_h */
