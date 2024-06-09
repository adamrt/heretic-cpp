#pragma once

#include <cassert>
#include <map>
#include <memory>
#include <string>

#include "Shader.h"

class ResourceManager {
public:
    static auto get_instance() -> ResourceManager*;

    auto add_shader(const std::string& name, const sg_shader_desc* shader_desc) -> void;
    auto get_shader(const std::string& name) -> std::shared_ptr<Shader>;

private:
    ResourceManager() { }
    static ResourceManager* instance;
    std::map<std::string, std::shared_ptr<Shader>> shaders;
};
