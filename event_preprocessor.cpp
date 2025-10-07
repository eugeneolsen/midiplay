#include "event_preprocessor.hpp"
#include "options.hpp"
#include "i18n.hpp"
#include <iostream>
#include <cmath>
#include <cstdlib>

#include "midi_constants.hpp"

using cxxmidi::Event;
using cxxmidi::Message;

namespace MidiPlay {

// Define the static constexpr array that was declared in the header
const char* const EventPreProcessor::keys_[] = {
    "Gb", "Db", "Ab", "Eb", "Bb", "F", "C",
    "G", "D", "A", "E", "B", "F#", "C#", "G#", "D#", "A#", "e#"
};

// Constructor
EventPreProcessor::EventPreProcessor() {
    reset();
}

// Reset all state variables to initial values
void EventPreProcessor::reset() {
    title_.clear();
    keySignature_.clear();
    timeSignature_ = TimeSignature();
    introSegments_.clear();
    
    verses_ = 0;
    uSecPerQuarter_ = 0;
    pauseTicks_ = MidiTicks();
    
    potentialStuckNote_ = false;
    firstTempo_ = true;
    
    // Load callback state variables
    currentTrack_ = 0;
    totalTrackTicks_ = 0;
    lastNoteOn_ = 0;
    lastNoteOff_ = 0;
}

// Main event processing method
bool EventPreProcessor::processEvent(cxxmidi::Event& event, const Options& options) {
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
        return shouldLoadControlChangeEvent(event);
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
            
            // Process custom meta events - check return value
            if (!processCustomMetaEvents(event, options)) {
                // Custom meta event found and processed - discard it
                return false;
            }
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
void EventPreProcessor::processTimeSignatureEvent(const Event& event) {
    Message message = event;
    
    if (message.IsMeta(Message::MetaType::TimeSignature) && message.size() == 6) {
        timeSignature_.beatsPerMeasure = message[2];
        timeSignature_.denominator = message[3];
        timeSignature_.clocksPerClick = message[4];
        timeSignature_.n32ndNotesPerQuaver = message[5];
    }
}

// Process tempo events
void EventPreProcessor::processTempoEvent(const Event& event, const Options& options) {
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

// Process key signature events
void EventPreProcessor::processKeySignatureEvent(const Event& event) {
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

// Process custom meta events for data extraction
bool EventPreProcessor::processCustomMetaEvents(const Event& event, const Options& options) {
    Message message = event;
    uint8_t type = message[1];
    
    // Handle deprecated events
    if (DEPRECATED_META_EVENT_VERSES == type) {
        if (verses_ == 0) {
            char c = static_cast<char>(message[2]);
            if (std::isdigit(c)) {
                verses_ = std::stoi(std::string{c});
            }
        }
        
        if (options.isVerbose() || options.isDisplayWarnings()) {
            std::cout << _("Warning: Deprecated Meta event for number of verses found") << std::endl;
        }
        
        return false; // Custom event found - discard it
    }
    
    if (DEPRECATED_META_EVENT_PAUSE == type) {
        pauseTicks_ = (static_cast<uint16_t>(message[2]) << 8) | message[3];
        
        if (options.isVerbose() || options.isDisplayWarnings()) {
            std::cout << _("Warning: Deprecated Meta event for pause found") << std::endl;
        }
        
        return false; // Custom event found - discard it
    }
    
    // Handle sequencer-specific events
    if (message.IsMeta(Message::MetaType::SequencerSpecific)) {
        int index = 2;
        if (message[index] != midiplay::CustomMessage::Type::Private) {
            int len = message[index++];
        }
        
        if (message[index++] == midiplay::CustomMessage::Type::Private) {
            if (message[index] == midiplay::CustomMessage::PrivateType::NumberOfVerses) {
                if (verses_ == 0) {
                    char c = static_cast<char>(message[++index]);
                    if (std::isdigit(c)) {
                        verses_ = std::stoi(std::string{c});
                    }
                }
                return false; // Custom event found - discard it
            }
            
            if (message[index] == midiplay::CustomMessage::PrivateType::PauseBetweenVerses) {
                pauseTicks_ = (static_cast<uint16_t>(message[++index]) << 8) | message[++index];
                return false; // Custom event found - discard it
            }
        }
    }
    
    // No custom event found - continue processing
    return true;
}

// Process introduction markers
void EventPreProcessor::processIntroductionMarkers(const Event& event) {
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

// Process track name events
void EventPreProcessor::processTrackNameEvent(const Event& event) {
    Message message = event;
    
    if (message.IsMeta(Message::MetaType::TrackName) && title_.empty()) {
        title_ = message.GetText();
    }
}

// Event filtering logic for control change events
bool EventPreProcessor::shouldLoadControlChangeEvent(const cxxmidi::Event& event) {
    Message message = event;
    
    // Allow NRPN (Non-Registered Parameter Number MSB & LSB)
    // Allow Data Entry MSB & LSB
    // These exceptions are used for organ stop settings.
    if (message.IsControlChange(Message::ControlType::NonRegisteredParameterNumberLsb) 
        || message.IsControlChange(Message::ControlType::NonRegisteredParameterNumberMsb) 
        || message.IsControlChange(Message::ControlType::DataEntryMsb) 
        || message.IsControlChange(Message::ControlType::DataEntryLsb)) {
        return true;   // Load these specific control change messages
    }
    
    return false;   // Throw away most control change messages. Use organ controls instead.
}

} // namespace MidiPlay