#pragma once

#include <cxxmidi/player/player_sync.hpp>
#include <cxxmidi/event.hpp>
#include <cxxmidi/message.hpp>
#include <vector>

#include "midi_loader.hpp"
#include "playback_state_machine.hpp"
#include "midi_markers.hpp"

namespace MidiPlay {

/**
 * @brief Interprets MIDI meta-events and executes musical directions
 * 
 * Processes musical markers in MIDI files to control playback behavior:
 * - Introduction markers for section jumping
 * - Ritardando markers for gradual slowdown
 * - D.C. al Fine (Da Capo al Fine) for repeat-to-fine
 * - Fine markers for early termination
 */
class MusicalDirector {
public:
    /**
     * @brief Constructor with dependency injection
     * @param player Reference to player for playback control
     * @param stateMachine Reference to state machine for state updates
     * @param midiLoader Reference to MIDI loader for file metadata
     */
    MusicalDirector(cxxmidi::player::PlayerSync& player,
                    PlaybackStateMachine& stateMachine,
                    const MidiLoader& midiLoader);
    
    ~MusicalDirector() = default;
    
    // Disable copy/move
    MusicalDirector(const MusicalDirector&) = delete;
    MusicalDirector& operator=(const MusicalDirector&) = delete;
    
    /**
     * @brief Handle MIDI event callback from player
     * @param event MIDI event being processed
     * @return true to send event to output device, false to suppress
     */
    bool handleEvent(cxxmidi::Event& event);
    
    /**
     * @brief Initialize intro segment iterator
     * 
     * Must be called before playback if introduction will be played.
     * Sets the iterator to the beginning of intro segments.
     */
    void initializeIntroSegments();
    
private:
    // === Dependencies ===
    cxxmidi::player::PlayerSync& player_;
    PlaybackStateMachine& stateMachine_;
    const MidiLoader& midiLoader_;
    
    // === State ===
    std::vector<IntroductionSegment>::const_iterator currentIntroSegment_;
    
    // === Event Processing Helpers ===
    /**
     * @brief Process introduction end marker (jumping logic)
     * @param message MIDI meta message containing marker
     */
    void processIntroMarker(const cxxmidi::Message& message);
    
    /**
     * @brief Process ritardando marker
     * @param message MIDI meta message containing marker
     */
    void processRitardandoMarker(const cxxmidi::Message& message);
    
    /**
     * @brief Process D.C. al Fine marker
     * @param message MIDI meta message containing marker
     * @return false to suppress event output
     */
    bool processDCAlFineMarker(const cxxmidi::Message& message);
    
    /**
     * @brief Process Fine marker (when in al Fine mode)
     * @param message MIDI meta message containing marker
     * @return false to suppress event output
     */
    bool processFineMarker(const cxxmidi::Message& message);
    
    // Note: Musical direction markers now in midi_markers.hpp
};

} // namespace MidiPlay