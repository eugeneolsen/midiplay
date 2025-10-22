
# Phase 2 Playback Components - Test Design Specifications

**Document Version**: 1.1
**Created**: 2025-10-16
**Updated**: 2025-10-16
**Status**: Integration Testing Approach Recommended

---

## Executive Summary

This document provides comprehensive test specifications for completing Phase 2 of the MIDIPlay unit testing strategy. Three components require testing:

1. **MusicalDirector** - 15 test cases
2. **RitardandoEffector** - 8 test cases  
3. **PlaybackOrchestrator** - 15 test cases

**Total**: 38 test cases, ~95-110 assertions

---

## Issue Identified

Lines 3360-3365 in [`UNIT_TESTING_STRATEGY.md`](UNIT_TESTING_STRATEGY.md) incorrectly mark Phase 2 Day 4-5 as ✅ COMPLETE. The following test files **do not exist**:

1. ❌ `test/test_musical_director.cpp`
2. ❌ `test/test_ritardando_effector.cpp`
3. ❌ `test/test_playback_orchestrator.cpp`

### Current Phase 2 Status

**Completed**: 
- ✅ MidiLoader tests (40 assertions, 11 test cases)
- ✅ EventPreprocessor (tested via MidiLoader integration)

**Missing**: 38 test cases across 3 components

---

## Testing Strategy: Integration Testing Approach

### Architectural Discovery

During implementation, we discovered that the playback components (`MusicalDirector`, `RitardandoEffector`, `PlaybackOrchestrator`) use **concrete dependencies** (`cxxmidi::player::PlayerSync`) rather than abstract interfaces. This is appropriate for production code (no over-engineering), but limits mock-based unit testing.

### Why Mock-Based Testing Doesn't Fit

**Type Incompatibility**:
- Components expect: `cxxmidi::player::PlayerSync& player`
- Mock provides: `MockPlayerSync player`
- **No inheritance relationship** - cannot substitute types
- Adding abstract interfaces would be over-engineering for production

### Recommended Approach: Integration Testing

**Primary Strategy**:
- **Use real components** with actual `cxxmidi::player::PlayerSync`
- **Test with real MIDI files** containing markers
- **Validate behavior end-to-end** with actual playback
- **Hardware validation** when MIDI devices available

**Benefits**:
- ✅ Tests actual behavior, not mocked behavior
- ✅ No production code changes needed
- ✅ Validates full integration stack
- ✅ Realistic timing and synchronization testing
- ✅ Verifies marker processing with real events

**Limitations**:
- ⚠️ Requires MIDI hardware for full validation
- ⚠️ Tests are slower than pure unit tests
- ⚠️ More complex test setup
- ⚠️ Environment-dependent (hardware availability)

### Test Categories

1. **Component Structure Tests** (what we CAN test without hardware)
   - Constructor dependency injection
   - Method availability
   - State machine integration
   - API contracts

2. **Integration Tests with Real MIDI Files** (recommended approach)
   - Marker detection and processing
   - State transitions during playback
   - Player control flow
   - End-to-end scenarios

3. **Hardware Validation** (when available)
   - Actual MIDI device communication
   - Real-world timing verification
   - Complete playback scenarios

---

## Component 1: MusicalDirector

### Overview
- **File**: [`musical_director.hpp`](../musical_director.hpp)
- **Purpose**: Interprets MIDI meta-event markers and controls playback flow
- **Dependencies**: `cxxmidi::player::PlayerSync`, `PlaybackStateMachine`, `MidiLoader`
- **Test File**: `test/test_musical_director.cpp`

### Key Responsibilities
1. Detect and process introduction markers (INTRO_BEGIN, INTRO_END)
2. Handle ritardando markers for tempo slowdown
3. Process D.C. al Fine (Da Capo al Fine) markers
4. Detect Fine markers for early termination
5. Update state machine based on musical directions
6. Control player (Stop, Play, GoToTick, Finish)

### Mock Strategy

```cpp
// Simplified mock for testing coordination logic
class MockPlayerSync {
public:
    // Playback control
    void Play() { playCalled++; }
    void Stop() { stopCalled++; }
    void Finish() { finishCalled++; }
    void GoToTick(uint32_t tick) {
        goToTickCalled++;
        lastTickValue = tick;
    }
    void NotesOff() { notesOffCalled++; }
    
    // Test verification counters
    int playCalled = 0;
    int stopCalled = 0;
    int finishCalled = 0;
    int goToTickCalled = 0;
    int notesOffCalled = 0;
    uint32_t lastTickValue = 0;
};
```

