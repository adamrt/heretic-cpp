#pragma once

#include <array>
#include <iostream>

#include "Camera.h"
#include "FFT.h"
#include "GUI.h"
#include "Renderer.h"
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
    Renderer renderer = {};
    GUI gui = {};
    Scene scene = {};
    Camera camera = {};

    std::vector<Record> records = {};
    std::vector<Scenario> scenarios = {};
    std::vector<Event> events = {};

private:
    State() {};
    static State* instance;
};
