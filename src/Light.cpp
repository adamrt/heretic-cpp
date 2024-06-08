#include "Light.h"
#include "State.h"

auto Light::render() -> void
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
    sg_draw(0, mesh->num_indices, 1);
}
