# Decision Log

---
## Decision
*   [2025-10-16 22:06:34] Phase 3 Integration Testing: Real PlayerSync Instead of FakePlayerSync

## Rationale
*   Initially planned to create FakePlayerSync test double to avoid hardware dependencies. However, discovered that real cxxmidi::player::PlayerSync works perfectly for initialization and setup tests without requiring MIDI hardware. Using real PlayerSync avoids type compatibility issues (PlaybackOrchestrator requires cxxmidi::player::PlayerSync& reference) and maintains our 'real components' testing philosophy from Phase 2.

## Implementation Details
*   Implemented 10 integration test cases using real cxxmidi::player::PlayerSync and cxxmidi::output::Default. Tests verify: component initialization, callback setup, speed/tempo configuration, synchronization, signal handling, and end-to-end scenarios. All tests run fast (<1 second) without hardware. FakePlayerSync was not needed and was removed from the codebase.

---
## Decision
*   [2025-10-16 21:33:09] Phase 3 DeviceManager Testing: Real YAML Files Strategy

## Rationale
*   Following Phase 2's successful pattern of using real MIDI files, we implemented DeviceManager tests using real YAML configuration files rather than mocks. This approach provides authentic testing of yaml-cpp library integration, file I/O, and error handling. Created 6 test fixture YAML files covering valid configs, minimal configs, syntax errors, missing fields, empty files, and multi-device scenarios.

## Implementation Details
*   Created test/test_device_manager.cpp with 8 test cases (37 assertions) covering: construction, YAML file discovery, parsing valid/invalid configs, device type names, multi-device configs, and Options integration. All test fixtures in test/fixtures/test_configs/. Fixed path consistency issue in test/test_midi_loader.cpp (changed 'test/fixtures/' to 'fixtures/' for consistency). All 68 tests passing (285 assertions total).

---
## Decision
*   [2025-10-16 17:38:16] Created CLI11 migration plan to replace getopt()

## Rationale
*   To eliminate global state issues discovered during testing and modernize command-line parsing with type-safe, object-oriented approach. CLI11 is header-only, modern C++, and eliminates all optind reset requirements in tests.

## Implementation Details
*   Documented in CLI11_MIGRATION_PLAN.md: Complete 4-phase migration strategy covering preparation, implementation, testing, and integration. Includes detailed code examples for all command-line options, risk assessment, and 1-day timeline. Key benefit: Tests become simpler (no optind resets needed) and code becomes more maintainable.

---
## Decision
*   [2025-10-16 17:35:11] Analysis: Should we replace getopt() with a modern C++ command-line parser?

## Rationale
*   During Phase 2 testing, we discovered getopt() uses global state (optind, optarg, opterr) that pollutes test isolation. This requires manual resets (optind=0) before each test. Modern C++ libraries offer object-oriented, stateless alternatives that would eliminate these issues and provide better type safety, validation, and developer experience.

## Implementation Details
*   Evaluated alternatives:

**Modern C++ Options:**
1. **CLI11** - Header-only, modern C++11+, excellent type safety
2. **cxxopts** - Header-only, similar to getopt but object-oriented
3. **Boost.Program_options** - Mature, full-featured, requires Boost dependency
4. **argparse** (C++17) - Python-like API, intuitive

**getopt() Issues:**
- Global mutable state (optind, optarg, opterr, optopt)
- Requires manual state resets in tests
- Not type-safe (everything is char*)
- Limited validation capabilities
- GNU extensions (optind=0) vs POSIX (optind=1) inconsistencies

**Migration Effort:**
- Options class is well-encapsulated (single file)
- ~200 lines of parsing logic
- Would need to update tests but testing would be simpler
- No changes to dependent code (Options API stays same)

**Recommendation:** Worth considering but NOT urgent. Current solution works with proper test isolation. Could be a future refactoring when making other Options changes.

---
## Decision
*   [2025-10-16 16:53:43] Fixed test isolation issue by resetting getopt global state

## Rationale
*   Tests were failing randomly due to getopt()'s global optind variable persisting between test cases. When tests ran in random order, previous test's getopt state affected subsequent tests causing parse failures.

## Implementation Details
*   Added 'optind = 1;' at the start of each test SECTION to reset getopt's global state. This ensures each test starts with clean state regardless of execution order. Also included <getopt.h> header for the optind declaration.

---
## Decision
*   [2025-10-16 16:48:19] Fixed callback dangling reference bug in MidiLoader

## Rationale
*   The MidiLoader::initializeLoadCallback() captured Options by reference in a lambda, storing it in midiFile_. When Options went out of scope in tests, the callback retained a dangling reference causing segfaults. This was a critical memory safety issue that manifested randomly based on test execution order.

## Implementation Details
*   Applied multiple defensive fixes: (1) Clear callback in resetState(), (2) Clear callback immediately after midiFile_.Load(), (3) Clear callback in destructor, (4) Clear callback in all error paths, (5) Initialize callback only after file existence check. This ensures the callback never outlives the Options object it references.

---
## Decision
*   [2025-10-09 23:07:03] Options Class Refactoring - Eliminated all exit() and abort() calls for testability

## Rationale
*   The Options class was calling exit(1) in handleTempoOption() and abort() in the default case, making error paths untestable. This violated separation of concerns (Options both validates AND terminates) and prevented comprehensive unit testing. The refactoring allows Options to focus on validation while main() decides program fate.

## Implementation Details
*   1. Changed handleTempoOption() from void to return int error code (INVALID_TEMPO instead of exit(1))
2. Replaced abort() in parse() default case with return INVALID_OPTION
3. Added MidiPlay::OptionsParseResult namespace with clear error codes (SUCCESS=0, HELP_DISPLAYED=1, MISSING_FILENAME=2, INVALID_TEMPO=3, INVALID_OPTION=4, VERSION_DISPLAYED=-2)
4. Fixed critical resetGetopt() bug (was calling itself recursively) - changed to optind=0 for GNU complete reinitialization
5. Enabled 2 previously disabled tests (missing filename, invalid tempo)
6. Updated all 4 translation files (es, pt_BR, fr_FR, fr_CA) with corrected error messages
7. All 32 Options tests now pass (155 total assertions)
8. Maintained 100% backward compatibility - main() requires zero changes

---
## Decision
*   [2025-10-09 17:06:31] Replaced all magic numbers in play.cpp with constexpr constants

## Rationale
*   Magic numbers make code harder to maintain and understand. Using named constexpr constants improves readability, maintainability, and follows modern C++ best practices. Also improved error handling by using distinct exit codes for different error conditions.

## Implementation Details
*   Added 17 constexpr constants at the top of play.cpp and replaced 19 magic number occurrences throughout the file. Reused MICROSECONDS_PER_MINUTE from options.hpp for consistency. Changed device timeout exit code from 2 to 6 to distinguish from file not found errors.

## Rationale
*   Magic numbers make code harder to maintain and understand. Using named constexpr constants improves readability, maintainability, and follows modern C++ best practices. Also improved error handling by using distinct exit codes for different error conditions.

## Implementation Details
Added 17 constexpr constants at the top of play.cpp and replaced 19 magic number occurrences throughout the file. Reused MICROSECONDS_PER_MINUTE from options.hpp for consistency. Changed device timeout exit code from 2 to 6 to distinguish from file not found errors.

## Implementation Details
*   Added 17 constexpr constants at the top of play.cpp and replaced 19 magic number occurrences throughout the file. Reused MICROSECONDS_PER_MINUTE from options.hpp for consistency. Changed device timeout exit code from 2 to 6 to distinguish from file not found errors.

---
## Decision
*   [2025-10-09 17:06:30] Phase 2, Item 2 Complete: DeviceManager + YAML Configuration Architecture

## Rationale
*   Successfully extracted device management from play.cpp into a modular DeviceManager class with YAML configuration support. This eliminates hardcoded device detection and enables external device configuration without code changes. The YAML configuration system provides priority-based file discovery and complete device customization including detection strings, MIDI channels, and bank/program settings.

## Implementation Details
*   Created device_manager.hpp/.cpp with factory pattern, implemented YAML parsing with yaml-cpp library, added configuration file discovery following Debian standards (current dir → user config → system config), updated installer system to v1.5.0 with proper dependencies, achieved 100% independence from legacy device headers, and validated production-ready installer package.

## Rationale
*   Successfully extracted device management from play.cpp into a modular DeviceManager class with YAML configuration support. This eliminates hardcoded device detection and enables external device configuration without code changes. The YAML configuration system provides priority-based file discovery and complete device customization including detection strings, MIDI channels, and bank/program settings.

## Implementation Details
Created device_manager.hpp/.cpp with factory pattern, implemented YAML parsing with yaml-cpp library, added configuration file discovery following Debian standards (current dir → user config → system config), updated installer system to v1.5.0 with proper dependencies, achieved 100% independence from legacy device headers, and validated production-ready installer package.

## Implementation Details
*   Created device_manager.hpp/.cpp with factory pattern, implemented YAML parsing with yaml-cpp library, added configuration file discovery following Debian standards (current dir → user config → system config), updated installer system to v1.5.0 with proper dependencies, achieved 100% independence from legacy device headers, and validated production-ready installer package.

---
## Decision
*   [2025-10-09 17:06:30] DeviceManager Implementation Successfully Tested and Verified

## Rationale
*   The DeviceManager extraction from play.cpp has been completed and thoroughly tested. Build compilation successful, smoke tests with two hymns passed, and device connections verified for all three supported device types (Casio CTX-3000, Yamaha PSR-EW425, Allen Protege). The extracted code maintains exact functional compatibility while improving modularity.

## Implementation Details
*   Created device_manager.hpp/.cpp with factory pattern implementation. Replaced 45 lines of inline device management code in play.cpp with clean DeviceManager interface. Uses existing device_constants.hpp and maintains backward compatibility. Architecture prepared for future YAML configuration support.

## Rationale
*   The DeviceManager extraction from play.cpp has been completed and thoroughly tested. Build compilation successful, smoke tests with two hymns passed, and device connections verified for all three supported device types (Casio CTX-3000, Yamaha PSR-EW425, Allen Protege). The extracted code maintains exact functional compatibility while improving modularity.

## Implementation Details
Created device_manager.hpp/.cpp with factory pattern implementation. Replaced 45 lines of inline device management code in play.cpp with clean DeviceManager interface. Uses existing device_constants.hpp and maintains backward compatibility. Architecture prepared for future YAML configuration support.

## Implementation Details
*   Created device_manager.hpp/.cpp with factory pattern implementation. Replaced 45 lines of inline device management code in play.cpp with clean DeviceManager interface. Uses existing device_constants.hpp and maintains backward compatibility. Architecture prepared for future YAML configuration support.

---
## Decision
*   [2025-10-09 17:06:30] DeviceManager Architecture Design - Factory Pattern with YAML Configuration Support

## Rationale
*   Extracted device management logic from play.cpp (lines 432-476) into a dedicated DeviceManager class using factory pattern. This separates device connection/detection logic from main application flow and prepares for future YAML configuration presets. Design maintains compatibility with existing ctx3000, psr_ew425, and protege device classes while leveraging device_constants.hpp.

