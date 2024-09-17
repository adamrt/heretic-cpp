#include <algorithm>

#include "Model.h"
#include "ResourceManager.h"
#include "State.h"

auto Model::update(float delta_time) -> void
{
    (void)delta_time;
    model_matrix = glm::mat4(1.0f);
    model_matrix = glm::translate(model_matrix, translation);
    model_matrix = glm::rotate(model_matrix, rotation.x, glm::vec3(1.0f, 0.0f, 0.0f));
    model_matrix = glm::rotate(model_matrix, rotation.y, glm::vec3(0.0f, 1.0f, 0.0f));
    model_matrix = glm::rotate(model_matrix, rotation.z, glm::vec3(0.0f, 0.0f, 1.0f));
    model_matrix = glm::scale(model_matrix, scale);
}

ColoredModel::ColoredModel(std::shared_ptr<Mesh> _mesh, glm::vec4 _color, glm::vec3 _position)
    : Model(_position)
    , color(_color)
{
    auto resources = ResourceManager::get_instance();

    mesh = _mesh;
    pipeline = resources->get_pipeline("colored")->get_pipeline();
    bindings.vertex_buffers[0] = mesh->vertex_buffer;
}

auto ColoredModel::render() -> void
{
    auto state = State::get_instance();
    vs_standard_params_t vs_params;
    vs_params.u_view_proj = state->camera.view_proj();
    vs_params.u_model = model_matrix;

    fs_colored_params_t fs_params;
    fs_params.u_color = color;

    sg_apply_pipeline(pipeline);
    sg_apply_bindings(&bindings);

    sg_range vs_range = SG_RANGE(vs_params);
    sg_range fs_range = SG_RANGE(fs_params);
    sg_apply_uniforms(SG_SHADERSTAGE_VS, SLOT_vs_standard_params, &vs_range);
    sg_apply_uniforms(SG_SHADERSTAGE_FS, SLOT_fs_colored_params, &fs_range);
    sg_draw(0, mesh->vertices.size(), 1);
}

Light::Light(std::shared_ptr<Mesh> _mesh, glm::vec4 _color, glm::vec3 _position)
    : ColoredModel(_mesh, _color, _position)
{
    scale = glm::vec3(0.3f);
};

TexturedModel::TexturedModel(std::shared_ptr<Mesh> _mesh, std::shared_ptr<Texture> _texture, glm::vec3 _position)
    : Model(_position)
    , texture(_texture)
{
    auto resources = ResourceManager::get_instance();

    mesh = _mesh;
    pipeline = resources->get_pipeline("textured")->get_pipeline();
    bindings.vertex_buffers[0] = mesh->vertex_buffer;
}

auto TexturedModel::render() -> void
{
    auto state = State::get_instance();
    vs_standard_params_t vs_params;
    vs_params.u_view_proj = state->camera.view_proj();
    vs_params.u_model = model_matrix;

    fs_textured_params_t fs_params;
    fs_params.u_ambient_color = state->scene.ambient_color;
    fs_params.u_ambient_strength = state->scene.ambient_strength;
    fs_params.u_render_mode = state->renderer.render_mode;
    fs_params.u_use_lighting = state->scene.use_lighting;

    std::vector<std::shared_ptr<Light>> enabled_lights;

    std::copy_if(state->scene.lights.begin(), state->scene.lights.end(), std::back_inserter(enabled_lights),
        [](const std::shared_ptr<Light>& item) { return item->is_enabled; });

    fs_params.u_light_count = enabled_lights.size();
    for (size_t i = 0; i < enabled_lights.size(); i++) {
        fs_params.u_light_colors[i] = enabled_lights[i]->color;
        fs_params.u_light_positions[i] = glm::vec4(enabled_lights[i]->translation, 1.0f);
    }

    bindings.fs.images[SLOT_tex] = texture->image;
    bindings.fs.samplers[SLOT_smp] = texture->sampler->sampler;

    sg_apply_pipeline(pipeline);
    sg_apply_bindings(&bindings);

    sg_range vs_range = SG_RANGE(vs_params);
    sg_range fs_range = SG_RANGE(fs_params);
    sg_apply_uniforms(SG_SHADERSTAGE_VS, SLOT_vs_standard_params, &vs_range);
    sg_apply_uniforms(SG_SHADERSTAGE_FS, SLOT_fs_textured_params, &fs_range);
    sg_draw(0, mesh->vertices.size(), 1);
}

