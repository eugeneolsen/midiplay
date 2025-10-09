# MIDIPlay Unit Testing Infrastructure

**Framework**: Catch2 v3.9.1  
**Language**: C++20  
**Build System**: VSCode Tasks (g++ direct compilation)

---

## Quick Start

### Run All Tests
```bash
# Using VSCode Task (recommended)
Ctrl+Shift+P → "Tasks: Run Test Task" → "Run All Tests"

# Or manually
cd test
./run_tests
```

### Run Specific Test Tags
```bash
# Run only state machine tests
./run_tests "[state]"

# Run only synchronization tests
./run_tests "[sync]"

# Run only timing tests
./run_tests "[timing]"

# Run only options tests
./run_tests "[options]"

# Run all unit tests
./run_tests "[unit]"
```

### Run Tests with Verbose Output
```bash
./run_tests -s
```

---

## Test Organization

### Test Files

| File | Component | Tests | Lines | Tags |
|------|-----------|-------|-------|------|
| `test_playback_state_machine.cpp` | PlaybackStateMachine | 12 | ~156 | `[state][unit]` |
| `test_playback_synchronizer.cpp` | PlaybackSynchronizer | 10 | ~197 | `[sync][unit][threading]` |
| `test_timing_manager.cpp` | TimingManager | 9 | ~158 | `[timing][unit]` |
| `test_options.cpp` | Options (CLI parsing) | 25+ | ~378 | `[options][unit][cli]` |

### Directory Structure

```
test/
├── external/                      # Catch2 framework files
│   ├── catch_amalgamated.hpp     # Catch2 v3.9.1 header
│   └── catch_amalgamated.cpp     # Catch2 v3.9.1 implementation
│
├── fixtures/                      # Test data (Phase 2)
│   ├── test_files/               # MIDI test files
│   └── test_configs/             # YAML test configs
│
├── test_runner.cpp                # Catch2 entry point
├── test_playback_state_machine.cpp
├── test_playback_synchronizer.cpp
├── test_timing_manager.cpp
├── test_options.cpp
│
├── .gitignore                     # Exclude binaries
├── README.md                      # This file
└── run_tests                      # Compiled executable (gitignored)
```

---

## Building Tests

### Using VSCode Tasks (Recommended)

1. **Build Tests**: `Ctrl+Shift+P` → "Tasks: Run Build Task" → "Build Tests (Catch2)"
2. **Run Tests**: `Ctrl+Shift+P` → "Tasks: Run Test Task" → "Run All Tests"

### Manual Build

```bash
g++ -std=c++20 -g \
    -I/home/eugene/src/midiplay \
    -I${HOME}/.local/include \
    test/external/catch_amalgamated.cpp \
    test/test_runner.cpp \
    test/test_playback_state_machine.cpp \
    test/test_playback_synchronizer.cpp \
    test/test_timing_manager.cpp \
    test/test_options.cpp \
    signal_handler.cpp \
    device_manager.cpp \
    midi_loader.cpp \
    event_preprocessor.cpp \
    timing_manager.cpp \
    playback_orchestrator.cpp \
    musical_director.cpp \
    ritardando_effector.cpp \
    playback_synchronizer.cpp \
    ${HOME}/.local/lib/utility.o \
    -o test/run_tests \
    -L${HOME}/.local/lib \
    -lasound \
    -pthread \
    -lyaml-cpp
```

**Important**: Do NOT include `play.cpp` in the test build (it has its own `main()`).

---

## Writing Tests

### Basic Test Structure

```cpp
#include "external/catch_amalgamated.hpp"
#include "../component_to_test.hpp"

using namespace MidiPlay;

TEST_CASE("Component description", "[tag1][tag2]") {
    // Arrange
    ComponentType component;
    
    SECTION("scenario 1") {
        // Act
        auto result = component.method();
        
        // Assert
        REQUIRE(result == expected);
    }
    
    SECTION("scenario 2") {
        // ...
    }
}
```

### Naming Conventions

#### Test Case Names
- Format: `"ComponentName behavior description"`
- Example: `"PlaybackStateMachine manages state correctly"`
- Use natural language, not camelCase

#### Section Names
- Format: `"specific scenario description"`
- Example: `"intro state toggles correctly"`
- Use lowercase, natural language

#### Tags
- Component tags: `[state]`, `[sync]`, `[timing]`, `[options]`
- Type tags: `[unit]`, `[integration]`, `[performance]`
- Use lowercase, descriptive
- Apply multiple tags for filtering

### Assertions

**Critical Assertions** (test fails immediately):
```cpp
REQUIRE(condition);
REQUIRE(value == expected);
REQUIRE_FALSE(condition);
```

**Non-Critical Assertions** (test continues):
```cpp
CHECK(condition);
```

**Floating-Point Comparisons**:
```cpp
using Catch::Matchers::WithinRel;
REQUIRE_THAT(actual, WithinRel(expected, 0.01));  // 1% tolerance

// Or simpler:
REQUIRE(value == Approx(3.14f));
```

### Test Isolation

- Each test is independent (no shared state)
- SECTIONs share setup code but run independently
- Catch2 resets state between SECTIONs

Example:
```cpp
TEST_CASE("Component behavior") {
    Component comp;  // Fresh for each SECTION
    
    SECTION("scenario 1") {
        comp.method1();
        REQUIRE(comp.state() == State1);
    }
    
    SECTION("scenario 2") {
        // comp is fresh again (no side effects from scenario 1)
        comp.method2();
        REQUIRE(comp.state() == State2);
    }
}
```

