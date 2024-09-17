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
    auto requested_key = std::make_tuple(time, weather, arrangement);
    auto fallback_key = std::make_tuple(MapTime::Day, MapWeather::None, 0);

    std::shared_ptr<FFTMesh> primary_mesh = nullptr;
    std::map<std::tuple<MapTime, MapWeather, int>, std::shared_ptr<Texture>> textures;
    std::map<std::tuple<MapTime, MapWeather, int>, std::shared_ptr<FFTMesh>> alt_meshes;
    std::map<std::tuple<MapTime, MapWeather, int>, std::shared_ptr<FFTMesh>> override_meshes;

    auto gns_records = read_gns_file(map_list[map_num].sector).read_records();

    for (auto& record : gns_records) {
        auto record_key = std::make_tuple(record.time(), record.weather(), record.arrangement());

        switch (record.resource_type()) {
        case ResourceType::MeshPrimary: {
            // Primary Mesh is always Day/None
            auto resource = read_mesh_file(record.sector(), record.length());
            primary_mesh = resource.read_mesh();
            break;
        }
        case ResourceType::Texture: {
            // There can be duplicate textures for the same time/weather. Use the first one.
            if (textures.find(record_key) == textures.end()) {
                auto resource = read_texture_file(record.sector());
                textures[record_key] = resource.read_texture();
            }
            break;
        }
        case ResourceType::MeshAlt: {
            auto resource = read_mesh_file(record.sector(), record.length());
            alt_meshes[record_key] = resource.read_mesh();
            break;
        }
        case ResourceType::MeshOverride: {
            auto resource = read_mesh_file(record.sector(), record.length());
            override_meshes[record_key] = resource.read_mesh();
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
        texture = textures[fallback_key];
        assert(texture != nullptr);
    }

    auto override_mesh = override_meshes[requested_key];
    if (primary_mesh == nullptr) {
        primary_mesh = std::make_shared<FFTMesh>();
        if (override_mesh == nullptr) {
            override_mesh = override_meshes[fallback_key];
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

    auto alt_mesh = alt_meshes[requested_key];
    if (alt_mesh != nullptr) {
        if (alt_mesh->vertices.size() > 0) {
            primary_mesh->vertices.insert(primary_mesh->vertices.end(), alt_mesh->vertices.begin(), alt_mesh->vertices.end());
        }
        if (alt_mesh->lights.size() > 0) {
            primary_mesh->lights = alt_mesh->lights;
        }
        if (alt_mesh->palette != nullptr) {
            primary_mesh->palette = alt_mesh->palette;
        }
        // Defaults w (alpha) is 0.0f, so 1.0f means we read the ambient color and background.
        if (alt_mesh->ambient_color.w == 1.0f) {
            primary_mesh->ambient_color = alt_mesh->ambient_color;
        }
        if (alt_mesh->background.first.w == 1.0f) {
            primary_mesh->background = alt_mesh->background;
        }
    }

    auto map = std::make_shared<FFTMap>();
    map->gns_records = gns_records;
    map->texture = texture;
    map->mesh = primary_mesh;

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
