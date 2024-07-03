#include <assert.h>

#include "Event.h"

auto Event::id() -> int
{
    return (data[0] & 0xFF) | ((data[1] & 0xFF) << 8) | ((data[2] & 0xFF) << 16) | ((data[3] & 0xFF) << 24);
}

auto Event::should_skip() -> bool
{
    return id() == 0xf2f2f2f2;
}

auto Event::text_offset() -> uint32_t
{
    assert(!should_skip());
    return id();
}

auto Event::next_instruction() -> Instruction
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

auto Event::instructions() -> std::vector<Instruction>
{
    std::vector<Instruction> commands;
    while (offset < text_offset()) {
        auto command = next_instruction();
        commands.push_back(command);
    }
    return commands;
}

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
