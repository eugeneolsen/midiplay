#include "midi_loader.hpp"
#include "options.hpp"
#include <iostream>
#include <cmath>
#include <libintl.h>

#define _(String) gettext(String)

#include "midi_constants.hpp"

using cxxmidi::Event;
using cxxmidi::Message;
using cxxmidi::Track;

namespace MidiPlay {

// Define the static constexpr array that was declared in the header
const char* const MidiLoader::keys_[] = {
    "Gb", "Db", "Ab", "Eb", "Bb", "F", "C",
    "G", "D", "A", "E", "B", "F#", "C#", "G#", "D#", "A#", "e#"
};

// Constructor
MidiLoader::MidiLoader() {
    resetState();
}

// Reset all state variables to initial values
void MidiLoader::resetState() {
    title_.clear();
    keySignature_.clear();
    timeSignature_ = TimeSignature();
    introSegments_.clear();
    
    verses_ = 0;
    uSecPerQuarter_ = 0;
    uSecPerTick_ = 0;
    pauseTicks_ = MidiTicks();
    
    playIntro_ = false;
    potentialStuckNote_ = false;
    firstTempo_ = true;
    
    // Load callback state variables
    currentTrack_ = 0;
    totalTrackTicks_ = 0;
    lastNoteOn_ = 0;
    lastNoteOff_ = 0;
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
    
    // Store verses from command-line options (if specified)
    verses_ = options.getVerses();
    
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
    
    // Initialize load callback before loading
    initializeLoadCallback(options);
    
    try {
        // Load the MIDI file (extracted from play.cpp line 381)
        midiFile_.Load(path.c_str());
        
        // Apply tempo override if specified via --tempo command line option
        if (tempoOverride > 0) {
            uSecPerQuarter_ = tempoOverride;
        }
        
        // Calculate timing values (extracted from play.cpp lines 383-390)
        std::vector<Track>& tracks = (std::vector<Track>&)midiFile_;
        uint16_t ppq = midiFile_.TimeDivision();
        uSecPerTick_ = uSecPerQuarter_ / ppq;
        
        if (pauseTicks_.isNull()) {
            pauseTicks_ = ppq;  // Default pause = 1 quarter note duration
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

// Main load callback implementation (extracted from play.cpp lines 157-366)
bool MidiLoader::loadCallback(Event& event, const Options& options) {
    Message message = event;
    uint8_t status = message[0];
    
    totalTrackTicks_ += event.Dt();
    
    if (message.IsSysex()) {
        return false;  // Throw away SysEx events. Player doesn't handle them.
    }
    
    // Throw away control change messages with specific exceptions:
    // NRPN (Non-Registered Parameter Number MSB & LSB)
    // Data Entry MSB & LSB
    // These exceptions are used for organ stop settings.
    if (message.IsControlChange()) {
        if (!message.IsControlChange(Message::ControlType::NonRegisteredParameterNumberLsb) 
            && !message.IsControlChange(Message::ControlType::NonRegisteredParameterNumberMsb) 
            && !message.IsControlChange(Message::ControlType::DataEntryMsb) 
            && !message.IsControlChange(Message::ControlType::DataEntryLsb)) {
            return false;   // Throw away most control change messages. Use organ controls instead.
        }
    }
    
    if (message.IsMeta()) {
        uint8_t type = message[1];
        
        if (message.IsMeta(Message::MetaType::Lyrics)) {
            return false;   // Throw away lyrics. Player doesn't handle them.
        }
        
        if (0 == totalTrackTicks_) {  // Processing for Time Zero Meta events
            processTimeSignatureEvent(event);
            processTempoEvent(event, options);
            processKeySignatureEvent(event);
            processCustomMetaEvents(event, options);
        }
    }   // Time 0
    
    if (currentTrack_ == 0) {   // Track 0-only messages
        processIntroductionMarkers(event);
    }   // Track 0-only message handling
    
    if (event.IsVoiceCategory(Message::Type::NoteOn) && event[2] != 0) {
        lastNoteOn_ = totalTrackTicks_;
    }
    
    if ((event.IsVoiceCategory(Message::Type::NoteOn) && event[2] == 0) || 
        event.IsVoiceCategory(Message::Type::NoteOff)) {
        lastNoteOff_ = totalTrackTicks_;
    }
    
    if (event.IsMeta(Message::MetaType::EndOfTrack)) {
        currentTrack_++;
        
        if (introSegments_.size()) {
            auto itintro = introSegments_.end();
            itintro--;
            uint32_t endIntro = itintro->end;
            
            if (totalTrackTicks_ == endIntro) {
                if (lastNoteOff_ >= endIntro) {
                    potentialStuckNote_ = true;
                }
            }
        }
        
        totalTrackTicks_ = 0;    // Reset ticks for next track
    }
    
    return true;
}

// Process time signature events
void MidiLoader::processTimeSignatureEvent(const Event& event) {
    Message message = event;
    
    if (message.IsMeta(Message::MetaType::TimeSignature) && message.size() == 6) {
        timeSignature_.beatsPerMeasure = message[2];
        timeSignature_.denominator = message[3];
        timeSignature_.clocksPerClick = message[4];
        timeSignature_.n32ndNotesPerQuaver = message[5];
    }
}

// Process tempo events (extracted from play.cpp lines 208-236)
void MidiLoader::processTempoEvent(const Event& event, const Options& options) {
    Message message = event;
    
    if (message.IsMeta(Message::MetaType::Tempo)) {
        // Get tempo from file - manual extraction of 24-bit value
        uSecPerQuarter_ = (static_cast<uint32_t>(event[2]) << 16) |
                          (static_cast<uint32_t>(event[3]) << 8) |
                          static_cast<uint32_t>(event[4]);
        
        if (firstTempo_) {
            bpm_ = options.getBpm();
            int uSecPerBeat = options.get_uSecPerBeat();
            float speed = options.getSpeed();

            if (uSecPerQuarter_ > 0) {
                int qpm = MidiPlay::MICROSECONDS_PER_MINUTE / uSecPerQuarter_;  // Quarter notes per minute
                fileTempo_ = qpm * (std::pow(2.0, timeSignature_.denominator) / MidiPlay::QUARTER_NOTE_DENOMINATOR);
            }
            else {
                uSecPerQuarter_ = Midi::DEFAULT_TEMPO_USEC_PER_QUARTER;  // Default to 120 bpm if no tempo specified
                fileTempo_ = Midi::DEFAULT_TEMPO_BPM;
            }
            
            if (bpm_ > 0) {
                int qpm = MidiPlay::MICROSECONDS_PER_MINUTE / uSecPerBeat;  // Quarter notes per minute
                bpm_ = qpm * (std::pow(2.0, timeSignature_.denominator) / MidiPlay::QUARTER_NOTE_DENOMINATOR);
                
                if (uSecPerBeat) {
                    // Note: speed calculation would need to be handled by caller
                }
            }
            else {
                bpm_ = fileTempo_;
            }
           
            firstTempo_ = false;
        }
    }
}

// Process key signature events (extracted from play.cpp lines 238-253)
void MidiLoader::processKeySignatureEvent(const Event& event) {
    Message message = event;
    
    if (message.IsMeta(Message::MetaType::KeySignature)) {
        int sf = static_cast<int8_t>(static_cast<uint8_t>(message[2]));
        int mi = (uint8_t)message[3];
        
        if (mi == 0) {
            keySignature_ = keys_[sf + MAJOR_KEY_OFFSET];
        }
        else {
            keySignature_ = keys_[sf + MINOR_KEY_OFFSET];
            keySignature_ += _(" minor");
        }
    }
}

// Process custom meta events (extracted from play.cpp lines 255-314)
void MidiLoader::processCustomMetaEvents(const Event& event, const Options& options) {
    Message message = event;
    uint8_t type = message[1];
    
    // Deprecated non-standard Meta events for verses and pause between verses
    // These are superseded by the Sequencer-Specific Meta event below.
    if (DEPRECATED_META_EVENT_VERSES == type) {   // Non-standard "number of verses" Meta event type for this sequencer
        // Extract the number of verses, if the event is present in the file, and then throw the event away.
        if (verses_ == 0) {    // If verses not specified in command line
            char c = static_cast<char>(message[2]);
            
            if (std::isdigit(c)) {
                std::string sVerse{c};
                verses_ = std::stoi(sVerse);
            }
        }

        if (options.isVerbose() || options.isDisplayWarnings()) {
            std::cout << _("Warning: Deprecated Meta event for number of verses found in MIDI file. "
                      "Please use the Sequencer-Specific Meta event instead.") << std::endl;
        }
        // Don't load the non-standard event - handled by returning false in main callback
    }
    
    if (DEPRECATED_META_EVENT_PAUSE == type) {   // Non-standard "pause between verses" Meta event type for this sequencer
        pauseTicks_ = (static_cast<uint16_t>(message[2]) << 8) | message[3];

        if (options.isVerbose() || options.isDisplayWarnings()) {
            std::cout << _("Warning: Deprecated Meta event for pause between verses found in MIDI file. "
                      "Please use the Sequencer-Specific Meta event instead.") << std::endl;
        }
        // Don't load the non-standard event - handled by returning false in main callback
    }
    
    if (message.IsMeta(Message::MetaType::SequencerSpecific)) {     // Sequencer-Specific Meta Event
        int index = 2;
        if (message[index] != midiplay::CustomMessage::Type::Private) {
            int len = message[index++];     // This does not conform to the MIDI standard
        }
        
        if (message[index++] == midiplay::CustomMessage::Type::Private) {   // Prototyping, test, private use and experimentation
            if (message[index] == midiplay::CustomMessage::PrivateType::NumberOfVerses) {   // Number of verses
                // Extract the number of verses, if the event is present in the file, and then throw the event away.
                if (verses_ == 0) {    // If verses not specified in command line
                    char c = static_cast<char>(message[++index]);
                    
                    if (std::isdigit(c)) {
                        std::string sVerse{c};
                        verses_ = std::stoi(sVerse);
                    }
                }
                // Don't load the non-standard event - handled by returning false in main callback
            }
            
            if (message[index] == midiplay::CustomMessage::PrivateType::PauseBetweenVerses) {  // Pause between verses
                pauseTicks_ = (static_cast<uint16_t>(message[++index]) << 8) | message[++index];
                // Don't load the non-standard event - handled by returning false in main callback
            }
        }
    }
}

// Process introduction markers (extracted from play.cpp lines 317-336)
void MidiLoader::processIntroductionMarkers(const Event& event) {
    if (event.IsMeta(Message::MetaType::Marker) && event.size() == 3) {
        std::string text = event.GetText();
        if (text == INTRO_BEGIN) {    // Beginning of introduction segment
            IntroductionSegment seg;
            seg.start = totalTrackTicks_;
            seg.end = 0;
            
            introSegments_.push_back(seg);
        }
        
        if (text == INTRO_END) {    // End of introduction segment
            if (!introSegments_.empty()) {
                introSegments_.back().end = totalTrackTicks_;
            }
        }
    }
}

// Process track name events (extracted from play.cpp lines 409-412)
void MidiLoader::processTrackNameEvent(const Event& event) {
    Message message = event;
    
    if (message.IsMeta(Message::MetaType::TrackName) && title_.empty()) {
        title_ = message.GetText();
    }
}

// Scan Track 0 for meta events (extracted from play.cpp lines 397-413)
void MidiLoader::scanTrackZeroMetaEvents() {
    std::vector<Track>& tracks = (std::vector<Track>&)midiFile_;
    
    if (tracks.empty()) {
        return;
    }
    
    uint32_t totalTicks = 0;
    
    // Scan Meta events in Track 0 at time 0
    for (auto it = tracks[0].begin(); it != tracks[0].end(); ++it) {
        Event event = *it;
        Message message = (Message)event;
        uint32_t dt = event.Dt();
        totalTicks += dt;
        
        if (event.Dt() != 0) continue;   // The following code is only for messages at time 0
        
        processTrackNameEvent(event);
    }
}

// Finalize loading process (extracted from play.cpp lines 415-423)
void MidiLoader::finalizeLoading() {
    // Set default verses if not specified
    if (verses_ == 0) {
        verses_ = MidiPlay::DEFAULT_VERSES;
    }
    
    // If there are no intro markers in file, can't play intro
    // regardless of command-line option
    if (introSegments_.size() == 0) {
        playIntro_ = false;      // Override command line option if no markers
    }
    // Otherwise playIntro_ retains value from options (set in loadFile)
}

} // namespace MidiPlay