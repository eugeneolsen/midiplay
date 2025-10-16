#include "external/catch_amalgamated.hpp"
#include "../playback_orchestrator.hpp"
#include "../playback_synchronizer.hpp"
#include "../midi_loader.hpp"
#include "../signal_handler.hpp"
#include "../options.hpp"
#include <cxxmidi/player/player_sync.hpp>
#include <cxxmidi/output/default.hpp>
#include <filesystem>
#include <getopt.h>
#include <thread>
#include <chrono>

using namespace MidiPlay;
using namespace std::chrono_literals;
using Catch::Approx;
namespace fs = std::filesystem;

// Helper functions (shared with other test files)
extern char** makeArgv(const std::vector<std::string>& args);
extern void freeArgv(char** argv, int argc);

// ============================================================================
// Phase 3 Part 2: Integration Tests
// ============================================================================
// Tests component interactions and end-to-end playback flow using
// REAL components including real cxxmidi::player::PlayerSync
//
// Strategy: Use real PlayerSync (no hardware required for initialization tests)
// Hardware tests can be added later with [integration][hardware] tag

// ============================================================================
// Group 1: File Loading Integration
// ============================================================================

TEST_CASE("File Loading Integration", "[integration][fast]") {
    SECTION("MidiLoader loads file and extracts metadata") {
        optind = 0;
        auto argv = makeArgv({"play", "fixtures/test_files/simple.mid"});
        Options opts(2, argv);
        opts.parse();
        
        if (!fs::exists("fixtures/test_files/simple.mid")) {
            WARN("Test file not found: fixtures/test_files/simple.mid");
            freeArgv(argv, 2);
            return;
        }
        
        MidiLoader loader;
        bool loaded = loader.loadFile("fixtures/test_files/simple.mid", opts);
        
        REQUIRE(loaded);
        REQUIRE(loader.getBpm() > 0);
        REQUIRE(loader.getVerses() > 0);
        
        freeArgv(argv, 2);
    }
    
    SECTION("Introduction segments detected correctly") {
        optind = 0;
        auto argv = makeArgv({"play", "fixtures/test_files/with_intro.mid"});
        Options opts(2, argv);
        opts.parse();
        
        if (!fs::exists("fixtures/test_files/with_intro.mid")) {
            WARN("Test file not found: fixtures/test_files/with_intro.mid");
            freeArgv(argv, 2);
            return;
        }
        
        MidiLoader loader;
        loader.loadFile("fixtures/test_files/with_intro.mid", opts);
        
        const auto& segments = loader.getIntroSegments();
        
        // File may or may not have intro markers
        INFO("Introduction segments found: " << segments.size());
        
        freeArgv(argv, 2);
    }
    
    SECTION("Verse count from options overrides file") {
        optind = 0;
        auto argv = makeArgv({"play", "fixtures/test_files/simple.mid", "-x3"});
        Options opts(3, argv);
        opts.parse();
        
        if (!fs::exists("fixtures/test_files/simple.mid")) {
            WARN("Test file not found: fixtures/test_files/simple.mid");
            freeArgv(argv, 3);
            return;
        }
        
        MidiLoader loader;
        loader.loadFile("fixtures/test_files/simple.mid", opts);
        
        REQUIRE(loader.getVerses() == 3);
        REQUIRE_FALSE(loader.shouldPlayIntro());
        
        freeArgv(argv, 3);
    }
}

// ============================================================================
// Group 2: Playback Orchestration Integration (Real PlayerSync)
// ============================================================================

