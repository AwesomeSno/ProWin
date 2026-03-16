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

    uint8_t rex = 0;
    // Handle REX prefix (40-4F)
    if (code[pos] >= 0x40 && code[pos] <= 0x4F) {
        rex = code[pos];
        if (rex & 0x08) is64Bit = true; // REX.W
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
                // MOV Rxx, imm64
                inst.imm = *(uint64_t*)(code + pos + 1);
                inst.hasImm = true;
                inst.length = pos + 9;
            } else {
                // MOV Exx, imm32
                inst.imm = *(uint32_t*)(code + pos + 1);
                inst.hasImm = true;
                inst.length = pos + 5;
            }
            break;

        case 0x50: case 0x51: case 0x52: case 0x53:
        case 0x54: case 0x55: case 0x56: case 0x57: // PUSH reg64
            inst.opcode = Opcode::PUSH;
            inst.reg1 = (opcode & 0x07) | ((rex & 0x01) << 3); // rex.b
            inst.length = pos + 1;
            break;

        case 0x58: case 0x59: case 0x5A: case 0x5B:
        case 0x5C: case 0x5D: case 0x5E: case 0x5F: // POP reg64
            inst.opcode = Opcode::POP;
            inst.reg1 = (opcode & 0x07) | ((rex & 0x01) << 3); // rex.b
            inst.length = pos + 1;
            break;

        case 0xC7: // MOV r/m, imm32
            if (code[pos + 1] >= 0xC0) { // Reg only
                inst.opcode = Opcode::MOV;
                inst.reg1 = code[pos + 1] & 0x07;
                inst.imm = *(uint32_t*)(code + pos + 2);
                inst.hasImm = true;
                inst.length = pos + 6;
            }
            break;

        case 0x81: // Group 1 (ADD, ADC, AND, etc.)
            if ((code[pos + 1] & 0x38) == 0x00) { // ADD
                inst.opcode = Opcode::ADD;
                inst.reg1 = code[pos + 1] & 0x07;
                inst.imm = *(uint32_t*)(code + pos + 2);
                inst.hasImm = true;
                inst.length = pos + 6;
            }
            break;

        case 0xFF: // Group 4/5 (DEC, INC, PUSH, CALL, etc.)
            {
                uint8_t modrm = code[pos + 1];
                uint8_t reg = (modrm >> 3) & 0x07;
                if (reg == 0x01) { // DEC /1
                    inst.opcode = Opcode::DEC;
                    inst.reg1 = modrm & 0x07;
                    inst.length = pos + 2;
                } else if (reg == 0x00) { // INC /0
                    inst.opcode = Opcode::INC;
                    inst.reg1 = modrm & 0x07;
                    inst.length = pos + 2;
                } else if (reg == 0x06) { // PUSH r/m64 /6
                    inst.opcode = Opcode::PUSH;
                    inst.reg1 = modrm & 0x07; // Destination for push logic in dispatcher
                    inst.length = pos + 2;
                } else if (reg == 0x02) { // CALL r/m64 /2
                    inst.opcode = Opcode::CALL;
                    inst.reg1 = modrm & 0x07;
                    inst.isReg2Reg = (modrm >= 0xC0);
                    inst.length = pos + 2;
                }
            }
            break;

        case 0xAB: // STOSD
            inst.opcode = Opcode::STOSD;
            inst.length = pos + 1;
            break;

        case 0x74: // JZ/JE rel8
            inst.opcode = Opcode::JZ;
            inst.disp = (int8_t)code[pos + 1];
            inst.hasDisp = true;
            inst.length = pos + 2;
            break;

        case 0x75: // JNZ/JNE rel8
            inst.opcode = Opcode::JNZ;
            inst.disp = (int8_t)code[pos + 1];
            inst.hasDisp = true;
            inst.length = pos + 2;
            break;

        case 0x7C: // JL rel8
            inst.opcode = Opcode::JL;
            inst.disp = (int8_t)code[pos + 1];
            inst.hasDisp = true;
            inst.length = pos + 2;
            break;

        case 0x7D: // JGE rel8
            inst.opcode = Opcode::JGE;
            inst.disp = (int8_t)code[pos + 1];
            inst.hasDisp = true;
            inst.length = pos + 2;
            break;

        case 0x7E: // JLE rel8
            inst.opcode = Opcode::JLE;
            inst.disp = (int8_t)code[pos + 1];
            inst.hasDisp = true;
            inst.length = pos + 2;
            break;

        case 0x7F: // JG rel8
            inst.opcode = Opcode::JG;
            inst.disp = (int8_t)code[pos + 1];
            inst.hasDisp = true;
            inst.length = pos + 2;
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

        case 0xC2: // RET imm16
            inst.opcode = Opcode::RET;
            inst.imm = *(uint16_t*)(code + pos + 1);
            inst.hasImm = true;
            inst.length = pos + 3;
            break;

        case 0xE8: // CALL rel32
            inst.opcode = Opcode::CALL;
            inst.imm = *(int32_t*)(code + pos + 1);
            inst.hasImm = true; 
            inst.length = pos + 5;
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
