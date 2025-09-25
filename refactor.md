# Code Smell Analysis and Refactoring Recommendations

## Overview

This document contains a comprehensive analysis of code smells found in the MIDI player project, along with prioritized refactoring recommendations. The analysis covers both the initial findings and additional magic numbers/strings identified through detailed code inspection.

## Project Context

This is a C++ MIDI file player optimized for Raspberry Pi, designed to play hymns through USB-to-MIDI converters to various organs and keyboards. The project uses cxxmidi library and supports custom MIDI meta events for controlling playback.

## Major Code Smells Identified

### 1. God Object / Long Method Anti-Pattern
**Location:** `play.cpp` main function (659 lines)

**Issues:**
- Single function handles option parsing, MIDI loading, device detection, playback control, and cleanup
- Massive inline callbacks (100+ lines) for MIDI event handling
- Multiple responsibilities mixed together

**Impact:** Difficult to test, maintain, and extend.

### 2. Excessive Global Variables
**Location:** `play.cpp`

**Issues:**
- 15+ global variables: `outport`, `sem`, `timesig`, `keySignature`, `introSegments`, etc.
- Makes code hard to test and reason about
- Poor encapsulation

### 3. Magic Numbers and Strings
**Hardcoded Constants Found:**

#### Magic Numbers:
**play.cpp:**
- `300` - device connection timeout loop iterations
- `2` - sleep seconds between connection attempts
- `Channel1, Channel3` - MIDI channel range for note-off
- `Note::kC2, Note::kC7` - note range for note-off
- `0x10, 0x11` - deprecated meta event types
- `0xFF7F` - sequencer-specific meta event
- `0x7D` - private event type
- `0x01, 0x02` - private subtypes
- `60000000` - microseconds per minute for BPM conversion
- `4` - denominator power in tempo calculation
- `1` - MIDI output port number
- `100000` - heartbeat callback interval
- `0.002` - ritardando speed decrement
- `10` - prelude speed factor
- `8` - default prelude speed
- `6, 9` - key signature array offsets

**ctx3000.hpp:**
- `32, 19` - bank/program for pipe organ
- `36, 48` - bank/program for brass/strings
- `7, 127` - volume control values

**psr-ew425.hpp:**
- `113, 20` - bank/program for chapel organ
- `112, 4` - bank/program for strings
- `0, 0` - bank MSB/LSB defaults
- `32` - bank LSB control number
- `7, 127` - volume control values

**options.hpp:**
- `0.90` - default prelude speed
- `0.5` - min prelude speed
- `2.0` - max prelude speed
- `60000000` - BPM to microseconds conversion
- `10.0` - prelude speed conversion divisor

#### Magic Strings:
**play.cpp:**
- `"1.4.3"` - version string
- Keys array: `{"Gb", "Db", "Ab", "Eb", "Bb", "F", "C", "G", "D", "A", "E", "B", "F#", "C#", "G#", "D#", "A#"}`
- `INTRO_BEGIN = "["`, `INTRO_END = "]"`
- `RITARDANDO_INDICATOR = R"(\\)"`
- `D_C_AL_FINE = "D.C. al Fine"`
- `FINE_INDICATOR = "Fine"`
- `"CASIO USB"`, `"Digital Keyboard"` - device detection strings
- Various output messages: `"Hymn ... was not found"`, `"Device connection timeout"`, `"Playing introduction"`, etc.

**options.hpp:**
- Long option names: `{"help", "version", "prelude", "goto", "channel", "staging", "stops", "tempo", "title", "warnings"}`
- Help text strings and error messages

**ticks.hpp:**
- `"MIDI ticks must be a non-negative integer."`
- `"Current MIDI Ticks:"`, `"Current MIDI Ticks: null"`

### 4. Namespace Pollution
**Location:** Multiple header files

**Issues:**
- `using namespace std;` in headers
- `using namespace cxxmidi;` in headers
- Pollutes global namespace for all including files

### 5. Code Duplication
**Location:** `ctx3000.hpp`, `psr-ew425.hpp`

**Issues:**
- Nearly identical `SelectProgram` methods
- Similar `SetDefaults` patterns
- Duplicate volume control code

### 6. Over-Engineered Classes
**Location:** `ticks.hpp`

**Issues:**
- Unnecessary operators and methods for simple optional<int> wrapper
- `display()` method mixes logic with output
- Missing `const` qualifiers

### 7. Poor Synchronization
**Location:** `play.cpp`

**Issues:**
- Uses old semaphore-based approach instead of modern C++ features
- Complex synchronization logic scattered throughout

