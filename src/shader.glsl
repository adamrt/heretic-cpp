@ctype mat4 glm::mat4
@ctype vec4 glm::vec4

@vs vs
uniform vs_params {
    mat4 mvp;
};

in vec3 a_position;
in vec3 a_normal;
in vec2 a_uv;
in vec4 a_color;

out vec3 v_normal;
out vec2 v_uv;
out vec4 v_color;

void main() {
    gl_Position = mvp * vec4(a_position, 1.0);
    v_normal = a_normal;
    v_uv = a_uv;
    v_color = a_color;
}
@end

@fs fs
in vec3 v_normal;
in vec2 v_uv;
in vec4 v_color;

out vec4 frag_color;

void main() {
    frag_color = v_color;
}
@end

@program standard vs fs
