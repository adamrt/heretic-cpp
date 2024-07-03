#include <algorithm>
#include <iostream>

#include "FFT.h"
#include "Model.h"
#include "ResourceManager.h"
#include "State.h"

State* State::instance = nullptr;

auto State::set_scenario(Scenario scenario) -> void
{
    current_scenario = scenario;
    auto it = std::find(scenarios.begin(), scenarios.end(), scenario);
    if (it != scenarios.end()) {
        current_scenario_index = std::distance(scenarios.begin(), it);
    } else {
        std::cout << "Scenario not found in list" << std::endl;
        current_scenario_index = -1;
        return;
    }

    set_map_from_scenario(scenario);
}

auto State::set_map(int map_num, MapTime time, MapWeather weather, int arrangement) -> bool
{
    auto state = State::get_instance();
    auto resources = ResourceManager::get_instance();
    auto reader = resources->get_bin_reader();

    auto map = reader->read_map(map_num, time, weather, arrangement);
    if (map == nullptr) {
        std::cout << "Failed to load map: " << map_num << std::endl;
        return false;
    }

    auto map_mesh = std::make_shared<Mesh>(map->mesh->vertices);
    auto map_model = std::make_shared<PalettedModel>(map_mesh, map->texture, map->mesh->palette);
    auto background = std::make_shared<Background>(map->mesh->background);

    map_model->scale = map_mesh->normalized_scale();
    map_model->translation = map_mesh->center_translation();

    state->records = map->gns_records;

    state->scene.clear();
    state->scene.add_model(background);
    state->scene.add_model(map_model);
    state->scene.ambient_color = map->mesh->ambient_color;

    for (std::shared_ptr<Light> light : map->mesh->lights) {
        state->scene.add_light(light);
    }

    current_map_index = map_num;
    return true;
}

auto State::set_map_from_scenario(Scenario scenario) -> bool
{
    auto event = events[scenario.event_id()];
    event_instructions = event.instructions();

    return set_map(scenario.map_id(), scenario.time(), scenario.weather());
}

auto State::next_scenario() -> void
{
    auto state = State::get_instance();
    Scenario scenario = {};

    if (state->current_scenario_index == scenarios.size() - 1) {
        state->current_scenario_index = 0;
        scenario = scenarios[state->current_scenario_index];
    } else {
        state->current_scenario_index++;
        scenario = scenarios[state->current_scenario_index];
    }

    state->set_scenario(scenario);
};

auto State::previous_scenario() -> void
{
    auto state = State::get_instance();
    Scenario scenario = {};

    if (state->current_scenario_index == 0) {
        state->current_scenario_index = scenarios.size() - 1;
        scenario = scenarios[state->current_scenario_index];
    } else {
        state->current_scenario_index--;
        scenario = scenarios[state->current_scenario_index];
    }

    state->set_scenario(scenario);
};
