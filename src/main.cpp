#define SOKOL_IMPL
#define SOKOL_GLCORE
#include "sokol_app.h"
#include "sokol_gfx.h"
#include "sokol_glue.h"
#include "sokol_log.h"

#define SOKOL_IMGUI_IMPL
#include "imgui.h"
#include "sokol_imgui.h"

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

#include "shader.glsl.h"

enum class Projection : int {
    Perspective = 0,
    Orthographic = 1,
};

class Camera {
public:
    static constexpr float NEARZ = 0.01f;
    static constexpr float FARZ = 100.0f;
    static constexpr float MIN_DIST = 2.5f;
    static constexpr float MAX_DIST = 100.0f;
    static constexpr float MIN_LAT = -85.0f;
    static constexpr float MAX_LAT = 85.0f;

    Projection projection = Projection::Perspective;

    auto handle_event(const sapp_event* ev) -> void;
    auto update() -> void;

    auto view_matrix() const -> glm::mat4 { return _view; }
    auto proj_matrix() const -> glm::mat4 { return _proj; }

private:
    auto orbit(float dx, float dy) -> void;
    auto zoom(float d) -> void;
    auto euclidean(float latitude, float longitude) -> glm::vec3;

    float _fov = 60.0f;
    float _distance = 5.0f;
    float _latitude = 30.f;
    float _longitude = 30.0f;
    glm::vec3 _eye;
    glm::vec3 _target;
    glm::mat4 _view;
    glm::mat4 _proj;
};

auto Camera::orbit(float dx, float dy) -> void
{
    _longitude -= dx;
    if (_longitude < 0.0f) {
        _longitude += 360.0f;
    }
    if (_longitude > 360.0f) {
        _longitude -= 360.0f;
    }

    _latitude = glm::clamp(_latitude + dy, MIN_LAT, MAX_LAT);
}

auto Camera::zoom(float d) -> void
{
    _distance = glm::clamp(_distance + d, MIN_DIST, MAX_DIST);
}

auto Camera::euclidean(float latitude, float longitude) -> glm::vec3
{
    const float lat = glm::radians(latitude);
    const float lng = glm::radians(longitude);
    return glm::vec3 { cosf(lat) * sinf(lng), sinf(lat), cosf(lat) * cosf(lng) };
}

auto Camera::update() -> void
{
    _eye = _target + euclidean(_latitude, _longitude) * _distance;
    _view = glm::lookAt(_eye, _target, glm::vec3 { 0.0f, 1.0f, 0.0f });

    const float aspect = sapp_widthf() / sapp_heightf();

    if (projection == Projection::Perspective) {
        _proj = glm::perspective(glm::radians(_fov), aspect, NEARZ, FARZ);
    } else {
        const float w = 1.0f * _distance;
        const float h = w / aspect;
        _proj = glm::ortho(-w, w, -h, h, NEARZ, FARZ);
    }
}

auto Camera::handle_event(const sapp_event* event) -> void
{
    switch (event->type) {
    case SAPP_EVENTTYPE_MOUSE_DOWN:
        if (event->mouse_button == SAPP_MOUSEBUTTON_LEFT) {
            sapp_lock_mouse(true);
        }
        break;
    case SAPP_EVENTTYPE_MOUSE_UP:
        if (event->mouse_button == SAPP_MOUSEBUTTON_LEFT) {
            sapp_lock_mouse(false);
        }
        break;
    case SAPP_EVENTTYPE_MOUSE_SCROLL:
        zoom(event->scroll_y * -0.5f);
        break;
    case SAPP_EVENTTYPE_MOUSE_MOVE:
        if (sapp_mouse_locked()) {
            orbit(event->mouse_dx * 0.25f, event->mouse_dy * 0.25f);
        }
        break;
    default:
        break;
    }
}

static struct {
    float rotation_speed = 1.0f;
    float rx, ry;

    Camera camera;

    struct {
        sg_color clear_color = { 0.0f, 0.5f, 0.7f, 1.0f };
        sg_pass_action pass_action;
        sg_pipeline pip;
        sg_bindings bind;
    } gfx;
} state;

