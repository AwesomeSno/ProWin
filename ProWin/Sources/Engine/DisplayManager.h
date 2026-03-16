#ifndef DisplayManager_h
#define DisplayManager_h

#include <cstdint>
#include <vector>

namespace ProWin {

class DisplayManager {
public:
    static DisplayManager& getInstance();

    void initialize(int width, int height);
    void* getVRAM() { return m_vram; }
    size_t getVRAMSize() { return m_width * m_height * 4; }
    
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
