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
    auto draw_commands() -> void;
    auto draw_events() -> void;
    auto draw_instructions() -> void;
    auto draw_messages() -> void;

    int scenarios_or_maps = 1;

    bool show_scenarios_table = false;
    bool show_records_table = false;
    bool show_events_table = false;
    bool show_instructions_table = false;
    bool show_messages_table = false;
};
