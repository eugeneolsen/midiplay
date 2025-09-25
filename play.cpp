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

#include <semaphore.h>
#include <cmath>


using namespace cxxmidi;
using namespace midiplay;
namespace fs = std::filesystem;

static std::string version = "1.5.3";

// Play-specific constants (not shared with other modules)
constexpr int64_t HEARTBEAT_CHECK_INTERVAL = 100000;
constexpr float RITARDANDO_DECREMENT = 0.002f;
constexpr int VERSE_DISPLAY_OFFSET = 1;

// Musical direction markers (used in event callback)
const std::string RITARDANDO_INDICATOR = R"(\)";
const std::string D_C_AL_FINE = "D.C. al Fine";
const std::string FINE_INDICATOR = "Fine";
const std::string INTRO_END = "]";

output::Default outport;

sem_t sem;

// Playback state variables
bool playingIntro = false;
bool ritardando = false;
bool lastVerse = false;
bool alFine = false;

bool displayWarnings = false;

std::chrono::_V2::system_clock::time_point startTime;
std::chrono::_V2::system_clock::time_point endTime;


// Finished callback
//
void finished()
{
     int ret = sem_post(&sem);
} 


// Signal handling is now handled by the SignalHandler class



int main(int argc, char **argv)
{
     // Signal handler will be set up after startTime is initialized

     int ret = sem_init(&sem, 0, 0);

     // Get command line arguments
     //
     Options options(argc, argv);
     int rc = options.parse(version);
     if (rc != 0) {
         if (rc < 0) {
             exit(0);
         } else {
             exit(rc);
         }
     }

     // Extract initial option values
     float speed = options.getSpeed();
     bool prepost = options.isPrePost();
     int bpm = options.getBpm();
     int uSecPerBeat = options.get_uSecPerBeat();
     std::string filename = options.getFileName();
     displayWarnings = options.isDisplayWarnings();

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

     // Get extracted data from MidiLoader
     cxxmidi::File& midifile = midiLoader.getFile();
     std::string title = midiLoader.getTitle();
     std::string keySignature = midiLoader.getKeySignature();
     int verses = midiLoader.getVerses();
     bool playIntro = midiLoader.shouldPlayIntro();
     const std::vector<MidiPlay::IntroductionSegment>& introSegments = midiLoader.getIntroSegments();
     MidiTicks ticksToPause = midiLoader.getPauseTicks();
     int uSecPerQuarter = midiLoader.getUSecPerQuarter();
     int uSecPerTick = midiLoader.getUSecPerTick();
     bool potentialStuckNote = midiLoader.hasPotentialStuckNote();

     // Iterator for introduction segments
     auto itintro = introSegments.begin();


     size_t portCount = outport.GetPortCount();
 #if defined(DEBUG)
     for (size_t i = 0; i < portCount; i++)
     {
       std::cout << i << ": " << outport.GetPortName(i) << std::endl;
     }

     std::cout << std::endl;
 #endif

   // Use DeviceManager to handle device connection and setup
   MidiPlay::DeviceManager deviceManager;
   
   // Load YAML configuration if available
   deviceManager.loadDevicePresets();
   
   try {
       // Connect to device and detect its type
       MidiPlay::DeviceInfo deviceInfo = deviceManager.connectAndDetectDevice(outport);
       
       // Create and configure device using factory pattern
       deviceManager.createAndConfigureDevice(deviceInfo.type, outport);
       
       // Display device information
       std::cout << "Connected to: " << deviceManager.getDeviceTypeName(deviceInfo.type)
                 << " (" << deviceInfo.portName << ")" << std::endl;
   }
   catch (const std::exception& e) {
       std::cout << e.what() << std::endl;
       exit(MidiPlay::EXIT_DEVICE_NOT_FOUND);
   }

   player::PlayerSync player(&outport);
   player.SetFile(&midifile);

   float tempo = player.GetSpeed();
   player.SetSpeed(tempo * speed);

     std::cout << "Playing: \"" << title << "\"" << " in " << keySignature << " - " << verses << " verse";
     if (verses > MidiPlay::DEFAULT_VERSES) {
         std::cout << "s";
     }
     std::cout << " at " << bpm * player.GetSpeed() << " bpm" << std::endl;


     // Heartbeat callback
     //
     player.SetCallbackHeartbeat(
         [&]() { 
                   if (ritardando)   // Diminish speed gradually
                   {
                       int64_t count = player.CurrentTimePos().count();
                       if (count % HEARTBEAT_CHECK_INTERVAL == 0)
                       {
                           float t = player.GetSpeed();
                           t -= RITARDANDO_DECREMENT;
                           //std::cout << "Speed: " << t << std::endl;
                           player.SetSpeed(t);
                       }
                   }
           });


   player.SetCallbackFinished(finished);


   // Event callback
   //
   player.SetCallbackEvent(
     [&](Event& event)->bool 
     {
         Message message = event;
 #ifdef DEBUG
         dumpEvent(event);
 #endif        
         if (playingIntro && introSegments.size() && message.IsMeta())
         {
             if (message.IsMeta(Message::MetaType::Marker) && message.GetText() == INTRO_END)
             {
                 itintro++;

                 if (itintro < introSegments.end())
                 {
                     uint32_t start = itintro->start;

                     //std::cout << "Jump to " << dec << start << std::endl;

                     player.Stop();
                     player.GoToTick(start);
                     player.Play();
                 }

                 if (itintro >= introSegments.end())
                 {
                     // Stop the introduction.  In some hymns, this is not at the end
                     player.Stop();
                     player.Finish();

                     if (potentialStuckNote) {
                         player.NotesOff();

                         if (displayWarnings) {
                             std::cout << "   Warning: Final intro marker not past last NoteOff event" << std::endl;
                         }
                     }
                 }
             }
         }

         if ((playingIntro || lastVerse) && message.IsMeta(Message::MetaType::Marker) && message.GetText() == RITARDANDO_INDICATOR) {
             // Start ritardando
             ritardando = true;
             std::cout << "  Ritardando" << std::endl;
         }

         if (lastVerse) {
             if (message.IsMeta(Message::MetaType::Marker) &&  message.GetText() == D_C_AL_FINE) {
                 std::cout << message.GetText() << std::endl;
                 alFine = true;
                 player.Stop();
                 player.Finish();
                 return false;   // Don't send event to output device
             }
         }

         if (alFine && message.IsMeta(Message::MetaType::Marker) && message.GetText() == FINE_INDICATOR) {
             player.Stop();
             player.Finish();
             return false;   // Don't send event to output device
         }

         return true;
     });

     // Start the elapsed time timer
     startTime = std::chrono::high_resolution_clock::now();
     
     // Set up signal handler now that all dependencies are available
     MidiPlay::SignalHandler signalHandler(outport, sem, startTime);
     signalHandler.setupSignalHandler();

     // Play intro
     if (playIntro)
     {
         playingIntro = true;
         ritardando = false;

         if (introSegments.size() > 0) 
         {
             itintro = introSegments.begin();
             player.GoToTick(itintro->start);
         }

         std::cout << " Playing introduction" << std::endl;

         player.Play();
         ret = sem_wait(&sem);   // Wait on the semaphore, which is posted in the Finished callback

         ritardando = false;
         playingIntro = false;
         player.SetSpeed(tempo * speed);     // Reset speed to starting speed

         player.Rewind();

         if (ticksToPause.has_value()) {
             usleep(ticksToPause.getTicks().value() * uSecPerTick);     // Pause before starting verse
         }
     }


     // Play verses
     for (int verse = 0; verse < verses; verse++)
     {
         ritardando = false;
         player.SetSpeed(tempo * speed);

         std::cout <<  " Playing verse " << verse + VERSE_DISPLAY_OFFSET;

         if (verse == verses - VERSE_DISPLAY_OFFSET)
         {
             lastVerse = true;
             std::cout << ", last verse";
         }

         //std::cout << " at speed " << speed;

         std::cout << std::endl;


         player.Play();
         ret = sem_wait(&sem);   // Wait on the semaphore, which is posted in the Finished callback


         if (!lastVerse)
         {
             player.Rewind();
             
             if (ticksToPause.has_value()) {
                 usleep(ticksToPause.getTicks().value() * uSecPerTick);     // Pause before starting next verse
             }
         }

         if (alFine) {
             player.Rewind();
             player.Play();
             ret = sem_wait(&sem);
         }
     }

     // End the timer
     endTime = std::chrono::high_resolution_clock::now();

     // Calculate the elapsed time
     std::chrono::duration<double> elapsed = endTime - startTime;

     // Convert the elapsed time to minutes and seconds
     int minutes = static_cast<int>(elapsed.count()) / MidiPlay::SECONDS_PER_MINUTE;
     int seconds = static_cast<int>(elapsed.count()) % MidiPlay::SECONDS_PER_MINUTE;

     std::cout << "Fine - elapsed time " << minutes << ":" << std::setw(2) << std::setfill('0') << seconds << std::endl << std::endl;

     ret = sem_destroy(&sem);  // Clean up the semaphore
}
