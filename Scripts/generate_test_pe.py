import struct

def generate_minimal_pe(filename):
    # DOS Header
    mz_header = struct.pack('<2s', b'MZ') + b'\x00' * 58 + struct.pack('<I', 0x40)
    
    # PE Header
    pe_sig = b'PE\x00\x00'
    file_header = struct.pack('<HHIIIHH', 
        0x8664,  # Machine: x64
        2,       # NumberOfSections
        0,       # TimeDateStamp
        0,       # PointerToSymbolTable
        0,       # NumberOfSymbols
        0xF0,    # SizeOfOptionalHeader
        0x0022   # Characteristics: EXECUTABLE_IMAGE | LARGE_ADDRESS_AWARE
    )
    
    # Optional Header (PE32+)
    # Standard fields (24 bytes)
    std_fields = struct.pack('<HBBIIIII',
        0x020B,  # Magic: PE32+
        0, 0,    # Major/MinorLinker
        0x1000,  # SizeOfCode
        0x1000,  # SizeOfInitializedData
        0,       # SizeOfUninitializedData
        0x1000,  # AddressOfEntryPoint (RVA)
        0x1000,  # BaseOfCode
    )
    
    # Windows fields (88 bytes) - corrected format for 21 items
    win_fields = struct.pack('<QIIHHHHHHIIIIHHQQQQII',
        0x00400000, # ImageBase
        0x1000,  # SectionAlignment
        0x200,   # FileAlignment
        0, 0,    # OSVer
        0, 0,    # ImageVer
        0, 0,    # SubsystemVer
        0,       # Win32Version
        0x3000,  # SizeOfImage
        0x400,   # SizeOfHeaders
        0,       # CheckSum
        3,       # Subsystem: WINDOWS_CUI
        0,       # DllCharacteristics
        0x100000, 0x1000, # Stack
        0x100000, 0x1000, # Heap
        0x00,    # LoaderFlags
        16       # NumberOfRvaAndSizes
    )
    
    # Data Directories (Empty)
    data_dirs = b'\x00' * (16 * 8)
    
    # Section Headers
    # .text
    text_section = struct.pack('<8sIIIIIIHHI',
        b'.text\x00\x00\x00',
        0x1000, # VirtualSize
        0x1000, # VirtualAddress
        0x200,  # SizeOfRawData
        0x400,  # PointerToRawData
        0, 0, 0, 0,
        0x60000020 # Characteristics: CODE | EXECUTE | READ
    )
    # .data
    data_section = struct.pack('<8sIIIIIIHHI',
        b'.data\x00\x00\x00',
        0x1000, # VirtualSize
        0x2000, # VirtualAddress
        0x200,  # SizeOfRawData
        0x600,  # PointerToRawData
        0, 0, 0, 0,
        0xC0000040 # Characteristics: INITIALIZED_DATA | READ | WRITE
    )
    
    with open(filename, 'wb') as f:
        f.write(mz_header)
        f.write(pe_sig)
        f.write(file_header)
        f.write(std_fields)
        f.write(win_fields)
        f.write(data_dirs)
        f.write(text_section)
        f.write(data_section)
        f.write(b'\x00' * (0x400 - f.tell())) # Padding to raw data
        
        # x64 Assembly to fill a 100x100 blue square
        # RDI is pre-loaded with VRAM Address by the engine
        # MOV EAX, 0xFFFF0000 (Blue in BGRA)
        # MOV RCX, 100 (height)
        # OuterLoop:
        #   MOV RDX, 100 (width)
        #   InnerLoop:
        #     STOSD (Store EAX at [RDI], then RDI += 4)
        #Dec RDX
        #     JNZ InnerLoop
        #   ADD RDI, (800-100)*4 (Skip to next line)
        #   DEC RCX
        #   JNZ OuterLoop
        # RET
        
        # Binary machine code:
        code = b''
        code += b'\xB8\xFF\x00\x00\xFF'                   # MOV EAX, 0xFF0000FF (Blue)
        code += b'\x48\xC7\xC1\x64\x00\x00\x00'           # MOV RCX, 100 (Outer count)
        
        # Outer loop start (offset 0x0C)
        code += b'\x48\xC7\xC2\x64\x00\x00\x00'           # MOV RDX, 100 (Inner count)
        
        # Inner loop start (offset 0x13 = 19)
        code += b'\xAB'                                   # STOSD (offset 19)
        code += b'\x48\xFF\xCA'                           # DEC RDX (offset 20)
        code += b'\x75\xFA'                               # JNZ Inner (offset 23, jump -6 to 19)
        
        # Outer loop logic
        code += b'\x48\x81\xC7\xF0\x0A\x00\x00'           # ADD RDI, 2800 (offset 25)
        code += b'\x48\xFF\xC9'                           # DEC RCX (offset 32)
        code += b'\x75\xE7'                               # JNZ Outer (offset 35, jump -25 to 12)
        
        code += b'\xC3'                                   # RET (offset 37)
        
        f.write(code)
        f.write(b'\x90' * (0x200 - len(code)))
        f.write(b'\x00' * 0x200) # .data raw data

if __name__ == '__main__':
    generate_minimal_pe('/Users/harinandanjv/Documents/ProWIn/test.exe')