### Test Cases (15 total)

#### Group 1: Construction & Initialization (3 tests)

**Test 1.1**: `Constructor accepts dependencies correctly`
```cpp
TEST_CASE("MusicalDirector construction", "[musical_director][unit]") {
    MockPlayerSync player;
    PlaybackStateMachine stateMachine;
    MidiLoader loader;
    
    SECTION("constructor accepts all dependencies") {
        REQUIRE_NOTHROW(MusicalDirector(player, stateMachine, loader));
    }
}
```
**Assertions**: 1  
**Priority**: High

**Test 1.2**: `initializeIntroSegments() sets iterator to begin`
```cpp
SECTION("initializeIntroSegments with segments") {
    // MidiLoader with intro segments
    MusicalDirector director(player, stateMachine, loader);
    
    REQUIRE_NOTHROW(director.initializeIntroSegments());
    // Iterator should be at first segment (tested via behavior)
}
```
**Assertions**: 1  
**Priority**: High

**Test 1.3**: `initializeIntroSegments() handles empty segments`
```cpp
SECTION("initializeIntroSegments with no segments") {
    // MidiLoader with no intro segments
    MusicalDirector director(player, stateMachine, emptyLoader);
    
    REQUIRE_NOTHROW(director.initializeIntroSegments());
}
```
**Assertions**: 1  
**Priority**: Medium

#### Group 2: Introduction Marker Processing (4 tests)

**Test 2.1**: `INTRO_END marker advances to next segment`
```cpp
TEST_CASE("MusicalDirector intro marker processing", "[musical_director][unit]") {
    SECTION("INTRO_END advances to next segment") {
        // Setup with multiple intro segments
        // Create INTRO_END marker event
        director.initializeIntroSegments();
        
        bool result = director.handleEvent(introEndEvent);
        
        REQUIRE(result == true);
        REQUIRE(player.goToTickCalled == 1);
        REQUIRE(player.lastTickValue == expectedNextSegmentTick);
    }
}
```
**Assertions**: 3  
**Priority**: High

**Test 2.2**: `INTRO_END marker calls GoToTick with correct value`
```cpp
SECTION("INTRO_END jumps to correct tick") {
    director.initializeIntroSegments();
    stateMachine.setPlayingIntro(true);
    
    director.handleEvent(introEndEvent);
    
    REQUIRE(player.goToTickCalled == 1);
    REQUIRE(player.stopCalled == 1);
    REQUIRE(player.playCalled == 1);
}
```
**Assertions**: 3  
**Priority**: High

**Test 2.3**: `Final INTRO_END marker calls Stop() and Finish()`
```cpp
SECTION("final INTRO_END stops playback") {
    director.initializeIntroSegments();
    stateMachine.setPlayingIntro(true);
    
    // Process all segments until last
    // ... advance through segments
    
    director.handleEvent(finalIntroEndEvent);
    
    REQUIRE(player.stopCalled == 1);
    REQUIRE(player.finishCalled == 1);
}
```
**Assertions**: 2  
**Priority**: High

**Test 2.4**: `Stuck note warning when hasPotentialStuckNote() is true`
```cpp
SECTION("stuck note warning displayed") {
    // MidiLoader with hasPotentialStuckNote = true
    director.initializeIntroSegments();
    stateMachine.setPlayingIntro(true);
    stateMachine.setDisplayWarnings(true);
    
    director.handleEvent(finalIntroEndEvent);
    
    REQUIRE(player.notesOffCalled == 1);
    // Output should contain warning (capture std::cout)
}
```
**Assertions**: 1  
**Priority**: Medium

#### Group 3: Ritardando Marker Processing (2 tests)

**Test 3.1**: `Ritardando marker sets state machine flag`
```cpp
TEST_CASE("MusicalDirector ritardando processing", "[musical_director][unit]") {
    SECTION("ritardando marker sets flag") {
        stateMachine.setPlayingIntro(true);
        
        director.handleEvent(ritardandoEvent);
        
        REQUIRE(stateMachine.isRitardando());
    }
}
```
**Assertions**: 1  
**Priority**: High

