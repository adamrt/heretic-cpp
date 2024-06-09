#include "Shader.h"

#include <sokol_gfx.h>

Shader::Shader(const sg_shader_desc* shader_desc)
{
    shader = sg_make_shader(shader_desc);
}

Shader::~Shader()
{
    sg_destroy_shader(shader);
}
