# Migration Plan: getopt() → CLI11

## Executive Summary

**Goal**: Replace `getopt()` with CLI11 for command-line parsing in [`options.hpp`](options.hpp)

**Benefits**:
- ✅ Eliminate global state issues (`optind`, `optarg`, `opterr`)
- ✅ Remove manual test cleanup requirements
- ✅ Type-safe argument parsing
- ✅ Better error messages for users
- ✅ Built-in validation
- ✅ Modern C++ idioms

**Risk**: Low - Well-encapsulated change in single file

**Effort**: 1-2 days (implementation + testing)

---

## Why CLI11?

### CLI11 Advantages
- **Header-only**: Single file, like Catch2 (easy integration)
- **Modern C++11+**: Fits project's C++20 standard
- **Zero global state**: Thread-safe, test-friendly
- **Type safety**: Compile-time checks
- **Active maintenance**: 4.2k+ GitHub stars, regular updates
- **MIT License**: Compatible with project licensing

### Current getopt() Problems

| Issue | Impact | CLI11 Solution |
|-------|--------|----------------|
| Global `optind` state | Test pollution, requires `optind=0` reset | Stateless parser object |
| `char** argv_` storage | Dangling pointer risks | Internal string storage |
| Manual type conversion | Runtime errors, verbose code | Automatic type conversion |
| Poor error messages | User confusion | Descriptive validation errors |
| GNU/POSIX differences | Platform inconsistencies | Consistent cross-platform |

---

## Migration Strategy

### Phase 1: Preparation (Day 1 Morning)

**1.1 Download CLI11**
```bash
cd /home/eugene/src/midiplay
mkdir -p external
wget https://github.com/CLIUtils/CLI11/releases/download/v2.4.1/CLI11.hpp \
     -O external/CLI11.hpp
```

**1.2 Create Branch**
```bash
git checkout -b feature/cli11-migration
```

**1.3 Backup Current Implementation**
```bash
cp options.hpp options.hpp.getopt.backup
```

### Phase 2: Implementation (Day 1 Afternoon)

**2.1 Update options.hpp Structure**

**Before (getopt):**
```cpp
class Options {
private:
    int argc_;
    char** argv_;  // ⚠️ Dangerous raw pointer
    
    int parse() {
        while ((opt = getopt_long(argc_, argv_, ...)) != -1) {
            switch (opt) { ... }
        }
        if (optind < argc_) {  // ⚠️ Global state
            filename_ = argv_[optind];
        }
    }
};
```

**After (CLI11):**
```cpp
#include "external/CLI11.hpp"

class Options {
private:
    // No argc_/argv_ storage needed!
    CLI::App app_{"Organ Pi MIDI Player", "play"};
    
    int parse() {
        setupCLI11Options();
        try {
            app_.parse(argc_, argv_);
        } catch (const CLI::ParseError &e) {
            return app_.exit(e);
        }
        return validateAndFinalize();
    }
    
    void setupCLI11Options() {
        // Define all options (see detailed code below)
    }
};
```

**2.2 Detailed CLI11 Option Definitions**