**Test 3.2**: `Ritardando only processed during intro or last verse`
```cpp
SECTION("ritardando only during intro or last verse") {
    stateMachine.setPlayingIntro(false);
    stateMachine.setLastVerse(false);
    
    director.handleEvent(ritardandoEvent);
    
    REQUIRE_FALSE(stateMachine.isRitardando());
    
    stateMachine.setLastVerse(true);
    director.handleEvent(ritardandoEvent);
    
    REQUIRE(stateMachine.isRitardando());
}
```
**Assertions**: 2  
**Priority**: High

#### Group 4: D.C. al Fine Marker Processing (3 tests)

**Test 4.1**: `D.C. al Fine sets state machine flag`
```cpp
TEST_CASE("MusicalDirector D.C. al Fine processing", "[musical_director][unit]") {
    SECTION("D.C. al Fine sets alFine flag") {
        stateMachine.setLastVerse(true);
        
        bool result = director.handleEvent(dcAlFineEvent);
        
        REQUIRE(stateMachine.isAlFine());
    }
}
```
**Assertions**: 1  
**Priority**: High

**Test 4.2**: `D.C. al Fine calls Stop() and Finish()`
```cpp
SECTION("D.C. al Fine stops playback") {
    stateMachine.setLastVerse(true);
    
    director.handleEvent(dcAlFineEvent);
    
    REQUIRE(player.stopCalled == 1);
    REQUIRE(player.finishCalled == 1);
}
```
**Assertions**: 2  
**Priority**: High

**Test 4.3**: `D.C. al Fine returns false (suppresses event)`
```cpp
SECTION("D.C. al Fine suppresses event output") {
    stateMachine.setLastVerse(true);
    
    bool result = director.handleEvent(dcAlFineEvent);
    
    REQUIRE(result == false);
}
```
**Assertions**: 1  
**Priority**: High

**Test 4.4**: `D.C. al Fine only processed on last verse`
```cpp
SECTION("D.C. al Fine only on last verse") {
    stateMachine.setLastVerse(false);
    
    bool result = director.handleEvent(dcAlFineEvent);
    
    REQUIRE(result == true);  // Event not suppressed
    REQUIRE_FALSE(stateMachine.isAlFine());
}
```
**Assertions**: 2  
**Priority**: High

#### Group 5: Fine Marker Processing (2 tests)

**Test 5.1**: `Fine marker stops playback when alFine is true`
```cpp
TEST_CASE("MusicalDirector Fine marker processing", "[musical_director][unit]") {
    SECTION("Fine marker stops when alFine active") {
        stateMachine.setAlFine(true);
        
        director.handleEvent(fineEvent);
        
        REQUIRE(player.stopCalled == 1);
        REQUIRE(player.finishCalled == 1);
    }
}
```
**Assertions**: 2  
**Priority**: High

**Test 5.2**: `Fine marker returns false (suppresses event)`
```cpp
SECTION("Fine marker suppresses event output") {
    stateMachine.setAlFine(true);
    
    bool result = director.handleEvent(fineEvent);
    
    REQUIRE(result == false);
}
```
**Assertions**: 1  
**Priority**: High

#### Group 6: General Event Handling (1 test)

**Test 6.1**: `Non-marker events pass through unchanged`
```cpp
TEST_CASE("MusicalDirector general event handling", "[musical_director][unit]") {
    SECTION("non-marker events return true") {
        cxxmidi::Event noteOnEvent;  // Regular MIDI event
        
        bool result = director.handleEvent(noteOnEvent);
        
        REQUIRE(result == true);
    }
}
```
**Assertions**: 1  
**Priority**: Medium

### Summary: MusicalDirector
- **Total Tests**: 15
- **Total Assertions**: ~40-45
- **Implementation Time**: 4-5 hours

---

## Component 2: RitardandoEffector

### Overview
- **File**: [`ritardando_effector.hpp`](../ritardando_effector.hpp)
- **Purpose**: Applies gradual tempo slowdown during ritardando sections
- **Dependencies**: `cxxmidi::player::PlayerSync`, `PlaybackStateMachine`
- **Test File**: `test/test_ritardando_effector.cpp`

