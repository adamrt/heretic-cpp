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
    oss << std::setw(3) << std::setfill('0') << std::to_string(id) << " " << name;
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

bool Record::operator<(const Record& other) const
{
    auto t = time();
    auto w = weather();
    auto ot = other.time();
    auto ow = other.weather();
    return std::tie(t, w) < std::tie(ot, ow);
}

// Equality operator for unique
bool Record::operator==(const Record& other) const
{
    auto t = time();
    auto w = weather();
    auto ot = other.time();
    auto ow = other.weather();
    return std::tie(t, w) == std::tie(ot, ow);
}

std::string to_string(ResourceType value)
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

std::string to_string(MapTime value)
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

std::string to_string(MapWeather value)
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

std::array<FFTMapDesc, 128> map_list = {
    FFTMapDesc { 0, 10026, "???", false },
    FFTMapDesc { 1, 11304, "At Main Gate of Igros Castle", true },
    FFTMapDesc { 2, 12656, "Back Gate of Lesalia Castle", true },
    FFTMapDesc { 3, 12938, "Hall of St. Murond Temple", true },
    FFTMapDesc { 4, 13570, "Office of Lesalia Castle", true },
    FFTMapDesc { 5, 14239, "Roof of Riovanes Castle", true },
    FFTMapDesc { 6, 14751, "At the Gate of Riovanes Castle", true },
    FFTMapDesc { 7, 15030, "Inside of Riovanes Castle", true },
    FFTMapDesc { 8, 15595, "Riovanes Castle", true },
    FFTMapDesc { 9, 16262, "Citadel of Igros Castle", true },
    FFTMapDesc { 10, 16347, "Inside of Igros Castle", true },
    FFTMapDesc { 11, 16852, "Office of Igros Castle", true },
    FFTMapDesc { 12, 17343, "At the Gate of Lionel Castle", true },
    FFTMapDesc { 13, 17627, "Inside of Lionel Castle", true },
    FFTMapDesc { 14, 18175, "Office of Lionel Castle", true },
    FFTMapDesc { 15, 19510, "At the Gate of Limberry Castle (1)", true },
    FFTMapDesc { 16, 20075, "Inside of Limberry Castle", true },
    FFTMapDesc { 17, 20162, "Underground Cemetary of Limberry Castle", true },
    FFTMapDesc { 18, 20745, "Office of Limberry Castle", true },
    FFTMapDesc { 19, 21411, "At the Gate of Limberry Castle (2)", true },
    FFTMapDesc { 20, 21692, "Inside of Zeltennia Castle", true },
    FFTMapDesc { 21, 22270, "Zeltennia Castle", true },
    FFTMapDesc { 22, 22938, "Magic City Gariland", true },
    FFTMapDesc { 23, 23282, "Belouve Residence", true },
    FFTMapDesc { 24, 23557, "Military Academy's Auditorium", true },
    FFTMapDesc { 25, 23899, "Yardow Fort City", true },
    FFTMapDesc { 26, 23988, "Weapon Storage of Yardow", true },
    FFTMapDesc { 27, 24266, "Goland Coal City", true },
    FFTMapDesc { 28, 24544, "Colliery Underground First Floor", true },
    FFTMapDesc { 29, 24822, "Colliery Underground Second Floor", true },
    FFTMapDesc { 30, 25099, "Colliery Underground Third Floor", true },
    FFTMapDesc { 31, 25764, "Dorter Trade City", true },
    FFTMapDesc { 32, 26042, "Slums in Dorter", true },
    FFTMapDesc { 33, 26229, "Hospital in Slums", true },
    FFTMapDesc { 34, 26362, "Cellar of Sand Mouse", true },
    FFTMapDesc { 35, 27028, "Zaland Fort City", true },
    FFTMapDesc { 36, 27643, "Church Outside of Town", true },
    FFTMapDesc { 37, 27793, "Ruins Outside Zaland", true },
    FFTMapDesc { 38, 28467, "Goug Machine City", true },
    FFTMapDesc { 39, 28555, "Underground Passage in Goland", true },
    FFTMapDesc { 40, 29165, "Slums in Goug", true },
    FFTMapDesc { 41, 29311, "Besrodio's House", true },
    FFTMapDesc { 42, 29653, "Warjilis Trade City", true },
    FFTMapDesc { 43, 29807, "Port of Warjilis", true },
    FFTMapDesc { 44, 30473, "Bervenia Free City", true },
    FFTMapDesc { 45, 30622, "Ruins of Zeltennia Castle's Church", true },
    FFTMapDesc { 46, 30966, "Cemetary of Heavenly Knight, Balbanes", true },
    FFTMapDesc { 47, 31697, "Zarghidas Trade City", true },
    FFTMapDesc { 48, 32365, "Slums of Zarghidas", true },
    FFTMapDesc { 49, 33032, "Fort Zeakden", true },
    FFTMapDesc { 50, 33701, "St. Murond Temple", true },
    FFTMapDesc { 51, 34349, "St. Murond Temple", true },
    FFTMapDesc { 52, 34440, "Chapel of St. Murond Temple", true },
    // MAP053 doesn't have expected primary mesh pointer
    FFTMapDesc { 53, 34566, "Entrance to Death City", true },
    FFTMapDesc { 54, 34647, "Lost Sacred Precincts", true },
    FFTMapDesc { 55, 34745, "Graveyard of Airships", true },
    FFTMapDesc { 56, 35350, "Orbonne Monastery", true },
    FFTMapDesc { 57, 35436, "Underground Book Storage First Floor", true },
    FFTMapDesc { 58, 35519, "Underground Book Storage Second Floor", true },
    FFTMapDesc { 59, 35603, "Underground Book Storage Third Floor", true },
    FFTMapDesc { 60, 35683, "Underground Book Storge Fourth Floor", true },
    FFTMapDesc { 61, 35765, "Underground Book Storage Fifth Floor", true },
    FFTMapDesc { 62, 36052, "Chapel of Orbonne Monastery", true },
    FFTMapDesc { 63, 36394, "Golgorand Execution Site", true },
    FFTMapDesc { 64, 36530, "In Front of Bethla Garrison's Sluice", true },
    FFTMapDesc { 65, 36612, "Granary of Bethla Garrison", true },
    FFTMapDesc { 66, 37214, "South Wall of Bethla Garrison", true },
    FFTMapDesc { 67, 37817, "Noth Wall of Bethla Garrison", true },
    FFTMapDesc { 68, 38386, "Bethla Garrison", true },
    FFTMapDesc { 69, 38473, "Murond Death City", true },
    FFTMapDesc { 70, 38622, "Nelveska Temple", true },
    FFTMapDesc { 71, 39288, "Dolbodar Swamp", true },
    FFTMapDesc { 72, 39826, "Fovoham Plains", true },
    FFTMapDesc { 73, 40120, "Inside of Windmill Shed", true },
    FFTMapDesc { 74, 40724, "Sweegy Woods", true },
    FFTMapDesc { 75, 41391, "Bervenia Volcano", true },
    FFTMapDesc { 76, 41865, "Zeklaus Desert", true },
    FFTMapDesc { 77, 42532, "Lenalia Plateau", true },
    FFTMapDesc { 78, 43200, "Zigolis Swamp", true },
    FFTMapDesc { 79, 43295, "Yuguo Woods", true },
    FFTMapDesc { 80, 43901, "Araguay Woods", true },
    FFTMapDesc { 81, 44569, "Grog Hill", true },
    FFTMapDesc { 82, 45044, "Bed Desert", true },
    FFTMapDesc { 83, 45164, "Zirekile Falls", true },
    FFTMapDesc { 84, 45829, "Bariaus Hill", true },
    FFTMapDesc { 85, 46498, "Mandalia Plains", true },
    FFTMapDesc { 86, 47167, "Doguola Pass", true },
    FFTMapDesc { 87, 47260, "Bariaus Valley", true },
    FFTMapDesc { 88, 47928, "Finath River", true },
    FFTMapDesc { 89, 48595, "Poeskas Lake", true },
    FFTMapDesc { 90, 49260, "Germinas Peak", true },
    FFTMapDesc { 91, 49538, "Thieves Fort", true },
    FFTMapDesc { 92, 50108, "Igros-Belouve Residence", true },
    FFTMapDesc { 93, 50387, "Broke Down Shed-Wooden Building", true },
    FFTMapDesc { 94, 50554, "Broke Down Shed-Stone Building", true },
    FFTMapDesc { 95, 51120, "Church", true },
    FFTMapDesc { 96, 51416, "Pub", true },
    FFTMapDesc { 97, 52082, "Inside Castle Gate in Lesalia", true },
    FFTMapDesc { 98, 52749, "Outside Castle Gate in Lesalia", true },
    FFTMapDesc { 99, 53414, "Main Street of Lesalia", true },
    FFTMapDesc { 100, 53502, "Public Cemetary", true },
    FFTMapDesc { 101, 53579, "Tutorial (1)", true },
    FFTMapDesc { 102, 53659, "Tutorial (2)", true },
    FFTMapDesc { 103, 54273, "Windmill Shed", true },
    FFTMapDesc { 104, 54359, "Belouve Residence", true },
    FFTMapDesc { 105, 54528, "TERMINATE", true },
    FFTMapDesc { 106, 54621, "DELTA", true },
    FFTMapDesc { 107, 54716, "NOGIAS", true },
    FFTMapDesc { 108, 54812, "VOYAGE", true },
    FFTMapDesc { 109, 54909, "BRIDGE", true },
    FFTMapDesc { 110, 55004, "VALKYRIES", true },
    FFTMapDesc { 111, 55097, "MLAPAN", true },
    FFTMapDesc { 112, 55192, "TIGER", true },
    FFTMapDesc { 113, 55286, "HORROR", true },
    FFTMapDesc { 114, 55383, "END", true },
    FFTMapDesc { 115, 56051, "Banished Fort", true },
    FFTMapDesc { 116, 56123, "Arena", true },
    FFTMapDesc { 117, 56201, "???", true },
    FFTMapDesc { 118, 56279, "???", true },
    FFTMapDesc { 119, 56356, "???", true },
    FFTMapDesc { 120, 0, "???", false },
    FFTMapDesc { 121, 0, "???", false },
    FFTMapDesc { 122, 0, "???", false },
    FFTMapDesc { 123, 0, "???", false },
    FFTMapDesc { 124, 0, "???", false },
    FFTMapDesc { 125, 56435, "???", true },
    FFTMapDesc { 126, 0, "???", false },
    FFTMapDesc { 127, 0, "???", false },
};
