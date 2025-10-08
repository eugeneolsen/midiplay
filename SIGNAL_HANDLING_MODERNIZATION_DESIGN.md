# Signal Handling Modernization Design

**Date**: 2025-10-08  
**Issue**: CODE_SMELLS.md Issue #17 - Static Singleton Pattern in SignalHandler  
**Goal**: Modernize from POSIX signals to C++20 cooperative cancellation  
**Inspired by**: PlaybackSynchronizer migration (POSIX semaphores → std::condition_variable)

---

## Problem Statement

### Current Implementation (POSIX-based)

**Architecture:**
```
User presses Ctrl+C
    ↓
POSIX SIGINT signal delivered
    ↓
Static C function: SignalHandler::handleSignal(int)
    ↓
Access via static pointer: s_instance->postSemaphoreAndCleanup()
    ↓
Cleanup: emergencyNotesOff(), notify synchronizer, display time, exit
```

**Issues:**
1. **Static singleton pattern** - `s_instance` pointer bridges C signal handler with C++ instance
2. **Race condition risk** - Signal could arrive after destructor (though unlikely in practice)
3. **Platform dependency** - Uses POSIX `sigaction` API
4. **Non-portable** - Signal handling varies across platforms
5. **Hard to test** - Static state makes unit testing difficult

### Desired Implementation (C++20-based)

**Architecture:**
```
User presses Ctrl+C
    ↓
Signal monitoring thread (std::jthread) detects Ctrl+C via std::atomic flag
    ↓
Request stop: stop_source.request_stop()
    ↓
Playback thread checks: stop_token.stop_requested()
    ↓
Cleanup: emergencyNotesOff(), notify synchronizer, display time, graceful return
```

**Benefits:**
1. ✅ **No static state** - Pure instance-based design
2. ✅ **Thread-safe** - std::atomic and stop_token handle synchronization
3. ✅ **Portable** - Standard C++20, works on all platforms
4. ✅ **Testable** - Can simulate stop requests without signals
5. ✅ **RAII-compliant** - Clean automatic cleanup
6. ✅ **Cooperative** - Playback thread participates in cancellation

---

## Design: InterruptionMonitor Class

### Core Concept

Replace `SignalHandler` with `InterruptionMonitor` that:
1. Runs a lightweight monitoring thread to detect Ctrl+C
2. Uses `std::stop_source` to request cooperative cancellation
3. Provides `std::stop_token` to playback code for checking
4. Performs cleanup when interruption is detected

### Class Interface

