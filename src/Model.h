#pragma once

#include <iostream>
#include <memory>
#include <random>
#include <vector>

#include "Mesh.h"
#include "Texture.h"

#include "sokol_gfx.h"

#include "glm/ext/matrix_transform.hpp"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

#include "shader.glsl.h"

class Model {
public:
    Model(std::shared_ptr<Mesh> mesh, std::shared_ptr<Texture>);
    Model(std::shared_ptr<Mesh> _mesh);
    Model() = delete;
    ~Model();

    virtual auto render() -> void;
    virtual auto update(float delta_time) -> void;

    glm::vec3 scale = { 1.0f, 1.0f, 1.0f };
    glm::vec3 translation = { 0.0f, 0.0f, 0.0f };
    glm::vec3 rotation = { 0.0f, 0.0f, 0.0f };
    glm::mat4 model_matrix = glm::mat4(1.0f);

    glm::vec4 color = { 1.0f, 1.0f, 1.0f, 1.0f };

    // Rendering
    std::shared_ptr<Mesh> mesh;
    std::shared_ptr<Texture> texture;
    sg_shader shader = {};
    sg_pipeline pipeline = {};
    sg_bindings bindings = {};
};
