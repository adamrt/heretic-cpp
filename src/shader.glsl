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
    vec4 u_ambient_color;
};

uniform fs_light_params {
    vec4 position[3];
    vec4 color[3];
} dir_lights;

uniform texture2D tex;
uniform sampler smp;

in vec4 v_position;
in vec3 v_normal;
in vec2 v_uv;
in vec4 v_color;

out vec4 frag_color;

void main() {
    vec3 norm = normalize(v_normal);
    vec4 diffuse_light = vec4(0.0, 0.0, 0.0, 1.0);
    vec4 tex_color = texture(sampler2D(tex, smp), v_uv);

    for (int i = 0; i < 3; i++) {
        vec3 direction = normalize(dir_lights.position[i].xyz - v_position.xyz);
        float intensity = max(dot(norm, direction), 0.0);
        diffuse_light += dir_lights.color[i] * intensity;
    }
    frag_color = (u_ambient_color + diffuse_light) * tex_color;
}
@end

@program standard vs fs
