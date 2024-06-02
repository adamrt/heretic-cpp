#pragma once

#include <vector>

#include "Renderable.h"

class Scene {
public:
    auto add_renderable(const Renderable& renderable) -> void;
    auto update(float delta_time, float rotation_speed) -> void;
    auto render(const glm::mat4& view_proj) -> void;

private:
    std::vector<Renderable> renderables;
};