```cpp
// interruption_monitor.hpp
#pragma once

#include <atomic>
#include <thread>
#include <stop_token>
#include <chrono>
#include <cxxmidi/output/default.hpp>

namespace MidiPlay {

/**
 * @brief Modern C++20 interruption handling using cooperative cancellation
 * 
 * Replaces POSIX signal handling with std::jthread and stop_token.
 * Monitors for Ctrl+C via atomic flag and requests cooperative
 * cancellation through standard C++ mechanisms.
 * 
 * Key improvements over POSIX signal handler:
 * - No static singleton pattern
 * - Portable across all C++20 platforms
 * - Testable without OS signals
 * - Thread-safe through std::atomic
 * - RAII-based cleanup
 */
class InterruptionMonitor {
public:
    /**
     * @brief Constructor - dependency injection
     * @param outport Reference to MIDI output port for emergency notes-off
     * @param synchronizer Reference to PlaybackSynchronizer
     * @param startTime Reference to start time for elapsed time
     */
    template<typename OutputType>
    InterruptionMonitor(OutputType& outport,
                       PlaybackSynchronizer& synchronizer,
                       const std::chrono::time_point<std::chrono::high_resolution_clock>& startTime)
        : m_outport(outport)
        , m_synchronizer(synchronizer)
        , m_startTime(startTime)
        , m_stopSource()
        , m_interruptRequested(false)
    {
        // Install minimal signal handler that sets atomic flag
        installSignalHandler();
        
        // Start monitoring thread
        m_monitorThread = std::jthread([this](std::stop_token stoken) {
            this->monitorForInterruption(stoken);
        });
    }
    
    /**
     * @brief Destructor - automatic cleanup via RAII
     */
    ~InterruptionMonitor() {
        // jthread automatically joins on destruction
        // Atomic flag automatically cleaned up
        restoreSignalHandler();
    }
    
    // Disable copy/move
    InterruptionMonitor(const InterruptionMonitor&) = delete;
    InterruptionMonitor& operator=(const InterruptionMonitor&) = delete;
    InterruptionMonitor(InterruptionMonitor&&) = delete;
    InterruptionMonitor& operator=(InterruptionMonitor&&) = delete;
    
    /**
     * @brief Get stop token for cooperative cancellation
     * @return std::stop_token that playback code can check
     */
    std::stop_token getStopToken() const {
        return m_stopSource.get_token();
    }
    
    /**
     * @brief Check if interruption was requested
     * @return true if Ctrl+C was pressed
     */
    bool isInterrupted() const {
        return m_interruptRequested.load(std::memory_order_acquire);
    }

private:
    // Dependencies
    cxxmidi::output::Default& m_outport;
    PlaybackSynchronizer& m_synchronizer;
    const std::chrono::time_point<std::chrono::high_resolution_clock>& m_startTime;
    
    // C++20 cooperative cancellation
    std::stop_source m_stopSource;
    std::jthread m_monitorThread;
    
    // Atomic flag for signal → thread communication
    std::atomic<bool> m_interruptRequested;
    
    // Previous signal handler for restoration
    struct sigaction m_oldHandler;
    
    /**
     * @brief Minimal static signal handler
     * Only sets atomic flag - thread does the rest
     */
    static void signalHandler(int signum);
    
    /**
     * @brief Install minimal signal handler
     */
    void installSignalHandler();
    
    /**
     * @brief Restore previous signal handler
     */
    void restoreSignalHandler();
    
    /**
     * @brief Monitor thread function
     * Checks atomic flag and requests cooperative stop
     */
    void monitorForInterruption(std::stop_token stoken);
    
    /**
     * @brief Perform cleanup when interruption detected
     */
    void performCleanup();
    
    /**
     * @brief Send emergency notes-off
     */
    void emergencyNotesOff();
    
    /**
     * @brief Display elapsed time
     */
    void displayElapsedTime() const;
    
    // Static instance for minimal signal handler access
    inline static std::atomic<bool>* s_interruptFlag = nullptr;
};

} // namespace MidiPlay
```

### Implementation Details

```cpp
// interruption_monitor.cpp
#include "interruption_monitor.hpp"
#include "constants.hpp"
#include "i18n.hpp"
#include <cxxmidi/channel.hpp>
#include <iostream>
#include <iomanip>
#include <signal.h>

using cxxmidi::Event;
using cxxmidi::Message;
using cxxmidi::Note;

namespace MidiPlay {

void InterruptionMonitor::signalHandler(int signum) {
    // Minimal work in signal handler - just set atomic flag
    if (s_interruptFlag != nullptr) {
        s_interruptFlag->store(true, std::memory_order_release);
    }
}

void InterruptionMonitor::installSignalHandler() {
    // Point static flag to our atomic
    s_interruptFlag = &m_interruptRequested;
    
    struct sigaction sigIntHandler;
    sigIntHandler.sa_handler = InterruptionMonitor::signalHandler;
    sigemptyset(&sigIntHandler.sa_mask);
    sigIntHandler.sa_flags = 0;
    
    // Save old handler for restoration
    sigaction(SIGINT, &sigIntHandler, &m_oldHandler);
}

void InterruptionMonitor::restoreSignalHandler() {
    // Restore previous handler
    sigaction(SIGINT, &m_oldHandler, nullptr);
    s_interruptFlag = nullptr;
}

void InterruptionMonitor::monitorForInterruption(std::stop_token stoken) {
    while (!stoken.stop_requested()) {
        // Check atomic flag periodically
        if (m_interruptRequested.load(std::memory_order_acquire)) {
            // Request cooperative stop
            m_stopSource.request_stop();
            
            // Perform cleanup
            performCleanup();
            
            // Exit monitoring thread
            break;
        }
        
        // Sleep briefly to avoid busy-waiting
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

void InterruptionMonitor::performCleanup() {
    // Notify synchronizer
    m_synchronizer.notify();
    
    // Emergency notes-off
    emergencyNotesOff();
    
    // Display elapsed time
    displayElapsedTime();
}

void InterruptionMonitor::emergencyNotesOff() {
    Event e;
    
    for (int channel = cxxmidi::Channel1; channel <= cxxmidi::Channel3; channel++) {
        for (int note = Note::kC2; note <= Note::kC7; note++) {
            e = Event(0, channel | Message::Type::NoteOn, note, 0);
            m_outport.SendMessage(&e);
        }
    }
}

void InterruptionMonitor::displayElapsedTime() const {
    auto endTime = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = endTime - m_startTime;
    
    int minutes = static_cast<int>(elapsed.count()) / SECONDS_PER_MINUTE;
    int seconds = static_cast<int>(elapsed.count()) % SECONDS_PER_MINUTE;
    
    std::cout << _("\nElapsed time ") << minutes << ":"
              << std::setw(2) << std::setfill('0') << seconds
              << std::endl << std::endl;
}

} // namespace MidiPlay
```

