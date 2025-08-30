# Product Context
## Project Name
Organ Pi MIDI File Player

## Goals
Create a software MIDI sequencer to play MIDI files through USB-to-MIDI converters to organs and keyboards, initially for Allen Protégé-16 on Raspberry Pi. Developed for people with technical aptitude, with goal for more user-friendly interface in future.

## Features
*   Simple Linux command line syntax
*   Define introduction using MIDI markers
*   Play configurable verses from MIDI or command line
*   Command line flags for prelude/postlude without introduction
*   Support for custom MIDI Meta events (0xFF7F) for verses and pauses
*   Version display on command line

## Architecture
C++ using gnu++20 standard, customized fork of cxxmidi library, standard library. Runs as Linux command line application with USB-to-MIDI converter.

## System Requirements
*   Raspberry Pi 4B or better (Orange Pi 5 tested)
*   64-bit Debian 12 (Bookworm) or later, 4GB RAM
*   NVME SSD recommended but fast SD card works
*   Allen Protégé-16 organ primarily, Casio/Yamaha keyboards recently added

## Current Architecture Issues
{'code_smells': ['God object: play.cpp main() function is 659 lines with multiple responsibilities', 'Excessive global variables: 15+ globals in play.cpp (outport, sem, timesig, etc.)', "Magic numbers/strings: Hardcoded constants throughout (timeout 300, meta events 0x10, strings like version '1.4.3')", "Namespace pollution: 'using namespace std;' and 'using namespace cxxmidi;' in headers", 'Code duplication: Similar SelectProgram/SetDefaults in device header classes', 'Poor synchronization: Old semaphore-based approach', 'Incomplete features: TODO comments for goto, channel override'], 'refactoring_priorities': ['High/Low: Remove namespace directives from headers, extract magic numbers/strings to constants, clean up dead code', 'Medium/Medium: Refactor main() into smaller functions (setupDevice, loadMidiFile, play...), create device config system, encapsulate globals', 'High/High: Extract callback handlers, implement configuration-driven device setup, eliminate duplication among device classes'], 'refactoring_phases': ['Phase 1 (1-2 days): Create constants.hpp/messages.hpp/device_config.hpp, clean headers (remove namespaces, dead code), add const', 'Phase 2 (3-5 days): Extract device setup logic (factory pattern), break down main(), create handler classes, encapsulate state', 'Phase 3 (5-7 days): External JSON/YAML config for devices, comprehensive error handling, complete missing features (goto/channel/stops)', 'Phase 4 (2-3 days): Unit tests, performance optimization, documentation update']}

## Files Overview
*   Headers: ctx3000.hpp (Casio CT-X3000), custommessage.hpp, options.hpp, protege.hpp (Allen Protégé), psr-ew425.hpp (Yamaha PSR-EW425), ticks.hpp
*   Source: play.cpp (main application)
*   Docs: README.md, refactor.md
*   Config: .gitignore, .vscode/launch.json
*   Version: 1.4.4

