#include <math.h>

#include <algorithm>
#include <array>
#include <assert.h>
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

std::map<int, Command> command_list = {
    { 0x00, { "", {} } },
    { 0x01, { "Unknown Command!", {} } },
    { 0x10, { "DisplayMessage", { 1, 1, 2, 1, 1, 1, 2, 2, 2, 1 } } },
    { 0x11, { "UnitAnim", { 1, 1, 1, 1, 1 } } },
    { 0x12, { "Chapter 3 Start BS", { 2 } } },
    { 0x13, { "ChangeMapBeta", { 1, 1 } } },
    { 0x16, { "Pause", {} } },
    { 0x18, { "Effect", { 2, 1, 1, 1, 1 } } },
    { 0x19, { "Camera", { 2, 2, 2, 2, 2, 2, 2, 2 } } },
    { 0x1A, { "MapDarkness", { 1, 1, 1, 1, 1 } } },
    { 0x1B, { "MapLight", { 2, 2, 2, 2, 2, 2, 2 } } },
    { 0x1C, { "EventSpeed", { 1 } } },
    { 0x1D, { "CameraFusionStart", {} } },
    { 0x1E, { "CameraFusionEnd", {} } },
    { 0x1F, { "Focus", { 1, 1, 1, 1, 1 } } },
    { 0x21, { "SoundEffect", { 2 } } },
    { 0x22, { "SwitchTrack", { 1, 1, 1 } } },
    { 0x27, { "ReloadMapState", {} } },
    { 0x28, { "WalkTo", { 1, 1, 1, 1, 1, 1, 1, 1 } } },
    { 0x29, { "WaitWalk", { 1, 1 } } },
    { 0x2A, { "BlockStart", {} } },
    { 0x2B, { "BlockEnd", {} } },
    { 0x2C, { "FaceUnit2", { 1, 1, 1, 1, 1, 1, 1 } } },
    { 0x2D, { "RotateUnit", { 1, 1, 1, 1, 1, 1 } } },
    { 0x2E, { "Background", { 1, 1, 1, 1, 1, 1, 1, 1 } } },
    { 0x31, { "ColorBGBeta", { 1, 1, 1, 1, 1 } } },
    { 0x32, { "ColorUnit", { 1, 1, 1, 1, 1, 1, 1 } } },
    { 0x33, { "ColorField", { 1, 1, 1, 1, 1 } } },
    { 0x38, { "FocusSpeed", { 2 } } },
    { 0x3B, { "SpriteMove", { 1, 1, 2, 2, 2, 1, 1, 2 } } },
    { 0x3C, { "Weather", { 1, 1 } } },
    { 0x3D, { "RemoveUnit", { 1, 1 } } },
    { 0x3E, { "ColorScreen", { 1, 1, 1, 1, 1, 1, 1, 2 } } },
    { 0x41, { "EarthquakeStart", { 1, 1, 1, 1 } } },
    { 0x42, { "EarthquakeEnd", {} } },
    { 0x43, { "CallFunction", { 1 } } },
    { 0x44, { "Draw", { 1, 1 } } },
    { 0x45, { "AddUnit", { 1, 1, 1 } } },
    { 0x46, { "Erase", { 1, 1 } } },
    { 0x47, { "AddGhostUnit", { 1, 1, 1, 1, 1, 1, 1, 1 } } },
    { 0x48, { "WaitAddUnit", {} } },
    { 0x49, { "AddUnitStart", {} } },
    { 0x4A, { "AddUnitEnd", {} } },
    { 0x4B, { "WaitAddUnitEnd", {} } },
    { 0x4C, { "ChangeMap", { 1, 1 } } },
    { 0x4D, { "Reveal", { 1 } } },
    { 0x4E, { "UnitShadow", { 1, 1, 1 } } },
    { 0x50, { "PortraitCol", { 1 } } },
    { 0x51, { "ChangeDialog", { 1, 2, 1, 1 } } },
    { 0x53, { "FaceUnit", { 1, 1, 1, 1, 1, 1, 1 } } },
    { 0x54, { "Use3DObject", { 1, 1 } } },
    { 0x55, { "UseFieldObject", { 1, 1 } } },
    { 0x56, { "Wait3DObject", {} } },
    { 0x57, { "WaitFieldObject", {} } },
    { 0x58, { "LoadEVTCHR", { 1, 1, 1 } } },
    { 0x59, { "SaveEVTCHR", { 1 } } },
    { 0x5A, { "SaveEVTCHRClear", { 1 } } },
    { 0x5B, { "LoadEVTCHRClear", { 1 } } },
    { 0x5F, { "WarpUnit", { 1, 1, 1, 1, 1, 1 } } },
    { 0x60, { "FadeSound", { 1, 1 } } },
    { 0x63, { "CameraSpeedCurve", { 1 } } },
    { 0x64, { "WaitRotateUnit", { 1, 1 } } },
    { 0x65, { "WaitRotateAll", {} } },
    { 0x68, { "MirrorSprite", { 1, 1, 1 } } },
    { 0x69, { "FaceTile", { 1, 1, 1, 1, 1, 1, 1, 1 } } },
    { 0x6A, { "EditBGSound", { 1, 1, 1, 1, 1 } } },
    { 0x6B, { "BGSound", { 1, 1, 1, 1, 1 } } },
    { 0x6E, { "SpriteMoveBeta", { 1, 1, 2, 2, 2, 1, 1, 2 } } },
    { 0x6F, { "WaitSpriteMove", { 1, 1 } } },
    { 0x70, { "Jump", { 1, 1, 1, 1 } } },
    { 0x76, { "DarkScreen", { 1, 1, 1, 1, 1, 1 } } },
    { 0x77, { "RemoveDarkScreen", {} } },
    { 0x78, { "DisplayConditions", { 1, 1 } } },
    { 0x79, { "WalkToAnim", { 1, 1, 2 } } },
    { 0x7A, { "DismissUnit", { 1, 1 } } },
    { 0x7D, { "ShowGraphic", { 1 } } },
    { 0x7E, { "WaitValue", { 2, 2 } } },
    { 0x7F, { "EVTCHRPalette", { 1, 1, 1, 1 } } },
    { 0x80, { "March", { 1, 1, 1 } } },
    { 0x83, { "ChangeStats", { 1, 1, 1, 2 } } },
    { 0x84, { "PlayTune", { 1 } } },
    { 0x85, { "UnlockDate", { 1 } } },
    { 0x86, { "TempWeapon", { 1, 1, 1 } } },
    { 0x87, { "Arrow", { 1, 1, 1, 1 } } },
    { 0x88, { "MapUnfreeze", {} } },
    { 0x89, { "MapFreeze", {} } },
    { 0x8A, { "EffectStart", {} } },
    { 0x8B, { "EffectEnd", {} } },
    { 0x8C, { "UnitAnimRotate", { 1, 1, 1, 1, 1, 1 } } },
    { 0x8E, { "WaitGraphicPrint", {} } },
    { 0x91, { "ShowMapTitle", { 1, 1, 1 } } },
    { 0x92, { "InflictStatus", { 1, 1, 1, 1, 1 } } },
    { 0x94, { "TeleportOut", { 1, 1 } } },
    { 0x96, { "AppendMapState ", {} } },
    { 0x97, { "ResetPalette", { 1, 1 } } },
    { 0x98, { "TeleportIn", { 1, 1 } } },
    { 0x99, { "BlueRemoveUnit", { 1, 1 } } },
    { 0xA0, { "LTE", {} } },
    { 0xA1, { "GTE", {} } },
    { 0xA2, { "EQ", {} } },
    { 0xA3, { "NEQ", {} } },
    { 0xA4, { "LT", {} } },
    { 0xA5, { "GT", {} } },
    { 0xB0, { "ADD", { 2, 2 } } },
    { 0xB1, { "ADDVar", { 2, 2 } } },
    { 0xB2, { "SUB", { 2, 2 } } },
    { 0xB3, { "SUBVar", { 2, 2 } } },
    { 0xB4, { "MULT", { 2, 2 } } },
    { 0xB5, { "MULTVar", { 2, 2 } } },
    { 0xB6, { "DIV", { 2, 2 } } },
    { 0xB7, { "DIVVar", { 2, 2 } } },
    { 0xB8, { "MOD", { 2, 2 } } },
    { 0xB9, { "MODVar", { 2, 2 } } },
    { 0xBA, { "AND", { 2, 2 } } },
    { 0xBB, { "ANDVar", { 2, 2 } } },
    { 0xBC, { "OR", { 2, 2 } } },
    { 0xBD, { "ORVar", { 2, 2 } } },
    { 0xBE, { "ZERO", { 2 } } },
    { 0xD0, { "JumpForwardIfZero", { 1 } } },
    { 0xD1, { "JumpForward ", { 1 } } },
    { 0xD2, { "ForwardTarget ", { 1 } } },
    { 0xD3, { "JumpBack ", { 1 } } },
    { 0xD5, { "BackTarget ", { 1 } } },
    { 0xDB, { "EventEnd", {} } },
    { 0xE3, { "EventEnd2", {} } },
    { 0xE5, { "WaitForInstruction", { 1, 1 } } },
    { 0xF1, { "Wait", { 2 } } },
    { 0xF2, { "Pad", {} } },
};

