#pragma once

#include <string>

#include "sokol_gfx.h"
#include "stb_image.h"

struct Texture {
    Texture(std::string filename);
    ~Texture();

    sg_image load_png(const char* filename);

    sg_image image = {};
    sg_sampler sampler = {};
};
