@ctype mat4 glm::mat4
@ctype vec4 glm::vec4

@vs vs_standard
uniform vs_standard_params{
    mat4 u_view_proj;
    mat4 u_model;
};

in vec3 a_position;
in vec3 a_normal;
in vec2 a_uv;
in float a_palette_index;

out vec4 v_position;
out vec3 v_normal;
out vec2 v_uv;
out float v_palette_index;

void main() {
    v_position = u_model * vec4(a_position, 1.0);
    v_normal = mat3(transpose(inverse(u_model))) * a_normal;
    if (length(v_normal) > 0.0) {
        v_normal = normalize(v_normal);
    } else {
        v_normal = vec3(0.0, 0.0, 0.0);
    }

    v_uv = a_uv;
    v_palette_index = a_palette_index;
    gl_Position = u_view_proj * u_model * vec4(a_position, 1.0);
}
@end

@vs vs_background
in vec3 a_position;

out vec2 v_uv;

void main() {
    gl_Position = vec4(a_position, 1.0);
    v_uv = a_position.xy;
}
@end

@fs fs_textured
uniform fs_textured_params {
    int   u_render_mode;

    int   u_use_lighting;
    vec4  u_ambient_color;
    float u_ambient_strength;
    vec4  u_light_positions[10];
    vec4  u_light_colors[10];
    int   u_light_count;
};

uniform texture2D tex;
uniform sampler smp;

in vec4 v_position;
in vec3 v_normal;
in vec2 v_uv;
in float v_palette_index;

out vec4 frag_color;

void main() {
    vec4 out_color;

    vec3 norm = normalize(v_normal);
    vec4 diffuse_light = vec4(0.0, 0.0, 0.0, 1.0);
    for (int i = 0; i < u_light_count; i++) {
        vec3 direction = normalize(u_light_positions[i].xyz - v_position.xyz);
        float intensity = max(dot(norm, direction), 0.0);
        diffuse_light += u_light_colors[i] * intensity;
    }
    vec4 light = u_ambient_color * u_ambient_strength + diffuse_light;

    if (u_render_mode == 0) { // Textured
        out_color = texture(sampler2D(tex, smp), v_uv);
    } else if (u_render_mode == 1) { // White
        out_color = vec4(1.0, 1.0, 1.0 , 1.0);
    } else if (u_render_mode == 2) { // Normals
        out_color = vec4(v_normal, 1.0);
    }

    // Don't use light for normals
    if (u_render_mode != 2 && u_use_lighting == 1) {
        out_color = out_color * light;
    }

    frag_color = out_color;
}
@end

@fs fs_paletted
uniform fs_paletted_params{
    int   u_render_mode;

    int   u_use_lighting;
    vec4  u_ambient_color;
    float u_ambient_strength;
    vec4  u_light_positions[10];
    vec4  u_light_colors[10];
    int   u_light_count;
};

uniform texture2D tex;
uniform texture2D palette;
uniform sampler smp;

in vec4 v_position;
in vec3 v_normal;
in vec2 v_uv;
in float v_palette_index;

out vec4 frag_color;

void main() {
    vec4 out_color;

    vec3 norm = normalize(v_normal);
    vec4 diffuse_light = vec4(0.0, 0.0, 0.0, 1.0);
    for (int i = 0; i < u_light_count; i++) {
        vec3 direction = normalize(u_light_positions[i].xyz - v_position.xyz);
        float intensity = max(dot(norm, direction), 0.0);
        diffuse_light += u_light_colors[i] * intensity;
    }
    vec4 light = u_ambient_color * u_ambient_strength + diffuse_light;

    // Draw black for triangles without normals (untextured triangles)
    if (v_normal.x + v_normal.y + v_normal.z + v_uv.x + v_uv.y == 0.0) {
        // Draw black for things without normals and uv coords.
        // The uv coords and normal could actually be 0 so we check them both.
        frag_color = light * vec4(0.0, 0.0, 0.0, 1.0);
        return;
    }

    if (u_render_mode == 0) { // Paletted
        // This has to be 256.0 instead of 255 (really 255.1 is fine).
        // And palette_pos needs to be calculated then cast to uint,
        // not casting each to uint then calculating. Otherwise there
        // will be distortion in perspective projection on some gpus.
        vec4 tex_color = texture(sampler2D(tex, smp), v_uv) * 255.0;
        uint palette_pos = uint(v_palette_index * 16 + tex_color.r);
        vec4 color = texture(sampler2D(palette, smp), vec2(float(palette_pos) / 255.0, 0.0));
        if (color.a < 0.5)
            discard;
        out_color = color;
    } else if (u_render_mode == 1) { // White
        out_color = vec4(1.0, 1.0, 1.0 , 1.0);
    } else if (u_render_mode == 2) { // Normals
        out_color = vec4(v_normal, 1.0);
    }

    // Don't use light for normals
    if (u_render_mode != 2 && u_use_lighting == 1) {
        out_color = out_color * light;
    }

    frag_color = out_color;
}
@end

@fs fs_colored
uniform fs_colored_params {
    vec4 u_color;
};

in vec4 v_position;
in vec3 v_normal;
in vec2 v_uv;
in float v_palette_index;

out vec4 frag_color;

void main() {
     frag_color = u_color;
}
@end

@fs fs_background
uniform fs_background_params {
    vec4 u_top_color;
    vec4 u_bottom_color;
};

in vec2 v_uv;

out vec4 frag_color;

void main() {
    frag_color = mix(u_bottom_color, u_top_color, v_uv.y);
}
@end

@program textured   vs_standard   fs_textured
@program paletted   vs_standard   fs_paletted
@program colored    vs_standard   fs_colored
@program background vs_background fs_background
