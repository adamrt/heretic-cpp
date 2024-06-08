#pragma once

#include <string>
#include <vector>

#include "glm/glm.hpp"
#include "sokol_gfx.h"

struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 tex_coords;
};

class Mesh {
public:
    Mesh(std::string filename);

    std::vector<Vertex> parse_obj(const std::string filename);

    sg_buffer vertex_buffer;
    int num_vertices;
};
