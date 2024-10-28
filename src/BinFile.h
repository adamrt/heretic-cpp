#pragma once

#include <cstdint>
#include <memory>
#include <vector>

#include "Event.h"
#include "Scenario.h"
#include "Texture.h"

// GLOBAL_SCALE is used to scale the vertex data. This might be used later to
// scale camera movement and other positional related instruction data.
//
// We could use the original i16 data and pass those to OpenGL and let it
// normalize, but OpenGL will do it based on the full range of i16, not the max
// value of our vertex data. So we just pic a number here that is reasonable and
// use it everywhere.
constexpr float GLOBAL_SCALE = 256.0f;

// DEGREE_PER_UNIT is used to convert the angles in the BIN files to degrees.
// ie, the angle and rotation values of a Camera Instruction.
constexpr float DEGREE_PER_UNIT = 45.0f / 512.0f; // 0.087890625

// BinFile represents an individual file in the FFT BIN.
class BinFile {
public:
    BinFile(std::vector<uint8_t> data)
        : m_data(data) {};

    auto read_u8() -> uint8_t;
    auto read_u16() -> uint16_t;
    auto read_u32() -> uint32_t;
    auto read_i8() -> int8_t;
    auto read_i16() -> int16_t;
    auto read_i32() -> int32_t;

    auto read_bytes(int num) -> std::vector<uint8_t>;

protected:
    std::vector<uint8_t> m_data;
    uint64_t m_offset = 0;

private:
    template <typename T>
    auto read() -> T;
};

class GNSFile : public BinFile {
public:
    auto read_records() -> std::vector<Record>;
};

class TextureFile : public BinFile {
public:
    auto read_texture() -> std::shared_ptr<Texture>;
};

class MeshFile : public BinFile {
public:
    auto read_mesh() -> std::shared_ptr<FFTMesh>;

private:
    auto read_vertices() -> std::vector<Vertex>;
    auto read_palette() -> std::shared_ptr<Texture>;
    auto read_lights() -> std::tuple<std::vector<std::shared_ptr<Light>>, glm::vec4, std::pair<glm::vec4, glm::vec4>>;
    auto read_background() -> std::pair<glm::vec4, glm::vec4>;

    auto read_position() -> glm::vec3;
    auto read_normal() -> glm::vec3;
    auto read_light_color() -> float;
    auto read_f1x3x12() -> float;
    auto read_rgb8() -> glm::vec4;
    auto read_rgb15() -> glm::vec4;
};

class EventFile : public BinFile {
public:
    auto read_events() -> std::vector<Event>;
};

class AttackOutFile : public BinFile {
public:
    auto read_scenarios() -> std::vector<Scenario>;
};
