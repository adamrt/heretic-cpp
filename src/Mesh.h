#pragma once

#include <string>
#include <vector>

#include "glm/glm.hpp"
#include "sokol_gfx.h"

struct Vertex {
    glm::vec3 position = {};
    glm::vec3 normal = {};
    glm::vec2 tex_coords = {};
};

class Mesh {
public:
    Mesh(std::string filename);
    Mesh(std::vector<Vertex> vertices);
    ~Mesh()
    {
        sg_destroy_buffer(vertex_buffer);
    }

    glm::vec3 normalized_scale() const;
    glm::vec3 center_translation() const;

    std::vector<Vertex> parse_obj(const std::string filename);

    sg_buffer vertex_buffer = {};
    std::vector<Vertex> vertices = {};
};
