#include <math.h>

#include <array>
#include <assert.h>
#include <iomanip>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <utility>

#include "ResourceManager.h"

#include "FFT.h"

// process_tex_coords has two functions:
//
// 1. Update the v coordinate to the specific page of the texture. FFT
//    Textures have 4 pages (256x1024) and the original V specifies
//    the pixel on one of the 4 pages. Multiply the page by the height
//    of a single page (256).
// 2. Normalize the coordinates that can be U:0-255 and V:0-1023. Just
//    divide them by their max to get a 0.0-1.0 value.
static glm::vec2 process_tex_coords(float u, float v, uint8_t page)
{
    u = u / 255.0f;
    v = (v + (page * 256)) / 1023.0f;
    return { u, v };
}

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

auto BinFile::read_rgb8() -> glm::vec3
{
    float r = (float)read_u8() / 255.0f;
    float g = (float)read_u8() / 255.0f;
    float b = (float)read_u8() / 255.0f;
    return { r, g, b };
}

// https://ffhacktics.com/wiki/ATTACK.OUT
auto BinReader::read_scenarios() -> std::vector<Scenario>
{
    const int attack_offset = 0x10938;
    auto attack_out = read_file(2448, 125956);
    attack_out.data.erase(attack_out.data.begin(), attack_out.data.begin() + attack_offset);
    return attack_out.read_scenarios();
}

// https://ffhacktics.com/wiki/ATTACK.OUT
auto BinReader::read_events() -> std::vector<Event>
{
    auto test_evt = read_file(3707, 4096000);
    // test_evt.data.erase(test_evt.data.begin(), test_evt.data.begin() + attack_offset);
    return test_evt.read_events();
}

auto BinReader::read_map(int mapnum) -> std::shared_ptr<FFTMap>
{
    int sector = map_list[mapnum].sector;

    auto gns = read_file(sector, GNS_MAX_SIZE);
    auto records = gns.read_records();

    auto map = std::make_shared<FFTMap>();

    map->records = records;

    for (auto& record : records) {
        auto resource = read_file(record.sector(), record.length());

        switch (record.resource_type()) {
        case ResourceType::MeshPrimary: {
            map->vertices = resource.read_vertices();
            map->palette = resource.read_palette();
            map->lights = resource.read_lights();
            auto bg = resource.read_background();
            map->background_top = bg.first;
            map->background_bottom = bg.second;
            break;
        }
        case ResourceType::Texture:
            map->texture = resource.read_texture();
            break;
        case ResourceType::MeshOverride:
            // Sometimes there is no primary mesh (ie MAP002.GNS), there is
            // only an override. Usually a non-battle map. So we treat this
            // one as the primary, only if the primary hasn't been set. Kinda
            // Hacky until we start treating each GNS Record as a Scenario.
            // NOTE: We used to check mesh.is_valid, but we don't anymore. Maybe
            // we should?
            if (map->vertices.size() == 0) {
                map->vertices = resource.read_vertices();
                return nullptr;
            }
            break;
        default:
            break;
        }
    }

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
    constexpr int scenario_size = 24;
    std::vector<Scenario> scenarios;
    // FIXME: Why 300?
    for (int i = 0; i < 300; i++) {
        auto bytes = read_bytes(scenario_size);
        Scenario scenario { bytes };
        scenarios.push_back(scenario);
    }
    return scenarios;
}

auto BinFile::read_events() -> std::vector<Event>
{
    constexpr int event_size = 8192;
    int event_id = 1;
    std::vector<Event> events;
    for (int i = 1; i < 100; i++) {
        offset = i * 4;
        auto bytes = read_bytes(event_size);
        Event event { bytes };
        events.push_back(event);
    }
    return events;
}

