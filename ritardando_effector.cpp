#include "ritardando_effector.hpp"

namespace MidiPlay {

RitardandoEffector::RitardandoEffector(cxxmidi::player::PlayerSync& player,
                                       PlaybackStateMachine& stateMachine,
                                       float decrementRate)
    : player_(player)
    , stateMachine_(stateMachine)
    , decrementRate_(decrementRate)
{
}

void RitardandoEffector::handleHeartbeat() {
    if (stateMachine_.isRitardando()) {
        // Diminish speed gradually
        int64_t count = player_.CurrentTimePos().count();
        if (count % HEARTBEAT_CHECK_INTERVAL == 0) {
            float currentSpeed = player_.GetSpeed();
            currentSpeed -= decrementRate_;
            player_.SetSpeed(currentSpeed);
        }
    }
}

} // namespace MidiPlay