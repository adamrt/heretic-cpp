#pragma once

class GUI {
public:
    GUI();
    ~GUI();

    auto new_frame() -> void;
    auto render() -> void;

private:
    auto draw() -> void;
    auto draw_scenarios() -> void;
    auto draw_records() -> void;

    bool show_scenario_table = false;
    bool show_records_table = false;
    int current_scenario = 0;
};