void gfx_init()
{
    sg_desc desc = {};
    desc.environment = sglue_environment();
    desc.logger.func = slog_func;
    sg_setup(&desc);

    // clang-format off
    float vertices[] = {
        -1.0, -1.0, -1.0,   1.0, 0.0, 0.0, 1.0,
         1.0, -1.0, -1.0,   1.0, 0.0, 0.0, 1.0,
         1.0,  1.0, -1.0,   1.0, 0.0, 0.0, 1.0,
        -1.0,  1.0, -1.0,   1.0, 0.0, 0.0, 1.0,

        -1.0, -1.0,  1.0,   0.0, 1.0, 0.0, 1.0,
         1.0, -1.0,  1.0,   0.0, 1.0, 0.0, 1.0,
         1.0,  1.0,  1.0,   0.0, 1.0, 0.0, 1.0,
        -1.0,  1.0,  1.0,   0.0, 1.0, 0.0, 1.0,

        -1.0, -1.0, -1.0,   0.0, 0.0, 1.0, 1.0,
        -1.0,  1.0, -1.0,   0.0, 0.0, 1.0, 1.0,
        -1.0,  1.0,  1.0,   0.0, 0.0, 1.0, 1.0,
        -1.0, -1.0,  1.0,   0.0, 0.0, 1.0, 1.0,

        1.0, -1.0, -1.0,    1.0, 0.5, 0.0, 1.0,
        1.0,  1.0, -1.0,    1.0, 0.5, 0.0, 1.0,
        1.0,  1.0,  1.0,    1.0, 0.5, 0.0, 1.0,
        1.0, -1.0,  1.0,    1.0, 0.5, 0.0, 1.0,

        -1.0, -1.0, -1.0,   0.0, 0.5, 1.0, 1.0,
        -1.0, -1.0,  1.0,   0.0, 0.5, 1.0, 1.0,
         1.0, -1.0,  1.0,   0.0, 0.5, 1.0, 1.0,
         1.0, -1.0, -1.0,   0.0, 0.5, 1.0, 1.0,

        -1.0,  1.0, -1.0,   1.0, 0.0, 0.5, 1.0,
        -1.0,  1.0,  1.0,   1.0, 0.0, 0.5, 1.0,
         1.0,  1.0,  1.0,   1.0, 0.0, 0.5, 1.0,
         1.0,  1.0, -1.0,   1.0, 0.0, 0.5, 1.0
    };

    uint16_t indices[] = {
        0,   1,  2,  0,  2,  3,
        6,   5,  4,  7,  6,  4,
        8,   9, 10,  8, 10, 11,
        14, 13, 12, 15, 14, 12,
        16, 17, 18, 16, 18, 19,
        22, 21, 20, 23, 22, 20
    };
    // clang-format on

    sg_shader shd = sg_make_shader(cube_shader_desc(sg_query_backend()));

    sg_buffer_desc vbuf_desc = {};
    vbuf_desc.data = SG_RANGE(vertices);
    vbuf_desc.label = "cube-vertices";
    sg_buffer vbuf = sg_make_buffer(&vbuf_desc);

    sg_buffer_desc ibuf_desc = {};
    ibuf_desc.type = SG_BUFFERTYPE_INDEXBUFFER;
    ibuf_desc.data = SG_RANGE(indices);
    ibuf_desc.label = "cube-indices";
    sg_buffer ibuf = sg_make_buffer(&ibuf_desc);

    sg_pipeline_desc pip_desc = {};
    pip_desc.shader = shd;
    pip_desc.index_type = SG_INDEXTYPE_UINT16;
    pip_desc.cull_mode = SG_CULLMODE_BACK;
    pip_desc.label = "cupe-pipeline";

    pip_desc.layout = {};
    pip_desc.layout.buffers[0].stride = 28;
    pip_desc.layout.attrs[ATTR_vs_position].format = SG_VERTEXFORMAT_FLOAT3;
    pip_desc.layout.attrs[ATTR_vs_color0].format = SG_VERTEXFORMAT_FLOAT4;

    pip_desc.depth = {};
    pip_desc.depth.write_enabled = true;
    pip_desc.depth.compare = SG_COMPAREFUNC_LESS_EQUAL;
    sg_pipeline pip = sg_make_pipeline(&pip_desc);

    sg_bindings bind_desc = {};
    bind_desc.vertex_buffers[0] = vbuf;
    bind_desc.index_buffer = ibuf;

    state.gfx.pass_action.colors[0].load_action = SG_LOADACTION_CLEAR;
    state.gfx.pip = pip;
    state.gfx.bind = bind_desc;
}

