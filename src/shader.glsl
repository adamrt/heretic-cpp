@ctype mat4 glm::mat4
@ctype vec4 glm::vec4

@vs vs
uniform vs_standard_params{
    mat4 u_view_proj;
    mat4 u_model;
};

in vec3 a_position;
in vec3 a_normal;
in vec2 a_uv;
in vec4 a_color;

out vec4 v_position;
out vec3 v_normal;
out vec2 v_uv;
out vec4 v_color;

void main() {
    v_position = u_model * vec4(a_position, 1.0);
    v_normal = mat3(transpose(inverse(u_model))) * a_normal;
    v_uv = a_uv;
    v_color = a_color;
    gl_Position = u_view_proj * u_model * vec4(a_position, 1.0);
}
@end

@fs fs
uniform fs_standard_params {
    vec4  u_ambient_color;
    float u_ambient_strength;
    int   u_use_lighting;
    int   u_render_mode;
};

uniform fs_light_params {
    vec4 position[10];
    vec4 color[10];
    int  u_num_lights;
} dir_lights;

uniform texture2D tex;
uniform sampler smp;

in vec4 v_position;
in vec3 v_normal;
in vec2 v_uv;
in vec4 v_color;

out vec4 frag_color;

void main() {
    vec4 out_color;

    if (u_render_mode == 0) { // Textured
        out_color = texture(sampler2D(tex, smp), v_uv);
    } else if (u_render_mode == 1) { // Colored
        out_color = v_color;
    } else if (u_render_mode == 2) { // Normals
        out_color = vec4(v_normal, 1.0);
    }

    if (u_render_mode == 2) {
        frag_color = out_color;
    } else {
        if (u_use_lighting == 1) {
            vec3 norm = normalize(v_normal);
            vec4 diffuse_light = vec4(0.0, 0.0, 0.0, 1.0);
            for (int i = 0; i < dir_lights.u_num_lights; i++) {
                vec3 direction = normalize(dir_lights.position[i].xyz - v_position.xyz);
                float intensity = max(dot(norm, direction), 0.0);
                diffuse_light += dir_lights.color[i] * intensity;
            }
            vec4 ambient = u_ambient_color * u_ambient_strength;
            frag_color = (ambient + diffuse_light) * out_color;
        } else {
            frag_color = out_color;
        }
    }
}
@end

@fs light_fs
in vec4 v_position;
in vec3 v_normal;
in vec2 v_uv;
in vec4 v_color;

out vec4 frag_color;

void main() {
     frag_color = v_color;
}
@end

@program standard vs fs
@program light    vs light_fs
