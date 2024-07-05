#pragma once

#include <iostream>
#include <map>
#include <span>
#include <string>
#include <variant>
#include <vector>

// An instruction single instruction and its parameters from an Event.
struct Instruction {
    // The command id.
    uint8_t command;

    // A vector of parameters that are always 1 or 2 bytes.
    std::vector<std::variant<uint8_t, uint16_t>> params;
};

// An event is a list of instructions for a particular scenario.
//
// Events are alway 8192 (0x2000) bytes long. There are 3 components.
// - text_offset: First 4 bytes is a pointer to the to the text_section.
//   - If the offset is 0xF2F2F2F2, then the event should be skipped.
// - code_section: Bytes 5 to text_offset is the code section.
// - text_section: Bytes text_offset thru 8192 is the text section.
class Event {
public:
    Event(std::vector<uint8_t> data);

    auto instructions() -> std::vector<Instruction>;
    auto messages() -> std::vector<std::string>;
    auto next_instruction() -> Instruction;

    bool should_skip;
    std::vector<uint8_t> code_section;
    std::vector<uint8_t> text_section;

private:
    // text_offset is a constant value to points to the start of the text_section.
    uint32_t text_offset;

    // code_offset is the current pointer position into the code_section.
    uint32_t code_offset = 0;
};

// A command represents an bit of functionality in FFT Events. This is used
// stricly to have a list of commands and their parameters. See command_list.
struct Command {
    std::string name;

    // A vector of integers that represent the size, in bytes of each parameter.
    std::vector<int> params;
};

// A list of commands available in the game.
//
// This does not include the Event Instruction Upgrade hack since we only handle
// the vanilla game.
//
// https://ffhacktics.com/wiki/Event_Instructions
extern std::map<int, Command> command_list;
