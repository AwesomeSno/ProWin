#include "InstructionDispatcher.h"
#include <cstdio>

namespace ProWin {

bool InstructionDispatcher::execute(const Instruction& inst, CPUContext& context) {
    switch (inst.opcode) {
        case Opcode::NOP:
            printf("[ProWin] Interpreter: NOP at 0x%llx\n", context.rip);
            break;

        case Opcode::RET:
            printf("[ProWin] Interpreter: RET at 0x%llx\n", context.rip);
            // Simplified: Stop execution for now
            return false;

        case Opcode::MOV:
            if (inst.hasImm) {
                if (uint64_t* dst = context.getGPR(inst.reg1)) {
                    printf("[ProWin] Interpreter: MOV R%d, 0x%llx\n", inst.reg1, inst.imm);
                    *dst = inst.imm;
                }
            } else if (inst.isReg2Reg) {
                uint64_t* dst = context.getGPR(inst.reg1);
                uint64_t* src = context.getGPR(inst.reg2);
                if (dst && src) {
                    printf("[ProWin] Interpreter: MOV R%d, R%d (0x%llx)\n", inst.reg1, inst.reg2, *src);
                    *dst = *src;
                }
            }
            break;

        case Opcode::ADD:
            if (inst.isReg2Reg) {
                uint64_t* dst = context.getGPR(inst.reg1);
                uint64_t* src = context.getGPR(inst.reg2);
                if (dst && src) {
                    printf("[ProWin] Interpreter: ADD R%d, R%d (0x%llx + 0x%llx)\n", inst.reg1, inst.reg2, *dst, *src);
                    *dst += *src;
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
    // Advance RIP
    context.rip += inst.length;
    return true;
}

}
