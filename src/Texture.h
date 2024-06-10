#pragma once

#include <array>
#include <cstdint>
#include <string>

#include "sokol_gfx.h"
#include "stb_image.h"

#define TEXTURE_NUM_PIXELS 262144      // 256 * 1024
#define TEXTURE_NUM_BYTES (262144 * 4) // 256 * 1024 * 4
#define TEXTURE_WIDTH 256
#define TEXTURE_HEIGHT 1024
#define PALETTE_NUM_BYTES (16 * 16 * 4)

struct Texture {
    Texture(std::string filename);
    Texture(std::array<uint8_t, TEXTURE_NUM_BYTES> data);
    Texture(std::array<uint8_t, PALETTE_NUM_BYTES> data);
    ~Texture();

    sg_image load_png(const char* filename);

    sg_image image = {};
    sg_sampler sampler = {};
};
