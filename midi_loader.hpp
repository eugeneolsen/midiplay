#pragma once

#include <cxxmidi/file.hpp>
#include <cxxmidi/event.hpp>
#include <cxxmidi/message.hpp>
#include <string>
#include <vector>
#include <cstdint>
#include <sys/stat.h>

#include "ticks.hpp"
#include "custommessage.hpp"
#include "constants.hpp"

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
 * MidiLoader - Handles MIDI file loading, parsing, and meta-event processing
 * 
 * This class encapsulates all MIDI file loading logic previously scattered
 * throughout play.cpp, providing a clean interface for MIDI file operations.
 * 
 * Responsibilities:
 * - MIDI file loading and validation
 * - Meta-event processing (tempo, key signature, time signature)
 * - Custom event handling (verses, introduction segments)
 * - File path resolution and existence checking
 * - Post-load data extraction and validation
 */
class MidiLoader {
public:
    /**
     * Constructor
     */
    MidiLoader();
    
    /**
     * Destructor
     */
    ~MidiLoader() = default;
    
    // Disable copy constructor and assignment operator
    MidiLoader(const MidiLoader&) = delete;
    MidiLoader& operator=(const MidiLoader&) = delete;
    
    /**
     * Main loading interface - loads and parses MIDI file
     * @param path Full path to MIDI file
     * @param options Command line options affecting loading behavior
     * @return true if loading successful, false otherwise
     */
    bool loadFile(const std::string& path, const Options& options);
    
    // Getters for extracted MIDI file data
    cxxmidi::File& getFile() { return midiFile_; }
    const cxxmidi::File& getFile() const { return midiFile_; }
    
    const std::string& getTitle() const { return title_; }
    const std::string& getKeySignature() const { return keySignature_; }
    const TimeSignature& getTimeSignature() const { return timeSignature_; }
    const std::vector<IntroductionSegment>& getIntroSegments() const { return introSegments_; }
    
    // Calculated values from MIDI processing
    int getVerses() const { return verses_; }
    int getUSecPerQuarter() const { return uSecPerQuarter_; }
    int getUSecPerTick() const { return uSecPerTick_; }
    int getFileTempo() const { return fileTempo_; }
    int getBpm() const { return bpm_; }
    MidiTicks getPauseTicks() const { return pauseTicks_; }
    float getSpeed() const { return speed_; }
    
    // State flags
    bool shouldPlayIntro() const { return playIntro_; }
    bool hasPotentialStuckNote() const { return potentialStuckNote_; }
    bool isFirstTempo() const { return firstTempo_; }
    bool isVerbose() const { return isVerbose_; }
    
    /**
     * Static utility method for file existence checking
     * @param path File path to check
     * @return true if file exists, false otherwise
     */
    static bool fileExists(const std::string& path);

private:
    // Internal loading methods
    void initializeLoadCallback(const Options& options);
    void scanTrackZeroMetaEvents();
    void finalizeLoading();
    void resetState();
    
    // Event processing helpers
    void processTempoEvent(const cxxmidi::Event& event, const Options& options);
    void processKeySignatureEvent(const cxxmidi::Event& event);
    void processTimeSignatureEvent(const cxxmidi::Event& event);
    void processCustomMetaEvents(const cxxmidi::Event& event, const Options& options);
    void processIntroductionMarkers(const cxxmidi::Event& event);
    void processTrackNameEvent(const cxxmidi::Event& event);
    
    // Load callback implementation (extracted from play.cpp)
    bool loadCallback(cxxmidi::Event& event, const Options& options);
    
    // Member variables (previously global variables in play.cpp)
    cxxmidi::File midiFile_;
    std::string title_;
    std::string keySignature_;
    TimeSignature timeSignature_;
    std::vector<IntroductionSegment> introSegments_;
    
    int verses_;
    int uSecPerQuarter_;
    int uSecPerTick_;
    int fileTempo_;  // Tempo from file, if any
    int bpm_;
    MidiTicks pauseTicks_;
    float speed_;
    
    bool playIntro_;
    bool potentialStuckNote_;
    bool firstTempo_;
    bool isVerbose_; // For debug output
    
    // Load callback state variables
    int currentTrack_;
    int totalTrackTicks_;
    int lastNoteOn_;
    int lastNoteOff_;
    std::vector<IntroductionSegment>::iterator currentIntroIterator_;
    
    // Constants (moved from global scope in play.cpp)
    static const char* const keys_[18];  // 18 key signatures
    
    static constexpr int MAJOR_KEY_OFFSET = 6;
    static constexpr int MINOR_KEY_OFFSET = 9;
    static constexpr uint8_t DEPRECATED_META_EVENT_VERSES = 0x10;
    static constexpr uint8_t DEPRECATED_META_EVENT_PAUSE = 0x11;
    
    // Introduction and musical direction markers
    static constexpr const char* INTRO_BEGIN = "[";
    static constexpr const char* INTRO_END = "]";
    static constexpr const char* RITARDANDO_INDICATOR = R"(\)";
    static constexpr const char* D_C_AL_FINE = "D.C. al Fine";
    static constexpr const char* FINE_INDICATOR = "Fine";
};

} // namespace MidiPlay