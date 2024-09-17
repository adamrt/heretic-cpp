#include "BinFile.h"
#include "ResourceManager.h"

auto BinFile::read_bytes(int num) -> std::vector<uint8_t>
{
    std::vector<uint8_t> bytes = {};
    bytes.resize(num);

    for (int i = 0; i < num; i++) {
        bytes.at(i) = read_u8();
    }
    return bytes;
}

auto BinFile::read_u8() -> uint8_t
{
    uint8_t value;
    memcpy(&value, &m_data[m_offset], sizeof(uint8_t));
    m_offset += sizeof(uint8_t);
    return value;
}

auto BinFile::read_u16() -> uint16_t
{
    uint16_t value;
    memcpy(&value, &m_data[m_offset], sizeof(uint16_t));
    m_offset += sizeof(uint16_t);
    return value;
}

auto BinFile::read_u32() -> uint32_t
{
    uint32_t value;
    memcpy(&value, &m_data[m_offset], sizeof(uint32_t));
    m_offset += sizeof(uint32_t);
    return value;
}

auto BinFile::read_i8() -> int8_t
{
    int8_t value;
    memcpy(&value, &m_data[m_offset], sizeof(int8_t));
    m_offset += sizeof(int8_t);
    return value;
}

auto BinFile::read_i16() -> int16_t
{
    int16_t value;
    memcpy(&value, &m_data[m_offset], sizeof(int16_t));
    m_offset += sizeof(int16_t);
    return value;
}

auto BinFile::read_i32() -> int32_t
{
    int32_t value;
    memcpy(&value, &m_data[m_offset], sizeof(int32_t));
    m_offset += sizeof(int32_t);
    return value;
}

//
// GNSFile
//

auto GNSFile::read_records() -> std::vector<Record>
{
    std::vector<Record> records;
    while (true) {
        Record record { read_bytes(20) };
        if (record.resource_type() == ResourceType::End) {
            break;
        }
        records.push_back(record);
    }
    return records;
}

//
// TextureFile
//

auto TextureFile::read_texture() -> std::shared_ptr<Texture>
{
    std::array<uint8_t, FFT_TEXTURE_NUM_BYTES> pixels = {};

    for (int i = 0, j = 0; i < FFT_TEXTURE_RAW_SIZE; i++, j += 8) {
        uint8_t raw_pixel = m_data.at(i);
        uint8_t right = ((raw_pixel & 0x0F));
        uint8_t left = ((raw_pixel & 0xF0) >> 4);
        pixels.at(j + 0) = right;
        pixels.at(j + 1) = right;
        pixels.at(j + 2) = right;
        pixels.at(j + 3) = right;
        pixels.at(j + 4) = left;
        pixels.at(j + 5) = left;
        pixels.at(j + 6) = left;
        pixels.at(j + 7) = left;
    }

    auto texture = std::make_shared<Texture>(pixels);
    return texture;
}

//
// MeshFile
//

auto MeshFile::read_mesh() -> std::shared_ptr<FFTMesh>
{
    auto mesh = std::make_shared<FFTMesh>();
    mesh->vertices = read_vertices();
    mesh->palette = read_palette();

    auto [lights, ambient_color, background] = read_lights();
    mesh->lights = lights;
    mesh->ambient_color = ambient_color;
    mesh->background = background;

    return mesh;
}

