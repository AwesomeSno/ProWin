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
    void* getBaseAddress() const { return m_mappedBase; }
    uint64_t getEntryPoint() const { return m_entryPoint; }

private:
    void* m_mappedBase;
    size_t m_imageSize;
    uint64_t m_entryPoint;
    std::string m_filePath;
};

}

#endif /* PEImage_h */
