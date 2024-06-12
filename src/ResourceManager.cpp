#include "ResourceManager.h"

#include "shader.glsl.h"

ResourceManager* ResourceManager::instance = nullptr;

ResourceManager::ResourceManager()
{
    auto textured_shader = add_shader("textured", std::make_shared<Shader>(textured_shader_desc(sg_query_backend())));
    add_pipeline("textured", std::make_shared<Pipeline>(textured_shader));

    auto colored_shader = add_shader("colored", std::make_shared<Shader>(colored_shader_desc(sg_query_backend())));
    add_pipeline("colored", std::make_shared<Pipeline>(colored_shader));

    auto paletted_shader = add_shader("paletted", std::make_shared<Shader>(paletted_shader_desc(sg_query_backend())));
    add_pipeline("paletted", std::make_shared<Pipeline>(paletted_shader));

    add_mesh("cube", std::make_shared<Mesh>("res/cube.obj"));
}

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

auto ResourceManager::add_mesh(const std::string& name, std::shared_ptr<Mesh> mesh) -> std::shared_ptr<Mesh>
{
    meshes[name] = mesh;
    return mesh;
}

auto ResourceManager::get_mesh(const std::string& name) -> std::shared_ptr<Mesh>
{
    auto it = meshes.find(name);
    if (it != meshes.end()) {
        return it->second;
    }
    assert(false);
}

auto ResourceManager::set_bin_reader(std::shared_ptr<BinReader> _bin_reader) -> std::shared_ptr<BinReader>
{
    bin_reader = _bin_reader;
    return bin_reader;
}

auto ResourceManager::get_bin_reader() -> std::shared_ptr<BinReader>
{
    return bin_reader;
}
