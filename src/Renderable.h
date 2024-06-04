#pragma once

#include <memory>
#include <random>
#include <vector>

#include "Mesh.h"

#include "sokol_gfx.h"

#include "glm/ext/matrix_transform.hpp"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

#include "shader.glsl.h"

float random_float(float min, float max);
std::vector<float> parse_obj(const std::string filename);

class Renderable {
public:
    Renderable(std::shared_ptr<Mesh> resources, sg_pipeline& pip);

    auto render(const glm::mat4& view_proj) -> void;
    auto update(float delta_time, float rotation_speed) -> void;
    auto set_model_matrix(const glm::mat4& matrix) -> void;
    auto translate(const glm::vec3& translation) -> void;
    auto rotate(float angle, const glm::vec3& axis) -> void;
    auto scale(const glm::vec3& scaling_factors) -> void;

private:
    std::shared_ptr<Mesh> shared_resources;
    sg_pipeline pipeline = {};
    sg_bindings bindings = {};
    glm::mat4 model_matrix;
    float rspeed;
};
