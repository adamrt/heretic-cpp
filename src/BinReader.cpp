#include "BinReader.h"
#include "ResourceManager.h"

BinReader::BinReader(std::string filename)
{

    file = fopen(filename.c_str(), "r");
    if (!file) {
        fprintf(stderr, "Failed to open file: %s\n", filename.c_str());
        exit(1);
    }
}

BinReader::~BinReader()
{
    fclose(file);
}

// read_sector reads a sector to `out_bytes`.
auto BinReader::read_sector(int32_t sector_num) -> std::array<uint8_t, SECTOR_SIZE>
{
    int32_t seek_to = (sector_num * SECTOR_SIZE_RAW) + SECTOR_HEADER_SIZE;
    if (fseek(file, seek_to, SEEK_SET) != 0) {
        assert(false);
    }
    auto sector = std::array<uint8_t, SECTOR_SIZE>();
    size_t n = fread(sector.data(), sizeof(uint8_t), SECTOR_SIZE, file);
    assert(n == SECTOR_SIZE);
    return sector;
}

// read_file reads an entire file, sector by sector.
auto BinReader::read_file(uint32_t sector_num, uint32_t size) -> BinFile
{
    uint32_t occupied_sectors = ceil((float)size / (float)SECTOR_SIZE);
    BinFile out_file;
    for (uint32_t i = 0; i < occupied_sectors; i++) {
        auto sector_data = read_sector(sector_num + i);
        out_file.data.insert(out_file.data.end(), sector_data.begin(), sector_data.end());
        out_file.length += SECTOR_SIZE;
    }
    out_file.offset = 0;
    return out_file;
}

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
    memcpy(&value, &data[offset], sizeof(uint8_t));
    offset += sizeof(uint8_t);
    return value;
}

auto BinFile::read_u16() -> uint16_t
{
    uint16_t value;
    memcpy(&value, &data[offset], sizeof(uint16_t));
    offset += sizeof(uint16_t);
    return value;
}

auto BinFile::read_u32() -> uint32_t
{
    uint32_t value;
    memcpy(&value, &data[offset], sizeof(uint32_t));
    offset += sizeof(uint32_t);
    return value;
}

auto BinFile::read_i8() -> int8_t
{
    int8_t value;
    memcpy(&value, &data[offset], sizeof(int8_t));
    offset += sizeof(int8_t);
    return value;
}

auto BinFile::read_i16() -> int16_t
{
    int16_t value;
    memcpy(&value, &data[offset], sizeof(int16_t));
    offset += sizeof(int16_t);
    return value;
}

auto BinFile::read_i32() -> int32_t
{
    int32_t value;
    memcpy(&value, &data[offset], sizeof(int32_t));
    offset += sizeof(int32_t);
    return value;
}

auto BinFile::read_f1x3x12() -> float
{
    float value = read_i16();
    return value / 4096.0f;
}

auto BinFile::read_position() -> glm::vec3
{
    float x = read_i16();
    float y = read_i16();
    float z = read_i16();

    y = -y;
    z = -z;

    return { x, y, z };
}

auto BinFile::read_normal() -> glm::vec3
{
    float x = read_f1x3x12();
    float y = read_f1x3x12();
    float z = read_f1x3x12();

    y = -y;
    z = -z;
    return { x, y, z };
}

auto BinFile::read_rgb15() -> glm::vec4
{
    uint16_t val = read_u16();
    uint8_t a = val == 0 ? 0x00 : 0xFF;
    uint8_t b = (val & 0x7C00) >> 7; // 0b0111110000000000
    uint8_t g = (val & 0x03E0) >> 2; // 0b0000001111100000
    uint8_t r = (val & 0x001F) << 3; // 0b0000000000011111
    return { r, g, b, a };
}

auto BinFile::read_rgb8() -> glm::vec4
{
    float r = (float)read_u8() / 255.0f;
    float g = (float)read_u8() / 255.0f;
    float b = (float)read_u8() / 255.0f;
    return { r, g, b, 1.0f };
}