// FIXME: Use these scenario names instead of just the map name
std::map<int, std::string> scenario_list = {
    { 0x001, "Dolbodar Swamp East 1" },
    { 0x002, "Dolbodar Swamp East 2" },
    { 0x003, "Dolbodar Swamp East 3" },
    { 0x004, "Dolbodar Swamp East 4" },
    { 0x005, "Dolbodar Swamp West 1" },
    { 0x006, "Dolbodar Swamp West 2" },
    { 0x007, "Dolbodar Swamp West 3" },
    { 0x008, "Dolbodar Swamp West 4" },
    { 0x009, "Tutorial (Charge Time Battle)" },
    { 0x00A, "Tutorial (How to Cast Spells)" },
    { 0x00B, "Tutorial (Move and Act)" },
    { 0x00C, "Tutorial (Online Help)" },
    { 0x00D, "Fovoham Plains South 1" },
    { 0x00E, "Fovoham Plains South 2" },
    { 0x00F, "Fovoham Plains South 3" },
    { 0x010, "Fovoham Plains South 4" },
    { 0x011, "Fovoham Plains West 1" },
    { 0x012, "Fovoham Plains West 2" },
    { 0x013, "Fovoham Plains West 3" },
    { 0x014, "Fovoham Plains West 4" },
    { 0x015, "Fovoham Plains East 1" },
    { 0x016, "Fovoham Plains East 2" },
    { 0x017, "Fovoham Plains East 3" },
    { 0x018, "Fovoham Plains East 4" },
    { 0x019, "Sweegy Woods East 1" },
    { 0x01A, "Sweegy Woods East 2" },
    { 0x01B, "Sweegy Woods East 3" },
    { 0x01C, "Sweegy Woods East 4" },
    { 0x01D, "Sweegy Woods West 1" },
    { 0x01E, "Sweegy Woods West 2" },
    { 0x01F, "Sweegy Woods West 3" },
    { 0x020, "Sweegy Woods West 4" },
    { 0x021, "Tutorial (Battle)" },
    { 0x025, "Bevernia Volcano North 1" },
    { 0x026, "Bevernia Volcano North 2" },
    { 0x027, "Bevernia Volcano North 3" },
    { 0x028, "Bevernia Volcano North 4" },
    { 0x029, "Bevernia Volcano South 1" },
    { 0x02A, "Bevernia Volcano South 2" },
    { 0x02B, "Bevernia Volcano South 3" },
    { 0x02C, "Bevernia Volcano South 4" },
    { 0x02D, "Tutorial (Battlefield Control)" },
    { 0x02E, "Tutorial (Abnormal Status)" },
    { 0x02F, "Tutorial (Options)" },
    { 0x031, "Zeklaus Desert North 1" },
    { 0x032, "Zeklaus Desert North 2" },
    { 0x033, "Zeklaus Desert North 3" },
    { 0x034, "Zeklaus Desert North 4" },
    { 0x035, "Zeklaus Desert South 1" },
    { 0x036, "Zeklaus Desert South 2" },
    { 0x037, "Zeklaus Desert South 3" },
    { 0x038, "Zeklaus Desert South 4" },
    { 0x039, "Zeklaus Desert East 1" },
    { 0x03A, "Zeklaus Desert East 2" },
    { 0x03B, "Zeklaus Desert East 3" },
    { 0x03C, "Zeklaus Desert East 4" },
    { 0x03D, "Lenalia Plateau South 1" },
    { 0x03E, "Lenalia Plateau South 2" },
    { 0x03F, "Lenalia Plateau South 3" },
    { 0x040, "Lenalia Plateau South 4" },
    { 0x041, "Lenalia Plateau North 1" },
    { 0x042, "Lenalia Plateau North 2" },
    { 0x043, "Lenalia Plateau North 3" },
    { 0x044, "Lenalia Plateau North 4" },
    //
    { 0x049, "Zigolis Swamp East 1" },
    { 0x04A, "Zigolis Swamp East 2" },
    { 0x04B, "Zigolis Swamp East 3" },
    { 0x04C, "Zigolis Swamp East 4" },
    { 0x04D, "Zigolis Swamp West 1" },
    { 0x04E, "Zigolis Swamp West 2" },
    { 0x04F, "Zigolis Swamp West 3" },
    { 0x050, "Zigolis Swamp West 4" },
    //
    { 0x052, "Zirekile Falls East 5" },
    { 0x053, "Barius Hill South 5" },
    { 0x054, "Lenalia Plateau South 5" },
    { 0x055, "Yuguo Woods West 1" },
    { 0x056, "Yuguo Woods West 2" },
    { 0x057, "Yuguo Woods West 3" },
    { 0x058, "Yuguo Woods West 4" },
    { 0x059, "Yuguo Woods East 1" },
    { 0x05A, "Yuguo Woods East 2" },
    { 0x05B, "Yuguo Woods East 3" },
    { 0x05C, "Yuguo Woods East 4" },
    { 0x05D, "Dolbodar Swamp West 5" },
    { 0x05E, "Grog Hill South 5" },
    { 0x05F, "Bevernia Volcano North 5" },
    { 0x060, "Barius Valley South 5" },
    { 0x061, "Araguay Woods West 1" },
    { 0x062, "Araguay Woods West 2" },
    { 0x063, "Araguay Woods West 3" },
    { 0x064, "Araguay Woods West 4" },
    { 0x065, "Araguay Woods East 1" },
    { 0x066, "Araguay Woods East 2" },
    { 0x067, "Araguay Woods East 3" },
    { 0x068, "Araguay Woods East 4" },
    { 0x069, "Finath River East 5" },
    { 0x06A, "Germina Peak North 5" },
    { 0x06B, "Araguay Woods South 5" },
    { 0x06C, "Yuguo Woods East 5" },
    { 0x06D, "Grog Hill West 1" },
    { 0x06E, "Grog Hill West 2" },
    { 0x06F, "Grog Hill West 3" },
    { 0x070, "Grog Hill West 4" },
    { 0x071, "Grog Hill South 1" },
    { 0x072, "Grog Hill South 2" },
    { 0x073, "Grog Hill South 3" },
    { 0x074, "Grog Hill South 4" },
    { 0x075, "Grog Hill East 1" },
    { 0x076, "Grog Hill East 2" },
    { 0x077, "Grog Hill East 3" },
    { 0x078, "Grog Hill East 4" },
    { 0x079, "Bed Desert South 1" },
    { 0x07A, "Bed Desert South 2" },
    { 0x07B, "Bed Desert South 3" },
    { 0x07C, "Bed Desert South 4" },
    { 0x07D, "Bed Desert North 1" },
    { 0x07E, "Bed Desert North 2" },
    { 0x07F, "Bed Desert North 3" },
    { 0x080, "Bed Desert North 4" },
    { 0x081, "Bed Desert North 5" },
    { 0x082, "Fovoham Plains West 5" },
    { 0x083, "Doguola Pass West 5" },
    { 0x084, "Sweegy Woods East 5" },
    { 0x085, "Zirekile Falls West 1" },
    { 0x086, "Zirekile Falls West 2" },
    { 0x087, "Zirekile Falls West 3" },
    { 0x088, "Zirekile Falls West 4" },
    { 0x089, "Zirekile Falls East 1" },
    { 0x08A, "Zirekile Falls East 2" },
    { 0x08B, "Zirekile Falls East 3" },
    { 0x08C, "Zirekile Falls East 4" },
    { 0x08D, "Zirekile Falls South 1" },
    { 0x08E, "Zirekile Falls South 2" },
    { 0x08F, "Zirekile Falls South 3" },
    { 0x090, "Zirekile Falls South 4" },
    { 0x091, "Barius Hill North 1" },
    { 0x092, "Barius Hill North 2" },
    { 0x093, "Barius Hill North 3" },
    { 0x094, "Barius Hill North 4" },
    { 0x095, "Barius Hill South 1" },
    { 0x096, "Barius Hill South 2" },
    { 0x097, "Barius Hill South 3" },
    { 0x098, "Barius Hill South 4" },
    { 0x099, "Poeskas Lake North 5" },
    { 0x09A, "Zigolis Swamp West 5" },
    { 0x09B, "Mandalia Plains South 5" },
    { 0x09C, "Zeklaus Desert South 5" },
    { 0x09D, "Mandalia Plains North 1" },
    { 0x09E, "Mandalia Plains North 2" },
    { 0x09F, "Mandalia Plains North 3" },
    { 0x0A0, "Mandalia Plains North 4" },
    { 0x0A1, "Mandalia Plains South 1" },
    { 0x0A2, "Mandalia Plains South 2" },
    { 0x0A3, "Mandalia Plains South 3" },
    { 0x0A4, "Mandalia Plains South 4" },
    { 0x0A5, "Mandalia Plains West 1" },
    { 0x0A6, "Mandalia Plains West 2" },
    { 0x0A7, "Mandalia Plains West 3" },
    { 0x0A8, "Mandalia Plains West 4" },
    { 0x0A9, "Doguola Pass East 1" },
    { 0x0AA, "Doguola Pass East 2" },
    { 0x0AB, "Doguola Pass East 3" },
    { 0x0AC, "Doguola Pass East 4" },
    { 0x0AD, "Doguola Pass West 1" },
    { 0x0AE, "Doguola Pass West 2" },
    { 0x0AF, "Doguola Pass West 3" },
    { 0x0B0, "Doguola Pass West 4" },
    { 0x0B1, "End 1" },
    { 0x0B2, "End 2" },
    { 0x0B3, "End 3" },
    { 0x0B4, "End 4" },
    { 0x0B5, "Barius Valley West 1" },
    { 0x0B6, "Barius Valley West 2" },
    { 0x0B7, "Barius Valley West 3" },
    { 0x0B8, "Barius Valley West 4" },
    { 0x0B9, "Barius Valley East 1" },
    { 0x0BA, "Barius Valley East 2" },
    { 0x0BB, "Barius Valley East 3" },
    { 0x0BC, "Barius Valley East 4" },
    { 0x0BD, "Barius Valley South 1" },
    { 0x0BE, "Barius Valley South 2" },
    { 0x0BF, "Barius Valley South 3" },
    { 0x0C0, "Barius Valley South 4" },
    { 0x0C1, "Finath River West 1" },
    { 0x0C2, "Finath River West 2" },
    { 0x0C3, "Finath River West 3" },
    { 0x0C4, "Finath River West 4" },
    { 0x0C5, "Finath River East 1" },
    { 0x0C6, "Finath River East 2" },
    { 0x0C7, "Finath River East 3" },
    { 0x0C8, "Finath River East 4" },
    { 0x0C9, "Horror" },
    { 0x0CA, "Horror" },
    { 0x0CB, "Horror" },
    { 0x0CC, "Horror" },
    { 0x0CD, "Poeskas Lake North 1" },
    { 0x0CE, "Poeskas Lake North 2" },
    { 0x0CF, "Poeskas Lake North 3" },
    { 0x0D0, "Poeskas Lake North 4" },
    { 0x0D1, "Poeskas Lake South 1" },
    { 0x0D2, "Poeskas Lake South 2" },
    { 0x0D3, "Poeskas Lake South 3" },
    { 0x0D4, "Poeskas Lake South 4" },
    { 0x0D5, "Voyage 1" },
    { 0x0D6, "Voyage 2" },
    { 0x0D7, "Voyage 3" },
    { 0x0D8, "Voyage 4" },
    { 0x0D9, "Germina Peak North 1" },
    { 0x0DA, "Germina Peak North 2" },
    { 0x0DB, "Germina Peak North 3" },
    { 0x0DC, "Germina Peak North 4" },
    { 0x0DD, "Germina Peak South 1" },
    { 0x0DE, "Germina Peak South 2" },
    { 0x0DF, "Germina Peak South 3" },
    { 0x0E0, "Germina Peak South 4" },
    { 0x0E1, "Bridge 1" },
    { 0x0E2, "Bridge 2" },
    { 0x0E3, "Bridge 3" },
    { 0x0E4, "Bridge 4" },
    { 0x0E5, "Tiger 1" },
    { 0x0E6, "Tiger 2" },
    { 0x0E7, "Tiger 3" },
    { 0x0E8, "Tiger 4" },
    { 0x0E9, "Mlapan 1" },
    { 0x0EA, "Mlapan 2" },
    { 0x0EB, "Mlapan 3" },
    { 0x0EC, "Mlapan 4" },
    { 0x0ED, "Valkyries 1" },
    { 0x0EE, "Valkyries 2" },
    { 0x0EF, "Valkyries 3" },
    { 0x0F0, "Valkyries 4" },
    { 0x0F1, "Delta 1" },
    { 0x0F2, "Delta 2" },
    { 0x0F3, "Delta 3" },
    { 0x0F4, "Delta 4" },
    { 0x0F5, "Terminate 1" },
    { 0x0F6, "Terminate 2" },
    { 0x0F7, "Terminate 3" },
    { 0x0F8, "Terminate 4" },
    { 0x0F9, "Nogias 1" },
    { 0x0FA, "Nogias 2" },
    { 0x0FB, "Nogias 3" },
    { 0x0FC, "Nogias 4" },
    //
    { 0x100, "Orbonne Prayer and BS" },
    { 0x101, "Larg's Praise and BS" },
    { 0x102, "Military Academy BS" },
    { 0x103, "Family Meeting BS" },
    { 0x104, "Balbanes Death and BS" },
    { 0x105, "Releasing Miluda BS" },
    { 0x106, "Introducing Algus BS" },
    { 0x107, "Returning to Igros and BS" },
    { 0x108, "Attack on the Beoulves BS" },
    { 0x109, "Interrogation BS" },
    { 0x10A, "Gustav vs Wiegraf " },
    { 0x10B, "Bedridden Dycedarg and BS" },
    { 0x10C, "Reed Whistle" },
    { 0x10D, "Wiegraf berating Golagros" },
    { 0x10E, "Finding Teta Missing" },
    { 0x10F, "Partings" },
    { 0x110, "Ch2 Start Orbonne Monastery" },
    { 0x111, "Ovelia Joins BS" },
    { 0x112, " Ramza Mustadio Agrias Ovelia Meeting BS" },
    { 0x113, "Ruins of Zaland and BS" },
    { 0x114, "Dycedarg and Gafgarion Reunion and BS" },
    { 0x115, "Besrodio Kidnapped BS" },
    { 0x116, "Gate of Lionel Castle and BS" },
    { 0x117, " Meet Draclau and BS" },
    { 0x118, "Goug Machine City Town" },
    { 0x119, "Besrodio Saved BS" },
    { 0x11A, "Warjilis Port and BS" },
    { 0x11B, "Draclau hires Gafgarion and BS" },
    { 0x11C, "Substitute and BS" },
    { 0x11D, "Gelwan's Death and BS" },
    { 0x11E, "Ch2 Start Orbonne Monastery BS" },
    { 0x11F, "Chapter 3 Start" },
    { 0x120, "Chapter 3 Start BS" },
    { 0x121, "Goland Coal City Post Battle" },
    { 0x122, "Steel Ball Found and BS" },
    { 0x123, "Worker 8 Activated and BS" },
    { 0x124, "Summoning Machine Found and BS" },
    { 0x125, "Cloud Summoned and BS" },
    { 0x126, "Talk with Zalbag in Lesalia and BS" },
    { 0x127, "Lesalia Gate Talk with Alma BS" },
    { 0x128, "Orbonne Monastery (Ch3) and BS" },
    { 0x129, "Meet Velius BS" },
    { 0x12A, "Malak and the Scriptures" },
    { 0x12B, "Delitas allegiance to Ovelia and BS" },
    { 0x12C, "Meet Again with Olan BS" },
    { 0x12D, "Exploding Frog BS" },
    { 0x12E, "Barinten threatens Vormav and BS" },
    { 0x12F, "Escaping Alma and BS" },
    { 0x130, "Ajora's vessel and BS" },
    { 0x131, "Reviving Malak BS" },
    { 0x132, "Searching for Alma and BS" },
    { 0x133, "Things Obtained and BS" },
    { 0x134, "Reunion and Beyond and BS" },
    { 0x135, "Those Who Squirm In Darkness" },
    { 0x136, "Those Who Squirm In Darkness BS" },
    { 0x137, " A Man with the Holy Stone and BS" },
    { 0x138, "Delita's Thoughts" },
    { 0x139, "Delita's Thoughts BS" },
    { 0x13A, "Unstoppable Cog BS" },
    { 0x13B, "Seized TG Cid and BS" },
    { 0x13C, "Assassination of Prince Larg and BS" },
    { 0x13D, "Rescue of Cid and BS" },
    { 0x13E, "Entrance to the other world" },
    { 0x13F, "Prince Goltanas Final Moments BS" },
    { 0x140, "Ambition of Dycedarg and BS" },
    { 0x141, "Men of Odd Appearance BS" },
    { 0x142, "The Mystery of Lucavi BS" },
    { 0x143, "Delitas Betrayal and BS" },
    { 0x144, "Mosfungus and BS" },
    { 0x145, "At the Gate of the Beoulve Castle " },
    { 0x146, "Funerals Final Moments and BS" },
    { 0x147, "Requiem and BS" },
    { 0x148, "Zarghidas Aeris and BS" },
    { 0x149, "Bar Deep Dungeon" },
    { 0x14A, "Bar Goland Coal City" },
    //
    { 0x180, "Sweegy Woods" },
    { 0x181, "Dorter Trade City1" },
    { 0x182, "Sand Rat Cellar" },
    { 0x183, "Orbonne Battle" },
    { 0x184, "Gariland Fight" },
    { 0x185, "Mandalia Plains" },
    { 0x186, "Family Meeting" },
    { 0x187, "Interrogation" },
    { 0x188, "Military Academy" },
    { 0x189, "Introducing Algus" },
    { 0x18A, "Gustav vs Wiegraf" },
    { 0x18B, "Miluda1" },
    { 0x18C, "Releasing Miluda" },
    { 0x18D, "Attack on the Beoulves" },
    { 0x18E, "Expelling Algus and BS" },
    { 0x18F, "Miluda2" },
    { 0x190, "Wiegraf1" },
    { 0x191, "Fort Zeakden" },
    { 0x192, "DD END versus Elidibs" },
    { 0x193, "Dorter2" },
    { 0x194, "Araguay Woods" },
    { 0x195, "Zirekile Falls" },
    { 0x196, "Ovelia Joins" },
    { 0x197, "Zaland Fort City" },
    { 0x198, "Ramza Mustadio Agrias Ovelia Meeting" },
    { 0x199, "Bariaus Hill" },
    { 0x19A, "Zigolis Swamp" },
    { 0x19B, "Goug Machine City" },
    { 0x19C, "Besrodio Saved" },
    { 0x19D, "Bariaus Valley" },
    { 0x19E, "Golgorand Execution Site" },
    { 0x19F, "Lionel Castle Gate " },
    { 0x1A0, "Inside of Lionel Castle" },
    { 0x1A1, "Goland Coal City" },
    { 0x1A2, "Goland Coal City Post Battle" },
    { 0x1A3, "Zarghidas" },
    { 0x1A4, "Outside Lesalia Gate Zalmo 1" },
    { 0x1A5, "Outside Lesalia Gate Talk with Alma" },
    { 0x1A6, "Underground Book Storage Second Floor" },
    { 0x1A7, "Underground Book Storage Third Floor" },
    { 0x1A8, "Underground Book Storage First Floor" },
    { 0x1A9, "Meet Velius" },
    { 0x1AA, "Grog Hill " },
    { 0x1AB, "Meet Again with Olan" },
    { 0x1AC, "Rescue Rafa" },
    { 0x1AD, "Exploding Frog" },
    { 0x1AE, "Yuguo Woods" },
    { 0x1AF, "Riovanes Castle Entrance" },
    { 0x1B0, "Inside of Riovanes Castle" },
    { 0x1B1, "Rooftop of Riovanes Castle" },
    { 0x1B2, "Reviving Malak" },
    { 0x1B3, "Underground Book Storage Fourth Floor" },
    { 0x1B4, "Underground Book Storage Fifth Floor" },
    { 0x1B5, "Entrance to the other world" },
    { 0x1B6, "Murond Death City" },
    { 0x1B7, "Lost Sacred Precincts" },
    { 0x1B8, "Graveyard of Airships" },
    { 0x1B9, "Graveyard of Airships" },
    { 0x1BA, "Doguola Pass" },
    { 0x1BB, "Bervenia Free City" },
    { 0x1BC, "Finath River" },
    { 0x1BD, "Zalmo II" },
    { 0x1BE, "Unstoppable Cog" },
    { 0x1BF, "Balk I" },
    { 0x1C0, "South Wall of Bethla Garrison" },
    { 0x1C1, "North Wall of Bethla Garrison" },
    { 0x1C2, "Bethla Sluice" },
    { 0x1C3, "Prince Goltana's Final Moments" },
    { 0x1C4, "Germinas Peak" },
    { 0x1C5, "Poeskas Lake" },
    { 0x1C6, "Outside of Limberry Castle" },
    { 0x1C7, "Men of Odd Appearance" },
    { 0x1C8, "Elmdor II" },
    { 0x1C9, "Zalera" },
    { 0x1CA, "The Mystery of Lucavi" },
    { 0x1CB, "Adramelk" },
    { 0x1CC, "St Murond Temple" },
    { 0x1CD, "Hall of St Murond Temple" },
    { 0x1CE, "Chapel of St Murond Temple" },
    { 0x1CF, "Colliery Underground Third Floor" },
    { 0x1D0, "Colliery Underground Second Floor" },
    { 0x1D1, "Colliery Underground First Floor" },
    { 0x1D2, "Underground Passage in Goland" },
    { 0x1D3, "Underground Passage in Goland Post Battle" },
    { 0x1D4, "Nelveska Temple" },
    { 0x1D5, "Reis Curse" },
    //
    { 0x1DB, "Test" },
    { 0x1DC, "Test" },
    { 0x1DD, "Test" },
    { 0x1DE, "Test" },
    { 0x1DF, "Test" },
    { 0x1E0, "Test" },
    { 0x1E1, "Test" },
    { 0x1E2, "Test" },
    { 0x1E3, "Test" },
    { 0x1E4, "Test" },
    { 0x1E5, "Test" },
    { 0x1E6, "Test" },
    { 0x1E7, "Test" },
    { 0x1E8, "Test" },
    { 0x1E9, "Test" },
    { 0x1EA, "Test" },
    { 0x1EB, "Test" },
    { 0x1EC, "Test" },
    { 0x1ED, "Test" },
    { 0x1EE, "Test" },
    { 0x1EF, "Test" },
    { 0x1F0, "Test" },
    { 0x1F1, "Test" },
    { 0x1F2, "Test" },
    { 0x1F3, "Test" },
    { 0x1F4, "Test" },
    { 0x1F5, "Test" },
    { 0x1F6, "Test" },
    { 0x1F7, "Test" },
    { 0x1F8, "Test" },
    { 0x1F9, "Test" },
    { 0x1FA, "Test" },
    { 0x1FB, "Test" },
    { 0x1FC, "Test" },
    { 0x1FD, "Test" },
    { 0x1FE, "Test" },
    { 0x1FF, "Test" },
};

