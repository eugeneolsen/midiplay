#include "external/catch_amalgamated.hpp"
#include "../timing_manager.hpp"
#include <thread>
#include <chrono>

using namespace MidiPlay;
using namespace std::chrono_literals;
using Catch::Matchers::WithinRel;

TEST_CASE("TimingManager basic timing", "[timing][unit]") {
    TimingManager tm;
    
    SECTION("measures elapsed time") {
        tm.startTimer();
        std::this_thread::sleep_for(100ms);
        tm.endTimer();
        
        double elapsed = tm.getElapsedSeconds();
        
        // Within 50% tolerance (0.05-0.15s for 0.1s sleep)
        REQUIRE_THAT(elapsed, WithinRel(0.1, 0.5));
    }
    
    SECTION("multiple measurements") {
        for (int i = 0; i < 3; i++) {
            tm.startTimer();
            std::this_thread::sleep_for(50ms);
            tm.endTimer();
            
            double elapsed = tm.getElapsedSeconds();
            REQUIRE(elapsed > 0.0);
            REQUIRE(elapsed < 0.2);  // Should be roughly 50ms
        }
    }
}

TEST_CASE("TimingManager formatted time", "[timing][unit]") {
    TimingManager tm;
    
    SECTION("formats seconds correctly") {
        tm.startTimer();
        std::this_thread::sleep_for(1500ms);
        tm.endTimer();
        
        std::string formatted = tm.getFormattedElapsedTime();
        
        // Should be "0:01" or "0:02"
        REQUIRE((formatted == "0:01" || formatted == "0:02"));
    }
    
    SECTION("formats minutes correctly") {
        tm.startTimer();
        std::this_thread::sleep_for(65000ms);  // ~65 seconds
        tm.endTimer();
        
        std::string formatted = tm.getFormattedElapsedTime();
        
        // Should be "1:05" or close to it
        REQUIRE((formatted == "1:04" || formatted == "1:05" || formatted == "1:06"));
    }
}

TEST_CASE("TimingManager edge cases", "[timing][unit]") {
    TimingManager tm;
    
    SECTION("very short duration") {
        tm.startTimer();
        tm.endTimer();
        
        double elapsed = tm.getElapsedSeconds();
        REQUIRE(elapsed >= 0.0);
        REQUIRE(elapsed < 0.01);
        
        std::string formatted = tm.getFormattedElapsedTime();
        REQUIRE(formatted == "0:00");
    }
    
    SECTION("zero duration formatting") {
        tm.startTimer();
        tm.endTimer();
        
        std::string formatted = tm.getFormattedElapsedTime();
        REQUIRE(formatted == "0:00");
    }
    
    SECTION("multiple cycles work") {
        for (int i = 0; i < 3; i++) {
            tm.startTimer();
            std::this_thread::sleep_for(50ms);
            tm.endTimer();
            
            double elapsed = tm.getElapsedSeconds();
            REQUIRE(elapsed > 0.0);
        }
    }
}

TEST_CASE("TimingManager reference validity", "[timing][unit]") {
    TimingManager tm;
    
    tm.startTimer();
    
    auto& startTime = tm.getStartTime();
    auto now = std::chrono::high_resolution_clock::now();
    
    auto diff = std::chrono::duration_cast<std::chrono::seconds>(now - startTime);
    
    REQUIRE(diff.count() <= 1);  // Start time should be very recent
}

TEST_CASE("TimingManager elapsed calculation", "[timing][unit]") {
    TimingManager tm;
    
    SECTION("elapsed is positive after delay") {
        tm.startTimer();
        std::this_thread::sleep_for(200ms);
        tm.endTimer();
        
        double elapsed = tm.getElapsedSeconds();
        REQUIRE(elapsed > 0.0);
        REQUIRE(elapsed >= 0.2);  // At least 200ms
        REQUIRE(elapsed < 0.5);   // But not too much more
    }
    
    SECTION("elapsed increases with longer delays") {
        tm.startTimer();
        std::this_thread::sleep_for(100ms);
        tm.endTimer();
        double elapsed1 = tm.getElapsedSeconds();
        
        tm.startTimer();
        std::this_thread::sleep_for(200ms);
        tm.endTimer();
        double elapsed2 = tm.getElapsedSeconds();
        
        REQUIRE(elapsed2 > elapsed1);
    }
}

TEST_CASE("TimingManager getStartTime reference", "[timing][unit]") {
    TimingManager tm;
    
    SECTION("returns valid reference") {
        tm.startTimer();
        
        const auto& ref1 = tm.getStartTime();
        const auto& ref2 = tm.getStartTime();
        
        // Both references should point to the same time
        REQUIRE(ref1 == ref2);
    }
}

TEST_CASE("TimingManager time formatting precision", "[timing][unit]") {
    TimingManager tm;
    
    SECTION("formats subsecond times as 00:00") {
        tm.startTimer();
        std::this_thread::sleep_for(500ms);
        tm.endTimer();
        
        std::string formatted = tm.getFormattedElapsedTime();
        REQUIRE(formatted == "0:00");  // Less than 1 second
    }
    
    SECTION("formats exact second boundaries") {
        tm.startTimer();
        std::this_thread::sleep_for(1000ms);
        tm.endTimer();
        
        std::string formatted = tm.getFormattedElapsedTime();
        REQUIRE((formatted == "0:01" || formatted == "0:00"));  // Allow rounding
    }
}