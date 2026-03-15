#include "InstructionDecoder.h"

namespace ProWin {

Instruction InstructionDecoder::decode(const uint8_t* code) {
    Instruction inst;
    inst.opcode = Opcode::UNKNOWN;
    inst.length = 0;
    inst.reg1 = 0;
    inst.reg2 = 0;
    inst.imm = 0;
    inst.disp = 0;
    inst.hasImm = false;
    inst.hasDisp = false;
    inst.isReg2Reg = false;

    if (!code) return inst;

    uint8_t pos = 0;
    bool is64Bit = false;

    // Handle REX prefix (40-4F)
    if (code[pos] >= 0x40 && code[pos] <= 0x4F) {
        if (code[pos] & 0x08) is64Bit = true; // REX.W
        // Other REX bits (R, X, B) can be used for register extension
        pos++;
    }

    uint8_t opcode = code[pos];
    
    switch (opcode) {
        case 0x90: // NOP
            inst.opcode = Opcode::NOP;
            inst.length = pos + 1;
            break;

        case 0xB8: case 0xB9: case 0xBA: case 0xBB:
        case 0xBC: case 0xBD: case 0xBE: case 0xBF: // MOV reg, imm
            inst.opcode = Opcode::MOV;
            inst.reg1 = opcode - 0xB8;
            if (is64Bit) {
                // MOV RAX, imm64 (1 byte opcode + 8 bytes imm)
                inst.imm = *(uint64_t*)(code + pos + 1);
                inst.hasImm = true;
                inst.length = pos + 9;
            } else {
                // MOV EAX, imm32 (1 byte opcode + 4 bytes imm)
                inst.imm = *(uint32_t*)(code + pos + 1);
                inst.hasImm = true;
                inst.length = pos + 5;
            }
            break;

        case 0x01: // ADD reg/mem, reg
        case 0x03: // ADD reg, reg/mem
            if (code[pos + 1] >= 0xC0) { // Reg-to-Reg only for now
                inst.opcode = Opcode::ADD;
                inst.isReg2Reg = true;
                if (opcode == 0x01) { // ADD r/m, r
                    inst.reg1 = code[pos + 1] & 0x07; // dst
                    inst.reg2 = (code[pos + 1] >> 3) & 0x07; // src
                } else { // ADD r, r/m
                    inst.reg1 = (code[pos + 1] >> 3) & 0x07; // dst
                    inst.reg2 = code[pos + 1] & 0x07; // src
                }
                inst.length = pos + 2;
            }
            break;

        case 0xC3: // RET
            inst.opcode = Opcode::RET;
            inst.length = pos + 1;
            break;

        case 0x0F: // Two-byte opcodes
            if (code[pos + 1] == 0x05) { // SYSCALL
                inst.opcode = Opcode::SYSCALL;
                inst.length = pos + 2;
            }
            break;

        default:
            inst.opcode = Opcode::UNKNOWN;
            inst.length = pos + 1;
            break;
    }

    return inst;
}

}
