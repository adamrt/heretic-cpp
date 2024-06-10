#pragma once

#include "glm/glm.hpp"
#include "sokol_app.h"

enum class Projection : int {
    Perspective = 0,
    Orthographic = 1,
};

class Camera {
public:
    static constexpr float NEARZ = 0.01f;
    static constexpr float FARZ = 100.0f;
    static constexpr float MIN_DIST = 0.5f;
    static constexpr float MAX_DIST = 100.0f;
    static constexpr float MIN_LAT = -85.0f;
    static constexpr float MAX_LAT = 85.0f;

    Projection projection = Projection::Perspective;

    auto update() -> void;
    auto orbit(float dx, float dy) -> void;
    auto zoom(float d) -> void;
    auto view_proj() const -> glm::mat4 { return _proj * _view; }

private:
    auto _euclidean(float latitude, float longitude) -> glm::vec3;

    float _fov = 60.0f;
    float _distance = 3.0f;
    float _latitude = 30.0f;
    float _longitude = 30.0f;
    glm::vec3 _eye = {};
    glm::vec3 _target = {};
    glm::mat4 _view = {};
    glm::mat4 _proj = {};
};