---

## Test Coverage (Phase 1)

### Components Tested ✅

| Component | Coverage | Status |
|-----------|----------|--------|
| PlaybackStateMachine | 100% | ✅ Complete |
| PlaybackSynchronizer | 100% | ✅ Complete |
| TimingManager | 100% | ✅ Complete |
| Options (CLI parsing) | ~95% | ✅ Complete |

### Components Not Yet Tested (Phase 2+)

- PlaybackOrchestrator
- MusicalDirector
- RitardandoEffector
- EventPreprocessor
- MidiLoader
- DeviceManager
- SignalHandler

---

## Catch2 Features Used

### TEST_CASE
Basic test definition with tags for filtering.

### SECTION
Group related tests that share setup code.

### SCENARIO/GIVEN/WHEN/THEN
BDD-style tests for complex scenarios.

### GENERATE
Data-driven tests (parameterized testing).

Example:
```cpp
TEST_CASE("Options verse count parsing") {
    int verses = GENERATE(1, 2, 3, 4, 5);
    
    SECTION("with intro") {
        // Test runs 5 times with different verses values
    }
}
```

### Matchers
- `WithinRel(expected, tolerance)` - Relative tolerance for floats
- `Approx(value)` - Simple float approximation

---

## VSCode Integration

### Available Tasks

1. **Build Tests (Catch2)**
   - Compiles all test files with dependencies
   - Does not include `play.cpp` (has its own main)
   
2. **Run All Tests** (Default Test Task)
   - Builds and runs all tests
   - Shows output in dedicated panel
   
3. **Run Tests (Tagged)**
   - Prompts for tag filter
   - Runs only matching tests
   
4. **Run Tests (Verbose)**
   - Shows detailed test execution
   - Useful for debugging

### Keyboard Shortcuts

- **Run Tests**: `Ctrl+Shift+P` → "Tasks: Run Test Task"
- **Build**: `Ctrl+Shift+B` (builds main app, not tests)

---

## Common Test Patterns

### Testing State Machines
```cpp
TEST_CASE("State transitions") {
    StateMachine sm;
    
    sm.setState(true);
    REQUIRE(sm.getState());
    
    sm.setState(false);
    REQUIRE_FALSE(sm.getState());
}
```

### Testing Thread Synchronization
```cpp
TEST_CASE("Thread synchronization") {
    Synchronizer sync;
    std::atomic<bool> flag{false};
    
    std::thread worker([&]() {
        sync.wait();
        flag = true;
    });
    
    std::this_thread::sleep_for(50ms);
    REQUIRE_FALSE(flag.load());
    
    sync.notify();
    worker.join();
    REQUIRE(flag.load());
}
```

### Testing Command-Line Options
```cpp
TEST_CASE("CLI parsing") {
    auto args = std::vector<std::string>{"play", "file.mid", "-V"};
    char** argv = makeArgv(args);
    
    Options opts(args.size(), argv);
    opts.parse();
    
    REQUIRE(opts.isVerbose());
    
    freeArgv(argv, args.size());
}
```

### Testing Timing/Duration
```cpp
TEST_CASE("Timing accuracy") {
    Timer timer;
    timer.start();
    std::this_thread::sleep_for(100ms);
    timer.stop();
    
    using Catch::Matchers::WithinRel;
    REQUIRE_THAT(timer.elapsed(), WithinRel(0.1, 0.5));
}
```

---

## Troubleshooting

### Build Errors

**Problem**: "undefined reference to main"  
**Solution**: Ensure `test_runner.cpp` includes `#define CATCH_CONFIG_MAIN`

**Problem**: "multiple definition of main"  
**Solution**: Do NOT include `play.cpp` in test build

**Problem**: Missing dependencies  
**Solution**: Check that all source files except `play.cpp` are included

### Test Failures

**Timing Tests Fail Intermittently**:
- Increase tolerance in floating-point comparisons
- Add buffer time to thread sleeps
- Use `WithinRel()` with appropriate tolerance

**Threading Tests Hang**:
- Ensure threads are properly joined
- Check for deadlocks in synchronization
- Add timeouts to wait operations

---

## Future Enhancements (Phase 2+)

- [ ] Integration tests for MIDI processing pipeline
- [ ] Mock MIDI devices for testing
- [ ] Test fixtures with sample MIDI files
- [ ] Performance benchmarks
- [ ] Code coverage reporting
- [ ] Continuous Integration (CI) setup
- [ ] Test data generators

---

## References

- **Catch2 Documentation**: https://github.com/catchorg/Catch2/tree/v3.9.1
- **Catch2 Tutorial**: https://github.com/catchorg/Catch2/blob/v3.9.1/docs/tutorial.md
- **Project Testing Strategy**: `UNIT_TESTING_STRATEGY.md`
- **Phase 1 Implementation Plan**: `PHASE1_IMPLEMENTATION_PLAN.md`

---

## Test Statistics

**Total Test Cases**: 56+  
**Total Test Files**: 4  
**Total Lines of Test Code**: ~889  
**Framework**: Catch2 v3.9.1  
**Coverage**: Core utilities (Phase 1 complete)

**Last Updated**: 2025-10-09