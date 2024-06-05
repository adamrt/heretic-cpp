#pragma once

#include <string>
#include <vector>

#include "sokol_gfx.h"

class Mesh {
public:
    Mesh(std::string filename);

    std::vector<float> parse_obj(const std::string filename);

    sg_buffer vertex_buffer;
    int num_indices;
};
