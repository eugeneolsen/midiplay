## Testing Strategy
### To Do:
1. Automate Unit Testing
2. Automate Integration Testing

### Test Files

- 1.mid - One long verse, which includes the introduction and 5 verses.  No marked introduction.
- 19.mid - Introduction + 3 verses
- 209.mid - Introduction + 2 verses
- 252.mid - Introduction + 1 verse
- 1202.mid - Introduction + 3 verses + D.C. al Fine

### 1. Unit Testing Approach

While we don't have a formal unit test framework, we can verify each component:

#### Test 1: Basic Synchronization
```cpp
// Test wait() and notify() in isolation
PlaybackSynchronizer sync;

std::thread waiter([&sync]() {
    sync.wait();  // Should block
    std::cout << "Woke up!" << std::endl;
});

std::this_thread::sleep_for(std::chrono::seconds(1));
sync.notify();  // Should wake waiter
waiter.join();
```

#### Test 2: Multiple Cycles
```cpp
// Test multiple wait/notify cycles (like verse playback)
PlaybackSynchronizer sync;

for (int i = 0; i < 3; i++) {
    std::thread t([&sync, i]() {
        sync.wait();
        std::cout << "Cycle " << i << " complete" << std::endl;
    });
    
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    sync.notify();
    t.join();
}
```

### 2. Integration Testing Checklist

Test all existing playback scenarios:

#### Playback Scenarios
- [ ] **Simple MIDI file** (no intro, 1 verse)
  - Verify: Plays once, displays elapsed time, exits cleanly
  
- [ ] **Multi-verse playback** (no intro, 3 verses)
  - Verify: All verses play, pauses between verses work
  
- [ ] **Introduction playback** (intro + verses)
  - Verify: Intro plays with marker jumping, verses follow
  
- [ ] **Ritardando** (with `\` marker)
  - Verify: Gradual slowdown during intro or last verse
  
- [ ] **D.C. al Fine** (with markers)
  - Verify: Returns to beginning, stops at Fine marker

#### Signal Handling
- [ ] **Ctrl+C during intro**
  - Verify: Stops immediately, displays elapsed time, no stuck notes
  
- [ ] **Ctrl+C during verse**
  - Verify: Stops immediately, displays elapsed time, no stuck notes
  
- [ ] **Ctrl+C during pause**
  - Verify: Exits cleanly, displays elapsed time

#### Edge Cases
- [ ] **Very short MIDI file** (< 1 second)
  - Verify: No race conditions, completes correctly
  
- [ ] **Very long MIDI file** (> 5 minutes)
  - Verify: Can interrupt at any time, memory stable

### 3. Memory and Thread Safety

#### Valgrind Check
```bash
valgrind --leak-check=full --show-leak-kinds=all ./play test.mid
```
**Expected**: No memory leaks, no invalid memory access

#### Thread Sanitizer Check
```bash
g++ -fsanitize=thread -g play.cpp ... -o play
./play test.mid
```
**Expected**: No data races, no thread safety issues

### 4. Performance Validation

The synchronization overhead should be negligible compared to MIDI playback:

```bash
# Measure with POSIX semaphore (current)
time ./play test.mid

# Measure with PlaybackSynchronizer (new)
time ./play test.mid
```

**Expected**: < 1% difference (within measurement noise)

---