auto BinFile::read_vertices() -> std::vector<Vertex>
{
    // 0x40 is always the location of the primary mesh pointer.
    // 0xC4 is always the primary mesh pointer.
    offset = 0x40;
    uint32_t primary_mesh_ptr = read_u32();
    if (primary_mesh_ptr != 0xC4) {
        return {};
    }

    offset = primary_mesh_ptr;

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

auto BinFile::read_lights() -> std::vector<std::shared_ptr<Light>>
{
    offset = 0x64;
    uint32_t intra_file_ptr = read_u32();
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

    std::vector<std::shared_ptr<Light>> lights = {};

    if (a->is_valid()) {
        lights.push_back(a);
    }
    if (b->is_valid()) {
        lights.push_back(b);
    }
    if (c->is_valid()) {
        lights.push_back(c);
    }

    // FIXME! Get ambient light somewhere else
    // mesh->ambient_light_color = read_rgb8();
    //
    return lights;
}

auto BinFile::read_background() -> std::pair<glm::vec4, glm::vec4>
{
    auto top = read_rgb8();
    auto bottom = read_rgb8();
    return { glm::vec4(top, 1.0f), glm::vec4(bottom, 1.0f) };
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

const FFTMapDesc map_list[128] = {
    { 0, 10026, "???", true },
    { 1, 11304, "At Main Gate of Igros Castle", true },
    { 2, 12656, "Back Gate of Lesalia Castle", true },
    { 3, 12938, "Hall of St. Murond Temple", true },
    { 4, 13570, "Office of Lesalia Castle", true },
    { 5, 14239, "Roof of Riovanes Castle", true },
    { 6, 14751, "At the Gate of Riovanes Castle", true },
    { 7, 15030, "Inside of Riovanes Castle", true },
    { 8, 15595, "Riovanes Castle", true },
    { 9, 16262, "Citadel of Igros Castle", true },
    { 10, 16347, "Inside of Igros Castle", true },
    { 11, 16852, "Office of Igros Castle", true },
    { 12, 17343, "At the Gate of Lionel Castle", true },
    { 13, 17627, "Inside of Lionel Castle", true },
    { 14, 18175, "Office of Lionel Castle", true },
    { 15, 19510, "At the Gate of Limberry Castle (1)", true },
    { 16, 20075, "Inside of Limberry Castle", true },
    { 17, 20162, "Underground Cemetary of Limberry Castle", true },
    { 18, 20745, "Office of Limberry Castle", true },
    { 19, 21411, "At the Gate of Limberry Castle (2)", true },
    { 20, 21692, "Inside of Zeltennia Castle", true },
    { 21, 22270, "Zeltennia Castle", true },
    { 22, 22938, "Magic City Gariland", true },
    { 23, 23282, "Belouve Residence", true },
    { 24, 23557, "Military Academy's Auditorium", true },
    { 25, 23899, "Yardow Fort City", true },
    { 26, 23988, "Weapon Storage of Yardow", true },
    { 27, 24266, "Goland Coal City", true },
    { 28, 24544, "Colliery Underground First Floor", true },
    { 29, 24822, "Colliery Underground Second Floor", true },
    { 30, 25099, "Colliery Underground Third Floor", true },
    { 31, 25764, "Dorter Trade City", true },
    { 32, 26042, "Slums in Dorter", true },
    { 33, 26229, "Hospital in Slums", true },
    { 34, 26362, "Cellar of Sand Mouse", true },
    { 35, 27028, "Zaland Fort City", true },
    { 36, 27643, "Church Outside of Town", true },
    { 37, 27793, "Ruins Outside Zaland", true },
    { 38, 28467, "Goug Machine City", true },
    { 39, 28555, "Underground Passage in Goland", true },
    { 40, 29165, "Slums in Goug", true },
    { 41, 29311, "Besrodio's House", true },
    { 42, 29653, "Warjilis Trade City", true },
    { 43, 29807, "Port of Warjilis", true },
    { 44, 30473, "Bervenia Free City", true },
    { 45, 30622, "Ruins of Zeltennia Castle's Church", true },
    { 46, 30966, "Cemetary of Heavenly Knight, Balbanes", true },
    { 47, 31697, "Zarghidas Trade City", true },
    { 48, 32365, "Slums of Zarghidas", true },
    { 49, 33032, "Fort Zeakden", true },
    { 50, 33701, "St. Murond Temple", true },
    { 51, 34349, "St. Murond Temple", true },
    { 52, 34440, "Chapel of St. Murond Temple", true },
    // MAP053 doesn't have expected primary mesh pointer
    { 53, 34566, "Entrance to Death City", true },
    { 54, 34647, "Lost Sacred Precincts", true },
    { 55, 34745, "Graveyard of Airships", true },
    { 56, 35350, "Orbonne Monastery", true },
    { 57, 35436, "Underground Book Storage First Floor", true },
    { 58, 35519, "Underground Book Storage Second Floor", true },
    { 59, 35603, "Underground Book Storage Third Floor", true },
    { 60, 35683, "Underground Book Storge Fourth Floor", true },
    { 61, 35765, "Underground Book Storage Fifth Floor", true },
    { 62, 36052, "Chapel of Orbonne Monastery", true },
    { 63, 36394, "Golgorand Execution Site", true },
    { 64, 36530, "In Front of Bethla Garrison's Sluice", true },
    { 65, 36612, "Granary of Bethla Garrison", true },
    { 66, 37214, "South Wall of Bethla Garrison", true },
    { 67, 37817, "Noth Wall of Bethla Garrison", true },
    { 68, 38386, "Bethla Garrison", true },
    { 69, 38473, "Murond Death City", true },
    { 70, 38622, "Nelveska Temple", true },
    { 71, 39288, "Dolbodar Swamp", true },
    { 72, 39826, "Fovoham Plains", true },
    { 73, 40120, "Inside of Windmill Shed", true },
    { 74, 40724, "Sweegy Woods", true },
    { 75, 41391, "Bervenia Volcano", true },
    { 76, 41865, "Zeklaus Desert", true },
    { 77, 42532, "Lenalia Plateau", true },
    { 78, 43200, "Zigolis Swamp", true },
    { 79, 43295, "Yuguo Woods", true },
    { 80, 43901, "Araguay Woods", true },
    { 81, 44569, "Grog Hill", true },
    { 82, 45044, "Bed Desert", true },
    { 83, 45164, "Zirekile Falls", true },
    { 84, 45829, "Bariaus Hill", true },
    { 85, 46498, "Mandalia Plains", true },
    { 86, 47167, "Doguola Pass", true },
    { 87, 47260, "Bariaus Valley", true },
    { 88, 47928, "Finath River", true },
    { 89, 48595, "Poeskas Lake", true },
    { 90, 49260, "Germinas Peak", true },
    { 91, 49538, "Thieves Fort", true },
    { 92, 50108, "Igros-Belouve Residence", true },
    { 93, 50387, "Broke Down Shed-Wooden Building", true },
    { 94, 50554, "Broke Down Shed-Stone Building", true },
    { 95, 51120, "Church", true },
    { 96, 51416, "Pub", true },
    { 97, 52082, "Inside Castle Gate in Lesalia", true },
    { 98, 52749, "Outside Castle Gate in Lesalia", true },
    { 99, 53414, "Main Street of Lesalia", true },
    { 100, 53502, "Public Cemetary", true },
    { 101, 53579, "Tutorial (1)", true },
    { 102, 53659, "Tutorial (2)", true },
    { 103, 54273, "Windmill Shed", true },
    { 104, 54359, "Belouve Residence", true },
    { 105, 54528, "TERMINATE", true },
    { 106, 54621, "DELTA", true },
    { 107, 54716, "NOGIAS", true },
    { 108, 54812, "VOYAGE", true },
    { 109, 54909, "BRIDGE", true },
    { 110, 55004, "VALKYRIES", true },
    { 111, 55097, "MLAPAN", true },
    { 112, 55192, "TIGER", true },
    { 113, 55286, "HORROR", true },
    { 114, 55383, "END", true },
    { 115, 56051, "Banished Fort", true },
    { 116, 56123, "Arena", true },
    { 117, 56201, "???", true },
    { 118, 56279, "???", true },
    { 119, 56356, "???", true },
    { 120, 0, "???", false },
    { 121, 0, "???", false },
    { 122, 0, "???", false },
    { 123, 0, "???", false },
    { 124, 0, "???", false },
    { 125, 56435, "???", true },
    { 126, 0, "???", false },
    { 127, 0, "???", false },
};

std::map<int, Instruction> instruction_list = {
    { 0x10, { "DisplayMessage", "This instruction is used to display any text stored after the event's instruction in various ways like a character thinking, speaking, or simply printing text on the screen.", R"(DisplayMessage(x10, DialogType, "Message ID": xMSG#,"Unit ID": xID,x00,"Portrait Row": xPR,"X Coordinate": +XXXXX,"Y Coordinate": +YYYYY,"Arrow Position": +ARPOS,"Dialog Box Opening Type": xOT))" } },
    { 0x11, { "UnitAnim", "This instruction is used load a humanoid shaped sprite off EVTCHR.BIN on a sprite using the original unit's palette.", R"(UnitAnim("Affected Units": xAU,"Multi Targeting": xMT,"Animation ID SEQ/EVTCHR": xANIM,x00))" } },
    { 0x13, { "ChangeMapBeta", "Changes the map the event is currently played in. Not recommended for use.", R"(ChangeMapBeta("Map": MAP,x00))" } },
    { 0x16, { "Pause", "Pause the event and resume it when the player presses either O (confirm), X (cancel) or select.", R"(Pause())" } },
    { 0x18, { "Effect", "Uses an ability or effect on target Unit. Only one Effect can play at a time or the game will freeze.", R"(Effect("Ability/Effect ID": xEFID,"Unit ID": xID,"X Coordinate": XXX,"Y Coordinate": YYY,"Unknown": x00))" } },
    { 0x19, { "Camera", "Moves camera around the map, like a small helicopter. The coordinates used are the map's absolute coordinates, not relative ones to where the camera currently is. All entry fields can range from -32768 to 32767. Tiles are 28x28, and Camera motion is 4x more precise, so 112 = 1 Tile. 1h is 12 pixels high, so 48 = 1h, and regular units are 3h tall.", R"(Camera("X axis": +XXXXX,"Z axis": +ZZZZZ,"Y axis": +YYYYY,"Angle": +ANGLE,"Map Rotation": +MAPRT,"Camera Rotation": +CAMRT,"Zoom": +ZOOM%,"Time": +TIMER))" } },
    { 0x1A, { "MapDarkness", "Modifies the color of the map's lighting. Battle-friendly.", R"(MapDarkness("Blending Mode": xBM,"Red": +RED,"Green": +GRN,"Blue": +BLU,"Time"},: TIM))" } },
    { 0x1B, { "MapLight", "Controls the map's lighting, and has the ability to change the color of the light. Battle-friendly.", R"(MapLight("Unknown Controls Lighting": +00000,"Unknown Controls Lighting": +00000,"Unknown Controls Lighting": ,+?????,"Red": +RRRED,"Green": +GREEN,"Blue": +BBLUE,"Time": +TIMER))" } },
    { 0x1C, { "EventSpeed", "This instruction determines the speed the event will play. It is only used alongside Lucavi transformation and death Effects to slow them down.", R"(EventSpeed("Speed": xSP))" } },
    { 0x1D, { "CameraFusionStart", "Place many camera instructions in this block make them transition smoothly.", R"(CameraFusionStart())" } },
    { 0x1E, { "CameraFusionEnd", "Place many camera instructions in this block make them transition smoothly.", R"(CameraFusionEnd())" } },
    { 0x1F, { "Focus", "The next Camera instruction will automatically center between Unit 1 and Unit 2. If the two are the same unit, it focuses on that single unit.", R"(Focus("Unit 1 ID": xID,x00,"Unit 2 ID": xID,x00,"Unknown": x00))" } },
    { 0x21, { "SoundEffect", "Terminate currently playing sound created by this instruction and plays the indexed sound once.", R"(SoundEffect("Sound ID": xSDID))" } },
    { 0x22, { "SwitchTrack", "Toggle between the first and second song assigned in ATTACK.OUT for the scenario.", R"(SwitchTrack(x01,"Volume": +VOL,"Time": TIM))" } },
    { 0x27, { "ReloadMapState", "Reloads the map with new, currently stored settings such as map arrangement (Variable x0030), weather and daytime. Map arrangements and daytime can be viewed using map2gl.", R"(ReloadMapState())" } },
    { 0x28, { "WalkTo", "FIXME", R"(FIXME)" } },
    { 0x29, { "WaitWalk", "Resumes instructions when the given unit arrives at destination.", R"(WaitWalk("Unit ID": xID,x00))" } },
    { 0x2A, { "BlockStart", "A block is a portion of the event played in a separate process. When the game finds a block, it will start executing it and will also resume the event whatever there is after the block at the same time.", R"(BlockStart())" } },
    { 0x2B, { "BlockEnd", "A block is a portion of the event played in a separate process. When the game finds a block, it will start executing it and will also resume the event whatever there is after the block at the same time.", R"(BlockEnd())" } },
    { 0x2C, { "FaceUnit2", "FIXME", R"(FIXME)" } },
    { 0x2D, { "RotateUnit", "The unit will rotate to a given direction.", R"(RotateUnit("Affected Units": xAU,"Multi Targeting": xMT,"Dire},ction": xDR,"Clockwise or Counter-clockwise": xCL,"Rotation Speed": xRS,"Delay": xDL))" } },
    { 0x2E, { "Background", "FIXME", R"(FIXME))" } },
    { 0x31, { "ColorBGBeta", "Colors Background to new value based on the current coloration, allowing better blending but ineffective on pure-black backgrounds.", R"(ColorBGBeta("Pre-set Color": xPR,"Red": +RED,"Green": +GRN,"Blue": +BLU,"Time": TIM))" } },
    { 0x32, { "ColorUnit", "Colors a unit gradually depending on its current state. (+127,-128,-128) is red and (+127,+127,+127) is white while (+000,+000,+000) returns the unit to its original color.", R"(ColorUnit("Affected Units": xAU,"Multi Targeting": xMT,"Pre-set Color": xPR,"Red": +RED,"Green": +GRN,"Blue": +BLU,"Time": TIM))" } },
    { 0x33, { "ColorField", "Colors entire map specified colors. (-128,-128,-128) is black and (+127,+127,+127) is white while (+000,+000,+000) returns the map to its original color.", R"(ColorField("Pre-set Color": xPR,"Red": +RED,"Green": +GRN,"Blue": +BLU,"Time": TIM))" } },
    { 0x38, { "FocusSpeed", "Sets the traveling speed for the camera instruction using Focus' settings.", R"(FocusSpeed("Travel Speed": +SPEED))" } },
    { 0x3B, { "SpriteMove", "Moves Target Unit to specified coordinates relative to its starting position, ignoring the field and the units statistics such as Jump.", R"(SpriteMove("Unit ID": xID,x00,"X Movement": +XXXXX,"Z Movement": +ZZZZZ,"Y Movement": +YYYYY,"Movement Type": xMV,"Unknown": x??,"Time": +TIMER))" } },
    { 0x3C, { "Weather", "Controls the weather's power, but not the weather type.", R"(Weather("Weather Power": xWP,"Unknown": x01))" } },
    { 0x3D, { "RemoveUnit", "Removes a unit from the field and memory entirely. This differs from BlueRemoveUnit in that it works immediately, allowing you to Add another unit/sprite right away, and that a Removed unit doesn't award its War Trophy.", R"(RemoveUnit("Unit ID": xID,x00))" } },
    { 0x3E, { "ColorScreen", "Colors the whole screen, in different possible ways. The Initial Color is applied immediately, while the Target Color gradually changes with a given amount of time assigned. (255,255,255) is white while (000,000,000) is no coloration. Not battle-friendly.", R"(ColorScreen("Blending Mode": xBM,"Initial Red": IRD,"Initial Green": IGR,"Initial Blue": IBL,"Target Red": TRD,"Target Green": TGR,"Target Blue": TBL,"Time": +TIMER))" } },
    { 0x41, { "EarthquakeStart", "Cause a seism with given parameters, but it does not control any sound related to it. Not battle-friendly.", R"(EarthquakeStart("Magnitude": MAG,"Mercalli Intensity Scale": MER,"Secondary Shock Magnitude": SMG,"Secondary Shock Delay": SSD))" } },
    { 0x42, { "EarthquakeEnd", "Stops an ongoing earthquake.", R"(EarthquakeEnd())" } },
    { 0x43, { "CallFunction", "Can call various different functions to alter many different things in the game.", R"(CallFunction("Function": xFC))" } },
    { 0x44, { "Draw", "Draws a loaded unit that is currently not being displayed.", R"(Draw("Unit ID": xID,x00))" } },
    { 0x45, { "AddUnit", "Adds a unit to the event that is not currently loaded.", R"(AddUnit("Unit ID": xID,x00,"Drawing": xDR))" } },
    { 0x46, { "Erase", "Erase a unit whose sprite is currently being displayed. Unit can be re-drawn later.", R"(Erase("Unit ID": xID,x00))" } },
    { 0x47, { "AddGhostUnit", "Adds a fake unit on the map which can be mostly controlled like a regular unit, with some exceptions. In battle, the unit cannot take action or be targeted, and units will be able to walk through it. You can use any spritesheet you want, but be warned: Adding a new spritesheet will leave you with the impossibility of removing the spritesheet even if you remove the Ghost Unit.", R"(AddGhostUnit("Spritesheet ID": xSP,x00,"Assigned Unit ID": xID,"X Coordinate": XXX,"Y Coordinate": YYY,"Elevation": xEL,"Facing Direction": xFD,"Drawing": xDR))" } },
    { 0x48, { "WaitAddUnit", "Waits until a normal or ghost unit is loaded into the event before resuming.", R"(WaitAddUnit())" } },
    { 0x49, { "AddUnitStart", "Creates an independently running block inside the event to load one or many units in the game. Does not slow down the event in any way, but make sure to use FIXME", R"(AddUnitStart())" } },
    { 0x4A, { "AddUnitEnd", "Creates an independently running block inside the event to load one or many units in the game. Does not slow down the event in any way, but make sure to use FIXME", R"(AddUnitEnd())" } },
    { 0x4B, { "WaitAddUnitEnd", "Waits until a currently running FIXME", R"(WaitAddUnitEnd())" } },
    { 0x4C, { "ChangeMap", "Fades to black, then changes the map, reloading all the settings such as Weather or State.", R"(ChangeMap("Map": MAP,x00))" } },
    { 0x4D, { "Reveal", "Progressively reveals the screen to unveil the scene. Required to start a new scenario.", R"(Reveal("Time": TIM))" } },
    { 0x4E, { "UnitShadow", "Remove or add back the shadow of a unit. Battle-friendly.", R"(UnitShadow("Unit ID": xID,x00,"Shadow": xSH))" } },
    { 0x4F, { "SetDaytime", "Stores day or night for the next time you will load/reload a map.", R"(SetDaytime("Daytime": xDT))" } },
    { 0x50, { "PortraitCol", "Sets the column that will be used to load a custom portrait from EVTFACE.BIN for the following Dialog Boxes.", R"(PortraitCol("Portrait Column": xPC))" } },
    { 0x51, { "ChangeDialog", "Changes the content of a Dialog Box or closes it.", R"(ChangeDialog("Target Dialog Box": xDB,"New Message ID": xMSG#,"Portrai},t Row": xPR,"Portrait Palette": xPP))" } },
    { 0x53, { "FaceUnit", "Makes unit(s) rotate to face a specified unit. The rotation direction is the shortest one (Clockwise as default).", R"(FaceUnit("Faced Unit ID": xFU,x00,"Affected Units": xAU,"Multi Targeting": xMT,"Clockwise or Counter-clockwise": xCL,"Rotation Spe},ed": xRS,"Delay": xDL))" } },
    { 0x54, { "Use3DObject", "Use an object that has a 3D animation.", R"(Use3DObject(Object ID: xID,"New State": xST))" } },
    { 0x55, { "UseFieldObject", "Changes the state of an object on the map.", R"(UseFieldObject(State Change ID: xID,x00))" } },
    { 0x56, { "Wait3DObject", "Waits until all activated 3D objects are done animating before resuming.", R"(Wait3DObject())" } },
    { 0x57, { "WaitFieldObject", "Waits for an object being used to complete its cycle before resuming.", R"(WaitFieldObject())" } },
    { 0x58, { "LoadEVTCHR", "Loads an EVTCHR slot from the CD to temporary memory.", R"(LoadEVTCHR("Memory Block": xBL,"EVTCHR Slot": xEV,x00))" } },
    { 0x59, { "SaveEVTCHR", "Saves the loaded EVTCHR Slot to a specific block in order to use its frames and animations.", R"(SaveEVTCHR("Memory Block": xBL))" } },
    { 0x5A, { "SaveEVTCHRClear", "Allows to save an EVTCHR slot once more in the given memory block.", R"(SaveEVTCHRClear("Memory Block": xBL,x00))" } },
    { 0x5B, { "LoadEVTCHRClear", "Allows to load an EVTCHR slot once more in the given memory block.", R"(LoadEVTCHRClear("Memory Block": xBL,x00))" } },
    { 0x5E, { "EndTrack", "Terminates the currently playing song immediately.", R"(EndTrack("Unknown": x??))" } },
    { 0x5F, { "WarpUnit", "Warps a unit to a new location instantly.", R"(WarpUnit("UnitID": xID,x00,"X Coordinate": XXX,"Y Coordina},te": YYY,"Elevation": xEL,"Facing Direction": xFD))" } },
    { 0x60, { "FadeSound", "Progressively fade sounds and music until they stop playing completely.", R"(FadeSound(x00,"Time": TIM))" } },
    { 0x63, { "CameraSpeedCurve", "Sets the acceleration strength and type of the next Camera instruction. The first digit affects the strength; a value of 0 is unnoticeable, a value of F is the most noticeable. The second digit determines the type of acceleration: None, Immediate, or Delayed.", R"(CameraSpeedCurve("Parameters": xPA))" } },
    { 0x64, { "WaitRotateUnit", "Waits for a unit to finish rotating/facing another unit before resuming.", R"(WaitRotateUnit("Unit ID": xID,x00))" } },
    { 0x65, { "WaitRotateAll", "Waits for every unit to finish rotating/facing another unit before resuming.", R"(WaitRotateAll())" } },
    { 0x68, { "MirrorSprite", "Mirrors a sprite and all its animations/ETVCHR as if they were facing left instead of right, and vice versa.", R"(MirrorSprite("Unit ID": xID,x00,"Mirror": xMI))" } },
    { 0x69, { "FaceTile", "Makes unit(s) rotate to face a specified tile. The rotation direction is the shortest one (Clockwise as default).", R"(FaceTile("Affected Units": xAU,"Multi Targeting": xMT,"X Coordinate": XXX,"Y Coordinate": YYY,"Unknown": x00,"Clockwise or Counter},-clockwise": xCL,"Rotation Speed": xRS,"Delay": xDL))" } },
    { 0x6A, { "EditBGSound", "Edits a currently playing Background sound.", R"(EditBGSound("Sound ID": xSD,"Echo": +ECH,"Volume": +VOL,"Unknow},n": x00,"Unknown": xWT))" } },
    { 0x6B, { "BGSound", "Plays the indexed background sound effect.", R"(BGSound("Sound ID": xSD,"Echo": +ECH,"Volume": +VOL,"Sound}, Stacking": xST,"Time": TIM))" } },
    { 0x6E, { "SpriteMoveBeta", "Very similar to its sister instruction, but instead uses different movement types and uses speed instead of time. Moves Target Unit to specified coordinates relative to its starting position, ignoring the field and the units statistics such as Jump.", R"(SpriteMoveBeta("Unit ID": xID,x00,"X Movement": +XXXXX,"Z Movement": +ZZZZZ,"Y Movement": +YYYYY,"Movement Type": xMV,"Unknown": x??,"Speed": +SPEED))" } },
    { 0x6F, { "WaitSpriteMove", "Wait for a unit to complete its custom movement before resuming.", R"(WaitSpriteMove("Unit ID": xID,x00))" } },
    { 0x70, { "Jump", "The given unit will jump up or down 1-4 tiles distance. It works perfectly for jumping down, but might look weird when trying to jump to tiles too height. Does not have an elevation parameter. If the height difference is very minimal, the unit will just walk instead. If the target tile is off the map, the unit will freeze in its frame and slide off the screen.", R"(Jump("Unit ID": xID,x00,"Distance": DST,"Direction": xDR))" } },
    { 0x76, { "DarkScreen", "Create a dark screen that allows units to join your party, display winning conditions and save your party back to formation.", R"(DarkScreen(x00,"Shape": xSH,"Screen Expansion Speed": ESP,"Rotation Speed": RTS,x00,"Square Expansion Speed": SES))" } },
    { 0x77, { "RemoveDarkScreen", "Removes the dark screen with the same parameters it was called with.", R"(RemoveDarkScreen())" } },
    { 0x78, { "DisplayConditions", "Display a Winning condition, Congratulations, War Trophies or Bonus Money received after the battle.", R"(DisplayConditions("Message": xMG,"Display Time": DST))" } },
    { 0x79, { "WalkToAnim", "Applies a normal or an EVTCHR animation to a given unit when it arrives at destination.", R"(FIXME)" } },
    { 0x7A, { "DismissUnit", "Removes the first unit which matches the given Special Job ID from the player's roster.", R"(DismissUnit("Special Job ID": xJB,x00))" } },
    { 0x7D, { "ShowGraphic", "Gradually prints a graphic on the screen, and erases it the same way.", R"(ShowGraphic("Graphic ID": xGR))" } },
    { 0x7E, { "WaitValue", "Waits until a given value has reached at least a certain amount before resuming.", R"(WaitValue("Address": xADDR,"Value": xVALU))" } },
    { 0x7F, { "EVTCHRPalette", "Changes the palette of a character by loading one from an EVTCHR Block.", R"(EVTCHRPalette("UnitID": xID,x00,"EVTCHR Block": xBL,"Palette ID": xPL))" } },
    { 0x80, { "March", "Make units start their walking animation, ready to fight. Even if not called, this instruction is automatically executed when a battle starts with 0 frames between units.", R"(March("Affected Units": xAU,"Multi Targeting": xMT,"Time": TIM))" } },
    { 0x83, { "ChangeStats", "Change the stats of affected units.", R"(ChangeStats("Affected Units": xAU,"Multi Targeting": xM},T,"Stat": xST,"Value": +VALUE))" } },
    { 0x84, { "PlayTune", "Plays a song file. Don't get excited, the list is very short and mostly useless.", R"(PlayTune("Song ID": xSG))" } },
    { 0x85, { "UnlockDate", "Sets the unlocking date of a treasure/land to the current day.", R"(UnlockDate("Treasure/Land": xTL))" } },
    { 0x86, { "TempWeapon", "Gives unit a temporary weapon to swing. Does not affect equipped items.", R"(TempWeapon("Unit ID": xID,x00,"Item ID": xIT))" } },
    { 0x87, { "Arrow", "Adds an arrow or bolt the next time a unit that has a bow (arrow) or a crossbow (bolt) shoots with a SEQ animation of x53, x54 or x55.", R"(Arrow("Target's Unit ID": xTG,x00,"Shooter's Unit ID": xSH,x00))" } },
    { 0x88, { "MapUnfreeze", "Allows animations of a map again.", R"(MapUnfreeze())" } },
    { 0x89, { "MapFreeze", "Freezes all animations of a map.", R"(MapFreeze())" } },
    { 0x8A, { "EffectStart", "Must be used in a block to activate the animation of a previously set Effect file.", R"(EffectStart())" } },
    { 0x8B, { "EffectEnd", "Must be used in a block to activate the animation of a previously set Effect file.", R"(EffectEnd())" } },
    { 0x8C, { "UnitAnimRotate", "Makes the unit immediately rotate to a certain direction and execute an animation.", R"(UnitAnimRotate("Unit ID": xID,x00,"Direction": xDR,"Animation ID": xANIM,x00))" } },
    { 0x8E, { "WaitGraphicPrint", "FIXME", R"(WaitGraphicPrint())" } },
    { 0x91, { "ShowMapTitle", "Displays a 256x20 4bit image (EVENT/MAPTITLE.BIN) of the current map's title on the screen. Only Map 1 to 120 have a graphic assigned to them. Trying to display the title of other maps will result in rubbish.", R"(ShowMapTitle(X Axis: XXX,Y Axis: +YYY,Speed: +SPD))" } },
    { 0x92, { "InflictStatus", "Inflict a status on a given unit.", R"(InflictStatus("Unit ID": xID,x00,"Status": xSS,"Unknown},": x0C,x00))" } },
    { 0x94, { "TeleportOut", "Unit will teleport out of the field.", R"(TeleportOut("Unit ID": xID,x00))" } },
    { 0x96, { "AppendMapState ", "FIXME", R"(AppendMapState())" } },
    { 0x97, { "ResetPalette", "Resets the palette color of a unit to its original state.", R"(ResetPalette("Unit ID": xID,x00))" } },
    { 0x98, { "TeleportIn", "Unit will teleport onto the field.", R"(TeleportIn("Unit ID": xID,x00))" } },
    { 0x99, { "BlueRemoveUnit", "Remove a non-Charmed enemy team unit from the map with a blue hueing. This differs from RemoveUnit in that it takes longer, so you can't Add another unit/sprite in immediately afterwards, and that a BlueRemoved unit will still award their War Trophies at the end.", R"(BlueRemoveUnit("Unit ID": xID,x00))" } },
    { 0xA0, { "LTE", "Less Than or Equal | Variable 0x0000 = If ( Variable 0x0000 <= Variable 0x0001 )", R"(LTE())" } },
    { 0xA1, { "GTE", "", R"(GTE())" } },
    { 0xA2, { "EQ", "Equal | Variable 0x0000 = If ( Variable 0x0000 == Variable 0x0001 )", R"(EQ())" } },
    { 0xA3, { "NEQ", "Not Equal | Variable 0x0000 = If ( Variable 0x0000 != Variable 0x0001 )", R"(NEQ())" } },
    { 0xA4, { "LT", "Less Than | Variable 0x0000 = If ( Variable 0x0000 < Variable 0x0001 )", R"(LT())" } },
    { 0xA5, { "GT", "Greater Than | Variable 0x0000 = If ( Variable 0x0000", R"(GT())" } },
    { 0xB0, { "ADD", "Add Immediate | Variable = Variable + ImmediateValue (Can overflow)", R"(ADD("Variable": xVARI,"Immediate Value": xVALU))" } },
    { 0xB1, { "ADDVar", "Add Variable | Variable1 = Variable1 + Variable2 (Can overflow)", R"(ADDVar("Variable 1": xVAR1,"Variable 2": xVAR2))" } },
    { 0xB2, { "SUB", "Subtract Immediate | Variable = Variable - ImmediateValue (Can overflow)", R"(SUB("Variable": xVARI,"Immediate Value": xVALU))" } },
    { 0xB3, { "SUBVar", "Subtract Variable | Variable1 = Variable1 - Variable2 (Can overflow)", R"(SUBVar("Variable 1": xVAR1,"Variable 2": xVAR2))" } },
    { 0xB4, { "MULT", "Multiply Immediate | Variable = Variable × ImmediateValue (low 32bit) Use this instruction for general multiplication.", R"(MULT("Variable": xVARI,"Immediate Value": xVALU))" } },
    { 0xB5, { "MULTVar", "Multiply Variable | Variable1 = Variable1 × Variable2 (low 32bit) Use this instruction for general multiplication.", R"(MULTVar("Variable 1": xVAR1,"Variable 2": xVAR2))" } },
    { 0xB6, { "DIV", "Divide Immediate | Variable = Variable ÷ ImmediateValue (low 32bit) Use this instruction for general division.", R"(DIV("Variable": xVARI,"Immediate Value": xVALU))" } },
    { 0xB7, { "DIVVar", "Divide Variable | Variable1 = Variable1 ÷ Variable2 (low 32bit) Use this instruction for general division.", R"(DIVVar("Variable 1": xVAR1,"Variable 2": xVAR2))" } },
    { 0xB8, { "MOD", "Divide Immediate (modulus) | Variable = Variable % ImmediateValue (high 32bit)", R"(MOD("Variable": xVARI,"Immediate Value": xVALU))" } },
    { 0xB9, { "MODVar", "Divide Immediate | Variable = Variable ÷ ImmediateValue (high 32bit)", R"(MODVar("Variable 1": xVAR1,"Variable 2": xVAR2))" } },
    { 0xBA, { "AND", "AND bitwise operand | Variable = Variable AND ImmediateValue This command compares the individual bits of the value within a Variable and a specific ImmediateValue, and produces a single binary output. This makes this a good way to disable a specific bit within a byte without changing the rest of the bits. With AND, the resulting bit outputs are set to 1 (TRUE) only if both values' bit inputs are 1. The bits available within a single byte are: * 0x80 * 0x40 * 0x20 * 0x10 * 0x08 * 0x04 * 0x02 * 0x01 So, for example, if you want to edit a unit's Battle Stats to remove the Dead status (which is bit 0x20), but leave their other statii unchanged, you would UnitAddress their Unit ID, LoadAddress their stats at 0x0058 and 0x01BB into temporary variables (let's say 0x0070 and 0x0071), and then run AND(x0070,x00DF) and AND(x0071,x00DF), before using SaveAddress to put the corrected values back. Because the Dead status is in bit 0x20, a value of DF means that every bit except 0x20 is set to 1. As a result, the other statii in that byte will remain unchanged, but by forcing 0x20 to be set to 1 in the ImmediateValue, it will zero out the Dead bit in the final result.", R"(AND("Variable": xVARI,"Immediate Value": xVALU))" } },
    { 0xBB, { "ANDVar", "AND Variable bitwise operand | Variable1 = Variable1 AND Variable2 This command compares the individual bits of the value between two Variables, and produces a single binary output. With AND, the resulting bit outputs are set to 1 (TRUE) only if both values' bit inputs are 1. This makes this a good way to disable a specific bit within a byte without changing the rest of the bits. The bits available within a single byte are: * 0x80 * 0x40 * 0x20 * 0x10 * 0x08 * 0x04 * 0x02 * 0x01 So, for example, if you want to edit a unit's Battle Stats to remove the Dead status (which is bit 0x20), but leave their other statii unchanged, you would UnitAddress their Unit ID, LoadAddress their stats at 0x0058 and 0x01BB into temporary variables (let's say 0x0070 and 0x0071), SET(x0072,x00DF), and then run ANDVar(x0070,x0072) and ANDVar(x0071,x0072), before using SaveAddress to put the corrected values back. Because the Dead status is in bit 0x20, a value of DF means that every bit except 0x20 is set to 1. As a result, the other statii in that byte will remain unchanged, but by forcing 0x20 to be set to 1 in the ImmediateValue, it will zero out the Dead bit in the final result.", R"(ANDVar("Variable 1": xVAR1,"Variable 2": xVAR2))" } },
    { 0xBC, { "OR", "OR bitwise operand | Variable = Variable OR ImmediateValue This command compares the individual bits of the value between a Variable and a specific ImmediateValue, and produces a single binary output. With OR, the resulting bit outputs are set to 1 (TRUE) if either value's bit input is 1. This makes this a good way to enable a specific bit within a byte without changing the rest of the bits. The bits available within a single byte are: * 0x80 * 0x40 * 0x20 * 0x10 * 0x08 * 0x04 * 0x02 * 0x01 So, for example, if you want to edit a unit's Battle Stats to add the Control flag (which is bit 0x08), but leave the other flags such as their Team untouched, you would LoadAddress the byte into an editable variable, then OR that variable against a value of 0008: OR(xVARI,x0008). This uses a zero value on every bit except 08, which uses a one value, forcing the output to always have Control flagged, while leaving the rest of the bits from the variable unaltered.", R"(OR("Variable": xVARI,"Immediate Value": xVALU))" } },
    { 0xBD, { "ORVar", "OR Variable bitwise operand | Variable1 = Variable1 OR Variable2 This command compares the individual bits of the value between a Variable and a specific ImmediateValue, and produces a single binary output. With OR, the resulting bit outputs are set to 1 (TRUE) if either value's bit input is 1. This makes this a good way to enable a specific bit within a byte without changing the rest of the bits. The bits available within a single byte are: * 0x80 * 0x40 * 0x20 * 0x10 * 0x08 * 0x04 * 0x02 * 0x01 So, for example, if you want to edit a unit's Battle Stats to add the Control flag (which is bit 0x08), but leave the other flags such as their Team untouched, you would LoadAddress the byte into an editable variable, then OR that variable against a value of 0008: OR(xVARI,x0008). This uses a zero value on every bit except 08, which uses a one value, forcing the output to always have Control flagged, while leaving the rest of the bits from the variable unaltered.", R"(ORVar("Variable 1": xVAR1,"Variable 2": xVAR2))" } },
    { 0xBE, { "ZERO", "Zero | Variable = 0", R"(ZERO("Variable": xVARI))" } },
    { 0xD0, { "JumpForwardIfZero", "FIXME", R"(JumpForwardIfZero("JumpForward ID": xJF))" } },
    { 0xD1, { "JumpForward ", "FIXME", R"(JumpForward("JumpForward ID": xJF))" } },
    { 0xD2, { "ForwardTarget ", "FIXME", R"(ForwardTarget("JumpForwar},d ID": xJF))" } },
    { 0xD3, { "JumpBack ", "FIXME", R"(JumpBack("JumpBack ID": xJB))" } },
    { 0xD5, { "BackTarget ", "FIXME", R"(BackTarget("JumpBack ID": xJB))" } },
    { 0xDB, { "EventEnd", "Marks the end of the event.", R"(EventEnd())" } },
    { 0xE3, { "EventEnd2", "A perfect copy of the other EventEnd. Marks the end of the event.", R"(EventEnd2())" } },
    { 0xE5, { "WaitForInstruction", "Wait for any given instruction type instance to complete before resuming.", R"(WaitForInstruction("Instruction Type": xIN,x00))" } },
    { 0xF1, { "Wait", "Wait for a given number of frames before resuming.", R"(Wait("Time": TIMER))" } },
    { 0xF2, { "Pad", "Doesn't do anything. Ideal for hex editing parts of the event out without any risk.", R"(Pad())" } },
};

