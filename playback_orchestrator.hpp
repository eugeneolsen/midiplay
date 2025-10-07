#pragma once

#include <cxxmidi/player/player_sync.hpp>
#include <cxxmidi/event.hpp>

#include "midi_loader.hpp"
#include "playback_synchronizer.hpp"
#include "playback_state_machine.hpp"
#include "musical_director.hpp"
#include "ritardando_effector.hpp"

namespace MidiPlay {

/**
 * @brief High-level playback orchestration coordinator
 * 
 * Composes specialized components to manage complete MIDI playback:
 * - PlaybackStateMachine: State management
 * - MusicalDirector: Musical direction interpretation
 * - RitardandoEffector: Tempo slowdown effects
 * 
 * The orchestrator sets up player callbacks that delegate to these
 * components and manages the high-level playback flow (intro → verses).
 */
class PlaybackOrchestrator {
public:
    /**
     * @brief Constructor with dependency injection
     * @param player Reference to cxxmidi PlayerSync instance (already contains outport)
     * @param synchronizer Reference to PlaybackSynchronizer for synchronization
     * @param midiLoader Reference to MidiLoader for file metadata
     */
    PlaybackOrchestrator(cxxmidi::player::PlayerSync& player,
                         PlaybackSynchronizer& synchronizer,
                         const MidiLoader& midiLoader);
    
    /**
     * @brief Destructor
     */
    ~PlaybackOrchestrator() = default;
    
    // Disable copy/move
    PlaybackOrchestrator(const PlaybackOrchestrator&) = delete;
    PlaybackOrchestrator& operator=(const PlaybackOrchestrator&) = delete;
    
    /**
     * @brief Initialize player with callbacks and base tempo
     * Gets speed from midiLoader (which obtained it from Options)
     */
    void initialize();
    
    /**
     * @brief Display playback information to console
     * Shows title, key, verses, and BPM with speed applied
     */
    void displayPlaybackInfo() const;
    
    /**
     * @brief Execute complete playback sequence (intro + verses)
     * Main orchestration method that handles:
     * - Optional introduction playback
     * - Verse iteration with pausing
     * - State management across playback sections
     */
    void executePlayback();
    
    /**
     * @brief Set whether warnings should be displayed
     */
    void setDisplayWarnings(bool display) { 
        stateMachine_.setDisplayWarnings(display); 
    }

private:
    // === Dependency References ===
    cxxmidi::player::PlayerSync& player_;  // Contains outport internally
    PlaybackSynchronizer& synchronizer_;
    const MidiLoader& midiLoader_;
    
    // === Owned Components ===
    PlaybackStateMachine stateMachine_;
    MusicalDirector musicalDirector_;
    RitardandoEffector ritardandoEffector_;
    
    // === Timing State ===
    float baseSpeed_{1.0f};       // Base tempo multiplier
    float baseTempo_{1.0f};       // Original player tempo
    
    // === Callback Handlers (delegate to components) ===
    /**
     * @brief Heartbeat callback - delegates to ritardandoEffector_
     */
    void heartbeatCallback();
    
    /**
     * @brief Event callback - delegates to musicalDirector_
     * @param event MIDI event being processed
     * @return true to send event to device, false to suppress
     */
    bool eventCallback(cxxmidi::Event& event);
    
    /**
     * @brief Finished callback - signals playback completion
     */
    void finishedCallback();
    
    // === Playback Flow Methods ===
    /**
     * @brief Play introduction section with marker-based jumping
     */
    void playIntroduction();
    
    /**
     * @brief Play all verses with inter-verse pausing
     */
    void playVerses();
    
    /**
     * @brief Set player speed (combines base tempo and speed multiplier)
     */
    void setPlayerSpeed(float speedMultiplier);
    
    // === Constants ===
    static constexpr int VERSE_DISPLAY_OFFSET = 1;
};

} // namespace MidiPlay