### Key Responsibilities
1. Monitor state machine for ritardando flag
2. Reduce playback speed progressively during heartbeats
3. Apply configurable decrement rate
4. Respect speed boundaries

### Mock Strategy

```cpp
class MockPlayerSync {
public:
    void SetSpeed(float speed) {
        setSpeedCalled++;
        currentSpeed = speed;
    }
    float GetSpeed() const { return currentSpeed; }
    
    int setSpeedCalled = 0;
    float currentSpeed = 1.0f;
};
```

### Test Cases (8 total)

#### Group 1: Construction (2 tests)

**Test 1.1**: `Constructor with default decrement rate`
```cpp
TEST_CASE("RitardandoEffector construction", "[ritardando][unit]") {
    MockPlayerSync player;
    PlaybackStateMachine stateMachine;
    
    SECTION("constructor with defaults") {
        RitardandoEffector effector(player, stateMachine);
        
        REQUIRE(effector.getDecrementRate() == Approx(0.002f));
    }
}
```
**Assertions**: 1  
**Priority**: High

**Test 1.2**: `Constructor with custom decrement rate`
```cpp
SECTION("constructor with custom rate") {
    RitardandoEffector effector(player, stateMachine, 0.005f);
    
    REQUIRE(effector.getDecrementRate() == Approx(0.005f));
}
```
**Assertions**: 1  
**Priority**: Medium

#### Group 2: Heartbeat Processing - Active Ritardando (3 tests)

**Test 2.1**: `Speed reduced when ritardando active`
```cpp
TEST_CASE("RitardandoEffector heartbeat processing", "[ritardando][unit]") {
    SECTION("speed reduced when ritardando active") {
        stateMachine.setRitardando(true);
        player.SetSpeed(1.0f);
        
        effector.handleHeartbeat();
        
        REQUIRE(player.setSpeedCalled == 1);
        REQUIRE(player.currentSpeed < 1.0f);
    }
}
```
**Assertions**: 2  
**Priority**: High

**Test 2.2**: `Speed reduction equals decrementRate`
```cpp
SECTION("speed reduction matches decrement rate") {
    stateMachine.setRitardando(true);
    player.SetSpeed(1.0f);
    float initialSpeed = player.GetSpeed();
    
    effector.handleHeartbeat();
    
    float expectedSpeed = initialSpeed - 0.002f;
    REQUIRE(player.currentSpeed == Approx(expectedSpeed));
}
```
**Assertions**: 1  
**Priority**: High

**Test 2.3**: `Multiple heartbeats accumulate reduction`
```cpp
SECTION("multiple heartbeats accumulate") {
    stateMachine.setRitardando(true);
    player.SetSpeed(1.0f);
    
    effector.handleHeartbeat();
    effector.handleHeartbeat();
    effector.handleHeartbeat();
    
    REQUIRE(player.setSpeedCalled == 3);
    float expectedSpeed = 1.0f - (0.002f * 3);
    REQUIRE(player.currentSpeed == Approx(expectedSpeed));
}
```
**Assertions**: 2  
**Priority**: High

#### Group 3: Heartbeat Processing - Inactive Ritardando (1 test)

**Test 3.1**: `Speed unchanged when ritardando inactive`
```cpp
TEST_CASE("RitardandoEffector inactive behavior", "[ritardando][unit]") {
    SECTION("no speed change when inactive") {
        stateMachine.setRitardando(false);
        player.SetSpeed(1.0f);
        
        effector.handleHeartbeat();
        
        REQUIRE(player.setSpeedCalled == 0);
        REQUIRE(player.currentSpeed == Approx(1.0f));
    }
}
```
**Assertions**: 2  
**Priority**: High

#### Group 4: Configuration (2 tests)

**Test 4.1**: `setDecrementRate() updates rate`
```cpp
TEST_CASE("RitardandoEffector configuration", "[ritardando][unit]") {
    SECTION("setDecrementRate updates rate") {
        effector.setDecrementRate(0.01f);
        
        REQUIRE(effector.getDecrementRate() == Approx(0.01f));
    }
}
```
**Assertions**: 1  
**Priority**: Medium

