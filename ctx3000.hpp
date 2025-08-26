#include <cxxmidi/output/default.hpp>
#include <cxxmidi/message.hpp>
#include <cxxmidi/event.hpp>

#include <cxxmidi/channel.hpp>

#pragma once

using namespace cxxmidi;

class ctx3000
{
private:
    output::Default &_outport;

    void SelectProgram(uint8_t channel, uint8_t bank, uint8_t program)
    {
        Event e = Event(0, channel | Message::kControlChange, 0, bank); // Bank Select MSB
        _outport.SendMessage(&e);

        e = Event(0, channel | Message::kProgramChange, program); // Program change
        _outport.SendMessage(&e);
    }

public:
    ctx3000(output::Default &outport) : _outport(outport)
    {
    }

    void SetDefaults()
    {
        SelectProgram(Channel1, 32, 19); // Bank 32, Program 19: Pipe Organ 1
        SelectProgram(Channel2, 32, 19); // "
        SelectProgram(Channel3, 36, 48); // Bank 36, Program 48: Brass and Strings

        Event e = Event(0, static_cast<uint8_t>(Channel3) | static_cast<uint8_t>(Message::kControlChange), 7, 127); // Full volume on Channel 3
        _outport.SendMessage(&e);
    }
};
