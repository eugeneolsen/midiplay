#include <cxxmidi/player/player_sync.hpp>
#include <cxxmidi/file.hpp>
#include <cxxmidi/output/default.hpp>
#include <cxxmidi/note.hpp>
#include <signal.h>
#include <cstdlib>
#include <string>
#include <unistd.h>
#include <sys/stat.h>

#include <iostream>
#include <iomanip>

#include <ecocommon/timer.hpp>

#include "options.hpp"
#include "ticks.hpp"
#include "custommessage.hpp"

#include "constants.hpp"
#include "device_constants.hpp"
#include "signal_handler.hpp"
#include "device_manager.hpp"
#include "midi_loader.hpp"
#include "timing_manager.hpp"
#include "playback_engine.hpp"
#include "playback_synchronizer.hpp"

#include <cmath>


namespace fs = std::filesystem;

using namespace midiplay;

using cxxmidi::output::Default;
using cxxmidi::player::PlayerSync;

// Version is now established from the latest git tag at build time
// The git tag takes the form "Version x.y.z"

// Signal handling is now handled by the SignalHandler class
// Timing is now handled by the TimingManager class
// Playback orchestration is now handled by the PlaybackEngine class



int main(int argc, char **argv)
{
     // Signal handler will be set up after startTime is initialized

     // Get command line arguments
     //
     Options options(argc, argv);
     int rc = options.parse();
     if (rc != 0) {
         if (rc < 0) {
             exit(0);
         } else {
             exit(rc);
         }
     }

     // Extract initial option values
     std::string filename = options.getFileName();
     
     std::string path;
     try {
         path = getFullPath(filename, options.isStaging());
     }
     catch (const std::runtime_error& e) {
         std::cout << "Error: " << e.what() << std::endl;
         exit(MidiPlay::EXIT_ENVIRONMENT_ERROR);
     }

     // Use MidiLoader to handle all MIDI file loading and parsing
     MidiPlay::MidiLoader midiLoader;
     
     if (!midiLoader.loadFile(path, options)) {
         exit(MidiPlay::EXIT_FILE_NOT_FOUND);
     }

     Default outport;

     size_t portCount = outport.GetPortCount();

     if (options.isVerbose()) {
        std::cout << "Detected " << portCount << " MIDI output ports:" << std::endl;

        for (size_t i = 0; i < portCount; i++)
        {
            std::cout << i << ": " << outport.GetPortName(i) << std::endl;
        }

        std::cout << std::endl;
     }

   // Use DeviceManager to handle device connection and setup
   MidiPlay::DeviceManager deviceManager(options);
   
   // Load YAML configuration if available
   deviceManager.loadDevicePresets();
   
   try {
       // Connect to device and detect its type
       MidiPlay::DeviceInfo deviceInfo = deviceManager.connectAndDetectDevice(outport);
       
       // Create and configure device using factory pattern
       deviceManager.createAndConfigureDevice(deviceInfo.type, outport);
       
       if (options.isVerbose()) {
            // Display device information
            std::cout << "Connected to: " << deviceManager.getDeviceTypeName(deviceInfo.type)
                        << " (" << deviceInfo.portName << ")" << std::endl;
        }

   }
   catch (const std::exception& e) {
       std::cout << e.what() << std::endl;
       exit(MidiPlay::EXIT_DEVICE_NOT_FOUND);
   }

   PlayerSync player(&outport);
   player.SetFile(&midiLoader.getFile());

     // Create timing manager
     MidiPlay::TimingManager timingManager;
     timingManager.startTimer();
     
     // Create modern C++ synchronization primitive (replaces POSIX semaphore)
     MidiPlay::PlaybackSynchronizer synchronizer;
     
     // Create playback engine with dependencies
     MidiPlay::PlaybackEngine playbackEngine(player, synchronizer, midiLoader);
     playbackEngine.initialize();
     playbackEngine.setDisplayWarnings(options.isDisplayWarnings());
     
     // Display what we're about to play
     playbackEngine.displayPlaybackInfo();
     
     // Set up signal handler now that all dependencies are available
     MidiPlay::SignalHandler signalHandler(outport, synchronizer, timingManager.getStartTime());
     signalHandler.setupSignalHandler();

     // Execute complete playback sequence (intro + verses)
     playbackEngine.executePlayback();

     // Display elapsed time
     timingManager.endTimer();
     timingManager.displayElapsedTime();
     
     // Note: synchronizer cleanup happens automatically via RAII
}
