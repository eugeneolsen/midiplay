#include "external/catch_amalgamated.hpp"
#include "../playback_synchronizer.hpp"
#include <thread>
#include <chrono>
#include <atomic>

using namespace MidiPlay;
using namespace std::chrono_literals;

TEST_CASE("PlaybackSynchronizer basic synchronization", "[sync][unit]") {
    PlaybackSynchronizer sync;
    std::atomic<bool> threadWokeUp{false};
    
    SECTION("wait blocks until notify") {
        std::thread waiter([&]() {
            sync.wait();
            threadWokeUp = true;
        });
        
        // Give thread time to enter wait
        std::this_thread::sleep_for(50ms);
        
        // Thread should still be waiting
        REQUIRE_FALSE(threadWokeUp.load());
        
        // Notify the thread
        sync.notify();
        waiter.join();
        
        // Thread should have woken
        REQUIRE(threadWokeUp.load());
    }
}

TEST_CASE("PlaybackSynchronizer multiple cycles", "[sync][unit]") {
    PlaybackSynchronizer sync;
    
    for (int i = 0; i < 3; i++) {
        std::atomic<bool> cycleComplete{false};
        
        std::thread t([&]() {
            sync.wait();
            cycleComplete = true;
        });
        
        std::this_thread::sleep_for(30ms);
        sync.notify();
        t.join();
        
        REQUIRE(cycleComplete.load());
    }
}

TEST_CASE("PlaybackSynchronizer edge cases", "[sync][unit]") {
    PlaybackSynchronizer sync;
    
    SECTION("notify before wait does not block") {
        sync.notify();
        
        auto start = std::chrono::high_resolution_clock::now();
        
        std::thread t([&]() {
            sync.wait();
        });
        t.join();
        
        auto duration = std::chrono::high_resolution_clock::now() - start;
        auto durationMs = std::chrono::duration_cast<std::chrono::milliseconds>(duration);
        
        REQUIRE(durationMs.count() < 100);  // Should complete quickly
    }
    
    SECTION("reset clears finished flag") {
        sync.notify();
        sync.reset();
        
        std::atomic<bool> wokeUp{false};
        std::thread t([&]() {
            sync.wait();
            wokeUp = true;
        });
        
        std::this_thread::sleep_for(50ms);
        REQUIRE_FALSE(wokeUp.load());
        
        sync.notify();
        t.join();
        REQUIRE(wokeUp.load());
    }
    
    SECTION("multiple notify calls are safe") {
        std::atomic<bool> wokeUp{false};
        
        std::thread t([&]() {
            sync.wait();
            wokeUp = true;
        });
        
        std::this_thread::sleep_for(30ms);
        
        // Multiple notifies should be safe
        sync.notify();
        sync.notify();
        sync.notify();
        
        t.join();
        REQUIRE(wokeUp.load());
    }
}

SCENARIO("Playback completes and main thread resumes", "[sync][scenario]") {
    GIVEN("A synchronizer and waiting main thread") {
        PlaybackSynchronizer sync;
        std::atomic<bool> playbackComplete{false};
        
        WHEN("Playback thread finishes and notifies") {
            std::thread playback([&]() {
                std::this_thread::sleep_for(100ms);
                playbackComplete = true;
                sync.notify();
            });
            
            THEN("Main thread wakes up after notification") {
                sync.wait();
                REQUIRE(playbackComplete.load());
                playback.join();
            }
        }
    }
}

TEST_CASE("PlaybackSynchronizer timing verification", "[sync][unit][threading]") {
    PlaybackSynchronizer sync;
    
    SECTION("wait doesn't return prematurely") {
        auto start = std::chrono::high_resolution_clock::now();
        std::atomic<bool> notified{false};
        
        std::thread waiter([&]() {
            sync.wait();
        });
        
        // Wait for a specific delay before notifying
        std::this_thread::sleep_for(100ms);
        notified = true;
        sync.notify();
        
        waiter.join();
        auto duration = std::chrono::high_resolution_clock::now() - start;
        auto durationMs = std::chrono::duration_cast<std::chrono::milliseconds>(duration);
        
        // Should have waited at least 100ms
        REQUIRE(durationMs.count() >= 100);
        REQUIRE(notified.load());
    }
}

TEST_CASE("PlaybackSynchronizer thread safety", "[sync][unit][threading]") {
    PlaybackSynchronizer sync;
    
    SECTION("sequential wait-notify cycles are safe") {
        for (int cycle = 0; cycle < 5; cycle++) {
            std::atomic<int> completionCount{0};
            
            std::thread t([&]() {
                sync.wait();
                completionCount++;
            });
            
            std::this_thread::sleep_for(20ms);
            sync.notify();
            t.join();
            
            REQUIRE(completionCount.load() == 1);
        }
    }
}

TEST_CASE("PlaybackSynchronizer reset behavior", "[sync][unit]") {
    PlaybackSynchronizer sync;
    
    SECTION("reset after notify allows new wait") {
        sync.notify();
        
        // First wait should not block (flag is set)
        std::thread t1([&]() {
            sync.wait();
        });
        t1.join();
        
        // Reset the flag
        sync.reset();
        
        // Now wait should block again
        std::atomic<bool> wokeUp{false};
        std::thread t2([&]() {
            sync.wait();
            wokeUp = true;
        });
        
        std::this_thread::sleep_for(50ms);
        REQUIRE_FALSE(wokeUp.load());
        
        sync.notify();
        t2.join();
        REQUIRE(wokeUp.load());
    }
}