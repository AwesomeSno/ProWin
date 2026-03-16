#include "InstructionDispatcher.h"
#include <cstdio>
#include "ALU.h"
#include "MemoryManager.h"

namespace ProWin {

bool InstructionDispatcher::execute(const Instruction& inst, CPUContext& context) {
    bool branchTaken = false;
    switch (inst.opcode) {
        case Opcode::NOP:
            printf("[ProWin] Interpreter: NOP at 0x%llx\n", context.rip);
            break;

        case Opcode::RET:
            {
                uint64_t retAddr = MemoryManager::read64(context.rsp);
                context.rsp += 8;
                if (inst.hasImm) {
                    context.rsp += inst.imm;
                }
                context.rip = retAddr;
                branchTaken = true;
                printf("[ProWin] Interpreter: RET to 0x%llx\n", context.rip);
            }
            break;

        case Opcode::CALL:
            {
                // Push return address (next instruction)
                uint64_t retAddr = context.rip + inst.length;
                context.rsp -= 8;
                MemoryManager::write64(context.rsp, retAddr);

                if (inst.hasImm) {
                    // CALL rel32: RIP = next_rip + displacement
                    context.rip = retAddr + (int64_t)(int32_t)inst.imm;
                } else {
                    // CALL r/m64
                    if (uint64_t* src = context.getGPR(inst.reg1)) {
                        context.rip = *src;
                    }
                }
                branchTaken = true;
                printf("[ProWin] Interpreter: CALL to 0x%llx\n", context.rip);
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

        case Opcode::INC:
            {
                if (uint64_t* reg = context.getGPR(inst.reg1)) {
                    uint64_t a = *reg;
                    *reg = a + 1;
                    updateFlags64(context, *reg, a, 1, false);
                }
            }
            break;

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

        case Opcode::DEC:
            if (uint64_t* reg = context.getGPR(inst.reg1)) {
                uint64_t a = *reg;
                *reg = a - 1;
                updateFlags64(context, *reg, a, 1, true);
            }
            break;

        case Opcode::STOSD:
            // Store EAX at [RDI], then RDI += 4
            {
                uint32_t val = (uint32_t)context.rax;
                uint32_t* ptr = (uint32_t*)context.rdi;
                // Safety: Basic check if within VRAM range? 
                // For now, assume test code is correct.
                *ptr = val;
                context.rdi += 4;
            }
            break;

        case Opcode::JZ:
            {
                // Reads RFLAGS bit 6 (ZF) — set by ADD, DEC, CMP, etc.
                bool zf = (context.rflags >> 6) & 1;
                if (zf == 1) {
                    context.rip += inst.length + inst.disp;
                    branchTaken = true;
                }
            }
            break;

        case Opcode::JNZ:
            {
                // Reads RFLAGS bit 6 (ZF) — set by ADD, DEC, CMP, etc.
                bool zf = (context.rflags >> 6) & 1;
                if (zf == 0) {
                    context.rip += inst.length + inst.disp;
                    branchTaken = true;
                }
            }
            break;

        case Opcode::JL:
            {
                // Reads RFLAGS bit 7 (SF) and bit 11 (OF)
                bool sf = (context.rflags >> 7) & 1;
                bool of = (context.rflags >> 11) & 1;
                if (sf != of) {
                    context.rip += inst.length + inst.disp;
                    branchTaken = true;
                }
            }
            break;

        case Opcode::JGE:
            {
                // Reads RFLAGS bit 7 (SF) and bit 11 (OF)
                bool sf = (context.rflags >> 7) & 1;
                bool of = (context.rflags >> 11) & 1;
                if (sf == of) {
                    context.rip += inst.length + inst.disp;
                    branchTaken = true;
                }
            }
            break;

        case Opcode::JLE:
            {
                // Reads RFLAGS bits 6 (ZF), 7 (SF), 11 (OF)
                bool zf = (context.rflags >> 6) & 1;
                bool sf = (context.rflags >> 7) & 1;
                bool of = (context.rflags >> 11) & 1;
                if (zf == 1 || sf != of) {
                    context.rip += inst.length + inst.disp;
                    branchTaken = true;
                }
            }
            break;

        case Opcode::JG:
            {
                // Reads RFLAGS bits 6 (ZF), 7 (SF), 11 (OF)
                bool zf = (context.rflags >> 6) & 1;
                bool sf = (context.rflags >> 7) & 1;
                bool of = (context.rflags >> 11) & 1;
                if (zf == 0 && sf == of) {
                    context.rip += inst.length + inst.disp;
                    branchTaken = true;
                }
            }
            break;

        case Opcode::SYSCALL:
            printf("[ProWin] Interpreter: SYSCALL at 0x%llx\n", context.rip);
            break;

        case Opcode::UNKNOWN:
            printf("[ProWin] Interpreter: Unknown opcode (0x%02X) at 0x%llx\n", *(uint8_t*)context.rip, context.rip);
            fflush(stdout);
            return false;

        default:
            printf("[ProWin] Interpreter: Executing opcode type %d\n", (int)inst.opcode);
            break;
    }

    fflush(stdout);
    // RIP is advanced ONLY here, never inside case handlers.
    if (!branchTaken) {
        context.rip += inst.length;
    }
    return true;
}

}
