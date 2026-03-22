#ifndef HeapManager_h
#define HeapManager_h

#include <cstdint>
#include <unordered_map>
#include <mutex>
#include <sys/mman.h>

namespace ProWin {

/**
 * HeapManager provides a simple memory heap for guest Windows applications.
 * Backs GetProcessHeap, HeapAlloc, HeapFree, VirtualAlloc, VirtualFree.
 */
class HeapManager {
public:
    static HeapManager& getInstance() {
        static HeapManager instance;
        return instance;
    }

    // Returns a fake HANDLE value for GetProcessHeap()
    uint64_t getProcessHeap() const { return m_defaultHeapHandle; }

    // HeapAlloc: allocates `size` bytes, returns guest-visible address
    uint64_t heapAlloc(uint64_t heapHandle, uint32_t flags, uint64_t size);

    // HeapFree: frees a previously allocated block
    bool heapFree(uint64_t heapHandle, uint32_t flags, uint64_t ptr);

    // VirtualAlloc: reserve/commit pages
    uint64_t virtualAlloc(uint64_t address, uint64_t size, uint32_t allocType, uint32_t protect);

    // VirtualFree: release pages
    bool virtualFree(uint64_t address, uint64_t size, uint32_t freeType);

    // Total bytes allocated (for diagnostics)
    uint64_t totalAllocated() const { return m_totalAllocated; }

private:
    HeapManager();

    struct Allocation {
        void* hostPtr;
        uint64_t size;
    };

    std::mutex m_mutex;
    std::unordered_map<uint64_t, Allocation> m_allocations;
    uint64_t m_totalAllocated;
    static constexpr uint64_t m_defaultHeapHandle = 0xAE000001ULL;
};

} // namespace ProWin

#endif /* HeapManager_h */
