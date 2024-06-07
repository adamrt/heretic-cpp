#include <algorithm>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <string>
#include <utility>

#include "glm/glm.hpp"

#include "Mesh.h"

// pos3 + norm3 + uv2 + color4
const int vertex_size = 12;

Mesh::Mesh(std::string filename)
{
    auto vertices = parse_obj(filename);

    sg_buffer_desc vbuf_desc = {};
    vbuf_desc.data = sg_range { vertices.data(), vertices.size() * sizeof(float) };
    vbuf_desc.label = "vertex-buffer";
    vertex_buffer = sg_make_buffer(&vbuf_desc);

    num_indices = vertices.size() / vertex_size;
}

std::vector<float> Mesh::parse_obj(const std::string filename)
{
    FILE* file = fopen(filename.c_str(), "r");
    if (!file) {
        std::cerr << "Error opening file: " << filename << std::endl;
        return {};
    }
    std::vector<float> results;
    std::vector<glm::vec3> positions;
    std::vector<glm::vec3> normals;
    std::vector<glm::vec2> texcoords;

    char line[1024];

    while (fgets(line, 1024, file)) {
        // Vertex information
        if (strncmp(line, "v ", 2) == 0) {
            glm::vec3 position {};
            if (sscanf(line, "v %f %f %f", &position.x, &position.y, &position.z) != 3) {
                std::cerr << "Error reading position" << std::endl;
            }
            positions.push_back(position);
        }
        if (strncmp(line, "vn ", 3) == 0) {
            glm::vec3 normal {};
            if (sscanf(line, "vn %f %f %f", &normal.x, &normal.y, &normal.z) != 3) {
                std::cerr << "Error reading normal" << std::endl;
            }
            normals.push_back(normal);
        }
        if (strncmp(line, "vt ", 3) == 0) {
            glm::vec2 texcoord;
            if (sscanf(line, "vt %f %f", &texcoord.x, &texcoord.y) != 2) {
                std::cerr << "Error reading texcoord" << std::endl;
            }
            texcoords.push_back(texcoord);
        }
        if (strncmp(line, "f ", 2) == 0) {
            int count = std::count(line, line + strlen(line), '/');
            if (std::strchr(line, '/')) {
                if (count == 6) { // Triangle
                    unsigned int position_indices[3];
                    unsigned int texcoord_indices[3];
                    unsigned int normal_indices[3];

                    if (sscanf(line, "f %u/%u/%u %u/%u/%u %u/%u/%u",
                            &position_indices[0], &texcoord_indices[0], &normal_indices[0],
                            &position_indices[1], &texcoord_indices[1], &normal_indices[1],
                            &position_indices[2], &texcoord_indices[2], &normal_indices[2])
                        != 9) {
                        std::cerr << "Error reading face" << std::endl;
                    }

                    for (int i = 0; i < 3; i++) {
                        auto v = positions[position_indices[i] - 1];
                        auto n = normals[normal_indices[i] - 1];
                        auto u = texcoords[texcoord_indices[i] - 1];
                        results.push_back(v.x);
                        results.push_back(v.y);
                        results.push_back(v.z);
                        results.push_back(n.x);
                        results.push_back(n.y);
                        results.push_back(n.z);
                        results.push_back(u.x);
                        results.push_back(u.y);
                        results.push_back(1.0f);
                        results.push_back(1.0f);
                        results.push_back(1.0f);
                        results.push_back(1.0f);
                    }

                } else if (count == 8) { // Quad
                    unsigned int position_indices[4];
                    unsigned int texcoord_indices[4];
                    unsigned int normal_indices[4];
                    if (sscanf(line, "f %u/%u/%u %u/%u/%u %u/%u/%u %u/%u/%u",
                            &position_indices[0], &texcoord_indices[0], &normal_indices[0],
                            &position_indices[1], &texcoord_indices[1], &normal_indices[1],
                            &position_indices[2], &texcoord_indices[2], &normal_indices[2],
                            &position_indices[3], &texcoord_indices[3], &normal_indices[3])
                        != 12) {
                        std::cerr << "Error reading face" << std::endl;
                    }

                    for (int i : { 0, 1, 2 }) {
                        auto v = positions[position_indices[i] - 1];
                        auto n = normals[normal_indices[i] - 1];
                        auto u = texcoords[texcoord_indices[i] - 1];
                        results.push_back(v.x);
                        results.push_back(v.y);
                        results.push_back(v.z);
                        results.push_back(n.x);
                        results.push_back(n.y);
                        results.push_back(n.z);
                        results.push_back(u.x);
                        results.push_back(u.y);
                        results.push_back(1.0f);
                        results.push_back(1.0f);
                        results.push_back(1.0f);
                        results.push_back(1.0f);
                    }

                    for (int i : { 0, 2, 3 }) {
                        auto v = positions[position_indices[i] - 1];
                        auto n = normals[normal_indices[i] - 1];
                        auto u = texcoords[texcoord_indices[i] - 1];
                        results.push_back(v.x);
                        results.push_back(v.y);
                        results.push_back(v.z);
                        results.push_back(n.x);
                        results.push_back(n.y);
                        results.push_back(n.z);
                        results.push_back(u.x);
                        results.push_back(u.y);
                        results.push_back(1.0f);
                        results.push_back(1.0f);
                        results.push_back(1.0f);
                        results.push_back(1.0f);
                    }
                } else {
                    std::cerr << "Unknown face format: " << count << line << std::endl;
                }
            }
        }
    }
    fclose(file);
    return results;
}

