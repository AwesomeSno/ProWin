#include "InstructionDispatcher.h"
#include <iostream>

namespace ProWin {

bool InstructionDispatcher::execute(const Instruction& inst, CPUContext& context) {
    switch (inst.opcode) {
        case Opcode::NOP:
            std::cout << "[ProWin] Interpreter: NOP at 0x" << std::hex << context.rip << std::endl;
            break;

        case Opcode::RET:
            std::cout << "[ProWin] Interpreter: RET at 0x" << std::hex << context.rip << std::endl;
            // Simplified: Stop execution for now
            return false;

        case Opcode::SYSCALL:
            std::cout << "[ProWin] Interpreter: SYSCALL at 0x" << std::hex << context.rip << std::endl;
            break;

        case Opcode::UNKNOWN:
            std::cerr << "[ProWin] Interpreter: Unknown opcode at 0x" << std::hex << context.rip << std::endl;
            return false;

        default:
            std::cout << "[ProWin] Interpreter: Executing opcode type " << (int)inst.opcode << std::endl;
            break;
    }

    // Advance RIP
    context.rip += inst.length;
    return true;
}

}
