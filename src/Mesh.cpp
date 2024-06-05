#include <cstdio>
#include <cstring>
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
    FILE* file;
    file = fopen(filename.c_str(), "r");
    char line[1024];

    std::vector<glm::vec3> vertices;
    std::vector<glm::vec2> uvs;
    std::vector<glm::vec3> normals;
    std::vector<uint16_t> indices;

    std::vector<float> results = {};

    while (fgets(line, 1024, file)) {
        // Vertex information
        if (strncmp(line, "v ", 2) == 0) {
            glm::vec3 v;
            sscanf(line, "v %f %f %f", &v.x, &v.y, &v.z);
            vertices.push_back(v);
        }
        if (strncmp(line, "vn ", 3) == 0) {
            glm::vec3 v;
            sscanf(line, "vn %f %f %f", &v.x, &v.y, &v.z);
            normals.push_back(v);
        }
        // Texture coordinate information
        if (strncmp(line, "vt ", 3) == 0) {
            glm::vec2 uv;
            sscanf(line, "vt %f %f", &uv.x, &uv.y);
            uvs.push_back(uv);
        }
        // Face information
        if (strncmp(line, "f ", 2) == 0) {
            unsigned int vertex_indices[3];
            unsigned int uv_indices[3];
            unsigned int normal_indices[3];
            sscanf(
                line, "f %u/%u/%u %u/%u/%u %u/%u/%u",
                &vertex_indices[0], &uv_indices[0], &normal_indices[0],
                &vertex_indices[1], &uv_indices[1], &normal_indices[1],
                &vertex_indices[2], &uv_indices[2], &normal_indices[2]);

            for (int i = 0; i < 3; i++) {
                auto v = vertices[vertex_indices[i] - 1];
                auto n = normals[normal_indices[i] - 1];
                auto u = uvs[uv_indices[i] - 1];

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
        };
    };
    fclose(file);
    return results;
}
