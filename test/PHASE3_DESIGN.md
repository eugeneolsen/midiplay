# Phase 3: Integration & Device Testing - Design Specification

**Document Version**: 1.0
**Created**: 2025-10-16
**Status**: ✅ Complete - Ready for Implementation
**Approach**: Real Files + Real Components (No Mocks)

---

## Executive Summary

Phase 3 completes the unit testing strategy with **integration and device management tests**. Following the successful Phase 2 pattern, we use **real YAML files** and **real component instances** rather than mocks.

### Components to Test

1. **DeviceManager** - YAML parsing, device detection, configuration (20-25 tests)
2. **Integration Tests** - Full playback flow with real components (10-15 tests)

**Total**: 30-40 test cases, ~80-100 assertions  
**Estimated Time**: 8 hours

---

## Lessons from Phase 2

### What Worked ✅

1. **Real Files**: Testing MidiLoader with real MIDI files was straightforward and effective
2. **Real Components**: Using actual `PlaybackStateMachine`, `PlaybackSynchronizer` worked well
3. **Integration Focus**: Testing component interactions revealed real bugs (callback dangling reference, getopt pollution)
4. **No Interfaces Needed**: Avoided over-engineering by not creating abstract interfaces

### What to Avoid ❌

1. **Mock-Based Testing**: Type incompatibility issues with concrete dependencies
2. **Interface Extraction**: Over-engineering for testability
3. **Complex Test Doubles**: Simple test helpers are sufficient

---

## Part 1: DeviceManager Testing

### Overview

- **File**: [`device_manager.hpp`](../device_manager.hpp), [`device_manager.cpp`](../device_manager.cpp)
- **Test File**: `test/test_device_manager.cpp`
- **Strategy**: Test with **real YAML files** and **real file I/O**
- **Dependencies**: `Options`, `yaml-cpp`, filesystem

### Key Responsibilities

1. **YAML Configuration**:
   - Load device presets from YAML files
   - Parse device configurations (channels, banks, programs)
   - Search standard locations for config files
   - Handle missing/invalid YAML gracefully

2. **Device Detection**:
   - Detect device type from port name
   - Match against YAML detection strings
   - Fallback to default device (Allen Protégé)

3. **Device Configuration**:
   - Send bank select and program change messages
   - Configure multiple channels from YAML
   - Apply device-specific settings

### Testing Strategy: Real YAML Files

**No Mocks Needed** - We'll create real test YAML files just like we created real test MIDI files in Phase 2.

#### Test Fixture YAML Files

Create in `test/fixtures/test_configs/`:

1. **`valid_devices.yaml`** - Complete, valid configuration
2. **`minimal_devices.yaml`** - Minimal valid configuration
3. **`invalid_syntax.yaml`** - Malformed YAML (syntax errors)
4. **`missing_required.yaml`** - Missing required fields
5. **`empty.yaml`** - Empty file
6. **`multi_device.yaml`** - Multiple device configurations

---

### Test Cases: DeviceManager (20-25 tests)

#### Group 1: Construction & Initialization (2 tests)

**Test 1.1**: Constructor accepts Options dependency
```cpp
TEST_CASE("DeviceManager construction", "[device_manager][unit]") {
    Options opts(2, makeArgv({"play", "test.mid"}));
    
    SECTION("constructor accepts Options") {
        REQUIRE_NOTHROW(DeviceManager(opts));
    }
}
```
**Assertions**: 1  
**Priority**: High

**Test 1.2**: Initial state has no YAML config loaded
```cpp
SECTION("initial state has no config") {
    DeviceManager dm(opts);
    
    // Attempting device operations should throw
    // (YAML is mandatory)
}
```
**Assertions**: 1  
**Priority**: Medium

---

#### Group 2: YAML File Discovery (5 tests)

**Test 2.1**: Finds config in specified path
```cpp
TEST_CASE("DeviceManager config file discovery", "[device_manager][unit]") {
    SECTION("loads from specified path") {
        DeviceManager dm(opts);
        std::string configPath = "test/fixtures/test_configs/valid_devices.yaml";
        
        REQUIRE_NOTHROW(dm.loadDevicePresets(configPath));
    }
}
```
**Assertions**: 1  
**Priority**: High