## Implementation Details
*   DeviceManager class with DeviceInfo struct, DeviceType enum, and methods for connectAndDetectDevice(), createDevice() factory, and future loadDevicePresets(). Uses BaseDevice interface pattern for polymorphism. Integrates with existing device_constants.hpp namespaced constants.

## Rationale
*   Extracted device management logic from play.cpp (lines 432-476) into a dedicated DeviceManager class using factory pattern. This separates device connection/detection logic from main application flow and prepares for future YAML configuration presets. Design maintains compatibility with existing ctx3000, psr_ew425, and protege device classes while leveraging device_constants.hpp.

## Implementation Details
DeviceManager class with DeviceInfo struct, DeviceType enum, and methods for connectAndDetectDevice(), createDevice() factory, and future loadDevicePresets(). Uses BaseDevice interface pattern for polymorphism. Integrates with existing device_constants.hpp namespaced constants.

## Implementation Details
*   DeviceManager class with DeviceInfo struct, DeviceType enum, and methods for connectAndDetectDevice(), createDevice() factory, and future loadDevicePresets(). Uses BaseDevice interface pattern for polymorphism. Integrates with existing device_constants.hpp namespaced constants.

---
## Decision
*   [2025-10-09 17:06:30] Successfully extracted signal handler functionality from play.cpp into separate module

## Rationale
*   Signal handling was deeply embedded in the monolithic play.cpp main function. Extracting it improves code organization, testability, and follows the migration strategy outlined in refactor.md Phase 2. This represents the first successful module extraction from the 700+ line monolithic file.

## Implementation Details
*   Created SignalHandler class with template constructor to handle MIDI output type compatibility. Encapsulates Ctrl+C signal handling, emergency MIDI notes-off, semaphore cleanup, and elapsed time display. Uses static instance pattern required by POSIX signal handling with inline static member. Original signal handling code (lines 108-149) removed from play.cpp and replaced with clean SignalHandler instantiation. Updated build configuration to compile both source files.

## Rationale
*   Signal handling was deeply embedded in the monolithic play.cpp main function. Extracting it improves code organization, testability, and follows the migration strategy outlined in refactor.md Phase 2. This represents the first successful module extraction from the 700+ line monolithic file.

## Implementation Details
Created SignalHandler class with template constructor to handle MIDI output type compatibility. Encapsulates Ctrl+C signal handling, emergency MIDI notes-off, semaphore cleanup, and elapsed time display. Uses static instance pattern required by POSIX signal handling with inline static member. Original signal handling code (lines 108-149) removed from play.cpp and replaced with clean SignalHandler instantiation. Updated build configuration to compile both source files.

## Implementation Details
*   Created SignalHandler class with template constructor to handle MIDI output type compatibility. Encapsulates Ctrl+C signal handling, emergency MIDI notes-off, semaphore cleanup, and elapsed time display. Uses static instance pattern required by POSIX signal handling with inline static member. Original signal handling code (lines 108-149) removed from play.cpp and replaced with clean SignalHandler instantiation. Updated build configuration to compile both source files.

---
## Decision
*   [2025-10-09 17:06:29] Phase 2, Item 4: Extracted PlaybackEngine and TimingManager modules from play.cpp

## Rationale
*   Completed the extraction of playback orchestration (243 lines) and timing management (52 lines) from play.cpp into separate, focused modules. This follows the architectural plan established in refactor.md Phase 2, achieving single responsibility principle and improving testability. The PlaybackEngine now handles all playback logic including callbacks, intro/verse management, ritardando, and musical direction markers. TimingManager encapsulates all time tracking and elapsed time display.

## Implementation Details
*   Created timing_manager.hpp/.cpp (simple, independent utility class) and playback_engine.hpp/.cpp (complex orchestrator). Key design decisions: (1) PlaybackEngine receives only player, semaphore, and midiLoader (removed redundant outport parameter since player contains it). (2) Speed parameter removed from initialize() - now retrieved from midiLoader which stores it from Options. (3) Playback info display moved into PlaybackEngine.displayPlaybackInfo() for better encapsulation. (4) Updated .vscode/tasks.json to include new source files. Main.cpp reduced from 172 to ~145 lines with cleaner orchestration-only logic.

## Rationale
*   Completed the extraction of playback orchestration (243 lines) and timing management (52 lines) from play.cpp into separate, focused modules. This follows the architectural plan established in refactor.md Phase 2, achieving single responsibility principle and improving testability. The PlaybackEngine now handles all playback logic including callbacks, intro/verse management, ritardando, and musical direction markers. TimingManager encapsulates all time tracking and elapsed time display.

## Implementation Details
Created timing_manager.hpp/.cpp (simple, independent utility class) and playback_engine.hpp/.cpp (complex orchestrator). Key design decisions: (1) PlaybackEngine receives only player, semaphore, and midiLoader (removed redundant outport parameter since player contains it). (2) Speed parameter removed from initialize() - now retrieved from midiLoader which stores it from Options. (3) Playback info display moved into PlaybackEngine.displayPlaybackInfo() for better encapsulation. (4) Updated .vscode/tasks.json to include new source files. Main.cpp reduced from 172 to ~145 lines with cleaner orchestration-only logic.

## Implementation Details
*   Created timing_manager.hpp/.cpp (simple, independent utility class) and playback_engine.hpp/.cpp (complex orchestrator). Key design decisions: (1) PlaybackEngine receives only player, semaphore, and midiLoader (removed redundant outport parameter since player contains it). (2) Speed parameter removed from initialize() - now retrieved from midiLoader which stores it from Options. (3) Playback info display moved into PlaybackEngine.displayPlaybackInfo() for better encapsulation. (4) Updated .vscode/tasks.json to include new source files. Main.cpp reduced from 172 to ~145 lines with cleaner orchestration-only logic.

---
## Decision
*   [2025-10-09 17:06:29] ConPort context synchronization - Product Context initialized and Active Context updated

## Rationale
*   The ConPort database had an empty Product Context despite the project being well-developed with clear architecture and goals. Updated Product Context with comprehensive project information from README.md and updated Active Context to reflect the completion of Phase 2, Item 3 (MIDI Loader extraction).

## Implementation Details
*   Populated Product Context with project description, technical stack, architecture details, supported devices, system requirements, and custom MIDI features. Updated Active Context to mark Phase 2 Items 1-3 as completed (Signal Handler, DeviceManager, MIDI Loader extractions) and identified next priority as playback engine extraction.

## Rationale
*   The ConPort database had an empty Product Context despite the project being well-developed with clear architecture and goals. Updated Product Context with comprehensive project information from README.md and updated Active Context to reflect the completion of Phase 2, Item 3 (MIDI Loader extraction).

## Implementation Details
Populated Product Context with project description, technical stack, architecture details, supported devices, system requirements, and custom MIDI features. Updated Active Context to mark Phase 2 Items 1-3 as completed (Signal Handler, DeviceManager, MIDI Loader extractions) and identified next priority as playback engine extraction.

## Implementation Details
*   Populated Product Context with project description, technical stack, architecture details, supported devices, system requirements, and custom MIDI features. Updated Active Context to mark Phase 2 Items 1-3 as completed (Signal Handler, DeviceManager, MIDI Loader extractions) and identified next priority as playback engine extraction.

---
## Decision
*   [2025-10-09 17:06:29] Added exception handling for getFullPath() environment variable errors

## Rationale
*   The getFullPath() function throws std::runtime_error when required environment variables are not set. Previously, this would cause the application to crash ungracefully. Added try/catch block to handle these exceptions with proper error messaging and appropriate exit codes.

## Implementation Details
*   Wrapped getFullPath() call on line 146 of play.cpp with try/catch block catching std::runtime_error. Added new EXIT_ENVIRONMENT_ERROR = 3 constant to constants.hpp for environment-related errors, distinct from EXIT_FILE_NOT_FOUND. Exception handler outputs error message to console and exits gracefully.

## Rationale
*   The getFullPath() function throws std::runtime_error when required environment variables are not set. Previously, this would cause the application to crash ungracefully. Added try/catch block to handle these exceptions with proper error messaging and appropriate exit codes.

## Implementation Details
Wrapped getFullPath() call on line 146 of play.cpp with try/catch block catching std::runtime_error. Added new EXIT_ENVIRONMENT_ERROR = 3 constant to constants.hpp for environment-related errors, distinct from EXIT_FILE_NOT_FOUND. Exception handler outputs error message to console and exits gracefully.

## Implementation Details
*   Wrapped getFullPath() call on line 146 of play.cpp with try/catch block catching std::runtime_error. Added new EXIT_ENVIRONMENT_ERROR = 3 constant to constants.hpp for environment-related errors, distinct from EXIT_FILE_NOT_FOUND. Exception handler outputs error message to console and exits gracefully.

---
## Decision
*   [2025-10-09 17:06:29] Deferred implementing full Dependency Inversion for the Options object.

## Rationale
*   The current Dependency Injection pattern is sufficient for now. The added complexity of an IOptions interface is not justified at this stage. This decision will be revisited when a unit testing framework is introduced, as the interface would be highly beneficial for mocking.

## Implementation Details
*   Development will proceed by passing the concrete Options class via constructors (Dependency Injection). No IOptions interface will be created at this time.

## Rationale
*   The current Dependency Injection pattern is sufficient for now. The added complexity of an IOptions interface is not justified at this stage. This decision will be revisited when a unit testing framework is introduced, as the interface would be highly beneficial for mocking.

## Implementation Details
Development will proceed by passing the concrete Options class via constructors (Dependency Injection). No IOptions interface will be created at this time.

## Implementation Details
*   Development will proceed by passing the concrete Options class via constructors (Dependency Injection). No IOptions interface will be created at this time.

---
## Decision
*   [2025-10-09 17:06:28] Implemented --verbose (-V) command-line flag for enhanced debugging output

## Rationale
*   Adding a verbose flag provides users and developers with optional detailed output about MIDI port detection, device connection, and other internal operations. This aids in debugging and troubleshooting without cluttering normal output.

## Implementation Details
*   Added private bool `_verbose` member to Options class (line 47), added public getter `isVerbose()` (lines 91-93), added -V flag to getopt_long call (line 117), and implemented case 'V' handler (lines 206-208). Verbose output now displays detected MIDI ports (lines 87-96) and connected device information (lines 111-115) in play.cpp when enabled.

## Rationale
*   Adding a verbose flag provides users and developers with optional detailed output about MIDI port detection, device connection, and other internal operations. This aids in debugging and troubleshooting without cluttering normal output.

## Implementation Details
Added private bool `_verbose` member to Options class (line 47), added public getter `isVerbose()` (lines 91-93), added -V flag to getopt_long call (line 117), and implemented case 'V' handler (lines 206-208). Verbose output now displays detected MIDI ports (lines 87-96) and connected device information (lines 111-115) in play.cpp when enabled.

## Implementation Details
*   Added private bool `_verbose` member to Options class (line 47), added public getter `isVerbose()` (lines 91-93), added -V flag to getopt_long call (line 117), and implemented case 'V' handler (lines 206-208). Verbose output now displays detected MIDI ports (lines 87-96) and connected device information (lines 111-115) in play.cpp when enabled.

