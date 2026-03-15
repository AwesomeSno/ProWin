#ifndef PEImage_h
#define PEImage_h

#include <string>
#include <vector>
#include "PEParser.h"

namespace ProWin {

class PEImage {
public:
    PEImage();
    ~PEImage();

    bool load(const std::string& filePath);
    bool resolveImports();
    bool applyRelocations(uint64_t delta);
    void* getBaseAddress() const { return m_mappedBase; }
    uint64_t getEntryPoint() const { return m_entryPoint; }

private:
    void* m_mappedBase;
    size_t m_imageSize;
    uint32_t m_importRVA;
    uint32_t m_importSize;
    uint32_t m_relocRVA;
    uint32_t m_relocSize;
    uint64_t m_preferredBaseAddress;
    uint64_t m_entryPoint;
    std::string m_filePath;
};

}

#endif /* PEImage_h */
