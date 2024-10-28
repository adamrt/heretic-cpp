#include "Camera.h"

#include "glm/ext/matrix_transform.hpp"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

auto FPSCamera::update() -> void
{
    float aspect = sapp_widthf() / sapp_heightf();
    if (projection == Projection::Perspective) {
        proj = glm::perspective(glm::radians(FOV), aspect, NEARZ, FARZ);
    } else {
        proj = glm::ortho(-ortho_scale * aspect, ortho_scale * aspect, -ortho_scale, ortho_scale, NEARZ, FARZ);
    }

    glm::vec3 newFront;
    newFront.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    newFront.y = sin(glm::radians(pitch));
    newFront.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    front = glm::normalize(newFront);
    // Also re-calculate the Right and Up vector
    right = glm::normalize(glm::cross(front, worldUp)); // Normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
    up = glm::normalize(glm::cross(right, front));
    view = glm::lookAt(position, position + front, up);
}

auto OrbitalCamera::reset() -> void
{
    _target = default_target;
    _fov = default_fov;
    _distance = default_distance;
    _latitude = default_latitude;
    _longitude = default_longitude;

    update();
}

auto OrbitalCamera::orbit(float dx, float dy) -> void
{
    _longitude -= dx;
    if (_longitude < 0.0f) {
        _longitude += 360.0f;
    }
    if (_longitude > 360.0f) {
        _longitude -= 360.0f;
    }

    _latitude = glm::clamp(_latitude + dy, MIN_LAT, MAX_LAT);
}

auto OrbitalCamera::pan(float dx, float dy) -> void
{
    const auto right = glm::normalize(glm::cross(_eye - _target, glm::vec3 { 0.0f, 1.0f, 0.0f }));
    const auto up = glm::normalize(glm::cross(right, _eye - _target));

    const auto d = right * dx + up * dy;
    _target += d;
}

auto OrbitalCamera::zoom(float d) -> void
{
    _distance = glm::clamp(_distance + d, MIN_DIST, MAX_DIST);
}

auto OrbitalCamera::_euclidean(float latitude, float longitude) -> glm::vec3
{
    const float lat = glm::radians(latitude);
    const float lng = glm::radians(longitude);
    return glm::vec3 { cosf(lat) * sinf(lng), sinf(lat), cosf(lat) * cosf(lng) };
}

auto OrbitalCamera::update() -> void
{
    _eye = _target + _euclidean(_latitude, _longitude) * _distance;
    _view = glm::lookAt(_eye, _target, glm::vec3 { 0.0f, 1.0f, 0.0f });

    const float aspect = sapp_widthf() / sapp_heightf();

    if (projection == Projection::Perspective) {
        _proj = glm::perspective(glm::radians(_fov), aspect, NEARZ, FARZ);
    } else {
        const float w = 1.0f * _distance;
        const float h = w / aspect;
        _proj = glm::ortho(-w, w, -h, h, NEARZ, FARZ);
    }
}
