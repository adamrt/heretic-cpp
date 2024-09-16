#include <algorithm>
#include <format>
#include <iomanip>
#include <sstream>
#include <utility>

#include "FFT.h"
#include "GUI.h"
#include "Model.h"
#include "ResourceManager.h"
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

auto GUI::draw_instructions() -> void
{
    auto state = State::get_instance();
    ImGui::SetNextWindowSize(ImVec2(800.0f, 600.0f));
    ImGui::Begin("Instructions");

    // Begin a table
    if (ImGui::BeginTable("Instructions", 11, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {

        ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthFixed, 200.0f);
        ImGui::TableSetupColumn("Param 1", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn("Param 2", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn("Param 3", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn("Param 4", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn("Param 5", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn("Param 6", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn("Param 7", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn("Param 8", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn("Param 9", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn("Param 10", ImGuiTableColumnFlags_WidthStretch);

        ImGui::TableHeadersRow();

        for (auto& instruction : state->current_event.instructions()) {
            ImGui::TableNextRow();

            auto column = 0;

            ImGui::TableSetColumnIndex(column);
            ImGui::Text("%s", command_list[instruction.command].name.c_str());

            for (auto& param : instruction.params) {
                column++;
                ImGui::TableSetColumnIndex(column);
                if (std::holds_alternative<uint8_t>(param)) {
                    ImGui::Text("0x%02X", std::get<uint8_t>(param));
                } else if (std::holds_alternative<uint16_t>(param)) {
                    ImGui::Text("0x%04X", std::get<uint16_t>(param));
                }
            }

            for (; column < 10; column++) {
                ImGui::TableSetColumnIndex(column);
            }
        }

        ImGui::EndTable();
    }

    ImGui::End();
}

auto GUI::draw_messages() -> void
{

    auto state = State::get_instance();
    ImGui::Begin("Messages");

    // Begin a table
    if (ImGui::BeginTable("Messages", 2, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {

        ImGui::TableSetupColumn("Index", ImGuiTableColumnFlags_WidthFixed, 200.0f);
        ImGui::TableSetupColumn("Message", ImGuiTableColumnFlags_WidthStretch);

        ImGui::TableHeadersRow();

        auto rows = 1;
        for (auto& message : state->current_event.messages()) {
            ImGui::TableNextRow();

            ImGui::TableSetColumnIndex(0);
            ImGui::Text("%d", rows);

            ImGui::TableSetColumnIndex(1);
            ImGui::Text("%s", message.c_str());
            rows++;
        }

        ImGui::EndTable();
    }

    ImGui::End();
}

auto GUI::draw_records() -> void
{

    auto state = State::get_instance();

    ImGui::Begin("Records");

    // Begin a table
    if (ImGui::BeginTable("Records", 6, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
        // Set up column headers
        ImGui::TableSetupColumn("Sector");
        ImGui::TableSetupColumn("Length");
        ImGui::TableSetupColumn("Type");
        ImGui::TableSetupColumn("Arrangement");
        ImGui::TableSetupColumn("Time");
        ImGui::TableSetupColumn("Weather");

        ImGui::TableHeadersRow();

        // Populate table with data
        for (auto& record : state->records) {
            ImGui::TableNextRow();

            // Column 0: ID
            ImGui::TableSetColumnIndex(0);
            ImGui::Text("%d", record.sector());

            // Column 1: Length
            ImGui::TableSetColumnIndex(1);
            ImGui::Text("%llu", record.length());

            // Column 2: Type
            ImGui::TableSetColumnIndex(2);
            ImGui::Text("%s", to_string(record.resource_type()).data());

            // Column 3: Arrangement
            ImGui::TableSetColumnIndex(3);
            ImGui::Text("%d", record.arrangement());

            // Column 4: Time
            ImGui::TableSetColumnIndex(4);
            ImGui::Text("%s", to_string(record.time()).data());

            // Column 5: Weather
            ImGui::TableSetColumnIndex(5);
            ImGui::Text("%s", to_string(record.weather()).data());
        }

        // End the table
        ImGui::EndTable();
    }

    // End the window
    ImGui::End();
}

auto GUI::draw() -> void
{
    auto resources = ResourceManager::get_instance();
    auto state = State::get_instance();

    ImGui::SetNextWindowSize(ImVec2(0, 0));
    ImGui::Begin("Heretic");

    if (ImGui::RadioButton("Scenarios", scenarios_or_maps == 0)) {
        scenarios_or_maps = 0;
        auto new_scenario = state->scenarios[state->current_scenario_index];
        state->set_scenario(new_scenario);
    }
    ImGui::SameLine();
    if (ImGui::RadioButton("Maps", scenarios_or_maps == 1)) {
        scenarios_or_maps = 1;
        state->set_map(state->current_map_index);
    }

    if (scenarios_or_maps == 0) {
        std::vector<std::string> scenario_names;
        scenario_names.reserve(state->scenarios.size());

        std::transform(
            state->scenarios.begin(),
            state->scenarios.end(),
            std::back_inserter(scenario_names),
            [](Scenario& s) { return scenario_list[s.id()]; });

        std::vector<const char*> scenario_name_ptrs;
        scenario_name_ptrs.reserve(scenario_names.size());
        for (const auto& name : scenario_names) {
            scenario_name_ptrs.push_back(name.c_str());
        }

        if (ImGui::Combo("Scenario", &state->current_scenario_index, scenario_name_ptrs.data(), scenario_name_ptrs.size())) {
            auto new_scenario = state->scenarios[state->current_scenario_index];
            state->set_scenario(new_scenario);
        }

        ImGui::Text("Map: %s", map_list[state->current_scenario.map_id()].name.c_str());
        ImGui::Text("Time: %s", to_string(state->current_scenario.time()).c_str());
        ImGui::Text("Weather: %s", to_string(state->current_scenario.weather()).c_str());

    } else if (scenarios_or_maps == 1) {
        std::vector<std::string> map_names;

        for (auto [k, v] : map_list) {
            map_names.push_back(v.repr());
        }

        std::vector<const char*> map_name_ptrs;
        map_name_ptrs.reserve(map_names.size());
        for (const auto& name : map_names) {
            map_name_ptrs.push_back(name.c_str());
        }

        if (ImGui::Combo("Map", &state->current_map_index, map_name_ptrs.data(), map_name_ptrs.size())) {
            state->set_map(state->current_map_index);
        }

        std::vector<std::string> style_names;

        auto records_copy = state->records;
        std::sort(records_copy.begin(), records_copy.end());

        // Remove duplicates
        auto last = std::unique(records_copy.begin(), records_copy.end());
        records_copy.erase(last, records_copy.end());

        std::transform(
            records_copy.begin(),
            records_copy.end(),
            std::back_inserter(style_names),
            [](Record& s) { return s.repr(); });

        // Remove duplicates by using std::unique and then erase the redundant elements

        std::vector<const char*> style_name_ptrs;
        style_name_ptrs.reserve(style_names.size());
        for (const auto& name : style_names) {
            style_name_ptrs.push_back(name.c_str());
        }

        if (ImGui::Combo("Style", &state->current_style_index, style_name_ptrs.data(), style_name_ptrs.size())) {
            auto record = records_copy[state->current_style_index];
            state->set_map(state->current_map_index, record.time(), record.weather(), record.arrangement());
        }
    }

    ImGui::NewLine();
    ImGui::Separator();
    if (ImGui::CollapsingHeader("Rendering")) {
        ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
        if (ImGui::RadioButton("Perspective", state->camera.projection == Projection::Perspective)) {
            state->camera.projection = Projection::Perspective;
        }
        ImGui::SameLine();
        if (ImGui::RadioButton("Orthographic", state->camera.projection == Projection::Orthographic)) {
            state->camera.projection = Projection::Orthographic;
        }

        // Render Mode
        if (ImGui::RadioButton("Textured", state->renderer.render_mode == 0)) {
            state->renderer.render_mode = 0;
        }
        ImGui::SameLine();
        if (ImGui::RadioButton("Colored", state->renderer.render_mode == 1)) {
            state->renderer.render_mode = 1;
        }
        ImGui::SameLine();
        if (ImGui::RadioButton("Normals", state->renderer.render_mode == 2)) {
            state->renderer.render_mode = 2;
        }

        ImGui::SliderFloat("Rotation", &state->scene.rotation_speed, 0.0f, 2.0f);
        // ImGui::ColorEdit3("Background", &state->renderer.clear_color.r);
    }
    ImGui::NewLine();
    if (ImGui::CollapsingHeader("Lighting")) {
        ImGui::Checkbox("Lighting Enabled", &state->scene.use_lighting);
        ImGui::SameLine();
        if (ImGui::Button(state->scene.lights.size() < 10 ? "Add Light" : "Max Lights!")) {
            if (state->scene.lights.size() < 10) {
                auto cube_mesh = resources->get_mesh("cube");
                auto position = glm::vec3 { rndf(-20, 20), rndf(-20, 20), rndf(-20, 20) };
                auto color = glm::vec4 { rndf(0.0, 1.0), rndf(0.0, 1.0), rndf(0.0, 1.0), 1.0f };
                auto light = std::make_shared<Light>(cube_mesh, color, position);
                state->scene.add_light(light);
            }
        }

        ImGui::SeparatorText("Ambient Lighting");
        ImGui::ColorEdit4("Color", &state->scene.ambient_color[0]);
        ImGui::SliderFloat("Strength", &state->scene.ambient_strength, 0.0f, 2.0f);

        for (size_t i = 0; i < state->scene.lights.size(); i++) {
            ImGui::PushID(i);
            std::string title = std::format("Light {}", static_cast<int>(i));
            ImGui::SeparatorText(title.c_str());
            ImGui::SliderFloat3("Position", &state->scene.lights[i]->translation[0], -50.0f, 50.0f, "%0.2f", 0);
            ImGui::ColorEdit4("Color", &state->scene.lights[i]->color[0], ImGuiColorEditFlags_None);
            ImGui::Checkbox("Enabled", &state->scene.lights[i]->is_enabled);
            ImGui::PopID();
        }
    }

    ImGui::NewLine();
    ImGui::Separator();

    if (ImGui::Button("Map_Records")) {
        show_records_table = !show_records_table;
    }
    ImGui::SameLine();
    if (ImGui::Button("Event_Instructions")) {
        std::cout << "Instructions" << std::endl;
        show_instructions_table = !show_instructions_table;
    }
    ImGui::SameLine();
    if (ImGui::Button("Event_Messages")) {
        show_messages_table = !show_messages_table;
    }
    if (ImGui::Button(sapp_is_fullscreen() ? "Switch to windowed" : "Switch to fullscreen")) {
        sapp_toggle_fullscreen();
    }

    ImGui::End();

    if (show_records_table) {
        draw_records();
    }

    if (show_instructions_table) {
        draw_instructions();
    }

    if (show_messages_table) {
        draw_messages();
    }
}
