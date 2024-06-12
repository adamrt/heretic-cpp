#include "Scene.h"
#include "Model.h"

auto Scene::add_model(std::shared_ptr<Model> model) -> void
{
    models.push_back(model);
}

auto Scene::add_light(std::shared_ptr<Light> light) -> void
{
    lights.push_back(light);
}

auto Scene::update(float delta_time) -> void
{
    for (auto model : models) {
        model->update(delta_time);
    }
    for (auto light : lights) {
        light->update(delta_time);
    }
}

auto Scene::render() -> void
{
    for (auto& model : models) {
        model->render();
    }

    for (auto& light : lights) {
        light->render();
    }
}

auto Scene::clear() -> void
{
    models.clear();
    lights.clear();
}
