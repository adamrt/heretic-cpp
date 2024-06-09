#include "ResourceManager.h"

ResourceManager* ResourceManager::instance = nullptr;

auto ResourceManager::get_instance() -> ResourceManager*
{
    if (instance == nullptr) {
        instance = new ResourceManager();
    }
    return instance;
}

auto ResourceManager::add_shader(const std::string& name, std::shared_ptr<Shader> shader) -> std::shared_ptr<Shader>
{
    shaders[name] = shader;
    return shader;
}

auto ResourceManager::get_shader(const std::string& name) -> std::shared_ptr<Shader>
{
    auto it = shaders.find(name);
    if (it != shaders.end()) {
        return it->second;
    }
    assert(false);
}

auto ResourceManager::add_pipeline(const std::string& name, std::shared_ptr<Pipeline> pipeline) -> std::shared_ptr<Pipeline>
{
    pipelines[name] = pipeline;
    return pipeline;
}

auto ResourceManager::get_pipeline(const std::string& name) -> std::shared_ptr<Pipeline>
{
    auto it = pipelines.find(name);
    if (it != pipelines.end()) {
        return it->second;
    }
    assert(false);
}
