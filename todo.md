# Refactoring Plan: Decompose God Classes

This plan addresses two "God Class" code smells in `PlaybackEngine` and `MidiLoader`. We will break them down into smaller, more focused classes, each adhering to the Single Responsibility Principle.

---

## Part 1: Decompose `PlaybackEngine` (High Priority)

The `PlaybackEngine` will be broken down into a high-level `PlaybackOrchestrator` and several specialized handler classes.

### New Class Structure for Playback

1.  **`PlaybackOrchestrator`:** The new high-level coordinator.
2.  **`PlaybackStateMachine`:** Manages all playback-related state flags.
3.  **`MusicalDirector`:** Interprets MIDI meta-events and musical directions.
4.  **`RitardandoEffector`:** Handles the gradual slowing of tempo (ritardando).

### Refactoring Steps for `PlaybackEngine`

- [x] **Step 1: Create `PlaybackStateMachine.hpp/.cpp`** ✅
    - Define a class to hold the boolean state flags: `playingIntro_`, `ritardando_`, `lastVerse_`, `alFine_`.
    - Provide public getter and setter methods for each state.

- [x] **Step 2: Create `RitardandoEffector.hpp/.cpp`** ✅
    - Create a class that takes a `float& decrementRate`, `cxxmidi::player::PlayerSync&`, and `PlaybackStateMachine&` in its constructor. (The Ritardando Decrement rate, currently defined by the `RITARDANDO_DECREMENT` constant, may become configurable from the MIDI file and/or the command line in the future.)
    - Move the `heartbeatCallback()` logic into a `handleHeartbeat()` method.
    - Move the `HEARTBEAT_CHECK_INTERVAL` constant into this class.

- [x] **Step 3: Create `MusicalDirector.hpp/.cpp`** ✅
    - Create a class that takes a `cxxmidi::player::PlayerSync&`, `PlaybackStateMachine&`, and `const MidiLoader&` in its constructor.
    - Move the `eventCallback()` logic and its helpers into a `handleEvent(cxxmidi::Event&)` method.
    - Move musical direction constants (`INTRO_END`, etc.) into this class.

- [x] **Step 4: Refactor `PlaybackEngine` into `PlaybackOrchestrator`** ✅
    - Rename `PlaybackEngine` to `PlaybackOrchestrator`.
    - It will own instances of `PlaybackStateMachine`, `MusicalDirector`, and `RitardandoEffector`.
    - The `initialize()` method will be simplified to set up the callbacks, delegating to the new handler classes.

- [x] **Step 5: Refactor `main()` in `play.cpp`** ✅
    - Update `main` to instantiate `PlaybackOrchestrator`.

---

## Part 2: Decompose `MidiLoader` Logic (High Priority)

**Load Callback** The load callback is essential for filtering unwanted MIDI messages during the loading process as well as messages that are not needed after the loading process (e.g. custom Meta Events). The plan is to decompose the logic *within* the callback into a dedicated event pre-processor class.

### New Class Structure for MIDI Loading

1.  **`MidiLoader` (Refined):** This class will still manage the file loading via `cxxmidi::File::Load` and will host the `loadCallback`. It will own an instance of the new `EventPreProcessor`. Its collected data (title, verses, etc.) will be retrieved from the `EventPreProcessor` after loading is complete.

2.  **`EventPreProcessor` (New):** This class will contain the core logic from the original `loadCallback`. It will be responsible for both filtering events (deciding if they should be loaded) and extracting metadata (Title, Key Signature, Verses, etc.).

### Refactoring Steps for `MidiLoader`

- [x] **Step 6: Create `EventPreProcessor.hpp/.cpp`** ✅
    - Create a new `EventPreProcessor` class. It will hold the state variables for extracted data (`title_`, `keySignature_`, `verses_`, `introSegments_`, etc.).
    - Create a primary public method: `bool processEvent(cxxmidi::Event& event)`. This method will contain the entire body of the original `MidiLoader::loadCallback`.
    - Move the helper methods (`processTempoEvent`, `processKeySignatureEvent`, etc.) from `MidiLoader` into `EventPreProcessor` as private helper methods.
    - Add public getters to this class for retrieving the extracted data (`getTitle()`, `getVerses()`, etc.).
    - Ensure that custom Meta Events are discarded (Fixes GitHub Issue #21).

- [x] **Step 7: Refine the `MidiLoader` Class** ✅
    - `MidiLoader` will now have a member variable `std::unique_ptr<EventPreProcessor> eventProcessor_`.
    - The `MidiLoader::loadFile` method will create a new instance of `EventPreProcessor`.
    - The lambda for `midiFile_.SetCallbackLoad` will be simplified to a one-line call: `[this](auto& e){ return eventProcessor_->processEvent(e); }`.
    - After `midiFile_.Load()` completes, `MidiLoader` will expose the extracted data by adding getter methods that simply forward the call to the `eventProcessor_` instance (e.g., `const std::string& MidiLoader::getTitle() const { return eventProcessor_->getTitle(); }`).
    - Remove the data state and processing logic that has been moved to `EventPreProcessor`.

- [x] **Step 8: Update Consumers** ✅
    - For consumers like `main()` and `PlaybackOrchestrator`, no significant changes should be needed, as the public interface of `MidiLoader` (its getters) will remain the same. This refactoring is now mostly internal to the MIDI loading subsystem.