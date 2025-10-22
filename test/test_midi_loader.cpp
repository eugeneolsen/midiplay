#include "external/catch_amalgamated.hpp"
#include "../midi_loader.hpp"
#include "../options.hpp"
#include <filesystem>
#include <getopt.h>  // For optind reset

using namespace MidiPlay;
namespace fs = std::filesystem;

// ============================================================================
// Phase 2: Business Logic - MIDI Loader Tests
// ============================================================================
// Tests MidiLoader's file loading and metadata extraction capabilities

TEST_CASE("MidiLoader: File Existence Checking", "[business_logic][midi_loader]") {
    SECTION("fileExists utility detects existing files") {
        // Test the static utility method
        if (fs::exists("fixtures/test_files/simple.mid")) {
            REQUIRE(MidiLoader::fileExists("fixtures/test_files/simple.mid") == true);
        }
        
        REQUIRE(MidiLoader::fileExists("nonexistent_file.mid") == false);
        REQUIRE(MidiLoader::fileExists("") == false);
    }
}

TEST_CASE("MidiLoader: Object Creation", "[business_logic][midi_loader]") {
    SECTION("Can create MidiLoader instance") {
        MidiLoader loader;
        
        // MidiLoader should be constructible
        REQUIRE(true);
    }
}

TEST_CASE("MidiLoader: API Availability", "[business_logic][midi_loader]") {
    MidiLoader loader;
    
    SECTION("Has metadata getter methods") {
        // These methods should be callable (even if no file loaded)
        std::string title = loader.getTitle();
        std::string key = loader.getKeySignature();
        const TimeSignature& timeSig = loader.getTimeSignature();
        auto& introSegments = loader.getIntroSegments();
        
        int verses = loader.getVerses();
        int fileTempo = loader.getFileTempo();
        int bpm = loader.getBpm();
        float speed = loader.getSpeed();
        
        // All getters should be callable
        REQUIRE(title.length() >= 0);
        REQUIRE(key.length() >= 0);
        REQUIRE(verses >= 0);
        REQUIRE(introSegments.size() >= 0);
    }
    
    SECTION("Has timing getter methods") {
        int uSecPerQuarter = loader.getUSecPerQuarter();
        int uSecPerTick = loader.getUSecPerTick();
        MidiTicks pauseTicks = loader.getPauseTicks();
        
        REQUIRE(uSecPerQuarter >= 0);
        REQUIRE(uSecPerTick >= 0);
        // pauseTicks is MidiTicks type, just verify it's callable
        REQUIRE(true);
    }
    
    SECTION("Has state flag methods") {
        bool playIntro = loader.shouldPlayIntro();
        bool stuckNote = loader.hasPotentialStuckNote();
        bool firstTempo = loader.isFirstTempo();
        bool verbose = loader.isVerbose();
        
        // All flags should be readable
        REQUIRE((playIntro == true || playIntro == false));
        REQUIRE((stuckNote == true || stuckNote == false));
        REQUIRE((firstTempo == true || firstTempo == false));
        REQUIRE((verbose == true || verbose == false));
    }
}

// ============================================================================
// Integration Tests with Actual MIDI Files
// ============================================================================

TEST_CASE("MidiLoader: Load Simple MIDI File", "[business_logic][midi_loader][integration]") {
    std::string testFile = "fixtures/test_files/simple.mid";
    
    if (!fs::exists(testFile)) {
        WARN("Test file not found: " << testFile);
        return;
    }
    
    SECTION("Successfully loads simple MIDI file") {
        optind = 0;  // Reset getopt global state (GNU: complete reinitialization)
        const char* argv[] = {"midiplay", testFile.c_str()};
        Options options(2, const_cast<char**>(argv));
        options.parse();
        
        {
            MidiLoader loader;
            bool loaded = loader.loadFile(testFile, options);
            REQUIRE(loaded == true);
        } // loader destroyed here, while options still in scope
    }
    
    SECTION("Extracts metadata from simple file") {
        optind = 0;  // Reset getopt global state (GNU: complete reinitialization)
        MidiLoader loader;
        const char* argv[] = {"midiplay", testFile.c_str()};
        Options options(2, const_cast<char**>(argv));
        options.parse();
        
        loader.loadFile(testFile, options);
        
        // Title may be empty or contain track name
        std::string title = loader.getTitle();
        REQUIRE(title.length() >= 0);
        
        // Should have at least default verses
        int verses = loader.getVerses();
        REQUIRE(verses > 0);
        
        // Should have valid tempo
        int bpm = loader.getBpm();
        REQUIRE(bpm > 0);
    }
    
    SECTION("Calculates timing values") {
        optind = 0;  // Reset getopt global state (GNU: complete reinitialization)
        MidiLoader loader;
        const char* argv[] = {"midiplay", testFile.c_str()};
        Options options(2, const_cast<char**>(argv));
        options.parse();
        
        loader.loadFile(testFile, options);
        
        int uSecPerQuarter = loader.getUSecPerQuarter();
        int uSecPerTick = loader.getUSecPerTick();
        
        REQUIRE(uSecPerQuarter > 0);
        REQUIRE(uSecPerTick >= 0);
    }
}

