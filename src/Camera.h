#pragma once

#include "glm/glm.hpp"
#include "sokol_app.h"
#include <glm/ext/matrix_clip_space.hpp>

enum class Projection : int {
    Perspective = 0,
    Orthographic = 1,
};

class OrbitalCamera {
public:
    auto reset() -> void;
    auto update() -> void;
    auto orbit(float dx, float dy) -> void;
    auto pan(float dx, float dy) -> void;
    auto zoom(float d) -> void;
    auto view_proj() const -> glm::mat4 { return _proj * _view; }

    Projection projection = Projection::Orthographic;

private:
    static constexpr float NEARZ = 0.01f;
    static constexpr float FARZ = 100.0f;
    static constexpr float MIN_DIST = 0.5f;
    static constexpr float MAX_DIST = 100.0f;
    static constexpr float MIN_LAT = -85.0f;
    static constexpr float MAX_LAT = 85.0f;

    static constexpr auto default_target = glm::vec3 { 0.0f, 0.0f, 0.0f };
    static constexpr auto default_fov = 60.0f;
    static constexpr auto default_distance = 1.0f;
    static constexpr auto default_latitude = 30.0f;
    static constexpr auto default_longitude = 30.0f;

    float _fov = default_fov;
    float _distance = default_distance;
    float _latitude = default_latitude;
    float _longitude = default_longitude;

    auto _euclidean(float latitude, float longitude) -> glm::vec3;

    glm::vec3 _eye = {};
    glm::vec3 _target = {};
    glm::mat4 _view = {};
    glm::mat4 _proj = {};
};

class FPSCamera {
public:
    static constexpr float NEARZ = 0.01f;
    static constexpr float FARZ = 100.0f;
    static constexpr float FOV = 60.0f;

    static constexpr float YAW = -90.0f;       // Facing towards negative Z-axis
    static constexpr float PITCH = 0.0f;       // Looking straight ahead
    static constexpr float SPEED = 5.0f;       // Movement speed
    static constexpr float SENSITIVITY = 0.1f; // Mouse sensitivity

    Projection projection = Projection::Perspective;

    auto update() -> void;
    auto view_proj() const -> glm::mat4 { return proj * view; }

    glm::vec3 position = { 0, 0, 0 };
    glm::mat4 view = {};
    glm::mat4 proj = {};
    glm::vec3 front;
    glm::vec3 up;
    glm::vec3 right;
    glm::vec3 worldUp = glm::vec3(0.0f, 1.0f, 0.0f);
    float ortho_scale = 1.0;

    float yaw = YAW;
    float pitch = PITCH;
    float speed = SPEED;
    float sensitivity = SENSITIVITY;
};
