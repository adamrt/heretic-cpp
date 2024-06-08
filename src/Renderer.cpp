#define SOKOL_GLCORE
#include "sokol_app.h"
#include "sokol_gfx.h"
#include "sokol_glue.h"
#include "sokol_log.h"

#include "Renderer.h"
#include "State.h"

Renderer::Renderer()
{
    sg_desc desc = {};
    desc.environment = sglue_environment();
    desc.logger.func = slog_func;
    sg_setup(&desc);
}

Renderer::~Renderer()
{
    sg_shutdown();
}

auto Renderer::begin_frame() -> void
{
    auto state = State::get_instance();
    sg_pass pass = {};
    pass.action.colors[0].clear_value = state->clear_color;
    pass.action.colors[0].load_action = SG_LOADACTION_CLEAR;
    pass.swapchain = sglue_swapchain();
    sg_begin_pass(&pass);
}

auto Renderer::end_frame() -> void
{
    sg_end_pass();
    sg_commit();
}
