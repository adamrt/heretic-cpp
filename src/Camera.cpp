#include "Camera.h"

#include "glm/glm.hpp"

#include "glm/ext/matrix_transform.hpp"
#include "glm/gtc/matrix_transform.hpp"

auto Camera::orbit(float dx, float dy) -> void
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

auto Camera::zoom(float d) -> void
{
    _distance = glm::clamp(_distance + d, MIN_DIST, MAX_DIST);
}

auto Camera::_euclidean(float latitude, float longitude) -> glm::vec3
{
    const float lat = glm::radians(latitude);
    const float lng = glm::radians(longitude);
    return glm::vec3 { cosf(lat) * sinf(lng), sinf(lat), cosf(lat) * cosf(lng) };
}

auto Camera::update() -> void
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
