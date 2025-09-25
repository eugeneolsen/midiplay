# System Patterns

---
## Phased Modular Extraction Pattern
*   [2025-09-25 18:36:05]
Systematic approach to breaking down monolithic C++ code into focused modules. Uses incremental extraction with shared constants files (constants.hpp, midi_constants.hpp, device_constants.hpp) and single-responsibility modules (signal_handler, device_manager, midi_loader). Maintains backward compatibility while improving testability and maintainability.

---
## YAML-Driven Device Configuration Pattern
*   [2025-09-25 18:35:56]
Factory pattern implementation with YAML configuration files for MIDI device management. Supports priority-based configuration discovery (current directory → user config → system config) and enables external device definition without code changes. Eliminates hardcoded device detection strings and MIDI channel mappings.