std::map<int, std::string> scenario_list = {
    { 0x001, "Dolbodar Swamp East 1" },
    { 0x002, "Dolbodar Swamp East 2" },
    { 0x003, "Dolbodar Swamp East 3" },
    { 0x004, "Dolbodar Swamp East 4" },
    { 0x005, "Dolbodar Swamp West 1" },
    { 0x006, "Dolbodar Swamp West 2" },
    { 0x007, "Dolbodar Swamp West 3" },
    { 0x008, "Dolbodar Swamp West 4" },
    { 0x009, "Tutorial (Charge Time Battle)" },
    { 0x00A, "Tutorial (How to Cast Spells)" },
    { 0x00B, "Tutorial (Move and Act)" },
    { 0x00C, "Tutorial (Online Help)" },
    { 0x00D, "Fovoham Plains South 1" },
    { 0x00E, "Fovoham Plains South 2" },
    { 0x00F, "Fovoham Plains South 3" },
    { 0x010, "Fovoham Plains South 4" },
    { 0x011, "Fovoham Plains West 1" },
    { 0x012, "Fovoham Plains West 2" },
    { 0x013, "Fovoham Plains West 3" },
    { 0x014, "Fovoham Plains West 4" },
    { 0x015, "Fovoham Plains East 1" },
    { 0x016, "Fovoham Plains East 2" },
    { 0x017, "Fovoham Plains East 3" },
    { 0x018, "Fovoham Plains East 4" },
    { 0x019, "Sweegy Woods East 1" },
    { 0x01A, "Sweegy Woods East 2" },
    { 0x01B, "Sweegy Woods East 3" },
    { 0x01C, "Sweegy Woods East 4" },
    { 0x01D, "Sweegy Woods West 1" },
    { 0x01E, "Sweegy Woods West 2" },
    { 0x01F, "Sweegy Woods West 3" },
    { 0x020, "Sweegy Woods West 4" },
    { 0x021, "Tutorial (Battle)" },
    { 0x025, "Bevernia Volcano North 1" },
    { 0x026, "Bevernia Volcano North 2" },
    { 0x027, "Bevernia Volcano North 3" },
    { 0x028, "Bevernia Volcano North 4" },
    { 0x029, "Bevernia Volcano South 1" },
    { 0x02A, "Bevernia Volcano South 2" },
    { 0x02B, "Bevernia Volcano South 3" },
    { 0x02C, "Bevernia Volcano South 4" },
    { 0x02D, "Tutorial (Battlefield Control)" },
    { 0x02E, "Tutorial (Abnormal Status)" },
    { 0x02F, "Tutorial (Options)" },
    { 0x031, "Zeklaus Desert North 1" },
    { 0x032, "Zeklaus Desert North 2" },
    { 0x033, "Zeklaus Desert North 3" },
    { 0x034, "Zeklaus Desert North 4" },
    { 0x035, "Zeklaus Desert South 1" },
    { 0x036, "Zeklaus Desert South 2" },
    { 0x037, "Zeklaus Desert South 3" },
    { 0x038, "Zeklaus Desert South 4" },
    { 0x039, "Zeklaus Desert East 1" },
    { 0x03A, "Zeklaus Desert East 2" },
    { 0x03B, "Zeklaus Desert East 3" },
    { 0x03C, "Zeklaus Desert East 4" },
    { 0x03D, "Lenalia Plateau South 1" },
    { 0x03E, "Lenalia Plateau South 2" },
    { 0x03F, "Lenalia Plateau South 3" },
    { 0x040, "Lenalia Plateau South 4" },
    { 0x041, "Lenalia Plateau North 1" },
    { 0x042, "Lenalia Plateau North 2" },
    { 0x043, "Lenalia Plateau North 3" },
    { 0x044, "Lenalia Plateau North 4" },
    //
    { 0x049, "Zigolis Swamp East 1" },
    { 0x04A, "Zigolis Swamp East 2" },
    { 0x04B, "Zigolis Swamp East 3" },
    { 0x04C, "Zigolis Swamp East 4" },
    { 0x04D, "Zigolis Swamp West 1" },
    { 0x04E, "Zigolis Swamp West 2" },
    { 0x04F, "Zigolis Swamp West 3" },
    { 0x050, "Zigolis Swamp West 4" },
    //
    { 0x052, "Zirekile Falls East 5" },
    { 0x053, "Barius Hill South 5" },
    { 0x054, "Lenalia Plateau South 5" },
    { 0x055, "Yuguo Woods West 1" },
    { 0x056, "Yuguo Woods West 2" },
    { 0x057, "Yuguo Woods West 3" },
    { 0x058, "Yuguo Woods West 4" },
    { 0x059, "Yuguo Woods East 1" },
    { 0x05A, "Yuguo Woods East 2" },
    { 0x05B, "Yuguo Woods East 3" },
    { 0x05C, "Yuguo Woods East 4" },
    { 0x05D, "Dolbodar Swamp West 5" },
    { 0x05E, "Grog Hill South 5" },
    { 0x05F, "Bevernia Volcano North 5" },
    { 0x060, "Barius Valley South 5" },
    { 0x061, "Araguay Woods West 1" },
    { 0x062, "Araguay Woods West 2" },
    { 0x063, "Araguay Woods West 3" },
    { 0x064, "Araguay Woods West 4" },
    { 0x065, "Araguay Woods East 1" },
    { 0x066, "Araguay Woods East 2" },
    { 0x067, "Araguay Woods East 3" },
    { 0x068, "Araguay Woods East 4" },
    { 0x069, "Finath River East 5" },
    { 0x06A, "Germina Peak North 5" },
    { 0x06B, "Araguay Woods South 5" },
    { 0x06C, "Yuguo Woods East 5" },
    { 0x06D, "Grog Hill West 1" },
    { 0x06E, "Grog Hill West 2" },
    { 0x06F, "Grog Hill West 3" },
    { 0x070, "Grog Hill West 4" },
    { 0x071, "Grog Hill South 1" },
    { 0x072, "Grog Hill South 2" },
    { 0x073, "Grog Hill South 3" },
    { 0x074, "Grog Hill South 4" },
    { 0x075, "Grog Hill East 1" },
    { 0x076, "Grog Hill East 2" },
    { 0x077, "Grog Hill East 3" },
    { 0x078, "Grog Hill East 4" },
    { 0x079, "Bed Desert South 1" },
    { 0x07A, "Bed Desert South 2" },
    { 0x07B, "Bed Desert South 3" },
    { 0x07C, "Bed Desert South 4" },
    { 0x07D, "Bed Desert North 1" },
    { 0x07E, "Bed Desert North 2" },
    { 0x07F, "Bed Desert North 3" },
    { 0x080, "Bed Desert North 4" },
    { 0x081, "Bed Desert North 5" },
    { 0x082, "Fovoham Plains West 5" },
    { 0x083, "Doguola Pass West 5" },
    { 0x084, "Sweegy Woods East 5" },
    { 0x085, "Zirekile Falls West 1" },
    { 0x086, "Zirekile Falls West 2" },
    { 0x087, "Zirekile Falls West 3" },
    { 0x088, "Zirekile Falls West 4" },
    { 0x089, "Zirekile Falls East 1" },
    { 0x08A, "Zirekile Falls East 2" },
    { 0x08B, "Zirekile Falls East 3" },
    { 0x08C, "Zirekile Falls East 4" },
    { 0x08D, "Zirekile Falls South 1" },
    { 0x08E, "Zirekile Falls South 2" },
    { 0x08F, "Zirekile Falls South 3" },
    { 0x090, "Zirekile Falls South 4" },
    { 0x091, "Barius Hill North 1" },
    { 0x092, "Barius Hill North 2" },
    { 0x093, "Barius Hill North 3" },
    { 0x094, "Barius Hill North 4" },
    { 0x095, "Barius Hill South 1" },
    { 0x096, "Barius Hill South 2" },
    { 0x097, "Barius Hill South 3" },
    { 0x098, "Barius Hill South 4" },
    { 0x099, "Poeskas Lake North 5" },
    { 0x09A, "Zigolis Swamp West 5" },
    { 0x09B, "Mandalia Plains South 5" },
    { 0x09C, "Zeklaus Desert South 5" },
    { 0x09D, "Mandalia Plains North 1" },
    { 0x09E, "Mandalia Plains North 2" },
    { 0x09F, "Mandalia Plains North 3" },
    { 0x0A0, "Mandalia Plains North 4" },
    { 0x0A1, "Mandalia Plains South 1" },
    { 0x0A2, "Mandalia Plains South 2" },
    { 0x0A3, "Mandalia Plains South 3" },
    { 0x0A4, "Mandalia Plains South 4" },
    { 0x0A5, "Mandalia Plains West 1" },
    { 0x0A6, "Mandalia Plains West 2" },
    { 0x0A7, "Mandalia Plains West 3" },
    { 0x0A8, "Mandalia Plains West 4" },
    { 0x0A9, "Doguola Pass East 1" },
    { 0x0AA, "Doguola Pass East 2" },
    { 0x0AB, "Doguola Pass East 3" },
    { 0x0AC, "Doguola Pass East 4" },
    { 0x0AD, "Doguola Pass West 1" },
    { 0x0AE, "Doguola Pass West 2" },
    { 0x0AF, "Doguola Pass West 3" },
    { 0x0B0, "Doguola Pass West 4" },
    { 0x0B1, "End 1" },
    { 0x0B2, "End 2" },
    { 0x0B3, "End 3" },
    { 0x0B4, "End 4" },
    { 0x0B5, "Barius Valley West 1" },
    { 0x0B6, "Barius Valley West 2" },
    { 0x0B7, "Barius Valley West 3" },
    { 0x0B8, "Barius Valley West 4" },
    { 0x0B9, "Barius Valley East 1" },
    { 0x0BA, "Barius Valley East 2" },
    { 0x0BB, "Barius Valley East 3" },
    { 0x0BC, "Barius Valley East 4" },
    { 0x0BD, "Barius Valley South 1" },
    { 0x0BE, "Barius Valley South 2" },
    { 0x0BF, "Barius Valley South 3" },
    { 0x0C0, "Barius Valley South 4" },
    { 0x0C1, "Finath River West 1" },
    { 0x0C2, "Finath River West 2" },
    { 0x0C3, "Finath River West 3" },
    { 0x0C4, "Finath River West 4" },
    { 0x0C5, "Finath River East 1" },
    { 0x0C6, "Finath River East 2" },
    { 0x0C7, "Finath River East 3" },
    { 0x0C8, "Finath River East 4" },
    { 0x0C9, "Horror" },
    { 0x0CA, "Horror" },
    { 0x0CB, "Horror" },
    { 0x0CC, "Horror" },
    { 0x0CD, "Poeskas Lake North 1" },
    { 0x0CE, "Poeskas Lake North 2" },
    { 0x0CF, "Poeskas Lake North 3" },
    { 0x0D0, "Poeskas Lake North 4" },
    { 0x0D1, "Poeskas Lake South 1" },
    { 0x0D2, "Poeskas Lake South 2" },
    { 0x0D3, "Poeskas Lake South 3" },
    { 0x0D4, "Poeskas Lake South 4" },
    { 0x0D5, "Voyage 1" },
    { 0x0D6, "Voyage 2" },
    { 0x0D7, "Voyage 3" },
    { 0x0D8, "Voyage 4" },
    { 0x0D9, "Germina Peak North 1" },
    { 0x0DA, "Germina Peak North 2" },
    { 0x0DB, "Germina Peak North 3" },
    { 0x0DC, "Germina Peak North 4" },
    { 0x0DD, "Germina Peak South 1" },
    { 0x0DE, "Germina Peak South 2" },
    { 0x0DF, "Germina Peak South 3" },
    { 0x0E0, "Germina Peak South 4" },
    { 0x0E1, "Bridge 1" },
    { 0x0E2, "Bridge 2" },
    { 0x0E3, "Bridge 3" },
    { 0x0E4, "Bridge 4" },
    { 0x0E5, "Tiger 1" },
    { 0x0E6, "Tiger 2" },
    { 0x0E7, "Tiger 3" },
    { 0x0E8, "Tiger 4" },
    { 0x0E9, "Mlapan 1" },
    { 0x0EA, "Mlapan 2" },
    { 0x0EB, "Mlapan 3" },
    { 0x0EC, "Mlapan 4" },
    { 0x0ED, "Valkyries 1" },
    { 0x0EE, "Valkyries 2" },
    { 0x0EF, "Valkyries 3" },
    { 0x0F0, "Valkyries 4" },
    { 0x0F1, "Delta 1" },
    { 0x0F2, "Delta 2" },
    { 0x0F3, "Delta 3" },
    { 0x0F4, "Delta 4" },
    { 0x0F5, "Terminate 1" },
    { 0x0F6, "Terminate 2" },
    { 0x0F7, "Terminate 3" },
    { 0x0F8, "Terminate 4" },
    { 0x0F9, "Nogias 1" },
    { 0x0FA, "Nogias 2" },
    { 0x0FB, "Nogias 3" },
    { 0x0FC, "Nogias 4" },
    //
    { 0x100, "Orbonne Prayer and BS" },
    { 0x101, "Larg's Praise and BS" },
    { 0x102, "Military Academy BS" },
    { 0x103, "Family Meeting BS" },
    { 0x104, "Balbanes Death and BS" },
    { 0x105, "Releasing Miluda BS" },
    { 0x106, "Introducing Algus BS" },
    { 0x107, "Returning to Igros and BS" },
    { 0x108, "Attack on the Beoulves BS" },
    { 0x109, "Interrogation BS" },
    { 0x10A, "Gustav vs Wiegraf " },
    { 0x10B, "Bedridden Dycedarg and BS" },
    { 0x10C, "Reed Whistle" },
    { 0x10D, "Wiegraf berating Golagros" },
    { 0x10E, "Finding Teta Missing" },
    { 0x10F, "Partings" },
    { 0x110, "Ch2 Start Orbonne Monastery" },
    { 0x111, "Ovelia Joins BS" },
    { 0x112, " Ramza Mustadio Agrias Ovelia Meeting BS" },
    { 0x113, "Ruins of Zaland and BS" },
    { 0x114, "Dycedarg and Gafgarion Reunion and BS" },
    { 0x115, "Besrodio Kidnapped BS" },
    { 0x116, "Gate of Lionel Castle and BS" },
    { 0x117, " Meet Draclau and BS" },
    { 0x118, "Goug Machine City Town" },
    { 0x119, "Besrodio Saved BS" },
    { 0x11A, "Warjilis Port and BS" },
    { 0x11B, "Draclau hires Gafgarion and BS" },
    { 0x11C, "Substitute and BS" },
    { 0x11D, "Gelwan's Death and BS" },
    { 0x11E, "Ch2 Start Orbonne Monastery BS" },
    { 0x11F, "Chapter 3 Start" },
    { 0x120, "Chapter 3 Start BS" },
    { 0x121, "Goland Coal City Post Battle" },
    { 0x122, "Steel Ball Found and BS" },
    { 0x123, "Worker 8 Activated and BS" },
    { 0x124, "Summoning Machine Found and BS" },
    { 0x125, "Cloud Summoned and BS" },
    { 0x126, "Talk with Zalbag in Lesalia and BS" },
    { 0x127, "Lesalia Gate Talk with Alma BS" },
    { 0x128, "Orbonne Monastery (Ch3) and BS" },
    { 0x129, "Meet Velius BS" },
    { 0x12A, "Malak and the Scriptures" },
    { 0x12B, "Delitas allegiance to Ovelia and BS" },
    { 0x12C, "Meet Again with Olan BS" },
    { 0x12D, "Exploding Frog BS" },
    { 0x12E, "Barinten threatens Vormav and BS" },
    { 0x12F, "Escaping Alma and BS" },
    { 0x130, "Ajora's vessel and BS" },
    { 0x131, "Reviving Malak BS" },
    { 0x132, "Searching for Alma and BS" },
    { 0x133, "Things Obtained and BS" },
    { 0x134, "Reunion and Beyond and BS" },
    { 0x135, "Those Who Squirm In Darkness" },
    { 0x136, "Those Who Squirm In Darkness BS" },
    { 0x137, " A Man with the Holy Stone and BS" },
    { 0x138, "Delita's Thoughts" },
    { 0x139, "Delita's Thoughts BS" },
    { 0x13A, "Unstoppable Cog BS" },
    { 0x13B, "Seized TG Cid and BS" },
    { 0x13C, "Assassination of Prince Larg and BS" },
    { 0x13D, "Rescue of Cid and BS" },
    { 0x13E, "Entrance to the other world" },
    { 0x13F, "Prince Goltanas Final Moments BS" },
    { 0x140, "Ambition of Dycedarg and BS" },
    { 0x141, "Men of Odd Appearance BS" },
    { 0x142, "The Mystery of Lucavi BS" },
    { 0x143, "Delitas Betrayal and BS" },
    { 0x144, "Mosfungus and BS" },
    { 0x145, "At the Gate of the Beoulve Castle " },
    { 0x146, "Funerals Final Moments and BS" },
    { 0x147, "Requiem and BS" },
    { 0x148, "Zarghidas Aeris and BS" },
    { 0x149, "Bar Deep Dungeon" },
    { 0x14A, "Bar Goland Coal City" },
    //
    { 0x180, "Sweegy Woods" },
    { 0x181, "Dorter Trade City1" },
    { 0x182, "Sand Rat Cellar" },
    { 0x183, "Orbonne Battle" },
    { 0x184, "Gariland Fight" },
    { 0x185, "Mandalia Plains" },
    { 0x186, "Family Meeting" },
    { 0x187, "Interrogation" },
    { 0x188, "Military Academy" },
    { 0x189, "Introducing Algus" },
    { 0x18A, "Gustav vs Wiegraf" },
    { 0x18B, "Miluda1" },
    { 0x18C, "Releasing Miluda" },
    { 0x18D, "Attack on the Beoulves" },
    { 0x18E, "Expelling Algus and BS" },
    { 0x18F, "Miluda2" },
    { 0x190, "Wiegraf1" },
    { 0x191, "Fort Zeakden" },
    { 0x192, "DD END versus Elidibs" },
    { 0x193, "Dorter2" },
    { 0x194, "Araguay Woods" },
    { 0x195, "Zirekile Falls" },
    { 0x196, "Ovelia Joins" },
    { 0x197, "Zaland Fort City" },
    { 0x198, "Ramza Mustadio Agrias Ovelia Meeting" },
    { 0x199, "Bariaus Hill" },
    { 0x19A, "Zigolis Swamp" },
    { 0x19B, "Goug Machine City" },
    { 0x19C, "Besrodio Saved" },
    { 0x19D, "Bariaus Valley" },
    { 0x19E, "Golgorand Execution Site" },
    { 0x19F, "Lionel Castle Gate " },
    { 0x1A0, "Inside of Lionel Castle" },
    { 0x1A1, "Goland Coal City" },
    { 0x1A2, "Goland Coal City Post Battle" },
    { 0x1A3, "Zarghidas" },
    { 0x1A4, "Outside Lesalia Gate Zalmo 1" },
    { 0x1A5, "Outside Lesalia Gate Talk with Alma" },
    { 0x1A6, "Underground Book Storage Second Floor" },
    { 0x1A7, "Underground Book Storage Third Floor" },
    { 0x1A8, "Underground Book Storage First Floor" },
    { 0x1A9, "Meet Velius" },
    { 0x1AA, "Grog Hill " },
    { 0x1AB, "Meet Again with Olan" },
    { 0x1AC, "Rescue Rafa" },
    { 0x1AD, "Exploding Frog" },
    { 0x1AE, "Yuguo Woods" },
    { 0x1AF, "Riovanes Castle Entrance" },
    { 0x1B0, "Inside of Riovanes Castle" },
    { 0x1B1, "Rooftop of Riovanes Castle" },
    { 0x1B2, "Reviving Malak" },
    { 0x1B3, "Underground Book Storage Fourth Floor" },
    { 0x1B4, "Underground Book Storage Fifth Floor" },
    { 0x1B5, "Entrance to the other world" },
    { 0x1B6, "Murond Death City" },
    { 0x1B7, "Lost Sacred Precincts" },
    { 0x1B8, "Graveyard of Airships" },
    { 0x1B9, "Graveyard of Airships" },
    { 0x1BA, "Doguola Pass" },
    { 0x1BB, "Bervenia Free City" },
    { 0x1BC, "Finath River" },
    { 0x1BD, "Zalmo II" },
    { 0x1BE, "Unstoppable Cog" },
    { 0x1BF, "Balk I" },
    { 0x1C0, "South Wall of Bethla Garrison" },
    { 0x1C1, "North Wall of Bethla Garrison" },
    { 0x1C2, "Bethla Sluice" },
    { 0x1C3, "Prince Goltana's Final Moments" },
    { 0x1C4, "Germinas Peak" },
    { 0x1C5, "Poeskas Lake" },
    { 0x1C6, "Outside of Limberry Castle" },
    { 0x1C7, "Men of Odd Appearance" },
    { 0x1C8, "Elmdor II" },
    { 0x1C9, "Zalera" },
    { 0x1CA, "The Mystery of Lucavi" },
    { 0x1CB, "Adramelk" },
    { 0x1CC, "St Murond Temple" },
    { 0x1CD, "Hall of St Murond Temple" },
    { 0x1CE, "Chapel of St Murond Temple" },
    { 0x1CF, "Colliery Underground Third Floor" },
    { 0x1D0, "Colliery Underground Second Floor" },
    { 0x1D1, "Colliery Underground First Floor" },
    { 0x1D2, "Underground Passage in Goland" },
    { 0x1D3, "Underground Passage in Goland Post Battle" },
    { 0x1D4, "Nelveska Temple" },
    { 0x1D5, "Reis Curse" },
    //
    { 0x1DB, "Test" },
    { 0x1DC, "Test" },
    { 0x1DD, "Test" },
    { 0x1DE, "Test" },
    { 0x1DF, "Test" },
    { 0x1E0, "Test" },
    { 0x1E1, "Test" },
    { 0x1E2, "Test" },
    { 0x1E3, "Test" },
    { 0x1E4, "Test" },
    { 0x1E5, "Test" },
    { 0x1E6, "Test" },
    { 0x1E7, "Test" },
    { 0x1E8, "Test" },
    { 0x1E9, "Test" },
    { 0x1EA, "Test" },
    { 0x1EB, "Test" },
    { 0x1EC, "Test" },
    { 0x1ED, "Test" },
    { 0x1EE, "Test" },
    { 0x1EF, "Test" },
    { 0x1F0, "Test" },
    { 0x1F1, "Test" },
    { 0x1F2, "Test" },
    { 0x1F3, "Test" },
    { 0x1F4, "Test" },
    { 0x1F5, "Test" },
    { 0x1F6, "Test" },
    { 0x1F7, "Test" },
    { 0x1F8, "Test" },
    { 0x1F9, "Test" },
    { 0x1FA, "Test" },
    { 0x1FB, "Test" },
    { 0x1FC, "Test" },
    { 0x1FD, "Test" },
    { 0x1FE, "Test" },
    { 0x1FF, "Test" },
};