auto MeshFile::read_vertices() -> std::vector<Vertex>
{
    // 0x40 is always the location of the primary mesh pointer.
    // 0xC4 is always the primary mesh pointer.
    m_offset = 0x40;
    uint32_t intra_file_ptr = read_u32();
    if (intra_file_ptr == 0) {
        return {};
    }

    m_offset = intra_file_ptr;

    // The number of each type of polygon.
    uint16_t N = read_u16(); // Textured triangles
    uint16_t P = read_u16(); // Textured quads
    uint16_t Q = read_u16(); // Untextured triangles
    uint16_t R = read_u16(); // Untextured quads

    // Validate maximum values
    assert(N < 512 && P < 768 && Q < 64 && R < 256);

    std::vector<Vertex> vertices;
    vertices.resize((N * 3) + (P * 3 * 2) + (Q * 3) + (R * 3 * 2));

    int index = 0;

    // Textured triangle vertices
    for (int i = index; i < N * 3; i = i + 3) {
        auto a = read_position();
        auto b = read_position();
        auto c = read_position();

        vertices.at(i + 0).position = a;
        vertices.at(i + 1).position = c; // b/c swap for CW->CCW
        vertices.at(i + 2).position = b;
    }

    index = index + (N * 3);

    // Textured quad vertices. Split into 2 triangles.
    for (int i = index; i < index + (P * 2 * 3); i = i + 6) {
        glm::vec3 a = read_position();
        glm::vec3 b = read_position();
        glm::vec3 c = read_position();
        glm::vec3 d = read_position();

        // Triangle A
        vertices.at(i + 0).position = a;
        vertices.at(i + 1).position = c; // b/c swap for CW->CCW
        vertices.at(i + 2).position = b;

        // Triangle B
        vertices.at(i + 3).position = b;
        vertices.at(i + 4).position = c; // c/d swap for CW-CCW
        vertices.at(i + 5).position = d;
    }

    index = index + (P * 2 * 3);

    // Untextured triangle vertices
    for (int i = index; i < index + (Q * 3); i = i + 3) {
        auto a = read_position();
        auto b = read_position();
        auto c = read_position();

        vertices.at(i + 0).position = a;
        vertices.at(i + 1).position = c; // b/c swap for CW->CCW
        vertices.at(i + 2).position = b;
    }

    index = index + (Q * 3);

    // Untextured quad vertices. Split into 2 triangles.
    for (int i = index; i < index + (R * 2 * 3); i = i + 6) {
        glm::vec3 a = read_position();
        glm::vec3 b = read_position();
        glm::vec3 c = read_position();
        glm::vec3 d = read_position();

        // Triangle A
        vertices.at(i + 0).position = a;
        vertices.at(i + 1).position = c; // b/c swap for CW->CCW
        vertices.at(i + 2).position = b;

        // Triangle B
        vertices.at(i + 3).position = b;
        vertices.at(i + 4).position = c; // c/d swap for CW-CCW
        vertices.at(i + 5).position = d;
    }

    index = index + (R * 2 * 3);

    // Reset index so we can start over for normals, using the same vertices.
    index = 0;

    // Triangle normals
    for (int i = index; i < N * 3; i = i + 3) {
        auto a = read_normal();
        auto b = read_normal();
        auto c = read_normal();

        vertices.at(i + 0).normal = a;
        vertices.at(i + 1).normal = c; // b/c swap for CW->CCW
        vertices.at(i + 2).normal = b;
    }

    index = index + (N * 3);

    // Quad normals. Split into 2 triangles.
    for (int i = index; i < index + (P * 2 * 3); i = i + 6) {
        glm::vec3 a = read_normal();
        glm::vec3 b = read_normal();
        glm::vec3 c = read_normal();
        glm::vec3 d = read_normal();

        // Triangle A
        vertices.at(i + 0).normal = a;
        vertices.at(i + 1).normal = c; // b/c swap for CW->CCW
        vertices.at(i + 2).normal = b;

        // Triangle B
        vertices.at(i + 3).normal = b;
        vertices.at(i + 4).normal = c; // c/d swap for CW-CCW
        vertices.at(i + 5).normal = d;
    }

    // Reset index so we can start over for texture data, using the same vertices.
    index = 0;

    // 16 palettes of 16 colors of 4 bytes
    // process_tex_coords has two functions:
    //
    // 1. Update the v coordinate to the specific page of the texture. FFT
    //    Textures have 4 pages (256x1024) and the original V specifies
    //    the pixel on one of the 4 pages. Multiply the page by the height
    //    of a single page (256).
    // 2. Normalize the coordinates that can be U:0-255 and V:0-1023. Just
    //    divide them by their max to get a 0.0-1.0 value.
    auto process_tex_coords = [](float u, float v, uint8_t page) -> glm::vec2 {
        u = u / 255.0f;
        v = (v + (page * 256)) / 1023.0f;
        return { u, v };
    };

    // Triangle UV
    for (int i = index; i < N * 3; i = i + 3) {
        float au = read_u8();
        float av = read_u8();
        float palette = read_u8();
        (void)read_u8(); // padding
        float bu = read_u8();
        float bv = read_u8();
        float page = (read_u8() & 0x03); // 0b00000011
        (void)read_u8();                 // padding
        float cu = read_u8();
        float cv = read_u8();

        glm::vec2 a = process_tex_coords(au, av, page);
        glm::vec2 b = process_tex_coords(bu, bv, page);
        glm::vec2 c = process_tex_coords(cu, cv, page);

        vertices.at(i + 0).tex_coords = a;
        vertices.at(i + 0).palette_index = palette;
        vertices.at(i + 1).tex_coords = c; // b/c swap for CW->CCW
        vertices.at(i + 1).palette_index = palette;
        vertices.at(i + 2).tex_coords = b;
        vertices.at(i + 2).palette_index = palette;
    }

    index = index + (N * 3);

    // Quad UV. Split into 2 triangles.
    for (int i = index; i < index + (P * 2 * 3); i = i + 6) {
        float au = read_u8();
        float av = read_u8();
        float palette = read_u8();
        (void)read_u8(); // padding
        float bu = read_u8();
        float bv = read_u8();
        float page = (read_u8() & 0x03); // 0b00000011
        (void)read_u8();                 // padding
        float cu = read_u8();
        float cv = read_u8();
        float du = read_u8();
        float dv = read_u8();

        glm::vec2 a = process_tex_coords(au, av, page);
        glm::vec2 b = process_tex_coords(bu, bv, page);
        glm::vec2 c = process_tex_coords(cu, cv, page);
        glm::vec2 d = process_tex_coords(du, dv, page);

        // Triangle A
        vertices.at(i + 0).tex_coords = a;
        vertices.at(i + 0).palette_index = palette;
        vertices.at(i + 1).tex_coords = c; // b/c swap for CW->CCW
        vertices.at(i + 1).palette_index = palette;
        vertices.at(i + 2).tex_coords = b;
        vertices.at(i + 2).palette_index = palette;

        // Triangle B
        vertices.at(i + 3).tex_coords = b;
        vertices.at(i + 3).palette_index = palette;
        vertices.at(i + 4).tex_coords = c; // c/d swap for CW-CCW
        vertices.at(i + 4).palette_index = palette;
        vertices.at(i + 5).tex_coords = d;
        vertices.at(i + 5).palette_index = palette;
    }

    return vertices;
}

