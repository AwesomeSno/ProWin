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
    m_importRVA = info.importDirectoryRVA;
    m_importSize = info.importDirectorySize;
    m_relocRVA = info.relocationDirectoryRVA;
    m_relocSize = info.relocationDirectorySize;
    m_preferredBaseAddress = info.imageBase;
    
    m_mappedBase = MemoryManager::reserve(info.imageBase, m_imageSize);
    
    if (!m_mappedBase) return false;
    
    uint64_t delta = (uint64_t)m_mappedBase - m_preferredBaseAddress;

    std::ifstream file(filePath, std::ios::binary);
    if (!file.is_open()) return false;

    // 1. Map Headers (typically first page/512 bytes)
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

    // 3. Apply Relocations if delta != 0 (ASLR)
    if (delta != 0 && m_relocRVA != 0) {
        applyRelocations(delta);
    }

    // 4. Set Final Protections
    MemoryManager::protect(m_mappedBase, m_imageSize, PROT_READ | PROT_EXEC);

    m_entryPoint = (uint64_t)((char*)m_mappedBase + info.entryPointRVA);
    m_filePath = filePath;

    return true;
}

bool PEImage::applyRelocations(uint64_t delta) {
    if (m_relocRVA == 0 || m_relocSize == 0) return true;

    IMAGE_BASE_RELOCATION* reloc = (IMAGE_BASE_RELOCATION*)((char*)m_mappedBase + m_relocRVA);
    void* end = (char*)reloc + m_relocSize;

    while ((void*)reloc < end && reloc->SizeOfBlock > 0) {
        uint32_t count = (reloc->SizeOfBlock - sizeof(IMAGE_BASE_RELOCATION)) / sizeof(uint16_t);
        uint16_t* entry = (uint16_t*)(reloc + 1);

        for (uint32_t i = 0; i < count; ++i) {
            uint16_t type = entry[i] >> 12;
            uint16_t offset = entry[i] & 0x0FFF;

            if (type == IMAGE_REL_BASED_DIR64) {
                uint64_t* patchAddr = (uint64_t*)((char*)m_mappedBase + reloc->VirtualAddress + offset);
                *patchAddr += delta;
            }
        }
        reloc = (IMAGE_BASE_RELOCATION*)((char*)reloc + reloc->SizeOfBlock);
    }

    return true;
}

}
