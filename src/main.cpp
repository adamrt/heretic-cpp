#include <memory>

// Project
#include "Camera.h"
#include "Renderable.h"
#include "Scene.h"
#include "cube.h"

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

static struct {
    float rotation_speed = 1.0f;
    float rx, ry;

    Camera camera;
    Scene scene;

    struct {
        sg_color clear_color = { 0.0f, 0.5f, 0.7f, 1.0f };
        sg_pass_action pass_action;
        sg_pipeline pip;
        sg_bindings bind;
    } gfx;
} state;

auto gfx_init() -> void
{
    sg_desc desc = {};
    desc.environment = sglue_environment();
    desc.logger.func = slog_func;
    sg_setup(&desc);

    state.gfx.pass_action.colors[0].load_action = SG_LOADACTION_CLEAR;
}

auto gui_init() -> void
{
    simgui_desc_t simgui_desc = {};
    simgui_desc.logger.func = slog_func;
    simgui_setup(&simgui_desc);
}

auto init() -> void
{
    gfx_init();
    gui_init();

    sg_shader cube_shader = sg_make_shader(cube_shader_desc(sg_query_backend()));
    std::shared_ptr<MeshResources> cube_resources = std::make_shared<MeshResources>(cube_vertices, cube_indices, cube_shader);

    for (int i = 0; i < 1024; i++) {
        float x = random_float(-30.0f, 30.0f);
        float y = random_float(-30.0f, 30.0f);
        float z = random_float(-30.0f, 30.0f);
        Renderable cube(cube_resources);
        cube.translate(glm::vec3(x, y, z));
        state.scene.add_renderable(cube);
    }
}

auto gui_draw() -> void
{
    ImGui::SetNextWindowSize(ImVec2(0, 0));

    ImGui::Begin("Hello, world!");
    if (ImGui::RadioButton("Perspective", state.camera.projection == Projection::Perspective)) {
        state.camera.projection = Projection::Perspective;
    }
    ImGui::SameLine();
    if (ImGui::RadioButton("Orthographic", state.camera.projection == Projection::Orthographic)) {
        state.camera.projection = Projection::Orthographic;
    }
    ImGui::SliderFloat("Rotation", &state.rotation_speed, 0.0f, 2.0f);
    ImGui::ColorEdit3("Background", &state.gfx.clear_color.r);
    if (ImGui::Button(sapp_is_fullscreen() ? "Switch to windowed" : "Switch to fullscreen")) {
        sapp_toggle_fullscreen();
    }
    ImGui::End();
}

auto frame() -> void
{
    const float t = (float)sapp_frame_duration();

    state.camera.update();
    state.scene.update(t);

    simgui_new_frame({
        sapp_width(),
        sapp_height(),
        sapp_frame_duration(),
        sapp_dpi_scale(),
    });

    state.gfx.pass_action.colors[0].clear_value = state.gfx.clear_color;

    sg_pass pass = {};
    pass.action = state.gfx.pass_action;
    pass.swapchain = sglue_swapchain();

    sg_begin_pass(&pass);
    {
        glm::mat4 view_proj = state.camera.proj_matrix() * state.camera.view_matrix();
        state.scene.render(view_proj);
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

    state.camera.handle_event(event);
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