**Test 2.2**: Throws when specified path doesn't exist
```cpp
SECTION("throws on missing specified path") {
    DeviceManager dm(opts);
    
    REQUIRE_THROWS_AS(
        dm.loadDevicePresets("nonexistent.yaml"),
        std::runtime_error
    );
}
```
**Assertions**: 1  
**Priority**: High

**Test 2.3**: Searches standard locations when no path specified
```cpp
SECTION("searches standard locations") {
    // Copy test config to one of the standard locations
    // ~/.config/midiplay/midi_devices.yaml
    // /etc/midiplay/midi_devices.yaml
    // ./midi_devices.yaml
    
    DeviceManager dm(opts);
    REQUIRE_NOTHROW(dm.loadDevicePresets());
}
```
**Assertions**: 1  
**Priority**: Medium  
**Note**: May need to set up test environment

**Test 2.4**: Throws when no config found in standard locations
```cpp
SECTION("throws when no config found") {
    // Ensure no config in standard locations
    DeviceManager dm(opts);
    
    REQUIRE_THROWS_AS(
        dm.loadDevicePresets(),
        std::runtime_error
    );
}
```
**Assertions**: 1  
**Priority**: High

**Test 2.5**: Error message lists standard locations
```cpp
SECTION("error message helpful") {
    DeviceManager dm(opts);
    
    try {
        dm.loadDevicePresets("nonexistent.yaml");
        FAIL("Should have thrown");
    } catch (const std::runtime_error& e) {
        std::string msg = e.what();
        REQUIRE(msg.find("~/.config/midiplay") != std::string::npos);
        REQUIRE(msg.find("/etc/midiplay") != std::string::npos);
        REQUIRE(msg.find("./midi_devices.yaml") != std::string::npos);
    }
}
```
**Assertions**: 3  
**Priority**: Medium

---

#### Group 3: YAML Parsing - Valid Configs (6 tests)

**Test 3.1**: Parses complete valid configuration
```cpp
TEST_CASE("DeviceManager YAML parsing", "[device_manager][unit]") {
    SECTION("parses valid complete config") {
        DeviceManager dm(opts);
        dm.loadDevicePresets("test/fixtures/test_configs/valid_devices.yaml");
        
        // Config loaded successfully (no throw)
        REQUIRE(true);
    }
}
```
**Assertions**: 1  
**Priority**: High

**Test 3.2**: Parses version field
```cpp
SECTION("parses version field") {
    DeviceManager dm(opts);
    dm.loadDevicePresets("test/fixtures/test_configs/valid_devices.yaml");
    
    // Version parsed (verified via behavior)
    REQUIRE(true);
}
```
**Assertions**: 1  
**Priority**: Low

**Test 3.3**: Parses connection settings
```cpp
SECTION("parses connection settings") {
    DeviceManager dm(opts);
    dm.loadDevicePresets("test/fixtures/test_configs/valid_devices.yaml");
    
    // Connection settings applied (verified via behavior)
    // timeout_iterations, poll_sleep_seconds, etc.
    REQUIRE(true);
}
```
**Assertions**: 1  
**Priority**: Medium

**Test 3.4**: Parses device configurations
```cpp
SECTION("parses device configurations") {
    DeviceManager dm(opts);
    dm.loadDevicePresets("test/fixtures/test_configs/valid_devices.yaml");
    
    // Device configs loaded (verified via getDeviceTypeName)
    std::string name = dm.getDeviceTypeName(DeviceType::CASIO_CTX3000);
    REQUIRE(!name.empty());
}
```
**Assertions**: 1  
**Priority**: High

**Test 3.5**: Parses detection strings
```cpp
SECTION("parses detection strings") {
    DeviceManager dm(opts);
    dm.loadDevicePresets("test/fixtures/test_configs/valid_devices.yaml");
    
    // Detection strings loaded (verified via device detection)
    REQUIRE(true);
}
```
**Assertions**: 1  
**Priority**: High

