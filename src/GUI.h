#pragma once

class GUI {
public:
    GUI();
    ~GUI();

    auto new_frame() -> void;
    auto render() -> void;

private:
    auto draw() -> void;
};
