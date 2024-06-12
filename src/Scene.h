#pragma once

#include <vector>

#include "Model.h"

class Scene {
public:
    auto add_model(std::shared_ptr<Model> renderable) -> void;
    auto add_light(std::shared_ptr<Light> light) -> void;
    auto update(float delta_time) -> void;
    auto render() -> void;
    auto clear() -> void;

    std::vector<std::shared_ptr<Model>> models = {};
    std::vector<std::shared_ptr<Light>> lights = {};
};