PalettedModel::PalettedModel(std::shared_ptr<Mesh> _mesh, std::shared_ptr<Texture> _texture, std::shared_ptr<Texture> _palette, glm::vec3 _position)
    : Model(_position)
    , texture(_texture)
    , palette(_palette)
{
    auto resources = ResourceManager::get_instance();

    mesh = _mesh;
    pipeline = resources->get_pipeline("paletted")->get_pipeline();
    bindings.vertex_buffers[0] = mesh->vertex_buffer;
}

auto PalettedModel::render() -> void
{
    auto state = State::get_instance();
    vs_standard_params_t vs_params;
    vs_params.u_view_proj = state->camera.view_proj();
    vs_params.u_model = model_matrix;

    fs_paletted_params_t fs_params;
    fs_params.u_ambient_color = state->scene.ambient_color;
    fs_params.u_ambient_strength = state->scene.ambient_strength;
    fs_params.u_render_mode = state->renderer.render_mode;
    fs_params.u_use_lighting = state->scene.use_lighting;

    std::vector<std::shared_ptr<Light>> enabled_lights;

    std::copy_if(state->scene.lights.begin(), state->scene.lights.end(), std::back_inserter(enabled_lights),
        [](const std::shared_ptr<Light>& item) { return item->is_enabled; });

    fs_params.u_light_count = enabled_lights.size();
    for (size_t i = 0; i < enabled_lights.size(); i++) {
        fs_params.u_light_colors[i] = enabled_lights[i]->color;
        fs_params.u_light_positions[i] = glm::vec4(enabled_lights[i]->translation, 1.0f);
    }

    bindings.fs.images[SLOT_tex] = texture->image;
    bindings.fs.images[SLOT_palette] = palette->image;
    bindings.fs.samplers[SLOT_smp] = texture->sampler->sampler;

    sg_apply_pipeline(pipeline);
    sg_apply_bindings(&bindings);

    sg_range vs_range = SG_RANGE(vs_params);
    sg_range fs_range = SG_RANGE(fs_params);
    sg_apply_uniforms(SG_SHADERSTAGE_VS, SLOT_vs_standard_params, &vs_range);
    sg_apply_uniforms(SG_SHADERSTAGE_FS, SLOT_fs_textured_params, &fs_range);
    sg_draw(0, mesh->vertices.size(), 1);
}

Background::Background(std::pair<glm::vec4, glm::vec4> background)
    : Background(background.first, background.second)
{
}

Background::Background(glm::vec4 _top, glm::vec4 _bottom)
    : Model(glm::vec3 { 0, 0, 0 })
    , top(_top)
    , bottom(_bottom)
{
    auto resources = ResourceManager::get_instance();

    mesh = resources->get_mesh("background");
    pipeline = resources->get_pipeline("background")->get_pipeline();
    bindings.vertex_buffers[0] = mesh->vertex_buffer;
}

auto Background::render() -> void
{
    fs_background_params_t fs_params;
    fs_params.u_top_color = top;
    fs_params.u_bottom_color = bottom;

    sg_apply_pipeline(pipeline);
    sg_apply_bindings(&bindings);

    sg_range fs_range = SG_RANGE(fs_params);
    sg_apply_uniforms(SG_SHADERSTAGE_FS, SLOT_fs_background_params, &fs_range);
    sg_draw(0, mesh->vertices_float.size(), 1);
}
