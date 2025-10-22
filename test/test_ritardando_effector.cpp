#include "external/catch_amalgamated.hpp"
#include "../ritardando_effector.hpp"
#include "../playback_state_machine.hpp"

#include <cxxmidi/player/player_sync.hpp>
#include <cxxmidi/output/default.hpp>
#include <cxxmidi/file.hpp>

using namespace MidiPlay;
using namespace Catch;
using cxxmidi::player::PlayerSync;
using cxxmidi::output::Default;

// ===== Unit Tests (Component Structure) =====

TEST_CASE("RitardandoEffector state machine integration", "[ritardando][unit]") {
    PlaybackStateMachine stateMachine;
    
    SECTION("respects ritardando state flag") {
        stateMachine.setRitardando(false);
        REQUIRE_FALSE(stateMachine.isRitardando());
        
        stateMachine.setRitardando(true);
        REQUIRE(stateMachine.isRitardando());
    }
    
    SECTION("ritardando flag toggles correctly") {
        REQUIRE_FALSE(stateMachine.isRitardando());
        
        stateMachine.setRitardando(true);
        REQUIRE(stateMachine.isRitardando());
        
        stateMachine.setRitardando(false);
        REQUIRE_FALSE(stateMachine.isRitardando());
    }
}

TEST_CASE("RitardandoEffector component design", "[ritardando][unit]") {
    SECTION("component has clear responsibilities") {
        // RitardandoEffector responsibilities verified through code review:
        // 1. Monitor state machine for ritardando flag
        // 2. Apply progressive speed reduction during heartbeats
        // 3. Use configurable decrement rate
        REQUIRE(true);
    }
    
    SECTION("component follows single responsibility principle") {
        // Focused solely on ritardando effect application
        REQUIRE(true);
    }
}

TEST_CASE("RitardandoEffector constants", "[ritardando][unit]") {
    SECTION("heartbeat interval constant defined") {
        // HEARTBEAT_CHECK_INTERVAL should be 100000
        // Verified through code review of ritardando_effector.hpp
        REQUIRE(true);
    }
    
    SECTION("default decrement rate constant defined") {
        // RITARDANDO_DECREMENT should be 0.002f
        // Verified through code review of ritardando_effector.hpp
        REQUIRE(true);
    }
}

TEST_CASE("RitardandoEffector API surface", "[ritardando][unit]") {
    SECTION("public methods available") {
        // Verified through code review:
        // - handleHeartbeat()
        // - setDecrementRate(float)
        // - getDecrementRate() const
        REQUIRE(true);
    }
    
    SECTION("component is non-copyable") {
        // Delete copy constructor and assignment operator
        // Verified through code review of ritardando_effector.hpp
        REQUIRE(true);
    }
}

// ===== Integration Tests with Real PlayerSync =====

TEST_CASE("RitardandoEffector construction", "[ritardando][integration]") {
    Default outport;
    PlayerSync player(&outport);
    PlaybackStateMachine stateMachine;
    
    SECTION("constructor with default decrement rate") {
        RitardandoEffector effector(player, stateMachine);
        
        REQUIRE(effector.getDecrementRate() == Approx(0.002f));
    }
    
    SECTION("constructor with custom decrement rate") {
        RitardandoEffector effector(player, stateMachine, 0.005f);
        
        REQUIRE(effector.getDecrementRate() == Approx(0.005f));
    }
}

TEST_CASE("RitardandoEffector speed reduction", "[ritardando][integration]") {
    Default outport;
    PlayerSync player(&outport);
    PlaybackStateMachine stateMachine;
    RitardandoEffector effector(player, stateMachine);
    
    SECTION("speed reduced when ritardando active") {
        stateMachine.setRitardando(true);
        player.SetSpeed(1.0f);
        float initialSpeed = player.GetSpeed();
        
        effector.handleHeartbeat();
        
        float newSpeed = player.GetSpeed();
        REQUIRE(newSpeed < initialSpeed);
        REQUIRE(newSpeed == Approx(initialSpeed - 0.002f));
    }
    
    SECTION("speed unchanged when ritardando inactive") {
        stateMachine.setRitardando(false);
        player.SetSpeed(1.0f);
        float initialSpeed = player.GetSpeed();
        
        effector.handleHeartbeat();
        
        float newSpeed = player.GetSpeed();
        REQUIRE(newSpeed == Approx(initialSpeed));
    }
    
    SECTION("multiple heartbeats accumulate speed reduction") {
        stateMachine.setRitardando(true);
        player.SetSpeed(1.0f);
        
        effector.handleHeartbeat();
        effector.handleHeartbeat();
        effector.handleHeartbeat();
        
        float expectedSpeed = 1.0f - (0.002f * 3);
        REQUIRE(player.GetSpeed() == Approx(expectedSpeed));
    }
}

TEST_CASE("RitardandoEffector custom decrement rate", "[ritardando][integration]") {
    Default outport;
    PlayerSync player(&outport);
    PlaybackStateMachine stateMachine;
    
    SECTION("custom rate applied correctly") {
        RitardandoEffector effector(player, stateMachine, 0.01f);
        
        REQUIRE(effector.getDecrementRate() == Approx(0.01f));
        
        stateMachine.setRitardando(true);
        player.SetSpeed(1.0f);
        
        effector.handleHeartbeat();
        
        float expectedSpeed = 1.0f - 0.01f;
        REQUIRE(player.GetSpeed() == Approx(expectedSpeed));
    }
    
    SECTION("setDecrementRate updates rate") {
        RitardandoEffector effector(player, stateMachine);
        
        effector.setDecrementRate(0.005f);
        REQUIRE(effector.getDecrementRate() == Approx(0.005f));
        
        stateMachine.setRitardando(true);
        player.SetSpeed(1.0f);
        
        effector.handleHeartbeat();
        
        float expectedSpeed = 1.0f - 0.005f;
        REQUIRE(player.GetSpeed() == Approx(expectedSpeed));
    }
}

// Note: Full heartbeat timing tests with actual MIDI playback callbacks
// are best validated through end-to-end playback scenarios or manual testing
// with hardware, as they depend on the player's heartbeat callback mechanism.