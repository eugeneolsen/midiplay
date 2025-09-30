# Decision Log

---
## Decision
*   [2025-09-30 19:21:42] Phase 2, Item 4 Complete: PlaybackEngine and TimingManager extraction with tempo override bugfix

## Rationale
*   Successfully completed the extraction of playback orchestration and timing management from play.cpp into dedicated modules. The extraction achieved all architectural goals: single responsibility, testability, maintainability, and reusability. The tempo override bug (--tempo command line option not affecting actual playback speed) was identified during testing, debugged, fixed, and validated. The PlaybackEngine now correctly applies tempo overrides by recalculating baseTempo_ after MidiLoader processes the override.

## Implementation Details
*   Created playback_engine.hpp/.cpp (258 lines) and timing_manager.hpp/.cpp (52 lines). Key achievements: (1) Reduced play.cpp from 172 to ~145 lines of pure orchestration. (2) Eliminated redundant parameters - PlaybackEngine receives only player, semaphore, and midiLoader. (3) Encapsulated all playback callbacks, intro/verse management, ritardando, and musical direction handling. (4) Fixed tempo override by ensuring PlaybackEngine.initialize() retrieves speed from MidiLoader after tempo override is applied. (5) All features tested and working: intro playback, multi-verse iteration, ritardando, D.C. al Fine, and tempo override.

---
## Decision
*   [2025-09-29 23:37:24] Phase 2, Item 4: Extracted PlaybackEngine and TimingManager modules from play.cpp

## Rationale
*   Completed the extraction of playback orchestration (243 lines) and timing management (52 lines) from play.cpp into separate, focused modules. This follows the architectural plan established in refactor.md Phase 2, achieving single responsibility principle and improving testability. The PlaybackEngine now handles all playback logic including callbacks, intro/verse management, ritardando, and musical direction markers. TimingManager encapsulates all time tracking and elapsed time display.

## Implementation Details
*   Created timing_manager.hpp/.cpp (simple, independent utility class) and playback_engine.hpp/.cpp (complex orchestrator). Key design decisions: (1) PlaybackEngine receives only player, semaphore, and midiLoader (removed redundant outport parameter since player contains it). (2) Speed parameter removed from initialize() - now retrieved from midiLoader which stores it from Options. (3) Playback info display moved into PlaybackEngine.displayPlaybackInfo() for better encapsulation. (4) Updated .vscode/tasks.json to include new source files. Main.cpp reduced from 172 to ~145 lines with cleaner orchestration-only logic.

---
## Decision
*   [2025-09-25 18:35:40] ConPort context synchronization - Product Context initialized and Active Context updated

## Rationale
*   The ConPort database had an empty Product Context despite the project being well-developed with clear architecture and goals. Updated Product Context with comprehensive project information from README.md and updated Active Context to reflect the completion of Phase 2, Item 3 (MIDI Loader extraction).

## Implementation Details
*   Populated Product Context with project description, technical stack, architecture details, supported devices, system requirements, and custom MIDI features. Updated Active Context to mark Phase 2 Items 1-3 as completed (Signal Handler, DeviceManager, MIDI Loader extractions) and identified next priority as playback engine extraction.

---
## Decision
*   [2025-09-22 16:06:30] Added exception handling for getFullPath() environment variable errors

## Rationale
*   The getFullPath() function throws std::runtime_error when required environment variables are not set. Previously, this would cause the application to crash ungracefully. Added try/catch block to handle these exceptions with proper error messaging and appropriate exit codes.

## Implementation Details
*   Wrapped getFullPath() call on line 146 of play.cpp with try/catch block catching std::runtime_error. Added new EXIT_ENVIRONMENT_ERROR = 3 constant to constants.hpp for environment-related errors, distinct from EXIT_FILE_NOT_FOUND. Exception handler outputs error message to console and exits gracefully.

---
## Decision
*   [2025-09-22 07:59:20] Deferred implementing full Dependency Inversion for the Options object.

