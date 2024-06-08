
#include "Model.h"
#include "State.h"

Model::~Model()
{
    sg_destroy_pipeline(pipeline);
    sg_destroy_shader(shader);

    std::cout << "Destroying Model" << std::endl;
}

Model::Model(std::shared_ptr<Mesh> _mesh)
    : mesh(_mesh)
{
    // Light Shader
    shader = sg_make_shader(colored_shader_desc(sg_query_backend()));

    sg_pipeline_desc pip_desc = {};
    pip_desc.shader = shader;
    pip_desc.cull_mode = SG_CULLMODE_BACK;
    pip_desc.face_winding = SG_FACEWINDING_CCW;
    pip_desc.label = "pipeline";
    // Unnecessary if data is contiguous
    // pip_desc.layout.buffers[0].stride = 48;
    pip_desc.layout.attrs[ATTR_vs_standard_a_position].format = SG_VERTEXFORMAT_FLOAT3;
    pip_desc.layout.attrs[ATTR_vs_standard_a_normal].format = SG_VERTEXFORMAT_FLOAT3;
    pip_desc.layout.attrs[ATTR_vs_standard_a_uv].format = SG_VERTEXFORMAT_FLOAT2;
    pip_desc.depth.write_enabled = true;
    pip_desc.depth.compare = SG_COMPAREFUNC_LESS_EQUAL;

    pipeline = sg_make_pipeline(&pip_desc);

    bindings.vertex_buffers[0] = mesh->vertex_buffer;
}

Model::Model(std::shared_ptr<Mesh> _mesh, std::shared_ptr<Texture> _texture)
    : mesh(_mesh)
    , texture(_texture)
{
    // Standard Shader
    shader = sg_make_shader(textured_shader_desc(sg_query_backend()));

    sg_pipeline_desc pip_desc = {};
    pip_desc.shader = shader;
    pip_desc.cull_mode = SG_CULLMODE_BACK;
    pip_desc.face_winding = SG_FACEWINDING_CCW;
    pip_desc.label = "pipeline";
    // Unnecessary if data is contiguous
    // pip_desc.layout.buffers[0].stride = 48;
    pip_desc.layout.attrs[ATTR_vs_standard_a_position].format = SG_VERTEXFORMAT_FLOAT3;
    pip_desc.layout.attrs[ATTR_vs_standard_a_normal].format = SG_VERTEXFORMAT_FLOAT3;
    pip_desc.layout.attrs[ATTR_vs_standard_a_uv].format = SG_VERTEXFORMAT_FLOAT2;
    pip_desc.depth.write_enabled = true;
    pip_desc.depth.compare = SG_COMPAREFUNC_LESS_EQUAL;

    pipeline = sg_make_pipeline(&pip_desc);

    bindings.vertex_buffers[0] = mesh->vertex_buffer;
}

auto Model::render() -> void
{
    auto state = State::get_instance();
    vs_standard_params_t vs_params;
    vs_params.u_view_proj = state->camera.view_proj();
    vs_params.u_model = model_matrix;

    fs_textured_params_t fs_params;
    fs_params.u_ambient_color = state->ambient_color;
    fs_params.u_ambient_strength = state->ambient_strength;
    fs_params.u_render_mode = state->render_mode;
    fs_params.u_use_lighting = state->use_lighting;

    fs_params.u_light_count = state->scene.lights.size();
    for (size_t i = 0; i < state->scene.lights.size(); i++) {
        fs_params.u_light_colors[i] = state->scene.lights[i]->color;
        fs_params.u_light_positions[i] = glm::vec4(state->scene.lights[i]->translation, 1.0f);
    }

    bindings.fs.images[SLOT_tex] = texture->image;
    bindings.fs.samplers[SLOT_smp] = texture->sampler;

    sg_apply_pipeline(pipeline);
    sg_apply_bindings(&bindings);

    sg_range vs_range = SG_RANGE(vs_params);
    sg_range fs_range = SG_RANGE(fs_params);
    sg_apply_uniforms(SG_SHADERSTAGE_VS, SLOT_vs_standard_params, &vs_range);
    sg_apply_uniforms(SG_SHADERSTAGE_FS, SLOT_fs_textured_params, &fs_range);
    sg_draw(0, mesh->num_vertices, 1);
}

auto Model::update(float delta_time) -> void
{
    (void)delta_time;
    model_matrix = glm::mat4(1.0f);
    model_matrix = glm::scale(model_matrix, scale);
    model_matrix = glm::rotate(model_matrix, rotation.x, glm::vec3(1.0f, 0.0f, 0.0f));
    model_matrix = glm::rotate(model_matrix, rotation.y, glm::vec3(0.0f, 1.0f, 0.0f));
    model_matrix = glm::rotate(model_matrix, rotation.z, glm::vec3(0.0f, 0.0f, 1.0f));
    model_matrix = glm::translate(model_matrix, translation);
}
