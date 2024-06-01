#define SOKOL_IMPL
#define SOKOL_GLCORE
#include "sokol_app.h"
#include "sokol_gfx.h"
#include "sokol_glue.h"
#include "sokol_log.h"

#define SOKOL_IMGUI_IMPL
#include "imgui.h"
#include "sokol_imgui.h"

static struct {
    float time;
    struct {
        sg_color clear_color;
        sg_pass_action pass_action;
    } gfx;
} state;

void gfx_init()
{
    sg_desc desc = {};
    desc.environment = sglue_environment();
    desc.logger.func = slog_func;
    sg_setup(&desc);

    state.gfx.pass_action.colors[0].load_action = SG_LOADACTION_CLEAR; // only needed once
    state.gfx.clear_color = { 0.0f, 0.5f, 0.7f, 1.0f };
}

void gui_init()
{
    simgui_desc_t simgui_desc = {};
    simgui_desc.logger.func = slog_func;
    simgui_setup(&simgui_desc);
}

void init()
{
    gfx_init();
    gui_init();
}

void gui_draw()
{
    static float f = 0.0f;
    ImGui::Begin("Hello, world!");
    ImGui::Text("New thing!");
    ImGui::SliderFloat("float", &f, 0.0f, 1.0f);
    ImGui::ColorEdit3("clear color", &state.gfx.clear_color.r);
    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
    ImGui::Text("w: %d, h: %d, dpi_scale: %.1f", sapp_width(), sapp_height(), sapp_dpi_scale());
    if (ImGui::Button(sapp_is_fullscreen() ? "Switch to windowed" : "Switch to fullscreen")) {
        sapp_toggle_fullscreen();
    }
    ImGui::End();
}

void frame()
{
    state.time += (float)sapp_frame_duration();

    // clear color
    state.gfx.pass_action.colors[0].clear_value = state.gfx.clear_color;

    simgui_new_frame({
        sapp_width(),
        sapp_height(),
        sapp_frame_duration(),
        sapp_dpi_scale(),
    });

    sg_pass pass = {};
    pass.action = state.gfx.pass_action;
    pass.swapchain = sglue_swapchain();

    gui_draw();

    sg_begin_pass(&pass);
    simgui_render();
    sg_end_pass();
    sg_commit();
}

void cleanup()
{
    simgui_shutdown();
    sg_shutdown();
}

void input(sapp_event const* event)
{
    if (event->type == SAPP_EVENTTYPE_KEY_DOWN) {
        if (event->key_code == SAPP_KEYCODE_ESCAPE) {
            sapp_quit();
        }
    }

    if (simgui_handle_event(event)) {
        return;
    }
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
