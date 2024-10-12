#include <memory>
#include <optional>
#include <utility>

#include "BinReader.h"
#include "Event.h"
#include "ResourceManager.h"
#include "Scenario.h"

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

auto BinReader::read_map(int map_num, MapTime time, MapWeather weather, int arrangement) -> std::shared_ptr<FFTMap>
{
    std::shared_ptr<FFTMesh> final_mesh = nullptr;
    std::shared_ptr<FFTMesh> primary_mesh = nullptr;
    std::shared_ptr<FFTMesh> alt_mesh = nullptr;
    std::shared_ptr<FFTMesh> override_mesh = nullptr;
    std::shared_ptr<Texture> texture = nullptr;
    std::shared_ptr<Texture> fallback_texture = nullptr;

    auto gns_records = read_gns_file(map_list[map_num].sector).read_records();

    for (auto& record : gns_records) {
        bool is_match = record.time() == time && record.weather() == weather && record.arrangement() == arrangement;
        bool is_default = record.time() == MapTime::Day && record.weather() == MapWeather::None && record.arrangement() == 0;

        // MeshPrimary and MeshOverride are special cases that use are always set to TimeDay, WeatherNone, Arrangement 0.
        switch (record.resource_type()) {

        case ResourceType::MeshPrimary:
            primary_mesh = read_mesh_file(record.sector(), record.length()).read_mesh();
            break;

        case ResourceType::MeshOverride:
            override_mesh = read_mesh_file(record.sector(), record.length()).read_mesh();
            break;

        case ResourceType::Texture:
            // Maps 51 and 105 have duplicate textures for the same conditions
            // but they are the same image data. Some maps don't have a texture
            // for the conditions so we need to fallback to the default.
            if (is_match) {
                texture = read_texture_file(record.sector()).read_texture();
            } else if (is_default) {
                fallback_texture = read_texture_file(record.sector()).read_texture();
            }
            break;

        case ResourceType::MeshAlt:
            if (is_match) {
                alt_mesh = read_mesh_file(record.sector(), record.length()).read_mesh();
            }
            break;

        default:
            std::cout << "Unknown resource type: " << std::endl;
        }
    }

    // A few maps have no primary mesh, so we need to create one.
    // 2, 8 ,15 ,16 ,18 ,33 ,34 ,41 ,55 ,68 ,92 ,94 ,95 ,96, 104
    final_mesh = primary_mesh != nullptr ? primary_mesh : std::make_shared<FFTMesh>();

    if (override_mesh != nullptr) {
        merge_meshes(final_mesh, override_mesh);
    }

    if (alt_mesh != nullptr) {
        merge_meshes(final_mesh, alt_mesh);
    }

    texture = texture != nullptr ? texture : fallback_texture;

    auto map = std::make_shared<FFTMap>();
    map->mesh = final_mesh;
    map->texture = texture;
    map->gns_records = gns_records;

    return map;
}

auto BinReader::read_scenarios() -> std::vector<Scenario>
{
    auto attack_out = read_attack_out_file();
    auto event_file = read_event_file();
    auto events = event_file.read_events();

    std::vector<Scenario> valid_scenarios;
    for (auto& scenario : attack_out.read_scenarios()) {
        // We only care about scenarios that have a valid event.
        auto event = events[scenario.id()];
        if (event.should_skip()) {
            continue;
        }
        valid_scenarios.push_back(scenario);
    }
    return valid_scenarios;
}

auto BinReader::read_events() -> std::vector<Event>
{
    auto event_file = read_event_file();
    return event_file.read_events();
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
    std::vector<uint8_t> data;
    for (uint32_t i = 0; i < occupied_sectors; i++) {
        auto sector_data = read_sector(sector_num + i);
        data.insert(data.end(), sector_data.begin(), sector_data.end());
    }

    BinFile out_file(data);
    return out_file;
}

auto BinReader::read_texture_file(uint32_t sector_num) -> TextureFile
{
    return TextureFile { read_file(sector_num, FFT_TEXTURE_RAW_SIZE) };
}

auto BinReader::read_mesh_file(uint32_t sector_num, uint32_t size) -> MeshFile
{
    return MeshFile { read_file(sector_num, size) };
}

auto BinReader::read_gns_file(uint32_t sector_num) -> GNSFile
{
    return GNSFile { read_file(sector_num, GNS_MAX_SIZE) };
}

auto BinReader::read_attack_out_file() -> AttackOutFile
{
    constexpr int attack_out_sector = 2448;
    constexpr int attack_out_size = 125956;

    return AttackOutFile { read_file(attack_out_sector, attack_out_size) };
}

auto BinReader::read_event_file() -> EventFile
{
    constexpr int event_file_sector = 3707;
    constexpr int event_file_size = 4096000;

    return EventFile { read_file(event_file_sector, event_file_size) };
}

auto merge_meshes(std::shared_ptr<FFTMesh> destination, std::shared_ptr<FFTMesh> source) -> void
{
    if (!source->vertices.empty()) {
        destination->vertices.insert(destination->vertices.end(), source->vertices.begin(), source->vertices.end());
    }

    if (!source->lights.empty()) {
        destination->lights = source->lights;
    }

    if (source->palette != nullptr) {
        destination->palette = source->palette;
    }

    if (source->ambient_color.r + source->ambient_color.g + source->ambient_color.b + source->ambient_color.a > 0.0f) {
        destination->ambient_color = source->ambient_color;
    }

    bool top_has_color = source->background.first.r + source->background.first.g + source->background.first.b + source->background.first.a > 0.0f;
    bool bottom_has_color = source->background.second.r + source->background.second.g + source->background.second.b + source->background.second.a > 0.0f;
    if (top_has_color || bottom_has_color) {
        destination->background = source->background;
    }
}
