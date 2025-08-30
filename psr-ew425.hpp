#pragma once

#include <cxxmidi/output/default.hpp>
#include <cxxmidi/message.hpp>
#include <cxxmidi/event.hpp>

#include <cstdint>
#include <cxxmidi/channel.hpp>

class psr_ew425
{
private:
    cxxmidi::output::Default & _outport;

    void SelectProgram(std::uint8_t channel, std::uint8_t bank, std::uint8_t program)
    {
        cxxmidi::Event e = cxxmidi::Event(0, channel | cxxmidi::Message::kControlChange, 0, 0 ); // Bank Select MSB
        _outport.SendMessage(&e);

        e = cxxmidi::Event(0, channel | cxxmidi::Message::kControlChange, 32, bank); // Bank Select LSB
        _outport.SendMessage(&e);

        e = cxxmidi::Event(0, channel | cxxmidi::Message::kProgramChange, program); // Program change
        _outport.SendMessage(&e);
    }

public:
    psr_ew425(cxxmidi::output::Default &outport) : _outport(outport)
    {
    }

    void SetDefaults()
    {
        SelectProgram(cxxmidi::Channel1, 113, 20); // Bank 113, Program 20: Chapel Organ
        SelectProgram(cxxmidi::Channel2, 113, 20); // Bank 0, Program 20: Church Organ 1
        SelectProgram(cxxmidi::Channel3, 112, 4); // Bank 112, Program 4: Strings

        cxxmidi::Event e = cxxmidi::Event(0, static_cast<std::uint8_t>(cxxmidi::Channel3) | static_cast<std::uint8_t>(cxxmidi::Message::kControlChange), 7, 127); // Full volume on Channel 3
        _outport.SendMessage(&e);
    }
};