---

## Alternative: Fully Signal-Free Design

### Pure C++20 Approach (No POSIX signals at all)

For platforms where we control the terminal, we could eliminate signals entirely:

```cpp
class InterruptionMonitor {
    // Poll std::cin for Ctrl+C or 'q' key
    void monitorForInterruption(std::stop_token stoken) {
        // Non-blocking input monitoring
        // Press 'q' or Ctrl+C to stop
    }
};
```

**Trade-off:**
- ✅ Zero platform dependencies
- ❌ Requires terminal control
- ❌ Different UX (explicit key vs Ctrl+C)

**Recommendation:** Keep minimal signal handler for Ctrl+C compatibility, but make it testable.

---

## Integration with PlaybackOrchestrator

### Current Playback Loop

```cpp
void PlaybackOrchestrator::executePlayback() {
    // Play intro
    playIntroduction();
    
    // Play verses
    for (int verse = 1; verse <= versesToPlay_; verse++) {
        playVerse(verse);
        if (verse < versesToPlay_) {
            pauseBetweenVerses();
        }
    }
    
    // Notify completion
    synchronizer_.notify();
}
```

### Modernized with Stop Token

```cpp
void PlaybackOrchestrator::executePlayback(std::stop_token stopToken) {
    // Check for interruption before intro
    if (stopToken.stop_requested()) {
        synchronizer_.notify();
        return;
    }
    
    playIntroduction(stopToken);
    
    for (int verse = 1; verse <= versesToPlay_; verse++) {
        // Check between verses
        if (stopToken.stop_requested()) {
            synchronizer_.notify();
            return;
        }
        
        playVerse(verse, stopToken);
        
        if (verse < versesToPlay_) {
            if (stopToken.stop_requested()) {
                synchronizer_.notify();
                return;
            }
            pauseBetweenVerses(stopToken);
        }
    }
    
    synchronizer_.notify();
}

private:
    void pauseBetweenVerses(std::stop_token stopToken) {
        auto pauseEnd = std::chrono::steady_clock::now() + pauseDuration_;
        
        while (std::chrono::steady_clock::now() < pauseEnd) {
            // Check for stop every 100ms
            if (stopToken.stop_requested()) {
                return;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }
```

---

## Migration Plan

### Phase 1: Create InterruptionMonitor (1-2 hours)
- [ ] Create `interruption_monitor.hpp`
- [ ] Create `interruption_monitor.cpp`
- [ ] Implement minimal signal → atomic flag handler
- [ ] Implement monitoring thread with stop_token
- [ ] Add cleanup methods (copy from SignalHandler)

### Phase 2: Update PlaybackOrchestrator (1 hour)
- [ ] Add `std::stop_token` parameter to `executePlayback()`
- [ ] Add stop checks in playback loops
- [ ] Update pause logic to check stop_token
- [ ] Add stop checks in callbacks

