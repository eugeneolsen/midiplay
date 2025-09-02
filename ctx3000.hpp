#pragma once

#include <cxxmidi/output/default.hpp>
#include <cxxmidi/message.hpp>
#include <cxxmidi/event.hpp>

#include <cstdint>
#include <cxxmidi/channel.hpp>

#include "midi_constants.hpp"
#include "device_constants.hpp"

class ctx3000
{
private:
    cxxmidi::output::Default &_outport;

    void SelectProgram(std::uint8_t channel, std::uint8_t bank, std::uint8_t program)
    {
        using namespace MidiPlay::Midi;
        cxxmidi::Event e = cxxmidi::Event(0, channel | cxxmidi::Message::kControlChange, CC_BANK_SELECT_MSB, bank); // Bank Select MSB
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
        using namespace MidiPlay::Device::Casio;
        using namespace MidiPlay::Midi;
        
        SelectProgram(cxxmidi::Channel1, BANK_32, PIPE_ORGAN_PROGRAM); // Pipe Organ 1
        SelectProgram(cxxmidi::Channel2, BANK_32, PIPE_ORGAN_PROGRAM); // Pipe Organ 1
        SelectProgram(cxxmidi::Channel3, BANK_36, BRASS_STRINGS_PROGRAM); // Brass and Strings

        cxxmidi::Event e = cxxmidi::Event(0, static_cast<std::uint8_t>(cxxmidi::Channel3) | static_cast<std::uint8_t>(cxxmidi::Message::kControlChange), CC_VOLUME, VOLUME_FULL); // Full volume on Channel 3
        _outport.SendMessage(&e);
    }
};
