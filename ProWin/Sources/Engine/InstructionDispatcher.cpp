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