void gui_init()
{
    simgui_desc_t simgui_desc = {};
    simgui_desc.logger.func = slog_func;
    simgui_setup(&simgui_desc);
}

void init()
{
    gfx_init();
    gui_init();
}

void gui_draw()
{
    ImGui::SetNextWindowSize(ImVec2(0, 0));

    ImGui::Begin("Hello, world!");
    if (ImGui::RadioButton("Perspective", state.camera.projection == Projection::Perspective)) {
        state.camera.projection = Projection::Perspective;
    }
    ImGui::SameLine();
    if (ImGui::RadioButton("Orthographic", state.camera.projection == Projection::Orthographic)) {
        state.camera.projection = Projection::Orthographic;
    }
    ImGui::SliderFloat("Rotation", &state.rotation_speed, 0.0f, 2.0f);
    ImGui::ColorEdit3("Background", &state.gfx.clear_color.r);
    if (ImGui::Button(sapp_is_fullscreen() ? "Switch to windowed" : "Switch to fullscreen")) {
        sapp_toggle_fullscreen();
    }
    ImGui::End();
}

void frame()
{
    state.camera.update();

    simgui_new_frame({
        sapp_width(),
        sapp_height(),
        sapp_frame_duration(),
        sapp_dpi_scale(),
    });

    vs_params_t vs_params;
    const float t = (float)sapp_frame_duration();
    state.rx += 1.0f * t * state.rotation_speed;
    state.ry += 2.0f * t * state.rotation_speed;
    glm::mat4 rxm = glm::rotate(glm::identity<glm::mat4>(), state.rx, glm::vec3(1.0f, 0.0f, 0.0f));
    glm::mat4 rym = glm::rotate(glm::identity<glm::mat4>(), state.ry, glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 model = rxm * rym;
    vs_params.mvp = state.camera.proj_matrix() * state.camera.view_matrix() * model;
    sg_range vs_range = SG_RANGE(vs_params);

    state.gfx.pass_action.colors[0].clear_value = state.gfx.clear_color;

    sg_pass pass = {};
    pass.action = state.gfx.pass_action;
    pass.swapchain = sglue_swapchain();

    sg_begin_pass(&pass);
    {
        sg_apply_pipeline(state.gfx.pip);
        sg_apply_bindings(&state.gfx.bind);
        sg_apply_uniforms(SG_SHADERSTAGE_VS, SLOT_vs_params, &vs_range);
        sg_draw(0, 36, 1);

        gui_draw();
        simgui_render();
    }
    sg_end_pass();

    sg_commit();
}

void cleanup()
{
    simgui_shutdown();
    sg_shutdown();
}

void input(sapp_event const* event)
{
    if (event->type == SAPP_EVENTTYPE_KEY_DOWN) {
        if (event->key_code == SAPP_KEYCODE_ESCAPE) {
            sapp_quit();
        }
    }

    if (simgui_handle_event(event)) {
        return;
    }

    state.camera.handle_event(event);
}

sapp_desc sokol_main(int argc, char* argv[])
{
    (void)argc;
    (void)argv;
    sapp_desc desc = {};
    desc.init_cb = init;
    desc.frame_cb = frame;
    desc.cleanup_cb = cleanup;
    desc.event_cb = input;
    desc.window_title = "Dear ImGui (sokol-app)";
    desc.ios_keyboard_resizes_canvas = false;
    desc.icon.sokol_default = true;
    desc.enable_clipboard = true;
    desc.logger.func = slog_func;
    return desc;
}
