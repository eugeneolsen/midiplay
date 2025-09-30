#pragma once

#include <cxxmidi/player/player_sync.hpp>
#include <cxxmidi/event.hpp>
#include <cxxmidi/message.hpp>
#include <semaphore.h>
#include <vector>

#include "midi_loader.hpp"

namespace MidiPlay {

/**
 * @brief Manages complete MIDI playback orchestration
 * 
 * This class encapsulates all playback control logic including:
 * - Player callback setup and management
 * - Introduction segment playback with marker-based jumping
 * - Multi-verse playback with inter-verse pausing
 * - Ritardando (gradual slowdown) control
 * - Musical direction handling (D.C. al Fine, Fine markers)
 * - Playback state management
 */
class PlaybackEngine {
public:
    /**
     * @brief Constructor with dependency injection
     * @param player Reference to cxxmidi PlayerSync instance (already contains outport)
     * @param semaphore Reference to semaphore for synchronization
     * @param midiLoader Reference to MidiLoader for file metadata
     */
    PlaybackEngine(cxxmidi::player::PlayerSync& player,
                   sem_t& semaphore,
                   const MidiLoader& midiLoader);
    
    /**
     * @brief Destructor
     */
    ~PlaybackEngine() = default;
    
    // Disable copy/move
    PlaybackEngine(const PlaybackEngine&) = delete;
    PlaybackEngine& operator=(const PlaybackEngine&) = delete;
    
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
    void setDisplayWarnings(bool display) { displayWarnings_ = display; }

private:
    // === Dependency References ===
    cxxmidi::player::PlayerSync& player_;  // Contains outport internally
    sem_t& semaphore_;
    const MidiLoader& midiLoader_;
    
    // === Playback State Variables ===
    bool playingIntro_;
    bool ritardando_;
    bool lastVerse_;
    bool alFine_;           // "Da Capo al Fine" flag
    bool displayWarnings_;
    float baseSpeed_;       // Base tempo multiplier
    float baseTempo_;       // Original player tempo
    
    // Introduction segment iterator
    std::vector<IntroductionSegment>::const_iterator currentIntroSegment_;
    
    // === Callback Implementation Methods ===
    /**
     * @brief Heartbeat callback - handles ritardando logic
     * Called periodically during playback to apply gradual speed reduction
     */
    void heartbeatCallback();
    
    /**
     * @brief Event callback - handles meta-events and musical directions
     * @param event MIDI event being processed
     * @return true to send event to device, false to suppress
     */
    bool eventCallback(cxxmidi::Event& event);
    
    /**
     * @brief Finished callback - signals playback completion
     */
    void finishedCallback();
    
    // === Playback Section Methods ===
    /**
     * @brief Play introduction section with marker-based jumping
     */
    void playIntroduction();
    
    /**
     * @brief Play all verses with inter-verse pausing
     */
    void playVerses();
    
    /**
     * @brief Reset playback state for new section
     */
    void resetPlaybackState();
    
    /**
     * @brief Set player speed (combines base tempo and speed multiplier)
     */
    void setPlayerSpeed(float speedMultiplier);
    
    // === Event Processing Helpers ===
    /**
     * @brief Process introduction marker events (jumping logic)
     * @param message MIDI meta message
     */
    void processIntroMarker(const cxxmidi::Message& message);
    
    /**
     * @brief Process ritardando marker
     * @param message MIDI meta message
     */
    void processRitardandoMarker(const cxxmidi::Message& message);
    
    /**
     * @brief Process D.C. al Fine marker
     * @param message MIDI meta message
     * @return false to suppress event output
     */
    bool processDCAlFineMarker(const cxxmidi::Message& message);
    
    /**
     * @brief Process Fine marker (when in al Fine mode)
     * @param message MIDI meta message
     * @return false to suppress event output
     */
    bool processFineMarker(const cxxmidi::Message& message);
    
    // === Constants (module-specific, not shared) ===
    static constexpr int64_t HEARTBEAT_CHECK_INTERVAL = 100000;
    static constexpr float RITARDANDO_DECREMENT = 0.002f;
    static constexpr int VERSE_DISPLAY_OFFSET = 1;
    
    // Musical direction markers (from MidiLoader but used here)
    static constexpr const char* INTRO_END = "]";
    static constexpr const char* RITARDANDO_INDICATOR = R"(\)";
    static constexpr const char* D_C_AL_FINE = "D.C. al Fine";
    static constexpr const char* FINE_INDICATOR = "Fine";
};

} // namespace MidiPlay