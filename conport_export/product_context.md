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
1.5.3

## Key Features
*   Simple Linux command line interface
*   MIDI marker-based introduction definition
*   Configurable verse count via MIDI file or command line
*   Prelude and postlude modes without introduction
*   Custom MIDI meta events for playback control
*   YAML-driven device configuration system
*   Multi-device support (Casio, Yamaha, Allen)

## Architecture
{'main_executable': 'play command', 'core_modules': ['DeviceManager (YAML configuration)', 'MIDI file loading and parsing', 'Signal handling', 'Device-specific implementations'], 'configuration': 'YAML-based device configuration with priority discovery', 'patterns': ['Factory pattern for device creation', 'Modular architecture']}

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