std::string resource_type_str(ResourceType value)
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

std::string map_time_str(MapTime value)
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

std::string map_weather_str(MapWeather value)
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

auto FFTMapDesc::repr() const -> std::string
{
    std::ostringstream oss;
    oss << std::setw(3) << std::setfill('0') << std::to_string(id) << " " << name;
    return oss.str();
}

auto Record::sector() -> int { return data[8] | (data[9] << 8); }
auto Record::length() -> uint64_t { return static_cast<uint32_t>(data[12]) | (static_cast<uint32_t>(data[13]) << 8) | (static_cast<uint32_t>(data[14]) << 16) | (static_cast<uint32_t>(data[15]) << 24); }
auto Record::resource_type() -> ResourceType { return static_cast<ResourceType>(data[4] | (data[5] << 8)); }
auto Record::arrangement() -> int { return data[1]; }
auto Record::time() const -> MapTime { return static_cast<MapTime>((data[3] >> 7) & 0x1); }
auto Record::weather() const -> MapWeather { return static_cast<MapWeather>((data[3] >> 4) & 0x7); }
auto Record::repr() -> std::string
{
    std::ostringstream oss;
    oss << map_time_str(time()) << " " << map_weather_str(weather()) << " " << arrangement();
    return oss.str();
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

auto Event::id() -> int { return (data[0] & 0xFF) | ((data[1] & 0xFF) << 8) | ((data[2] & 0xFF) << 16) | ((data[3] & 0xFF) << 24); }
auto Event::should_skip() -> bool { return id() == 0xf2f2f2f2; }
auto Event::text_offset() -> uint32_t
{
    assert(!should_skip());
    return id();
}

auto Event::next_command() -> Instruction
{
    auto bytecode = data[offset];
    offset++;

    if (command_list.find(bytecode) == command_list.end()) {
        Instruction instruction = {};
        instruction.command = 0x01;
        instruction.params.push_back(bytecode);
        return instruction;
    }

    auto command = command_list[bytecode];

    Instruction instruction = {};
    instruction.command = bytecode;
    for (auto const& param : command.params) {
        std::variant<uint8_t, uint16_t> result;
        if (param == 1) {
            result = static_cast<uint8_t>(data[offset]);
        } else if (param == 2) {
            result = static_cast<uint16_t>(data[offset] | (data[offset + 1] << 8));
        }
        offset += param;
        instruction.params.push_back(result);
    }

    return instruction;
}

auto Event::parse_event() -> std::vector<Instruction>
{
    std::vector<Instruction> commands;
    while (offset < text_offset()) {
        auto command = next_command();
        commands.push_back(command);
    }
    return commands;
}

auto Scenario::repr() -> std::string
{
    std::ostringstream oss;
    oss << std::setw(3) << std::setfill('0') << id() << " " << map_list[map_id()].name;
    return oss.str();
}
auto Scenario::id() const -> int { return data[0] | (data[1] << 8); }
auto Scenario::map_id() -> int { return data[2]; }
auto Scenario::weather() -> MapWeather { return static_cast<MapWeather>(data[3]); }
auto Scenario::time() -> MapTime { return static_cast<MapTime>(data[4]); }
auto Scenario::first_music() -> int { return data[5]; }
auto Scenario::second_music() -> int { return data[6]; }
auto Scenario::entd_id() -> int { return data[7] | (data[8] << 8); }
auto Scenario::first_grid() -> int { return data[9] | (data[10] << 8); }
auto Scenario::second_grid() -> int { return data[11] | (data[12] << 8); }
auto Scenario::require_ramza_unknown() -> int { return data[17]; }
auto Scenario::next_event_id() -> int { return data[18] | (data[19] << 8); }
auto Scenario::event_id() -> int { return data[22] | (data[23] << 8); }
