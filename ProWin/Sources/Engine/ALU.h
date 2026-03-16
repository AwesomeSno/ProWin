#pragma once

#include "CPUContext.h"
#include <cstdint>

namespace ProWin {

/**
 * Updates RFLAGS (CF, ZF, SF, OF) based on a 64-bit arithmetic operation.
 * 
 * Bits in RFLAGS:
 * CF (Carry)    : Bit 0
 * ZF (Zero)     : Bit 6
 * SF (Sign)     : Bit 7
 * OF (Overflow) : Bit 11
 */
inline void updateFlags64(CPUContext& ctx, uint64_t result, uint64_t a, uint64_t b, bool isSub) {
    // Zero Flag
    bool zf = (result == 0);
    
    // Sign Flag (bit 63)
    bool sf = (result >> 63) & 1;
    
    // Carry Flag (unsigned overflow)
    bool cf = isSub ? (a < b) : (result < a);
    
    // Overflow Flag (signed overflow)
    bool of = isSub
      ? (((a ^ b) & (a ^ result)) >> 63) & 1
      : (((~(a ^ b)) & (a ^ result)) >> 63) & 1;

    // Write flags into RFLAGS (bits: CF=0, ZF=6, SF=7, OF=11)
    ctx.rflags = (ctx.rflags & ~0xED1ULL)
               | (cf ? 1 : 0)
               | (zf ? (1 << 6) : 0)
               | (sf ? (1 << 7) : 0)
               | (of ? (1 << 11) : 0);
}

} // namespace ProWin
