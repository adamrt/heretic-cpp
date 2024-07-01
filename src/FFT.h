// This file contains ways to read BIN files.
#pragma once

#include <array>
#include <cstdint>
#include <cstdio>
#include <map>
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

// Max size of any resource file.
#define FILE_MAX_SIZE 131072
constexpr size_t SECTOR_SIZE = 2048;
constexpr size_t SECTOR_SIZE_RAW = 2352;
constexpr size_t SECTOR_HEADER_SIZE = 24;

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
    std::vector<uint8_t> data;

    auto repr() -> std::string;
    auto sector() -> int;
    auto length() -> uint64_t;
    auto resource_type() -> ResourceType;
    auto arrangement() -> int;
    auto time() -> MapTime;
    auto weather() -> MapWeather;
};

struct Event {
    std::vector<uint8_t> data;
    auto id() -> int;
};

struct Scenario {
    std::vector<uint8_t> data;

    bool operator==(const Scenario& other) const
    {
        return id() == other.id();
    }

    auto repr() -> std::string;
    auto id() const -> int;
    auto map_id() -> int;
    auto weather() -> MapWeather;
    auto time() -> MapTime;
    auto first_music() -> int;
    auto second_music() -> int;
    auto entd_id() -> int;
    auto first_grid() -> int;
    auto second_grid() -> int;
    auto require_ramza_unknown() -> int;
    auto event_id() -> int;
    // There is more from the "last line". Is it the next 24 bytes or what?
};

std::string map_weather_str(MapWeather value);
std::string map_time_str(MapTime value);
std::string resource_type_str(ResourceType value);

struct FFTMesh {
    std::vector<Vertex> vertices;
    std::shared_ptr<Texture> palette = nullptr;
    std::vector<std::shared_ptr<Light>> lights;
    glm::vec4 background_top = {};
    glm::vec4 background_bottom = {};
    // Add ambient_light_color
};

class FFTMap {
public:
    std::vector<Record> gns_records = {};
    std::shared_ptr<Texture> texture = {};
    std::shared_ptr<FFTMesh> mesh = {};
};

struct FFTMapDesc {
    uint8_t id;
    uint16_t sector;
    std::string name;
    bool valid;

    auto repr() const -> std::string;
};

struct Instruction {
    std::string name;
    std::string description;
    std::string usage;
};

extern std::array<FFTMapDesc, 128> map_list;
extern std::map<int, std::string> scenario_list;
extern std::map<int, Instruction> instruction_list;

// BinFile represents a file in a BIN file.
class BinFile {
public:
    auto read_bytes(int num) -> std::vector<uint8_t>;

    auto read_u8() -> uint8_t;
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

    auto read_scenarios() -> std::vector<Scenario>;
    auto read_event() -> Event;
    auto read_records() -> std::vector<Record>;
    auto read_mesh() -> std::shared_ptr<FFTMesh>;
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

    auto read_map(int mapnum, MapTime time, MapWeather weather) -> std::shared_ptr<FFTMap>;
    auto read_scenarios() -> std::vector<Scenario>;
    auto read_events() -> std::vector<Event>;

private:
    FILE* file;
    auto read_file(uint32_t sector, uint32_t size) -> BinFile;
    auto read_sector(int32_t sector_num) -> std::array<uint8_t, SECTOR_SIZE>;
};