---
## Decision
*   [2025-10-09 17:06:28] Moved displayWarnings flag to Options object for better configuration management

## Rationale
*   The displayWarnings flag is a command-line configurable option that belongs with other user preferences in the Options class. This improves encapsulation by keeping all command-line options together and makes the option more discoverable and maintainable.

## Implementation Details
*   Added private bool `_displayWarnings` member to Options class (line 48), added public getter `isDisplayWarnings()` (lines 95-97), and added -W flag handling in parse() method (lines 209-211). PlaybackEngine now retrieves this setting via `options.isDisplayWarnings()` at line 133 in play.cpp.

## Rationale
*   The displayWarnings flag is a command-line configurable option that belongs with other user preferences in the Options class. This improves encapsulation by keeping all command-line options together and makes the option more discoverable and maintainable.

## Implementation Details
Added private bool `_displayWarnings` member to Options class (line 48), added public getter `isDisplayWarnings()` (lines 95-97), and added -W flag handling in parse() method (lines 209-211). PlaybackEngine now retrieves this setting via `options.isDisplayWarnings()` at line 133 in play.cpp.

## Implementation Details
*   Added private bool `_displayWarnings` member to Options class (line 48), added public getter `isDisplayWarnings()` (lines 95-97), and added -W flag handling in parse() method (lines 209-211). PlaybackEngine now retrieves this setting via `options.isDisplayWarnings()` at line 133 in play.cpp.

---
## Decision
*   [2025-10-09 17:06:28] Refactored global outport to local scope in main() for better encapsulation

## Rationale
*   Moving outport from global to local scope in main() improves code organization, reduces global state, and follows best practices for resource management. This change supports the Phase 3 goal of cleaning up main() and improving overall architecture.

## Implementation Details
*   Changed `output::Default outport;` from global declaration (line 35 area) to local variable declaration inside main() at line 83. All references to outport remain unchanged as they already accessed it through function parameters or local scope.

## Rationale
*   Moving outport from global to local scope in main() improves code organization, reduces global state, and follows best practices for resource management. This change supports the Phase 3 goal of cleaning up main() and improving overall architecture.

## Implementation Details
Changed `output::Default outport;` from global declaration (line 35 area) to local variable declaration inside main() at line 83. All references to outport remain unchanged as they already accessed it through function parameters or local scope.

## Implementation Details
*   Changed `output::Default outport;` from global declaration (line 35 area) to local variable declaration inside main() at line 83. All references to outport remain unchanged as they already accessed it through function parameters or local scope.

---
## Decision
*   [2025-10-09 17:06:28] Phase 2, Item 4 Complete: PlaybackEngine and TimingManager extraction with tempo override bugfix

## Rationale
*   Successfully completed the extraction of playback orchestration and timing management from play.cpp into dedicated modules. The extraction achieved all architectural goals: single responsibility, testability, maintainability, and reusability. The tempo override bug (--tempo command line option not affecting actual playback speed) was identified during testing, debugged, fixed, and validated. The PlaybackEngine now correctly applies tempo overrides by recalculating baseTempo_ after MidiLoader processes the override.

## Implementation Details
*   Created playback_engine.hpp/.cpp (258 lines) and timing_manager.hpp/.cpp (52 lines). Key achievements: (1) Reduced play.cpp from 172 to ~145 lines of pure orchestration. (2) Eliminated redundant parameters - PlaybackEngine receives only player, semaphore, and midiLoader. (3) Encapsulated all playback callbacks, intro/verse management, ritardando, and musical direction handling. (4) Fixed tempo override by ensuring PlaybackEngine.initialize() retrieves speed from MidiLoader after tempo override is applied. (5) All features tested and working: intro playback, multi-verse iteration, ritardando, D.C. al Fine, and tempo override.

## Rationale
*   Successfully completed the extraction of playback orchestration and timing management from play.cpp into dedicated modules. The extraction achieved all architectural goals: single responsibility, testability, maintainability, and reusability. The tempo override bug (--tempo command line option not affecting actual playback speed) was identified during testing, debugged, fixed, and validated. The PlaybackEngine now correctly applies tempo overrides by recalculating baseTempo_ after MidiLoader processes the override.

## Implementation Details
Created playback_engine.hpp/.cpp (258 lines) and timing_manager.hpp/.cpp (52 lines). Key achievements: (1) Reduced play.cpp from 172 to ~145 lines of pure orchestration. (2) Eliminated redundant parameters - PlaybackEngine receives only player, semaphore, and midiLoader. (3) Encapsulated all playback callbacks, intro/verse management, ritardando, and musical direction handling. (4) Fixed tempo override by ensuring PlaybackEngine.initialize() retrieves speed from MidiLoader after tempo override is applied. (5) All features tested and working: intro playback, multi-verse iteration, ritardando, D.C. al Fine, and tempo override.

## Implementation Details
*   Created playback_engine.hpp/.cpp (258 lines) and timing_manager.hpp/.cpp (52 lines). Key achievements: (1) Reduced play.cpp from 172 to ~145 lines of pure orchestration. (2) Eliminated redundant parameters - PlaybackEngine receives only player, semaphore, and midiLoader. (3) Encapsulated all playback callbacks, intro/verse management, ritardando, and musical direction handling. (4) Fixed tempo override by ensuring PlaybackEngine.initialize() retrieves speed from MidiLoader after tempo override is applied. (5) All features tested and working: intro playback, multi-verse iteration, ritardando, D.C. al Fine, and tempo override.

---
## Decision
*   [2025-10-09 17:06:27] Fixed playIntro bug: -p and -x flags were playing introduction when they shouldn't

## Rationale
*   The MidiLoader was setting playIntro based solely on whether intro markers existed in the MIDI file, completely ignoring the command-line option. The -p (prelude) and -x (verses without intro) flags correctly set _playIntro = false in Options, but MidiLoader never retrieved this value, causing introductions to play even when explicitly disabled.

## Implementation Details
*   Added one line in midi_loader.cpp:63 to retrieve playIntro from options: `playIntro_ = options.isPlayIntro();`. Simplified finalizeLoading() method (lines 383-396) to only override playIntro_ if MIDI file has no intro markers (can't play intro if markers don't exist), otherwise retaining the value from command-line options. This ensures proper priority: command-line option first, then file capability check.

## Rationale
*   The MidiLoader was setting playIntro based solely on whether intro markers existed in the MIDI file, completely ignoring the command-line option. The -p (prelude) and -x (verses without intro) flags correctly set _playIntro = false in Options, but MidiLoader never retrieved this value, causing introductions to play even when explicitly disabled.

## Implementation Details
Added one line in midi_loader.cpp:63 to retrieve playIntro from options: `playIntro_ = options.isPlayIntro();`. Simplified finalizeLoading() method (lines 383-396) to only override playIntro_ if MIDI file has no intro markers (can't play intro if markers don't exist), otherwise retaining the value from command-line options. This ensures proper priority: command-line option first, then file capability check.

## Implementation Details
*   Added one line in midi_loader.cpp:63 to retrieve playIntro from options: `playIntro_ = options.isPlayIntro();`. Simplified finalizeLoading() method (lines 383-396) to only override playIntro_ if MIDI file has no intro markers (can't play intro if markers don't exist), otherwise retaining the value from command-line options. This ensures proper priority: command-line option first, then file capability check.

---
## Decision
*   [2025-10-09 17:06:27] Fixed verse count override bug: -n and -x flags were being ignored

## Rationale
*   The MidiLoader was retrieving speed and tempo overrides from Options but never retrieved the verse count. This caused command-line verse overrides (-n and -x flags) to be completely ignored, with the program always using MIDI file meta-events or default value instead.

## Implementation Details
*   Added single line in midi_loader.cpp:60 to retrieve verses from options: `verses_ = options.getVerses();`. This follows the same pattern as speed (line 58) and tempo override (line 62). The fix ensures command-line verse overrides take priority over MIDI file meta-events and default values, restoring intended behavior.

## Rationale
*   The MidiLoader was retrieving speed and tempo overrides from Options but never retrieved the verse count. This caused command-line verse overrides (-n and -x flags) to be completely ignored, with the program always using MIDI file meta-events or default value instead.

## Implementation Details
Added single line in midi_loader.cpp:60 to retrieve verses from options: `verses_ = options.getVerses();`. This follows the same pattern as speed (line 58) and tempo override (line 62). The fix ensures command-line verse overrides take priority over MIDI file meta-events and default values, restoring intended behavior.

## Implementation Details
*   Added single line in midi_loader.cpp:60 to retrieve verses from options: `verses_ = options.getVerses();`. This follows the same pattern as speed (line 58) and tempo override (line 62). The fix ensures command-line verse overrides take priority over MIDI file meta-events and default values, restoring intended behavior.

---
## Decision
*   [2025-10-09 17:06:27] Modernized synchronization from POSIX semaphores to standard C++ primitives

## Rationale
*   Replaced POSIX semaphore (sem_t) with std::condition_variable and std::mutex to achieve: (1) Elimination of the last global variable in play.cpp, (2) Better portability using standard C++ instead of POSIX-specific APIs, (3) Exception safety through RAII principles, (4) More idiomatic modern C++ code. This change completes Phase 3's goal of eliminating all global variables (except static version string).

## Implementation Details
*   Created PlaybackSynchronizer class with wait() and notify() methods that wrap std::condition_variable and std::mutex. Updated PlaybackEngine, SignalHandler, and play.cpp to use PlaybackSynchronizer& instead of sem_t&. Removed all POSIX semaphore includes and calls (sem_init, sem_wait, sem_post, sem_destroy). The PlaybackSynchronizer provides automatic reset after wait() for convenient use in multi-verse playback loops. All cleanup happens automatically via RAII destructors. Updated .vscode/tasks.json to include playback_synchronizer.cpp in build.

## Rationale
*   Replaced POSIX semaphore (sem_t) with std::condition_variable and std::mutex to achieve: (1) Elimination of the last global variable in play.cpp, (2) Better portability using standard C++ instead of POSIX-specific APIs, (3) Exception safety through RAII principles, (4) More idiomatic modern C++ code. This change completes Phase 3's goal of eliminating all global variables (except static version string).

## Implementation Details
Created PlaybackSynchronizer class with wait() and notify() methods that wrap std::condition_variable and std::mutex. Updated PlaybackEngine, SignalHandler, and play.cpp to use PlaybackSynchronizer& instead of sem_t&. Removed all POSIX semaphore includes and calls (sem_init, sem_wait, sem_post, sem_destroy). The PlaybackSynchronizer provides automatic reset after wait() for convenient use in multi-verse playback loops. All cleanup happens automatically via RAII destructors. Updated .vscode/tasks.json to include playback_synchronizer.cpp in build.

