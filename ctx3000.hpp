#pragma once

#include <cxxmidi/output/default.hpp>
#include <cxxmidi/message.hpp>
#include <cxxmidi/event.hpp>

#include <cstdint>
#include <cxxmidi/channel.hpp>

class ctx3000
{
private:
    cxxmidi::output::Default &_outport;

    void SelectProgram(std::uint8_t channel, std::uint8_t bank, std::uint8_t program)
    {
        cxxmidi::Event e = cxxmidi::Event(0, channel | cxxmidi::Message::kControlChange, 0, bank); // Bank Select MSB
        _outport.SendMessage(&e);

        e = cxxmidi::Event(0, channel | cxxmidi::Message::kProgramChange, program); // Program change
        _outport.SendMessage(&e);
    }

public:
    ctx3000(cxxmidi::output::Default &outport) : _outport(outport)
    {
    }

    void SetDefaults()
    {
        SelectProgram(cxxmidi::Channel1, 32, 19); // Bank 32, Program 19: Pipe Organ 1
        SelectProgram(cxxmidi::Channel2, 32, 19); // "
        SelectProgram(cxxmidi::Channel3, 36, 48); // Bank 36, Program 48: Brass and Strings

        cxxmidi::Event e = cxxmidi::Event(0, static_cast<std::uint8_t>(cxxmidi::Channel3) | static_cast<std::uint8_t>(cxxmidi::Message::kControlChange), 7, 127); // Full volume on Channel 3
        _outport.SendMessage(&e);
    }
};
