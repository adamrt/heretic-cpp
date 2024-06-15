#pragma once

#include <iostream>
#include <memory>
#include <random>
#include <vector>

#include "Mesh.h"
#include "Texture.h"
#include "shader.glsl.h"

#include "glm/ext/matrix_transform.hpp"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "sokol_gfx.h"

class Model {
public:
    Model(glm::vec3 position)
        : translation(position) {};

    virtual auto render() -> void = 0;
    auto update(float delta_time) -> void;

    glm::vec3 scale = { 1.0f, 1.0f, 1.0f };
    glm::vec3 translation = { 0.0f, 0.0f, 0.0f };
    glm::vec3 rotation = { 0.0f, 0.0f, 0.0f };
    glm::mat4 model_matrix = glm::mat4(1.0f);

    // Rendering
    std::shared_ptr<Mesh> mesh = nullptr;
    sg_pipeline pipeline = {};
    sg_bindings bindings = {};
};

class TexturedModel : public Model {
public:
    TexturedModel(std::shared_ptr<Mesh> _mesh, std::shared_ptr<Texture> _texture, glm::vec3 _position = { 0.0f, 0.0f, 0.0f });

    auto render() -> void override;

    std::shared_ptr<Texture> texture = nullptr;
};

class PalettedModel : public Model {
public:
    PalettedModel(std::shared_ptr<Mesh> _mesh, std::shared_ptr<Texture> _texture, std::shared_ptr<Texture> _palette, glm::vec3 _position = { 0.0f, 0.0f, 0.0f });

    auto render() -> void override;

    std::shared_ptr<Texture> texture = nullptr;
    std::shared_ptr<Texture> palette = nullptr;
};

class ColoredModel : public Model {
public:
    ColoredModel(std::shared_ptr<Mesh> _mesh, glm::vec4 _color = { 1.0f, 1.0f, 1.0f, 1.0f }, glm::vec3 _position = { 0.0f, 0.0f, 0.0f });

    auto render() -> void override;

    glm::vec4 color = {};
};

// Light is just a ColoredModel with a smaller scale. It always uses the cube
// mesh, so we might want to implicitly provide it in the future.
class Light : public ColoredModel {
public:
    Light(std::shared_ptr<Mesh> _mesh, glm::vec4 _color, glm::vec3 _position = { 0.0f, 0.0f, 0.0f });

    bool is_valid() { return color.x + color.y + color.z > 0.0f; }
};
