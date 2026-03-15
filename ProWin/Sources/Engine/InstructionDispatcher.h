#ifndef InstructionDispatcher_h
#define InstructionDispatcher_h

#include "InstructionDecoder.h"
#include "CPUContext.h"

namespace ProWin {

class InstructionDispatcher {
public:
    static bool execute(const Instruction& inst, CPUContext& context);
};

}

#endif /* InstructionDispatcher_h */
