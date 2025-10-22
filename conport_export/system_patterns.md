# System Patterns

---
## Test Isolation Pattern for getopt-based Options
*   [2025-10-16 17:41:59]
When testing code that uses getopt() for command-line parsing, always reset getopt's global state (optind, optarg, opterr) at the start of each test. Use optind=0 for GNU complete reinitialization. This prevents test execution order from affecting results and ensures clean state for each test case. Pattern applies to any code using getopt_long() or related POSIX command-line parsing functions.

---
## Phased Modular Extraction Pattern
*   [2025-10-09 17:06:35]
*   [2025-09-25 18:36:05]
Systematic approach to breaking down monolithic C++ code into focused modules. Uses incremental extraction with shared constants files (constants.hpp, midi_constants.hpp, device_constants.hpp) and single-responsibility modules (signal_handler, device_manager, midi_loader). Maintains backward compatibility while improving testability and maintainability.
---

---
## RAII Synchronization Wrapper Pattern
*   [2025-10-09 17:06:35]
*   [2025-10-01 14:07:44]
Pattern for wrapping C++ synchronization primitives (std::mutex + std::condition_variable) in a simple, type-safe RAII class. Key characteristics: (1) Encapsulates mutex and condition_variable with boolean predicate, (2) Provides simple wait() and notify() interface hiding locking complexity, (3) Automatic state reset after wait() for multi-cycle usage, (4) Deleted copy/move constructors to prevent synchronization issues, (5) Notify outside lock scope for performance. This pattern replaces error-prone manual synchronization with a clean, exception-safe interface. Applied in PlaybackSynchronizer class to replace POSIX semaphores.
---

---
## YAML-Driven Device Configuration Pattern
*   [2025-10-09 17:06:35]
*   [2025-09-25 18:35:56]
Factory pattern implementation with YAML configuration files for MIDI device management. Supports priority-based configuration discovery (current directory → user config → system config) and enables external device definition without code changes. Eliminates hardcoded device detection strings and MIDI channel mappings.
