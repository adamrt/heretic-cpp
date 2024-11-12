#include "Dispatcher.h"
#include "BinFile.h"
#include "Event.h"
#include "State.h"

Dispatcher* Dispatcher::instance = nullptr;

auto Dispatcher::get_instance() -> Dispatcher*
{
    if (instance == nullptr) {
        instance = new Dispatcher();
    }
    return instance;
}

auto Dispatcher::update() -> void
{

    auto it = m_transitions.begin();
    if (it == m_transitions.end()) {
        return;
    }

    if (it->current_frame >= it->frames) {
        it = m_transitions.erase(it);
    } else {
        update_camera_transition(*it);
    }
}

auto Dispatcher::dispatch(Event& event) -> void
{
    for (auto& instruction : event.instructions()) {
        switch (instruction.command) {
        case 0x19:
            float x = instruction.param_float(0);
            float y = instruction.param_float(1);
            float z = instruction.param_float(2);
            float angle = instruction.param_float(3);
            float map_rotation = instruction.param_float(4);
            float cam_rotation = instruction.param_float(5);
            float zoom = instruction.param_float(6);
            int frames = instruction.param_int(7);

            angle = angle * DEGREE_PER_UNIT;
            map_rotation = map_rotation * DEGREE_PER_UNIT;
            cam_rotation = cam_rotation * DEGREE_PER_UNIT;

            auto position = glm::vec3(x, -y, -z);
            camera(position, angle, map_rotation, cam_rotation, zoom, frames);
            break;
        }
    }
}

auto Dispatcher::camera(glm::vec3 position, float angle, float map_rotation, float cam_rotation, float zoom, int frames) -> void
{
    auto state = State::get_instance();
    CameraTransition transition = {
        .start_position = state->fps_camera.position,
        .target_position = position,
        .start_angle = state->fps_camera.pitch,
        .target_angle = angle,
        .start_map_rotation = 0,
        .target_map_rotation = map_rotation,
        .start_cam_rotation = state->fps_camera.yaw,
        .target_cam_rotation = cam_rotation,
        .start_zoom = 1.0f,
        .target_zoom = zoom,
        .frames = frames * 2,
        .current_frame = 0
    };

    m_transitions.push_back(transition);
}

auto Dispatcher::update_camera_transition(CameraTransition& transition) -> void
{
    auto state = State::get_instance();
    float t = static_cast<float>(transition.current_frame) / transition.frames;

    glm::vec3 current_position = glm::mix(transition.start_position, transition.target_position, t);
    float current_angle = glm::mix(transition.start_angle, transition.target_angle, t);
    float current_cam_rotation = glm::mix(transition.start_cam_rotation, transition.target_cam_rotation, t);

    state->fps_camera.position = current_position;
    state->fps_camera.pitch = current_angle;
    state->fps_camera.yaw = current_cam_rotation;

    transition.current_frame++;
}
