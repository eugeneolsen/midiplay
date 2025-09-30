# PlaybackEngine and TimingManager Extraction - Architectural Design

## Overview

This document details the extraction of playback control and timing functionality from [`play.cpp`](play.cpp) into two new modules as specified in Phase 2, Item 4 of [`refactor.md`](refactor.md:396).

## Design Philosophy

Following the architectural benefits outlined in refactor.md (lines 404-413):
- **Single Responsibility**: Each module has one clear purpose
- **Testability**: Independent unit testing capability
- **Maintainability**: Localized changes and clear boundaries
- **Reusability**: Modules could be reused in other MIDI applications

## Module 1: TimingManager

### Responsibility
Manages all time-tracking and elapsed time calculations for playback sessions.

### Code Location in play.cpp
- Lines 59-60: `startTime` and `endTime` variables
- Lines 268: Start timer initialization
- Lines 343-352: Elapsed time calculation and display

### Class Interface

```cpp
namespace MidiPlay {

/**
 * @brief Manages timing and elapsed time tracking for MIDI playback
 * 
 * This class encapsulates all time-tracking functionality including:
 * - Session start/end time recording
 * - Elapsed time calculation
 * - Formatted time display (MM:SS format)
 */
class TimingManager {
public:
    /**
     * @brief Constructor
     */
    TimingManager();
    
    /**
     * @brief Start the playback timer
     * Records the current time as the session start
     */
    void startTimer();
    
    /**
     * @brief End the playback timer
     * Records the current time as the session end
     */
    void endTimer();
    
    /**
     * @brief Get elapsed time in seconds
     * @return Elapsed time as double (seconds)
     */
    double getElapsedSeconds() const;
    
    /**
     * @brief Get formatted elapsed time string
     * @return Formatted string in "MM:SS" format
     */
    std::string getFormattedElapsedTime() const;
    
    /**
     * @brief Display elapsed time to console
     * Outputs "Fine - elapsed time MM:SS" message
     */
    void displayElapsedTime() const;
    
    /**
     * @brief Get start time (for SignalHandler compatibility)
     * @return Reference to start time point
     */
    const std::chrono::time_point<std::chrono::high_resolution_clock>& getStartTime() const {
        return startTime_;
    }

private:
    std::chrono::time_point<std::chrono::high_resolution_clock> startTime_;
    std::chrono::time_point<std::chrono::high_resolution_clock> endTime_;
    
    /**
     * @brief Helper to format elapsed seconds into MM:SS
     * @param seconds Total elapsed seconds
     * @return Formatted string
     */
    static std::string formatTime(int totalSeconds);
};

} // namespace MidiPlay
```

### Dependencies
- `<chrono>` - Time point and duration management
- `<string>` - Formatted output
- `<iomanip>` - Formatting utilities
- [`constants.hpp`](constants.hpp) - `SECONDS_PER_MINUTE`

### Benefits
- **Reusable**: Can be used by SignalHandler for interrupt time display
- **Testable**: Easy to unit test time calculations
- **Clear API**: Simple start/stop/display interface
- **Encapsulated**: All timing logic in one place

---

## Module 2: PlaybackEngine

### Responsibility
Manages complete MIDI playback orchestration including callbacks, intro/verse management, and musical direction handling (ritardando, D.C. al Fine, etc.)

### Code Location in play.cpp
- Lines 37-46: Playback-specific constants and markers
- Lines 47-55: Playback state variables
- Lines 64-68: Finished callback
- Lines 179-193: Heartbeat callback (ritardando logic)
- Lines 199-265: Event callback (intro jumping, ritardando, D.C. al Fine, Fine)
- Lines 274-340: Complete intro and verse playback loop

### Class Interface

