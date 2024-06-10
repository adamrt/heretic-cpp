#pragma once

#include <cassert>
#include <map>
#include <memory>
#include <string>

#include "Mesh.h"
#include "Pipeline.h"
#include "Shader.h"

class ResourceManager {
public:
    static auto get_instance() -> ResourceManager*;

    auto add_shader(const std::string& name, std::shared_ptr<Shader> shader) -> std::shared_ptr<Shader>;
    auto get_shader(const std::string& name) -> std::shared_ptr<Shader>;

    auto add_pipeline(const std::string& name, std::shared_ptr<Pipeline> pipeline) -> std::shared_ptr<Pipeline>;
    auto get_pipeline(const std::string& name) -> std::shared_ptr<Pipeline>;

    auto add_mesh(const std::string& name, std::shared_ptr<Mesh> mesh) -> std::shared_ptr<Mesh>;
    auto get_mesh(const std::string& name) -> std::shared_ptr<Mesh>;

private:
    ResourceManager();
    static ResourceManager* instance;
    std::map<std::string, std::shared_ptr<Shader>> shaders = {};
    std::map<std::string, std::shared_ptr<Pipeline>> pipelines = {};
    std::map<std::string, std::shared_ptr<Mesh>> meshes = {};
};