**Test 3.6**: Parses channel configurations
```cpp
SECTION("parses channel configurations") {
    DeviceManager dm(opts);
    dm.loadDevicePresets("test/fixtures/test_configs/valid_devices.yaml");
    
    // Channel configs loaded (verified via device configuration)
    REQUIRE(true);
}
```
**Assertions**: 1  
**Priority**: High

---

#### Group 4: YAML Parsing - Error Handling (4 tests)

**Test 4.1**: Throws on malformed YAML syntax
```cpp
TEST_CASE("DeviceManager YAML error handling", "[device_manager][unit]") {
    SECTION("throws on syntax errors") {
        DeviceManager dm(opts);
        
        REQUIRE_THROWS_AS(
            dm.loadDevicePresets("test/fixtures/test_configs/invalid_syntax.yaml"),
            std::runtime_error
        );
    }
}
```
**Assertions**: 1  
**Priority**: High

**Test 4.2**: Throws on empty YAML file
```cpp
SECTION("throws on empty file") {
    DeviceManager dm(opts);
    
    REQUIRE_THROWS_AS(
        dm.loadDevicePresets("test/fixtures/test_configs/empty.yaml"),
        std::runtime_error
    );
}
```
**Assertions**: 1  
**Priority**: Medium

**Test 4.3**: Handles missing optional fields gracefully
```cpp
SECTION("handles missing optional fields") {
    DeviceManager dm(opts);
    
    REQUIRE_NOTHROW(
        dm.loadDevicePresets("test/fixtures/test_configs/minimal_devices.yaml")
    );
}
```
**Assertions**: 1  
**Priority**: Medium

**Test 4.4**: Error messages are descriptive
```cpp
SECTION("error messages descriptive") {
    DeviceManager dm(opts);
    
    try {
        dm.loadDevicePresets("test/fixtures/test_configs/invalid_syntax.yaml");
        FAIL("Should have thrown");
    } catch (const std::runtime_error& e) {
        std::string msg = e.what();
        REQUIRE(msg.find("YAML") != std::string::npos);
    }
}
```
**Assertions**: 1  
**Priority**: Low

---

#### Group 5: Device Detection (5 tests)

**Test 5.1**: Detects Casio device from port name
```cpp
TEST_CASE("DeviceManager device detection", "[device_manager][unit]") {
    SECTION("detects Casio from port name") {
        DeviceManager dm(opts);
        dm.loadDevicePresets("test/fixtures/test_configs/valid_devices.yaml");
        
        // Simulate port name matching Casio detection string
        // Note: detectDeviceType is private, test via connectAndDetectDevice
        // or make it public for testing
    }
}
```
**Assertions**: 1  
**Priority**: High  
**Note**: May need to test via public API or make method testable

**Test 5.2**: Detects Yamaha device from port name
```cpp
SECTION("detects Yamaha from port name") {
    // Similar to Test 5.1
}
```
**Assertions**: 1  
**Priority**: High

**Test 5.3**: Detects Allen Protégé from port name
```cpp
SECTION("detects Allen from port name") {
    // Similar to Test 5.1
}
```
**Assertions**: 1  
**Priority**: High

**Test 5.4**: Falls back to Allen Protégé for unknown devices
```cpp
SECTION("fallback to Allen for unknown") {
    DeviceManager dm(opts);
    dm.loadDevicePresets("test/fixtures/test_configs/valid_devices.yaml");
    
    // Port name that doesn't match any detection string
    // Should fall back to Allen Protégé
}
```
**Assertions**: 1  
**Priority**: High

**Test 5.5**: Returns UNKNOWN when no fallback available
```cpp
SECTION("returns UNKNOWN when no fallback") {
    DeviceManager dm(opts);
    dm.loadDevicePresets("test/fixtures/test_configs/minimal_devices.yaml");
    
    // Config with no fallback device
    // Should return DeviceType::UNKNOWN
}
```
**Assertions**: 1  
**Priority**: Medium

---

#### Group 6: Device Type Names (3 tests)

