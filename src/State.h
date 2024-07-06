#pragma once

#include <array>
#include <iostream>
#include <vector>

#include "Camera.h"
#include "FFT.h"
#include "GUI.h"
#include "Renderer.h"
#include "Scenario.h"
#include "Scene.h"

#include "sokol_gfx.h"

class State {
public:
    State(const State&) = delete;
    State& operator=(const State&) = delete;

    static State* get_instance()
    {
        if (instance == nullptr) {
            instance = new State();
        }
        return instance;
    }

    auto set_scenario(const Scenario scenario) -> void;
    auto set_map(int map_num, MapTime time = MapTime::Day, MapWeather weather = MapWeather::None, int arrangement = 0) -> bool;
    auto next_scenario() -> void;
    auto previous_scenario() -> void;
    auto next_map() -> void;
    auto previous_map() -> void;

    Renderer renderer = {};
    GUI gui = {};
    Scene scene = {};
    Camera camera = {};

    std::vector<Scenario> scenarios = {};
    Event current_event = {};
    std::vector<Record> records = {};

    int current_scenario_index = 1;
    int current_map_index = 49;
    int current_style_index = 0;
    Scenario current_scenario = {};

private:
    State() {};
    static State* instance;
};
