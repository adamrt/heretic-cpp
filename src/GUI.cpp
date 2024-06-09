#include "GUI.h"
#include "Model.h"
#include "State.h"
#include "utils.h"

#include "sokol_app.h"
#include "sokol_gfx.h"
#include "sokol_log.h"

#define SOKOL_GLCORE
#include "imgui.h"
#include "sokol_imgui.h"

GUI::GUI()
{
    simgui_desc_t simgui_desc = {};
    simgui_desc.logger.func = slog_func;
    simgui_setup(&simgui_desc);
}

GUI::~GUI()
{
    simgui_shutdown();
}

auto GUI::render() -> void
{
    // Previously, simgui_new_frame(), was called before renderer->begin_frame()
    // but it seems okay here where it gets called after begin_frame(). This
    // allows a nice single render() call instead of two calls.
    simgui_new_frame({
        sapp_width(),
        sapp_height(),
        sapp_frame_duration(),
        sapp_dpi_scale(),
    });

    draw();
    simgui_render();
}

auto GUI::draw() -> void
{
    auto state = State::get_instance();
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
        if (ImGui::Button(state->scene.lights.size() < 10 ? "Add Light" : "Max Lights!")) {
            if (state->scene.lights.size() < 10) {
                auto position = glm::vec3 { rndf(-20, 20), rndf(-20, 20), rndf(-20, 20) };
                auto color = glm::vec4 { rndf(0.0, 1.0), rndf(0.0, 1.0), rndf(0.0, 1.0), 1.0f };
                auto light = std::make_shared<Light>(state->light_mesh, color, position);
                light->scale = glm::vec3(0.3f);
                state->scene.add_light(light);
            }
        }

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
            if (ImGui::Button("Delete")) {
                state->scene.lights.erase(state->scene.lights.begin() + i);
            }
            ImGui::PopID();
        }
    }

    if (ImGui::Button(sapp_is_fullscreen() ? "Switch to windowed" : "Switch to fullscreen")) {
        sapp_toggle_fullscreen();
    }
    ImGui::End();
}