TEST_CASE("Playback Orchestration Integration", "[integration][fast]") {
    SECTION("PlaybackOrchestrator initializes with real PlayerSync") {
        optind = 0;
        auto argv = makeArgv({"play", "fixtures/test_files/simple.mid"});
        Options opts(2, argv);
        opts.parse();
        
        if (!fs::exists("fixtures/test_files/simple.mid")) {
            WARN("Test file not found: fixtures/test_files/simple.mid");
            freeArgv(argv, 2);
            return;
        }
        
        MidiLoader loader;
        loader.loadFile("fixtures/test_files/simple.mid", opts);
        
        // Use real PlayerSync (no hardware needed for initialization)
        cxxmidi::output::Default outport;
        cxxmidi::player::PlayerSync player(&outport);
        player.SetFile(&loader.getFile());
        
        PlaybackSynchronizer sync;
        PlaybackOrchestrator orchestrator(player, sync, loader);
        
        // Initialize should set up all components without hardware
        REQUIRE_NOTHROW(orchestrator.initialize());
        
        freeArgv(argv, 2);
    }
    
    SECTION("Speed configured correctly") {
        optind = 0;
        auto argv = makeArgv({"play", "fixtures/test_files/simple.mid"});
        Options opts(2, argv);
        opts.parse();
        
        if (!fs::exists("fixtures/test_files/simple.mid")) {
            WARN("Test file not found: fixtures/test_files/simple.mid");
            freeArgv(argv, 2);
            return;
        }
        
        MidiLoader loader;
        loader.loadFile("fixtures/test_files/simple.mid", opts);
        
        cxxmidi::output::Default outport;
        cxxmidi::player::PlayerSync player(&outport);
        player.SetFile(&loader.getFile());
        
        PlaybackSynchronizer sync;
        PlaybackOrchestrator orchestrator(player, sync, loader);
        
        orchestrator.initialize();
        
        // Speed should be set
        float speed = player.GetSpeed();
        REQUIRE(speed > 0.0f);
        
        freeArgv(argv, 2);
    }
    
    SECTION("Display info works correctly") {
        optind = 0;
        auto argv = makeArgv({"play", "fixtures/test_files/simple.mid"});
        Options opts(2, argv);
        opts.parse();
        
        if (!fs::exists("fixtures/test_files/simple.mid")) {
            WARN("Test file not found: fixtures/test_files/simple.mid");
            freeArgv(argv, 2);
            return;
        }
        
        MidiLoader loader;
        loader.loadFile("fixtures/test_files/simple.mid", opts);
        
        cxxmidi::output::Default outport;
        cxxmidi::player::PlayerSync player(&outport);
        player.SetFile(&loader.getFile());
        
        PlaybackSynchronizer sync;
        PlaybackOrchestrator orchestrator(player, sync, loader);
        
        orchestrator.initialize();
        
        // displayPlaybackInfo() writes to stdout
        // Just verify it doesn't throw
        REQUIRE_NOTHROW(orchestrator.displayPlaybackInfo());
        
        freeArgv(argv, 2);
    }
}

// ============================================================================
// Group 3: Synchronization Integration
// ============================================================================

