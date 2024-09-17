#pragma once

#include <string>
#include <vector>

#include "glm/glm.hpp"
#include "sokol_gfx.h"

struct Vertex {
    glm::vec3 position = {};
    glm::vec3 normal = {};
    glm::vec2 tex_coords = {};
    float palette_index = {};
};

class Mesh {
public:
    Mesh(std::string filename);
    Mesh(std::vector<Vertex> vertices);
    Mesh(std::vector<glm::vec3> vertices);
    ~Mesh() { sg_destroy_buffer(vertex_buffer); }

    auto center_translation() const -> glm::vec3;

public:
    std::vector<Vertex> vertices = {};
    std::vector<glm::vec3> vertices_float = {};
    sg_buffer vertex_buffer = {};

private:
    auto parse_obj(const std::string filename) -> std::vector<Vertex>;
    auto normalized_scale() const -> glm::vec3;
};
