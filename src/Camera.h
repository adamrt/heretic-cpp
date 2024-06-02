#pragma once

#include "sokol_app.h"

#include "glm/glm.hpp"

enum class Projection : int {
    Perspective = 0,
    Orthographic = 1,
};

class Camera {
public:
    static constexpr float NEARZ = 0.01f;
    static constexpr float FARZ = 100.0f;
    static constexpr float MIN_DIST = 2.5f;
    static constexpr float MAX_DIST = 100.0f;
    static constexpr float MIN_LAT = -85.0f;
    static constexpr float MAX_LAT = 85.0f;

    Projection projection = Projection::Perspective;

    auto handle_event(const sapp_event* ev) -> void;
    auto update() -> void;

    auto view_matrix() const -> glm::mat4 { return _view; }
    auto proj_matrix() const -> glm::mat4 { return _proj; }

private:
    auto orbit(float dx, float dy) -> void;
    auto zoom(float d) -> void;
    auto euclidean(float latitude, float longitude) -> glm::vec3;

    float _fov = 60.0f;
    float _distance = 5.0f;
    float _latitude = 30.f;
    float _longitude = 30.0f;
    glm::vec3 _eye;
    glm::vec3 _target;
    glm::mat4 _view;
    glm::mat4 _proj;
};
