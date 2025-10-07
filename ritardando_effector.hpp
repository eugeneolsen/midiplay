#pragma once

#include <cxxmidi/player/player_sync.hpp>
#include "playback_state_machine.hpp"

namespace MidiPlay {

/**
 * @brief Handles gradual tempo slowdown (ritardando effect)
 * 
 * Applies progressive speed reduction during playback when ritardando
 * is active. The effect is triggered by the state machine and applied
 * via periodic heartbeat callbacks.
 */
class RitardandoEffector {
public:
    /**
     * @brief Constructor with dependency injection
     * @param player Reference to player for speed control
     * @param stateMachine Reference to state machine for ritardando flag
     * @param decrementRate Speed decrement per heartbeat interval (default: RITARDANDO_DECREMENT)
     */
    RitardandoEffector(cxxmidi::player::PlayerSync& player,
                       PlaybackStateMachine& stateMachine,
                       float decrementRate = RITARDANDO_DECREMENT);
    
    ~RitardandoEffector() = default;
    
    // Disable copy/move
    RitardandoEffector(const RitardandoEffector&) = delete;
    RitardandoEffector& operator=(const RitardandoEffector&) = delete;
    
    /**
     * @brief Handle heartbeat callback from player
     * 
     * Called periodically during playback. When ritardando is active,
     * progressively reduces playback speed at regular intervals.
     */
    void handleHeartbeat();
    
    /**
     * @brief Set the speed decrement rate
     * @param rate Speed reduction per heartbeat interval
     */
    void setDecrementRate(float rate) { decrementRate_ = rate; }
    
    /**
     * @brief Get the current decrement rate
     * @return Current speed reduction per heartbeat interval
     */
    float getDecrementRate() const { return decrementRate_; }
    
private:
    cxxmidi::player::PlayerSync& player_;
    PlaybackStateMachine& stateMachine_;
    float decrementRate_;
    
    // Heartbeat interval for checking and applying ritardando
    static constexpr int64_t HEARTBEAT_CHECK_INTERVAL = 100000;
    
    // Default speed decrement per interval
    static constexpr float RITARDANDO_DECREMENT = 0.002f;
};

} // namespace MidiPlay