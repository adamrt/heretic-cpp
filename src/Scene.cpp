#include "Scene.h"

auto Scene::add_renderable(std::shared_ptr<Model> renderable) -> void
{
    renderables.push_back(renderable);
}

auto Scene::update(float delta_time, float rotation_speed) -> void
{
    for (auto renderable : renderables) {
        renderable->update(delta_time, rotation_speed);
    }
}

auto Scene::render() -> void
{
    for (auto& renderable : renderables) {
        renderable->render();
    }
}
