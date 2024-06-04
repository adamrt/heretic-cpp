#pragma once

#include <vector>

#include "sokol_gfx.h"

class Mesh {
public:
    Mesh(const std::vector<float> vertices, const std::vector<uint16_t> indices);

    sg_buffer vertex_buffer;
    sg_buffer index_buffer;
    int num_indices;
};
