# Decision Log

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
