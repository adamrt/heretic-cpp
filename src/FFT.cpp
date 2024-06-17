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
    std::vector<Scenario> scenarios;
    for (int i = 0; i < 300; i++) {
        auto bytes = read_bytes(24);
        Scenario scenario { bytes };
        scenarios.push_back(scenario);
    }
    return scenarios;
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

    std::array<uint8_t, PALETTE_NUM_BYTES> pixels = {};

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
    std::array<uint8_t, TEXTURE_NUM_BYTES> pixels = {};

    for (int i = 0, j = 0; i < TEXTURE_RAW_SIZE; i++, j += 8) {
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