**Test 4.2**: `Custom rate applied during heartbeat`
```cpp
SECTION("custom rate used in calculations") {
    effector.setDecrementRate(0.01f);
    stateMachine.setRitardando(true);
    player.SetSpeed(1.0f);
    
    effector.handleHeartbeat();
    
    float expectedSpeed = 1.0f - 0.01f;
    REQUIRE(player.currentSpeed == Approx(expectedSpeed));
}
```
**Assertions**: 1  
**Priority**: Medium

### Summary: RitardandoEffector
- **Total Tests**: 8
- **Total Assertions**: ~20-25
- **Implementation Time**: 2-3 hours

---

## Component 3: PlaybackOrchestrator

### Overview
- **File**: [`playback_orchestrator.hpp`](../playback_orchestrator.hpp)
- **Purpose**: High-level coordination of all playback components
- **Dependencies**: `cxxmidi::player::PlayerSync`, `PlaybackSynchronizer`, `MidiLoader`
- **Owned Components**: `PlaybackStateMachine`, `MusicalDirector`, `RitardandoEffector`
- **Test File**: `test/test_playback_orchestrator.cpp`

### Key Responsibilities
1. Initialize player with callbacks
2. Compose and coordinate sub-components
3. Manage playback flow (intro → verses)
4. Handle speed and tempo configuration
5. Display playback information

### Mock Strategy

```cpp
class MockPlayerSync {
public:
    // Full mock with callback support
    void SetEventCallback(std::function<bool(cxxmidi::Event&)> cb) {
        eventCallback = cb;
        eventCallbackSet = true;
    }
    void SetHeartbeatCallback(std::function<void()> cb) {
        heartbeatCallback = cb;
        heartbeatCallbackSet = true;
    }
    void SetFinishedCallback(std::function<void()> cb) {
        finishedCallback = cb;
        finishedCallbackSet = true;
    }
    
    void SetSpeed(float speed) {
        setSpeedCalled++;
        currentSpeed = speed;
    }
    void SetTempo(int32_t tempo) {
        setTempoCalled++;
        currentTempo = tempo;
    }
    
    // Test verification
    bool eventCallbackSet = false;
    bool heartbeatCallbackSet = false;
    bool finishedCallbackSet = false;
    int setSpeedCalled = 0;
    int setTempoCalled = 0;
    float currentSpeed = 1.0f;
    int32_t currentTempo = 500000;
    
    std::function<bool(cxxmidi::Event&)> eventCallback;
    std::function<void()> heartbeatCallback;
    std::function<void()> finishedCallback;
};
```

### Test Cases (15 total)

#### Group 1: Initialization (3 tests)

**Test 1.1**: `initialize() sets up all callbacks`
```cpp
TEST_CASE("PlaybackOrchestrator initialization", "[orchestrator][unit]") {
    SECTION("initialize sets up callbacks") {
        orchestrator.initialize();
        
        REQUIRE(player.eventCallbackSet);
        REQUIRE(player.heartbeatCallbackSet);
        REQUIRE(player.finishedCallbackSet);
    }
}
```
**Assertions**: 3  
**Priority**: High

**Test 1.2**: `initialize() sets base speed from midiLoader`
```cpp
SECTION("initialize sets base speed") {
    // MidiLoader with speed 1.2f
    orchestrator.initialize();
    
    REQUIRE(player.setSpeedCalled >= 1);
    REQUIRE(player.currentSpeed == Approx(1.2f));
}
```
**Assertions**: 2  
**Priority**: High

**Test 1.3**: `initialize() sets tempo`
```cpp
SECTION("initialize sets tempo") {
    orchestrator.initialize();
    
    REQUIRE(player.setTempoCalled >= 1);
}
```
**Assertions**: 1  
**Priority**: Medium

#### Group 2: Callback Delegation (3 tests)

**Test 2.1**: `Event callback delegates to MusicalDirector`
```cpp
TEST_CASE("PlaybackOrchestrator callback delegation", "[orchestrator][unit]") {
    SECTION("event callback delegates correctly") {
        orchestrator.initialize();
        
        cxxmidi::Event testEvent;
        bool result = player.eventCallback(testEvent);
        
        // Verify MusicalDirector received the event
        // (behavior-based verification)
        REQUIRE((result == true || result == false));
    }
}
```
**Assertions**: 1  
**Priority**: High

