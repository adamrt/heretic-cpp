#pragma once

#include <map>
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
    Event(std::vector<uint8_t> data)
        : data(data) {};

    auto id() -> int;
    auto should_skip() -> bool;
    auto instructions() -> std::vector<Instruction>;

private:
    auto text_offset() -> uint32_t;
    auto next_instruction() -> Instruction;

    std::vector<uint8_t> data;
    uint32_t offset = 4; // Skip the first 4 bytes as its the id.
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