### 8. Incomplete Features
**Location:** `options.hpp`, `protege.hpp`

**Issues:**
- TODO comments for unimplemented features
- Dead/commented code left in `protege.hpp`

### 9. Mixed Concerns
**Location:** Throughout `play.cpp`

**Issues:**
- MIDI parsing, playback logic, device-specific setup all mixed
- Error handling inconsistent
- No separation between business logic and presentation

### 10. Hardcoded Device Configuration
**Location:** `ctx3000.hpp`, `psr-ew425.hpp`

**Issues:**
- Device-specific bank/program numbers hardcoded
- No way to configure without code changes
- Difficult to support new devices

## Prioritized Refactoring Recommendations

### High Impact, Low Effort:
1. **Remove namespace directives from headers** - Prevents pollution, improves compilation speed
2. **Remove dead/commented code** - Clean up unused code in `protege.hpp`
3. **Add const qualifiers** - Improve method signatures in `ticks.hpp`
4. **Extract magic numbers to named constants** - Create `constants.hpp` with MIDI-related constants
5. **Extract magic strings to constants** - Move device names, marker strings, and messages to constants

### Medium Impact, Medium Effort:
6. **Refactor main() into smaller functions** - Extract `setupDevice()`, `loadMidiFile()`, `playIntro()`, `playVerses()`
7. **Create device configuration system** - Replace hardcoded bank/program values with configurable presets
8. **Encapsulate globals** - Move into a `PlayerContext` class or pass as parameters
9. **Extract device setup logic** - Factory pattern for keyboard-specific initialization

### High Impact, High Effort:
10. **Extract callback handlers** - Create separate classes for MIDI event handling
11. **Implement proper error handling** - Add try-catch blocks and logging with configurable messages
12. **Complete TODO features** - Implement goto, channel override, stops functionality
13. **Eliminate duplication** - Base class for keyboard devices with common `SelectProgram` logic
14. **Implement configuration-driven device setup** - Load keyboard configurations from external files
15. **Add internationalization support** - Extract user-facing strings to resource files

## Detailed Implementation Plan

The refactoring will follow this sequence:

### Phase 1: Code Cleanup (1-2 days)
1. **Create constants.hpp** - Define all magic numbers as named constants
2. **Create messages.hpp** - Define all user-facing strings as constants
3. **Create device_config.hpp** - Structure for keyboard configurations
4. **Clean up headers** - Remove namespace pollution, dead code
5. **Add missing const qualifiers** - Improve const-correctness

### Phase 2: Structural Refactoring (3-5 days)
6. ✅ **Extract device setup logic** - Factory pattern with configuration loading - **COMPLETED**
7. **Break down main()** - Separate functions for setup, loading, playback
8. **Create handler classes** - For MIDI events and playback logic
9. **Encapsulate global state** - PlayerContext class

### Phase 3: Feature Enhancement (5-7 days)
10. **Implement configuration-driven device setup** - External JSON/YAML configuration
11. **Add comprehensive error handling** - Try-catch blocks and logging
12. **Complete missing features** - goto, channel override, stops
13. **Eliminate code duplication** - Common base classes

### Phase 4: Testing and Polish (2-3 days)
14. **Add unit tests** - For new classes and functions
15. **Performance optimization** - Profile and optimize bottlenecks
16. **Documentation update** - Update README and add API docs

## Benefits of Refactoring

- **Maintainability:** Easier to modify device configurations and add new features
- **Testability:** Smaller, focused functions are easier to unit test
- **Extensibility:** Configuration-driven approach supports new devices without code changes
- **Readability:** Named constants and extracted functions improve code comprehension
- **Reliability:** Better error handling and elimination of global state reduces bugs

## Risks and Mitigations

- **Breaking changes:** Implement changes incrementally with thorough testing
- **Performance impact:** Profile after each phase to ensure no degradation
- **Configuration complexity:** Start with simple JSON configuration, expand as needed

## Conclusion

This refactoring will transform the codebase from a monolithic script into a well-structured, maintainable application. The magic numbers and strings represent the most immediate opportunity for improvement, followed by structural changes to improve the overall architecture.

The phased approach ensures that changes can be implemented incrementally while maintaining functionality throughout the process.

---

## Modular Architecture Design for play.cpp

### Overview

The current [`play.cpp`](play.cpp) file is 706 lines long and handles multiple responsibilities. This section outlines a comprehensive modular architecture to break it into logical, maintainable components.

### Proposed Module Structure

