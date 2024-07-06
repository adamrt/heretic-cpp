#pragma once

#include <array>
#include <cstdint>
#include <cstdio>
#include <glm/glm.hpp>
#include <memory>
#include <string>
#include <vector>

#include "BinFile.h"
#include "Event.h"
#include "Scenario.h"

// Binary file read for Final Fantasy Tactics PSX (Original and Greatest Hits)
//
// Serial: SCUS-94221
// md5sum: b156ba386436d20fd5ed8d37bab6b624
class BinReader {
public:
    BinReader(std::string filename);
    ~BinReader();

    auto read_map(int mapnum, MapTime time, MapWeather weather, int arrangement) -> std::shared_ptr<FFTMap>;
    auto read_scenarios() -> std::vector<Scenario>;
    auto read_events() -> std::vector<Event>;

private:
    FILE* file;
    auto read_sector(int32_t sector_num) -> std::array<uint8_t, SECTOR_SIZE>;
    auto read_file(uint32_t sector, uint32_t size) -> BinFile;

    // File types to read
    auto read_gns_file(uint32_t sector) -> GNSFile;
    auto read_texture_file(uint32_t sector) -> TextureFile;
    auto read_mesh_file(uint32_t sector, uint32_t size) -> MeshFile;

    // Specific Files on disk
    auto read_attack_out_file() -> AttackOutFile;
    auto read_event_file() -> EventFile;
};