```cpp
namespace MidiPlay {

/**
 * @brief Manages complete MIDI playback orchestration
 * 
 * This class encapsulates all playback control logic including:
 * - Player callback setup and management
 * - Introduction segment playback with marker-based jumping
 * - Multi-verse playback with inter-verse pausing
 * - Ritardando (gradual slowdown) control
 * - Musical direction handling (D.C. al Fine, Fine markers)
 * - Playback state management
 */
class PlaybackEngine {
public:
    /**
     * @brief Constructor with dependency injection
     * @param player Reference to cxxmidi PlayerSync instance (already contains outport)
     * @param semaphore Reference to semaphore for synchronization
     * @param midiLoader Reference to MidiLoader for file metadata
     */
    PlaybackEngine(cxxmidi::player::PlayerSync& player,
                   sem_t& semaphore,
                   const MidiLoader& midiLoader);
    
    /**
     * @brief Destructor
     */
    ~PlaybackEngine() = default;
    
    // Disable copy/move
    PlaybackEngine(const PlaybackEngine&) = delete;
    PlaybackEngine& operator=(const PlaybackEngine&) = delete;
    
    /**
     * @brief Initialize player with callbacks and base tempo
     * @param baseSpeed Base playback speed multiplier from command line
     */
    void initialize(float baseSpeed);
    
    /**
     * @brief Execute complete playback sequence (intro + verses)
     * Main orchestration method that handles:
     * - Optional introduction playback
     * - Verse iteration with pausing
     * - State management across playback sections
     */
    void executePlayback();
    
    /**
     * @brief Get whether warnings should be displayed
     */
    void setDisplayWarnings(bool display) { displayWarnings_ = display; }

private:
    // === Dependency References ===
    cxxmidi::player::PlayerSync& player_;  // Contains outport internally
    sem_t& semaphore_;
    const MidiLoader& midiLoader_;
    
    // === Playback State Variables ===
    bool playingIntro_;
    bool ritardando_;
    bool lastVerse_;
    bool alFine_;           // "Da Capo al Fine" flag
    bool displayWarnings_;
    float baseSpeed_;       // Base tempo multiplier
    float baseTempo_;       // Original player tempo
    
    // Introduction segment iterator
    std::vector<IntroductionSegment>::const_iterator currentIntroSegment_;
    
    // === Callback Implementation Methods ===
    /**
     * @brief Heartbeat callback - handles ritardando logic
     * Called periodically during playback to apply gradual speed reduction
     */
    void heartbeatCallback();
    
    /**
     * @brief Event callback - handles meta-events and musical directions
     * @param event MIDI event being processed
     * @return true to send event to device, false to suppress
     */
    bool eventCallback(cxxmidi::Event& event);
    
    /**
     * @brief Finished callback - signals playback completion
     */
    void finishedCallback();
    
    // === Playback Section Methods ===
    /**
     * @brief Play introduction section with marker-based jumping
     */
    void playIntroduction();
    
    /**
     * @brief Play all verses with inter-verse pausing
     */
    void playVerses();
    
    /**
     * @brief Reset playback state for new section
     */
    void resetPlaybackState();
    
    /**
     * @brief Set player speed (combines base tempo and speed multiplier)
     */
    void setPlayerSpeed(float speedMultiplier);
    
    // === Event Processing Helpers ===
    /**
     * @brief Process introduction marker events (jumping logic)
     * @param message MIDI meta message
     */
    void processIntroMarker(const cxxmidi::Message& message);
    
    /**
     * @brief Process ritardando marker
     * @param message MIDI meta message
     */
    void processRitardandoMarker(const cxxmidi::Message& message);
    
    /**
     * @brief Process D.C. al Fine marker
     * @param message MIDI meta message
     * @return false to suppress event output
     */
    bool processDCAlFineMarker(const cxxmidi::Message& message);
    
    /**
     * @brief Process Fine marker (when in al Fine mode)
     * @param message MIDI meta message
     * @return false to suppress event output
     */
    bool processFineMarker(const cxxmidi::Message& message);
    
    // === Constants (module-specific, not shared) ===
    static constexpr int64_t HEARTBEAT_CHECK_INTERVAL = 100000;
    static constexpr float RITARDANDO_DECREMENT = 0.002f;
    static constexpr int VERSE_DISPLAY_OFFSET = 1;
    
    // Musical direction markers (from MidiLoader but used here)
    static constexpr const char* INTRO_END = "]";
    static constexpr const char* RITARDANDO_INDICATOR = R"(\)";
    static constexpr const char* D_C_AL_FINE = "D.C. al Fine";
    static constexpr const char* FINE_INDICATOR = "Fine";
};

} // namespace MidiPlay
```

