#!/bin/bash

# Record start time
SECONDS=0

# Function to display elapsed time
display_elapsed_time() {
    local total_seconds=$SECONDS
    local hours=$((total_seconds / 3600))
    local minutes=$(((total_seconds % 3600) / 60))
    local seconds=$((total_seconds % 60))
    
    echo ""
    echo "=========================================="
    printf "Total execution time: %dh %dm %ds\n" $hours $minutes $seconds
    echo "=========================================="
}

# Ensure timing is displayed on exit (success or error)
trap display_elapsed_time EXIT

echo "Running test cases for the music player..."
echo ""
echo "Test Case 1: Simple playback"
echo "  Expected: No introduction, 2 measures in 4 seconds"
../play -s simple
sleep 2

echo "Test Case 2: Playback with intro and verses"
echo "  Expected: Introduction played, followed by 2 verses"
../play -s with_intro
sleep 2

echo "Test Case 3: Playback with -x1 flag and verbose mode"
echo "  Expected: No introduction, 1 verse played with verbose console output"
../play -s with_intro -x1 -V
sleep 2

echo "Test Case 4: Playback with -p (prelude/postlude) flag"
echo "  Expected: No introduction, 2 verses played at 90% speed"
../play -s with_intro -p
sleep 2

echo "Test Case 5: Playback with --tempo flag"
echo "  Expected: Introduction, 2 verses played at 160 BPM - noticeaably fast tempo"
../play -s with_intro -t160
sleep 2

echo "Test Case 6: Playback with DC al Fine"
echo "  Expected: Plays to the DC al Fine sign, then jumps back to the beginning and plays to the Fine sign"
../play -s dc_al_fine
sleep 2

echo "Test Case 7: Playback with DC al Fine and -n2 flag"
echo "  Expected: Plays 2 verses, jumps back to the beginning after the last verse and plays to the Fine sign"
../play -s dc_al_fine -n2
sleep 2
echo "Test Case 8: Playback with ritardando"
echo "  Expected: Introduction, 1 verse with dramatic ritardando effect in last half to end"
../play -s ritardando
