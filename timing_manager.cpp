#include "timing_manager.hpp"
#include "constants.hpp"

#include <iostream>
#include <iomanip>
#include <sstream>

namespace MidiPlay {

TimingManager::TimingManager() 
    : startTime_()
    , endTime_()
{
}

void TimingManager::startTimer() {
    startTime_ = std::chrono::high_resolution_clock::now();
}

void TimingManager::endTimer() {
    endTime_ = std::chrono::high_resolution_clock::now();
}

double TimingManager::getElapsedSeconds() const {
    std::chrono::duration<double> elapsed = endTime_ - startTime_;
    return elapsed.count();
}

std::string TimingManager::getFormattedElapsedTime() const {
    int totalSeconds = static_cast<int>(getElapsedSeconds());
    return formatTime(totalSeconds);
}

void TimingManager::displayElapsedTime() const {
    int totalSeconds = static_cast<int>(getElapsedSeconds());
    int minutes = totalSeconds / MidiPlay::SECONDS_PER_MINUTE;
    int seconds = totalSeconds % MidiPlay::SECONDS_PER_MINUTE;
    
    std::cout << "Fine - elapsed time " << minutes << ":" 
              << std::setw(2) << std::setfill('0') << seconds 
              << std::endl << std::endl;
}

std::string TimingManager::formatTime(int totalSeconds) {
    int minutes = totalSeconds / MidiPlay::SECONDS_PER_MINUTE;
    int seconds = totalSeconds % MidiPlay::SECONDS_PER_MINUTE;
    
    std::ostringstream oss;
    oss << minutes << ":" << std::setw(2) << std::setfill('0') << seconds;
    return oss.str();
}

} // namespace MidiPlay