// normalized_scale returns a glm::vec3 that can be used to scale the mesh
// to (-1, 1) on each coordinate.
// glm::vec3 Mesh::normalized_scale() const
// {
//     if (triangles.empty()) {
//         return glm::vec3(1.0f, 1.0f, 1.0f);
//     }

//     float min_x = std::numeric_limits<float>::max();
//     float max_x = std::numeric_limits<float>::min();
//     float min_y = std::numeric_limits<float>::max();
//     float max_y = std::numeric_limits<float>::min();
//     float min_z = std::numeric_limits<float>::max();
//     float max_z = std::numeric_limits<float>::min();

//     for (const auto& triangle : triangles) {
//         for (const auto& vertex : triangle.vertices) {
//             min_x = std::min(min_x, vertex.position.x);
//             max_x = std::max(max_x, vertex.position.x);
//             min_y = std::min(min_y, vertex.position.y);
//             max_y = std::max(max_y, vertex.position.y);
//             min_z = std::min(min_z, vertex.position.z);
//             max_z = std::max(max_z, vertex.position.z);
//         }
//     }

//     float size_x = max_x - min_x;
//     float size_y = max_y - min_y;
//     float size_z = max_z - min_z;

//     float largest_dimension = std::max({ size_x, size_y, size_z });
//     float scaling_factor = 2.0f / largest_dimension;

//     return glm::vec3(scaling_factor, scaling_factor, scaling_factor);
// }

// // center_translation returns a glm::vec3 that can be used to translate the
// // mesh to the center of (0, 0).
// glm::vec3 Mesh::center_translation() const
// {
//     float min_x = std::numeric_limits<float>::max();
//     float max_x = std::numeric_limits<float>::min();
//     float min_y = std::numeric_limits<float>::max();
//     float max_y = std::numeric_limits<float>::min();
//     float min_z = std::numeric_limits<float>::max();
//     float max_z = std::numeric_limits<float>::min();

//     for (auto& t : triangles) {
//         for (int i = 0; i < 3; i++) {
//             // Min
//             min_x = std::min(t.vertices[i].position.x, min_x);
//             min_y = std::min(t.vertices[i].position.y, min_y);
//             min_z = std::min(t.vertices[i].position.z, min_z);
//             // max
//             max_x = std::max(t.vertices[i].position.x, max_x);
//             max_y = std::max(t.vertices[i].position.y, max_y);
//             max_z = std::max(t.vertices[i].position.z, max_z);
//         }
//     }

//     float x = -(max_x + min_x) / 2.0;
//     float y = -(max_y + min_y) / 2.0;
//     float z = -(max_z + min_z) / 2.0;

//     return glm::vec3(x, y, z);
// }
