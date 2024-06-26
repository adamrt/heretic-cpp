#pragma once

#include <array>
#include <cstdint>
#include <string>

#include "sokol_gfx.h"
#include "stb_image.h"

// FFT Texture dimensions are always 256 * 256 * 4 pages (256*1024).
constexpr int FFT_TEXTURE_WIDTH = 256;
constexpr int FFT_TEXTURE_HEIGHT = 1024;
constexpr int FFT_TEXTURE_NUM_PIXELS = (256 * 1024);                // 262144
constexpr int FFT_TEXTURE_NUM_BYTES = (FFT_TEXTURE_NUM_PIXELS * 4); // Pixel * 4 bytes per pixel (RGBA8).
constexpr int FFT_TEXTURE_RAW_SIZE = (FFT_TEXTURE_NUM_PIXELS / 2);  // Each pixel on disk 1/2 a byte.

// FFT Palette dimensions are always 256 * 1.
constexpr int FFT_PALETTE_NUM_PIXELS = (16 * 16);
constexpr int FFT_PALETTE_NUM_BYTES = (FFT_PALETTE_NUM_PIXELS * 4);

struct Texture {
    Texture(std::string filename);
    Texture(std::array<uint8_t, FFT_TEXTURE_NUM_BYTES> data);
    Texture(std::array<uint8_t, FFT_PALETTE_NUM_BYTES> data);
    ~Texture();

    sg_image load_png(const char* filename);

    sg_image image = {};
    sg_sampler sampler = {};
};
