#include <array>
#include <iostream>
#include <memory>

// Project
#include "Camera.h"
#include "Light.h"
#include "Renderable.h"
#include "Scene.h"
#include "State.h"
#include "Texture.h"
#include "cube.h"

// STB
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

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

Scene scene;
State state;

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
    sg_shader cube_shader = sg_make_shader(standard_shader_desc(sg_query_backend()));
    auto tex = std::make_shared<Texture>("res/head.tga");
    auto obj = parse_obj("res/head.obj");
    auto cube_resources = std::make_shared<Mesh>(obj);

    sg_pipeline_desc pip_desc = {};
    pip_desc.shader = cube_shader;
    pip_desc.cull_mode = SG_CULLMODE_BACK;
    pip_desc.face_winding = SG_FACEWINDING_CCW;
    pip_desc.label = "pipeline";
    // Unnecessary if data is contiguous
    // pip_desc.layout.buffers[0].stride = 48;
    pip_desc.layout.attrs[ATTR_vs_a_position].format = SG_VERTEXFORMAT_FLOAT3;
    pip_desc.layout.attrs[ATTR_vs_a_normal].format = SG_VERTEXFORMAT_FLOAT3;
    pip_desc.layout.attrs[ATTR_vs_a_uv].format = SG_VERTEXFORMAT_FLOAT2;
    pip_desc.layout.attrs[ATTR_vs_a_color].format = SG_VERTEXFORMAT_FLOAT4;
    pip_desc.depth.write_enabled = true;
    pip_desc.depth.compare = SG_COMPAREFUNC_LESS_EQUAL;
    sg_pipeline color_pipeline = sg_make_pipeline(&pip_desc);

    auto cube = std::make_shared<Renderable>(cube_resources, tex, color_pipeline, state);
    scene.add_renderable(cube);
}

auto gui_draw() -> void
{
    ImGui::SetNextWindowSize(ImVec2(0, 0));

    ImGui::Begin("Hello, world!");
    ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
    if (ImGui::RadioButton("Perspective", state.camera.projection == Projection::Perspective)) {
        state.camera.projection = Projection::Perspective;
    }
    ImGui::SameLine();
    if (ImGui::RadioButton("Orthographic", state.camera.projection == Projection::Orthographic)) {
        state.camera.projection = Projection::Orthographic;
    }
    ImGui::SliderFloat("Rotation", &state.rotation_speed, 0.0f, 2.0f);
    ImGui::ColorEdit3("Background", &state.clear_color.r);
    if (ImGui::Button(sapp_is_fullscreen() ? "Switch to windowed" : "Switch to fullscreen")) {
        sapp_toggle_fullscreen();
    }
    if (ImGui::CollapsingHeader("Lighting")) {
        ImGui::ColorEdit4("Ambient Color", &state.ambient_color[0]);
        ImGui::SliderFloat("Ambient Strength", &state.ambient_strength, 0.0f, 1.0f);

        for (int i = 0; i < 3; i++) {
            ImGui::PushID(i);
            char title[10];
            sprintf(title, "Light %d", i);
            ImGui::SeparatorText(title);
            ImGui::SliderFloat4("Position", &state.lights[i].position[0], -50.0f, 50.0f, "%0.2f", 0);
            ImGui::ColorEdit4("Color", &state.lights[i].color[0], ImGuiColorEditFlags_None);
            ImGui::PopID();
        }
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

    state.camera.handle_event(event);
}

auto frame() -> void
{
    const float t = (float)sapp_frame_duration();

    state.camera.update();
    scene.update(t, state.rotation_speed);

    simgui_new_frame({
        sapp_width(),
        sapp_height(),
        sapp_frame_duration(),
        sapp_dpi_scale(),
    });

    sg_pass pass = {};
    pass.action.colors[0].clear_value = state.clear_color;
    pass.action.colors[0].load_action = SG_LOADACTION_CLEAR;
    pass.swapchain = sglue_swapchain();

    sg_begin_pass(&pass);
    {
        scene.render(state.camera.view_proj());
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
