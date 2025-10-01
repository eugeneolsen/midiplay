#pragma once

#include <mutex>
#include <condition_variable>

namespace MidiPlay {

/**
 * @brief Modern C++ synchronization primitive for playback control
 * 
 * Replaces POSIX semaphore (sem_t) with standard C++ synchronization.
 * Provides a simple wait/notify mechanism for coordinating playback
 * completion between the player thread and main thread.
 * 
 * This class uses std::condition_variable and std::mutex to provide
 * thread-safe, exception-safe synchronization through RAII principles.
 * 
 * Usage Pattern:
 * 1. Main thread calls wait() - blocks until playback completes
 * 2. Callback or signal handler calls notify() - wakes the waiting thread
 * 3. wait() automatically resets state for next use
 * 
 * Thread-safe: All methods can be called from multiple threads safely.
 * Exception-safe: Uses RAII lock guards, no manual cleanup required.
 */
class PlaybackSynchronizer {
public:
    /**
     * @brief Constructor - initializes synchronization state
     * 
     * Creates a new synchronizer with finished_ = false.
     * No manual initialization required (unlike sem_init).
     */
    PlaybackSynchronizer() = default;
    
    /**
     * @brief Destructor - automatic cleanup
     * 
     * RAII ensures proper cleanup with no manual destruction needed
     * (unlike sem_destroy).
     */
    ~PlaybackSynchronizer() = default;
    
    // Delete copy/move to prevent synchronization issues
    PlaybackSynchronizer(const PlaybackSynchronizer&) = delete;
    PlaybackSynchronizer& operator=(const PlaybackSynchronizer&) = delete;
    PlaybackSynchronizer(PlaybackSynchronizer&&) = delete;
    PlaybackSynchronizer& operator=(PlaybackSynchronizer&&) = delete;
    
    /**
     * @brief Wait for playback completion or interruption
     * 
     * Blocks the calling thread until notify() is called by another thread.
     * Automatically resets the finished flag after waking up, ready for
     * the next wait/notify cycle.
     * 
     * Thread-safe: Multiple threads can call this (though we only use one).
     * Exception-safe: Uses RAII std::unique_lock.
     * Spurious wakeup protection: Uses predicate to prevent false wakeups.
     * 
     * Equivalent to: sem_wait(&sem)
     */
    void wait();
    
    /**
     * @brief Signal playback completion or interruption
     * 
     * Wakes up one thread waiting in wait().
     * Can be safely called from signal handlers or callbacks.
     * 
     * Thread-safe: Can be called from any thread at any time.
     * Safe to call even if no thread is waiting.
     * 
     * Equivalent to: sem_post(&sem)
     */
    void notify();
    
    /**
     * @brief Reset synchronization state for next playback
     * 
     * Sets finished_ back to false. This is called automatically
     * at the end of wait(), so explicit calls are rarely needed.
     * 
     * Thread-safe: Protected by mutex.
     */
    void reset();

private:
    std::mutex mutex_;                  // Protects finished_ flag
    std::condition_variable cv_;        // Signals state changes
    bool finished_ = false;             // Playback completion flag
};

} // namespace MidiPlay