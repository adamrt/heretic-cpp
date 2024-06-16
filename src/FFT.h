// This file contains ways to read BIN files.
#pragma once

#include <array>
#include <cstdint>
#include <cstdio>
#include <memory>
#include <string.h>
#include <string>
#include <utility>
#include <vector>

#include "Model.h"
#include "Texture.h"

#include "glm/glm.hpp"

#define GNS_MAX_SIZE 2388
#define RECORD_MAX_NUM 100

#define TEXTURE_WIDTH 256
#define TEXTURE_HEIGHT 1024
#define TEXTURE_NUM_PIXELS 262144      // 256 * 1024
#define TEXTURE_NUM_BYTES (262144 * 4) // 256 * 1024 * 4
#define TEXTURE_RAW_SIZE (TEXTURE_NUM_PIXELS / 2)

#define PALETTE_NUM_BYTES (16 * 16 * 4)

// Max size of any resource file.
#define FILE_MAX_SIZE 131072
#define SECTOR_SIZE 2048
#define SECTOR_SIZE_RAW 2352
#define SECTOR_HEADER_SIZE 24

enum class ResourceType : int {
    Texture = 0x1701,
    MeshPrimary = 0x2E01,
    MeshOverride = 0x2F01,
    MeshAlt = 0x3001,
    End = 0x3101,
};

enum class MapTime {
    Day = 0x0,
    Night = 0x1,
};

enum class MapWeather {
    None = 0x0,
    NoneAlt = 0x1,
    Normal = 0x2,
    Strong = 0x3,
    VeryStrong = 0x4,
};

// Record represents a GNS record.
struct Record {
    int sector;
    uint64_t len;
    ResourceType type;
    int arrangement;
    MapTime time;
    MapWeather weather;
};

std::string map_weather_str(MapWeather value);
std::string map_time_str(MapTime value);
std::string resource_type_str(ResourceType value);

class FFTMap {
public:
    std::vector<Vertex> vertices = {};

    std::vector<Record> records = {};

    std::shared_ptr<Texture> texture = {};
    std::shared_ptr<Texture> palette = {};

    std::vector<std::shared_ptr<Light>> lights = {};

    glm::vec3 ambient_light_color = {};
    glm::vec3 background_top = {};
    glm::vec3 background_bottom = {};
};

struct FFTMapDesc {
    uint8_t id;
    uint16_t sector;
    std::string name;
    bool valid;
};

extern const FFTMapDesc map_list[128];

// BinFile represents a file in a BIN file.
class BinFile {
public:
    auto read_u8()
        -> uint8_t;
    auto read_u16() -> uint16_t;
    auto read_u32() -> uint32_t;
    auto read_i8() -> int8_t;
    auto read_i16() -> int16_t;
    auto read_i32() -> int32_t;

    auto read_f1x3x12() -> float;
    auto read_position() -> glm::vec3;
    auto read_normal() -> glm::vec3;
    auto read_rgb15() -> glm::vec4;
    auto read_rgb8() -> glm::vec3;

    auto read_records() -> std::vector<Record>;
    auto read_vertices() -> std::vector<Vertex>;
    auto read_texture() -> std::shared_ptr<Texture>;
    auto read_palette() -> std::shared_ptr<Texture>;
    auto read_lights() -> std::vector<std::shared_ptr<Light>>;
    auto read_background() -> std::pair<glm::vec4, glm::vec4>;

    std::vector<uint8_t> data;
    uint64_t length;
    uint64_t offset;
};

class BinReader {
public:
    BinReader(std::string filename);
    ~BinReader();

    auto read_map(int mapnum) -> std::shared_ptr<FFTMap>;

private:
    FILE* file;
    auto read_file(uint32_t sector, uint32_t size) -> BinFile;
    auto read_sector(int32_t sector_num) -> std::array<uint8_t, SECTOR_SIZE>;
};
