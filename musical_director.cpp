#include "musical_director.hpp"
#include "i18n.hpp"

#include <iostream>

using cxxmidi::Event;
using cxxmidi::Message;

namespace MidiPlay {

MusicalDirector::MusicalDirector(cxxmidi::player::PlayerSync& player,
                                 PlaybackStateMachine& stateMachine,
                                 const MidiLoader& midiLoader)
    : player_(player)
    , stateMachine_(stateMachine)
    , midiLoader_(midiLoader)
    , currentIntroSegment_()
{
}

void MusicalDirector::initializeIntroSegments() {
    const std::vector<IntroductionSegment>& introSegments = midiLoader_.getIntroSegments();
    if (introSegments.size() > 0) {
        currentIntroSegment_ = introSegments.begin();
    }
}

bool MusicalDirector::handleEvent(Event& event) {
    Message message = event;
    
#ifdef DEBUG
    if (midiLoader_.isVerbose()) {
        dumpEvent(event);
    }
#endif

    // Process introduction markers if playing intro
    if (stateMachine_.isPlayingIntro() && midiLoader_.getIntroSegments().size() > 0 && message.IsMeta()) {
        if (message.IsMeta(Message::MetaType::Marker) && message.GetText() == MidiMarkers::INTRO_END) {
            processIntroMarker(message);
        }
    }
    
    // Process ritardando markers (intro or last verse)
    if ((stateMachine_.isPlayingIntro() || stateMachine_.isLastVerse()) && message.IsMeta(Message::MetaType::Marker)
        && message.GetText() == MidiMarkers::RITARDANDO_INDICATOR) {
        processRitardandoMarker(message);
    }
    
    // Process D.C. al Fine marker (last verse only)
    if (stateMachine_.isLastVerse()) {
        if (message.IsMeta(Message::MetaType::Marker) && message.GetText() == MidiMarkers::D_C_AL_FINE) {
            return processDCAlFineMarker(message);
        }
    }
    
    // Process Fine marker (when in al Fine mode)
    if (stateMachine_.isAlFine() && message.IsMeta(Message::MetaType::Marker)
        && message.GetText() == MidiMarkers::FINE_INDICATOR) {
        return processFineMarker(message);
    }
    
    return true;  // Send event to output device
}

void MusicalDirector::processIntroMarker(const Message& message) {
    const std::vector<IntroductionSegment>& introSegments = midiLoader_.getIntroSegments();
    
    currentIntroSegment_++;
    
    if (currentIntroSegment_ < introSegments.end()) {
        uint32_t start = currentIntroSegment_->start;
        player_.Stop();
        player_.GoToTick(start);
        player_.Play();
    }
    
    if (currentIntroSegment_ >= introSegments.end()) {
        // Stop the introduction. In some hymns, this is not at the end
        player_.Stop();
        player_.Finish();
        
        if (midiLoader_.hasPotentialStuckNote()) {
            player_.NotesOff();
            
            if (stateMachine_.shouldDisplayWarnings()) {
                std::cout << _("   Warning: Final intro marker not past last NoteOff event") << std::endl;
            }
        }
    }
}

void MusicalDirector::processRitardandoMarker(const Message& message) {
    stateMachine_.setRitardando(true);
    std::cout << _("  Ritardando") << std::endl;
}

bool MusicalDirector::processDCAlFineMarker(const Message& message) {
    std::cout << message.GetText() << std::endl;
    stateMachine_.setAlFine(true);
    player_.Stop();
    player_.Finish();
    return false;  // Don't send event to output device
}

bool MusicalDirector::processFineMarker(const Message& message) {
    player_.Stop();
    player_.Finish();
    return false;  // Don't send event to output device
}

} // namespace MidiPlay