// https://ffhacktics.com/wiki/ATTACK.OUT
auto BinReader::read_scenarios() -> std::vector<Scenario>
{
    constexpr int attack_out_sector = 2448;
    constexpr int attack_out_size = 125956;
    constexpr int attack_intra_file_offset = 0x10938;

    auto attack_out = read_file(attack_out_sector, attack_out_size);
    attack_out.data.erase(attack_out.data.begin(), attack_out.data.begin() + attack_intra_file_offset);
    return attack_out.read_scenarios();
}

// https://ffhacktics.com/wiki/TEST.EVT
auto BinReader::read_events() -> std::vector<Event>
{
    const int test_evt_sector = 3707;
    const int test_evt_size = 4096000;

    std::vector<Event> events = {};
    for (int event_id = 0; event_id < 100; event_id++) {
        auto test_evt = read_file((event_id * 4) + test_evt_sector, test_evt_size);
        events.push_back(test_evt.read_event());
    }
    return events;
}

auto BinReader::read_map(int map_num, MapTime time, MapWeather weather, int arrangement) -> std::shared_ptr<FFTMap>
{
    auto requested_key = std::make_tuple(time, weather, arrangement);
    auto default_key = std::make_tuple(MapTime::Day, MapWeather::None, 0);

    int sector = map_list[map_num].sector;
    auto gns_file = read_file(sector, GNS_MAX_SIZE);
    auto gns_records = gns_file.read_records();

    std::shared_ptr<FFTMesh> primary_mesh = nullptr;
    std::map<std::tuple<MapTime, MapWeather, int>, std::shared_ptr<FFTMesh>> meshes;
    std::map<std::tuple<MapTime, MapWeather, int>, std::shared_ptr<Texture>> textures;

    for (auto& record : gns_records) {
        auto record_key = std::make_tuple(record.time(), record.weather(), record.arrangement());
        auto resource = read_file(record.sector(), record.length());

        switch (record.resource_type()) {
        case ResourceType::MeshPrimary: {
            // Primary Mesh is always Day/None
            primary_mesh = resource.read_mesh();
            break;
        }
        case ResourceType::Texture: {
            // There can be duplicate textures for the same time/weather. Use the first one.
            if (textures.find(record_key) == textures.end()) {
                textures[record_key] = resource.read_texture();
            }
            break;
        }
        case ResourceType::MeshAlt: {
            meshes[record_key] = resource.read_mesh();
            break;
        }
        case ResourceType::MeshOverride: {
            break;
        }
        case ResourceType::End: {
            break;
        }
        default:
            std::cout << "Unknown resource type: " << std::endl;
        }
    }

    auto texture = textures[requested_key];
    if (texture == nullptr) {
        texture = textures[default_key];
        assert(texture != nullptr);
    }

    auto override_mesh = meshes[requested_key];
    if (primary_mesh == nullptr) {
        primary_mesh = std::make_shared<FFTMesh>();
        if (override_mesh == nullptr) {
            override_mesh = meshes[default_key];
            assert(override_mesh != nullptr);
        }
    }

    if (override_mesh != nullptr) {
        if (override_mesh->vertices.size() > 0) {
            primary_mesh->vertices.insert(primary_mesh->vertices.end(), override_mesh->vertices.begin(), override_mesh->vertices.end());
        }
        if (override_mesh->lights.size() > 0) {
            primary_mesh->lights = override_mesh->lights;
        }
        if (override_mesh->palette != nullptr) {
            primary_mesh->palette = override_mesh->palette;
        }
        // Defaults w (alpha) is 0.0f, so 1.0f means we read the ambient color and background.
        if (override_mesh->ambient_color.w == 1.0f) {
            primary_mesh->ambient_color = override_mesh->ambient_color;
        }
        if (override_mesh->background.first.w == 1.0f) {
            primary_mesh->background = override_mesh->background;
        }
    }

    // FIXME: There are no vertices in primary or alt. Ex: Map 104.
    if (primary_mesh->vertices.size() == 0) {
        return nullptr;
    }

    // FIXME: Another hack. How are we getting a null palette?
    if (primary_mesh->palette == nullptr) {
        return nullptr;
    }

    auto map = std::make_shared<FFTMap>();
    map->gns_records = gns_records;
    map->texture = texture;
    map->mesh = primary_mesh;

    return map;
}

