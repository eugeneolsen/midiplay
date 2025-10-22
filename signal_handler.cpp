#include "signal_handler.hpp"
#include "constants.hpp"
#include "i18n.hpp"
#include <cxxmidi/channel.hpp>
#include <iostream>
#include <iomanip>
#include <cstdlib>

using cxxmidi::Event;
using cxxmidi::Message;
using cxxmidi::Note;

namespace MidiPlay {

// Static instance is now defined inline in the header

SignalHandler::~SignalHandler() {
    s_instance = nullptr;
}

void SignalHandler::setupSignalHandler() {
    struct sigaction sigIntHandler;
    sigIntHandler.sa_handler = SignalHandler::handleSignal;
    sigemptyset(&sigIntHandler.sa_mask);
    sigIntHandler.sa_flags = 0;
    sigaction(SIGINT, &sigIntHandler, NULL);
}

void SignalHandler::handleSignal(int signum) {
    if (s_instance != nullptr) {
        s_instance->postSemaphoreAndCleanup(signum);
    } else {
        // Fallback if no instance exists
        std::exit(signum);
    }
}

void SignalHandler::emergencyNotesOff() {
    // Turn all notes off to prevent stuck notes
    Event e;
    
    // Turn all notes off on channels 1-3 to prevent stuck notes
    for (int channel = cxxmidi::Channel1; channel <= cxxmidi::Channel3; channel++) {
        for (int note = Note::kC2; note <= Note::kC7; note++) {
            e = Event(0, channel | Message::Type::NoteOn, note, 0); // Note Off (velocity 0)
            m_outport.SendMessage(&e);
        }
    }
}

void SignalHandler::displayElapsedTime() const {
    auto endTime = std::chrono::high_resolution_clock::now();
    
    // Calculate the elapsed time
    std::chrono::duration<double> elapsed = endTime - m_startTime;
    
    // Convert the elapsed time to minutes and seconds
    int minutes = static_cast<int>(elapsed.count()) / SECONDS_PER_MINUTE;
    int seconds = static_cast<int>(elapsed.count()) % SECONDS_PER_MINUTE;
    
    std::cout << _("\nElapsed time ") << minutes << ":"
              << std::setw(2) << std::setfill('0') << seconds
              << std::endl << std::endl;
}

void SignalHandler::postSemaphoreAndCleanup(int signum) {
    // Notify synchronizer to wake up waiting threads
    m_synchronizer.notify();
    
    // Emergency notes-off to prevent stuck notes
    emergencyNotesOff();
    
    // Display elapsed time
    displayElapsedTime();
    
    // Exit with the signal number
    // Note: PlaybackSynchronizer cleanup happens automatically via RAII
    std::exit(signum);
}

} // namespace MidiPlay