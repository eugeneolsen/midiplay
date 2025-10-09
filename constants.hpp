#pragma once

namespace MidiPlay {
    // Application-wide shared constants
    constexpr int MICROSECONDS_PER_MINUTE = 60000000;
    constexpr int SECONDS_PER_MINUTE = 60;
    constexpr int QUARTER_NOTE_DENOMINATOR = 4;
    constexpr int EXIT_FILE_NOT_FOUND = 2;
    constexpr int EXIT_DEVICE_NOT_FOUND = 6;
    constexpr int EXIT_ENVIRONMENT_ERROR = 3;
    constexpr int DEFAULT_VERSES = 1;
    
    // Options parse result codes
    namespace OptionsParseResult {
        constexpr int SUCCESS = 0;
        constexpr int HELP_DISPLAYED = 1;
        constexpr int MISSING_FILENAME = 2;
        constexpr int INVALID_TEMPO = 3;
        constexpr int INVALID_OPTION = 4;
        constexpr int VERSION_DISPLAYED = -2;  // Existing convention
    }
}