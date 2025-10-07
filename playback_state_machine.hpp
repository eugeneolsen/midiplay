#pragma once

namespace MidiPlay {

/**
 * @brief Centralized playback state management
 * 
 * Manages all boolean state flags for playback control.
 * Provides single source of truth for playback state with
 * clear getters and setters for each state variable.
 */
class PlaybackStateMachine {
public:
    PlaybackStateMachine() = default;
    ~PlaybackStateMachine() = default;
    
    // Disable copy/move
    PlaybackStateMachine(const PlaybackStateMachine&) = delete;
    PlaybackStateMachine& operator=(const PlaybackStateMachine&) = delete;
    
    // === State Getters ===
    bool isPlayingIntro() const { return playingIntro_; }
    bool isRitardando() const { return ritardando_; }
    bool isLastVerse() const { return lastVerse_; }
    bool isAlFine() const { return alFine_; }
    bool shouldDisplayWarnings() const { return displayWarnings_; }
    
    // === State Setters ===
    void setPlayingIntro(bool playing) { playingIntro_ = playing; }
    void setRitardando(bool active) { ritardando_ = active; }
    void setLastVerse(bool isLast) { lastVerse_ = isLast; }
    void setAlFine(bool active) { alFine_ = active; }
    void setDisplayWarnings(bool display) { displayWarnings_ = display; }
    
    // === Composite Operations ===
    /**
     * @brief Reset all state flags to initial values
     */
    void reset() {
        playingIntro_ = false;
        ritardando_ = false;
        lastVerse_ = false;
        alFine_ = false;
    }
    
private:
    bool playingIntro_{false};      // Currently playing introduction section
    bool ritardando_{false};        // Ritardando (gradual slowdown) is active
    bool lastVerse_{false};         // Currently playing the last verse
    bool alFine_{false};            // D.C. al Fine (Da Capo al Fine) mode active
    bool displayWarnings_{false};   // Whether to display warnings
};

} // namespace MidiPlay