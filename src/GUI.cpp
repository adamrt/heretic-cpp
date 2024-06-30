#include <algorithm>

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

auto GUI::draw_scenarios() -> void
{

    auto state = State::get_instance();

    ImGui::Begin("Scenarios");

    // Begin a table
    if (ImGui::BeginTable("Scenarios", 4, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
        // Set up column headers
        ImGui::TableSetupColumn("ID");
        ImGui::TableSetupColumn("Map");
        ImGui::TableSetupColumn("Time");
        ImGui::TableSetupColumn("Weather");

        ImGui::TableHeadersRow();

        // Populate table with data
        for (auto& scenario : state->scenarios) {
            ImGui::TableNextRow();

            // Column 0: ID
            ImGui::TableSetColumnIndex(0);
            ImGui::Text("%d", scenario.id());

            ImGui::TableSetColumnIndex(1);
            ImGui::Text("%s", map_list[scenario.map()].name.c_str());

            ImGui::TableSetColumnIndex(2);
            ImGui::Text("%s", map_time_str(scenario.time()).c_str());

            ImGui::TableSetColumnIndex(3);
            ImGui::Text("%s", map_weather_str(scenario.weather()).c_str());
        }

        // End the table
        ImGui::EndTable();
    }

    // End the window
    ImGui::End();
}

auto GUI::draw_instructions() -> void
{

    auto state = State::get_instance();
    ImGui::SetNextWindowSize(ImVec2(1175.0f, 800.0f));
    ImGui::Begin("Instructions");

    // Begin a table
    if (ImGui::BeginTable("Instructions", 3, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
        // Set up column headers
        ImGui::TableSetupColumn("ID", ImGuiTableColumnFlags_WidthFixed, 50.0f);
        ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthFixed, 120.0f);
        ImGui::TableSetupColumn("Description", ImGuiTableColumnFlags_WidthFixed, 1000.0f);

        ImGui::TableHeadersRow();

        // Populate table with data
        for (auto& [key, value] : instruction_list) {
            ImGui::TableNextRow();

            // Column 0: ID
            ImGui::TableSetColumnIndex(0);
            ImGui::Text("0x%X", key);

            ImGui::TableSetColumnIndex(1);
            ImGui::Text("%s", value.name.c_str());

            ImGui::TableSetColumnIndex(2);
            ImGui::Text("%s", value.description.c_str());
        }

        // End the table
        ImGui::EndTable();
    }

    // End the window
    ImGui::End();
}

auto GUI::draw_events() -> void
{

    auto state = State::get_instance();
    ImGui::SetNextWindowSize(ImVec2(1175.0f, 800.0f));
    ImGui::Begin("Events");

    // Begin a table
    if (ImGui::BeginTable("Events", 1, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
        // Set up column headers
        ImGui::TableSetupColumn("ID", ImGuiTableColumnFlags_WidthFixed, 100.0f);

        ImGui::TableHeadersRow();

        // Populate table with data
        for (auto& event : state->events) {
            if (event.id() != 0xF2F2F2F2) {
                continue;
            }
            ImGui::TableNextRow();

            // Column 0: ID
            ImGui::TableSetColumnIndex(0);
            ImGui::Text("0x%X", event.id());
        }

        // End the table
        ImGui::EndTable();
    }

    // End the window
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
            ImGui::Text("%ld", record.length());

            // Column 2: Type
            ImGui::TableSetColumnIndex(2);
            ImGui::Text("%s", resource_type_str(record.resource_type()).c_str());

            // Column 3: Arrangement
            ImGui::TableSetColumnIndex(3);
            ImGui::Text("%d", record.arrangement());

            // Column 4: Time
            ImGui::TableSetColumnIndex(4);
            ImGui::Text("%s", map_time_str(record.time()).c_str());

            // Column 5: Weather
            ImGui::TableSetColumnIndex(5);
            ImGui::Text("%s", map_weather_str(record.weather()).c_str());
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
    ImGui::Begin("Hello, world!");

    std::vector<std::string> scenario_names;
    scenario_names.reserve(state->scenarios.size());

    std::transform(
        state->scenarios.begin(),
        state->scenarios.end(),
        std::back_inserter(scenario_names),
        [](Scenario& s) { return s.repr(); });

    std::vector<const char*> scenario_name_ptrs;
    scenario_name_ptrs.reserve(scenario_names.size());
    for (const auto& name : scenario_names) {
        scenario_name_ptrs.push_back(name.c_str());
    }

    if (ImGui::Combo("Select Scenario", &current_scenario, scenario_name_ptrs.data(), scenario_name_ptrs.size())) {
        // Item selected, handle selection here
        std::cout << "Selected item: " << map_list[state->scenarios[current_scenario].map()].name << std::endl;
    }

    auto map_desc = map_list[state->scene.map_num];
    ImGui::Text("%d. %s", map_desc.id, map_desc.name.c_str());
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
    ImGui::ColorEdit3("Background", &state->renderer.clear_color.r);
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
        ImGui::SliderFloat("Strength", &state->scene.ambient_strength, 0.0f, 1.0f);

        for (size_t i = 0; i < state->scene.lights.size(); i++) {
            ImGui::PushID(i);
            char title[10];
            sprintf(title, "Light %d", (int)i);
            ImGui::SeparatorText(title);
            ImGui::SliderFloat3("Position", &state->scene.lights[i]->translation[0], -50.0f, 50.0f, "%0.2f", 0);
            ImGui::ColorEdit4("Color", &state->scene.lights[i]->color[0], ImGuiColorEditFlags_None);
            ImGui::Checkbox("Enabled", &state->scene.lights[i]->is_enabled);
            ImGui::PopID();
        }
    }

    if (ImGui::Button("Show Map Records")) {
        show_records_table = !show_records_table;
    }
    ImGui::SameLine();
    if (ImGui::Button("Show All Scenarios")) {
        show_scenario_table = !show_scenario_table;
    }
    if (ImGui::Button("Show Instructions")) {
        show_instructions_table = !show_instructions_table;
    }
    if (ImGui::Button("Show Events")) {
        show_events_table = !show_events_table;
    }

    if (ImGui::Button(sapp_is_fullscreen() ? "Switch to windowed" : "Switch to fullscreen")) {
        sapp_toggle_fullscreen();
    }

    ImGui::End();

    if (show_records_table) {
        if (state->records.size() > 0) {
            draw_records();
        }
    }

    if (show_scenario_table) {
        if (state->scenarios.size() > 0) {
            draw_scenarios();
        }
    }

    if (show_instructions_table) {
        draw_instructions();
    }

    if (show_events_table) {
        if (state->events.size() > 0) {
            draw_events();
        }
    }
}
