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

    Projection projection = Projection::Orthographic;

    auto reset() -> void;
    auto update() -> void;
    auto orbit(float dx, float dy) -> void;
    auto pan(float dx, float dy) -> void;
    auto zoom(float d) -> void;
    auto view_proj() const -> glm::mat4 { return _proj * _view; }

private:
    static constexpr glm::vec3 default_target = { 0.0f, 0.0f, 0.0f };
    static constexpr float default_fov = 60.0f;
    static constexpr float default_distance = 2.0f;
    static constexpr float default_latitude = 30.0f;
    static constexpr float default_longitude = 30.0f;

    auto _euclidean(float latitude, float longitude) -> glm::vec3;

    float _fov = default_fov;
    float _distance = default_distance;
    float _latitude = default_latitude;
    float _longitude = default_longitude;

    glm::vec3 _eye = {};
    glm::vec3 _target = {};
    glm::mat4 _view = {};
    glm::mat4 _proj = {};
};