#### Core Application Files:
```
play.cpp                    // Main entry point (~150 lines)
├── midi_loader.hpp/.cpp    // MIDI file loading and parsing
├── playback_engine.hpp/.cpp // Playback control and callbacks  
├── device_manager.hpp/.cpp  // MIDI device detection and setup
├── timing_manager.hpp/.cpp  // Tempo, timing, and synchronization
└── signal_handler.hpp/.cpp  // Signal handling (Ctrl+C)
```

#### Supporting Files:
```
types.hpp                   // Common types and structures
file_utils.hpp/.cpp         // File existence checking, path utilities
```

#### Constants Organization:
```
constants.hpp               // Application-wide shared constants
midi_constants.hpp          // MIDI protocol constants
device_constants.hpp        // Device-specific constants
```

### Detailed Module Breakdown

#### A. `midi_loader.hpp/.cpp` (Lines 193-404)
**Responsibility**: MIDI file loading, parsing, and meta-event processing

**Key Functions**:
- Load callback with tempo, key signature, time signature extraction
- Verse counting and intro segment detection
- Custom meta-event handling (verses, pause between verses)
- Key signature processing with major/minor detection

**Constants Used**:
```cpp
constexpr int MAJOR_KEY_OFFSET = 6;
constexpr int MINOR_KEY_OFFSET = 9;
constexpr uint8_t META_EVENT_VERSES = 0x10;
constexpr uint8_t META_EVENT_PAUSE = 0x11;
constexpr int QUARTER_NOTE_DENOMINATOR = 4;
```

#### B. `playback_engine.hpp/.cpp` (Lines 535-620, 625-691)
**Responsibility**: Player callbacks, intro/verse management, ritardando control

**Key Functions**:
- Heartbeat and event callbacks
- Introduction and verse playback logic
- Ritardando and musical direction handling (D.C. al Fine, Fine)
- Intro segment jumping logic

**Constants Used**:
```cpp
constexpr int64_t HEARTBEAT_CHECK_INTERVAL = 100000;
constexpr float RITARDANDO_DECREMENT = 0.002f;
constexpr int VERSE_DISPLAY_OFFSET = 1;
```

#### C. `device_manager.hpp/.cpp` (Lines 464-518)
**Responsibility**: MIDI device detection, connection, and device-specific setup

**Key Functions**:
- Device timeout and connection logic
- Casio, Yamaha, and Protege device setup
- Port management and device identification
- Device-specific program/bank selection

**Constants Used**:
```cpp
constexpr int CONNECTION_TIMEOUT = 300;
constexpr size_t MIN_PORT_COUNT = 2;
constexpr int POLL_SLEEP_SECONDS = 2;
constexpr int OUTPUT_PORT_INDEX = 1;
```

#### D. `timing_manager.hpp/.cpp` (Lines 622-624, 693-703)
**Responsibility**: Elapsed time tracking, timing calculations

**Key Functions**:
- Start/end timer functionality
- Time display formatting (minutes:seconds)
- Elapsed time calculation

**Constants Used**:
```cpp
constexpr int SECONDS_PER_MINUTE = 60;
```

#### E. `signal_handler.hpp/.cpp` (Lines 112-142)
**Responsibility**: Signal handling and emergency cleanup

**Key Functions**:
- Ctrl+C handling with graceful shutdown
- Emergency notes-off functionality
- Semaphore cleanup
- Elapsed time display on interrupt

### Constants Organization Strategy

#### Layered Constants Architecture:

```cpp
// constants.hpp - Application-wide shared constants
namespace MidiPlay {
    constexpr int MICROSECONDS_PER_MINUTE = 60000000;
    constexpr int SECONDS_PER_MINUTE = 60;
    constexpr int QUARTER_NOTE_DENOMINATOR = 4;
    constexpr int EXIT_FILE_NOT_FOUND = 2;
    constexpr int EXIT_DEVICE_NOT_FOUND = 6;
    constexpr int DEFAULT_VERSES = 1;
}

// midi_constants.hpp - MIDI protocol shared constants
namespace MidiPlay::Midi {
    constexpr std::uint8_t CC_BANK_SELECT_MSB = 0;
    constexpr std::uint8_t CC_BANK_SELECT_LSB = 32;
    constexpr std::uint8_t CC_VOLUME = 7;
    constexpr std::uint8_t VOLUME_FULL = 127;
    constexpr std::uint8_t VOLUME_OFF = 0;
}

// device_constants.hpp - Device management and device-specific constants
namespace MidiPlay::Device {
    // Connection constants
    constexpr int CONNECTION_TIMEOUT = 300;
    constexpr size_t MIN_PORT_COUNT = 2;
    constexpr int POLL_SLEEP_SECONDS = 2;
    constexpr int OUTPUT_PORT_INDEX = 1;
    
    // Device-specific program/bank combinations
    namespace Casio {
        constexpr std::uint8_t PIPE_ORGAN_BANK = 32;
        constexpr std::uint8_t PIPE_ORGAN_PROGRAM = 19;
        constexpr std::uint8_t BRASS_STRINGS_BANK = 36;
        constexpr std::uint8_t BRASS_STRINGS_PROGRAM = 48;
    }
    
    namespace Yamaha {
        constexpr std::uint8_t CHAPEL_ORGAN_BANK = 113;
        constexpr std::uint8_t CHAPEL_ORGAN_PROGRAM = 20;
        constexpr std::uint8_t STRINGS_BANK = 112;
        constexpr std::uint8_t STRINGS_PROGRAM = 4;
    }
}
```