### Phase 3: Update main() (30 minutes)
- [ ] Replace `SignalHandler` with `InterruptionMonitor`
- [ ] Pass `stop_token` to `playbackOrchestrator.executePlayback()`
- [ ] Update includes
- [ ] Test compilation

### Phase 4: Testing (1 hour)
- [ ] Test Ctrl+C during intro
- [ ] Test Ctrl+C during verse
- [ ] Test Ctrl+C during pause
- [ ] Test normal completion
- [ ] Verify elapsed time display
- [ ] Verify emergency notes-off

### Phase 5: Cleanup (30 minutes)
- [ ] Delete `signal_handler.hpp`
- [ ] Delete `signal_handler.cpp`
- [ ] Update `.vscode/tasks.json` build configuration
- [ ] Update documentation
- [ ] Log decision in ConPort

**Total Effort: ~4-5 hours**

---

## Comparison: Before vs After

### Memory & Performance

| Aspect | POSIX Signals | C++20 stop_token |
|--------|---------------|------------------|
| Static state | 1 pointer | 1 atomic bool* (transitional) |
| Thread overhead | None | 1 monitoring thread (~8KB stack) |
| Latency | Immediate | ~100ms polling interval |
| Portability | POSIX only | Any C++20 compiler |
| Testability | Difficult | Easy (mock stop_token) |

### Code Clarity

**Before (POSIX):**
```cpp
SignalHandler handler(outport, sync, startTime);
handler.setupSignalHandler();
orchestrator.executePlayback();
```

**After (C++20):**
```cpp
InterruptionMonitor monitor(outport, sync, startTime);
orchestrator.executePlayback(monitor.getStopToken());
```

---

## Testing Strategy

### Unit Tests (New Capability!)

```cpp
TEST(InterruptionMonitorTest, CooperativeCancellation) {
    MockOutput outport;
    PlaybackSynchronizer sync;
    auto startTime = std::chrono::high_resolution_clock::now();
    
    InterruptionMonitor monitor(outport, sync, startTime);
    
    // Simulate stop request (no OS signal needed!)
    std::stop_source source = /* get from monitor somehow */;
    source.request_stop();
    
    EXPECT_TRUE(monitor.isInterrupted());
}
```

### Integration Tests

Same as current:
- Ctrl+C during various playback phases
- Normal completion
- Emergency cleanup verification

---

## Decision Log Entry

```yaml
decision_id: TBD
summary: "Modernize signal handling from POSIX to C++20 cooperative cancellation"
rationale: |
  Current SignalHandler uses static singleton pattern to bridge POSIX signal
  handlers with C++ instance methods. This violates modern C++ best practices
  and limits testability. C++20 provides std::jthread and std::stop_token for
  cooperative cancellation, enabling:
  - Pure instance-based design (no static state)
  - Standard C++ (improved portability)
  - Unit testability (mock stop requests)
  - Better separation of concerns
  
  Mirrors our successful semaphore → condition_variable migration.
  
implementation_details: |
  Replace SignalHandler with InterruptionMonitor class that:
  1. Uses minimal signal handler → atomic flag for Ctrl+C detection
  2. Runs std::jthread monitoring thread to check flag
  3. Requests stop via std::stop_source
  4. Provides std::stop_token to playback code
  5. Performs cleanup when interruption detected
  
  PlaybackOrchestrator updated to check stop_token in loops.
  
tags:
  - modernization
  - cpp20
  - signal_handling
  - code_smell_fix
  - issue_17
```

---

## Open Questions

1. **Latency acceptable?** - 100ms polling vs immediate signal delivery
2. **Fallback needed?** - Keep old code for non-C++20 builds?
3. **Portability verified?** - Test on ARM64, x86_64, etc.

---

## References

- C++20 std::jthread: https://en.cppreference.com/w/cpp/thread/jthread
- C++20 std::stop_token: https://en.cppreference.com/w/cpp/thread/stop_token
- Our PlaybackSynchronizer design (semaphore migration)
- CODE_SMELLS.md Issue #17