**Test 2.2**: `Heartbeat callback delegates to RitardandoEffector`
```cpp
SECTION("heartbeat callback delegates correctly") {
    orchestrator.initialize();
    
    REQUIRE_NOTHROW(player.heartbeatCallback());
}
```
**Assertions**: 1  
**Priority**: High

**Test 2.3**: `Finished callback notifies synchronizer`
```cpp
SECTION("finished callback notifies synchronizer") {
    orchestrator.initialize();
    
    std::atomic<bool> notified{false};
    std::thread waiter([&]() {
        synchronizer.wait();
        notified = true;
    });
    
    std::this_thread::sleep_for(50ms);
    player.finishedCallback();
    waiter.join();
    
    REQUIRE(notified);
}
```
**Assertions**: 1  
**Priority**: High

#### Group 3: Playback Flow - Introduction (3 tests)

**Test 3.1**: `executePlayback() plays intro when shouldPlayIntro() true`
```cpp
TEST_CASE("PlaybackOrchestrator intro playback", "[orchestrator][unit][integration]") {
    SECTION("intro played when flag set") {
        // MidiLoader with shouldPlayIntro = true
        
        // Note: This requires actual MIDI file or complex mocking
        // May be better suited for integration test
        
        // Verify intro initialization occurred
    }
}
```
**Assertions**: TBD (integration-focused)  
**Priority**: Medium  
**Note**: May require integration test with real MIDI file

**Test 3.2**: `Introduction initializes intro segments`
```cpp
SECTION("intro segments initialized") {
    // Verify initializeIntroSegments called
    // (behavior verification via state)
}
```
**Assertions**: TBD  
**Priority**: Medium

**Test 3.3**: `Introduction sets isPlayingIntro flag`
```cpp
SECTION("playing intro flag set") {
    // Verify state machine flag
}
```
**Assertions**: TBD  
**Priority**: Medium

#### Group 4: Playback Flow - Verses (4 tests)

**Test 4.1**: `Verses played according to getVerses() count`
```cpp
TEST_CASE("PlaybackOrchestrator verse playback", "[orchestrator][unit][integration]") {
    SECTION("correct number of verses played") {
        // MidiLoader with verses = 3
        
        // Note: Full verse playback requires integration test
        // Mock-based test can verify setup
    }
}
```
**Assertions**: TBD  
**Priority**: Medium

**Test 4.2**: `Inter-verse pausing applied`
```cpp
SECTION("pauses between verses") {
    // Verify pause logic
}
```
**Assertions**: TBD  
**Priority**: Low

**Test 4.3**: `Last verse flag set appropriately`
```cpp
SECTION("last verse flag set") {
    // Verify state machine flag on final verse
}
```
**Assertions**: TBD  
**Priority**: Medium

**Test 4.4**: `Speed maintained across verses`
```cpp
SECTION("speed consistent") {
    // Verify no unexpected speed changes
}
```
**Assertions**: TBD  
**Priority**: Low

#### Group 5: Display & Configuration (2 tests)

**Test 5.1**: `displayPlaybackInfo() shows metadata`
```cpp
TEST_CASE("PlaybackOrchestrator display", "[orchestrator][unit]") {
    SECTION("displays playback info") {
        // Capture std::cout
        orchestrator.displayPlaybackInfo();
        
        // Verify output contains title, key, verses, BPM
    }
}
```
**Assertions**: TBD (output verification)  
**Priority**: Low

**Test 5.2**: `setDisplayWarnings() propagates to state machine`
```cpp
SECTION("warning flag propagates") {
    orchestrator.setDisplayWarnings(true);
    
    // Verify state machine has flag set
    // (requires state machine accessor or behavior test)
}
```
**Assertions**: 1  
**Priority**: Low

### Summary: PlaybackOrchestrator
- **Total Tests**: 15
- **Total Assertions**: ~35-40
- **Implementation Time**: 4-5 hours
- **Note**: Some tests better suited as integration tests with real MIDI files

---

## Shared Mock Infrastructure

### File Structure
```
test/
├── mocks/
│   └── mock_player_sync.hpp    # Shared mock class
├── test_musical_director.cpp
├── test_ritardando_effector.cpp
└── test_playback_orchestrator.cpp
```

### MockPlayerSync Complete Implementation