## Rationale
*   The current Dependency Injection pattern is sufficient for now. The added complexity of an IOptions interface is not justified at this stage. This decision will be revisited when a unit testing framework is introduced, as the interface would be highly beneficial for mocking.

## Implementation Details
*   Development will proceed by passing the concrete Options class via constructors (Dependency Injection). No IOptions interface will be created at this time.

---
## Decision
*   [2025-09-17 17:32:31] Phase 2, Item 2 Complete: DeviceManager + YAML Configuration Architecture

## Rationale
*   Successfully extracted device management from play.cpp into a modular DeviceManager class with YAML configuration support. This eliminates hardcoded device detection and enables external device configuration without code changes. The YAML configuration system provides priority-based file discovery and complete device customization including detection strings, MIDI channels, and bank/program settings.

## Implementation Details
*   Created device_manager.hpp/.cpp with factory pattern, implemented YAML parsing with yaml-cpp library, added configuration file discovery following Debian standards (current dir → user config → system config), updated installer system to v1.5.0 with proper dependencies, achieved 100% independence from legacy device headers, and validated production-ready installer package.

---
## Decision
*   [2025-09-16 16:09:39] DeviceManager Implementation Successfully Tested and Verified

## Rationale
*   The DeviceManager extraction from play.cpp has been completed and thoroughly tested. Build compilation successful, smoke tests with two hymns passed, and device connections verified for all three supported device types (Casio CTX-3000, Yamaha PSR-EW425, Allen Protege). The extracted code maintains exact functional compatibility while improving modularity.

## Implementation Details
*   Created device_manager.hpp/.cpp with factory pattern implementation. Replaced 45 lines of inline device management code in play.cpp with clean DeviceManager interface. Uses existing device_constants.hpp and maintains backward compatibility. Architecture prepared for future YAML configuration support.

---
## Decision
*   [2025-09-16 15:45:35] DeviceManager Architecture Design - Factory Pattern with YAML Configuration Support

## Rationale
*   Extracted device management logic from play.cpp (lines 432-476) into a dedicated DeviceManager class using factory pattern. This separates device connection/detection logic from main application flow and prepares for future YAML configuration presets. Design maintains compatibility with existing ctx3000, psr_ew425, and protege device classes while leveraging device_constants.hpp.

## Implementation Details
*   DeviceManager class with DeviceInfo struct, DeviceType enum, and methods for connectAndDetectDevice(), createDevice() factory, and future loadDevicePresets(). Uses BaseDevice interface pattern for polymorphism. Integrates with existing device_constants.hpp namespaced constants.

---
## Decision
*   [2025-09-15 22:38:06] Successfully extracted signal handler functionality from play.cpp into separate module

## Rationale
*   Signal handling was deeply embedded in the monolithic play.cpp main function. Extracting it improves code organization, testability, and follows the migration strategy outlined in refactor.md Phase 2. This represents the first successful module extraction from the 700+ line monolithic file.

## Implementation Details
*   Created SignalHandler class with template constructor to handle MIDI output type compatibility. Encapsulates Ctrl+C signal handling, emergency MIDI notes-off, semaphore cleanup, and elapsed time display. Uses static instance pattern required by POSIX signal handling with inline static member. Original signal handling code (lines 108-149) removed from play.cpp and replaced with clean SignalHandler instantiation. Updated build configuration to compile both source files.

---
## Decision
*   [2025-09-02 18:58:59] Replaced all magic numbers in play.cpp with constexpr constants

## Rationale
*   Magic numbers make code harder to maintain and understand. Using named constexpr constants improves readability, maintainability, and follows modern C++ best practices. Also improved error handling by using distinct exit codes for different error conditions.

## Implementation Details
*   Added 17 constexpr constants at the top of play.cpp and replaced 19 magic number occurrences throughout the file. Reused MICROSECONDS_PER_MINUTE from options.hpp for consistency. Changed device timeout exit code from 2 to 6 to distinguish from file not found errors.
