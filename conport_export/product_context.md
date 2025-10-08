# Product Context
## Project Name
Organ Pi 🎹 MIDI File Player

## Description
C++ MIDI sequencer for Raspberry Pi that plays MIDI files through USB-to-MIDI converters to organs and keyboards

## Primary Purpose
Software MIDI sequencer designed specifically for church organ applications, particularly Allen Protégé-16 organs

## Target Platform
Raspberry Pi 4B+ running Debian 12 (Bookworm) or Raspberry Pi OS 64-bit

## Supported Devices
{'primary': 'Allen Protégé-16 organ', 'testing': ['Casio consumer keyboards', 'Yamaha consumer keyboards'], 'future': 'Other Allen organ models (untested)'}

## Technical Stack
{'language': 'C++ (gnu++20 standard)', 'dependencies': ['cxxmidi (customized fork)', 'yaml-cpp library', 'libyaml-cpp0.7'], 'build_system': 'Standard C++ compilation', 'libraries': 'Standard library + custom MIDI handling'}

## Current Version
1.5.6-dev (Phase 3 at 98% completion)

## Key Features
*   Simple Linux command line interface
*   MIDI marker-based introduction definition
*   Configurable verse count via MIDI file or command line
*   Prelude and postlude modes without introduction
*   Custom MIDI meta events for playback control
*   YAML-driven device configuration system
*   Multi-device support (Casio, Yamaha, Allen)

## Architecture
{'main_executable': 'play command', 'core_modules': ['DeviceManager (YAML configuration)', 'PlaybackEngine (playback orchestration and callbacks)', 'TimingManager (tempo, speed, timing calculations)', 'SignalHandler (Ctrl+C graceful shutdown)', 'PlaybackSynchronizer (modern C++ thread synchronization)', 'MidiLoader (MIDI file loading and parsing)'], 'configuration': 'YAML-based device configuration with priority discovery', 'patterns': ['Factory pattern for device creation', 'Dependency injection throughout', 'RAII-based resource management', 'Modern C++ synchronization primitives', 'Modular architecture with single responsibility'], 'phase_3_achievements': ['Eliminated ALL global variables from main program (except static version string)', 'Modernized synchronization from POSIX semaphores to C++ std::condition_variable', 'Improved portability by removing POSIX-specific dependencies', 'Enhanced exception safety through RAII design patterns', 'Main function reduced to clean orchestration code (~145 lines)']}

## System Requirements
{'hardware': 'Raspberry Pi 4B or better (Orange Pi 5 tested)', 'os': '64-bit Debian 12 (Bookworm) or later', 'memory': '4GB RAM (may work with less)', 'storage': 'NVME SSD recommended, fast SD card acceptable', 'connectivity': 'USB-to-MIDI adapter'}

## Custom Midi Features
{'meta_events': 'Sequencer-Specific Meta Event 0xFF7F with private event type 0x7D', 'verse_control': 'Event type 0x01 for number of verses (1-9)', 'pause_control': 'Event type 0x02 for pause between verses (16-bit tick count)', 'deprecated_events': 'FF 0x10 and FF 0x11 (legacy verse and pause control)'}

## Installation
{'method': 'Debian package installer', 'current_installer_version': 'v1.5.0', 'package_format': 'tar.gz and zip archives with install.sh script', 'dependencies_managed': 'Automatic libyaml-cpp0.7 installation'}

## Target Users
People with technical aptitude using church organs

## Future Plans
More user-friendly touch-screen interface in development

## Repository Context
Active development with structured refactoring plan

## Refactoring Status
{'phase_1': 'COMPLETED - Constants extraction and code organization', 'phase_2': 'COMPLETED - Module extraction (Signal Handler, DeviceManager, MidiLoader, PlaybackEngine, TimingManager)', 'phase_3': '98% COMPLETE - Main refactoring and architectural cleanup', 'phase_3_completed_tasks': ['Eliminated ALL global variables from main program (except static version string)', 'Modernized synchronization from POSIX semaphores to C++ std::condition_variable', 'Removed namespace pollution (using namespace directives)', 'Comprehensive code smell review and critical fixes', 'Complete i18n coverage for all user-facing strings', 'Replaced deprecated POSIX APIs with modern C++ equivalents'], 'remaining_phase_3_tasks': ['Eliminate remaining code duplication (midi markers, etc.)', 'Refactor oversized methods (Options::parse)', 'Final code smell remediation (16 documented issues)', 'Document TODO features (optional)']}

## Code Quality
{'analysis_completed': '2025-10-07', 'total_issues_identified': 22, 'issues_fixed': 15, 'high_priority_complete': '100% (5/5)', 'medium_priority_complete': '100% (10/10) - ALL MEDIUM PRIORITY ISSUES RESOLVED!', 'low_priority_remaining': '7 polish items', 'documentation': 'CODE_SMELLS.md - comprehensive analysis with priorities and recommendations', 'milestone_achieved': 'All high and medium priority code smells resolved - only low-priority style/convention improvements remain', 'recent_fixes': ['Issue #6: Device key constants extracted and helper methods created', 'Issue #7: Musical marker constants centralized in midi_markers.hpp', 'Issue #8: Error handling standardized in DeviceManager (exceptions for unrecoverable errors)', 'Issue #9: Boolean flag replaced with std::optional<YamlConfig> for clearer semantics', 'Issue #10: Refactored oversized Options::parse() method into focused helper methods', 'Issue #11: Key signature offsets already defined (verified and marked complete)', 'Issue #12: Removed unnecessary TimeSignature message size check', 'Issue #13: Dead code - unused variable assignment (manually fixed)', 'Issue #14: Placeholder/TODO code (manually fixed)', 'Issue #15: Debug output wrapped in _() macros for full i18n consistency']}