### Benefits of Layered Constants:

1. **Clear Ownership**: Constants live with their primary usage context
2. **Namespace Separation**: Prevents naming conflicts
3. **Reduced Dependencies**: Only include what you need
4. **Easy Migration**: Can move constants as modules evolve
5. **Eliminates Duplication**: Shared MIDI constants across all device classes
6. **Better Organization**: Clear separation between protocol constants and device-specific values

### Migration Strategy

#### Phase 1: Create Shared Constants ✅ COMPLETED
1. ✅ Create `constants.hpp` with application-wide constants
2. ✅ Create `midi_constants.hpp` with MIDI protocol constants  
3. ✅ Create `device_constants.hpp` with device-specific values
4. ✅ Update device classes to use shared constants
5. ✅ Move `MICROSECONDS_PER_MINUTE` from `options.hpp` to shared constants
6. ✅ Update all references to use namespaced constants

#### Phase 2: Extract Modules
1. ✅ `signal_handler.cpp` (smallest, most isolated) - **COMPLETED**
2. ✅ `device_manager.cpp` (now benefits from shared device constants) - **COMPLETED**
3. ✅ `midi_loader.cpp` (complex but well-defined boundaries) - **COMPLETED**
4. `playback_engine.cpp` and `timing_manager.cpp`

#### Future Enhancement: Options Class Const-Correctness
**Priority:** Medium | **Effort:** Low | **Impact:** Code Quality

**Issue:** The Options class getter methods (`getBpm()`, `getVerses()`, `getSpeed()`, `isStaging()`, etc.) are not const qualified, which:
- Prevents passing Options objects as `const&` parameters
- Violates C++ best practices for immutable operations
- Forces unnecessary mutable references in clean interfaces like MidiLoader

**Solution:**
```cpp
// In options.hpp - add const to all getter methods:
int getBpm() const { return _bpm; }
int getVerses() const { return _verses; }
float getSpeed() const { return _speed; }
bool isStaging() const { return _staging; }
std::string getFileName() const { return _filename; }
// ... etc for all 10+ getter methods
```

**Benefits:**
- **Better API Design** - Getters should be const since they don't modify state
- **Const-Correctness** - Enables passing `const Options&` to functions
- **Cleaner MidiLoader Interface** - Could change `loadFile(path, Options& options)` to `loadFile(path, const Options& options)`
- **Thread Safety** - Const methods can be safely called from multiple threads
- **Documentation** - Clearly indicates which methods are read-only

**Implementation:** ~15 minutes to add const qualifiers and update MidiLoader interface

#### Phase 3: Refactor Main (Future)
- Orchestration-only main function
- Dependency injection between modules
- Clean separation of concerns

### Key Architectural Benefits

1. **Single Responsibility**: Each module has one clear purpose
2. **Testability**: Independent unit testing capability
3. **Maintainability**: Localized changes and clear boundaries
4. **Reusability**: Modules could be reused in other MIDI applications
5. **Clear Constants Strategy**: Avoids both magic numbers and god header files
6. **Reduced Compilation Dependencies**: Faster builds and cleaner includes
7. **Device Extensibility**: Easy to add new device types with shared MIDI constants

### Implementation Status

**Phase 1 - COMPLETED**: 
- ✅ Created shared constants files (`constants.hpp`, `midi_constants.hpp`, `device_constants.hpp`)
- ✅ Updated `ctx3000.hpp` and `psr-ew425.hpp` to use shared constants
- ✅ Updated `play.cpp` to use namespaced constants
- ✅ Updated `options.hpp` to use shared `MICROSECONDS_PER_MINUTE`
- ✅ Eliminated magic numbers across device classes
- ✅ Improved code readability with meaningful constant names

This modular architecture transforms the monolithic 706-line file into manageable, focused components while maintaining clear constant ownership and following modern C++ best practices.