## Implementation Details
*   Created PlaybackSynchronizer class with wait() and notify() methods that wrap std::condition_variable and std::mutex. Updated PlaybackEngine, SignalHandler, and play.cpp to use PlaybackSynchronizer& instead of sem_t&. Removed all POSIX semaphore includes and calls (sem_init, sem_wait, sem_post, sem_destroy). The PlaybackSynchronizer provides automatic reset after wait() for convenient use in multi-verse playback loops. All cleanup happens automatically via RAII destructors. Updated .vscode/tasks.json to include playback_synchronizer.cpp in build.

---
## Decision
*   [2025-10-09 17:06:26] ThreadSanitizer ARM64 VMA Range Error Analysis and Solutions

## Rationale
*   Analyzed ThreadSanitizer VMA range error on ARM64 system (47-bit VMA not supported, only 39/42/48). Created comprehensive documentation with multiple solution approaches including AddressSanitizer alternative.

## Implementation Details
*   Created THREAD_SANITIZER_FIXES.md, TSAN_ADVANCED_SOLUTIONS.md, and VSCODE_DEFAULT_TASK_GUIDE.md. Recommended using AddressSanitizer (-fsanitize=address,undefined,leak) as primary solution due to full ARM64 support.

## Rationale
*   Analyzed ThreadSanitizer VMA range error on ARM64 system (47-bit VMA not supported, only 39/42/48). Created comprehensive documentation with multiple solution approaches including AddressSanitizer alternative.

## Implementation Details
Created THREAD_SANITIZER_FIXES.md, TSAN_ADVANCED_SOLUTIONS.md, and VSCODE_DEFAULT_TASK_GUIDE.md. Recommended using AddressSanitizer (-fsanitize=address,undefined,leak) as primary solution due to full ARM64 support.

## Implementation Details
*   Created THREAD_SANITIZER_FIXES.md, TSAN_ADVANCED_SOLUTIONS.md, and VSCODE_DEFAULT_TASK_GUIDE.md. Recommended using AddressSanitizer (-fsanitize=address,undefined,leak) as primary solution due to full ARM64 support.

---
## Decision
*   [2025-10-09 17:06:26] Implemented centralized internationalization with proper plural handling

## Rationale
*   The original code had internationalization macros scattered across modules and used manual pluralization (adding 's') which doesn't work for all languages. We centralized all i18n functionality in i18n.hpp and implemented proper ngettext-based plural handling.

## Implementation Details
*   Created i18n.hpp with centralized macros, updated all modules to use it, fixed plural case in playback_engine.cpp, updated all .po files with proper plural forms, and tested with multiple locales.

## Rationale
*   The original code had internationalization macros scattered across modules and used manual pluralization (adding 's') which doesn't work for all languages. We centralized all i18n functionality in i18n.hpp and implemented proper ngettext-based plural handling.

## Implementation Details
Created i18n.hpp with centralized macros, updated all modules to use it, fixed plural case in playback_engine.cpp, updated all .po files with proper plural forms, and tested with multiple locales.

## Implementation Details
*   Created i18n.hpp with centralized macros, updated all modules to use it, fixed plural case in playback_engine.cpp, updated all .po files with proper plural forms, and tested with multiple locales.

---
## Decision
*   [2025-10-09 17:06:26] Completed Phase 3: Production-Ready i18n Build Integration

## Rationale
*   Integrated translation compilation into build system and installer package workflow. This ensures translations are automatically compiled during development and properly distributed in production packages, with no manual intervention required.

## Implementation Details
*   1. Created po/compile-translations.sh for automated compilation of all 4 languages (es, pt_BR, fr_FR, fr_CA)
2. Updated update-installer-package.sh to compile translations and copy .mo files to installer structure
3. Modified .vscode/tasks.json to compile translations before C++ build (dependsOn)
4. Enhanced install.sh to verify and report installed translation files
5. Created comprehensive po/README.md with plural forms documentation
6. Fixed arithmetic increment issues with set -e (bash strict mode)
7. Updated version to 1.5.7 across all installer files
8. Successfully tested complete workflow: compile → package → verified 4 translations in installer

## Rationale
*   Integrated translation compilation into build system and installer package workflow. This ensures translations are automatically compiled during development and properly distributed in production packages, with no manual intervention required.

## Implementation Details
1. Created po/compile-translations.sh for automated compilation of all 4 languages (es, pt_BR, fr_FR, fr_CA)
2. Updated update-installer-package.sh to compile translations and copy .mo files to installer structure
3. Modified .vscode/tasks.json to compile translations before C++ build (dependsOn)
4. Enhanced install.sh to verify and report installed translation files
5. Created comprehensive po/README.md with plural forms documentation
6. Fixed arithmetic increment issues with set -e (bash strict mode)
7. Updated version to 1.5.7 across all installer files
8. Successfully tested complete workflow: compile → package → verified 4 translations in installer

## Implementation Details
*   1. Created po/compile-translations.sh for automated compilation of all 4 languages (es, pt_BR, fr_FR, fr_CA)
2. Updated update-installer-package.sh to compile translations and copy .mo files to installer structure
3. Modified .vscode/tasks.json to compile translations before C++ build (dependsOn)
4. Enhanced install.sh to verify and report installed translation files
5. Created comprehensive po/README.md with plural forms documentation
6. Fixed arithmetic increment issues with set -e (bash strict mode)
7. Updated version to 1.5.7 across all installer files
8. Successfully tested complete workflow: compile → package → verified 4 translations in installer

---
## Decision
*   [2025-10-09 17:06:26] Adopt GNU gettext for Internationalization (i18n)

## Rationale
*   Selected GNU gettext as the standard, robust, and scalable solution for internationalization over simpler methods like a custom messages.hpp or a YAML-based system. The long-term benefits of its standard tooling and workflow outweigh the moderate initial setup cost.

## Implementation Details
*   Implementation will be done in three incremental phases: 1) Proof of Concept, 2) Full Code Refactoring, 3) Build System Integration. The existing .vscode/tasks.json build system will be retained, augmented with helper shell scripts for the gettext workflow, and the compiler will be linked with '-lintl'.

## Rationale
*   Selected GNU gettext as the standard, robust, and scalable solution for internationalization over simpler methods like a custom messages.hpp or a YAML-based system. The long-term benefits of its standard tooling and workflow outweigh the moderate initial setup cost.

## Implementation Details
Implementation will be done in three incremental phases: 1) Proof of Concept, 2) Full Code Refactoring, 3) Build System Integration. The existing .vscode/tasks.json build system will be retained, augmented with helper shell scripts for the gettext workflow, and the compiler will be linked with '-lintl'.

## Implementation Details
*   Implementation will be done in three incremental phases: 1) Proof of Concept, 2) Full Code Refactoring, 3) Build System Integration. The existing .vscode/tasks.json build system will be retained, augmented with helper shell scripts for the gettext workflow, and the compiler will be linked with '-lintl'.

---
## Decision
*   [2025-10-09 17:06:26] Synchronization Modernization - Replaced POSIX semaphores with C++ std::condition_variable

## Rationale
*   The codebase used POSIX semaphores (sem_t) for playback synchronization, which had several issues: (1) Global variable sem_t sem violated encapsulation, (2) POSIX-specific code reduced portability to non-POSIX platforms like Windows, (3) Manual initialization/cleanup (sem_init/sem_destroy) was error-prone and not exception-safe, (4) C-style API was not idiomatic modern C++. Modernizing to std::condition_variable addresses all these issues while maintaining identical functionality and performance.

## Implementation Details
*   Created PlaybackSynchronizer class encapsulating std::mutex and std::condition_variable with a boolean flag. The class provides wait() and notify() methods that replace sem_wait() and sem_post() respectively. Key implementation details: (1) RAII-based design with automatic cleanup, (2) Automatic reset of finished_ flag after wait() completes for seamless multi-cycle usage, (3) Notify outside lock scope for optimal performance, (4) Delete copy/move constructors to prevent synchronization issues. Updated PlaybackEngine and SignalHandler to accept PlaybackSynchronizer& instead of sem_t&. Updated play.cpp to create local PlaybackSynchronizer instance instead of global sem_t. This change naturally eliminated the last remaining global variable in play.cpp.

## Rationale
*   The codebase used POSIX semaphores (sem_t) for playback synchronization, which had several issues: (1) Global variable sem_t sem violated encapsulation, (2) POSIX-specific code reduced portability to non-POSIX platforms like Windows, (3) Manual initialization/cleanup (sem_init/sem_destroy) was error-prone and not exception-safe, (4) C-style API was not idiomatic modern C++. Modernizing to std::condition_variable addresses all these issues while maintaining identical functionality and performance.

## Implementation Details
Created PlaybackSynchronizer class encapsulating std::mutex and std::condition_variable with a boolean flag. The class provides wait() and notify() methods that replace sem_wait() and sem_post() respectively. Key implementation details: (1) RAII-based design with automatic cleanup, (2) Automatic reset of finished_ flag after wait() completes for seamless multi-cycle usage, (3) Notify outside lock scope for optimal performance, (4) Delete copy/move constructors to prevent synchronization issues. Updated PlaybackEngine and SignalHandler to accept PlaybackSynchronizer& instead of sem_t&. Updated play.cpp to create local PlaybackSynchronizer instance instead of global sem_t. This change naturally eliminated the last remaining global variable in play.cpp.

## Implementation Details
*   Created PlaybackSynchronizer class encapsulating std::mutex and std::condition_variable with a boolean flag. The class provides wait() and notify() methods that replace sem_wait() and sem_post() respectively. Key implementation details: (1) RAII-based design with automatic cleanup, (2) Automatic reset of finished_ flag after wait() completes for seamless multi-cycle usage, (3) Notify outside lock scope for optimal performance, (4) Delete copy/move constructors to prevent synchronization issues. Updated PlaybackEngine and SignalHandler to accept PlaybackSynchronizer& instead of sem_t&. Updated play.cpp to create local PlaybackSynchronizer instance instead of global sem_t. This change naturally eliminated the last remaining global variable in play.cpp.

---
## Decision
*   [2025-10-09 17:06:25] Bug Fix: Title Not Displaying After EventPreProcessor Refactoring

