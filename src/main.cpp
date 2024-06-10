#include <array>
#include <iostream>
#include <memory>

#include "Camera.h"
#include "FFT.h"
#include "Model.h"
#include "Pipeline.h"
#include "Shader.h"
#include "State.h"
#include "Texture.h"
#include "utils.h"

// GLM
#include "glm/glm.hpp"

// Sokol
#define SOKOL_IMPL
#define SOKOL_GLCORE
#include "sokol_app.h"
#include "sokol_gfx.h"
#include "sokol_glue.h"
#include "sokol_log.h"

// ImGUI
#define SOKOL_IMGUI_IMPL
#include "imgui.h"
#include "sokol_imgui.h"

auto init() -> void
{
    auto state = State::get_instance();

    BinReader reader("/home/adam/sync/emu/fft.bin");
    auto map = reader.read_map(49);

    auto map_mesh = std::make_shared<Mesh>(map->vertices);

    auto model = std::make_shared<TexturedModel>(map_mesh, map->texture);
    model->scale = map_mesh->normalized_scale();
    model->translation = map_mesh->center_translation();
    state->scene.add_model(model);

    for (std::shared_ptr<Light> light : map->lights) {
        state->scene.add_light(light);
    }
}

auto input(sapp_event const* event) -> void
{
    auto state = State::get_instance();

    if (event->type == SAPP_EVENTTYPE_KEY_DOWN) {
        if (event->key_code == SAPP_KEYCODE_ESCAPE) {
            sapp_quit();
        }
    }

    if (simgui_handle_event(event)) {
        return;
    }

    switch (event->type) {
    case SAPP_EVENTTYPE_MOUSE_DOWN:
        if (event->mouse_button == SAPP_MOUSEBUTTON_LEFT) {
            sapp_lock_mouse(true);
        }
        break;
    case SAPP_EVENTTYPE_MOUSE_UP:
        if (event->mouse_button == SAPP_MOUSEBUTTON_LEFT) {
            sapp_lock_mouse(false);
        }
        break;
    case SAPP_EVENTTYPE_MOUSE_SCROLL:
        state->camera.zoom(event->scroll_y * -0.5f);
        break;
    case SAPP_EVENTTYPE_MOUSE_MOVE:
        if (sapp_mouse_locked()) {
            state->camera.orbit(event->mouse_dx * 0.25f, event->mouse_dy * 0.25f);
        }
        break;
    default:
        break;
    }
}

auto frame() -> void
{
    auto state = State::get_instance();
    const float delta = (float)sapp_frame_duration();

    // Update
    state->camera.update();
    state->scene.update(delta);
    for (auto& model : state->scene.models) {
        model->rotation += state->rotation_speed * delta;
        model->update(delta);
    }

    // Render
    state->renderer.begin_frame();
    state->scene.render();
    state->gui.render();
    state->renderer.end_frame();
}

sapp_desc sokol_main(int argc, char* argv[])
{
    (void)argc;
    (void)argv;
    sapp_desc desc = {};
    desc.init_cb = init;
    desc.frame_cb = frame;
    desc.event_cb = input;
    desc.window_title = "Dear ImGui (sokol-app)";
    desc.ios_keyboard_resizes_canvas = false;
    desc.icon.sokol_default = true;
    desc.enable_clipboard = true;
    desc.logger.func = slog_func;
    return desc;
}
