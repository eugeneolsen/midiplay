#pragma once

#include <cxxmidi/event.hpp>
#include <cxxmidi/message.hpp>
#include <string>
#include <vector>
#include <cstdint>

#include "ticks.hpp"
#include "custommessage.hpp"
#include "constants.hpp"
#include "midi_markers.hpp"

// Forward declaration
class Options;

namespace MidiPlay {

/**
 * Time signature information extracted from MIDI file
 */
struct TimeSignature {
    uint8_t beatsPerMeasure;
    uint8_t denominator;
    uint8_t clocksPerClick;      // of metronome
    uint8_t n32ndNotesPerQuaver;
    
    // Default constructor
    TimeSignature() : beatsPerMeasure(0), denominator(0), clocksPerClick(0), n32ndNotesPerQuaver(0) {}
};

/**
 * Introduction segment boundaries found in MIDI file
 */
struct IntroductionSegment {
    uint32_t start;
    uint32_t end;
    
    // Default constructor
    IntroductionSegment() : start(0), end(0) {}
    IntroductionSegment(uint32_t s, uint32_t e) : start(s), end(e) {}
};

/**
 * EventPreProcessor - Handles MIDI event filtering and metadata extraction
 * 
 * This class contains the core logic from the original MidiLoader::loadCallback.
 * It is responsible for both filtering events (deciding if they should be loaded)
 * and extracting metadata (Title, Key Signature, Verses, etc.).
 * 
 * IMPORTANT: Custom meta events are processed for data but return false
 * to prevent them from being loaded into the MIDI file (Fixes GitHub Issue #21).
 */
class EventPreProcessor {
public:
    /**
     * Constructor
     */
    EventPreProcessor();
    
    /**
     * Destructor
     */
    ~EventPreProcessor() = default;
    
    // Disable copy/move
    EventPreProcessor(const EventPreProcessor&) = delete;
    EventPreProcessor& operator=(const EventPreProcessor&) = delete;
    
    /**
     * Main event processing method
     * @param event MIDI event to process
     * @param options Command line options affecting processing
     * @return true if event should be loaded, false if it should be discarded
     */
    bool processEvent(cxxmidi::Event& event, const Options& options);
    
    // Getters for extracted data
    const std::string& getTitle() const { return title_; }
    const std::string& getKeySignature() const { return keySignature_; }
    const TimeSignature& getTimeSignature() const { return timeSignature_; }
    const std::vector<IntroductionSegment>& getIntroSegments() const { return introSegments_; }
    
    // Calculated values
    int getVerses() const { return verses_; }
    int getUSecPerQuarter() const { return uSecPerQuarter_; }
    int getFileTempo() const { return fileTempo_; }
    int getBpm() const { return bpm_; }
    MidiTicks getPauseTicks() const { return pauseTicks_; }
    
    // State flags
    bool hasPotentialStuckNote() const { return potentialStuckNote_; }
    bool isFirstTempo() const { return firstTempo_; }
    
    /**
     * Reset processor state for new file
     */
    void reset();
    
    /**
     * Set verses from command-line options (called after loading)
     * Only sets if verses not already extracted from MIDI file
     * @param optionVerses Verse count from command-line options
     */
    void setVersesFromOptions(int optionVerses);

private:
    // Event processing helpers (moved from MidiLoader)
    void processTempoEvent(const cxxmidi::Event& event, const Options& options);
    void processKeySignatureEvent(const cxxmidi::Event& event);
    void processTimeSignatureEvent(const cxxmidi::Event& event);
    
    /**
     * Process custom meta events for data extraction
     * @param event MIDI event to process
     * @param options Command line options
     * @return true if no custom event found (continue processing), 
     *         false if custom event found and should be discarded
     */
    bool processCustomMetaEvents(const cxxmidi::Event& event, const Options& options);
    
    void processIntroductionMarkers(const cxxmidi::Event& event);
    void processTrackNameEvent(const cxxmidi::Event& event);
    
    // Event filtering logic
    bool shouldLoadControlChangeEvent(const cxxmidi::Event& event);
    
    // Member variables (moved from MidiLoader)
    std::string title_;
    std::string keySignature_;
    TimeSignature timeSignature_;
    std::vector<IntroductionSegment> introSegments_;
    
    int verses_;
    int uSecPerQuarter_;
    int fileTempo_;
    int bpm_;
    MidiTicks pauseTicks_;
    
    bool potentialStuckNote_;
    bool firstTempo_;
    
    // Load callback state variables
    int currentTrack_;
    int totalTrackTicks_;
    int lastNoteOn_;
    int lastNoteOff_;
    
    // Constants (moved from MidiLoader)
    static const char* const keys_[18];
    
    static constexpr int MAJOR_KEY_OFFSET = 6;
    static constexpr int MINOR_KEY_OFFSET = 9;
    static constexpr uint8_t DEPRECATED_META_EVENT_VERSES = 0x10;
    static constexpr uint8_t DEPRECATED_META_EVENT_PAUSE = 0x11;
    
    // Note: Musical direction markers now in midi_markers.hpp
};

} // namespace MidiPlay