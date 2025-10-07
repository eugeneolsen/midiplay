#include "playback_engine.hpp"
#include "constants.hpp"
#include "i18n.hpp"

#include <cmath>
#include <iostream>
#include <unistd.h>

#include <ecocommon/utility.hpp>

using cxxmidi::Event;
using cxxmidi::Message;
using cxxmidi::player::PlayerSync;

namespace MidiPlay {

PlaybackEngine::PlaybackEngine(PlayerSync& player,
                               PlaybackSynchronizer& synchronizer,
                               const MidiLoader& midiLoader)
    : player_(player)
    , synchronizer_(synchronizer)
    , midiLoader_(midiLoader)
    , playingIntro_(false)
    , ritardando_(false)
    , lastVerse_(false)
    , alFine_(false)
    , displayWarnings_(false)
    , baseSpeed_(1.0f)
    , baseTempo_(1.0f)
    , currentIntroSegment_()
{
}

void PlaybackEngine::initialize() {
    baseSpeed_ = midiLoader_.getSpeed();
    baseTempo_ = static_cast<float>(midiLoader_.getBpm() / static_cast<float>(midiLoader_.getFileTempo()));
    
    // Set initial speed
    player_.SetSpeed(baseTempo_ * baseSpeed_);
    
    // Setup heartbeat callback for ritardando control
    player_.SetCallbackHeartbeat([this]() {
        heartbeatCallback();
    });
    
    // Setup event callback for meta-event processing
    player_.SetCallbackEvent([this](Event& event) -> bool {
        return eventCallback(event);
    });
    
    // Setup finished callback
    player_.SetCallbackFinished([this]() {
        finishedCallback();
    });
}

void PlaybackEngine::displayPlaybackInfo() const {
    std::cout << _("Playing: \"") << midiLoader_.getTitle() << "\""
              << _(" in ") << midiLoader_.getKeySignature()
              << _(" - ") << formatPlural(midiLoader_.getVerses(), "verse", "verses");
    
    std::cout << _(" at ") << static_cast<int>(std::round(midiLoader_.getBpm() * baseSpeed_))
                << _(" bpm") << std::endl;
}

void PlaybackEngine::executePlayback() {
    // Play introduction if available
    if (midiLoader_.shouldPlayIntro()) {
        playIntroduction();
    }
    
    // Play all verses
    playVerses();
}

// === Callback Implementation Methods ===

void PlaybackEngine::heartbeatCallback() {
    if (ritardando_) {
        // Diminish speed gradually
        int64_t count = player_.CurrentTimePos().count();
        if (count % HEARTBEAT_CHECK_INTERVAL == 0) {
            float currentSpeed = player_.GetSpeed();
            currentSpeed -= RITARDANDO_DECREMENT;
            player_.SetSpeed(currentSpeed);
        }
    }
}

bool PlaybackEngine::eventCallback(Event& event) {
    Message message = event;
    
#ifdef DEBUG
    if (midiLoader_.isVerbose()) {
        dumpEvent(event);
    }
#endif

    // Process introduction markers if playing intro
    if (playingIntro_ && midiLoader_.getIntroSegments().size() > 0 && message.IsMeta()) {
        if (message.IsMeta(Message::MetaType::Marker) && message.GetText() == MidiMarkers::INTRO_END) {
            processIntroMarker(message);
        }
    }
    
    // Process ritardando markers (intro or last verse)
    if ((playingIntro_ || lastVerse_) && message.IsMeta(Message::MetaType::Marker)
        && message.GetText() == MidiMarkers::RITARDANDO_INDICATOR) {
        processRitardandoMarker(message);
    }
    
    // Process D.C. al Fine marker (last verse only)
    if (lastVerse_) {
        if (message.IsMeta(Message::MetaType::Marker) && message.GetText() == MidiMarkers::D_C_AL_FINE) {
            return processDCAlFineMarker(message);
        }
    }
    
    // Process Fine marker (when in al Fine mode)
    if (alFine_ && message.IsMeta(Message::MetaType::Marker)
        && message.GetText() == MidiMarkers::FINE_INDICATOR) {
        return processFineMarker(message);
    }
    
    return true;  // Send event to output device
}

void PlaybackEngine::finishedCallback() {
    synchronizer_.notify();
}

// === Playback Section Methods ===

void PlaybackEngine::playIntroduction() {
    playingIntro_ = true;
    ritardando_ = false;
    
    const std::vector<IntroductionSegment>& introSegments = midiLoader_.getIntroSegments();
    
    if (introSegments.size() > 0) {
        currentIntroSegment_ = introSegments.begin();
        player_.GoToTick(currentIntroSegment_->start);
    }
    
    std::cout << _(" Playing introduction") << std::endl;
    
    player_.Play();
    synchronizer_.wait();  // Wait for playback to finish
    
    // Reset state after introduction
    ritardando_ = false;
    playingIntro_ = false;
    setPlayerSpeed(baseSpeed_);  // Reset speed to starting speed
    
    player_.Rewind();
    
    // Pause between intro and verses if specified
    MidiTicks pauseTicks = midiLoader_.getPauseTicks();
    if (pauseTicks.has_value()) {
        int uSecPerTick = midiLoader_.getUSecPerTick();
        usleep(pauseTicks.getTicks().value() * uSecPerTick);
    }
}

void PlaybackEngine::playVerses() {
    int verses = midiLoader_.getVerses();
    MidiTicks pauseTicks = midiLoader_.getPauseTicks();
    int uSecPerTick = midiLoader_.getUSecPerTick();
    
    for (int verse = 0; verse < verses; verse++) {
        ritardando_ = false;
        setPlayerSpeed(baseSpeed_);
        
        std::cout << _(" Playing verse ") << verse + VERSE_DISPLAY_OFFSET;
        
        if (verse == verses - VERSE_DISPLAY_OFFSET) {
            lastVerse_ = true;
            std::cout << _(", last verse");
        }
        
        std::cout << std::endl;
        
        player_.Play();
        synchronizer_.wait();  // Wait for playback to finish
        
        if (!lastVerse_) {
            player_.Rewind();
            
            // Pause before starting next verse
            if (pauseTicks.has_value()) {
                usleep(pauseTicks.getTicks().value() * uSecPerTick);
            }
        }
        
        // Handle D.C. al Fine (Da Capo al Fine - return to beginning until Fine)
        if (alFine_) {
            player_.Rewind();
            player_.Play();
            synchronizer_.wait();
        }
    }
}

void PlaybackEngine::resetPlaybackState() {
    playingIntro_ = false;
    ritardando_ = false;
    lastVerse_ = false;
    alFine_ = false;
}

void PlaybackEngine::setPlayerSpeed(float speedMultiplier) {
    player_.SetSpeed(baseTempo_ * speedMultiplier);
}

// === Event Processing Helpers ===

void PlaybackEngine::processIntroMarker(const Message& message) {
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
            
            if (displayWarnings_) {
                std::cout << _("   Warning: Final intro marker not past last NoteOff event") << std::endl;
            }
        }
    }
}

void PlaybackEngine::processRitardandoMarker(const Message& message) {
    ritardando_ = true;
    std::cout << _("  Ritardando") << std::endl;
}

bool PlaybackEngine::processDCAlFineMarker(const Message& message) {
    std::cout << message.GetText() << std::endl;
    alFine_ = true;
    player_.Stop();
    player_.Finish();
    return false;  // Don't send event to output device
}

bool PlaybackEngine::processFineMarker(const Message& message) {
    player_.Stop();
    player_.Finish();
    return false;  // Don't send event to output device
}

} // namespace MidiPlay