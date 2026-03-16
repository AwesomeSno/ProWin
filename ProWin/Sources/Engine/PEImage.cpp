#include "PEImage.h"
#include "MemoryManager.h"
#include "StubManager.h"
#include <fstream>
#include <cstring>
#include <unistd.h>
#include <iostream>

#include <sys/mman.h>

namespace ProWin {

static int peCharsToPosixProt(uint32_t characteristics) {
    int prot = 0;
    if (characteristics & IMAGE_SCN_MEM_READ)    prot |= PROT_READ;
    if (characteristics & IMAGE_SCN_MEM_WRITE)   prot |= PROT_WRITE;
    if (characteristics & IMAGE_SCN_MEM_EXECUTE) prot |= PROT_EXEC;
    if (prot == 0) prot = PROT_NONE; // guard pages
    return prot;
}

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
    if (!MemoryManager::commit(m_mappedBase, pageSize, PROT_READ | PROT_WRITE)) {
        printf("[ProWin] PEImage ERROR: Failed to commit header memory\n");
        return false;
    }
    
    // Safety: Only read what exists in the file (file is likely < 16KB pageSize)
    file.seekg(0, std::ios::end);
    size_t fileSize = file.tellg();
    file.seekg(0, std::ios::beg);

    size_t readSize = (fileSize < pageSize) ? fileSize : pageSize;
    char* headerBuf = new char[readSize];
    file.read(headerBuf, readSize);
    memcpy(m_mappedBase, headerBuf, readSize);
    delete[] headerBuf;
    
    // Clear any fail/eof bits if we hit end of file while reading header
    file.clear();

    // 2. Map Sections
    for (const auto& section : info.sections) {
        if (section.rawDataSize == 0) continue;

        void* sectionAddr = (char*)m_mappedBase + section.virtualAddress;
        if (!MemoryManager::commit(sectionAddr, section.virtualSize, PROT_READ | PROT_WRITE)) {
            printf("[ProWin] PEImage ERROR: Failed to commit section %s\n", section.name.c_str());
            return false;
        }
        
        file.seekg(section.rawDataPtr, std::ios::beg);
        file.read((char*)sectionAddr, section.rawDataSize);
        
        if (file.fail()) {
            printf("[ProWin] PEImage ERROR: Failed to read section data for %s\n", section.name.c_str());
            return false;
        }

        // Zero-fill remaining virtual size if any
        if (section.virtualSize > section.rawDataSize) {
            memset((char*)sectionAddr + section.rawDataSize, 0, section.virtualSize - section.rawDataSize);
        }
        
        printf("[ProWin] PEImage: Mapped section %s to 0x%p (size: 0x%x)\n", 
               section.name.c_str(), sectionAddr, (unsigned int)section.rawDataSize);
    }

    // 3. Apply Relocations if delta != 0 (ASLR)
    if (delta != 0 && m_relocRVA != 0) {
        applyRelocations(delta);
    }

    // 3.5 Resolve Imports
    if (!resolveImports()) {
        printf("[ProWin] PEImage ERROR: Failed to resolve imports\n");
        return false;
    }

    // 4. Set Final Protections per section
    // MemoryManager initially commits sections as RW to allow loading and relocations.
    // Now we apply the actual protections from the PE header.
    for (const auto& section : info.sections) {
        if (section.virtualSize == 0) continue;
        
        void* sectionAddr = (char*)m_mappedBase + section.virtualAddress;
        int prot = peCharsToPosixProt(section.characteristics);
        
        // Ensure address and size are page-aligned for mprotect?
        // MemoryManager::reserve/commit usually handles this, but let's be careful.
        if (mprotect(sectionAddr, section.virtualSize, prot) != 0) {
            printf("[PEImage] WARNING: mprotect failed for section %s: %s\n", 
                   section.name.c_str(), strerror(errno));
        } else {
            printf("[PEImage] Section %s -> prot=0x%x\n", section.name.c_str(), prot);
        }
    }

    m_entryPoint = (uint64_t)((char*)m_mappedBase + info.entryPointRVA);
    m_filePath = filePath;

    printf("[ProWin] PEImage: Load complete. EntryPoint: 0x%llx. Byte at entry: 0x%02X\n", 
           m_entryPoint, *(uint8_t*)m_entryPoint);
    fflush(stdout);

    return true;
}

bool PEImage::resolveImports() {
    if (m_importRVA == 0) return true;

    IMAGE_IMPORT_DESCRIPTOR* importDesc = (IMAGE_IMPORT_DESCRIPTOR*)((char*)m_mappedBase + m_importRVA);
    int dllCount = 0;
    int importCount = 0;

    while (importDesc->Name != 0) {
        const char* dllName = (const char*)m_mappedBase + importDesc->Name;
        dllCount++;

        // OriginalFirstThunk (INT) and FirstThunk (IAT)
        uint32_t intRVA = importDesc->DUMMYUNIONNAME.OriginalFirstThunk;
        uint32_t iatRVA = importDesc->FirstThunk;

        // Fallback if no INT
        if (intRVA == 0) intRVA = iatRVA;

        IMAGE_THUNK_DATA64* thunkINT = (IMAGE_THUNK_DATA64*)((char*)m_mappedBase + intRVA);
        IMAGE_THUNK_DATA64* thunkIAT = (IMAGE_THUNK_DATA64*)((char*)m_mappedBase + iatRVA);

        while (thunkINT->u1.AddressOfData != 0) {
            importCount++;
            if (!(thunkINT->u1.Ordinal & IMAGE_ORDINAL_FLAG64)) {
                IMAGE_IMPORT_BY_NAME* importByName = (IMAGE_IMPORT_BY_NAME*)((char*)m_mappedBase + thunkINT->u1.AddressOfData);
                const char* funcName = (const char*)importByName->Name;
                
                void* stubAddr = StubManager::getInstance().getOrCreateStub(dllName, funcName);
                thunkIAT->u1.Function = (uint64_t)stubAddr;
            } else {
                uint16_t ordinal = (uint16_t)(thunkINT->u1.Ordinal & 0xFFFF);
                // For ordinals, we'd need a different StubManager lookup
                // For now, just null stub
                thunkIAT->u1.Function = 0;
                printf("[PEImage] Warning: Import by ordinal %d from %s set to NULL\n", ordinal, dllName);
            }
            
            thunkINT++;
            thunkIAT++;
        }
        
        importDesc++;
    }

    printf("[PEImage] IAT Resolution: %d entries across %d DLLs\n", importCount, dllCount);
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