auto MeshFile::read_palette() -> std::shared_ptr<Texture>
{
    m_offset = 0x44;
    uint32_t intra_file_ptr = read_u32();
    if (intra_file_ptr == 0) {
        return nullptr;
    }
    m_offset = intra_file_ptr;

    std::array<uint8_t, FFT_PALETTE_NUM_BYTES> pixels = {};

    for (int i = 0; i < 16 * 16 * 4; i = i + 4) {
        glm::vec4 c = read_rgb15();
        pixels.at(i + 0) = c.x;
        pixels.at(i + 1) = c.y;
        pixels.at(i + 2) = c.z;
        pixels.at(i + 3) = c.w;
    }

    auto palette = std::make_shared<Texture>(pixels);
    return palette;
}

auto MeshFile::read_lights() -> std::tuple<std::vector<std::shared_ptr<Light>>, glm::vec4, std::pair<glm::vec4, glm::vec4>>
{
    m_offset = 0x64;
    uint32_t intra_file_ptr = read_u32();
    if (intra_file_ptr == 0) {
        return {};
    }

    m_offset = intra_file_ptr;

    glm::vec4 a_color = {};
    glm::vec4 b_color = {};
    glm::vec4 c_color = {};

    glm::vec3 a_pos = {};
    glm::vec3 b_pos = {};
    glm::vec3 c_pos = {};

    a_color.x = read_light_color();
    b_color.x = read_light_color();
    c_color.x = read_light_color();
    a_color.y = read_light_color();
    b_color.y = read_light_color();
    c_color.y = read_light_color();
    a_color.z = read_light_color();
    b_color.z = read_light_color();
    c_color.z = read_light_color();

    a_color.w = 1.0f;
    b_color.w = 1.0f;
    c_color.w = 1.0f;

    a_pos = read_position();
    b_pos = read_position();
    c_pos = read_position();

    auto resources = ResourceManager::get_instance();
    auto cube_mesh = resources->get_mesh("cube");
    auto a = std::make_shared<Light>(cube_mesh, a_color, a_pos);
    auto b = std::make_shared<Light>(cube_mesh, b_color, b_pos);
    auto c = std::make_shared<Light>(cube_mesh, c_color, c_pos);

    std::vector<std::shared_ptr<Light>> directional_lights = {};

    if (a->is_valid()) {
        directional_lights.push_back(a);
    }
    if (b->is_valid()) {
        directional_lights.push_back(b);
    }
    if (c->is_valid()) {
        directional_lights.push_back(c);
    }

    auto ambient_color = read_rgb8();
    auto background = read_background();

    return { directional_lights, ambient_color, background };
}

