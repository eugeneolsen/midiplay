#include <cxxmidi/output/default.hpp>
#include <cxxmidi/message.hpp>
#include <cxxmidi/event.hpp>

#include <ecomidi/channel.hpp>

#pragma once

using namespace cxxmidi;

class psr_ew425
{
private:
    output::Default &_outport;

    void SelectProgram(uint8_t channel, uint8_t bank, uint8_t program)
    {
        Event e = Event(0, channel | Message::kControlChange, 0, 0); // Bank Select MSB
        _outport.SendMessage(&e);

        e = Event(0, channel | Message::kControlChange, 32, bank); // Bank Select LSB
        _outport.SendMessage(&e);

        e = Event(0, channel | Message::kProgramChange, program); // Program change
        _outport.SendMessage(&e);
    }

public:
    psr_ew425(output::Default &outport) : _outport(outport)
    {
    }

    void SetDefaults()
    {
        SelectProgram(Channel1, 113, 20); // Bank 113, Program 20: Chapel Organ
        SelectProgram(Channel2, 113, 20);   // Bank 0, Program 20: Church Organ 1
        SelectProgram(Channel3, 112, 4);  // Bank 112, Program 49: Strings

        Event e = Event(0, Channel3 | Message::kControlChange, 7, 127); // Full volume on Channel 3
        _outport.SendMessage(&e);
    }
};