auto BinFile::read_records() -> std::vector<Record>
{
    std::vector<Record> records;

    while (true) {
        Record record { read_bytes(20) };
        // This record type marks the end of the records for this GNS file.
        if (record.resource_type() == ResourceType::End) {
            return records;
        }

        records.push_back(record);

        // Satefy check in case there is a bad read.
        assert(records.size() < RECORD_MAX_NUM);
    }
    return records;
}

auto BinFile::read_scenarios() -> std::vector<Scenario>
{
    constexpr int scenario_num = 500;
    constexpr int scenario_size = 24;

    std::vector<Scenario> scenarios;
    for (int i = 0; i < scenario_num; i++) {
        auto bytes = read_bytes(scenario_size);
        Scenario scenario { bytes };

        // We don't care about the test map as it doesn't have a texture. Skip
        // all its scenarios.
        if (scenario.map_id() == 0) {
            continue;
        }

        scenarios.push_back(scenario);
    }
    return scenarios;
}

auto BinFile::read_event() -> Event
{
    constexpr int event_size = 8192;

    auto bytes = read_bytes(event_size);
    Event event { bytes };
    return event;
}

auto BinFile::read_mesh() -> std::shared_ptr<FFTMesh>
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

auto BinFile::read_vertices() -> std::vector<Vertex>
{
    // 0x40 is always the location of the primary mesh pointer.
    // 0xC4 is always the primary mesh pointer.
    offset = 0x40;
    uint32_t intra_file_ptr = read_u32();
    if (intra_file_ptr == 0) {
        return {};
    }

    offset = intra_file_ptr;

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

    // Validate
    // uint32_t expected_num_vertices = (uint32_t)(N * 3) + (P * 3 * 2) + (Q * 3) + (R * 3 * 2);
    // if (mesh->rtices != expected_num_vertices) {
    //     return false;
    // }

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

// 16 palettes of 16 colors of 4 bytes
auto BinFile::read_palette() -> std::shared_ptr<Texture>
{
    offset = 0x44;
    uint32_t intra_file_ptr = read_u32();
    if (intra_file_ptr == 0) {
        return nullptr;
    }
    offset = intra_file_ptr;

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

auto BinFile::read_lights() -> std::tuple<std::vector<std::shared_ptr<Light>>, glm::vec4, std::pair<glm::vec4, glm::vec4>>
{
    offset = 0x64;
    uint32_t intra_file_ptr = read_u32();
    if (intra_file_ptr == 0) {
        return {};
    }

    offset = intra_file_ptr;

    glm::vec4 a_color = {};
    glm::vec4 b_color = {};
    glm::vec4 c_color = {};

    glm::vec3 a_pos = {};
    glm::vec3 b_pos = {};
    glm::vec3 c_pos = {};

    a_color.x = read_f1x3x12();
    b_color.x = read_f1x3x12();
    c_color.x = read_f1x3x12();
    a_color.y = read_f1x3x12();
    b_color.y = read_f1x3x12();
    c_color.y = read_f1x3x12();
    a_color.z = read_f1x3x12();
    b_color.z = read_f1x3x12();
    c_color.z = read_f1x3x12();

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

auto BinFile::read_background() -> std::pair<glm::vec4, glm::vec4>
{
    auto top = read_rgb8();
    auto bottom = read_rgb8();
    return { top, bottom };
}

auto BinFile::read_texture() -> std::shared_ptr<Texture>
{
    std::array<uint8_t, FFT_TEXTURE_NUM_BYTES> pixels = {};

    for (int i = 0, j = 0; i < FFT_TEXTURE_RAW_SIZE; i++, j += 8) {
        uint8_t raw_pixel = data.at(i);
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

// process_tex_coords has two functions:
//
// 1. Update the v coordinate to the specific page of the texture. FFT
//    Textures have 4 pages (256x1024) and the original V specifies
//    the pixel on one of the 4 pages. Multiply the page by the height
//    of a single page (256).
// 2. Normalize the coordinates that can be U:0-255 and V:0-1023. Just
//    divide them by their max to get a 0.0-1.0 value.
glm::vec2 process_tex_coords(float u, float v, uint8_t page)
{
    u = u / 255.0f;
    v = (v + (page * 256)) / 1023.0f;
    return { u, v };
}
