#include "Pipeline.h"
#include "ResourceManager.h"

#include "glm/glm.hpp"
#include "sokol_gfx.h"

#include "shader.glsl.h"

Pipeline::Pipeline(std::shared_ptr<Shader> shader)
{
    auto desc = Pipeline::standard_desc();
    desc.shader = shader->get_shader();
    pipeline = sg_make_pipeline(desc);
}

Pipeline::~Pipeline()
{
    sg_destroy_pipeline(pipeline);
}

// standard_desc is the description we use for basically everything.
// It is missing the shader which will need to be added.
auto Pipeline::standard_desc() -> sg_pipeline_desc
{

    sg_pipeline_desc desc = {};
    // desc.shader must be populated before sg_make_pipeline();
    desc.cull_mode = SG_CULLMODE_BACK;
    desc.face_winding = SG_FACEWINDING_CCW;
    desc.label = "pipeline";
    desc.layout.attrs[ATTR_vs_standard_a_position].format = SG_VERTEXFORMAT_FLOAT3;
    desc.layout.attrs[ATTR_vs_standard_a_normal].format = SG_VERTEXFORMAT_FLOAT3;
    desc.layout.attrs[ATTR_vs_standard_a_uv].format = SG_VERTEXFORMAT_FLOAT2;
    desc.layout.attrs[ATTR_vs_standard_a_palette_index].format = SG_VERTEXFORMAT_FLOAT;
    desc.depth.write_enabled = true;
    desc.depth.compare = SG_COMPAREFUNC_LESS_EQUAL;
    return desc;
}
