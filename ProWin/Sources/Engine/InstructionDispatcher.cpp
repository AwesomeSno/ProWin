#include "InstructionDispatcher.h"
#include <cstdio>
#include "ALU.h"
#include "MemoryManager.h"

namespace ProWin {

bool InstructionDispatcher::execute(const Instruction& inst, CPUContext& context) {
    bool branchTaken = false;
    switch (inst.opcode) {
        case Opcode::NOP:
            break;

        // ===== STACK & CONTROL FLOW =====
        case Opcode::RET:
            {
                uint64_t retAddr = MemoryManager::read64(context.rsp);
                context.rsp += 8;
                if (inst.hasImm) {
                    context.rsp += inst.imm;
                }
                context.rip = retAddr;
                branchTaken = true;
            }
            break;

        case Opcode::CALL:
            {
                uint64_t retAddr = context.rip + inst.length;
                context.rsp -= 8;
                MemoryManager::write64(context.rsp, retAddr);

                if (inst.hasImm) {
                    context.rip = retAddr + (int64_t)(int32_t)inst.imm;
                } else {
                    if (uint64_t* src = context.getGPR(inst.reg1)) {
                        context.rip = *src;
                    }
                }
                branchTaken = true;
            }
            break;

        case Opcode::PUSH:
            {
                if (uint64_t* val = context.getGPR(inst.reg1)) {
                    context.rsp -= 8;
                    MemoryManager::write64(context.rsp, *val);
                }
            }
            break;

        case Opcode::POP:
            {
                if (uint64_t* reg = context.getGPR(inst.reg1)) {
                    *reg = MemoryManager::read64(context.rsp);
                    context.rsp += 8;
                }
            }
            break;

        // ===== DATA MOVEMENT =====
        case Opcode::MOV:
            if (inst.hasImm) {
                if (uint64_t* dst = context.getGPR(inst.reg1)) {
                    *dst = inst.imm;
                }
            } else if (inst.isReg2Reg) {
                uint64_t* dst = context.getGPR(inst.reg1);
                uint64_t* src = context.getGPR(inst.reg2);
                if (dst && src) *dst = *src;
            }
            break;

        case Opcode::LEA:
            {
                if (uint64_t* dst = context.getGPR(inst.reg1)) {
                    if (inst.isReg2Reg && inst.hasDisp) {
                        // LEA dst, [reg2 + disp]
                        if (uint64_t* base = context.getGPR(inst.reg2)) {
                            *dst = *base + (int64_t)inst.disp;
                        }
                    } else if (inst.hasDisp) {
                        // LEA dst, [RIP + disp32]
                        *dst = context.rip + inst.length + (int64_t)inst.disp;
                    }
                }
            }
            break;

        case Opcode::XCHG:
            {
                uint64_t* a = context.getGPR(inst.reg1);
                uint64_t* b = context.getGPR(inst.reg2);
                if (a && b) {
                    uint64_t tmp = *a;
                    *a = *b;
                    *b = tmp;
                }
            }
            break;

        case Opcode::MOVZX:
            {
                uint64_t* dst = context.getGPR(inst.reg1);
                uint64_t* src = context.getGPR(inst.reg2);
                if (dst && src) {
                    if (inst.hasImm && inst.imm == 16) {
                        *dst = *src & 0xFFFF; // zero-extend from 16-bit
                    } else {
                        *dst = *src & 0xFF;   // zero-extend from 8-bit
                    }
                }
            }
            break;

        case Opcode::MOVSX:
            {
                uint64_t* dst = context.getGPR(inst.reg1);
                uint64_t* src = context.getGPR(inst.reg2);
                if (dst && src) {
                    if (inst.hasImm && inst.imm == 16) {
                        *dst = (uint64_t)(int64_t)(int16_t)(*src & 0xFFFF);
                    } else {
                        *dst = (uint64_t)(int64_t)(int8_t)(*src & 0xFF);
                    }
                }
            }
            break;

        // ===== ARITHMETIC =====
        case Opcode::ADD:
            if (inst.hasImm) {
                if (uint64_t* dst = context.getGPR(inst.reg1)) {
                    uint64_t a = *dst;
                    uint64_t b = inst.imm;
                    *dst = a + b;
                    updateFlags64(context, *dst, a, b, false);
                }
            } else if (inst.isReg2Reg) {
                uint64_t* dst = context.getGPR(inst.reg1);
                uint64_t* src = context.getGPR(inst.reg2);
                if (dst && src) {
                    uint64_t a = *dst;
                    uint64_t b = *src;
                    *dst = a + b;
                    updateFlags64(context, *dst, a, b, false);
                }
            }
            break;

        case Opcode::SUB:
            if (inst.hasImm) {
                if (uint64_t* dst = context.getGPR(inst.reg1)) {
                    uint64_t a = *dst;
                    uint64_t b = inst.imm;
                    *dst = a - b;
                    updateFlags64(context, *dst, a, b, true);
                }
            } else if (inst.isReg2Reg) {
                uint64_t* dst = context.getGPR(inst.reg1);
                uint64_t* src = context.getGPR(inst.reg2);
                if (dst && src) {
                    uint64_t a = *dst;
                    uint64_t b = *src;
                    *dst = a - b;
                    updateFlags64(context, *dst, a, b, true);
                }
            }
            break;

        case Opcode::CMP:
            // CMP is identical to SUB but does NOT write the result
            if (inst.hasImm) {
                if (uint64_t* dst = context.getGPR(inst.reg1)) {
                    uint64_t a = *dst;
                    uint64_t b = inst.imm;
                    uint64_t result = a - b;
                    updateFlags64(context, result, a, b, true);
                }
            } else if (inst.isReg2Reg) {
                uint64_t* dst = context.getGPR(inst.reg1);
                uint64_t* src = context.getGPR(inst.reg2);
                if (dst && src) {
                    uint64_t a = *dst;
                    uint64_t b = *src;
                    uint64_t result = a - b;
                    updateFlags64(context, result, a, b, true);
                }
            }
            break;

        case Opcode::INC:
            if (uint64_t* reg = context.getGPR(inst.reg1)) {
                uint64_t a = *reg;
                *reg = a + 1;
                updateFlags64(context, *reg, a, 1, false);
            }
            break;

        case Opcode::DEC:
            if (uint64_t* reg = context.getGPR(inst.reg1)) {
                uint64_t a = *reg;
                *reg = a - 1;
                updateFlags64(context, *reg, a, 1, true);
            }
            break;

        case Opcode::NEG:
            if (uint64_t* reg = context.getGPR(inst.reg1)) {
                uint64_t a = *reg;
                *reg = (uint64_t)(-(int64_t)a);
                updateFlags64(context, *reg, 0, a, true);
            }
            break;

        case Opcode::IMUL:
            if (inst.isReg2Reg && inst.hasImm) {
                // IMUL r, r/m, imm
                uint64_t* dst = context.getGPR(inst.reg1);
                uint64_t* src = context.getGPR(inst.reg2);
                if (dst && src) {
                    *dst = (uint64_t)((int64_t)*src * (int64_t)inst.imm);
                }
            } else if (inst.isReg2Reg) {
                // IMUL r, r/m (two-operand)
                uint64_t* dst = context.getGPR(inst.reg1);
                uint64_t* src = context.getGPR(inst.reg2);
                if (dst && src) {
                    *dst = (uint64_t)((int64_t)*dst * (int64_t)*src);
                }
            } else {
                // IMUL r/m (one-operand: result in RDX:RAX)
                if (uint64_t* src = context.getGPR(inst.reg1)) {
                    __int128 result = (__int128)(int64_t)context.rax * (__int128)(int64_t)*src;
                    context.rax = (uint64_t)result;
                    context.rdx = (uint64_t)(result >> 64);
                }
            }
            break;

        case Opcode::CDQ:
            // CDQ (32-bit): sign-extend EAX into EDX:EAX
            // CQO (REX.W): sign-extend RAX into RDX:RAX
            context.rdx = ((int64_t)context.rax < 0) ? 0xFFFFFFFFFFFFFFFFULL : 0;
            break;

        // ===== LOGICAL =====
        case Opcode::AND:
            if (inst.hasImm) {
                if (uint64_t* dst = context.getGPR(inst.reg1)) {
                    *dst &= inst.imm;
                    updateFlags64(context, *dst, *dst, 0, false);
                }
            } else if (inst.isReg2Reg) {
                uint64_t* dst = context.getGPR(inst.reg1);
                uint64_t* src = context.getGPR(inst.reg2);
                if (dst && src) {
                    *dst &= *src;
                    updateFlags64(context, *dst, *dst, 0, false);
                }
            }
            break;

        case Opcode::OR:
            if (inst.hasImm) {
                if (uint64_t* dst = context.getGPR(inst.reg1)) {
                    *dst |= inst.imm;
                    updateFlags64(context, *dst, *dst, 0, false);
                }
            } else if (inst.isReg2Reg) {
                uint64_t* dst = context.getGPR(inst.reg1);
                uint64_t* src = context.getGPR(inst.reg2);
                if (dst && src) {
                    *dst |= *src;
                    updateFlags64(context, *dst, *dst, 0, false);
                }
            }
            break;

        case Opcode::XOR:
            if (inst.hasImm) {
                if (uint64_t* dst = context.getGPR(inst.reg1)) {
                    *dst ^= inst.imm;
                    updateFlags64(context, *dst, *dst, 0, false);
                }
            } else if (inst.isReg2Reg) {
                uint64_t* dst = context.getGPR(inst.reg1);
                uint64_t* src = context.getGPR(inst.reg2);
                if (dst && src) {
                    *dst ^= *src;
                    updateFlags64(context, *dst, *dst, 0, false);
                }
            }
            break;

        case Opcode::NOT:
            if (uint64_t* reg = context.getGPR(inst.reg1)) {
                *reg = ~(*reg);
            }
            break;

        case Opcode::TEST:
            // TEST sets flags like AND but discards result
            if (inst.hasImm) {
                if (uint64_t* dst = context.getGPR(inst.reg1)) {
                    uint64_t result = *dst & inst.imm;
                    updateFlags64(context, result, result, 0, false);
                }
            } else if (inst.isReg2Reg) {
                uint64_t* a = context.getGPR(inst.reg1);
                uint64_t* b = context.getGPR(inst.reg2);
                if (a && b) {
                    uint64_t result = *a & *b;
                    updateFlags64(context, result, result, 0, false);
                }
            }
            break;

        // ===== SHIFTS =====
        case Opcode::SHL:
            if (uint64_t* dst = context.getGPR(inst.reg1)) {
                uint8_t count = inst.imm & 0x3F;
                uint64_t a = *dst;
                *dst = a << count;
                updateFlags64(context, *dst, a, count, false);
            }
            break;

        case Opcode::SHR:
            if (uint64_t* dst = context.getGPR(inst.reg1)) {
                uint8_t count = inst.imm & 0x3F;
                uint64_t a = *dst;
                *dst = a >> count;
                updateFlags64(context, *dst, a, count, false);
            }
            break;

        case Opcode::SAR:
            if (uint64_t* dst = context.getGPR(inst.reg1)) {
                uint8_t count = inst.imm & 0x3F;
                int64_t a = (int64_t)*dst;
                *dst = (uint64_t)(a >> count);
                updateFlags64(context, *dst, (uint64_t)a, count, false);
            }
            break;

        // ===== STRING OPERATIONS =====
        case Opcode::STOSD:
            {
                uint32_t val = (uint32_t)context.rax;
                uint32_t* ptr = (uint32_t*)context.rdi;
                *ptr = val;
                context.rdi += 4;
            }
            break;

        case Opcode::STOSB:
            {
                uint8_t val = (uint8_t)context.rax;
                *(uint8_t*)context.rdi = val;
                context.rdi += 1;
            }
            break;

        // ===== JUMPS =====
        case Opcode::JMP:
            if (inst.hasDisp) {
                context.rip += inst.length + inst.disp;
                branchTaken = true;
            } else if (inst.isReg2Reg) {
                if (uint64_t* src = context.getGPR(inst.reg1)) {
                    context.rip = *src;
                    branchTaken = true;
                }
            }
            break;

        // ===== CONDITIONAL JUMPS =====
        case Opcode::JZ:
            if ((context.rflags >> 6) & 1) { context.rip += inst.length + inst.disp; branchTaken = true; }
            break;
        case Opcode::JNZ:
            if (!((context.rflags >> 6) & 1)) { context.rip += inst.length + inst.disp; branchTaken = true; }
            break;
        case Opcode::JL:
            { bool sf = (context.rflags >> 7) & 1; bool of = (context.rflags >> 11) & 1;
              if (sf != of) { context.rip += inst.length + inst.disp; branchTaken = true; } }
            break;
        case Opcode::JGE:
            { bool sf = (context.rflags >> 7) & 1; bool of = (context.rflags >> 11) & 1;
              if (sf == of) { context.rip += inst.length + inst.disp; branchTaken = true; } }
            break;
        case Opcode::JLE:
            { bool zf = (context.rflags >> 6) & 1; bool sf = (context.rflags >> 7) & 1; bool of = (context.rflags >> 11) & 1;
              if (zf || sf != of) { context.rip += inst.length + inst.disp; branchTaken = true; } }
            break;
        case Opcode::JG:
            { bool zf = (context.rflags >> 6) & 1; bool sf = (context.rflags >> 7) & 1; bool of = (context.rflags >> 11) & 1;
              if (!zf && sf == of) { context.rip += inst.length + inst.disp; branchTaken = true; } }
            break;
        case Opcode::JB:
            if (context.rflags & 1) { context.rip += inst.length + inst.disp; branchTaken = true; }
            break;
        case Opcode::JAE:
            if (!(context.rflags & 1)) { context.rip += inst.length + inst.disp; branchTaken = true; }
            break;
        case Opcode::JBE:
            { bool cf = context.rflags & 1; bool zf = (context.rflags >> 6) & 1;
              if (cf || zf) { context.rip += inst.length + inst.disp; branchTaken = true; } }
            break;
        case Opcode::JA:
            { bool cf = context.rflags & 1; bool zf = (context.rflags >> 6) & 1;
              if (!cf && !zf) { context.rip += inst.length + inst.disp; branchTaken = true; } }
            break;
        case Opcode::JS:
            if ((context.rflags >> 7) & 1) { context.rip += inst.length + inst.disp; branchTaken = true; }
            break;
        case Opcode::JNS:
            if (!((context.rflags >> 7) & 1)) { context.rip += inst.length + inst.disp; branchTaken = true; }
            break;

        // ===== SYSTEM =====
        case Opcode::SYSCALL:
            printf("[ProWin] Interpreter: SYSCALL at 0x%llx (RAX=0x%llx)\n", context.rip, context.rax);
            break;

        case Opcode::UNKNOWN:
            printf("[ProWin] Interpreter: Unknown opcode (0x%02X) at 0x%llx\n", *(uint8_t*)context.rip, context.rip);
            fflush(stdout);
            return false;

        default:
            break;
    }

    fflush(stdout);
    if (!branchTaken) {
        context.rip += inst.length;
    }
    return true;
}

}