auto MeshFile::read_background() -> std::pair<glm::vec4, glm::vec4>
{
    auto top = read_rgb8();
    auto bottom = read_rgb8();
    return { top, bottom };
}

auto MeshFile::read_position() -> glm::vec3
{
    float x = read_i16() / GLOBAL_SCALE;
    float y = read_i16() / GLOBAL_SCALE;
    float z = read_i16() / GLOBAL_SCALE;

    y = -y;
    z = -z;

    return { x, y, z };
}

auto MeshFile::read_normal() -> glm::vec3
{
    float x = read_f1x3x12();
    float y = read_f1x3x12();
    float z = read_f1x3x12();

    y = -y;
    z = -z;

    return { x, y, z };
}

// read_light_color clamps the value between 0.0 and 1.0. These unclamped values
// are used to affect the lighting model but it isn't understood yet.
// https://ffhacktics.com/wiki/Maps/Mesh#Light_colors_and_positions.2C_background_gradient_colors
auto MeshFile::read_light_color() -> float
{
    float val = read_f1x3x12();
    return std::min(std::max(0.0f, val), 1.0f);
}

auto MeshFile::read_f1x3x12() -> float
{
    float value = read_i16();
    return value / 4096.0f;
}

auto MeshFile::read_rgb8() -> glm::vec4
{
    float r = (float)read_u8() / 255.0f;
    float g = (float)read_u8() / 255.0f;
    float b = (float)read_u8() / 255.0f;
    return { r, g, b, 1.0f };
}

auto MeshFile::read_rgb15() -> glm::vec4
{
    uint16_t val = read_u16();
    uint8_t a = val == 0 ? 0x00 : 0xFF;
    uint8_t b = (val & 0x7C00) >> 7; // 0b0111110000000000
    uint8_t g = (val & 0x03E0) >> 2; // 0b0000001111100000
    uint8_t r = (val & 0x001F) << 3; // 0b0000000000011111
    return { r, g, b, a };
}

//
// AttackOutFile
//

// read_scenarios returns all scenarios from the ATTACK.OUT file. The
// BinReader.read_scenarios() function will filter out unusable scenarios.
//
// https://ffhacktics.com/wiki/ATTACK.OUT
auto AttackOutFile::read_scenarios() -> std::vector<Scenario>
{
    constexpr int scenario_count = 488;
    constexpr int scenario_size = 24;
    constexpr int scenario_intra_file_offset = 0x10938;

    // Skip ahead to the scenario data in the file.
    m_offset = scenario_intra_file_offset;

    std::vector<Scenario> scenarios;
    for (int i = 0; i < scenario_count; i++) {
        auto bytes = read_bytes(scenario_size);
        Scenario scenario { bytes };
        scenarios.push_back(scenario);
    }
    return scenarios;
}

auto EventFile::read_events() -> std::vector<Event>
{

    constexpr int event_count = 500;
    constexpr int event_size = 8192;

    std::vector<Event> events;
    for (int i = 0; i < event_count; i++) {
        auto bytes = read_bytes(event_size);
        Event event { bytes };
        events.push_back(event);
    }
    return events;
}
