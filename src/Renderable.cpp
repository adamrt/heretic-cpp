#include <cstdio>
#include <cstring>
#include <utility>

#include "Renderable.h"

float random_float(float min, float max)
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(min, max);
    return dis(gen);
}

Renderable::Renderable(std::shared_ptr<Mesh> resources, sg_pipeline& pip, sg_image _image, sg_sampler _sampler, State& _state)
    : state(_state)
    , shared_resources(resources)
    , pipeline(pip)
    , image(_image)
    , sampler(_sampler)
    , model_matrix(1.0f)
{
    bindings.vertex_buffers[0] = resources->vertex_buffer;

    rspeed = random_float(-5.0f, 5.0f);
}

auto Renderable::render(const glm::mat4& view_proj) -> void
{
    vs_standard_params_t vs_params;
    vs_params.u_view_proj = view_proj;
    vs_params.u_model = model_matrix;

    fs_standard_params_t fs_params;
    fs_params.u_ambient_color = glm::vec4 { 0.1f, 0.1f, 0.1f, 1.0f };

    fs_light_params_t light_params;
    light_params.color[0] = state.lights[0].color;
    light_params.color[1] = state.lights[1].color;
    light_params.color[2] = state.lights[2].color;
    light_params.position[0] = state.lights[0].position;
    light_params.position[1] = state.lights[1].position;
    light_params.position[2] = state.lights[2].position;

    bindings.fs.images[SLOT_tex] = image;
    bindings.fs.samplers[SLOT_smp] = sampler;

    sg_apply_pipeline(pipeline);
    sg_apply_bindings(&bindings);

    sg_range vs_range = SG_RANGE(vs_params);
    sg_range fs_range = SG_RANGE(fs_params);
    sg_range light_range = SG_RANGE(light_params);
    sg_apply_uniforms(SG_SHADERSTAGE_VS, SLOT_vs_standard_params, &vs_range);
    sg_apply_uniforms(SG_SHADERSTAGE_FS, SLOT_fs_standard_params, &fs_range);
    sg_apply_uniforms(SG_SHADERSTAGE_FS, SLOT_fs_light_params, &light_range);
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

std::vector<float> parse_obj(const std::string filename)
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
