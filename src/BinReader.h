#pragma once

#include <array>
#include <cstdint>
#include <cstdio>
#include <glm/glm.hpp>
#include <memory>
#include <string>
#include <vector>

#include "FFT.h"

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
    auto read_rgb8() -> glm::vec4;

    auto read_scenarios() -> std::vector<Scenario>;
    auto read_event() -> Event;
    auto read_records() -> std::vector<Record>;
    auto read_mesh() -> std::shared_ptr<FFTMesh>;
    auto read_vertices() -> std::vector<Vertex>;
    auto read_texture() -> std::shared_ptr<Texture>;
    auto read_palette() -> std::shared_ptr<Texture>;
    auto read_lights() -> std::tuple<std::vector<std::shared_ptr<Light>>, glm::vec4, std::pair<glm::vec4, glm::vec4>>;
    auto read_background() -> std::pair<glm::vec4, glm::vec4>;

    std::vector<uint8_t> data;
    uint64_t length;
    uint64_t offset;
};

class BinReader {
public:
    BinReader(std::string filename);
    ~BinReader();

    auto read_map(int mapnum, MapTime time, MapWeather weather, int arrangement) -> std::shared_ptr<FFTMap>;
    auto read_scenarios() -> std::vector<Scenario>;
    auto read_events() -> std::vector<Event>;

private:
    FILE* file;
    auto read_file(uint32_t sector, uint32_t size) -> BinFile;
    auto read_sector(int32_t sector_num) -> std::array<uint8_t, SECTOR_SIZE>;
};

glm::vec2 process_tex_coords(float u, float v, uint8_t page);
