# MIDIPlay Component Block Diagram

```mermaid
graph LR
    %% Main Application Entry Point (on the left)
    A[play.cpp<br/>Main Application] --> B[options.hpp]
    A --> H[midi_loader.cpp]
    A --> C[device_manager.cpp]
    A --> E[timing_manager.cpp]
    A --> F[playback_synchronizer.cpp]
    A --> D[playback_engine.cpp]
    A --> G[signal_handler.cpp]
    
    %% Configuration
    B[options.hpp<br/>Command Line Options] -->|Provides Configuration| A
    B -->|Configuration| H
    B -->|Configuration| C
    B -->|Configuration| E
    B -->|Configuration| F
    B -->|Configuration| D
    B -->|Configuration| G
    
    %% MIDI Loading (called first)
    H[midi_loader.cpp<br/>MIDI File Processing] -->|Loads MIDI| A
    
    %% Device Management (called second)
    C[device_manager.cpp<br/>Device Detection & Configuration] -->|Device Setup| A
    
    %% Timing Management (called third)
    E[timing_manager.cpp<br/>Time Tracking] -->|Provides Timing| A
    
    %% Synchronization (called fourth)
    F[playback_synchronizer.cpp<br/>Thread Sync] -->|Synchronizes| A
    
    %% Playback Engine (called fifth)
    D[playback_engine.cpp<br/>Playback Orchestration] -->|Controls Playback| A
    
    %% Signal Handling (called last)
    G[signal_handler.cpp<br/>Graceful Shutdown] -->|Handles Signals| A
    
    %% Styling
    classDef mainApp fill:#e1f5fe,stroke:#01579b,stroke-width:2px,color:#000
    classDef config fill:#fff3e0,stroke:#e65100,stroke-width:2px,color:#000
    classDef cppFile fill:#f3e5f5,stroke:#4a148c,stroke-width:2px,color:#000
    
    class A mainApp
    class B config
    class C,D,E,F,G,H cppFile
```

## Component Descriptions

### Main Application
- **play.cpp**: Entry point that orchestrates all components and manages the application flow

### Configuration
- **options.hpp**: Handles command-line argument parsing and provides configuration to other components

### Core Components (.cpp files)
- **device_manager.cpp**: Handles MIDI device detection, connection, and configuration
- **playback_engine.cpp**: Orchestrates MIDI playback including introduction, verses, and musical directions
- **timing_manager.cpp**: Tracks elapsed time and provides timing information
- **playback_synchronizer.cpp**: Provides thread synchronization for playback control
- **signal_handler.cpp**: Manages graceful shutdown on SIGINT (Ctrl+C) with emergency notes-off
- **midi_loader.cpp**: Responsible for loading, parsing, and extracting metadata from MIDI files