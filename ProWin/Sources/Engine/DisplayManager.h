#ifndef DisplayManager_h
#define DisplayManager_h

#include <cstdint>
#include <vector>
#include "EngineConstants.h"

namespace ProWin {

class DisplayManager {
public:
    static DisplayManager& getInstance();

    void initialize(int width, int height);
    void* getVRAM() { return m_vram; }
    size_t getVRAMSize() { return (size_t)m_width * m_height * PROWIN_VRAM_BPP; }
    
    int getWidth() const { return m_width; }
    int getHeight() const { return m_height; }

private:
    DisplayManager();
    ~DisplayManager();

    int m_width;
    int m_height;
    void* m_vram;
};

}

#endif /* DisplayManager_h */
