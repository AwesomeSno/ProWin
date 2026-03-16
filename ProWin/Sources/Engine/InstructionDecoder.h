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
    NOP,
    STOSD,
    DEC,
    JNZ,
    JZ,
    JL,
    JGE,
    JLE,
    JG,
    INC
};

struct Instruction {
    Opcode opcode;
    uint8_t length;
    
    // Operands
    uint8_t reg1;    // Destination (Index into CPUContext.gpr)
    uint8_t reg2;    // Source (Index into CPUContext.gpr)
    uint64_t imm;    // Immediate value
    int32_t disp;    // Displacement (offset)
    bool hasImm;
    bool hasDisp;
    bool isReg2Reg; // True if both operands are registers
};

class InstructionDecoder {
public:
    static Instruction decode(const uint8_t* code);
};

}

#endif /* InstructionDecoder_h */
