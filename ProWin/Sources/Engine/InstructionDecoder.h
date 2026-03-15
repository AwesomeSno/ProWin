#ifndef InstructionDecoder_h
#define InstructionDecoder_h

#include <cstdint>
#include <vector>
#include "CPUContext.h"

namespace ProWin {

enum class Opcode {
    UNKNOWN,
    MOV,
    ADD,
    SUB,
    PUSH,
    POP,
    CALL,
    RET,
    JMP,
    CMP,
    LEA,
    SYSCALL,
    NOP
};

struct Instruction {
    Opcode opcode;
    uint8_t length;
    // Operands and metadata would go here
};

class InstructionDecoder {
public:
    static Instruction decode(const uint8_t* code);
};

}

#endif /* InstructionDecoder_h */