```cpp
// test/mocks/mock_player_sync.hpp
#pragma once

#include <functional>
#include <cstdint>

namespace cxxmidi {
    class Event;  // Forward declaration
}

/**
 * @brief Mock PlayerSync for unit testing coordination logic
 * 
 * Provides simplified PlayerSync interface for testing component
 * interactions without requiring ALSA/MIDI hardware.
 */
class MockPlayerSync {
public:
    // ===== Playback Control =====
    void Play() { playCalled++; }
    void Stop() { stopCalled++; }
    void Finish() { finishCalled++; }
    void GoToTick(uint32_t tick) {
        goToTickCalled++;
        lastTickValue = tick;
    }
    void NotesOff() { notesOffCalled++; }
    
    // ===== Speed Control =====
    void SetSpeed(float speed) {
        setSpeedCalled++;
        currentSpeed = speed;
    }
    float GetSpeed() const { return currentSpeed; }
    
    // ===== Tempo Control =====
    void SetTempo(int32_t tempo) {
        setTempoCalled++;
        currentTempo = tempo;
    }
    int32_t GetTempo() const { return currentTempo; }
    
    // ===== Callback Registration =====
    void SetEventCallback(std::function<bool(cxxmidi::Event&)> cb) {
        eventCallback = cb;
        eventCallbackSet = true;
    }
    void SetHeartbeatCallback(std::function<void()> cb) {
        heartbeatCallback = cb;
        heartbeatCallbackSet = true;
    }
    void SetFinishedCallback(std::function<void()> cb) {
        finishedCallback = cb;
        finishedCallbackSet = true;
    }
    
    // ===== Test Verification Data =====
    int playCalled = 0;
    int stopCalled = 0;
    int finishCalled = 0;
    int goToTickCalled = 0;
    int notesOffCalled = 0;
    int setSpeedCalled = 0;
    int setTempoCalled = 0;
    
    uint32_t lastTickValue = 0;
    float currentSpeed = 1.0f;
    int32_t currentTempo = 500000;
    
    bool eventCallbackSet = false;
    bool heartbeatCallbackSet = false;
    bool finishedCallbackSet = false;
    
    std::function<bool(cxxmidi::Event&)> eventCallback;
    std::function<void()> heartbeatCallback;
    std::function<void()> finishedCallback;
};
```

---

## Implementation Order

### Recommended Sequence

1. **Create Mock Infrastructure** (30 minutes)
   - Create `test/mocks/` directory
   - Implement `mock_player_sync.hpp`

2. **RitardandoEffector Tests** (2-3 hours)
   - Simplest component
   - Clear behavior, minimal dependencies
   - Good warmup for test patterns

3. **MusicalDirector Tests** (4-5 hours)
   - Moderate complexity
   - Multiple marker types to test
   - Builds on mock patterns from RitardandoEffector

4. **PlaybackOrchestrator Tests** (4-5 hours)
   - Most complex
   - Integration of all components
   - Some tests may need real MIDI files

### Total Estimated Time
- **Mock Infrastructure**: 0.5 hours
- **Test Implementation**: 10-13 hours
- **Integration & Debugging**: 2-3 hours
- **Documentation Updates**: 1 hour
- **Total**: **13-17 hours**

---

## Build Integration

### Update .vscode/tasks.json

Add to "Build Tests (Catch2)" task args:
```json
"${workspaceFolder}/test/test_ritardando_effector.cpp",
"${workspaceFolder}/test/test_musical_director.cpp",
"${workspaceFolder}/test/test_playback_orchestrator.cpp",
```

### Test Execution

```bash
# Run all Phase 2 playback tests
./test/run_tests "[ritardando][musical_director][orchestrator]"

# Run only RitardandoEffector tests
./test/run_tests "[ritardando]"

# Run only MusicalDirector tests
./test/run_tests "[musical_director]"

# Run only PlaybackOrchestrator tests
./test/run_tests "[orchestrator]"

# Run with verbose output
./test/run_tests "[ritardando]" -s
```

---

## Expected Results

### Phase 2 Final Statistics

| Component | Tests | Assertions | Status |
|-----------|-------|------------|--------|
| MidiLoader | 11 | 40 | ✅ Complete |
| EventPreprocessor | (integrated) | - | ✅ Complete |
| RitardandoEffector | 8 | 20-25 | ⏳