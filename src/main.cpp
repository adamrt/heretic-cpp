#include <array>
#include <iostream>
#include <memory>

// Project
#include "Camera.h"
#include "Light.h"
#include "Model.h"
#include "State.h"
#include "Texture.h"

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

// GLM
#include "glm/glm.hpp"

// Forward declarations
auto gfx_init() -> void;
auto gui_init() -> void;
auto gui_draw() -> void;
auto world_init() -> void;

auto state = State::get_instance();

auto init() -> void
{
    gfx_init();
    gui_init();
    world_init();
}

auto gfx_init() -> void
{
    sg_desc desc = {};
    desc.environment = sglue_environment();
    desc.logger.func = slog_func;
    sg_setup(&desc);
}

auto gui_init() -> void
{
    simgui_desc_t simgui_desc = {};
    simgui_desc.logger.func = slog_func;
    simgui_setup(&simgui_desc);
}

auto world_init() -> void
{
    auto light_mesh = std::make_shared<Mesh>("res/cube.obj");

    auto model_texture = std::make_shared<Texture>("res/cube.png");
    auto model_mesh = std::make_shared<Mesh>("res/cube.obj");
    auto model = std::make_shared<Model>(model_mesh, model_texture);

    state->scene.add_light(std::make_shared<Light>(light_mesh, glm::vec4 { 40.0f, 0.0f, 0.0f, 0.0f }, glm::vec4 { 0.0f, 0.0f, 1.0f, 1.0f }));
    state->scene.add_light(std::make_shared<Light>(light_mesh, glm::vec4 { 0.0f, 40.0f, 0.0f, 0.0f }, glm::vec4 { 1.0f, 0.0f, 0.0f, 1.0f }));
    state->scene.add_light(std::make_shared<Light>(light_mesh, glm::vec4 { 0.0f, 0.0f, 40.0f, 0.0f }, glm::vec4 { 0.5f, 0.5f, 0.5f, 1.0f }));

    state->scene.add_model(model);
}

auto gui_draw() -> void
{
    ImGui::SetNextWindowSize(ImVec2(0, 0));
    ImGui::Begin("Hello, world!");
    ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
    if (ImGui::RadioButton("Perspective", state->camera.projection == Projection::Perspective)) {
        state->camera.projection = Projection::Perspective;
    }
    ImGui::SameLine();
    if (ImGui::RadioButton("Orthographic", state->camera.projection == Projection::Orthographic)) {
        state->camera.projection = Projection::Orthographic;
    }

    // Render Mode
    if (ImGui::RadioButton("Textured", state->render_mode == 0)) {
        state->render_mode = 0;
    }
    ImGui::SameLine();
    if (ImGui::RadioButton("Colored", state->render_mode == 1)) {
        state->render_mode = 1;
    }
    ImGui::SameLine();
    if (ImGui::RadioButton("Normals", state->render_mode == 2)) {
        state->render_mode = 2;
    }

    ImGui::SliderFloat("Rotation", &state->rotation_speed, 0.0f, 2.0f);
    ImGui::ColorEdit3("Background", &state->clear_color.r);
    if (ImGui::CollapsingHeader("Lighting")) {
        ImGui::Checkbox("Enabled", &state->use_lighting);
        ImGui::ColorEdit4("Ambient Color", &state->ambient_color[0]);
        ImGui::SliderFloat("Ambient Strength", &state->ambient_strength, 0.0f, 1.0f);

        for (size_t i = 0; i < state->scene.lights.size(); i++) {
            ImGui::PushID(i);
            char title[10];
            sprintf(title, "Light %d", (int)i);
            ImGui::SeparatorText(title);
            ImGui::SliderFloat3("Position", &state->scene.lights[i]->translation[0], -50.0f, 50.0f, "%0.2f", 0);
            ImGui::ColorEdit4("Color", &state->scene.lights[i]->color[0], ImGuiColorEditFlags_None);
            ImGui::PopID();
        }
    }
    if (ImGui::Button("Delete Light")) {
        state->scene.lights.pop_back();
    }
    if (ImGui::Button(sapp_is_fullscreen() ? "Switch to windowed" : "Switch to fullscreen")) {
        sapp_toggle_fullscreen();
    }
    ImGui::End();
}

auto input(sapp_event const* event) -> void
{
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

    const float delta = (float)sapp_frame_duration();
    for (auto& model : state->scene.models) {
        model->rotation += state->rotation_speed * delta;
        model->update(delta);
    }

    state->camera.update();
    state->scene.update(delta);

    simgui_new_frame({
        sapp_width(),
        sapp_height(),
        sapp_frame_duration(),
        sapp_dpi_scale(),
    });

    sg_pass pass = {};
    pass.action.colors[0].clear_value = state->clear_color;
    pass.action.colors[0].load_action = SG_LOADACTION_CLEAR;
    pass.swapchain = sglue_swapchain();

    sg_begin_pass(&pass);
    {
        state->scene.render();
        gui_draw();
        simgui_render();
    }
    sg_end_pass();

    sg_commit();
}

auto cleanup() -> void
{
    simgui_shutdown();
    sg_shutdown();
}

sapp_desc sokol_main(int argc, char* argv[])
{
    (void)argc;
    (void)argv;
    sapp_desc desc = {};
    desc.init_cb = init;
    desc.frame_cb = frame;
    desc.cleanup_cb = cleanup;
    desc.event_cb = input;
    desc.window_title = "Dear ImGui (sokol-app)";
    desc.ios_keyboard_resizes_canvas = false;
    desc.icon.sokol_default = true;
    desc.enable_clipboard = true;
    desc.logger.func = slog_func;
    return desc;
}
