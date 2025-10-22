#include "playback_orchestrator.hpp"
#include "constants.hpp"
#include "i18n.hpp"

#include <cmath>
#include <iostream>
#include <thread>
#include <chrono>

#include <ecocommon/utility.hpp>

using cxxmidi::Event;
using cxxmidi::player::PlayerSync;

namespace MidiPlay {

PlaybackOrchestrator::PlaybackOrchestrator(PlayerSync& player,
                                           PlaybackSynchronizer& synchronizer,
                                           const MidiLoader& midiLoader)
    : player_(player)
    , synchronizer_(synchronizer)
    , midiLoader_(midiLoader)
    , stateMachine_()
    , musicalDirector_(player, stateMachine_, midiLoader)
    , ritardandoEffector_(player, stateMachine_)
    , baseSpeed_(1.0f)
    , baseTempo_(1.0f)
{
}

void PlaybackOrchestrator::initialize() {
    baseSpeed_ = midiLoader_.getSpeed();
    baseTempo_ = static_cast<float>(midiLoader_.getBpm() / static_cast<float>(midiLoader_.getFileTempo()));
    
    // Set initial speed
    player_.SetSpeed(baseTempo_ * baseSpeed_);
    
    // Setup heartbeat callback - delegates to ritardandoEffector_
    player_.SetCallbackHeartbeat([this]() {
        heartbeatCallback();
    });
    
    // Setup event callback - delegates to musicalDirector_
    player_.SetCallbackEvent([this](Event& event) -> bool {
        return eventCallback(event);
    });
    
    // Setup finished callback
    player_.SetCallbackFinished([this]() {
        finishedCallback();
    });
}

void PlaybackOrchestrator::displayPlaybackInfo() const {
    std::cout << _("Playing: \"") << midiLoader_.getTitle() << "\""
              << _(" in ") << midiLoader_.getKeySignature()
              << _(" - ") << formatPlural(midiLoader_.getVerses(), "verse", "verses");
    
    std::cout << _(" at ") << static_cast<int>(std::round(midiLoader_.getBpm() * baseSpeed_))
                << _(" bpm") << std::endl;
}

void PlaybackOrchestrator::executePlayback() {
    // Play introduction if available
    if (midiLoader_.shouldPlayIntro()) {
        playIntroduction();
    }
    
    // Play all verses
    playVerses();
}

// === Callback Handlers (delegate to components) ===

void PlaybackOrchestrator::heartbeatCallback() {
    ritardandoEffector_.handleHeartbeat();
}

bool PlaybackOrchestrator::eventCallback(Event& event) {
    return musicalDirector_.handleEvent(event);
}

void PlaybackOrchestrator::finishedCallback() {
    synchronizer_.notify();
}

// === Playback Flow Methods ===

void PlaybackOrchestrator::playIntroduction() {
    stateMachine_.setPlayingIntro(true);
    stateMachine_.setRitardando(false);
    
    const std::vector<IntroductionSegment>& introSegments = midiLoader_.getIntroSegments();
    
    if (introSegments.size() > 0) {
        musicalDirector_.initializeIntroSegments();
        player_.GoToTick(introSegments.begin()->start);
    }
    
    std::cout << _(" Playing introduction") << std::endl;
    
    player_.Play();
    synchronizer_.wait();  // Wait for playback to finish
    
    // Reset state after introduction
    stateMachine_.setRitardando(false);
    stateMachine_.setPlayingIntro(false);
    setPlayerSpeed(baseSpeed_);  // Reset speed to starting speed
    
    player_.Rewind();
    
    // Pause between intro and verses if specified
    MidiTicks pauseTicks = midiLoader_.getPauseTicks();
    if (pauseTicks.has_value()) {
        int uSecPerTick = midiLoader_.getUSecPerTick();
        std::this_thread::sleep_for(std::chrono::microseconds(pauseTicks.getTicks().value() * uSecPerTick));
    }
}

void PlaybackOrchestrator::playVerses() {
    int verses = midiLoader_.getVerses();
    MidiTicks pauseTicks = midiLoader_.getPauseTicks();
    int uSecPerTick = midiLoader_.getUSecPerTick();
    
    for (int verse = 0; verse < verses; verse++) {
        stateMachine_.setRitardando(false);
        setPlayerSpeed(baseSpeed_);
        
        std::cout << _(" Playing verse ") << verse + VERSE_DISPLAY_OFFSET;
        
        if (verse == verses - VERSE_DISPLAY_OFFSET) {
            stateMachine_.setLastVerse(true);
            std::cout << _(", last verse");
        }
        
        std::cout << std::endl;
        
        player_.Play();
        synchronizer_.wait();  // Wait for playback to finish
        
        if (!stateMachine_.isLastVerse()) {
            player_.Rewind();
            
            // Pause before starting next verse
            if (pauseTicks.has_value()) {
                std::this_thread::sleep_for(std::chrono::microseconds(pauseTicks.getTicks().value() * uSecPerTick));
            }
        }
        
        // Handle D.C. al Fine (Da Capo al Fine - return to beginning until Fine)
        if (stateMachine_.isAlFine()) {
            player_.Rewind();
            player_.Play();
            synchronizer_.wait();
        }
    }
}

void PlaybackOrchestrator::setPlayerSpeed(float speedMultiplier) {
    player_.SetSpeed(baseTempo_ * speedMultiplier);
}

} // namespace MidiPlay