std::string resource_type_str(ResourceType value)
{
    switch (value) {
    case ResourceType::Texture:
        return "Texture";
    case ResourceType::MeshPrimary:
        return "MeshPrimary";
    case ResourceType::MeshOverride:
        return "MeshOverride";
    case ResourceType::MeshAlt:
        return "MeshAlt";
    case ResourceType::End:
        return "End";
    default:
        return "Unknown";
    }
}

std::string map_time_str(MapTime value)
{
    switch (value) {
    case MapTime::Day:
        return "Day";
    case MapTime::Night:
        return "Night";
    default:
        return "Unknown";
    }
}

std::string map_weather_str(MapWeather value)
{
    switch (value) {
    case MapWeather::None:
        return "None";
    case MapWeather::NoneAlt:
        return "NoneAlt";
    case MapWeather::Normal:
        return "Normal";
    case MapWeather::Strong:
        return "Strong";
    case MapWeather::VeryStrong:
        return "VeryStrong";
    default:
        return "Unknown";
    }
}

auto Record::sector() -> int { return data[8] | (data[9] << 8); }
auto Record::length() -> uint64_t { return static_cast<uint32_t>(data[12]) | (static_cast<uint32_t>(data[13]) << 8) | (static_cast<uint32_t>(data[14]) << 16) | (static_cast<uint32_t>(data[15]) << 24); }
auto Record::resource_type() -> ResourceType { return static_cast<ResourceType>(data[4] | (data[5] << 8)); }
auto Record::arrangement() -> int { return data[0]; }
auto Record::time() -> MapTime { return static_cast<MapTime>((data[3] >> 7) & 0x1); }
auto Record::weather() -> MapWeather { return static_cast<MapWeather>((data[3] >> 4) & 0x7); }

auto Event::id() -> int { return (data[0] & 0xFF) | ((data[1] & 0xFF) << 8) | ((data[2] & 0xFF) << 16) | ((data[3] & 0xFF) << 24); }

auto Scenario::repr() -> std::string
{
    std::ostringstream oss;
    oss << std::setw(3) << std::setfill('0') << id() << " " << map_list[map()].name;
    return oss.str();
}
auto Scenario::id() -> int { return data[0] | (data[1] << 8); }
auto Scenario::map() -> int { return data[2]; }
auto Scenario::weather() -> MapWeather { return static_cast<MapWeather>(data[3]); }
auto Scenario::time() -> MapTime { return static_cast<MapTime>(data[4]); }
auto Scenario::first_music() -> int { return data[5]; }
auto Scenario::second_music() -> int { return data[6]; }
auto Scenario::entd() -> int { return data[7] | (data[8] << 8); }
auto Scenario::first_grid() -> int { return data[9] | (data[10] << 8); }
auto Scenario::second_grid() -> int { return data[11] | (data[12] << 8); }
auto Scenario::require_ramza_unknown() -> int { return data[17]; }
auto Scenario::event_script() -> int { return data[22] | (data[23] << 8); }
