#pragma once

#include <vector>

#include "Renderable.h"

class Scene {
public:
    auto add_renderable(std::shared_ptr<Renderable> renderable) -> void;
    auto update(float delta_time, float rotation_speed) -> void;
    auto render(const glm::mat4& view_proj) -> void;

private:
    std::vector<std::shared_ptr<Renderable>> renderables;
};
