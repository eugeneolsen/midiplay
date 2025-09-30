#pragma once

#include <cstdint>

namespace MidiPlay {
    namespace Midi {
        // MIDI Control Change Numbers
        constexpr std::uint8_t CC_BANK_SELECT_MSB = 0;
        constexpr std::uint8_t CC_BANK_SELECT_LSB = 32;
        constexpr std::uint8_t CC_VOLUME = 7;
        
        // Standard MIDI Values
        constexpr std::uint8_t VOLUME_FULL = 127;
        constexpr std::uint8_t VOLUME_OFF = 0;
        constexpr std::uint8_t BANK_SELECT_OFF = 0;
        constexpr std::uint8_t DEFAULT_TEMPO_BPM = 120;
        constexpr int DEFAULT_TEMPO_USEC_PER_QUARTER = 500000; // 120 BPM
    }
}