## Rationale
*   The EventPreProcessor had a processTrackNameEvent() method to extract the title from MIDI TrackName meta events, but this method was never called in the processEvent() method. This was an oversight during the EventPreProcessor extraction (Decision #22).

## Implementation Details
*   Added processTrackNameEvent(event) call in the time-zero meta event processing block (line 76 of event_preprocessor.cpp), right after processKeySignatureEvent(). This ensures the title is extracted when processing TrackName meta events at time zero.

## Rationale
*   The EventPreProcessor had a processTrackNameEvent() method to extract the title from MIDI TrackName meta events, but this method was never called in the processEvent() method. This was an oversight during the EventPreProcessor extraction (Decision #22).

## Implementation Details
Added processTrackNameEvent(event) call in the time-zero meta event processing block (line 76 of event_preprocessor.cpp), right after processKeySignatureEvent(). This ensures the title is extracted when processing TrackName meta events at time zero.

## Implementation Details
*   Added processTrackNameEvent(event) call in the time-zero meta event processing block (line 76 of event_preprocessor.cpp), right after processKeySignatureEvent(). This ensures the title is extracted when processing TrackName meta events at time zero.

---
## Decision
*   [2025-10-09 17:06:25] Bug Fix: Verse Count Override Not Honoring Command-Line Options

## Rationale
*   After EventPreProcessor extraction, the verse count from MIDI files was being used even when command-line options (-n or -x flags) specified a different count. The EventPreProcessor only looked at verses from the MIDI file and never checked the Options object. Command-line options should take priority over MIDI file metadata.

## Implementation Details
*   Added setVersesFromOptions(int optionVerses) method to EventPreProcessor that applies command-line verse count with proper priority: (1) Command-line option takes precedence, (2) MIDI file value if no command-line option, (3) DEFAULT_VERSES if neither specified. Modified processCustomMetaEvents() to check options.getVerses() before using MIDI file verses. Added call to eventProcessor_->setVersesFromOptions(options.getVerses()) in MidiLoader::loadFile() after MIDI parsing completes.

## Rationale
*   After EventPreProcessor extraction, the verse count from MIDI files was being used even when command-line options (-n or -x flags) specified a different count. The EventPreProcessor only looked at verses from the MIDI file and never checked the Options object. Command-line options should take priority over MIDI file metadata.

## Implementation Details
Added setVersesFromOptions(int optionVerses) method to EventPreProcessor that applies command-line verse count with proper priority: (1) Command-line option takes precedence, (2) MIDI file value if no command-line option, (3) DEFAULT_VERSES if neither specified. Modified processCustomMetaEvents() to check options.getVerses() before using MIDI file verses. Added call to eventProcessor_->setVersesFromOptions(options.getVerses()) in MidiLoader::loadFile() after MIDI parsing completes.

## Implementation Details
*   Added setVersesFromOptions(int optionVerses) method to EventPreProcessor that applies command-line verse count with proper priority: (1) Command-line option takes precedence, (2) MIDI file value if no command-line option, (3) DEFAULT_VERSES if neither specified. Modified processCustomMetaEvents() to check options.getVerses() before using MIDI file verses. Added call to eventProcessor_->setVersesFromOptions(options.getVerses()) in MidiLoader::loadFile() after MIDI parsing completes.

---
## Decision
*   [2025-10-09 17:06:25] PlaybackEngine Decomposition - God Class Refactoring

## Rationale
*   Decomposed the 263-line PlaybackEngine 'God Class' into four focused classes following Single Responsibility Principle: PlaybackStateMachine (state management), RitardandoEffector (tempo effects), MusicalDirector (musical direction interpretation), and PlaybackOrchestrator (high-level coordination). This addresses code smell identified in todo.md and improves maintainability, testability, and extensibility.

## Implementation Details
*   Created playback_state_machine.hpp (56 lines), ritardando_effector.hpp/cpp (66+26 lines), musical_director.hpp/cpp (100+114 lines), and playback_orchestrator.hpp/cpp (126+167 lines). Updated play.cpp to use PlaybackOrchestrator instead of PlaybackEngine. Updated .vscode/tasks.json build configuration. Fixed two bugs discovered during refactoring: (1) Title not displaying - added processTrackNameEvent() call in EventPreProcessor, (2) Verses not honoring command-line options - added setVersesFromOptions() method to apply command-line overrides after MIDI parsing.

## Rationale
*   Decomposed the 263-line PlaybackEngine 'God Class' into four focused classes following Single Responsibility Principle: PlaybackStateMachine (state management), RitardandoEffector (tempo effects), MusicalDirector (musical direction interpretation), and PlaybackOrchestrator (high-level coordination). This addresses code smell identified in todo.md and improves maintainability, testability, and extensibility.

## Implementation Details
Created playback_state_machine.hpp (56 lines), ritardando_effector.hpp/cpp (66+26 lines), musical_director.hpp/cpp (100+114 lines), and playback_orchestrator.hpp/cpp (126+167 lines). Updated play.cpp to use PlaybackOrchestrator instead of PlaybackEngine. Updated .vscode/tasks.json build configuration. Fixed two bugs discovered during refactoring: (1) Title not displaying - added processTrackNameEvent() call in EventPreProcessor, (2) Verses not honoring command-line options - added setVersesFromOptions() method to apply command-line overrides after MIDI parsing.

## Implementation Details
*   Created playback_state_machine.hpp (56 lines), ritardando_effector.hpp/cpp (66+26 lines), musical_director.hpp/cpp (100+114 lines), and playback_orchestrator.hpp/cpp (126+167 lines). Updated play.cpp to use PlaybackOrchestrator instead of PlaybackEngine. Updated .vscode/tasks.json build configuration. Fixed two bugs discovered during refactoring: (1) Title not displaying - added processTrackNameEvent() call in EventPreProcessor, (2) Verses not honoring command-line options - added setVersesFromOptions() method to apply command-line overrides after MIDI parsing.

---
## Decision
*   [2025-10-09 17:06:25] EventPreProcessor Refactoring for Custom Meta Event Handling

## Rationale
*   Refactored MidiLoader according to todo.md Part 2 plan by extracting event processing logic into separate EventPreProcessor class. This improves separation of concerns and fixes GitHub Issue #21 by properly discarding custom meta events.

## Implementation Details
*   Created event_preprocessor.hpp and event_preprocessor.cpp with corrected custom event logic. Modified midi_loader.hpp/cpp to use pimpl pattern with EventPreProcessor delegation. Updated .vscode/tasks.json build configuration. Custom events now return false when found (discarded) and true when not found.

## Rationale
*   Refactored MidiLoader according to todo.md Part 2 plan by extracting event processing logic into separate EventPreProcessor class. This improves separation of concerns and fixes GitHub Issue #21 by properly discarding custom meta events.

## Implementation Details
Created event_preprocessor.hpp and event_preprocessor.cpp with corrected custom event logic. Modified midi_loader.hpp/cpp to use pimpl pattern with EventPreProcessor delegation. Updated .vscode/tasks.json build configuration. Custom events now return false when found (discarded) and true when not found.

## Implementation Details
*   Created event_preprocessor.hpp and event_preprocessor.cpp with corrected custom event logic. Modified midi_loader.hpp/cpp to use pimpl pattern with EventPreProcessor delegation. Updated .vscode/tasks.json build configuration. Custom events now return false when found (discarded) and true when not found.

---
## Decision
*   [2025-10-09 17:06:24] Fixed Issue #9: Replaced Boolean Flag with std::optional in DeviceManager

## Rationale
*   DeviceManager used a yamlConfig struct alongside a separate yamlLoaded bool flag, creating potential for invalid state (yamlConfig exists but yamlLoaded=false). This pattern is error-prone and less expressive than modern C++ alternatives. std::optional provides clearer semantics: either we have valid YAML config or we don't.

## Implementation Details
*   Replaced 'YamlConfig yamlConfig; bool yamlLoaded;' with 'std::optional<YamlConfig> yamlConfig_'. Updated all yamlLoaded checks to yamlConfig_.has_value(). Changed all yamlConfig accesses to use yamlConfig_-> operator (equivalent to yamlConfig_.value()). Modified parseYamlContent() to create a new YamlConfig, populate it, then assign to optional using std::move. Added <optional> include. Build completed successfully with no errors.

## Rationale
*   DeviceManager used a yamlConfig struct alongside a separate yamlLoaded bool flag, creating potential for invalid state (yamlConfig exists but yamlLoaded=false). This pattern is error-prone and less expressive than modern C++ alternatives. std::optional provides clearer semantics: either we have valid YAML config or we don't.

## Implementation Details
Replaced 'YamlConfig yamlConfig; bool yamlLoaded;' with 'std::optional<YamlConfig> yamlConfig_'. Updated all yamlLoaded checks to yamlConfig_.has_value(). Changed all yamlConfig accesses to use yamlConfig_-> operator (equivalent to yamlConfig_.value()). Modified parseYamlContent() to create a new YamlConfig, populate it, then assign to optional using std::move. Added <optional> include. Build completed successfully with no errors.

## Implementation Details
*   Replaced 'YamlConfig yamlConfig; bool yamlLoaded;' with 'std::optional<YamlConfig> yamlConfig_'. Updated all yamlLoaded checks to yamlConfig_.has_value(). Changed all yamlConfig accesses to use yamlConfig_-> operator (equivalent to yamlConfig_.value()). Modified parseYamlContent() to create a new YamlConfig, populate it, then assign to optional using std::move. Added <optional> include. Build completed successfully with no errors.

---
## Decision
*   [2025-10-09 17:06:24] Fixed Issue #8: Standardized Error Handling in DeviceManager

## Rationale
*   DeviceManager had inconsistent error handling with a mix of bool returns and exceptions. Methods like loadDevicePresets() returned bool but always threw exceptions (never returned false), while parseYamlFile() caught exceptions and returned bool only to have the caller throw on false. This created confusion about the error handling contract and violated the principle of using exceptions for unrecoverable errors.

## Implementation Details
*   Changed loadDevicePresets(), parseYamlFile(), and parseYamlContent() from bool return to void, making them throw exceptions directly for unrecoverable errors (file not found, parse failures). Updated play.cpp to ensure loadDevicePresets() is called within try/catch block alongside other device operations. Maintained bool return for waitForDeviceConnection() as timeout is an expected operational state. All error messages were already translated in .po files. Build completed successfully with no errors.

## Rationale
*   DeviceManager had inconsistent error handling with a mix of bool returns and exceptions. Methods like loadDevicePresets() returned bool but always threw exceptions (never returned false), while parseYamlFile() caught exceptions and returned bool only to have the caller throw on false. This created confusion about the error handling contract and violated the principle of using exceptions for unrecoverable errors.

## Implementation Details
Changed loadDevicePresets(), parseYamlFile(), and parseYamlContent() from bool return to void, making them throw exceptions directly for unrecoverable errors (file not found, parse failures). Updated play.cpp to ensure loadDevicePresets() is called within try/catch block alongside other device operations. Maintained bool return for waitForDeviceConnection() as timeout is an expected operational state. All error messages were already translated in .po files. Build completed successfully with no errors.

## Implementation Details
*   Changed loadDevicePresets(), parseYamlFile(), and parseYamlContent() from bool return to void, making them throw exceptions directly for unrecoverable errors (file not found, parse failures). Updated play.cpp to ensure loadDevicePresets() is called within try/catch block alongside other device operations. Maintained bool return for waitForDeviceConnection() as timeout is an expected operational state. All error messages were already translated in .po files. Build completed successfully with no errors.

---
## Decision
*   [2025-10-09 17:06:24] Fixed Issue #7: Eliminated Musical Marker Constants Duplication

## Rationale
*   Musical marker constants (INTRO_BEGIN, INTRO_END, RITARDANDO_INDICATOR, D_C_AL_FINE, FINE_INDICATOR) were duplicated across three header files (event_preprocessor.hpp, musical_director.hpp, playback_engine.hpp), violating the DRY principle and creating maintenance burden.

## Implementation Details
*   Created new midi_markers.hpp header file containing all marker constants in the MidiPlay::MidiMarkers namespace. Updated all three affected headers to include midi_markers.hpp and removed duplicate definitions. Updated implementation files (event_preprocessor.cpp, musical_director.cpp, playback_engine.cpp) to reference constants via MidiMarkers:: namespace. Build completed successfully with no errors.

## Rationale
*   Musical marker constants (INTRO_BEGIN, INTRO_END, RITARDANDO_INDICATOR, D_C_AL_FINE, FINE_INDICATOR) were duplicated across three header files (event_preprocessor.hpp, musical_director.hpp, playback_engine.hpp), violating the DRY principle and creating maintenance burden.

## Implementation Details
Created new midi_markers.hpp header file containing all marker constants in the MidiPlay::MidiMarkers namespace. Updated all three affected headers to include midi_markers.hpp and removed duplicate definitions. Updated implementation files (event_preprocessor.cpp, musical_director.cpp, playback_engine.cpp) to reference constants via MidiMarkers:: namespace. Build completed successfully with no errors.

## Implementation Details
*   Created new midi_markers.hpp header file containing all marker constants in the MidiPlay::MidiMarkers namespace. Updated all three affected headers to include midi_markers.hpp and removed duplicate definitions. Updated implementation files (event_preprocessor.cpp, musical_director.cpp, playback_engine.cpp) to reference constants via MidiMarkers:: namespace. Build completed successfully with no errors.

---
## Decision
*   [2025-10-09 17:06:24] Code Smell Review and Quality Improvements - 6 Critical Fixes Applied

## Rationale
*   Conducted comprehensive code smell analysis across entire codebase following Phase 3 refactoring completion. Created CODE_SMELLS.md documenting 22 issues categorized by priority (High: 5, Medium: 10, Low: 7). Prioritized and fixed 6 critical issues to improve code quality, internationalization coverage, portability, and modernization.

## Implementation Details
*   Fixed 5 High Priority issues: (1) Added _() i18n wrapper to signal_handler.cpp elapsed time message, (2) Added _() i18n wrapper to device_manager.cpp exception, (3) Added explicit return EXIT_SUCCESS to play.cpp main(), (4) Replaced deprecated POSIX usleep() with std::this_thread::sleep_for() in playback_orchestrator.cpp, (5) Removed namespace pollution (using namespace midiplay;) from play.cpp. Fixed 1 Medium Priority issue: (6) Eliminated device key string duplication by creating DeviceKeys namespace with constants and helper methods (deviceTypeToKey(), deviceKeyToType()) in device_manager.hpp/cpp. Updated all 4 translation files (es.po, fr_CA.po, fr_FR.po, pt_BR.po) with 3 new strings. Updated po/POTFILES.in to reflect refactored file structure. Created comprehensive CODE_SMELLS.md documentation (696 lines) for future refactoring sessions.

## Rationale
*   Conducted comprehensive code smell analysis across entire codebase following Phase 3 refactoring completion. Created CODE_SMELLS.md documenting 22 issues categorized by priority (High: 5, Medium: 10, Low: 7). Prioritized and fixed 6 critical issues to improve code quality, internationalization coverage, portability, and modernization.

## Implementation Details
Fixed 5 High Priority issues: (1) Added _() i18n wrapper to signal_handler.cpp elapsed time message, (2) Added _() i18n wrapper to device_manager.cpp exception, (3) Added explicit return EXIT_SUCCESS to play.cpp main(), (4) Replaced deprecated POSIX usleep() with std::this_thread::sleep_for() in playback_orchestrator.cpp, (5) Removed namespace pollution (using namespace midiplay;) from play.cpp. Fixed 1 Medium Priority issue: (6) Eliminated device key string duplication by creating DeviceKeys namespace with constants and helper methods (deviceTypeToKey(), deviceKeyToType()) in device_manager.hpp/cpp. Updated all 4 translation files (es.po, fr_CA.po, fr_FR.po, pt_BR.po) with 3 new strings. Updated po/POTFILES.in to reflect refactored file structure. Created comprehensive CODE_SMELLS.md documentation (696 lines) for future refactoring sessions.

## Implementation Details
*   Fixed 5 High Priority issues: (1) Added _() i18n wrapper to signal_handler.cpp elapsed time message, (2) Added _() i18n wrapper to device_manager.cpp exception, (3) Added explicit return EXIT_SUCCESS to play.cpp main(), (4) Replaced deprecated POSIX usleep() with std::this_thread::sleep_for() in playback_orchestrator.cpp, (5) Removed namespace pollution (using namespace midiplay;) from play.cpp. Fixed 1 Medium Priority issue: (6) Eliminated device key string duplication by creating DeviceKeys namespace with constants and helper methods (deviceTypeToKey(), deviceKeyToType()) in device_manager.hpp/cpp. Updated all 4 translation files (es.po, fr_CA.po, fr_FR.po, pt_BR.po) with 3 new strings. Updated po/POTFILES.in to reflect refactored file structure. Created comprehensive CODE_SMELLS.md documentation (696 lines) for future refactoring sessions.

---
## Decision
*   [2025-10-09 17:06:23] Fixed Issue #16: Standardized Naming Convention in Options Class

## Rationale
*   The Options class had inconsistent private member naming - mixing snake_case (_filename) with camelCase (_uSecPerBeat, _playIntro, _displayWarnings, _urlName). This violates C++ Core Guidelines C.131 which recommends consistent naming. Additionally, the getter get_uSecPerBeat() used snake_case while all other getters used camelCase.

## Implementation Details
*   Standardized all private members to trailing underscore convention (C++ Core Guidelines preference): _argc → argc_, _bpm → bpm_, _uSecPerBeat → usec_per_beat_, _playIntro → play_intro_, _displayWarnings → display_warnings_, _urlName → url_name_, etc. Renamed getter get_uSecPerBeat() → getUsecPerBeat() for consistency with other camelCase getters (getBpm(), getSpeed(), etc.). Updated all usages in options.hpp, midi_loader.cpp:53, and event_preprocessor.cpp:145. Build completed successfully with no errors.

## Rationale
*   The Options class had inconsistent private member naming - mixing snake_case (_filename) with camelCase (_uSecPerBeat, _playIntro, _displayWarnings, _urlName). This violates C++ Core Guidelines C.131 which recommends consistent naming. Additionally, the getter get_uSecPerBeat() used snake_case while all other getters used camelCase.

## Implementation Details
Standardized all private members to trailing underscore convention (C++ Core Guidelines preference): _argc → argc_, _bpm → bpm_, _uSecPerBeat → usec_per_beat_, _playIntro → play_intro_, _displayWarnings → display_warnings_, _urlName → url_name_, etc. Renamed getter get_uSecPerBeat() → getUsecPerBeat() for consistency with other camelCase getters (getBpm(), getSpeed(), etc.). Updated all usages in options.hpp, midi_loader.cpp:53, and event_preprocessor.cpp:145. Build completed successfully with no errors.

## Implementation Details
*   Standardized all private members to trailing underscore convention (C++ Core Guidelines preference): _argc → argc_, _bpm → bpm_, _uSecPerBeat → usec_per_beat_, _playIntro → play_intro_, _displayWarnings → display_warnings_, _urlName → url_name_, etc. Renamed getter get_uSecPerBeat() → getUsecPerBeat() for consistency with other camelCase getters (getBpm(), getSpeed(), etc.). Updated all usages in options.hpp, midi_loader.cpp:53, and event_preprocessor.cpp:145. Build completed successfully with no errors.

---
## Decision
*   [2025-10-09 17:06:23] Fixed Issue #10: Refactored Oversized parse() Method in Options Class

## Rationale
*   The Options::parse() method was 152 lines long, violating the Single Responsibility Principle. It handled all command-line argument parsing logic in one monolithic method, making it hard to test individual option handlers and reducing maintainability. This was the last remaining medium-priority code smell.

## Implementation Details
*   Extracted four private helper methods from parse(): displayVersion() for version output, displayHelp() for help text display, handlePreludeOption(optarg) for prelude/postlude processing, handleTempoOption(optarg) for tempo validation and setting, and handleVersesOption(optarg, playIntro) for verse count handling. Refactored parse() method to delegate to these helpers, reducing it from 152 lines to ~90 lines. Each handler now has a single, clear responsibility. Build completed successfully with no errors.

## Rationale
*   The Options::parse() method was 152 lines long, violating the Single Responsibility Principle. It handled all command-line argument parsing logic in one monolithic method, making it hard to test individual option handlers and reducing maintainability. This was the last remaining medium-priority code smell.

## Implementation Details
Extracted four private helper methods from parse(): displayVersion() for version output, displayHelp() for help text display, handlePreludeOption(optarg) for prelude/postlude processing, handleTempoOption(optarg) for tempo validation and setting, and handleVersesOption(optarg, playIntro) for verse count handling. Refactored parse() method to delegate to these helpers, reducing it from 152 lines to ~90 lines. Each handler now has a single, clear responsibility. Build completed successfully with no errors.

## Implementation Details
*   Extracted four private helper methods from parse(): displayVersion() for version output, displayHelp() for help text display, handlePreludeOption(optarg) for prelude/postlude processing, handleTempoOption(optarg) for tempo validation and setting, and handleVersesOption(optarg, playIntro) for verse count handling. Refactored parse() method to delegate to these helpers, reducing it from 152 lines to ~90 lines. Each handler now has a single, clear responsibility. Build completed successfully with no errors.

---
## Decision
*   [2025-10-09 17:06:23] Fixed Issue #15: Debug Output Without i18n - Wrapped in _() Macros

## Rationale
*   Debug output string 'Available MIDI ports:' in device_manager.cpp was not wrapped in _() macro, breaking i18n consistency. While this is DEBUG-only output, wrapping it maintains full i18n coverage and allows for translated debug output in multi-language development environments.

## Implementation Details
*   Wrapped the debug string at device_manager.cpp:366 in _() macro. Added translations to all four .po files: es.po (Spanish), fr_FR.po (French), fr_CA.po (French Canadian), and pt_BR.po (Portuguese Brazilian). Build completed successfully.

## Rationale
*   Debug output string 'Available MIDI ports:' in device_manager.cpp was not wrapped in _() macro, breaking i18n consistency. While this is DEBUG-only output, wrapping it maintains full i18n coverage and allows for translated debug output in multi-language development environments.

## Implementation Details
Wrapped the debug string at device_manager.cpp:366 in _() macro. Added translations to all four .po files: es.po (Spanish), fr_FR.po (French), fr_CA.po (French Canadian), and pt_BR.po (Portuguese Brazilian). Build completed successfully.

## Implementation Details
*   Wrapped the debug string at device_manager.cpp:366 in _() macro. Added translations to all four .po files: es.po (Spanish), fr_FR.po (French), fr_CA.po (French Canadian), and pt_BR.po (Portuguese Brazilian). Build completed successfully.

---
## Decision
*   [2025-10-09 17:06:23] Fixed Issue #12: Removed Unnecessary TimeSignature Message Size Check

## Rationale
*   The code had a hardcoded magic number (6) checking the size of TimeSignature MIDI messages. This check was unnecessary since IsMeta(MetaType::TimeSignature) already validates the message type, and the MIDI standard defines the TimeSignature meta event structure. The size check added no value and reduced code clarity.

## Implementation Details
*   Removed '&& message.size() == 6' from the conditional in event_preprocessor.cpp:125. The check now simply validates message.IsMeta(Message::MetaType::TimeSignature) before extracting time signature data. Build completed successfully with no errors.

## Rationale
*   The code had a hardcoded magic number (6) checking the size of TimeSignature MIDI messages. This check was unnecessary since IsMeta(MetaType::TimeSignature) already validates the message type, and the MIDI standard defines the TimeSignature meta event structure. The size check added no value and reduced code clarity.

## Implementation Details
Removed '&& message.size() == 6' from the conditional in event_preprocessor.cpp:125. The check now simply validates message.IsMeta(Message::MetaType::TimeSignature) before extracting time signature data. Build completed successfully with no errors.

## Implementation Details
*   Removed '&& message.size() == 6' from the conditional in event_preprocessor.cpp:125. The check now simply validates message.IsMeta(Message::MetaType::TimeSignature) before extracting time signature data. Build completed successfully with no errors.

---
## Decision
*   [2025-10-09 17:06:20] Automated version detection from Git tags for installer scripts

## Rationale
*   Eliminates hard-coded version numbers and ensures consistency between build system and installer. Uses same approach as build system (git describe --tags) with semantic version extraction matching options.hpp pattern. Follows DRY principle by centralizing version logic in reusable libraries.

## Implementation Details
*   Created 5 SRP-compliant library modules (lib/version.sh, translations.sh, packaging.sh, metadata.sh, validation.sh). Implemented 4-tier version detection: manual override → Git tags → .VERSION file → binary extraction → directory parsing. Refactored update-installer-package.sh, create-installer-archive.sh, and install.sh to use libraries. Fixed bug where archives could be created without translations. Reduced code duplication by ~200+ lines.

## Rationale
*   Eliminates hard-coded version numbers and ensures consistency between build system and installer. Uses same approach as build system (git describe --tags) with semantic version extraction matching options.hpp pattern. Follows DRY principle by centralizing version logic in reusable libraries.

## Implementation Details
Created 5 SRP-compliant library modules (lib/version.sh, translations.sh, packaging.sh, metadata.sh, validation.sh). Implemented 4-tier version detection: manual override → Git tags → .VERSION file → binary extraction → directory parsing. Refactored update-installer-package.sh, create-installer-archive.sh, and install.sh to use libraries. Fixed bug where archives could be created without translations. Reduced code duplication by ~200+ lines.

## Implementation Details
*   Created 5 SRP-compliant library modules (lib/version.sh, translations.sh, packaging.sh, metadata.sh, validation.sh). Implemented 4-tier version detection: manual override → Git tags → .VERSION file → binary extraction → directory parsing. Refactored update-installer-package.sh, create-installer-archive.sh, and install.sh to use libraries. Fixed bug where archives could be created without translations. Reduced code duplication by ~200+ lines.

---
## Decision
*   [2025-10-08 20:23:31] Automated version detection from Git tags for installer scripts

## Rationale
*   Eliminates hard-coded version numbers and ensures consistency between build system and installer. Uses same approach as build system (git describe --tags) with semantic version extraction matching options.hpp pattern. Follows DRY principle by centralizing version logic in reusable libraries.

## Implementation Details
*   Created 5 SRP-compliant library modules (lib/version.sh, translations.sh, packaging.sh, metadata.sh, validation.sh). Implemented 4-tier version detection: manual override → Git tags → .VERSION file → binary extraction → directory parsing. Refactored update-installer-package.sh, create-installer-archive.sh, and install.sh to use libraries. Fixed bug where archives could be created without translations. Reduced code duplication by ~200+ lines.

---
## Decision
*   [2025-10-08 17:47:40] Fixed Issue #16: Standardized Naming Convention in Options Class

## Rationale
*   The Options class had inconsistent private member naming - mixing snake_case (_filename) with camelCase (_uSecPerBeat, _playIntro, _displayWarnings, _urlName). This violates C++ Core Guidelines C.131 which recommends consistent naming. Additionally, the getter get_uSecPerBeat() used snake_case while all other getters used camelCase.

## Implementation Details
*   Standardized all private members to trailing underscore convention (C++ Core Guidelines preference): _argc → argc_, _bpm → bpm_, _uSecPerBeat → usec_per_beat_, _playIntro → play_intro_, _displayWarnings → display_warnings_, _urlName → url_name_, etc. Renamed getter get_uSecPerBeat() → getUsecPerBeat() for consistency with other camelCase getters (getBpm(), getSpeed(), etc.). Updated all usages in options.hpp, midi_loader.cpp:53, and event_preprocessor.cpp:145. Build completed successfully with no errors.

---
## Decision
*   [2025-10-08 00:28:30] Fixed Issue #10: Refactored Oversized parse() Method in Options Class

## Rationale
*   The Options::parse() method was 152 lines long, violating the Single Responsibility Principle. It handled all command-line argument parsing logic in one monolithic method, making it hard to test individual option handlers and reducing maintainability. This was the last remaining medium-priority code smell.

## Implementation Details
*   Extracted four private helper methods from parse(): displayVersion() for version output, displayHelp() for help text display, handlePreludeOption(optarg) for prelude/postlude processing, handleTempoOption(optarg) for tempo validation and setting, and handleVersesOption(optarg, playIntro) for verse count handling. Refactored parse() method to delegate to these helpers, reducing it from 152 lines to ~90 lines. Each handler now has a single, clear responsibility. Build completed successfully with no errors.

---
## Decision
*   [2025-10-08 00:14:56] Fixed Issue #15: Debug Output Without i18n - Wrapped in _() Macros

## Rationale
*   Debug output string 'Available MIDI ports:' in device_manager.cpp was not wrapped in _() macro, breaking i18n consistency. While this is DEBUG-only output, wrapping it maintains full i18n coverage and allows for translated debug output in multi-language development environments.

## Implementation Details
*   Wrapped the debug string at device_manager.cpp:366 in _() macro. Added translations to all four .po files: es.po (Spanish), fr_FR.po (French), fr_CA.po (French Canadian), and pt_BR.po (Portuguese Brazilian). Build completed successfully.

---
## Decision
*   [2025-10-07 23:46:40] Fixed Issue #12: Removed Unnecessary TimeSignature Message Size Check

## Rationale
*   The code had a hardcoded magic number (6) checking the size of TimeSignature MIDI messages. This check was unnecessary since IsMeta(MetaType::TimeSignature) already validates the message type, and the MIDI standard defines the TimeSignature meta event structure. The size check added no value and reduced code clarity.

## Implementation Details
*   Removed '&& message.size() == 6' from the conditional in event_preprocessor.cpp:125. The check now simply validates message.IsMeta(Message::MetaType::TimeSignature) before extracting time signature data. Build completed successfully with no errors.

---
## Decision
*   [2025-10-07 23:21:33] Fixed Issue #9: Replaced Boolean Flag with std::optional in DeviceManager

## Rationale
*   DeviceManager used a yamlConfig struct alongside a separate yamlLoaded bool flag, creating potential for invalid state (yamlConfig exists but yamlLoaded=false). This pattern is error-prone and less expressive than modern C++ alternatives. std::optional provides clearer semantics: either we have valid YAML config or we don't.

## Implementation Details
*   Replaced 'YamlConfig yamlConfig; bool yamlLoaded;' with 'std::optional<YamlConfig> yamlConfig_'. Updated all yamlLoaded checks to yamlConfig_.has_value(). Changed all yamlConfig accesses to use yamlConfig_-> operator (equivalent to yamlConfig_.value()). Modified parseYamlContent() to create a new YamlConfig, populate it, then assign to optional using std::move. Added <optional> include. Build completed successfully with no errors.

---
## Decision
*   [2025-10-07 23:11:50] Fixed Issue #8: Standardized Error Handling in DeviceManager

## Rationale
*   DeviceManager had inconsistent error handling with a mix of bool returns and exceptions. Methods like loadDevicePresets() returned bool but always threw exceptions (never returned false), while parseYamlFile() caught exceptions and returned bool only to have the caller throw on false. This created confusion about the error handling contract and violated the principle of using exceptions for unrecoverable errors.

## Implementation Details
*   Changed loadDevicePresets(), parseYamlFile(), and parseYamlContent() from bool return to void, making them throw exceptions directly for unrecoverable errors (file not found, parse failures). Updated play.cpp to ensure loadDevicePresets() is called within try/catch block alongside other device operations. Maintained bool return for waitForDeviceConnection() as timeout is an expected operational state. All error messages were already translated in .po files. Build completed successfully with no errors.

---
## Decision
*   [2025-10-07 22:58:59] Fixed Issue #7: Eliminated Musical Marker Constants Duplication

## Rationale
*   Musical marker constants (INTRO_BEGIN, INTRO_END, RITARDANDO_INDICATOR, D_C_AL_FINE, FINE_INDICATOR) were duplicated across three header files (event_preprocessor.hpp, musical_director.hpp, playback_engine.hpp), violating the DRY principle and creating maintenance burden.

## Implementation Details
*   Created new midi_markers.hpp header file containing all marker constants in the MidiPlay::MidiMarkers namespace. Updated all three affected headers to include midi_markers.hpp and removed duplicate definitions. Updated implementation files (event_preprocessor.cpp, musical_director.cpp, playback_engine.cpp) to reference constants via MidiMarkers:: namespace. Build completed successfully with no errors.

---
## Decision
*   [2025-10-07 22:44:29] Code Smell Review and Quality Improvements - 6 Critical Fixes Applied

## Rationale
*   Conducted comprehensive code smell analysis across entire codebase following Phase 3 refactoring completion. Created CODE_SMELLS.md documenting 22 issues categorized by priority (High: 5, Medium: 10, Low: 7). Prioritized and fixed 6 critical issues to improve code quality, internationalization coverage, portability, and modernization.

## Implementation Details
*   Fixed 5 High Priority issues: (1) Added _() i18n wrapper to signal_handler.cpp elapsed time message, (2) Added _() i18n wrapper to device_manager.cpp exception, (3) Added explicit return EXIT_SUCCESS to play.cpp main(), (4) Replaced deprecated POSIX usleep() with std::this_thread::sleep_for() in playback_orchestrator.cpp, (5) Removed namespace pollution (using namespace midiplay;) from play.cpp. Fixed 1 Medium Priority issue: (6) Eliminated device key string duplication by creating DeviceKeys namespace with constants and helper methods (deviceTypeToKey(), deviceKeyToType()) in device_manager.hpp/cpp. Updated all 4 translation files (es.po, fr_CA.po, fr_FR.po, pt_BR.po) with 3 new strings. Updated po/POTFILES.in to reflect refactored file structure. Created comprehensive CODE_SMELLS.md documentation (696 lines) for future refactoring sessions.

---
## Decision
*   [2025-10-07 20:04:18] Bug Fix: Title Not Displaying After EventPreProcessor Refactoring

## Rationale
*   The EventPreProcessor had a processTrackNameEvent() method to extract the title from MIDI TrackName meta events, but this method was never called in the processEvent() method. This was an oversight during the EventPreProcessor extraction (Decision #22).

## Implementation Details
*   Added processTrackNameEvent(event) call in the time-zero meta event processing block (line 76 of event_preprocessor.cpp), right after processKeySignatureEvent(). This ensures the title is extracted when processing TrackName meta events at time zero.

---
## Decision
*   [2025-10-07 20:04:18] Bug Fix: Verse Count Override Not Honoring Command-Line Options

## Rationale
*   After EventPreProcessor extraction, the verse count from MIDI files was being used even when command-line options (-n or -x flags) specified a different count. The EventPreProcessor only looked at verses from the MIDI file and never checked the Options object. Command-line options should take priority over MIDI file metadata.

## Implementation Details
*   Added setVersesFromOptions(int optionVerses) method to EventPreProcessor that applies command-line verse count with proper priority: (1) Command-line option takes precedence, (2) MIDI file value if no command-line option, (3) DEFAULT_VERSES if neither specified. Modified processCustomMetaEvents() to check options.getVerses() before using MIDI file verses. Added call to eventProcessor_->setVersesFromOptions(options.getVerses()) in MidiLoader::loadFile() after MIDI parsing completes.

---
## Decision
*   [2025-10-07 20:03:18] PlaybackEngine Decomposition - God Class Refactoring

## Rationale
*   Decomposed the 263-line PlaybackEngine 'God Class' into four focused classes following Single Responsibility Principle: PlaybackStateMachine (state management), RitardandoEffector (tempo effects), MusicalDirector (musical direction interpretation), and PlaybackOrchestrator (high-level coordination). This addresses code smell identified in todo.md and improves maintainability, testability, and extensibility.

## Implementation Details
*   Created playback_state_machine.hpp (56 lines), ritardando_effector.hpp/cpp (66+26 lines), musical_director.hpp/cpp (100+114 lines), and playback_orchestrator.hpp/cpp (126+167 lines). Updated play.cpp to use PlaybackOrchestrator instead of PlaybackEngine. Updated .vscode/tasks.json build configuration. Fixed two bugs discovered during refactoring: (1) Title not displaying - added processTrackNameEvent() call in EventPreProcessor, (2) Verses not honoring command-line options - added setVersesFromOptions() method to apply command-line overrides after MIDI parsing.

---
## Decision
*   [2025-10-07 15:50:16] EventPreProcessor Refactoring for Custom Meta Event Handling

## Rationale
*   Refactored MidiLoader according to todo.md Part 2 plan by extracting event processing logic into separate EventPreProcessor class. This improves separation of concerns and fixes GitHub Issue #21 by properly discarding custom meta events.

## Implementation Details
*   Created event_preprocessor.hpp and event_preprocessor.cpp with corrected custom event logic. Modified midi_loader.hpp/cpp to use pimpl pattern with EventPreProcessor delegation. Updated .vscode/tasks.json build configuration. Custom events now return false when found (discarded) and true when not found.

---
## Decision
*   [2025-10-07 15:48:49] ThreadSanitizer ARM64 VMA Range Error Analysis and Solutions

## Rationale
*   Analyzed ThreadSanitizer VMA range error on ARM64 system (47-bit VMA not supported, only 39/42/48). Created comprehensive documentation with multiple solution approaches including AddressSanitizer alternative.

## Implementation Details
*   Created THREAD_SANITIZER_FIXES.md, TSAN_ADVANCED_SOLUTIONS.md, and VSCODE_DEFAULT_TASK_GUIDE.md. Recommended using AddressSanitizer (-fsanitize=address,undefined,leak) as primary solution due to full ARM64 support.

---
## Decision
*   [2025-10-06 23:51:25] Implemented centralized internationalization with proper plural handling

## Rationale
*   The original code had internationalization macros scattered across modules and used manual pluralization (adding 's') which doesn't work for all languages. We centralized all i18n functionality in i18n.hpp and implemented proper ngettext-based plural handling.

## Implementation Details
*   Created i18n.hpp with centralized macros, updated all modules to use it, fixed plural case in playback_engine.cpp, updated all .po files with proper plural forms, and tested with multiple locales.

---
## Decision
*   [2025-10-02 22:17:28] Completed Phase 3: Production-Ready i18n Build Integration

## Rationale
*   Integrated translation compilation into build system and installer package workflow. This ensures translations are automatically compiled during development and properly distributed in production packages, with no manual intervention required.

## Implementation Details
*   1. Created po/compile-translations.sh for automated compilation of all 4 languages (es, pt_BR, fr_FR, fr_CA)
2. Updated update-installer-package.sh to compile translations and copy .mo files to installer structure
3. Modified .vscode/tasks.json to compile translations before C++ build (dependsOn)
4. Enhanced install.sh to verify and report installed translation files
5. Created comprehensive po/README.md with plural forms documentation
6. Fixed arithmetic increment issues with set -e (bash strict mode)
7. Updated version to 1.5.7 across all installer files
8. Successfully tested complete workflow: compile → package → verified 4 translations in installer

---
## Decision
*   [2025-10-02 19:24:29] Adopt GNU gettext for Internationalization (i18n)

## Rationale
*   Selected GNU gettext as the standard, robust, and scalable solution for internationalization over simpler methods like a custom messages.hpp or a YAML-based system. The long-term benefits of its standard tooling and workflow outweigh the moderate initial setup cost.

## Implementation Details
*   Implementation will be done in three incremental phases: 1) Proof of Concept, 2) Full Code Refactoring, 3) Build System Integration. The existing .vscode/tasks.json build system will be retained, augmented with helper shell scripts for the gettext workflow, and the compiler will be linked with '-lintl'.

