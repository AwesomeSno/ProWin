#pragma once
// Single source of truth for all engine-level constants
// This file is parsed by generate_test_pe.py — do NOT rename constants
#define PROWIN_VRAM_WIDTH   800
#define PROWIN_VRAM_HEIGHT  600
#define PROWIN_VRAM_BPP     4       // Bytes per pixel (BGRA8)
#define PROWIN_VRAM_STRIDE  (PROWIN_VRAM_WIDTH * PROWIN_VRAM_BPP)
