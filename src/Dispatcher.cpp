#include "Dispatcher.h"
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

Dispatcher::Dispatcher()
{
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
    // auto state = State::get_instance();

    for (auto& instruction : event.instructions()) {
        switch (instruction.command) {
        case 0x19:
            std::cout << "0x19" << std::endl;
            float x = instruction.param_float(0);
            float y = instruction.param_float(1);
            float z = instruction.param_float(2);
            float angle = instruction.param_float(3);
            float map_rotation = instruction.param_float(4);
            float zoom = instruction.param_float(5);
            int frames = instruction.param_int(6);

            auto position = glm::vec3(x, y, z); // * state->scale;

            camera(glm::vec3(x, y, z), angle, map_rotation, zoom, frames);
            break;
        }
    }
}

auto Dispatcher::camera(glm::vec3 position, float angle, float map_rotation, float map_zoom, int frames) -> void
{
    (void)position;
    (void)angle;
    (void)map_zoom;
    CameraTransition transition = {
        // .start_position = m_camera->get_eye(),
        // .target_position = position,
        // .start_angle = 0,
        // .target_angle = angle,
        .start_map_rotation = 35.0f,
        .target_map_rotation = map_rotation / 128,
        // .start_zoom = (float)m_camera->get_zoom(),
        // .target_zoom = (float)map_zoom,
        .frames = frames,
        .current_frame = 0
    };

    m_transitions.push_back(transition);
}

auto Dispatcher::update_camera_transition(CameraTransition& transition) -> void
{
    auto state = State::get_instance();
    float t = static_cast<float>(transition.current_frame) / transition.frames;

    glm::vec3 current_position = glm::mix(transition.start_position, transition.target_position, t);
    float current_angle = transition.start_angle + t * (transition.target_angle - transition.start_angle);
    float current_map_rotation = transition.start_map_rotation + t * (transition.target_map_rotation - transition.start_map_rotation);
    float current_map_zoom = transition.start_zoom + t * (transition.target_zoom - transition.start_zoom);

    float distance = glm::distance(current_position, glm::vec3 { 0, 0, 0 });

    (void)current_angle;
    (void)current_map_zoom;
    (void)distance;

    // state->camera.set_distance(distance);
    // state->camera.set_position(current_position);
    // state->camera.set_angle(current_angle);
    std::cout << "frame: " << transition.frames << std::endl;
    std::cout << "current_map_rotation: " << current_map_rotation << " target: " << transition.target_map_rotation << std::endl;
    for (auto& model : state->scene.models) {
        model->rotation = glm::vec3 { 0, current_map_rotation, 0 };
    }
    // state->camera.set_rotation(current_map_rotation);
    // state->camera.set_zoom(current_map_zoom);

    transition.current_frame++;
}
