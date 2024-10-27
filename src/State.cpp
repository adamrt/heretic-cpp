#include <algorithm>
#include <iostream>
#include <optional>

#include "Dispatcher.h"
#include "FFT.h"
#include "Model.h"
#include "ResourceManager.h"
#include "State.h"

State* State::instance = nullptr;

auto State::set_scenario(Scenario scenario) -> void
{
    auto it = std::find(scenarios.begin(), scenarios.end(), scenario);
    if (it != scenarios.end()) {
        current_scenario_index = std::distance(scenarios.begin(), it);
    } else {
        std::cout << "Scenario not found in list" << std::endl;
        current_scenario_index = -1;
        return;
    }

    current_style_index = 0;

    current_event = events[scenario.id()];
    current_scenario = scenario;

    set_map(scenario.map_id(), scenario.time(), scenario.weather());

    auto dispatcher = Dispatcher::get_instance();
    dispatcher->dispatch(current_event);
}

auto State::set_map(int map_num, MapTime time, MapWeather weather, int arrangement) -> bool
{
    auto state = State::get_instance();
    auto resources = ResourceManager::get_instance();
    auto reader = resources->get_bin_reader();

    while (true) {
        auto desc = map_list[map_num];
        if (!desc.valid) {
            std::cout << "Invalid map: " << map_num << std::endl;
            map_num++;
            if (map_num >= (int)map_list.size()) {
                map_num = 0;
            }
            continue;
        }
        break;
    }

    auto map = reader->read_map(map_num, time, weather, arrangement);
    if (map == nullptr) {
        std::cout << "Failed to load map: " << map_num << std::endl;
        return false;
    }

    auto map_mesh = std::make_shared<Mesh>(map->mesh->vertices);
    auto map_model = std::make_shared<PalettedModel>(map_mesh, map->texture, map->mesh->palette);
    auto background = std::make_shared<Background>(map->mesh->background);
    auto map_center_translation = map_mesh->center_translation();
    map_model->translation = map_center_translation;

    state->records = map->gns_records;

    state->scene.clear();
    state->scene.add_model(background);
    state->scene.add_model(map_model);
    state->scene.ambient_color = map->mesh->ambient_color;

    // Add lights with the same translation from the map. We don't take the
    // lights position into the center_translation calculation because they are
    // so far away.
    for (std::shared_ptr<Light> light : map->mesh->lights) {
        light->translation += map_center_translation;
        state->scene.add_light(light);
    }

    current_map_index = map_num;
    return true;
}

auto State::next_scenario() -> void
{
    auto state = State::get_instance();
    std::optional<Scenario> scenario = {};

    if (state->current_scenario_index == (int)scenarios.size() - 1) {
        state->current_scenario_index = 0;
        scenario = scenarios[state->current_scenario_index];
    } else {
        state->current_scenario_index++;
        scenario = scenarios[state->current_scenario_index];
    }

    state->set_scenario(*scenario);
};

auto State::previous_scenario() -> void
{
    auto state = State::get_instance();
    std::optional<Scenario> scenario = {};

    if (state->current_scenario_index == 0) {
        state->current_scenario_index = scenarios.size() - 1;
        scenario = scenarios[state->current_scenario_index];
    } else {
        state->current_scenario_index--;
        scenario = scenarios[state->current_scenario_index];
    }

    state->set_scenario(*scenario);
};

auto State::next_map() -> void
{
    auto state = State::get_instance();

    state->current_map_index++;

    while (true) {
        auto desc = map_list[state->current_map_index];
        if (!desc.valid) {
            state->current_map_index++;
            if (state->current_map_index >= (int)map_list.size()) {
                state->current_map_index = 0;
            }
            continue;
        }
        break;
    }

    state->set_map(state->current_map_index);
};

auto State::previous_map() -> void
{
    auto state = State::get_instance();

    state->current_map_index--;

    while (true) {
        auto desc = map_list[state->current_map_index];
        if (!desc.valid) {
            state->current_map_index--;
            if (state->current_map_index < 0) {
                state->current_map_index = map_list.size() - 1;
            }
            continue;
        }
        break;
    }

    state->set_map(state->current_map_index);
};