---
## Decision
*   [2025-10-01 14:07:00] Synchronization Modernization - Replaced POSIX semaphores with C++ std::condition_variable

## Rationale
*   The codebase used POSIX semaphores (sem_t) for playback synchronization, which had several issues: (1) Global variable sem_t sem violated encapsulation, (2) POSIX-specific code reduced portability to non-POSIX platforms like Windows, (3) Manual initialization/cleanup (sem_init/sem_destroy) was error-prone and not exception-safe, (4) C-style API was not idiomatic modern C++. Modernizing to std::condition_variable addresses all these issues while maintaining identical functionality and performance.

## Implementation Details
*   Created PlaybackSynchronizer class encapsulating std::mutex and std::condition_variable with a boolean flag. The class provides wait() and notify() methods that replace sem_wait() and sem_post() respectively. Key implementation details: (1) RAII-based design with automatic cleanup, (2) Automatic reset of finished_ flag after wait() completes for seamless multi-cycle usage, (3) Notify outside lock scope for optimal performance, (4) Delete copy/move constructors to prevent synchronization issues. Updated PlaybackEngine and SignalHandler to accept PlaybackSynchronizer& instead of sem_t&. Updated play.cpp to create local PlaybackSynchronizer instance instead of global sem_t. This change naturally eliminated the last remaining global variable in play.cpp.