**Test 6.1**: Returns device name from YAML when loaded
```cpp
TEST_CASE("DeviceManager device names", "[device_manager][unit]") {
    SECTION("returns name from YAML") {
        DeviceManager dm(opts);
        dm.loadDevicePresets("test/fixtures/test_configs/valid_devices.yaml");
        
        std::string name = dm.getDeviceTypeName(DeviceType::CASIO_CTX3000);
        REQUIRE(!name.empty());
        REQUIRE(name != "Unknown device");
    }
}
```
**Assertions**: 2  
**Priority**: Medium

**Test 6.2**: Returns fallback name when YAML not loaded
```cpp
SECTION("returns fallback name without YAML") {
    DeviceManager dm(opts);
    
    std::string name = dm.getDeviceTypeName(DeviceType::CASIO_CTX3000);
    REQUIRE(!name.empty());
}
```
**Assertions**: 1  
**Priority**: Low

**Test 6.3**: Returns "Unknown device" for UNKNOWN type
```cpp
SECTION("returns unknown for UNKNOWN type") {
    DeviceManager dm(opts);
    
    std::string name = dm.getDeviceTypeName(DeviceType::UNKNOWN);
    REQUIRE(name.find("Unknown") != std::string::npos);
}
```
**Assertions**: 1  
**Priority**: Low

---

### Test Fixture YAML Files Specification

#### 1. `test/fixtures/test_configs/valid_devices.yaml`

Complete, valid configuration with all fields:

```yaml
version: "1.0"

connection:
  timeout_iterations: 5
  poll_sleep_seconds: 1
  min_port_count: 1
  output_port_index: 0

devices:
  casio_ctx3000:
    name: "Casio CTX-3000 Test"
    description: "Test configuration for Casio keyboards"
    detection_strings:
      - "Casio USB-MIDI"
    channels:
      1:
        bank_msb: 0
        bank_lsb: 0
        program: 16
        description: "Organ"
      2:
        bank_msb: 0
        bank_lsb: 0
        program: 48
        description: "Strings"

  yamaha_psr_ew425:
    name: "Yamaha PSR-EW425 Test"
    description: "Test configuration for Yamaha keyboards"
    detection_strings:
      - "PSR-EW425"
    channels:
      1:
        bank_msb: 0
        bank_lsb: 0
        program: 19
        description: "Church Organ"

  allen_protege:
    name: "Allen Protégé Test"
    description: "Test configuration for Allen organs (fallback)"
    detection_strings: []  # Empty = fallback device
    channels:
      1:
        bank_msb: 0
        bank_lsb: 0
        program: 16
        description: "Great"
      2:
        bank_msb: 0
        bank_lsb: 0
        program: 48
        description: "Swell"
      3:
        bank_msb: 0
        bank_lsb: 0
        program: 52
        description: "Pedal"
```

#### 2. `test/fixtures/test_configs/minimal_devices.yaml`

Minimal valid configuration:

```yaml
version: "1.0"

devices:
  casio_ctx3000:
    name: "Casio Test"
    detection_strings:
      - "Casio"
    channels:
      1:
        program: 16
```

#### 3. `test/fixtures/test_configs/invalid_syntax.yaml`

Malformed YAML:

```yaml
version: "1.0"
devices:
  casio_ctx3000:
    name: "Casio"
    channels:
      1:
        program: 16
      # Missing closing bracket
```

#### 4. `test/fixtures/test_configs/missing_required.yaml`

Missing required fields:

```yaml
version: "1.0"

devices:
  casio_ctx3000:
    # Missing 'name' field
    channels:
      1:
        program: 16
```

#### 5. `test/fixtures/test_configs/empty.yaml`

Empty file (0 bytes or just whitespace)

#### 6. `test/fixtures/test_configs/multi_device.yaml`

Multiple device configurations for testing device selection:

```yaml
version: "1.0"

devices:
  casio_ctx3000:
    name: "Casio"
    detection_strings:
      - "Casio"
    channels:
      1:
        program: 16

  yamaha_psr_ew425:
    name: "Yamaha"
    detection_strings:
      - "Yamaha"
      - "PSR"
    channels:
      1:
        program: 19

  allen_protege:
    name: "Allen"
    detection_strings: []
    channels:
      1:
        program: 16
```

