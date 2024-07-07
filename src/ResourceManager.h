#pragma once

#include <cassert>
#include <map>
#include <memory>
#include <string>

#include "BinReader.h"
#include "FFT.h"
#include "Mesh.h"
#include "Pipeline.h"
#include "Shader.h"

class ResourceManager {
public:
    static auto get_instance() -> ResourceManager*;

    auto add_shader(const std::string& name, std::shared_ptr<Shader> shader) -> std::shared_ptr<Shader>;
    auto get_shader(const std::string& name) -> std::shared_ptr<Shader>;

    auto add_sampler(const std::string& name, std::shared_ptr<Sampler> sampler) -> std::shared_ptr<Sampler>;
    auto get_sampler(const std::string& name) -> std::shared_ptr<Sampler>;

    auto add_pipeline(const std::string& name, std::shared_ptr<Pipeline> pipeline) -> std::shared_ptr<Pipeline>;
    auto get_pipeline(const std::string& name) -> std::shared_ptr<Pipeline>;

    auto add_mesh(const std::string& name, std::shared_ptr<Mesh> mesh) -> std::shared_ptr<Mesh>;
    auto get_mesh(const std::string& name) -> std::shared_ptr<Mesh>;

    auto set_bin_reader(std::shared_ptr<BinReader> bin_reader) -> std::shared_ptr<BinReader>;
    auto get_bin_reader() -> std::shared_ptr<BinReader>;

private:
    ResourceManager();
    static ResourceManager* instance;

    std::shared_ptr<BinReader> bin_reader = nullptr;

    std::map<std::string, std::shared_ptr<Shader>> shaders = {};
    std::map<std::string, std::shared_ptr<Sampler>> samplers = {};
    std::map<std::string, std::shared_ptr<Pipeline>> pipelines = {};
    std::map<std::string, std::shared_ptr<Mesh>> meshes = {};
};
