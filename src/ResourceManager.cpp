#include "ResourceManager.h"

ResourceManager* ResourceManager::instance = nullptr;

auto ResourceManager::get_instance() -> ResourceManager*
{
    if (instance == nullptr) {
        instance = new ResourceManager();
    }
    return instance;
}

auto ResourceManager::add_shader(const std::string& name, const sg_shader_desc* shader_desc) -> void
{
    auto shader = std::make_shared<Shader>(shader_desc);
    shaders[name] = shader;
}

auto ResourceManager::get_shader(const std::string& name) -> std::shared_ptr<Shader>
{
    auto it = shaders.find(name);
    if (it != shaders.end()) {
        return it->second;
    }
    assert(false);
}
