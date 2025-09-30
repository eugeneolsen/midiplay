#pragma once

#include <chrono>
#include <string>

namespace MidiPlay {

/**
 * @brief Manages timing and elapsed time tracking for MIDI playback
 * 
 * This class encapsulates all time-tracking functionality including:
 * - Session start/end time recording
 * - Elapsed time calculation
 * - Formatted time display (MM:SS format)
 */
class TimingManager {
public:
    /**
     * @brief Constructor
     */
    TimingManager();
    
    /**
     * @brief Start the playback timer
     * Records the current time as the session start
     */
    void startTimer();
    
    /**
     * @brief End the playback timer
     * Records the current time as the session end
     */
    void endTimer();
    
    /**
     * @brief Get elapsed time in seconds
     * @return Elapsed time as double (seconds)
     */
    double getElapsedSeconds() const;
    
    /**
     * @brief Get formatted elapsed time string
     * @return Formatted string in "MM:SS" format
     */
    std::string getFormattedElapsedTime() const;
    
    /**
     * @brief Display elapsed time to console
     * Outputs "Fine - elapsed time MM:SS" message
     */
    void displayElapsedTime() const;
    
    /**
     * @brief Get start time (for SignalHandler compatibility)
     * @return Reference to start time point
     */
    const std::chrono::time_point<std::chrono::high_resolution_clock>& getStartTime() const {
        return startTime_;
    }

private:
    std::chrono::time_point<std::chrono::high_resolution_clock> startTime_;
    std::chrono::time_point<std::chrono::high_resolution_clock> endTime_;
    
    /**
     * @brief Helper to format elapsed seconds into MM:SS
     * @param totalSeconds Total elapsed seconds
     * @return Formatted string
     */
    static std::string formatTime(int totalSeconds);
};

} // namespace MidiPlay