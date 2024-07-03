// This file contains ways to read BIN files.
#pragma once

#include <algorithm>
#include <array>
#include <cstdint>
#include <cstdio>
#include <map>
#include <memory>
#include <string.h>
#include <string>
#include <utility>
#include <variant>
#include <vector>

#include "Event.h"
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
    auto time() const -> MapTime;
    auto weather() const -> MapWeather;

    bool operator<(const Record& other) const;
    bool operator==(const Record& other) const;
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
    auto next_event_id() -> int;
    // There is more from the "last line". Is it the next 24 bytes or what?
};

std::string map_weather_str(MapWeather value);
std::string map_time_str(MapTime value);
std::string resource_type_str(ResourceType value);

struct FFTMesh {
    std::vector<Vertex> vertices;
    std::shared_ptr<Texture> palette = nullptr;
    std::vector<std::shared_ptr<Light>> lights;
    glm::vec4 ambient_color = {};
    std::pair<glm::vec4, glm::vec4> background = {};
};

class FFTMap {
public:
    std::vector<Record> gns_records = {};
    std::shared_ptr<Texture> texture = nullptr;
    std::shared_ptr<FFTMesh> mesh = nullptr;
};

struct FFTMapDesc {
    uint8_t id;
    uint16_t sector;
    std::string name;
    bool valid;

    auto repr() const -> std::string;
};

extern std::array<FFTMapDesc, 128> map_list;
extern std::map<int, std::string> scenario_list;