```cpp
void Options::setupCLI11Options() {
    app_.description("Organ Pi play MIDI file command, version " + getSemanticVersion());
    
    // Positional argument (required)
    app_.add_option("filename", filename_, "MIDI file to play")
        ->required()
        ->check(CLI::ExistingFile);  // ✨ Built-in validation!
    
    // Optional second positional
    app_.add_option("url_name", url_name_, "Optional URL name");
    
    // Flags (boolean options)
    app_.add_flag("-s,--staging", staging_, 
                  "Play file from staging directory");
    
    app_.add_flag("-V,--verbose", verbose_, 
                  "Verbose output");
    
    app_.add_flag("-W,--warnings", display_warnings_, 
                  "Display warnings");
    
    app_.add_flag("-v,--version", 
                  "Display version")->callback([this]() {
        displayVersion();
        throw CLI::Success();  // Exit cleanly
    });
    
    app_.add_flag("-h,--help", 
                  "Display help")->callback([this]() {
        std::cout << app_.help();
        throw CLI::Success();
    });
    
    // Options with values
    app_.add_option("-t,--tempo", bpm_, 
                    "Force tempo (beats per minute)")
        ->check(CLI::PositiveNumber)  // ✨ Validation!
        ->transform([this](std::string val) {
            usec_per_beat_ = MidiPlay::MICROSECONDS_PER_MINUTE / std::stoi(val);
            return val;
        });
    
    app_.add_option("-T,--title", title_, 
                    "Hymn title");
    
    // Verses with introduction
    app_.add_option("-n", verses_, 
                    "Play introduction + N verses")
        ->check(CLI::PositiveNumber)
        ->transform([this](std::string val) {
            play_intro_ = true;
            return val;
        });
    
    // Verses without introduction  
    app_.add_option("-x", verses_, 
                    "Play N verses without introduction")
        ->check(CLI::PositiveNumber)
        ->transform([this](std::string val) {
            play_intro_ = false;
            return val;
        });
    
    // Prelude with optional speed
    auto* prelude = app_.add_option("-p,--prelude", 
                                     "Prelude/postlude mode");
    prelude->type_name("[speed]");
    prelude->default_val("9");
    prelude->callback([this](CLI::results_t res) {
        verses_ = 2;
        play_intro_ = false;
        prepost_ = true;
        
        if (!res.empty()) {
            float speedOption = std::stof(res[0]) / PRELUDE_SPEED_DIVISOR;
            if (speedOption >= PRELUDE_MIN_SPEED && 
                speedOption <= PRELUDE_MAX_SPEED) {
                speed_ = speedOption;
            } else {
                speed_ = 1.0f;
            }
        } else {
            speed_ = DEFAULT_PRELUDE_SPEED;
        }
    });
    
    // Goto (not yet implemented)
    app_.add_option("-g,--goto", 
                    "Start at measure/marker (not yet implemented)")
        ->callback([](std::string) {
            std::cout << _("Goto option not yet implemented. "
                          "Starting at the beginning.") << std::endl;
        });
    
    // Future options (commented out for now)
    // app_.add_option("-c,--channel", "Force channel (Swell=1, Great=2)");
    // app_.add_option("-S,--stops", "YAML file with stop definitions");
}
```

**2.3 Remove getopt Dependencies**

```cpp
// REMOVE these lines:
#include <getopt.h>
static struct option long_options[] = { ... };

// REMOVE these member variables:
int argc_;
char** argv_;

// ADD:
#include "external/CLI11.hpp"
CLI::App app_;
```

**2.4 Constructor Changes**

```cpp
// Before:
Options(int argc, char** argv) {
    argc_ = argc;
    argv_ = argv;
    verses_ = 0;
}

// After:
Options(int argc, char** argv) 
    : app_("Organ Pi MIDI Player", "play") {
    verses_ = 0;
    // argc/argv passed directly to parse()
}
```

**2.5 Parse Method Simplification**

```cpp
int parse(int argc, char** argv) {  // Note: now takes args
    setupCLI11Options();
    
    try {
        app_.parse(argc, argv);
    } catch (const CLI::ParseError &e) {
        return app_.exit(e);  // Handles help/version automatically
    }
    
    // Validation logic (if filename is required, CLI11 already checked)
    return MidiPlay::OptionsParseResult::SUCCESS;
}
```

### Phase 3: Testing (Day 2 Morning)

**3.1 Update Test Files**

**test/test_options.cpp:**
```cpp
// REMOVE:
#include <getopt.h>
optind = 0;  // No longer needed! ✨

// Tests remain the same - same public API!
TEST_CASE("Options: Parse valid arguments", "[options]") {
    const char* argv[] = {"play", "test.mid", "-V"};
    Options opts(3, const_cast<char**>(argv));
    int result = opts.parse();
    
    REQUIRE(result == 0);
    REQUIRE(opts.getFileName() == "test.mid");
    REQUIRE(opts.isVerbose() == true);
}
```

**test/test_midi_loader.cpp:**
```cpp
// REMOVE all optind = 0 resets! ✨
// Tests work without modification!

SECTION("Successfully loads simple MIDI file") {
    // optind = 0;  ← DELETE THIS LINE
    const char* argv[] = {"midiplay", testFile.c_str()};
    Options options(2, const_cast<char**>(argv));
    options.parse();
    // ... rest unchanged
}
```

**3.2 Test Categories**

