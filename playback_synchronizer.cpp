#include "playback_synchronizer.hpp"

namespace MidiPlay {

void PlaybackSynchronizer::wait() {
    std::unique_lock<std::mutex> lock(mutex_);
    
    // Wait until finished_ becomes true
    // The predicate lambda prevents spurious wakeups
    cv_.wait(lock, [this]{ return finished_; });
    
    // Automatically reset for next use
    // This eliminates the need for manual reset calls
    finished_ = false;
}

void PlaybackSynchronizer::notify() {
    {
        // Acquire lock, set flag, release lock
        std::lock_guard<std::mutex> lock(mutex_);
        finished_ = true;
    }
    // Notify outside the lock for better performance
    // This is a best practice for condition variables
    cv_.notify_one();
}

void PlaybackSynchronizer::reset() {
    std::lock_guard<std::mutex> lock(mutex_);
    finished_ = false;
}

} // namespace MidiPlay