#include "Renderable.h"
#include <cstdio>
#include <cstring>
#include <utility>

float random_float(float min, float max)
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(min, max);
    return dis(gen);
}

MeshResources::MeshResources(const std::vector<float> vertices, const std::vector<uint16_t> indices)
{
    sg_buffer_desc vbuf_desc = {};
    vbuf_desc.data = sg_range { vertices.data(), vertices.size() * sizeof(float) };
    vbuf_desc.label = "vertex-buffer";
    vertex_buffer = sg_make_buffer(&vbuf_desc);

    sg_buffer_desc ibuf_desc = {};
    ibuf_desc.type = SG_BUFFERTYPE_INDEXBUFFER;
    ibuf_desc.data = sg_range { indices.data(), indices.size() * sizeof(uint16_t) };
    ibuf_desc.label = "index-buffer";
    index_buffer = sg_make_buffer(&ibuf_desc);

    num_indices = indices.size();
}

Renderable::Renderable(std::shared_ptr<MeshResources> resources, sg_pipeline& pip)
    : shared_resources(resources)
    , pipeline(pip)
    , model_matrix(1.0f)
{
    bindings.vertex_buffers[0] = resources->vertex_buffer;
    bindings.index_buffer = resources->index_buffer;

    rspeed = random_float(-5.0f, 5.0f);
}

auto Renderable::render(const glm::mat4& view_proj) -> void
{
    glm::mat4 mvp = view_proj * model_matrix;
    vs_params_t vs_params;
    vs_params.mvp = mvp;

    sg_apply_pipeline(pipeline);
    sg_apply_bindings(&bindings);

    sg_range vs_range = SG_RANGE(vs_params);
    sg_apply_uniforms(SG_SHADERSTAGE_VS, SLOT_vs_params, &vs_range);
    sg_draw(0, shared_resources->num_indices, 1);
}

auto Renderable::update(float delta_time, float rotation_speed) -> void
{
    auto x = 1.0f * delta_time * rspeed * rotation_speed;
    auto y = 2.0f * delta_time * rspeed * rotation_speed;

    rotate(x, glm::vec3(1.0f, 0.0f, 0.0f));
    rotate(y, glm::vec3(0.0f, 1.0f, 0.0f));
}

auto Renderable::set_model_matrix(const glm::mat4& matrix) -> void
{
    model_matrix = matrix;
}

auto Renderable::translate(const glm::vec3& translation) -> void
{
    model_matrix = glm::translate(model_matrix, translation);
}

auto Renderable::rotate(float angle, const glm::vec3& axis) -> void
{
    model_matrix = glm::rotate(model_matrix, angle, axis);
}

auto Renderable::scale(const glm::vec3& scaling_factors) -> void
{
    model_matrix = glm::scale(model_matrix, scaling_factors);
}

std::pair<std::vector<float>, std::vector<uint16_t>> parse_obj(const std::string filename)
{

    FILE* file;
    file = fopen(filename.c_str(), "r");
    char line[1024];

    std::vector<glm::vec2> uvs;
    std::vector<float> vertices;
    std::vector<uint16_t> indices;

    while (fgets(line, 1024, file)) {
        // Vertex information
        if (strncmp(line, "v ", 2) == 0) {
            float x, y, z;
            sscanf(line, "v %f %f %f", &x, &y, &z);
            vertices.push_back(x);
            vertices.push_back(y);
            vertices.push_back(z);
            vertices.push_back(1.0f);
            vertices.push_back(1.0f);
            vertices.push_back(1.0f);
            vertices.push_back(1.0f);
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
            unsigned int texture_indices[3];
            unsigned int normal_indices[3];
            sscanf(
                line, "f %u/%u/%u %u/%u/%u %u/%u/%u",
                &vertex_indices[0], &texture_indices[0], &normal_indices[0],
                &vertex_indices[1], &texture_indices[1], &normal_indices[1],
                &vertex_indices[2], &texture_indices[2], &normal_indices[2]);

            indices.push_back(vertex_indices[0] - 1);
            indices.push_back(vertex_indices[1] - 1);
            indices.push_back(vertex_indices[2] - 1);
        };
    };
    fclose(file);
    return { vertices, indices };
}
