#ifndef WinEnvironment_h
#define WinEnvironment_h

#include <cstdint>
#include <cstring>
#include <sys/mman.h>

namespace ProWin {

/**
 * WinEnvironment sets up the minimal Windows process environment
 * that guest code expects: PEB, TEB, and GS segment base.
 *
 * Layout (simplified):
 *   TEB at GS base (GS:[0x00] = self-pointer, GS:[0x60] = PEB pointer)
 *   PEB contains ProcessParameters, ImageBaseAddress, etc.
 */
class WinEnvironment {
public:
    static WinEnvironment& getInstance() {
        static WinEnvironment instance;
        return instance;
    }

    // Allocates and initializes TEB + PEB. Must be called before engine starts.
    bool initialize(uint64_t imageBase);

    // Returns the base address of the TEB (for GS segment setup)
    uint64_t getTEBBase() const { return m_tebBase; }

    // Returns PEB address
    uint64_t getPEBBase() const { return m_pebBase; }

    // Read from GS-relative offset (emulates GS:[offset] access)
    uint64_t readGS(uint32_t offset) const;

private:
    WinEnvironment() : m_tebBase(0), m_pebBase(0), m_tebMemory(nullptr), m_pebMemory(nullptr) {}
    
    uint64_t m_tebBase;
    uint64_t m_pebBase;
    uint8_t* m_tebMemory;
    uint8_t* m_pebMemory;

    static constexpr size_t TEB_SIZE = 0x1000; // 4KB
    static constexpr size_t PEB_SIZE = 0x1000; // 4KB
};

} // namespace ProWin

#endif /* WinEnvironment_h */