---
## Decision
*   [2025-10-01 12:22:20] Fixed playIntro bug: -p and -x flags were playing introduction when they shouldn't

## Rationale
*   The MidiLoader was setting playIntro based solely on whether intro markers existed in the MIDI file, completely ignoring the command-line option. The -p (prelude) and -x (verses without intro) flags correctly set _playIntro = false in Options, but MidiLoader never retrieved this value, causing introductions to play even when explicitly disabled.

## Implementation Details
*   Added one line in midi_loader.cpp:63 to retrieve playIntro from options: `playIntro_ = options.isPlayIntro();`. Simplified finalizeLoading() method (lines 383-396) to only override playIntro_ if MIDI file has no intro markers (can't play intro if markers don't exist), otherwise retaining the value from command-line options. This ensures proper priority: command-line option first, then file capability check.

---
## Decision
*   [2025-10-01 12:11:29] Fixed verse count override bug: -n and -x flags were being ignored

## Rationale
*   The MidiLoader was retrieving speed and tempo overrides from Options but never retrieved the verse count. This caused command-line verse overrides (-n and -x flags) to be completely ignored, with the program always using MIDI file meta-events or default value instead.

## Implementation Details
*   Added single line in midi_loader.cpp:60 to retrieve verses from options: `verses_ = options.getVerses();`. This follows the same pattern as speed (line 58) and tempo override (line 62). The fix ensures command-line verse overrides take priority over MIDI file meta-events and default values, restoring intended behavior.