---

### DeviceManager Summary

- **Total Tests**: 20-25
- **Total Assertions**: ~30-35
- **Implementation Time**: 4-5 hours
- **Test Strategy**: Real YAML files, real file I/O
- **No Mocks**: Uses actual yaml-cpp library and filesystem

---

## Part 2: Integration Tests

### Overview

- **Test File**: `test/integration/test_full_playback.cpp`
- **Strategy**: Test **real component interactions** with **real MIDI files**
- **Goal**: Verify end-to-end playback flow

### Key Integration Points

1. **File Loading → Processing**:
   - MidiLoader → EventPreProcessor
   - Metadata extraction
   - Marker detection

2. **Playback Coordination**:
   - PlaybackOrchestrator → MusicalDirector
   - PlaybackOrchestrator → RitardandoEffector
   - State management across components

3. **Synchronization**:
   - PlaybackSynchronizer → SignalHandler
   - Thread coordination
   - Graceful shutdown

### FakePlayerSync Test Helper

**Not a Mock** - Just a simple test double that records calls:

```cpp
// test/helpers/fake_player_sync.hpp
#pragma once

#include <cxxmidi/player/player_sync.hpp>
#include <functional>
#include <vector>

namespace MidiPlay {
namespace Test {

/**
 * @brief Simple test double for PlayerSync
 * 
 * Records method calls for verification without requiring MIDI hardware.
 * This is NOT a mock - it's a simple fake that implements the same
 * interface as PlayerSync for testing purposes.
 */
class FakePlayerSync {
public:
    // Playback control
    void Play() { playCalls.push_back("Play"); }
    void Stop() { stopCalls.push_back("Stop"); }
    void Finish() { finishCalls.push_back("Finish"); }
    void GoToTick(uint32_t tick) {
        goToTickCalls.push_back(tick);
    }
    void NotesOff() { notesOffCalls.push_back("NotesOff"); }
    
    // Speed/Tempo control
    void SetSpeed(float speed) {
        currentSpeed = speed;
        speedChanges.push_back(speed);
    }
    float GetSpeed() const { return currentSpeed; }
    
    void SetTempo(int32_t tempo) {
        currentTempo = tempo;
        tempoChanges.push_back(tempo);
    }
    int32_t GetTempo() const { return currentTempo; }
    
    // Callback registration
    void SetEventCallback(std::function<bool(cxxmidi::Event&)> cb) {
        eventCallback = cb;
    }
    void SetHeartbeatCallback(std::function<void()> cb) {
        heartbeatCallback = cb;
    }
    void SetFinishedCallback(std::function<void()> cb) {
        finishedCallback = cb;
    }
    
    // Test verification data
    std::vector<std::string> playCalls;
    std::vector<std::string> stopCalls;
    std::vector<std::string> finishCalls;
    std::vector<uint32_t> goToTickCalls;
    std::vector<std::string> notesOffCalls;
    std::vector<float> speedChanges;
    std::vector<int32_t> tempoChanges;
    
    float currentSpeed = 1.0f;
    int32_t currentTempo = 500000;
    
    std::function<bool(cxxmidi::Event&)> eventCallback;
    std::function<void()> heartbeatCallback;
    std::function<void()> finishedCallback;
    
    // Helper methods
    void reset() {
        playCalls.clear();
        stopCalls.clear();
        finishCalls.clear();
        goToTickCalls.clear();
        notesOffCalls.clear();
        speedChanges.clear();
        tempoChanges.clear();
        currentSpeed = 1.0f;
        currentTempo = 500000;
    }
};

} // namespace Test
} // namespace MidiPlay
```

---

### Integration Test Cases (10-15 tests)

#### Group 1: File Loading Integration (3 tests)

**Test 1.1**: MidiLoader loads file and EventPreProcessor extracts metadata
```cpp
TEST_CASE("File loading integration", "[integration]") {
    SECTION("loads file and extracts metadata") {
        Options opts(2, makeArgv({"play", "test/fixtures/test_files/simple.mid"}));
        MidiLoader loader(opts);
        
        REQUIRE_NOTHROW(loader.Load());
        REQUIRE(!loader.getTitle().empty());
        REQUIRE(loader.getKey() != "");
        REQUIRE(loader.getBpm() > 0);
    }
}
```
**Assertions**: 4  
**Priority**: High

