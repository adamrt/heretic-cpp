#include <algorithm>
#include <assert.h>
#include <cmath>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <map>
#include <memory>
#include <numeric>
#include <optional>
#include <regex>
#include <span>
#include <sstream>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "Event.h"
#include "Font.h"

auto Instruction::param_float(int index) const -> float
{
    uint16_t stored_value = std::get<uint16_t>(params[index]);
    int16_t signed_value = static_cast<int16_t>(stored_value);
    return static_cast<float>(signed_value);
}

auto Instruction::param_int(int index) const -> int
{
    uint16_t stored_value = std::get<uint16_t>(params[index]);
    int16_t signed_value = static_cast<int16_t>(stored_value);
    return static_cast<int>(signed_value);
}

Event::Event(std::vector<uint8_t> data)
{
    m_text_offset = static_cast<uint32_t>((data[0] & 0xFF) | ((data[1] & 0xFF) << 8) | ((data[2] & 0xFF) << 16) | ((data[3] & 0xFF) << 24));

    m_should_skip = m_text_offset == 0xF2F2F2F2;
    if (m_should_skip) {
        return;
    }

    m_text_section = std::vector<uint8_t>(data.begin() + m_text_offset, data.end());
    m_code_section = std::vector<uint8_t>(data.begin() + 4, data.begin() + m_text_offset);
}

auto Event::next_instruction() -> Instruction
{
    assert(!m_should_skip);
    auto bytecode = m_code_section[m_code_offset];
    m_code_offset++;

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
        // Store everything as a uint8_t or uint16_t. We can cast them later.
        std::variant<uint8_t, uint16_t> result;
        if (param == 1) {
            result = static_cast<uint8_t>(m_code_section[m_code_offset]);
        } else if (param == 2) {
            result = static_cast<uint16_t>(m_code_section[m_code_offset] | (m_code_section[m_code_offset + 1] << 8));
        }
        m_code_offset += param;
        instruction.params.push_back(result);
    }

    return instruction;
}

auto Event::instructions() -> std::vector<Instruction>
{
    if (!m_cached_instructions.empty()) {
        return m_cached_instructions;
    }

    assert(!m_should_skip);
    std::vector<Instruction> instructions;
    auto len = m_code_section.size();

    while (m_code_offset < len) {
        auto instruction = next_instruction();
        instructions.push_back(instruction);
    }

    // Reset the index in case we want to read again.
    m_code_offset = 0;

    m_cached_instructions = instructions;

    return instructions;
}

std::vector<std::string> split_string(const std::string& str, char delimiter)
{
    std::istringstream stream(str);
    std::vector<std::string> tokens;
    std::string token;

    while (getline(stream, token, delimiter)) {
        tokens.push_back(token);
    }

    return tokens;
}

auto Event::messages() -> std::vector<std::string>
{
    if (!m_cached_messages.empty()) {
        return m_cached_messages;
    }

    printf("Reading messages\n");
    assert(!m_should_skip);
    auto message_vec = std::vector<std::string> {};

    uint8_t delemiter = 0xFE;
    std::string delemiter_str(1, static_cast<char>(delemiter));

    for (int i = 0; i < (int)m_text_section.size(); i++) {
        uint8_t byte = m_text_section[i];

        // These are special characters. We need to handle them differently.
        // https://ffhacktics.com/wiki/Text_Format#Special_Characters
        switch (byte) {
        case 0xFE:
            // This is the message delimiter.
            message_vec.push_back(delemiter_str);
            continue;
        case 0xE0: {
            // Character name stored somewhere else. Hard coding for now.
            message_vec.push_back("Ramza");
            continue;
        }
        case 0xE2: {
            uint8_t delay = m_text_section[++i];
            std::ostringstream ss;
            ss << "{Delay: " << (int)delay << "}";
            message_vec.push_back(ss.str());
            continue;
        }
        case 0xE3: {
            uint8_t color = m_text_section[++i];
            std::ostringstream ss;
            ss << "{Color: " << (int)color << "}";
            message_vec.push_back(ss.str());
            continue;
        }
        case 0xF0:
        case 0xF1:
        case 0xF2:
        case 0xF3: {
            // This is a jump to another point in the text section.
            // The next 2 bytes are the jump location and how many bytes to read.
            // https://gomtuu.org/fft/trans/compression/
            auto second_byte = m_text_section[++i];
            auto third_byte = m_text_section[++i];
            (void)second_byte;
            (void)third_byte;
            message_vec.push_back("{TextJump}");
            continue;
        }
        case 0xF8:
            message_vec.push_back("\n");
            continue;
        case 0xFA:
            // This one is not in the list but it is very common between words.
            // It works well as a space though.
            message_vec.push_back(" ");
            continue;
        case 0xFF:
            message_vec.push_back("{Close}");
            continue;
        }

        // Bytes higher than 0xCF are two byte characters.
        // https://ffhacktics.com/wiki/Font
        if (byte > 0xCF) {
            auto second_byte = m_text_section[i + 1];
            // combine the two bytes, c and z into a single 16 bit value, in little endian.
            uint16_t combined = (second_byte | (byte << 8));
            if (font.find(combined) != font.end()) {
                message_vec.push_back(font[combined]);
                i++;
            } else {
                // Print the unknown byte and its second byte. But we don't
                // actually consume the second byte. This is because if they are
                // an instruction, like the ones above (0xFA, 0xF8, etc), we
                // don't want to consume the second byte as a two byte character.
                std::ostringstream ss;
                ss << "{Unknown: 0x" << std::hex << byte << std::dec << " & 0x" << std::hex << second_byte << std::dec << "}";
                message_vec.push_back(ss.str());
            }
            continue;
        }

        if (font.find(byte) == font.end()) {
            message_vec.push_back("?");
            continue;
        }

        message_vec.push_back(font[byte]);
    }

    // All text for the event.
    auto event_text = std::accumulate(message_vec.begin(), message_vec.end(), std::string(""));
    auto messages = split_string(event_text, delemiter);

    std::vector<std::string> event_messages;
    for (auto& instruction : instructions()) {
        if (instruction.command == 0x10) {
            auto pointer = std::get<uint16_t>(instruction.params[2]);
            if (pointer > messages.size()) {
                continue;
            }
            auto message = messages[pointer];
            event_messages.push_back(message);
        }
        // FIXME: Handle 0x51 as well. It uses a different param index.
    }
    m_cached_messages = event_messages;
    return event_messages;
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
