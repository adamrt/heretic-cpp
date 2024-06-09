#pragma once

#include <memory>
#include <string>

#include <sokol_gfx.h>

class Shader {
public:
    Shader(const sg_shader_desc* shader_desc);
    ~Shader();

    sg_shader get_shader() const { return shader; }

private:
    sg_shader shader = {};
};
