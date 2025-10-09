#include "external/catch_amalgamated.hpp"
#include "../playback_state_machine.hpp"

using namespace MidiPlay;

TEST_CASE("PlaybackStateMachine initialization", "[state][unit]") {
    PlaybackStateMachine sm;
    
    REQUIRE_FALSE(sm.isPlayingIntro());
    REQUIRE_FALSE(sm.isRitardando());
    REQUIRE_FALSE(sm.isLastVerse());
    REQUIRE_FALSE(sm.isAlFine());
    REQUIRE_FALSE(sm.shouldDisplayWarnings());
}

TEST_CASE("PlaybackStateMachine state transitions", "[state][unit]") {
    PlaybackStateMachine sm;
    
    SECTION("intro state toggles correctly") {
        sm.setPlayingIntro(true);
        REQUIRE(sm.isPlayingIntro());
        
        sm.setPlayingIntro(false);
        REQUIRE_FALSE(sm.isPlayingIntro());
    }
    
    SECTION("ritardando state toggles correctly") {
        sm.setRitardando(true);
        REQUIRE(sm.isRitardando());
        
        sm.setRitardando(false);
        REQUIRE_FALSE(sm.isRitardando());
    }
    
    SECTION("lastVerse state toggles correctly") {
        sm.setLastVerse(true);
        REQUIRE(sm.isLastVerse());
        
        sm.setLastVerse(false);
        REQUIRE_FALSE(sm.isLastVerse());
    }
    
    SECTION("alFine state toggles correctly") {
        sm.setAlFine(true);
        REQUIRE(sm.isAlFine());
        
        sm.setAlFine(false);
        REQUIRE_FALSE(sm.isAlFine());
    }
    
    SECTION("displayWarnings state toggles correctly") {
        sm.setDisplayWarnings(true);
        REQUIRE(sm.shouldDisplayWarnings());
        
        sm.setDisplayWarnings(false);
        REQUIRE_FALSE(sm.shouldDisplayWarnings());
    }
    
    SECTION("multiple states operate independently") {
        sm.setPlayingIntro(true);
        sm.setRitardando(true);
        
        REQUIRE(sm.isPlayingIntro());
        REQUIRE(sm.isRitardando());
        REQUIRE_FALSE(sm.isLastVerse());
        REQUIRE_FALSE(sm.isAlFine());
    }
}

TEST_CASE("PlaybackStateMachine reset behavior", "[state][unit]") {
    PlaybackStateMachine sm;
    
    // Set all flags
    sm.setPlayingIntro(true);
    sm.setRitardando(true);
    sm.setLastVerse(true);
    sm.setAlFine(true);
    sm.setDisplayWarnings(true);
    
    // Verify all set
    REQUIRE(sm.isPlayingIntro());
    REQUIRE(sm.isRitardando());
    REQUIRE(sm.isLastVerse());
    REQUIRE(sm.isAlFine());
    REQUIRE(sm.shouldDisplayWarnings());
    
    // Reset
    sm.reset();
    
    // Verify playback flags cleared (displayWarnings persists)
    REQUIRE_FALSE(sm.isPlayingIntro());
    REQUIRE_FALSE(sm.isRitardando());
    REQUIRE_FALSE(sm.isLastVerse());
    REQUIRE_FALSE(sm.isAlFine());
    REQUIRE(sm.shouldDisplayWarnings());  // NOT cleared by reset()
}

TEST_CASE("PlaybackStateMachine state combinations", "[state][unit]") {
    PlaybackStateMachine sm;
    
    SECTION("can set all playback states together") {
        sm.setPlayingIntro(true);
        sm.setRitardando(true);
        sm.setLastVerse(true);
        sm.setAlFine(true);
        
        REQUIRE(sm.isPlayingIntro());
        REQUIRE(sm.isRitardando());
        REQUIRE(sm.isLastVerse());
        REQUIRE(sm.isAlFine());
    }
    
    SECTION("can clear states independently") {
        sm.setPlayingIntro(true);
        sm.setRitardando(true);
        sm.setLastVerse(true);
        
        sm.setRitardando(false);
        
        REQUIRE(sm.isPlayingIntro());
        REQUIRE_FALSE(sm.isRitardando());
        REQUIRE(sm.isLastVerse());
    }
}

TEST_CASE("PlaybackStateMachine multiple resets", "[state][unit]") {
    PlaybackStateMachine sm;
    
    // First cycle
    sm.setPlayingIntro(true);
    sm.setRitardando(true);
    sm.reset();
    REQUIRE_FALSE(sm.isPlayingIntro());
    REQUIRE_FALSE(sm.isRitardando());
    
    // Second cycle
    sm.setLastVerse(true);
    sm.setAlFine(true);
    sm.reset();
    REQUIRE_FALSE(sm.isLastVerse());
    REQUIRE_FALSE(sm.isAlFine());
    
    // Verify reset is idempotent
    sm.reset();
    REQUIRE_FALSE(sm.isPlayingIntro());
    REQUIRE_FALSE(sm.isRitardando());
    REQUIRE_FALSE(sm.isLastVerse());
    REQUIRE_FALSE(sm.isAlFine());
}