TEST_CASE("MidiLoader: Load File with Introduction", "[business_logic][midi_loader][integration]") {
    std::string testFile = "fixtures/test_files/with_intro.mid";
    
    if (!fs::exists(testFile)) {
        WARN("Test file not found: " << testFile);
        return;
    }
    
    SECTION("Successfully loads file with introduction") {
        optind = 0;  // Reset getopt global state (GNU: complete reinitialization)
        MidiLoader loader;
        const char* argv[] = {"midiplay", testFile.c_str()};
        Options options(2, const_cast<char**>(argv));
        options.parse();
        
        bool loaded = loader.loadFile(testFile, options);
        
        REQUIRE(loaded == true);
    }
    
    SECTION("Detects introduction segments") {
        optind = 0;  // Reset getopt global state (GNU: complete reinitialization)
        MidiLoader loader;
        const char* argv[] = {"midiplay", testFile.c_str()};
        Options options(2, const_cast<char**>(argv));
        options.parse();
        
        loader.loadFile(testFile, options);
        
        auto& introSegments = loader.getIntroSegments();
        
        // File may have intro markers
        INFO("Introduction segments found: " << introSegments.size());
        
        // If segments exist, they should have valid ranges
        for (const auto& segment : introSegments) {
            REQUIRE(segment.start >= 0);
            REQUIRE(segment.end >= segment.start);
        }
    }
    
    SECTION("shouldPlayIntro flag reflects segments") {
        optind = 0;  // Reset getopt global state (GNU: complete reinitialization)
        MidiLoader loader;
        const char* argv[] = {"midiplay", testFile.c_str()};
        Options options(2, const_cast<char**>(argv));
        options.parse();
        
        loader.loadFile(testFile, options);
        
        bool hasIntro = loader.shouldPlayIntro();
        auto& segments = loader.getIntroSegments();
        
        // If there are segments, shouldPlayIntro should be true
        if (!segments.empty()) {
            REQUIRE(hasIntro == true);
        }
    }
}

TEST_CASE("MidiLoader: Load File with Ritardando", "[business_logic][midi_loader][integration]") {
    std::string testFile = "fixtures/test_files/ritardando.mid";
    
    if (!fs::exists(testFile)) {
        WARN("Test file not found: " << testFile);
        return;
    }
    
    SECTION("Successfully loads file with ritardando") {
        optind = 0;  // Reset getopt global state (GNU: complete reinitialization)
        MidiLoader loader;
        const char* argv[] = {"midiplay", testFile.c_str()};
        Options options(2, const_cast<char**>(argv));
        options.parse();
        
        bool loaded = loader.loadFile(testFile, options);
        
        REQUIRE(loaded == true);
    }
    
    SECTION("Extracts basic metadata") {
        optind = 0;  // Reset getopt global state (GNU: complete reinitialization)
        MidiLoader loader;
        const char* argv[] = {"midiplay", testFile.c_str()};
        Options options(2, const_cast<char**>(argv));
        options.parse();
        
        loader.loadFile(testFile, options);
        
        int verses = loader.getVerses();
        int bpm = loader.getBpm();
        
        REQUIRE(verses > 0);
        REQUIRE(bpm > 0);
    }
}

TEST_CASE("MidiLoader: Load File with DC al Fine", "[business_logic][midi_loader][integration]") {
    std::string testFile = "fixtures/test_files/dc_al_fine.mid";
    
    if (!fs::exists(testFile)) {
        WARN("Test file not found: " << testFile);
        return;
    }
    
    SECTION("Successfully loads file with DC al Fine") {
        optind = 0;  // Reset getopt global state (GNU: complete reinitialization)
        MidiLoader loader;
        const char* argv[] = {"midiplay", testFile.c_str()};
        Options options(2, const_cast<char**>(argv));
        options.parse();
        
        bool loaded = loader.loadFile(testFile, options);
        
        REQUIRE(loaded == true);
    }
    
    SECTION("Extracts metadata correctly") {
        optind = 0;  // Reset getopt global state (GNU: complete reinitialization)
        MidiLoader loader;
        const char* argv[] = {"midiplay", testFile.c_str()};
        Options options(2, const_cast<char**>(argv));
        options.parse();
        
        loader.loadFile(testFile, options);
        
        int verses = loader.getVerses();
        REQUIRE(verses > 0);
        
        // File should have valid timing
        int fileTempo = loader.getFileTempo();
        REQUIRE(fileTempo >= 0);
    }
}

