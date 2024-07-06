#include <math.h>

#include <algorithm>
#include <array>
#include <iomanip>
#include <optional>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <utility>

#include "ResourceManager.h"

#include "FFT.h"
#include "Texture.h"

auto FFTMapDesc::repr() const -> std::string
{
    std::ostringstream oss;
    auto is_valid = valid ? "" : "[INVALID]";
    oss << is_valid << std::setw(3) << std::setfill('0') << std::to_string(id) << " " << name;
    return oss.str();
}

auto Record::repr() -> std::string
{
    std::ostringstream oss;
    oss << to_string(time()) << " " << to_string(weather()) << " " << arrangement();
    return oss.str();
}

auto Record::sector() -> int
{
    return data[8] | (data[9] << 8);
}

auto Record::length() -> uint64_t
{
    return static_cast<uint32_t>(data[12]) | (static_cast<uint32_t>(data[13]) << 8) | (static_cast<uint32_t>(data[14]) << 16) | (static_cast<uint32_t>(data[15]) << 24);
}

auto Record::resource_type() -> ResourceType
{
    return static_cast<ResourceType>(data[4] | (data[5] << 8));
}

auto Record::arrangement() -> int
{
    return data[1];
}

auto Record::time() const -> MapTime
{
    return static_cast<MapTime>((data[3] >> 7) & 0x1);
}

auto Record::weather() const -> MapWeather
{
    return static_cast<MapWeather>((data[3] >> 4) & 0x7);
}

auto Record::operator<(const Record& other) const -> bool
{
    auto t = time();
    auto w = weather();
    auto ot = other.time();
    auto ow = other.weather();
    return std::tie(t, w) < std::tie(ot, ow);
}

// Equality operator for unique
auto Record::operator==(const Record& other) const -> bool
{
    auto t = time();
    auto w = weather();
    auto ot = other.time();
    auto ow = other.weather();
    return std::tie(t, w) == std::tie(ot, ow);
}

auto to_string(ResourceType value) -> std::string
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

auto to_string(MapTime value) -> std::string
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

auto to_string(MapWeather value) -> std::string
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

