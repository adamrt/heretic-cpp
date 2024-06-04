#pragma once

#include <vector>

#include "sokol_gfx.h"

class Mesh {
public:
    Mesh(const std::vector<float> vertices);

    sg_buffer vertex_buffer;
    int num_indices;
};