---
## Decision
*   [2025-10-01 01:37:32] Modernized synchronization from POSIX semaphores to standard C++ primitives

## Rationale
*   Replaced POSIX semaphore (sem_t) with std::condition_variable and std::mutex to achieve: (1) Elimination of the last global variable in play.cpp, (2) Better portability using standard C++ instead of POSIX-specific APIs, (3) Exception safety through RAII principles, (4) More idiomatic modern C++ code. This change completes Phase 3's goal of eliminating all global variables (except static version string).

## Implementation Details
*   Created PlaybackSynchronizer class with wait() and notify() methods that wrap std::condition_variable and std::mutex. Updated PlaybackEngine, SignalHandler, and play.cpp to use PlaybackSynchronizer& instead of sem_t&. Removed all POSIX semaphore includes and calls (sem_init, sem_wait, sem_post, sem_destroy). The PlaybackSynchronizer provides automatic reset after wait() for convenient use in multi-verse playback loops. All cleanup happens automatically via RAII destructors. Updated .vscode/tasks.json to include playback_synchronizer.cpp in build.

---
## Decision
*   [2025-10-01 01:12:47] Implemented --verbose (-V) command-line flag for enhanced debugging output

## Rationale
*   Adding a verbose flag provides users and developers with optional detailed output about MIDI port detection, device connection, and other internal operations. This aids in debugging and troubleshooting without cluttering normal output.

## Implementation Details
*   Added private bool `_verbose` member to Options class (line 47), added public getter `isVerbose()` (lines 91-93), added -V flag to getopt_long call (line 117), and implemented case 'V' handler (lines 206-208). Verbose output now displays detected MIDI ports (lines 87-96) and connected device information (lines 111-115) in play.cpp when enabled.

---
## Decision
*   [2025-10-01 01:12:34] Moved displayWarnings flag to Options object for better configuration management

## Rationale
*   The displayWarnings flag is a command-line configurable option that belongs with other user preferences in the Options class. This improves encapsulation by keeping all command-line options together and makes the option more discoverable and maintainable.

## Implementation Details
*   Added private bool `_displayWarnings` member to Options class (line 48), added public getter `isDisplayWarnings()` (lines 95-97), and added -W flag handling in parse() method (lines 209-211). PlaybackEngine now retrieves this setting via `options.isDisplayWarnings()` at line 133 in play.cpp.

---
## Decision
*   [2025-10-01 01:12:15] Refactored global outport to local scope in main() for better encapsulation

## Rationale
*   Moving outport from global to local scope in main() improves code organization, reduces global state, and follows best practices for resource management. This change supports the Phase 3 goal of cleaning up main() and improving overall architecture.

## Implementation Details
*   Changed `output::Default outport;` from global declaration (line 35 area) to local variable declaration inside main() at line 83. All references to outport remain unchanged as they already accessed it through function parameters or local scope.

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
