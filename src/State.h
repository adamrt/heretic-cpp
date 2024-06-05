#pragma once

#include <array>

#include "sokol_gfx.h"

#include "Camera.h"
#include "Light.h"

struct State {
    Camera camera;
    float rotation_speed = 0.0f;
    sg_color clear_color = { 0.0f, 0.5f, 0.7f, 1.0f };

    glm::vec4 ambient_color = { 1.0, 1.0, 1.0, 1.0f };
    float ambient_strength = 0.2f;

    std::array<Light, 3> lights = {
        Light { glm::vec4 { 100.0f, 0.0f, 0.0f, 0.0f }, glm::vec4 { 0.0f, 0.0f, 1.0f, 1.0f } },
        Light { glm::vec4 { 0.0f, 100.0f, 0.0f, 0.0f }, glm::vec4 { 1.0f, 0.0f, 0.0f, 1.0f } },
        Light { glm::vec4 { 0.0f, 0.0f, 100.0f, 0.0f }, glm::vec4 { 0.5f, 0.5f, 0.5f, 1.0f } },
    };
};