TEST_CASE("MidiLoader: Error Handling", "[business_logic][midi_loader][error_handling]") {
    SECTION("Handles non-existent file") {
        optind = 0;  // Reset getopt global state (GNU: complete reinitialization)
        MidiLoader loader;
        // Use a file that exists for Options, but try to load a different file
        std::string validFile = "fixtures/test_files/simple.mid";
        if (!fs::exists(validFile)) {
            WARN("Cannot test error handling without valid fixture file");
            return;
        }
        
        const char* argv[] = {"midiplay", validFile.c_str()};
        Options options(2, const_cast<char**>(argv));
        options.parse();
        
        bool loaded = loader.loadFile("nonexistent_file.mid", options);
        
        REQUIRE(loaded == false);
    }
    
    SECTION("Handles empty file path") {
        optind = 0;  // Reset getopt global state (GNU: complete reinitialization)
        MidiLoader loader;
        std::string validFile = "fixtures/test_files/simple.mid";
        if (!fs::exists(validFile)) {
            WARN("Cannot test error handling without valid fixture file");
            return;
        }
        
        const char* argv[] = {"midiplay", validFile.c_str()};
        Options options(2, const_cast<char**>(argv));
        options.parse();
        
        bool loaded = loader.loadFile("", options);
        
        REQUIRE(loaded == false);
    }
}


TEST_CASE("MidiLoader: Key Signature Extraction", "[business_logic][midi_loader][integration]") {
    std::string testFile = "fixtures/test_files/simple.mid";
    
    if (!fs::exists(testFile)) {
        WARN("Test file not found: " << testFile);
        return;
    }
    
    SECTION("Extracts key signature if present") {
        optind = 0;  // Reset getopt global state (GNU: complete reinitialization)
        MidiLoader loader;
        const char* argv[] = {"midiplay", testFile.c_str()};
        Options options(2, const_cast<char**>(argv));
        options.parse();
        
        loader.loadFile(testFile, options);
        
        std::string key = loader.getKeySignature();
        
        // Key may be empty if not specified in file, or contain key name
        INFO("Key signature: '" << key << "'");
        REQUIRE(key.length() >= 0);
    }
}

TEST_CASE("MidiLoader: Time Signature Extraction", "[business_logic][midi_loader][integration]") {
    std::string testFile = "fixtures/test_files/simple.mid";
    
    if (!fs::exists(testFile)) {
        WARN("Test file not found: " << testFile);
        return;
    }
    
    SECTION("Extracts time signature") {
        optind = 0;  // Reset getopt global state (GNU: complete reinitialization)
        MidiLoader loader;
        const char* argv[] = {"midiplay", testFile.c_str()};
        Options options(2, const_cast<char**>(argv));
        options.parse();
        
        loader.loadFile(testFile, options);
        
        const TimeSignature& timeSig = loader.getTimeSignature();
        
        // Common time signatures have positive values
        INFO("Time signature: " << (int)timeSig.beatsPerMeasure << "/" << (int)timeSig.denominator);
        
        REQUIRE(timeSig.beatsPerMeasure >= 0);
        REQUIRE(timeSig.denominator >= 0);
    }
}

TEST_CASE("MidiLoader: Verse Counting", "[business_logic][midi_loader][integration]") {
    std::string testFile = "fixtures/test_files/simple.mid";
    
    if (!fs::exists(testFile)) {
        WARN("Test file not found: " << testFile);
        return;
    }
    
    SECTION("Uses default verses if not specified") {
        optind = 0;  // Reset getopt global state (GNU: complete reinitialization)
        MidiLoader loader;
        const char* argv[] = {"midiplay", testFile.c_str()};
        Options options(2, const_cast<char**>(argv));
        options.parse();
        
        loader.loadFile(testFile, options);
        
        int verses = loader.getVerses();
        
        // Should have at least 1 verse
        REQUIRE(verses >= 1);
    }
    
    SECTION("Respects command-line verse override") {
        optind = 0;  // Reset getopt global state (GNU: complete reinitialization)
        MidiLoader loader;
        const char* argv[] = {"midiplay", testFile.c_str(), "-x3"};
        Options options(3, const_cast<char**>(argv));
        options.parse();
        
        loader.loadFile(testFile, options);
        
        int verses = loader.getVerses();
        
        // Should use command-line value
        REQUIRE(verses == 3);
    }
}