### Dependencies
- `<cxxmidi/player/player_sync.hpp>` - Player control (contains output internally)
- `<cxxmidi/event.hpp>`, `<cxxmidi/message.hpp>` - Event handling
- `<semaphore.h>` - Synchronization
- `<unistd.h>` - `usleep()` for inter-verse pausing
- [`midi_loader.hpp`](midi_loader.hpp) - File metadata and intro segments
- [`constants.hpp`](constants.hpp) - `SECONDS_PER_MINUTE`, `DEFAULT_VERSES`

### Benefits
- **Complete Encapsulation**: All playback logic in one cohesive class
- **Single Responsibility**: Manages playback orchestration only
- **Testability**: Can be unit tested with mock player/semaphore
- **State Management**: Clear playback state tracking
- **Callback Encapsulation**: Lambda captures eliminated, proper method binding

---

## Module Interactions

### Dependency Flow

```
main.cpp
  ├─> TimingManager (startTimer, displayElapsedTime)
  ├─> PlaybackEngine (initialize, executePlayback)
  └─> SignalHandler (uses TimingManager.getStartTime() and outport for emergency notes-off)

PlaybackEngine
  ├─> MidiLoader (getIntroSegments, getVerses, etc.)
  ├─> player::PlayerSync (contains outport internally)
  └─> sem_t (synchronization)

TimingManager
  └─> (independent, no dependencies on other modules)
```

### Call Sequence

```
main():
  1. Create MidiLoader, load MIDI file
  2. Create DeviceManager, connect device
  3. Create player::PlayerSync
  4. Create TimingManager
  5. timingManager.startTimer()
  6. Create PlaybackEngine(player, sem, midiLoader)
  7. playbackEngine.initialize(speed)
  8. Create SignalHandler(outport, sem, timingManager.getStartTime())
  9. playbackEngine.executePlayback()  // Blocks until completion
  10. timingManager.endTimer()
  11. timingManager.displayElapsedTime()
```

---

## Code Extraction Mapping

### From play.cpp to TimingManager

| play.cpp Lines | Destination | Notes |
|---------------|-------------|-------|
| 59-60 | `TimingManager` private members | `startTime_`, `endTime_` |
| 268 | `TimingManager::startTimer()` | Single line moved to method |
| 343 | `TimingManager::endTimer()` | Single line moved to method |
| 346-352 | `TimingManager::displayElapsedTime()` | Complete calculation and display logic |

### From play.cpp to PlaybackEngine

| play.cpp Lines | Destination | Notes |
|---------------|-------------|-------|
| 37-46 | `PlaybackEngine` static constants | `HEARTBEAT_CHECK_INTERVAL`, markers, etc. |
| 47-55 | `PlaybackEngine` private members | State variables |
| 64-68 | `PlaybackEngine::finishedCallback()` | Callback method |
| 179-193 | `PlaybackEngine::heartbeatCallback()` | Ritardando logic |
| 199-265 | `PlaybackEngine::eventCallback()` | Complete event handling |
| 274-300 | `PlaybackEngine::playIntroduction()` | Introduction playback section |
| 303-340 | `PlaybackEngine::playVerses()` | Verse playback loop |
| - | `PlaybackEngine::executePlayback()` | New orchestration method calling intro + verses |

### Remaining in play.cpp

| Lines | Content | Reason to Keep |
|-------|---------|----------------|
| 1-33 | Includes and setup | Application entry point concerns |
| 75-176 | Main function setup | Orchestration logic, option parsing, file loading |
| 196 | `player.SetCallbackFinished()` | Moved to `PlaybackEngine::initialize()` |

---

## Implementation Strategy

