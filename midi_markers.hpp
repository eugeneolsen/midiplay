#pragma once

/**
 * @file midi_markers.hpp
 * @brief MIDI marker text constants used throughout the application
 * 
 * This header defines text markers used in MIDI files for musical direction
 * and structural control. These markers appear in MIDI Meta Events (Text or Marker)
 * and are interpreted by various components to control playback behavior.
 * 
 * Centralizing these constants ensures consistency across:
 * - EventPreProcessor (during MIDI file loading)
 * - MusicalDirector (during playback interpretation)
 * - PlaybackEngine (legacy - may be refactored)
 */

namespace MidiPlay {
    /**
     * @namespace MidiMarkers
     * @brief Text markers for musical direction and structure
     */
    namespace MidiMarkers {
        /**
         * @brief Introduction begin marker
         * Marks the start of an introduction segment that may be skipped after first verse
         */
        constexpr const char* INTRO_BEGIN = "[";
        
        /**
         * @brief Introduction end marker
         * Marks the end of an introduction segment, triggers jump logic during intro playback
         */
        constexpr const char* INTRO_END = "]";
        
        /**
         * @brief Ritardando indicator
         * Triggers gradual tempo slowdown (typically at end of piece)
         */
        constexpr const char* RITARDANDO_INDICATOR = R"(\)";
        
        /**
         * @brief Da Capo al Fine marker
         * Instructs playback to jump back to beginning and play until Fine marker
         */
        constexpr const char* D_C_AL_FINE = "D.C. al Fine";
        
        /**
         * @brief Fine marker
         * Marks early termination point when in "al Fine" mode
         */
        constexpr const char* FINE_INDICATOR = "Fine";
    }
}