#include "DisplayManager.h"
#include "MemoryManager.h"
#include <cstring>
#include <cstdio>

namespace ProWin {

DisplayManager::DisplayManager() : m_width(800), m_height(600), m_vram(nullptr) {
}

DisplayManager::~DisplayManager() {
    if (m_vram) {
        MemoryManager::release(m_vram, getVRAMSize());
    }
}

DisplayManager& DisplayManager::getInstance() {
    static DisplayManager instance;
    return instance;
}

void DisplayManager::initialize(int width, int height) {
    m_width = width;
    m_height = height;
    size_t size = getVRAMSize();

    // Let the OS pick a safe address for virtual VRAM
    m_vram = MemoryManager::reserve(0, size);
    
    if (m_vram) {
        MemoryManager::commit(m_vram, size, PROT_READ | PROT_WRITE);
        // Fill with black
        memset(m_vram, 0, size);
        printf("[ProWin] Display: Initialized %dx%d VRAM at %p\n", m_width, m_height, m_vram);
    } else {
        printf("[ProWin] Display ERROR: Failed to allocate VRAM!\n");
    }
}

}