### Phase 1: Create TimingManager (Simpler, Independent)
1. Create `timing_manager.hpp` with class declaration
2. Create `timing_manager.cpp` with implementation
3. Update `play.cpp` to use `TimingManager`
4. Update `signal_handler.cpp` to use `TimingManager::getStartTime()`

### Phase 2: Create PlaybackEngine (More Complex)
1. Create `playback_engine.hpp` with class declaration
2. Create `playback_engine.cpp` with implementation
   - Extract callback methods
   - Extract playback orchestration methods
3. Update `play.cpp` to use `PlaybackEngine`
4. Remove extracted code from `play.cpp`

### Phase 3: Testing and Validation
1. Compile and verify no build errors
2. Test introduction playback
3. Test multi-verse playback
4. Test ritardando functionality
5. Test D.C. al Fine / Fine markers
6. Test signal handling (Ctrl+C) integration

---

## Expected Result

### New main.cpp Structure (~80 lines)

```cpp
int main(int argc, char **argv) {
    // 1. Initialization
    sem_init(&sem, 0, 0);
    Options options(argc, argv);
    // Parse options...
    
    // 2. Load MIDI file
    MidiLoader midiLoader;
    midiLoader.loadFile(path, options);
    
    // 3. Connect device
    DeviceManager deviceManager;
    deviceManager.connectAndDetectDevice(outport);
    deviceManager.createAndConfigureDevice(...);
    
    // 4. Setup player
    player::PlayerSync player(&outport);
    player.SetFile(&midiLoader.getFile());
    
    // 5. Create timing manager
    TimingManager timingManager;
    timingManager.startTimer();
    
    // 6. Create playback engine
    PlaybackEngine playbackEngine(player, sem, midiLoader);
    playbackEngine.initialize(speed);
    playbackEngine.setDisplayWarnings(displayWarnings);
    
    // 7. Setup signal handler
    SignalHandler signalHandler(outport, sem, timingManager.getStartTime());
    signalHandler.setupSignalHandler();
    
    // 8. Execute playback (single call!)
    playbackEngine.executePlayback();
    
    // 9. Display completion
    timingManager.endTimer();
    timingManager.displayElapsedTime();
    
    // 10. Cleanup
    sem_destroy(&sem);
}
```

---

## Benefits Summary

### Architectural Improvements
✅ **Single Responsibility**: Each module has one clear purpose  
✅ **Testability**: Both modules can be unit tested independently  
✅ **Maintainability**: Playback logic localized to PlaybackEngine  
✅ **Reusability**: TimingManager is a general utility class  
✅ **Reduced Complexity**: main() becomes pure orchestration (80 vs 355 lines)  
✅ **Clear Boundaries**: Well-defined interfaces and dependencies  

### Code Quality Improvements
✅ **Eliminated Global Variables**: All state encapsulated in classes  
✅ **Eliminated Lambda Captures**: Callbacks are proper class methods  
✅ **Better Synchronization**: Clearer ownership of semaphore usage  
✅ **Improved Readability**: Named methods vs. inline lambdas  

### Testing Improvements
✅ **Mock-Friendly**: Can inject mock player/semaphore for testing  
✅ **Isolated Testing**: Each module can be tested independently  
✅ **State Testing**: Can verify playback state transitions  

---

## Alignment with refactor.md Goals

This extraction completes **Phase 2, Item 4** as specified in [`refactor.md`](refactor.md:396):
> 4. `playback_engine.cpp` and `timing_manager.cpp`

And directly supports **Phase 3** goals (lines 399-403):
> Phase 3: Refactor Main (Future)
> - Orchestration-only main function
> - Dependency injection between modules
> - Clean separation of concerns

This design achieves all 7 architectural benefits listed in refactor.md (lines 404-413).

---

## Next Steps

1. ✅ Review and approve this architectural design
2. Create implementation files following this specification
3. Update play.cpp to use new modules
4. Test thoroughly with multiple hymns
5. Update refactor.md to mark Phase 2, Item 4 as complete
6. Log decision in ConPort for future reference