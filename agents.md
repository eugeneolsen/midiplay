---
name: "MidiPlay"
description: "Organ Pi MIDI Player"
category: "Project Category"
author: "Eugene C. Olsen"
authorUrl: "https://github.com/eugeneolsen" # Optional field
tags: ["C++", "MIDI", "cxxmidi", "custom MIDI player"]
lastUpdated: "2025-09-25"
---

# Organ Pi MIDI Player

## Project Overview

The Organ Pi MIDI file player is a software MIDI sequencer that plays a MIDI file through MIDI OUT port 1 through a USB-to-MIDI converter.  This program was developed specifically to run on a Raspberry Pi 4B (or better) running Raspberry Pi OS 64-bit (or any Debian Linux derivative, such as Ubuntu) through a USB to MIDI adapter to an Allen Protégé-16 organ.  Other organs may be supported in the future.

## Tech Stack

The player is a Linux binary that is invoked as the `play` command from a bash command line.

## Development Guidelines

### Code Style

- Use consistent code formatting tools
- Follow language-specific best practices
- Keep code clean and readable

### Naming Conventions

- File naming: TBD
- Variable naming: TBD
- Function naming: TBD
- Class naming: TBD

### Git Workflow

- Branch naming conventions: TBD
- Commit message format: TBD
- Pull Request process: TBD

## Environment Setup

### Development Requirements

- C++ standard: gnu++20
