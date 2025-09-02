#pragma once

#include <cxxmidi/output/default.hpp>
#include <cxxmidi/message.hpp>
#include <cxxmidi/event.hpp>

#include <cstdint>
#include <cxxmidi/channel.hpp>

#include "midi_constants.hpp"
#include "device_constants.hpp"

class psr_ew425
{
private:
    cxxmidi::output::Default & _outport;

    void SelectProgram(std::uint8_t channel, std::uint8_t bank, std::uint8_t program)
    {
        using namespace MidiPlay::Midi;
        cxxmidi::Event e = cxxmidi::Event(0, channel | cxxmidi::Message::kControlChange, CC_BANK_SELECT_MSB, BANK_SELECT_OFF); // Bank Select MSB
        _outport.SendMessage(&e);

        e = cxxmidi::Event(0, channel | cxxmidi::Message::kControlChange, CC_BANK_SELECT_LSB, bank); // Bank Select LSB
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
        using namespace MidiPlay::Device::Yamaha;
        using namespace MidiPlay::Midi;
        
        SelectProgram(cxxmidi::Channel1, BANK_113, CHAPEL_ORGAN_PROGRAM); // Chapel Organ
        SelectProgram(cxxmidi::Channel2, BANK_113, CHAPEL_ORGAN_PROGRAM); // Chapel Organ
        SelectProgram(cxxmidi::Channel3, BANK_112, STRINGS_PROGRAM); // Strings

        cxxmidi::Event e = cxxmidi::Event(0, static_cast<std::uint8_t>(cxxmidi::Channel3) | static_cast<std::uint8_t>(cxxmidi::Message::kControlChange), CC_VOLUME, VOLUME_FULL); // Full volume on Channel 3
        _outport.SendMessage(&e);
    }
};
