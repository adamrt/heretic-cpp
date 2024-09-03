#pragma once

#include <memory>

#include "Camera.h"
#include "Event.h"

class Dispatcher {
public:
    ~Dispatcher();

    static auto get_instance() -> Dispatcher*;

    auto update() -> void;
    auto dispatch(Event& event) -> void;
    auto clear() -> void
    {
        m_transitions.clear();
    };

private:
    struct CameraTransition {
        glm::vec3 start_position;
        glm::vec3 target_position;
        float start_angle;
        float target_angle;
        float start_map_rotation;
        float target_map_rotation;
        float start_zoom;
        float target_zoom;
        int frames;
        int current_frame;
    };

    Dispatcher();
    static Dispatcher* instance;

    // 0x19
    auto camera(glm::vec3 position, float angle, float map_rotation, float map_zoom, int frames) -> void;

    auto update_camera_transition(CameraTransition& transition) -> void;

    std::vector<CameraTransition> m_transitions;
};
