#include <iomanip>
#include <sstream>

#include "FFT.h"
#include "Scenario.h"

bool Scenario::operator==(const Scenario& other) const
{
    return id() == other.id();
}

auto Scenario::repr() const -> std::string
{
    std::ostringstream oss;
    oss << std::setw(3) << std::setfill('0') << id() << " " << map_list[map_id()].name;
    return oss.str();
}

auto Scenario::id() const -> int
{
    return data[0] | (data[1] << 8);
}

auto Scenario::map_id() const -> int
{
    return data[2];
}

auto Scenario::weather() const -> MapWeather
{
    return static_cast<MapWeather>(data[3]);
}

auto Scenario::time() const -> MapTime
{
    return static_cast<MapTime>(data[4]);
}

auto Scenario::first_music() const -> int
{
    return data[5];
}

auto Scenario::second_music() const -> int
{
    return data[6];
}

auto Scenario::entd_id() const -> int
{
    return data[7] | (data[8] << 8);
}

auto Scenario::first_grid() const -> int
{
    return data[9] | (data[10] << 8);
}

auto Scenario::second_grid() const -> int
{
    return data[11] | (data[12] << 8);
}

auto Scenario::require_ramza_unknown() const -> int
{
    return data[17];
}

auto Scenario::next_scenario() const -> int
{
    return data[18] | (data[19] << 8);
}

auto Scenario::next_step() const -> int
{
    return data[20];
}

auto Scenario::event_id() const -> int
{
    assert(false); // See header file documentation.

    return (data[22] | (data[23] << 8));
}
