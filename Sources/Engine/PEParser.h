#ifndef PEParser_h
#define PEParser_h

#include <string>
#include <vector>
#include <cstdint>
#include "PEStructures.h"

namespace ProWin {

class PEParser {
public:
    struct PEInfo {
        bool isValid;
        bool is64Bit;
        uint64_t imageBase;
        uint32_t entryPointRVA;
        uint32_t sizeOfImage;
    };

    static PEInfo getInfo(const std::string& filePath);
};

}

#endif /* PEParser_h */
