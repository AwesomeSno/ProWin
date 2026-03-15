#ifndef PEParser_h
#define PEParser_h

#include <string>
#include <vector>
#include <cstdint>
#include "PEStructures.h"

namespace ProWin {

class PEParser {
public:
    struct Section {
        std::string name;
        uint32_t virtualAddress;
        uint32_t virtualSize;
        uint32_t rawDataSize;
        uint32_t rawDataPtr;
        uint32_t characteristics;
    };

    struct PEInfo {
        bool isValid;
        bool is64Bit;
        uint64_t imageBase;
        uint32_t entryPointRVA;
        uint32_t sizeOfImage;
        std::vector<Section> sections;
    };

    static PEInfo getInfo(const std::string& filePath);
};

}

#endif /* PEParser_h */