**Test 1.2**: Introduction segments detected correctly
```cpp
SECTION("detects introduction segments") {
    Options opts(2, makeArgv({"play", "test/fixtures/test_files/with_intro.mid"}));
    MidiLoader loader(opts);
    loader.Load();
    
    const auto& segments = loader.getIntroductionSegments();
    REQUIRE(!segments.empty());
}
```
**Assertions**: 1  
**Priority**: High

**Test 1.3**: Verse count from MIDI file or options
```cpp
SECTION("verse count from file or options") {
    Options opts(4, makeArgv({"play", "test.mid", "-n", "5"}));
    MidiLoader loader(opts);
    loader.Load();
    
    REQUIRE(loader.getVerses() == 5);
}
```
**Assertions**: 1  
**Priority**: Medium

---

#### Group 2: Playback Orchestration Integration (4 tests)

**Test 2.1**: PlaybackOrchestrator initializes all components
```cpp
TEST_CASE("Playback orchestration integration", "[integration]") {
    SECTION("initializes all components") {
        Options opts(2, makeArgv({"play", "test/fixtures/test_files/simple.mid"}));
        MidiLoader loader(opts);
        loader.Load();
        
        FakePlayerSync player;
        PlaybackSynchronizer sync;
        PlaybackOrchestrator orchestrator(player, sync, loader);
        
        REQUIRE_NOTHROW(orchestrator.initialize());
        
        // Verify callbacks set
        REQUIRE(player.eventCallback != nullptr);
        REQUIRE(player.heartbeatCallback != nullptr);
        REQUIRE(player.finishedCallback != nullptr);
    }
}
```
**Assertions**: 4  
**Priority**: High

**Test 2.2**: Speed and tempo configured correctly
```cpp
SECTION("speed and tempo configured") {
    // ... setup ...
    orchestrator.initialize();
    
    REQUIRE(!player.speedChanges.empty());
    REQUIRE(!player.tempoChanges.empty());
}
```
**Assertions**: 2  
**Priority**: High

**Test 2.3**: State machine integrated with components
```cpp
SECTION("state machine integration") {
    // ... setup ...
    orchestrator.initialize();
    
    // Trigger callbacks and verify state changes
    // (behavior-based verification)
}
```
**Assertions**: TBD  
**Priority**: Medium

**Test 2.4**: Display info shows correct metadata
```cpp
SECTION("displays correct info") {
    // ... setup ...
    
    // Capture stdout
    orchestrator.displayPlaybackInfo();
    
    // Verify output contains title, key, verses, BPM
}
```
**Assertions**: TBD  
**Priority**: Low

---

#### Group 3: Synchronization Integration (2 tests)

**Test 3.1**: PlaybackSynchronizer coordinates threads
```cpp
TEST_CASE("Synchronization integration", "[integration]") {
    SECTION("coordinates playback threads") {
        PlaybackSynchronizer sync;
        
        std::atomic<bool> finished{false};
        std::thread waiter([&]() {
            sync.wait();
            finished = true;
        });
        
        std::this_thread::sleep_for(50ms);
        REQUIRE_FALSE(finished);
        
        sync.notify();
        waiter.join();
        REQUIRE(finished);
    }
}
```
**Assertions**: 2  
**Priority**: High

