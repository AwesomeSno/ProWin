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

    uint8_t rexB = (rex & 0x01);       // extends ModRM r/m or opcode reg
    uint8_t rexR = (rex & 0x04) >> 2;  // extends ModRM reg field
    uint8_t opcode = code[pos];
    
    switch (opcode) {
        case 0x90: // NOP
            inst.opcode = Opcode::NOP;
            inst.length = pos + 1;
            break;

        // ===== MOV reg, imm =====
        case 0xB8: case 0xB9: case 0xBA: case 0xBB:
        case 0xBC: case 0xBD: case 0xBE: case 0xBF:
            inst.opcode = Opcode::MOV;
            inst.reg1 = (opcode - 0xB8) | (rexB << 3);
            if (is64Bit) {
                inst.imm = *(uint64_t*)(code + pos + 1);
                inst.hasImm = true;
                inst.length = pos + 9;
            } else {
                inst.imm = *(uint32_t*)(code + pos + 1);
                inst.hasImm = true;
                inst.length = pos + 5;
            }
            break;

        // ===== MOV r8, imm8 =====
        case 0xB0: case 0xB1: case 0xB2: case 0xB3:
        case 0xB4: case 0xB5: case 0xB6: case 0xB7:
            inst.opcode = Opcode::MOV;
            inst.reg1 = (opcode - 0xB0) | (rexB << 3);
            inst.imm = code[pos + 1];
            inst.hasImm = true;
            inst.length = pos + 2;
            break;

        // ===== PUSH / POP reg =====
        case 0x50: case 0x51: case 0x52: case 0x53:
        case 0x54: case 0x55: case 0x56: case 0x57:
            inst.opcode = Opcode::PUSH;
            inst.reg1 = (opcode & 0x07) | (rexB << 3);
            inst.length = pos + 1;
            break;

        case 0x58: case 0x59: case 0x5A: case 0x5B:
        case 0x5C: case 0x5D: case 0x5E: case 0x5F:
            inst.opcode = Opcode::POP;
            inst.reg1 = (opcode & 0x07) | (rexB << 3);
            inst.length = pos + 1;
            break;

        // ===== MOV r/m, imm32 =====
        case 0xC7:
            if (code[pos + 1] >= 0xC0) {
                inst.opcode = Opcode::MOV;
                inst.reg1 = (code[pos + 1] & 0x07) | (rexB << 3);
                inst.imm = *(uint32_t*)(code + pos + 2);
                inst.hasImm = true;
                inst.length = pos + 6;
            }
            break;

        // ===== Group 1 r/m, imm32 (ADD, OR, ADC, SBB, AND, SUB, XOR, CMP) =====
        case 0x81:
            {
                uint8_t modrm = code[pos + 1];
                uint8_t regOp = (modrm >> 3) & 0x07;
                inst.reg1 = (modrm & 0x07) | (rexB << 3);
                inst.imm = *(uint32_t*)(code + pos + 2);
                inst.hasImm = true;
                inst.length = pos + 6;
                switch (regOp) {
                    case 0: inst.opcode = Opcode::ADD; break;
                    case 1: inst.opcode = Opcode::OR;  break;
                    case 4: inst.opcode = Opcode::AND; break;
                    case 5: inst.opcode = Opcode::SUB; break;
                    case 6: inst.opcode = Opcode::XOR; break;
                    case 7: inst.opcode = Opcode::CMP; break;
                    default: inst.opcode = Opcode::UNKNOWN; break;
                }
            }
            break;

        // ===== Group 1 r/m, imm8 sign-extended =====
        case 0x83:
            {
                uint8_t modrm = code[pos + 1];
                uint8_t regOp = (modrm >> 3) & 0x07;
                inst.reg1 = (modrm & 0x07) | (rexB << 3);
                inst.imm = (uint64_t)(int64_t)(int8_t)code[pos + 2]; // sign-extend
                inst.hasImm = true;
                inst.length = pos + 3;
                switch (regOp) {
                    case 0: inst.opcode = Opcode::ADD; break;
                    case 1: inst.opcode = Opcode::OR;  break;
                    case 4: inst.opcode = Opcode::AND; break;
                    case 5: inst.opcode = Opcode::SUB; break;
                    case 6: inst.opcode = Opcode::XOR; break;
                    case 7: inst.opcode = Opcode::CMP; break;
                    default: inst.opcode = Opcode::UNKNOWN; break;
                }
            }
            break;

        // ===== FF Group (INC, DEC, CALL, PUSH) =====
        case 0xFF:
            {
                uint8_t modrm = code[pos + 1];
                uint8_t reg = (modrm >> 3) & 0x07;
                inst.reg1 = (modrm & 0x07) | (rexB << 3);
                inst.length = pos + 2;
                if (reg == 0x00) inst.opcode = Opcode::INC;
                else if (reg == 0x01) inst.opcode = Opcode::DEC;
                else if (reg == 0x02) { inst.opcode = Opcode::CALL; inst.isReg2Reg = (modrm >= 0xC0); }
                else if (reg == 0x04) { inst.opcode = Opcode::JMP; inst.isReg2Reg = (modrm >= 0xC0); }
                else if (reg == 0x06) inst.opcode = Opcode::PUSH;
            }
            break;

        // ===== F7 Group (TEST, NOT, NEG, MUL, IMUL, DIV) =====
        case 0xF7:
            {
                uint8_t modrm = code[pos + 1];
                uint8_t reg = (modrm >> 3) & 0x07;
                inst.reg1 = (modrm & 0x07) | (rexB << 3);
                if (reg == 0x00) { // TEST r/m, imm32
                    inst.opcode = Opcode::TEST;
                    inst.imm = *(uint32_t*)(code + pos + 2);
                    inst.hasImm = true;
                    inst.length = pos + 6;
                } else if (reg == 0x02) { // NOT r/m
                    inst.opcode = Opcode::NOT;
                    inst.length = pos + 2;
                } else if (reg == 0x03) { // NEG r/m
                    inst.opcode = Opcode::NEG;
                    inst.length = pos + 2;
                } else if (reg == 0x05) { // IMUL r/m (one operand)
                    inst.opcode = Opcode::IMUL;
                    inst.length = pos + 2;
                } else {
                    inst.length = pos + 2;
                }
            }
            break;

        // ===== STOSD / STOSB =====
        case 0xAB: // STOSD / STOSQ
            inst.opcode = Opcode::STOSD;
            inst.length = pos + 1;
            break;

        case 0xAA: // STOSB
            inst.opcode = Opcode::STOSB;
            inst.length = pos + 1;
            break;

        // ===== Jcc rel8 =====
        case 0x74: inst.opcode = Opcode::JZ;  inst.disp = (int8_t)code[pos + 1]; inst.hasDisp = true; inst.length = pos + 2; break;
        case 0x75: inst.opcode = Opcode::JNZ; inst.disp = (int8_t)code[pos + 1]; inst.hasDisp = true; inst.length = pos + 2; break;
        case 0x7C: inst.opcode = Opcode::JL;  inst.disp = (int8_t)code[pos + 1]; inst.hasDisp = true; inst.length = pos + 2; break;
        case 0x7D: inst.opcode = Opcode::JGE; inst.disp = (int8_t)code[pos + 1]; inst.hasDisp = true; inst.length = pos + 2; break;
        case 0x7E: inst.opcode = Opcode::JLE; inst.disp = (int8_t)code[pos + 1]; inst.hasDisp = true; inst.length = pos + 2; break;
        case 0x7F: inst.opcode = Opcode::JG;  inst.disp = (int8_t)code[pos + 1]; inst.hasDisp = true; inst.length = pos + 2; break;
        case 0x72: inst.opcode = Opcode::JB;  inst.disp = (int8_t)code[pos + 1]; inst.hasDisp = true; inst.length = pos + 2; break;
        case 0x73: inst.opcode = Opcode::JAE; inst.disp = (int8_t)code[pos + 1]; inst.hasDisp = true; inst.length = pos + 2; break;
        case 0x76: inst.opcode = Opcode::JBE; inst.disp = (int8_t)code[pos + 1]; inst.hasDisp = true; inst.length = pos + 2; break;
        case 0x77: inst.opcode = Opcode::JA;  inst.disp = (int8_t)code[pos + 1]; inst.hasDisp = true; inst.length = pos + 2; break;
        case 0x78: inst.opcode = Opcode::JS;  inst.disp = (int8_t)code[pos + 1]; inst.hasDisp = true; inst.length = pos + 2; break;
        case 0x79: inst.opcode = Opcode::JNS; inst.disp = (int8_t)code[pos + 1]; inst.hasDisp = true; inst.length = pos + 2; break;

        // ===== JMP rel8 =====
        case 0xEB:
            inst.opcode = Opcode::JMP;
            inst.disp = (int8_t)code[pos + 1];
            inst.hasDisp = true;
            inst.length = pos + 2;
            break;

        // ===== JMP rel32 =====
        case 0xE9:
            inst.opcode = Opcode::JMP;
            inst.disp = *(int32_t*)(code + pos + 1);
            inst.hasDisp = true;
            inst.length = pos + 5;
            break;

        // ===== ADD/SUB/CMP/AND/OR/XOR reg-to-reg =====
        case 0x01: // ADD r/m, r
        case 0x03: // ADD r, r/m
            if (code[pos + 1] >= 0xC0) {
                inst.opcode = Opcode::ADD;
                inst.isReg2Reg = true;
                if (opcode == 0x01) {
                    inst.reg1 = (code[pos + 1] & 0x07) | (rexB << 3);
                    inst.reg2 = ((code[pos + 1] >> 3) & 0x07) | (rexR << 3);
                } else {
                    inst.reg1 = ((code[pos + 1] >> 3) & 0x07) | (rexR << 3);
                    inst.reg2 = (code[pos + 1] & 0x07) | (rexB << 3);
                }
                inst.length = pos + 2;
            }
            break;

        case 0x29: // SUB r/m, r
        case 0x2B: // SUB r, r/m
            if (code[pos + 1] >= 0xC0) {
                inst.opcode = Opcode::SUB;
                inst.isReg2Reg = true;
                if (opcode == 0x29) {
                    inst.reg1 = (code[pos + 1] & 0x07) | (rexB << 3);
                    inst.reg2 = ((code[pos + 1] >> 3) & 0x07) | (rexR << 3);
                } else {
                    inst.reg1 = ((code[pos + 1] >> 3) & 0x07) | (rexR << 3);
                    inst.reg2 = (code[pos + 1] & 0x07) | (rexB << 3);
                }
                inst.length = pos + 2;
            }
            break;

        case 0x39: // CMP r/m, r
        case 0x3B: // CMP r, r/m
            if (code[pos + 1] >= 0xC0) {
                inst.opcode = Opcode::CMP;
                inst.isReg2Reg = true;
                if (opcode == 0x39) {
                    inst.reg1 = (code[pos + 1] & 0x07) | (rexB << 3);
                    inst.reg2 = ((code[pos + 1] >> 3) & 0x07) | (rexR << 3);
                } else {
                    inst.reg1 = ((code[pos + 1] >> 3) & 0x07) | (rexR << 3);
                    inst.reg2 = (code[pos + 1] & 0x07) | (rexB << 3);
                }
                inst.length = pos + 2;
            }
            break;

        case 0x21: // AND r/m, r
        case 0x23: // AND r, r/m
            if (code[pos + 1] >= 0xC0) {
                inst.opcode = Opcode::AND;
                inst.isReg2Reg = true;
                if (opcode == 0x21) {
                    inst.reg1 = (code[pos + 1] & 0x07) | (rexB << 3);
                    inst.reg2 = ((code[pos + 1] >> 3) & 0x07) | (rexR << 3);
                } else {
                    inst.reg1 = ((code[pos + 1] >> 3) & 0x07) | (rexR << 3);
                    inst.reg2 = (code[pos + 1] & 0x07) | (rexB << 3);
                }
                inst.length = pos + 2;
            }
            break;

        case 0x09: // OR r/m, r
        case 0x0B: // OR r, r/m
            if (code[pos + 1] >= 0xC0) {
                inst.opcode = Opcode::OR;
                inst.isReg2Reg = true;
                if (opcode == 0x09) {
                    inst.reg1 = (code[pos + 1] & 0x07) | (rexB << 3);
                    inst.reg2 = ((code[pos + 1] >> 3) & 0x07) | (rexR << 3);
                } else {
                    inst.reg1 = ((code[pos + 1] >> 3) & 0x07) | (rexR << 3);
                    inst.reg2 = (code[pos + 1] & 0x07) | (rexB << 3);
                }
                inst.length = pos + 2;
            }
            break;

        case 0x31: // XOR r/m, r
        case 0x33: // XOR r, r/m
            if (code[pos + 1] >= 0xC0) {
                inst.opcode = Opcode::XOR;
                inst.isReg2Reg = true;
                if (opcode == 0x31) {
                    inst.reg1 = (code[pos + 1] & 0x07) | (rexB << 3);
                    inst.reg2 = ((code[pos + 1] >> 3) & 0x07) | (rexR << 3);
                } else {
                    inst.reg1 = ((code[pos + 1] >> 3) & 0x07) | (rexR << 3);
                    inst.reg2 = (code[pos + 1] & 0x07) | (rexB << 3);
                }
                inst.length = pos + 2;
            }
            break;

        case 0x85: // TEST r/m, r (reg-to-reg)
            if (code[pos + 1] >= 0xC0) {
                inst.opcode = Opcode::TEST;
                inst.isReg2Reg = true;
                inst.reg1 = (code[pos + 1] & 0x07) | (rexB << 3);
                inst.reg2 = ((code[pos + 1] >> 3) & 0x07) | (rexR << 3);
                inst.length = pos + 2;
            }
            break;

        // ===== MOV r/m64, reg (0x89) / MOV reg, r/m64 (0x8B) =====
        case 0x89: // MOV r/m, r
            if (code[pos + 1] >= 0xC0) {
                inst.opcode = Opcode::MOV;
                inst.isReg2Reg = true;
                inst.reg1 = (code[pos + 1] & 0x07) | (rexB << 3);     // dst
                inst.reg2 = ((code[pos + 1] >> 3) & 0x07) | (rexR << 3); // src
                inst.length = pos + 2;
            }
            break;

        case 0x8B: // MOV r, r/m
            if (code[pos + 1] >= 0xC0) {
                inst.opcode = Opcode::MOV;
                inst.isReg2Reg = true;
                inst.reg1 = ((code[pos + 1] >> 3) & 0x07) | (rexR << 3); // dst
                inst.reg2 = (code[pos + 1] & 0x07) | (rexB << 3);        // src
                inst.length = pos + 2;
            }
            break;

        // ===== LEA =====
        case 0x8D:
            {
                uint8_t modrm = code[pos + 1];
                uint8_t mod = modrm >> 6;
                uint8_t rm = (modrm & 0x07) | (rexB << 3);
                inst.opcode = Opcode::LEA;
                inst.reg1 = ((modrm >> 3) & 0x07) | (rexR << 3); // dst

                if (mod == 0 && (modrm & 0x07) == 0x05) {
                    // [RIP + disp32]
                    inst.disp = *(int32_t*)(code + pos + 2);
                    inst.hasDisp = true;
                    inst.length = pos + 6;
                } else if (mod == 0x01) { // [reg + disp8]
                    inst.reg2 = rm;
                    inst.disp = (int8_t)code[pos + 2];
                    inst.hasDisp = true;
                    inst.isReg2Reg = true;
                    inst.length = pos + 3;
                } else if (mod == 0x02) { // [reg + disp32]
                    inst.reg2 = rm;
                    inst.disp = *(int32_t*)(code + pos + 2);
                    inst.hasDisp = true;
                    inst.isReg2Reg = true;
                    inst.length = pos + 6;
                } else if (mod == 0x03) { // reg, reg (unusual for LEA but valid)
                    inst.reg2 = rm;
                    inst.isReg2Reg = true;
                    inst.length = pos + 2;
                } else {
                    inst.length = pos + 2;
                }
            }
            break;

        // ===== XCHG reg, RAX =====
        case 0x91: case 0x92: case 0x93: case 0x94:
        case 0x95: case 0x96: case 0x97:
            inst.opcode = Opcode::XCHG;
            inst.reg1 = 0; // RAX
            inst.reg2 = (opcode - 0x90) | (rexB << 3);
            inst.isReg2Reg = true;
            inst.length = pos + 1;
            break;

        case 0x87: // XCHG r/m, r
            if (code[pos + 1] >= 0xC0) {
                inst.opcode = Opcode::XCHG;
                inst.isReg2Reg = true;
                inst.reg1 = (code[pos + 1] & 0x07) | (rexB << 3);
                inst.reg2 = ((code[pos + 1] >> 3) & 0x07) | (rexR << 3);
                inst.length = pos + 2;
            }
            break;

        // ===== Shift Group (C1) =====
        case 0xC1:
            {
                uint8_t modrm = code[pos + 1];
                uint8_t regOp = (modrm >> 3) & 0x07;
                inst.reg1 = (modrm & 0x07) | (rexB << 3);
                inst.imm = code[pos + 2];
                inst.hasImm = true;
                inst.length = pos + 3;
                switch (regOp) {
                    case 4: inst.opcode = Opcode::SHL; break;
                    case 5: inst.opcode = Opcode::SHR; break;
                    case 7: inst.opcode = Opcode::SAR; break;
                    default: inst.opcode = Opcode::UNKNOWN; break;
                }
            }
            break;

        // ===== Shift Group (D1 — shift by 1) =====
        case 0xD1:
            {
                uint8_t modrm = code[pos + 1];
                uint8_t regOp = (modrm >> 3) & 0x07;
                inst.reg1 = (modrm & 0x07) | (rexB << 3);
                inst.imm = 1;
                inst.hasImm = true;
                inst.length = pos + 2;
                switch (regOp) {
                    case 4: inst.opcode = Opcode::SHL; break;
                    case 5: inst.opcode = Opcode::SHR; break;
                    case 7: inst.opcode = Opcode::SAR; break;
                    default: inst.opcode = Opcode::UNKNOWN; break;
                }
            }
            break;

        // ===== CDQ / CQO (99) =====
        case 0x99:
            inst.opcode = Opcode::CDQ;
            inst.length = pos + 1;
            break;

        // ===== IMUL r, r/m (two-operand: 0F AF) handled below =====
        // ===== IMUL r, r/m, imm8 (6B) =====
        case 0x6B:
            if (code[pos + 1] >= 0xC0) {
                inst.opcode = Opcode::IMUL;
                inst.isReg2Reg = true;
                inst.reg1 = ((code[pos + 1] >> 3) & 0x07) | (rexR << 3);
                inst.reg2 = (code[pos + 1] & 0x07) | (rexB << 3);
                inst.imm = (uint64_t)(int64_t)(int8_t)code[pos + 2];
                inst.hasImm = true;
                inst.length = pos + 3;
            }
            break;

        // ===== IMUL r, r/m, imm32 (69) =====
        case 0x69:
            if (code[pos + 1] >= 0xC0) {
                inst.opcode = Opcode::IMUL;
                inst.isReg2Reg = true;
                inst.reg1 = ((code[pos + 1] >> 3) & 0x07) | (rexR << 3);
                inst.reg2 = (code[pos + 1] & 0x07) | (rexB << 3);
                inst.imm = *(int32_t*)(code + pos + 2);
                inst.hasImm = true;
                inst.length = pos + 6;
            }
            break;

        // ===== RET =====
        case 0xC3:
            inst.opcode = Opcode::RET;
            inst.length = pos + 1;
            break;

        case 0xC2: // RET imm16
            inst.opcode = Opcode::RET;
            inst.imm = *(uint16_t*)(code + pos + 1);
            inst.hasImm = true;
            inst.length = pos + 3;
            break;

        // ===== CALL rel32 =====
        case 0xE8:
            inst.opcode = Opcode::CALL;
            inst.imm = *(int32_t*)(code + pos + 1);
            inst.hasImm = true; 
            inst.length = pos + 5;
            break;

        // ===== Two-byte opcodes (0F prefix) =====
        case 0x0F:
            {
                uint8_t op2 = code[pos + 1];
                switch (op2) {
                    case 0x05: // SYSCALL
                        inst.opcode = Opcode::SYSCALL;
                        inst.length = pos + 2;
                        break;

                    case 0xAF: // IMUL r, r/m
                        if (code[pos + 2] >= 0xC0) {
                            inst.opcode = Opcode::IMUL;
                            inst.isReg2Reg = true;
                            inst.reg1 = ((code[pos + 2] >> 3) & 0x07) | (rexR << 3);
                            inst.reg2 = (code[pos + 2] & 0x07) | (rexB << 3);
                            inst.length = pos + 3;
                        }
                        break;

                    case 0xB6: // MOVZX r, r/m8
                        if (code[pos + 2] >= 0xC0) {
                            inst.opcode = Opcode::MOVZX;
                            inst.isReg2Reg = true;
                            inst.reg1 = ((code[pos + 2] >> 3) & 0x07) | (rexR << 3);
                            inst.reg2 = (code[pos + 2] & 0x07) | (rexB << 3);
                            inst.length = pos + 3;
                        }
                        break;

                    case 0xB7: // MOVZX r, r/m16
                        if (code[pos + 2] >= 0xC0) {
                            inst.opcode = Opcode::MOVZX;
                            inst.isReg2Reg = true;
                            inst.reg1 = ((code[pos + 2] >> 3) & 0x07) | (rexR << 3);
                            inst.reg2 = (code[pos + 2] & 0x07) | (rexB << 3);
                            inst.imm = 16; // flag for 16-bit source
                            inst.hasImm = true;
                            inst.length = pos + 3;
                        }
                        break;

                    case 0xBE: // MOVSX r, r/m8
                        if (code[pos + 2] >= 0xC0) {
                            inst.opcode = Opcode::MOVSX;
                            inst.isReg2Reg = true;
                            inst.reg1 = ((code[pos + 2] >> 3) & 0x07) | (rexR << 3);
                            inst.reg2 = (code[pos + 2] & 0x07) | (rexB << 3);
                            inst.length = pos + 3;
                        }
                        break;

                    case 0xBF: // MOVSX r, r/m16
                        if (code[pos + 2] >= 0xC0) {
                            inst.opcode = Opcode::MOVSX;
                            inst.isReg2Reg = true;
                            inst.reg1 = ((code[pos + 2] >> 3) & 0x07) | (rexR << 3);
                            inst.reg2 = (code[pos + 2] & 0x07) | (rexB << 3);
                            inst.imm = 16;
                            inst.hasImm = true;
                            inst.length = pos + 3;
                        }
                        break;

                    // Jcc rel32 (0F 8x)
                    case 0x84: inst.opcode = Opcode::JZ;  inst.disp = *(int32_t*)(code + pos + 2); inst.hasDisp = true; inst.length = pos + 6; break;
                    case 0x85: inst.opcode = Opcode::JNZ; inst.disp = *(int32_t*)(code + pos + 2); inst.hasDisp = true; inst.length = pos + 6; break;
                    case 0x8C: inst.opcode = Opcode::JL;  inst.disp = *(int32_t*)(code + pos + 2); inst.hasDisp = true; inst.length = pos + 6; break;
                    case 0x8D: inst.opcode = Opcode::JGE; inst.disp = *(int32_t*)(code + pos + 2); inst.hasDisp = true; inst.length = pos + 6; break;
                    case 0x8E: inst.opcode = Opcode::JLE; inst.disp = *(int32_t*)(code + pos + 2); inst.hasDisp = true; inst.length = pos + 6; break;
                    case 0x8F: inst.opcode = Opcode::JG;  inst.disp = *(int32_t*)(code + pos + 2); inst.hasDisp = true; inst.length = pos + 6; break;
                    case 0x82: inst.opcode = Opcode::JB;  inst.disp = *(int32_t*)(code + pos + 2); inst.hasDisp = true; inst.length = pos + 6; break;
                    case 0x83: inst.opcode = Opcode::JAE; inst.disp = *(int32_t*)(code + pos + 2); inst.hasDisp = true; inst.length = pos + 6; break;
                    case 0x86: inst.opcode = Opcode::JBE; inst.disp = *(int32_t*)(code + pos + 2); inst.hasDisp = true; inst.length = pos + 6; break;
                    case 0x87: inst.opcode = Opcode::JA;  inst.disp = *(int32_t*)(code + pos + 2); inst.hasDisp = true; inst.length = pos + 6; break;
                    case 0x88: inst.opcode = Opcode::JS;  inst.disp = *(int32_t*)(code + pos + 2); inst.hasDisp = true; inst.length = pos + 6; break;
                    case 0x89: inst.opcode = Opcode::JNS; inst.disp = *(int32_t*)(code + pos + 2); inst.hasDisp = true; inst.length = pos + 6; break;

                    default:
                        inst.opcode = Opcode::UNKNOWN;
                        inst.length = pos + 2;
                        break;
                }
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
