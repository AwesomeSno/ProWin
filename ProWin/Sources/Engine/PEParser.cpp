#include "PEParser.h"
#include <fstream>
#include <iostream>

namespace ProWin {

PEParser::PEInfo PEParser::getInfo(const std::string& filePath) {
    PEInfo info = {false, false, 0, 0, 0};
    
    std::cout << "[ProWin] PEParser: Attempting to open " << filePath << std::endl;
    std::ifstream file(filePath, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "[ProWin] PEParser: Failed to open file (Permissions/Sandbox?)" << std::endl;
        return info;
    }
    
    IMAGE_DOS_HEADER dosHeader;
    file.read(reinterpret_cast<char*>(&dosHeader), sizeof(dosHeader));
    
    if (dosHeader.e_magic != 0x5A4D) { // 'MZ'
        std::cerr << "[ProWin] PEParser: Invalid DOS Magic: 0x" << std::hex << dosHeader.e_magic << std::endl;
        return info;
    }
    
    file.seekg(dosHeader.e_lfanew, std::ios::beg);
    
    uint32_t peSignature;
    file.read(reinterpret_cast<char*>(&peSignature), sizeof(peSignature));
    
    if (peSignature != 0x00004550) { // 'PE\0\0'
        return info;
    }
    
    IMAGE_FILE_HEADER fileHeader;
    file.read(reinterpret_cast<char*>(&fileHeader), sizeof(fileHeader));
    
    // Check if it's 64-bit (PE32+)
    uint16_t optionalHeaderMagic;
    file.read(reinterpret_cast<char*>(&optionalHeaderMagic), sizeof(optionalHeaderMagic));
    
    if (optionalHeaderMagic == 0x020B) { // PE32+
        info.is64Bit = true;
        // Re-read or seek to get full optional header
        file.seekg(dosHeader.e_lfanew + sizeof(uint32_t) + sizeof(IMAGE_FILE_HEADER), std::ios::beg);
        IMAGE_OPTIONAL_HEADER64 opt64;
        file.read(reinterpret_cast<char*>(&opt64), sizeof(opt64));
        
        info.imageBase = opt64.ImageBase;
        info.entryPointRVA = opt64.AddressOfEntryPoint;
        info.sizeOfImage = opt64.SizeOfImage;
        info.importDirectoryRVA = opt64.DataDirectory[1].VirtualAddress;
        info.importDirectorySize = opt64.DataDirectory[1].Size;
        info.relocationDirectoryRVA = opt64.DataDirectory[5].VirtualAddress;
        info.relocationDirectorySize = opt64.DataDirectory[5].Size;
        info.isValid = true;
        
        // Parse Sections
        file.seekg(dosHeader.e_lfanew + sizeof(uint32_t) + sizeof(IMAGE_FILE_HEADER) + fileHeader.SizeOfOptionalHeader, std::ios::beg);
        
        for (int i = 0; i < fileHeader.NumberOfSections; ++i) {
            IMAGE_SECTION_HEADER sectionHeader;
            file.read(reinterpret_cast<char*>(&sectionHeader), sizeof(sectionHeader));
            
            Section section;
            char name[9] = {0};
            memcpy(name, sectionHeader.Name, 8);
            section.name = name;
            section.virtualAddress = sectionHeader.VirtualAddress;
            section.virtualSize = sectionHeader.Misc.VirtualSize;
            section.rawDataSize = sectionHeader.SizeOfRawData;
            section.rawDataPtr = sectionHeader.PointerToRawData;
            section.characteristics = sectionHeader.Characteristics;
            
            info.sections.push_back(section);
        }
    } else if (optionalHeaderMagic == 0x010B) { // PE32
        info.is64Bit = false;
        // TODO: Implement PE32 (32-bit) parsing
        // For now just mark as valid but base is different
        info.isValid = true;
    }
    
    return info;
}

}
