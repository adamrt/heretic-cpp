#pragma once

#include "sokol_gfx.h"

class Renderer {
public:
    Renderer();
    ~Renderer();
    auto begin_frame() -> void;
    auto end_frame() -> void;

    int render_mode = { 0 };
    sg_color clear_color = { 0.0f, 0.5f, 0.7f, 1.0f };
};
