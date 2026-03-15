#include "InstructionDecoder.h"

namespace ProWin {

Instruction InstructionDecoder::decode(const uint8_t* code) {
    Instruction inst;
    inst.opcode = Opcode::UNKNOWN;
    inst.length = 0;

    if (!code) return inst;

    uint8_t b = code[0];

    // Very basic decoding for common x64 opcodes (Simplified subset)
    switch (b) {
        case 0x90: // NOP
            inst.opcode = Opcode::NOP;
            inst.length = 1;
            break;
        case 0x50: case 0x51: case 0x52: case 0x53:
        case 0x54: case 0x55: case 0x56: case 0x57: // PUSH reg
            inst.opcode = Opcode::PUSH;
            inst.length = 1;
            break;
        case 0x58: case 0x59: case 0x5A: case 0x5B:
        case 0x5C: case 0x5D: case 0x5E: case 0x5F: // POP reg
            inst.opcode = Opcode::POP;
            inst.length = 1;
            break;
        case 0xC3: // RET
            inst.opcode = Opcode::RET;
            inst.length = 1;
            break;
        case 0xE8: // CALL relative
            inst.opcode = Opcode::CALL;
            inst.length = 5;
            break;
        case 0x0F: // Two-byte opcodes
            if (code[1] == 0x05) { // SYSCALL
                inst.opcode = Opcode::SYSCALL;
                inst.length = 2;
            }
            break;
        default:
            // Fallback for complex opcodes (REX prefix, ModR/M etc. - to be implemented)
            inst.opcode = Opcode::UNKNOWN;
            inst.length = 1; 
            break;
    }

    return inst;
}

}
