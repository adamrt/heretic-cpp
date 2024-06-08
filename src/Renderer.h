#pragma once

class Renderer {
public:
    Renderer();
    ~Renderer();
    auto begin_frame() -> void;
    auto end_frame() -> void;
};
