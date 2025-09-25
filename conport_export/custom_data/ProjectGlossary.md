# Custom Data: ProjectGlossary

### Allen Protégé-16

*   [2025-09-25 18:36:42]

```json
Primary target MIDI organ for this application. Church organ that receives MIDI commands via USB-to-MIDI adapter for automated hymn playback.
```

---
### Custom MIDI Meta Events

*   [2025-09-25 18:36:42]

```json
Sequencer-Specific Meta Event 0xFF7F with private event type 0x7D used for controlling verse count (0x01) and pause between verses (0x02). Enables single-verse MIDI files to specify playback behavior.
```

---
### DeviceManager

*   [2025-09-25 18:36:23]

```json
Modular class that handles MIDI device detection, connection, and device-specific setup using factory pattern with YAML configuration support. Replaces hardcoded device detection logic from play.cpp.
```

---
### Phase 2 Refactoring

*   [2025-09-25 18:36:42]

```json
Structural refactoring phase focused on extracting modules from monolithic play.cpp. Includes signal_handler, device_manager, midi_loader, and planned playback_engine/timing_manager extractions.
```

---
### YAML Configuration

*   [2025-09-25 18:36:42]

```json
External configuration system that allows defining MIDI device properties (detection strings, MIDI channels, bank/program settings) without code changes. Uses priority-based file discovery: current directory → user config → system config.
```
