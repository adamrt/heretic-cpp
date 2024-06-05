
#include "Renderable.h"

float random_float(float min, float max)
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(min, max);
    return dis(gen);
}

Renderable::~Renderable()
{
    sg_destroy_pipeline(pipeline);
    sg_destroy_shader(shader);

    std::cout << "Destroying Renderable" << std::endl;
}

Renderable::Renderable(State& _state, std::shared_ptr<Mesh> resources, std::shared_ptr<Texture> tex)
    : state(_state)
    , shared_resources(resources)
    , texture(tex)
{
    shader = sg_make_shader(standard_shader_desc(sg_query_backend()));

    sg_pipeline_desc pip_desc = {};
    pip_desc.shader = shader;
    pip_desc.cull_mode = SG_CULLMODE_BACK;
    pip_desc.face_winding = SG_FACEWINDING_CCW;
    pip_desc.label = "pipeline";
    // Unnecessary if data is contiguous
    // pip_desc.layout.buffers[0].stride = 48;
    pip_desc.layout.attrs[ATTR_vs_a_position].format = SG_VERTEXFORMAT_FLOAT3;
    pip_desc.layout.attrs[ATTR_vs_a_normal].format = SG_VERTEXFORMAT_FLOAT3;
    pip_desc.layout.attrs[ATTR_vs_a_uv].format = SG_VERTEXFORMAT_FLOAT2;
    pip_desc.layout.attrs[ATTR_vs_a_color].format = SG_VERTEXFORMAT_FLOAT4;
    pip_desc.depth.write_enabled = true;
    pip_desc.depth.compare = SG_COMPAREFUNC_LESS_EQUAL;

    pipeline = sg_make_pipeline(&pip_desc);

    bindings.vertex_buffers[0] = resources->vertex_buffer;

    rspeed = random_float(-5.0f, 5.0f);
}

auto Renderable::render(const glm::mat4& view_proj) -> void
{
    vs_standard_params_t vs_params;
    vs_params.u_view_proj = view_proj;
    vs_params.u_model = model_matrix;

    fs_standard_params_t fs_params;
    fs_params.u_ambient_color = state.ambient_color;
    fs_params.u_ambient_strength = state.ambient_strength;
    fs_params.u_render_mode = state.render_mode;
    fs_params.u_use_lighting = state.use_lighting;

    fs_light_params_t light_params;
    light_params.color[0] = state.lights[0].color;
    light_params.color[1] = state.lights[1].color;
    light_params.color[2] = state.lights[2].color;
    light_params.position[0] = state.lights[0].position;
    light_params.position[1] = state.lights[1].position;
    light_params.position[2] = state.lights[2].position;

    bindings.fs.images[SLOT_tex] = texture->image;
    bindings.fs.samplers[SLOT_smp] = texture->sampler;

    sg_apply_pipeline(pipeline);
    sg_apply_bindings(&bindings);

    sg_range vs_range = SG_RANGE(vs_params);
    sg_range fs_range = SG_RANGE(fs_params);
    sg_range light_range = SG_RANGE(light_params);
    sg_apply_uniforms(SG_SHADERSTAGE_VS, SLOT_vs_standard_params, &vs_range);
    sg_apply_uniforms(SG_SHADERSTAGE_FS, SLOT_fs_standard_params, &fs_range);
    sg_apply_uniforms(SG_SHADERSTAGE_FS, SLOT_fs_light_params, &light_range);
    sg_draw(0, shared_resources->num_indices, 1);
}

auto Renderable::update(float delta_time, float rotation_speed) -> void
{
    auto x = 1.0f * delta_time * rspeed * rotation_speed;
    auto y = 2.0f * delta_time * rspeed * rotation_speed;

    rotate(x, glm::vec3(1.0f, 0.0f, 0.0f));
    rotate(y, glm::vec3(0.0f, 1.0f, 0.0f));
}

auto Renderable::set_model_matrix(const glm::mat4& matrix) -> void
{
    model_matrix = matrix;
}

auto Renderable::translate(const glm::vec3& translation) -> void
{
    model_matrix = glm::translate(model_matrix, translation);
}

auto Renderable::rotate(float angle, const glm::vec3& axis) -> void
{
    model_matrix = glm::rotate(model_matrix, angle, axis);
}

auto Renderable::scale(const glm::vec3& scaling_factors) -> void
{
    model_matrix = glm::scale(model_matrix, scaling_factors);
}

auto Renderable::scale(float f) -> void
{
    scale(glm::vec3 { f, f, f });
}