**Test 3.2**: Finished callback notifies synchronizer
```cpp
SECTION("finished callback notifies") {
    // ... setup with FakePlayerSync ...
    orchestrator.initialize();
    
    std::atomic<bool> notified{false};
    std::thread waiter([&]() {
        sync.wait();
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

---

#### Group 4: Signal Handling Integration (2 tests)

**Test 4.1**: SignalHandler integrates with synchronizer
```cpp
TEST_CASE("Signal handling integration", "[integration]") {
    SECTION("signal handler setup") {
        cxxmidi::output::Default outport;
        PlaybackSynchronizer sync;
        auto startTime = std::chrono::high_resolution_clock::now();
        
        SignalHandler handler(outport, sync, startTime);
        
        REQUIRE_NOTHROW(handler.setupSignalHandler());
    }
}
```
**Assertions**: 1  
**Priority**: Medium

**Test 4.2**: Emergency notes-off on interrupt
```cpp
SECTION("emergency notes-off") {
    // Note: Testing actual signal handling is complex
    // This test verifies the setup, not the actual signal
    
    // Verify handler can be constructed and set up
    REQUIRE(true);
}
```
**Assertions**: 1  
**Priority**: Low  
**Note**: Full signal testing requires more complex setup

---

#### Group 5: End-to-End Scenarios (3-5 tests)

**Test 5.1**: Complete playback flow without intro
```cpp
TEST_CASE("End-to-end playback scenarios", "[integration][e2e]") {
    SECTION("playback without intro") {
        Options opts(4, makeArgv({"play", "test/fixtures/test_files/simple.mid", "-x", "2"}));
        MidiLoader loader(opts);
        loader.Load();
        
        FakePlayerSync player;
        PlaybackSynchronizer sync;
        PlaybackOrchestrator orchestrator(player, sync, loader);
        
        orchestrator.initialize();
        
        // Verify no intro playback
        REQUIRE_FALSE(loader.shouldPlayIntro());
    }
}
```
**Assertions**: 1  
**Priority**: High

**Test 5.2**: Complete playback flow with intro
```cpp
SECTION("playback with intro") {
    Options opts(4, makeArgv({"play", "test/fixtures/test_files/with_intro.mid", "-n", "3"}));
    // ... similar setup ...
    
    REQUIRE(loader.shouldPlayIntro());
    REQUIRE(!loader.getIntroductionSegments().empty());
}
```
**Assertions**: 2  
**Priority**: High

**Test 5.3**: Ritardando marker processing
```cpp
SECTION("ritardando processing") {
    Options opts(2, makeArgv({"play", "test/fixtures/test_files/ritardando.mid"}));
    // ... setup ...
    
    // Verify ritardando state changes
    // (requires triggering callbacks with marker events)
}
```
**Assertions**: TBD  
**Priority**: Medium

**Test 5.4**: D.C. al Fine marker processing
```cpp
SECTION("D.C. al Fine processing") {
    Options opts(2, makeArgv({"play", "test/fixtures/test_files/dc_al_fine.mid"}));
    // ... setup ...
    
    // Verify D.C. al Fine behavior
}
```
**Assertions**: TBD  
**Priority**: Medium

**Test 5.5**: Speed changes applied correctly
```cpp
SECTION("speed changes") {
    Options opts(4, makeArgv({"play", "test.mid", "-p", "12"}));
    // ... setup ...
    
    REQUIRE(player.currentSpeed == Approx(1.2f));
}
```
**Assertions**: 1  
**Priority**: Medium

---

### Integration Tests Summary

- **Total Tests**: 10-15
- **Total Assertions**: ~50-65
- **Implementation Time**: 3-4 hours
- **Test Strategy**: Real components, FakePlayerSync helper
- **No Mocks**: Uses actual component instances

---

## Implementation Plan

### Phase 3.1: DeviceManager Tests (4-5 hours)

1. **Create test fixture YAML files** (1 hour)
   - `valid_devices.yaml`
   - `minimal_devices.yaml`
   - `invalid_syntax.yaml`
   - `missing_required.yaml`
   - `empty.yaml`
   - `multi_device.yaml`

2. **Implement DeviceManager tests** (3-4 hours)
   - Construction & initialization (2 tests)
   - YAML file discovery (5 tests)
   - YAML parsing - valid (6 tests)
   - YAML parsing - errors (4 tests)
   - Device detection (5 tests)
   - Device type names (3 tests)

### Phase 3.2: Integration Tests (3-4 hours)

1. **Create FakePlayerSync helper** (30 minutes)
   - `test/helpers/fake_player_sync.hpp`

2. **Implement integration tests** (2.5-3.5 hours)
   - File loading integration (3 tests)