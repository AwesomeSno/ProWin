import struct
import re
import pathlib

# Parse constants from EngineConstants.h
constants = {}
header_path = pathlib.Path("ProWin/Sources/Engine/EngineConstants.h")
if not header_path.exists():
    # Fallback for when script is run from project root or similar
    header_path = pathlib.Path(__file__).parent.parent / "ProWin/Sources/Engine/EngineConstants.h"

try:
    header_content = header_path.read_text()
    for m in re.finditer(r'#define (PROWIN_\w+)\s+(\d+)', header_content):
        constants[m.group(1)] = int(m.group(2))
except Exception as e:
    print(f"Warning: Could not parse EngineConstants.h: {e}")
    # Default values as fallback
    constants = {
        "PROWIN_VRAM_WIDTH": 800,
        "PROWIN_VRAM_HEIGHT": 600,
        "PROWIN_VRAM_BPP": 4
    }

VRAM_WIDTH = constants["PROWIN_VRAM_WIDTH"]
VRAM_HEIGHT = constants["PROWIN_VRAM_HEIGHT"]
VRAM_BPP = constants["PROWIN_VRAM_BPP"]

assert VRAM_WIDTH > 0, "Failed to parse PROWIN_VRAM_WIDTH from EngineConstants.h"

def generate_minimal_pe(filename):
    # ... (header generation remains same)
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
    std_fields = struct.pack('<HBBIIIII',
        0x020B,  # Magic: PE32+
        0, 0,    # Major/MinorLinker
        0x1000,  # SizeOfCode
        0x1000,  # SizeOfInitializedData
        0,       # SizeOfUninitializedData
        0x1000,  # AddressOfEntryPoint (RVA)
        0x1000,  # BaseOfCode
    )
    
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
    
    data_dirs = b'\x00' * (16 * 8)
    
    text_section = struct.pack('<8sIIIIIIHHI',
        b'.text\x00\x00\x00',
        0x1000, # VirtualSize
        0x1000, # VirtualAddress
        0x200,  # SizeOfRawData
        0x400,  # PointerToRawData
        0, 0, 0, 0,
        0x60000020 # Characteristics: CODE | EXECUTE | READ
    )
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
        
        # x64 Assembly to fill a 100x100 pattern square
        # RDI is pre-loaded with VRAM Address by the engine
        
        square_size = 100
        skip_bytes = (VRAM_WIDTH - square_size) * VRAM_BPP

        code = b''
        code += b'\xB8\xFF\x00\xFF\x00'                    # MOV EAX, 0x00FF00FF
        code += b'\x48\xC7\xC1' + struct.pack('<I', square_size) # MOV RCX, 100
        
        # Outer loop start
        code += b'\x48\xC7\xC2' + struct.pack('<I', square_size) # MOV RDX, 100
        
        # Inner loop start
        code += b'\xAB'                                   # STOSD
        code += b'\x48\xFF\xCA'                           # DEC RDX
        code += b'\x75\xFA'                               # JNZ Inner (-6)
        
        # Outer loop logic
        code += b'\x48\x81\xC7' + struct.pack('<I', skip_bytes) # ADD RDI, skip_bytes
        code += b'\x48\xFF\xC9'                           # DEC RCX
        code += b'\x75\xE7'                               # JNZ Outer (-25 approx, depends on skip_bytes encoding size)
        
        # Re-calculating jumps precisely
        # Inner loop jump is fixed at -6
        # Outer loop jump:
        # Start of Outer: offset 10 (after MOV EAX and MOV RCX)
        # End of Outer (before jump): offset 10 + 7 (MOV RDX) + 1 (STOSD) + 3 (DEC RDX) + 2 (JNZ Inner) + 7 (ADD RDI) + 3 (DEC RCX) = offset 33
        # Jump from offset 35 to offset 10: 10 - 35 = -25. Which is 0xE7 in int8.
        
        code += b'\xC3'                                   # RET
        
        f.write(code)
        f.write(b'\x90' * (0x200 - len(code)))
        f.write(b'\x00' * 0x200) # .data raw data

if __name__ == '__main__':
    generate_minimal_pe('/Users/harinandanjv/Documents/ProWIn/test.exe')