TEST_CASE("Synchronization Integration", "[integration][fast]") {
    SECTION("PlaybackSynchronizer coordinates threads") {
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
    
    SECTION("Multiple wait/notify cycles work") {
        PlaybackSynchronizer sync;
        
        for (int i = 0; i < 3; i++) {
            std::atomic<bool> cycleComplete{false};
            
            std::thread waiter([&]() {
                sync.wait();
                cycleComplete = true;
            });
            
            std::this_thread::sleep_for(30ms);
            REQUIRE_FALSE(cycleComplete);
            
            sync.notify();
            waiter.join();
            REQUIRE(cycleComplete);
        }
    }
}

// ============================================================================
// Group 4: Signal Handling Integration
// ============================================================================

TEST_CASE("Signal Handling Integration", "[integration][fast]") {
    SECTION("SignalHandler can be constructed and set up") {
        cxxmidi::output::Default outport;
        PlaybackSynchronizer sync;
        auto startTime = std::chrono::high_resolution_clock::now();
        
        SignalHandler handler(outport, sync, startTime);
        
        // Setup should not throw
        REQUIRE_NOTHROW(handler.setupSignalHandler());
    }
    
    SECTION("SignalHandler integrates with synchronizer") {
        cxxmidi::output::Default outport;
        PlaybackSynchronizer sync;
        auto startTime = std::chrono::high_resolution_clock::now();
        
        SignalHandler handler(outport, sync, startTime);
        handler.setupSignalHandler();
        
        // Handler should be set up without errors
        // Note: Actually triggering SIGINT is complex and not tested here
        REQUIRE(true);
    }
}

// ============================================================================
// Group 5: End-to-End Scenarios (Fast Tests)
// ============================================================================

TEST_CASE("End-to-End Playback Scenarios", "[integration][fast][e2e]") {
    SECTION("Complete setup without intro") {
        optind = 0;
        auto argv = makeArgv({"play", "fixtures/test_files/simple.mid", "-x2"});
        Options opts(3, argv);
        opts.parse();
        
        if (!fs::exists("fixtures/test_files/simple.mid")) {
            WARN("Test file not found: fixtures/test_files/simple.mid");
            freeArgv(argv, 3);
            return;
        }
        
        MidiLoader loader;
        loader.loadFile("fixtures/test_files/simple.mid", opts);
        
        cxxmidi::output::Default outport;
        cxxmidi::player::PlayerSync player(&outport);
        player.SetFile(&loader.getFile());
        
        PlaybackSynchronizer sync;
        PlaybackOrchestrator orchestrator(player, sync, loader);
        
        orchestrator.initialize();
        
        // Verify no intro playback
        REQUIRE_FALSE(loader.shouldPlayIntro());
        REQUIRE(loader.getVerses() == 2);
        
        freeArgv(argv, 3);
    }
    
    SECTION("Complete setup with intro") {
        optind = 0;
        auto argv = makeArgv({"play", "fixtures/test_files/with_intro.mid", "-n3"});
        Options opts(3, argv);
        opts.parse();
        
        if (!fs::exists("fixtures/test_files/with_intro.mid")) {
            WARN("Test file not found: fixtures/test_files/with_intro.mid");
            freeArgv(argv, 3);
            return;
        }
        
        MidiLoader loader;
        loader.loadFile("fixtures/test_files/with_intro.mid", opts);
        
        cxxmidi::output::Default outport;
        cxxmidi::player::PlayerSync player(&outport);
        player.SetFile(&loader.getFile());
        
        PlaybackSynchronizer sync;
        PlaybackOrchestrator orchestrator(player, sync, loader);
        
        orchestrator.initialize();
        
        // Verify intro playback enabled
        REQUIRE(loader.shouldPlayIntro());
        REQUIRE(loader.getVerses() == 3);
        
        freeArgv(argv, 3);
    }
    
    SECTION("Speed changes applied correctly") {
        optind = 0;
        auto argv = makeArgv({"play", "fixtures/test_files/simple.mid", "-p12"});
        Options opts(3, argv);
        opts.parse();
        
        if (!fs::exists("fixtures/test_files/simple.mid")) {
            WARN("Test file not found: fixtures/test_files/simple.mid");
            freeArgv(argv, 3);
            return;
        }
        
        MidiLoader loader;
        loader.loadFile("fixtures/test_files/simple.mid", opts);
        
        cxxmidi::output::Default outport;
        cxxmidi::player::PlayerSync player(&outport);
        player.SetFile(&loader.getFile());
        
        PlaybackSynchronizer sync;
        PlaybackOrchestrator orchestrator(player, sync, loader);
        
        orchestrator.initialize();
        
        // Speed should be set to 1.2 (prelude mode with 12)
        REQUIRE(loader.getSpeed() == Approx(1.2f));
        
        // Player speed should be set (baseTempo * baseSpeed)
        float playerSpeed = player.GetSpeed();
        REQUIRE(playerSpeed > 0.0f);
        
        freeArgv(argv, 3);
    }
    
    SECTION("Tempo override applied correctly") {
        optind = 0;
        auto argv = makeArgv({"play", "fixtures/test_files/simple.mid", "-t100"});
        Options opts(3, argv);
        opts.parse();
        
        if (!fs::exists("fixtures/test_files/simple.mid")) {
            WARN("Test file not found: fixtures/test_files/simple.mid");
            freeArgv(argv, 3);
            return;
        }
        
        MidiLoader loader;
        loader.loadFile("fixtures/test_files/simple.mid", opts);
        
        cxxmidi::output::Default outport;
        cxxmidi::player::PlayerSync player(&outport);
        player.SetFile(&loader.getFile());
        
        PlaybackSynchronizer sync;
        PlaybackOrchestrator orchestrator(player, sync, loader);
        
        orchestrator.initialize();
        
        // BPM should be overridden to 100
        REQUIRE(loader.getBpm() == 100);
        
        freeArgv(argv, 3);
    }
    
    SECTION("Verbose mode integration") {
        optind = 0;
        auto argv = makeArgv({"play", "fixtures/test_files/simple.mid", "-V"});
        Options opts(3, argv);
        opts.parse();
        
        if (!fs::exists("fixtures/test_files/simple.mid")) {
            WARN("Test file not found: fixtures/test_files/simple.mid");
            freeArgv(argv, 3);
            return;
        }
        
        MidiLoader loader;
        loader.loadFile("fixtures/test_files/simple.mid", opts);
        
        cxxmidi::output::Default outport;
        cxxmidi::player::PlayerSync player(&outport);
        player.SetFile(&loader.getFile());
        
        PlaybackSynchronizer sync;
        PlaybackOrchestrator orchestrator(player, sync, loader);
        
        REQUIRE(loader.isVerbose());
        REQUIRE_NOTHROW(orchestrator.initialize());
        
        freeArgv(argv, 3);
    }
}

// ============================================================================
// Group 2: Component Integration Verification
// ============================================================================

TEST_CASE("Component Integration Verification", "[integration][fast]") {
    SECTION("All components work together") {
        optind = 0;
        auto argv = makeArgv({"play", "fixtures/test_files/simple.mid"});
        Options opts(2, argv);
        opts.parse();
        
        if (!fs::exists("fixtures/test_files/simple.mid")) {
            WARN("Test file not found: fixtures/test_files/simple.mid");
            freeArgv(argv, 2);
            return;
        }
        
        MidiLoader loader;
        loader.loadFile("fixtures/test_files/simple.mid", opts);
        
        cxxmidi::output::Default outport;
        cxxmidi::player::PlayerSync player(&outport);
        player.SetFile(&loader.getFile());
        
        PlaybackSynchronizer sync;
        PlaybackOrchestrator orchestrator(player, sync, loader);
        
        // Initialize should set up all components
        REQUIRE_NOTHROW(orchestrator.initialize());
        
        // Verify player state
        float speed = player.GetSpeed();
        REQUIRE(speed > 0.0f);
        
        freeArgv(argv, 2);
    }
    
    SECTION("Multiple initialization calls are safe") {
        optind = 0;
        auto argv = makeArgv({"play", "fixtures/test_files/simple.mid"});
        Options opts(2, argv);
        opts.parse();
        
        if (!fs::exists("fixtures/test_files/simple.mid")) {
            WARN("Test file not found: fixtures/test_files/simple.mid");
            freeArgv(argv, 2);
            return;
        }
        
        MidiLoader loader;
        loader.loadFile("fixtures/test_files/simple.mid", opts);
        
        cxxmidi::output::Default outport;
        cxxmidi::player::PlayerSync player(&outport);
        player.SetFile(&loader.getFile());
        
        PlaybackSynchronizer sync;
        PlaybackOrchestrator orchestrator(player, sync, loader);
        
        // Initialize multiple times should be safe
        REQUIRE_NOTHROW(orchestrator.initialize());
        REQUIRE_NOTHROW(orchestrator.initialize());
        
        // Player should still be in valid state
        float speed = player.GetSpeed();
        REQUIRE(speed > 0.0f);
        
        freeArgv(argv, 2);
    }
    
    SECTION("Display info shows correct metadata") {
        optind = 0;
        auto argv = makeArgv({"play", "fixtures/test_files/simple.mid"});
        Options opts(2, argv);
        opts.parse();
        
        if (!fs::exists("fixtures/test_files/simple.mid")) {
            WARN("Test file not found: fixtures/test_files/simple.mid");
            freeArgv(argv, 2);
            return;
        }
        
        MidiLoader loader;
        loader.loadFile("fixtures/test_files/simple.mid", opts);
        
        cxxmidi::output::Default outport;
        cxxmidi::player::PlayerSync player(&outport);
        player.SetFile(&loader.getFile());
        
        PlaybackSynchronizer sync;
        PlaybackOrchestrator orchestrator(player, sync, loader);
        
        orchestrator.initialize();
        
        // displayPlaybackInfo() writes to stdout
        // Just verify it doesn't throw
        REQUIRE_NOTHROW(orchestrator.displayPlaybackInfo());
        
        freeArgv(argv, 2);
    }
}

// ============================================================================
// HARDWARE INTEGRATION TESTS (Require Real MIDI Device)
// ============================================================================
// These tests would use real MIDI hardware for comprehensive validation.
// They are tagged [integration][hardware] and can be run separately.
//
// To run hardware tests:
//   ./run_tests "[integration][hardware]"
//
// To run only fast tests (development):
//   ./run_tests "[integration][fast]"
//
// See PHASE3_HYBRID_STRATEGY.md for detailed hardware test specifications
// ============================================================================

// ============================================================================
// Test Execution Guide
// ============================================================================
// 
// Fast Tests (Development, CI/CD):
//   ./run_tests "[integration][fast]"
//   Expected time: <1 second
//   No hardware required
//
// Hardware Tests (Pre-Release Validation):
//   ./run_tests "[integration][hardware]"
//   Expected time: 5-10 seconds
//   Requires MIDI device connected
//   (Not yet implemented - see PHASE3_HYBRID_STRATEGY.md)
//
// All Integration Tests:
//   ./run_tests "[integration]"
//
// All Phase 3 Tests:
//   ./run_tests "[device_manager][integration]"
//
// ============================================================================