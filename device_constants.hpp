#pragma once

#include <cstdint>

namespace MidiPlay {
    namespace Device {
        // Device connection constants
        constexpr int CONNECTION_TIMEOUT = 300;
        constexpr std::size_t MIN_PORT_COUNT = 2;
        constexpr int POLL_SLEEP_SECONDS = 2;
        constexpr int OUTPUT_PORT_INDEX = 1;
        
        // Casio CTX-3000 device-specific constants
        namespace Casio {
            constexpr std::uint8_t BANK_32 = 32;
            constexpr std::uint8_t BANK_36 = 36;
            constexpr std::uint8_t PIPE_ORGAN_PROGRAM = 19;
            constexpr std::uint8_t BRASS_STRINGS_PROGRAM = 48;
        }
        
        // Yamaha PSR-EW425 device-specific constants
        namespace Yamaha {
            constexpr std::uint8_t BANK_112 = 112;
            constexpr std::uint8_t BANK_113 = 113;
            constexpr std::uint8_t CHAPEL_ORGAN_PROGRAM = 20;
            constexpr std::uint8_t STRINGS_PROGRAM = 4;
        }
    }
}