| Category | Test Count | Expected Result |
|----------|------------|-----------------|
| Unit tests (test_options.cpp) | ~15 | All pass |
| Integration (test_midi_loader.cpp) | 11 | All pass |
| Full suite | 43 | All pass |

**3.3 Test Execution**

```bash
cd test

# Build with CLI11
g++ -std=c++20 -I.. -I../external \
    test_runner.cpp test_options.cpp test_midi_loader.cpp \
    external/catch_amalgamated.cpp \
    ../midi_loader.cpp ../event_preprocessor.cpp \
    # ... other sources
    -o run_tests

# Run tests
./run_tests "[options]"   # Should pass without optind resets
./run_tests "[midi_loader]"  # Should pass without optind resets
./run_tests  # Full suite
```

### Phase 4: Integration (Day 2 Afternoon)

**4.1 Update Build Files**

**.vscode/tasks.json:**
```json
{
    "args": [
        "-I${workspaceFolder}",
        "-I${workspaceFolder}/external",  // ADD THIS
        "-I${userHome}/.local/include",
        // ... rest unchanged
    ]
}
```

**4.2 Update Documentation**

Update these files to mention CLI11:
- `README.md` - Add CLI11 to dependencies
- `test/PHASE2_IMPLEMENTATION_SUMMARY.md` - Remove optind workaround notes

**4.3 Production Testing**

```bash
# Build production binary
./build.sh  # or your build command

# Test all command-line scenarios
./play test.mid
./play test.mid -V
./play test.mid -t120
./play test.mid -x3
./play test.mid -p9
./play --help
./play --version
```

---

## Risk Assessment & Mitigation

| Risk | Likelihood | Impact | Mitigation |
|------|-----------|--------|------------|
| CLI11 compatibility issues | Low | Medium | Test thoroughly before merge |
| Behavior changes | Low | High | Extensive manual testing |
| Build system issues | Low | Low | Update all task configs |
| Test failures | Medium | Low | Fixed public API maintains compatibility |
| User confusion | Low | Low | Same CLI interface, better errors |

**Rollback Plan**: Keep `options.hpp.getopt.backup` until 1 week of production use

---

## Success Criteria

✅ **All tests pass** (195 assertions in 43 test cases)  
✅ **No `optind` resets needed** in any test file  
✅ **Production CLI works** identically to current  
✅ **Code is cleaner** (fewer lines, more readable)  
✅ **Better error messages** for end users

---

## Timeline

| Phase | Duration | Dependencies |
|-------|----------|--------------|
| 1. Preparation | 1 hour | None |
| 2. Implementation | 4 hours | Phase 1 |
| 3. Testing | 2 hours | Phase 2 |
| 4. Integration | 1 hour | Phase 3 |
| **Total** | **1 working day** | |

*Buffer: +0.5 days for unexpected issues*

---

## Benefits Summary

### Developer Experience
- 🎯 **Simpler tests**: No global state management
- 🎯 **Type safety**: Catch errors at compile time
- 🎯 **Less boilerplate**: CLI11 handles parsing details
- 🎯 **Better debugging**: Clear error messages

### User Experience  
- 🎯 **Better help text**: Auto-formatted, consistent
- 🎯 **Validation errors**: "tempo must be positive" vs "invalid option"
- 🎯 **Consistent behavior**: No platform quirks

### Code Quality
- 🎯 **Fewer lines**: ~100 lines removed
- 🎯 **Modern C++**: Fits project's C++20 standard
- 🎯 **Maintainable**: Industry-standard library

---

## Post-Migration Cleanup

Once stable (1 week):
1. Delete `options.hpp.getopt.backup`
2. Remove getopt documentation references
3. Update ConPort with decision
4. Consider CLI11 for other projects

---

## References

- **CLI11 Docs**: https://cliutils.github.io/CLI11/book/
- **GitHub**: https://github.com/CLIUtils/CLI11
- **Examples**: https://github.com/CLIUtils/CLI11/tree/main/examples

---

## Decision Log

This migration addresses:
- Decision D-71: getopt state pollution
- Decision D-72: Analysis of getopt replacement

**Status**: ⏳ Approved, awaiting implementation

**Owner**: TBD

**Target Date**: TBD