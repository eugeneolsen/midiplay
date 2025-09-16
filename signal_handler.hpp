#pragma once

#include <signal.h>
#include <semaphore.h>
#include <chrono>
#include <cxxmidi/output/default.hpp>
#include <cxxmidi/event.hpp>
#include <cxxmidi/note.hpp>
#include <cxxmidi/message.hpp>

namespace MidiPlay {

/**
 * @brief Handles SIGINT (Ctrl+C) signal for graceful MIDI player shutdown
 * 
 * This class encapsulates signal handling functionality including:
 * - Emergency MIDI notes-off to prevent stuck notes
 * - Semaphore cleanup for proper synchronization
 * - Elapsed time display on interrupt
 * - Graceful application exit
 */
class SignalHandler {
public:
    /**
     * @brief Constructor - dependency injection
     * @param outport Reference to MIDI output port for emergency notes-off
     * @param semaphore Reference to semaphore for synchronization cleanup
     * @param startTime Reference to start time for elapsed time calculation
     */
    template<typename OutputType>
    SignalHandler(OutputType& outport,
                 sem_t& semaphore,
                 const std::chrono::time_point<std::chrono::high_resolution_clock>& startTime)
        : m_outport(outport)
        , m_semaphore(semaphore)
        , m_startTime(startTime)
    {
        // Ensure only one instance exists
        if (s_instance != nullptr) {
            throw std::runtime_error("SignalHandler: Only one instance allowed");
        }
        s_instance = this;
    }
    
    /**
     * @brief Destructor - cleanup static instance pointer
     */
    ~SignalHandler();
    
    /**
     * @brief Setup signal handling - registers the SIGINT handler
     */
    void setupSignalHandler();
    
    /**
     * @brief Static signal handler function (required by POSIX)
     * @param signum Signal number that was received
     */
    static void handleSignal(int signum);
    
    // Disable copy/move to prevent multiple instances
    SignalHandler(const SignalHandler&) = delete;
    SignalHandler& operator=(const SignalHandler&) = delete;
    SignalHandler(SignalHandler&&) = delete;
    SignalHandler& operator=(SignalHandler&&) = delete;

private:
    // Dependencies injected via constructor
    cxxmidi::output::Default& m_outport;
    sem_t& m_semaphore;
    const std::chrono::time_point<std::chrono::high_resolution_clock>& m_startTime;
    
    // Static instance pointer for signal handler access
    inline static SignalHandler* s_instance = nullptr;
    
    /**
     * @brief Send emergency notes-off to all channels and notes
     * Prevents stuck notes when application is interrupted
     */
    void emergencyNotesOff();
    
    /**
     * @brief Display elapsed time since start
     * Shows formatted MM:SS elapsed time
     */
    void displayElapsedTime() const;
    
    /**
     * @brief Post semaphore, cleanup, and exit
     * @param signum Signal number for exit code
     */
    void postSemaphoreAndCleanup(int signum);
};

} // namespace MidiPlay