std::map<int, FFTMapDesc> map_list = {
    { 0, { 0, 10026, "Unknown", false } }, // No texture
    { 1, { 1, 11304, "At Main Gate of Igros Castle", true } },
    { 2, { 2, 12656, "Back Gate of Lesalia Castle", true } },
    { 3, { 3, 12938, "Hall of St. Murond Temple", true } },
    { 4, { 4, 13570, "Office of Lesalia Castle", true } },
    { 5, { 5, 14239, "Roof of Riovanes Castle", true } },
    { 6, { 6, 14751, "At the Gate of Riovanes Castle", true } },
    { 7, { 7, 15030, "Inside of Riovanes Castle", true } },
    { 8, { 8, 15595, "Riovanes Castle", true } },
    { 9, { 9, 16262, "Citadel of Igros Castle", true } },
    { 10, { 10, 16347, "Inside of Igros Castle", true } },
    { 11, { 11, 16852, "Office of Igros Castle", true } },
    { 12, { 12, 17343, "At the Gate of Lionel Castle", true } },
    { 13, { 13, 17627, "Inside of Lionel Castle", true } },
    { 14, { 14, 18175, "Office of Lionel Castle", true } },
    { 15, { 15, 19510, "At the Gate of Limberry Castle (1)", true } },
    { 16, { 16, 20075, "Inside of Limberry Castle", true } },
    { 17, { 17, 20162, "Underground Cemetary of Limberry Castle", true } },
    { 18, { 18, 20745, "Office of Limberry Castle", true } },
    { 19, { 19, 21411, "At the Gate of Limberry Castle (2)", true } },
    { 20, { 20, 21692, "Inside of Zeltennia Castle", true } },
    { 21, { 21, 22270, "Zeltennia Castle", true } },
    { 22, { 22, 22938, "Magic City Gariland", true } },
    { 23, { 23, 23282, "Belouve Residence", true } },
    { 24, { 24, 23557, "Military Academy's Auditorium", true } },
    { 25, { 25, 23899, "Yardow Fort City", true } },
    { 26, { 26, 23988, "Weapon Storage of Yardow", true } },
    { 27, { 27, 24266, "Goland Coal City", true } },
    { 28, { 28, 24544, "Colliery Underground First Floor", true } },
    { 29, { 29, 24822, "Colliery Underground Second Floor", true } },
    { 30, { 30, 25099, "Colliery Underground Third Floor", true } },
    { 31, { 31, 25764, "Dorter Trade City", true } },
    { 32, { 32, 26042, "Slums in Dorter", true } },
    { 33, { 33, 26229, "Hospital in Slums", true } },
    { 34, { 34, 26362, "Cellar of Sand Mouse", true } },
    { 35, { 35, 27028, "Zaland Fort City", true } },
    { 36, { 36, 27643, "Church Outside of Town", true } },
    { 37, { 37, 27793, "Ruins Outside Zaland", true } },
    { 38, { 38, 28467, "Goug Machine City", true } },
    { 39, { 39, 28555, "Underground Passage in Goland", true } },
    { 40, { 40, 29165, "Slums in Goug", true } },
    { 41, { 41, 29311, "Besrodio's House", true } },
    { 42, { 42, 29653, "Warjilis Trade City", true } },
    { 43, { 43, 29807, "Port of Warjilis", true } },
    { 44, { 44, 30473, "Bervenia Free City", true } },
    { 45, { 45, 30622, "Ruins of Zeltennia Castle's Church", true } },
    { 46, { 46, 30966, "Cemetary of Heavenly Knight, Balbanes", true } },
    { 47, { 47, 31697, "Zarghidas Trade City", true } },
    { 48, { 48, 32365, "Slums of Zarghidas", true } },
    { 49, { 49, 33032, "Fort Zeakden", true } },
    { 50, { 50, 33701, "St. Murond Temple", true } },
    { 51, { 51, 34349, "St. Murond Temple", true } },
    { 52, { 52, 34440, "Chapel of St. Murond Temple", true } },
    { 53, { 53, 34566, "Entrance to Death City", true } },
    { 54, { 54, 34647, "Lost Sacred Precincts", true } },
    { 55, { 55, 34745, "Graveyard of Airships", true } },
    { 56, { 56, 35350, "Orbonne Monastery", true } },
    { 57, { 57, 35436, "Underground Book Storage First Floor", true } },
    { 58, { 58, 35519, "Underground Book Storage Second Floor", true } },
    { 59, { 59, 35603, "Underground Book Storage Third Floor", true } },
    { 60, { 60, 35683, "Underground Book Storge Fourth Floor", true } },
    { 61, { 61, 35765, "Underground Book Storage Fifth Floor", true } },
    { 62, { 62, 36052, "Chapel of Orbonne Monastery", true } },
    { 63, { 63, 36394, "Golgorand Execution Site", true } },
    { 64, { 64, 36530, "In Front of Bethla Garrison's Sluice", true } },
    { 65, { 65, 36612, "Granary of Bethla Garrison", true } },
    { 66, { 66, 37214, "South Wall of Bethla Garrison", true } },
    { 67, { 67, 37817, "Noth Wall of Bethla Garrison", true } },
    { 68, { 68, 38386, "Bethla Garrison", true } },
    { 69, { 69, 38473, "Murond Death City", true } },
    { 70, { 70, 38622, "Nelveska Temple", true } },
    { 71, { 71, 39288, "Dolbodar Swamp", true } },
    { 72, { 72, 39826, "Fovoham Plains", true } },
    { 73, { 73, 40120, "Inside of Windmill Shed", true } },
    { 74, { 74, 40724, "Sweegy Woods", true } },
    { 75, { 75, 41391, "Bervenia Volcano", true } },
    { 76, { 76, 41865, "Zeklaus Desert", true } },
    { 77, { 77, 42532, "Lenalia Plateau", true } },
    { 78, { 78, 43200, "Zigolis Swamp", true } },
    { 79, { 79, 43295, "Yuguo Woods", true } },
    { 80, { 80, 43901, "Araguay Woods", true } },
    { 81, { 81, 44569, "Grog Hill", true } },
    { 82, { 82, 45044, "Bed Desert", true } },
    { 83, { 83, 45164, "Zirekile Falls", true } },
    { 84, { 84, 45829, "Bariaus Hill", true } },
    { 85, { 85, 46498, "Mandalia Plains", true } },
    { 86, { 86, 47167, "Doguola Pass", true } },
    { 87, { 87, 47260, "Bariaus Valley", true } },
    { 88, { 88, 47928, "Finath River", true } },
    { 89, { 89, 48595, "Poeskas Lake", true } },
    { 90, { 90, 49260, "Germinas Peak", true } },
    { 91, { 91, 49538, "Thieves Fort", true } },
    { 92, { 92, 50108, "Igros-Belouve Residence", true } },
    { 93, { 93, 50387, "Broke Down Shed-Wooden Building", true } },
    { 94, { 94, 50554, "Broke Down Shed-Stone Building", true } },
    { 95, { 95, 51120, "Church", true } },
    { 96, { 96, 51416, "Pub", true } },
    { 97, { 97, 52082, "Inside Castle Gate in Lesalia", true } },
    { 98, { 98, 52749, "Outside Castle Gate in Lesalia", true } },
    { 99, { 99, 53414, "Main Street of Lesalia", true } },
    { 100, { 100, 53502, "Public Cemetary", true } },
    { 101, { 101, 53579, "Tutorial (1)", true } },
    { 102, { 102, 53659, "Tutorial (2)", true } },
    { 103, { 103, 54273, "Windmill Shed", true } },
    { 104, { 104, 54359, "Belouve Residence", true } },
    { 105, { 105, 54528, "TERMINATE", true } },
    { 106, { 106, 54621, "DELTA", true } },
    { 107, { 107, 54716, "NOGIAS", true } },
    { 108, { 108, 54812, "VOYAGE", true } },
    { 109, { 109, 54909, "BRIDGE", true } },
    { 110, { 110, 55004, "VALKYRIES", true } },
    { 111, { 111, 55097, "MLAPAN", true } },
    { 112, { 112, 55192, "TIGER", true } },
    { 113, { 113, 55286, "HORROR", true } },
    { 114, { 114, 55383, "END", true } },
    { 115, { 115, 56051, "Banished Fort", true } },
    { 116, { 116, 56123, "Arena", true } },
    { 117, { 117, 56201, "Unknown", true } },
    { 118, { 118, 56279, "Unknown", true } },
    { 119, { 119, 56356, "Unknown", true } },
    { 120, { 120, 0, "???", false } },
    { 121, { 121, 0, "???", false } },
    { 122, { 122, 0, "???", false } },
    { 123, { 123, 0, "???", false } },
    { 124, { 124, 0, "???", false } },
    { 125, { 125, 56435, "Unknown", true } },
    { 126, { 126, 0, "???", false } },
    { 127, { 127, 0, "???", false } },
};
