#include "midi_loader.hpp"
#include "options.hpp"
#include "i18n.hpp"
#include <iostream>
#include <cmath>

#include "midi_constants.hpp"
#include "event_preprocessor.hpp"

using cxxmidi::Event;
using cxxmidi::Message;

namespace MidiPlay {

// Constructor
MidiLoader::MidiLoader() {
    eventProcessor_ = std::make_unique<EventPreProcessor>();
    resetState();
}

// Destructor - defined here for pimpl pattern
MidiLoader::~MidiLoader() = default;

// Reset all state variables to initial values
void MidiLoader::resetState() {
    eventProcessor_->reset();
    
    uSecPerTick_ = 0;
    speed_ = 0.0f;
    
    playIntro_ = false;
    isVerbose_ = false;
}

// Static utility method for file existence checking (extracted from play.cpp lines 91-94)
bool MidiLoader::fileExists(const std::string& path) {
    struct stat buffer;
    return (stat(path.c_str(), &buffer) == 0);
}

// Main loading interface
bool MidiLoader::loadFile(const std::string& path, const Options& options) {
    // Reset state for new file
    resetState();
    
    // Store speed from options
    speed_ = options.getSpeed();
    
    // Store playIntro flag from command-line options
    playIntro_ = options.isPlayIntro();
    
    // Check for tempo override from command line
    int tempoOverride = options.get_uSecPerBeat();
    
    // Check if file exists (extracted from play.cpp lines 369-378)
    if (!fileExists(path)) {
        std::cout << _("Hymn ") << options.getFileName() << _(" was not found");
        
        if (options.isStaging()) {
            std::cout << _(" in the staging folder.\n") << std::endl;
        } else {
            std::cout << _(".\n") << std::endl;
        }
        
        return false;
    }

    isVerbose_ = options.isVerbose();
    
    // Initialize load callback before loading
    initializeLoadCallback(options);
    
    try {
        // Load the MIDI file (extracted from play.cpp line 381)
        midiFile_.Load(path.c_str());
        
        // Calculate timing values (extracted from play.cpp lines 383-390)
        uint16_t ppq = midiFile_.TimeDivision();
        uSecPerTick_ = eventProcessor_->getUSecPerQuarter() / ppq;
        
        if (eventProcessor_->getPauseTicks().isNull()) {
            // Set default pause in EventPreProcessor
            // This will need to be handled by EventPreProcessor
        }
        
        // Scan Track 0 for meta events (extracted from play.cpp lines 397-413)
        scanTrackZeroMetaEvents();
        
        // Finalize loading process (extracted from play.cpp lines 415-423)
        finalizeLoading();
        
        return true;
    }
    catch (const std::exception& e) {
        std::cerr << _("Error loading MIDI file: ") << e.what() << std::endl;
        return false;
    }
}

// Initialize the load callback (extracted and refactored from play.cpp lines 157-366)
void MidiLoader::initializeLoadCallback(const Options& options) {
    midiFile_.SetCallbackLoad(
        [this, &options](Event& event) -> bool {
            return this->loadCallback(event, options);
        }
    );
}

// Simplified load callback that delegates to EventPreProcessor
bool MidiLoader::loadCallback(Event& event, const Options& options) {
    return eventProcessor_->processEvent(event, options);
}

// Scan Track 0 for meta events (extracted from play.cpp lines 397-413)
void MidiLoader::scanTrackZeroMetaEvents() {
    // This method may be simplified or removed since EventPreProcessor handles this
    // For now, keeping the structure but delegating to EventPreProcessor if needed
}

// Finalize loading process (extracted from play.cpp lines 415-423)
void MidiLoader::finalizeLoading() {
    // Set default verses if not specified - EventPreProcessor handles this
    // If there are no intro markers in file, can't play intro
    // regardless of command-line option
    if (eventProcessor_->getIntroSegments().size() == 0) {
        playIntro_ = false;      // Override command line option if no markers
    }
    // Otherwise playIntro_ retains value from options (set in loadFile)
}

// Forwarding getter implementations
const std::string& MidiLoader::getTitle() const {
    return eventProcessor_->getTitle();
}

const std::string& MidiLoader::getKeySignature() const {
    return eventProcessor_->getKeySignature();
}

const TimeSignature& MidiLoader::getTimeSignature() const {
    return eventProcessor_->getTimeSignature();
}

const std::vector<IntroductionSegment>& MidiLoader::getIntroSegments() const {
    return eventProcessor_->getIntroSegments();
}

int MidiLoader::getVerses() const {
    return eventProcessor_->getVerses();
}

int MidiLoader::getUSecPerQuarter() const {
    return eventProcessor_->getUSecPerQuarter();
}

int MidiLoader::getFileTempo() const {
    return eventProcessor_->getFileTempo();
}

int MidiLoader::getBpm() const {
    return eventProcessor_->getBpm();
}

MidiTicks MidiLoader::getPauseTicks() const {
    return eventProcessor_->getPauseTicks();
}

bool MidiLoader::hasPotentialStuckNote() const {
    return eventProcessor_->hasPotentialStuckNote();
}

bool MidiLoader::isFirstTempo() const {
    return eventProcessor_->isFirstTempo();
}

} // namespace MidiPlay