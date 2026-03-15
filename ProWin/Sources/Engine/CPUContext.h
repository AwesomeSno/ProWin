#ifndef CPUContext_h
#define CPUContext_h

#include <cstdint>

namespace ProWin {

/// CPUContext represents the full register state of an x64 processor.
/// This is used to track the emulated state as we translate/execute x64 instructions.
struct CPUContext {
    // General Purpose Registers (64-bit)
    uint64_t rax, rbx, rcx, rdx;
    uint64_t rsi, rdi, rbp, rsp;
    uint64_t r8,  r9,  r10, r11;
    uint64_t r12, r13, r14, r15;

    // Instruction Pointer
    uint64_t rip;

    // RFLAGS Register
    uint64_t rflags;

    // Segment Registers (Selectors)
    uint16_t cs, ds, es, fs, gs, ss;

    // SIMD / Floating Point (Simplified for now - 128-bit XMM registers)
    struct {
        uint64_t low;
        uint64_t high;
    } xmm[16];

    CPUContext() {
        // Initialize everything to zero
        rax = rbx = rcx = rdx = 0;
        rsi = rdi = rbp = rsp = 0;
        r8 = r9 = r10 = r11 = 0;
        r12 = r13 = r14 = r15 = 0;
        rip = 0;
        rflags = 0x202; // Initial IF (Interrupt Flag) usually set
        cs = ds = es = fs = gs = ss = 0;
        for(int i=0; i<16; ++i) {
            xmm[i].low = 0;
            xmm[i].high = 0;
        }
    }
};

}

#endif /* CPUContext_h */
