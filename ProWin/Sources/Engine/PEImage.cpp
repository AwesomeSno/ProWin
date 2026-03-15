#include "PEImage.h"
#include "MemoryManager.h"
#include <fstream>
#include <cstring>
#include <unistd.h>
#include <iostream>

namespace ProWin {

PEImage::PEImage() : m_mappedBase(nullptr), m_imageSize(0), m_entryPoint(0) {}

PEImage::~PEImage() {
    if (m_mappedBase) {
        MemoryManager::release(m_mappedBase, m_imageSize);
    }
}

bool PEImage::load(const std::string& filePath) {
    PEParser::PEInfo info = PEParser::getInfo(filePath);
    if (!info.isValid) return false;

    m_imageSize = info.sizeOfImage;
    m_mappedBase = MemoryManager::reserve(info.imageBase, m_imageSize);
    
    if (!m_mappedBase) return false;

    std::ifstream file(filePath, std::ios::binary);
    if (!file.is_open()) return false;

    // 1. Map Headers (typically first page/512 bytes)
    // For simplicity, we commit the first page for headers
    size_t pageSize = getpagesize();
    MemoryManager::commit(m_mappedBase, pageSize, PROT_READ | PROT_WRITE);
    
    char* headerBuf = new char[pageSize];
    file.read(headerBuf, pageSize);
    memcpy(m_mappedBase, headerBuf, pageSize);
    delete[] headerBuf;

    // 2. Map Sections
    for (const auto& section : info.sections) {
        if (section.rawDataSize == 0) continue;

        void* sectionAddr = (char*)m_mappedBase + section.virtualAddress;
        size_t commitSize = (section.virtualSize + pageSize - 1) & ~(pageSize - 1);
        
        MemoryManager::commit(sectionAddr, commitSize, PROT_READ | PROT_WRITE);
        
        file.seekg(section.rawDataPtr, std::ios::beg);
        file.read((char*)sectionAddr, section.rawDataSize);
        
        // Zero-fill remaining virtual size if any
        if (section.virtualSize > section.rawDataSize) {
            memset((char*)sectionAddr + section.rawDataSize, 0, section.virtualSize - section.rawDataSize);
        }
    }

    // 3. Set Final Protections (simplified: just marks READ|EXEC for everything for now)
    // REAL implementation should map characteristics to protection flags
    MemoryManager::protect(m_mappedBase, m_imageSize, PROT_READ | PROT_EXEC);

    m_entryPoint = (uint64_t)((char*)m_mappedBase + info.entryPointRVA);
    m_filePath = filePath;

    return true;
}

}
