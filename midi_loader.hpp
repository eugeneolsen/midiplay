#pragma once

#include <cxxmidi/file.hpp>
#include <cxxmidi/event.hpp>
#include <cxxmidi/message.hpp>
#include <string>
#include <vector>
#include <cstdint>
#include <sys/stat.h>
#include <memory>

#include "ticks.hpp"
#include "custommessage.hpp"
#include "constants.hpp"
#include "event_preprocessor.hpp"

// Forward declaration
class Options;

namespace MidiPlay {

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
     * Destructor - defined in .cpp to support pimpl pattern
     */
    ~MidiLoader();
    
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
    
    // Forwarding getters to EventPreProcessor
    const std::string& getTitle() const;
    const std::string& getKeySignature() const;
    const MidiPlay::TimeSignature& getTimeSignature() const;
    const std::vector<MidiPlay::IntroductionSegment>& getIntroSegments() const;
    
    // Calculated values from MIDI processing
    int getVerses() const;
    int getUSecPerQuarter() const;
    int getUSecPerTick() const { return uSecPerTick_; }
    int getFileTempo() const;
    int getBpm() const;
    MidiTicks getPauseTicks() const;
    float getSpeed() const { return speed_; }
    
    // State flags
    bool shouldPlayIntro() const { return playIntro_; }
    bool hasPotentialStuckNote() const;
    bool isFirstTempo() const;
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
    
    // Simplified load callback that delegates to EventPreProcessor
    bool loadCallback(cxxmidi::Event& event, const Options& options);
    
    // Member variables
    cxxmidi::File midiFile_;
    std::unique_ptr<EventPreProcessor> eventProcessor_;
    
    // Calculated timing values
    int uSecPerTick_;
    float speed_;
    
    // State flags
    bool playIntro_;
    bool isVerbose_; // For debug output
};

} // namespace MidiPlay