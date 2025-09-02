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

#include <ecocommon/utility.hpp>
#include "options.hpp"
#include "ticks.hpp"
#include "custommessage.hpp"

#include "ctx3000.hpp"
#include "psr-ew425.hpp"
#include "protege.hpp"
#include "constants.hpp"
#include "device_constants.hpp"

#include <semaphore.h>
#include <cmath>


using namespace cxxmidi;
using namespace midiplay;
namespace fs = std::filesystem;

static std::string version = "1.4.7";

// Constants to replace magic numbers
constexpr int MAJOR_KEY_OFFSET = 6;
constexpr int MINOR_KEY_OFFSET = 9;
constexpr uint8_t DEPRECATED_META_EVENT_VERSES = 0x10;
constexpr uint8_t DEPRECATED_META_EVENT_PAUSE = 0x11;
// Play-specific constants (not shared with other modules)
constexpr int64_t HEARTBEAT_CHECK_INTERVAL = 100000;
constexpr float RITARDANDO_DECREMENT = 0.002f;
constexpr int VERSE_DISPLAY_OFFSET = 1;

// Shared constants moved to constants.hpp and device_constants.hpp

output::Default outport;

sem_t sem;

struct _timesig {
   uint8_t beatsPerMeasure;
   uint8_t denominator;
   uint8_t clocksPerClick;   // of metronome
   uint8_t n32ndNotesPerQuaver;
};

std::string keySignature;

const char * const keys[] = {"Gb", "Db", "Ab", "Eb", "Bb", "F", "C",
                               "G", "D", "A", "E", "B", "F#", "C#", "G#", "D#", "A#", "e#"};

const std::string INTRO_BEGIN = "[";
const std::string INTRO_END = "]";
const std::string RITARDANDO_INDICATOR = R"(\)";
const std::string D_C_AL_FINE = "D.C. al Fine";
const std::string FINE_INDICATOR = "Fine";

struct _introSegment {
   uint32_t start;
   uint32_t end;
};

std::vector<struct _introSegment> introSegments;
bool playIntro = false;    // Don't play intro unless MIDI file specifies number of verses with Meta event or command line indicates
bool playingIntro = false;
bool ritardando = false;
bool lastVerse = false;
bool alFine = false;

bool firstTempo = true;

bool potentialStuckNote = false;
bool displayWarnings = false;

std::chrono::_V2::system_clock::time_point startTime;
std::chrono::_V2::system_clock::time_point endTime;


inline bool exists(const std::string& name) {
    struct stat buffer;   
    return (stat (name.c_str(), &buffer) == 0); 
}


// Finished callback
//
void finished()
{
     int ret = sem_post(&sem);
} 


// Callback for Ctrl+c
//
void control_c(int signum)  
{
     int ret = sem_post(&sem);

     // Turn all notes off.
     Event e;

     for (int channel = Channel1; channel <= Channel3; channel++)
     {
       for (int note = Note::kC2; note <= Note::kC7; note++)
       {
         e = Event(0, channel | Message::Type::NoteOn, note, 0); // Note Off
         outport.SendMessage(&e);
       }
     }

     ret = sem_destroy(&sem);  // Clean up the semaphore

     endTime = std::chrono::high_resolution_clock::now();

     // Calculate the elapsed time
     std::chrono::duration<double> elapsed = endTime - startTime;

     // Convert the elapsed time to minutes and seconds
     int minutes = static_cast<int>(elapsed.count()) / MidiPlay::SECONDS_PER_MINUTE;
     int seconds = static_cast<int>(elapsed.count()) % MidiPlay::SECONDS_PER_MINUTE;

     std::cout << "\nElapsed time " << minutes << ":" << std::setw(2) << std::setfill('0') << seconds << std::endl << std::endl;

     exit(signum);
}



int main(int argc, char **argv)
{
     // Set up Ctrl-C interrupt handler
     struct sigaction sigIntHandler;
     sigIntHandler.sa_handler = control_c;
     sigemptyset(&sigIntHandler.sa_mask);
     sigIntHandler.sa_flags = 0;
     sigaction(SIGINT, &sigIntHandler, NULL);

     int ret = sem_init(&sem, 0, 0);

     std::string title;

     std::vector<struct _introSegment>::iterator itintro;   // iterator for introduction segments

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

     playIntro = options.isPlayIntro();
     int verses = options.getVerses();
     float speed = options.getSpeed();
     bool prepost = options.isPrePost();
     int bpm = options.getBpm();
     int uSecPerBeat = options.get_uSecPerBeat();
     std::string filename = options.getFileName();  
     displayWarnings = options.isDisplayWarnings();

     int uSecPerQuarter = 0;     // This comes from the Tempo Meta event from the file.
     int uSecPerTick = 0;        // Calculated from uSecPerQuarter / File::TimeDivision()

     MidiTicks ticksToPause;


     std::string path = getFullPath(filename, options.isStaging());

     File midifile;
     struct _timesig timesig;

     midifile.SetCallbackLoad(
         [&](Event& event)->bool 
         {
             static int track;
             static int totalTrackTicks;
             static int lastNoteOn;
             static int lastNoteOff;

             Message message = event;

             uint8_t status = message[0];

             totalTrackTicks += event.Dt();

             if (message.IsSysex()) {
                 return false;    // Throw away SysEx events.  Player doesn't handle them.
             }

             // Throw away control change messages with specific exceptions:
             // NRPN (Non-Registered Parameter Number MSB & LSB)
             // Data Entry MSB & LSB
             // These exceptions are used for organ stop settings.
            if (message.IsControlChange()) {
                if (!message.IsControlChange(Message::ControlType::NonRegisteredParameterNumberLsb) 
                    && !message.IsControlChange(Message::ControlType::NonRegisteredParameterNumberMsb) 
                    && !message.IsControlChange(Message::ControlType::DataEntryMsb) 
                    && !message.IsControlChange(Message::ControlType::DataEntryLsb)) {
                    return false;   // Throw away most control change messages. Use organ controls instead.
                }
            }

             if (message.IsMeta())
             {
                 uint8_t type = message[1];

                 if (message.IsMeta(Message::MetaType::Lyrics)) {
                    return false;   // Throw away lyrics. Player doesn't handle them.
                 }

                 if (0 == totalTrackTicks)    // Processing for Time Zero Meta events
                 {
                     // Get Time Signature
                     if (message.IsMeta(Message::MetaType::TimeSignature) && message.size() == 6)
                     {
                         timesig.beatsPerMeasure = message[2];
                         timesig.denominator = message[3];
                         timesig.clocksPerClick = message[4];
                         timesig.n32ndNotesPerQuaver = message[5];
                     }
                     
                     // Get Tempo
                     if (message.IsMeta(Message::MetaType::Tempo))
                     {
                         // Get tempo from file
                         uSecPerQuarter = cxxmidi::utils::ExtractTempo(event[2], event[3], event[4]);

                         if (firstTempo) {
                             if (0 == bpm) {     // If BPM not overridden on command line
                                 if (uSecPerQuarter != 0) {
                                     int qpm = MidiPlay::MICROSECONDS_PER_MINUTE / uSecPerQuarter;  // Quarter notes per minute
                                     bpm = qpm * (std::pow(2.0, timesig.denominator) / MidiPlay::QUARTER_NOTE_DENOMINATOR);
                                 }

                                 if (uSecPerBeat != 0 && speed == 1.0)
                                 {
                                     speed = (float) uSecPerQuarter / (float) uSecPerBeat;
                                 }
                             }
                             else {
                                 int qpm = MidiPlay::MICROSECONDS_PER_MINUTE / uSecPerQuarter;  // Quarter notes per minute
                                 bpm = qpm * (std::pow(2.0, timesig.denominator) / MidiPlay::QUARTER_NOTE_DENOMINATOR);

                                 if (uSecPerBeat) {
                                     speed = (float) uSecPerQuarter / (float) uSecPerBeat;
                                 }
                             }

                             firstTempo = false;
                         }
                     }

                     // Get Key Signature
                     if (message.IsMeta(Message::MetaType::KeySignature))
                     {
                         int sf = static_cast<int8_t>(static_cast<uint8_t>(message[2]));
                         int mi = (uint8_t)message[3];

                         if (mi == 0)
                         {
                             keySignature = keys[sf + MAJOR_KEY_OFFSET];
                         }
                         else
                         {
                             keySignature = keys[sf + MINOR_KEY_OFFSET];
                             keySignature += " minor";
                         }
                     }

                     // Deprecated non-standard Meta events for verses and pause between verses
                     // These are superseded by the Sequencer-Specific Meta event below.
                     //
                     if (DEPRECATED_META_EVENT_VERSES == type)   // Non-standard "number of verses" Meta event type for this sequencer
                     {
                         // Extract the number of verses, if the event is present in the file, and then throw the event away.
                         if (verses == 0)    // If verses not specified in command line
                         {
                             char c = static_cast<char>(message[2]);

                             if (std::isdigit(c))
                             {
                                 std::string sVerse{c};
                                 verses = std::stoi(sVerse);
                                 playIntro = true;
                             }
                         }

                         return false;   // Don't load the non-standard event.
                     }

                     if (DEPRECATED_META_EVENT_PAUSE == type)   // Non-standard "pause between verses" Meta event type for this sequencer
                     {
                         ticksToPause = (static_cast<uint16_t>(message[2]) << 8) | message[3];

                         return false;   // Don't load the non-standard event.
                     }
                     //
                     // End deprecated non-standard Meta events for verses and pause between verses

                     if (message.IsMeta(Message::MetaType::SequencerSpecific)) {     // Sequencer-Specific Meta Event
                         int index = 2;
                         if (message[index] != CustomMessage::Type::Private) {
                             int len = message[index++];     // This does not conform to the MIDI standard
                         }

                         if (message[index++] == CustomMessage::Type::Private) {   // Prototyping, test, private use and experimentation
                             if (message[index] == CustomMessage::PrivateType::NumberOfVerses){   // Number of verses
                                 // Extract the number of verses, if the event is present in the file, and then throw the event away.
                                 if (verses == 0)    // If verses not specified in command line
                                 {
                                     char c = static_cast<char>(message[++index]);

                                     if (std::isdigit(c))
                                     {
                                         std::string sVerse{c};
                                         verses = std::stoi(sVerse);
                                         playIntro = true;
                                     }
                                 }

                                 return false;   // Don't load the non-standard event.
                             }

                             if (message[index] == CustomMessage::PrivateType::PauseBetweenVerses) {  // Pause between verses
                                 ticksToPause = (static_cast<uint16_t>(message[++index]) << 8) | message[++index];

                                 return false;   // Don't load the non-standard event.
                             }
                         }
                     }
                 }
             }   // Time 0

             if (track == 0) {   // Track 0-only messages
                 if (event.IsMeta(Message::MetaType::Marker) && event.size() == 3) {
                     std::string text = event.GetText();
                     if (text == INTRO_BEGIN)    // Beginning of introduction segment
                     {
                         struct _introSegment seg;
                         seg.start = totalTrackTicks;
                         seg.end = 0;

                         introSegments.push_back(seg);
                     }

                     if (text == INTRO_END)    // End of introduction segment
                     {
                         itintro = introSegments.end();
                         itintro--;
                         itintro->end = totalTrackTicks;
                     }
                 }
             }   // Track 0-only message handling

             if (event.IsVoiceCategory(Message::Type::NoteOn) && event[2] != 0) {
                 lastNoteOn = totalTrackTicks;
             }

             if ((event.IsVoiceCategory(Message::Type::NoteOn) && event[2] == 0) || event.IsVoiceCategory(Message::Type::NoteOff)) {
                 lastNoteOff = totalTrackTicks;
             }

             if (event.IsMeta(Message::MetaType::EndOfTrack)) {
                 track++;

                 if (introSegments.size()) {
                     itintro = introSegments.end();
                     itintro--;
                     uint32_t endIntro = itintro->end;

                     if (totalTrackTicks == endIntro) {
                         if (lastNoteOff >= endIntro) {
                             potentialStuckNote = true;
                         }
                     }
                 }

                 totalTrackTicks = 0;    // Reset ticks for next track
             }

             return true;
         }
     );

     // Check to see that the hymn file exists
     if (false == exists(path)) {
         std::cout << "Hymn " << filename << " was not found";

         if (options.isStaging()) {
             std::cout << " in the staging folder.\n" << std::endl;
         } else {
             std::cout << ".\n" << std::endl;
         }

         exit(MidiPlay::EXIT_FILE_NOT_FOUND);
     }

     midifile.Load(path.c_str());    // Load up the hymn file

     std::vector<Track> &tracks = (std::vector<Track> &)midifile;

     uint16_t ppq = midifile.TimeDivision();
     uSecPerTick = uSecPerQuarter / ppq;

     if (ticksToPause.isNull()) {
         ticksToPause = ppq;     // Default pause = 1 quarter note duration
     }

     Track::iterator it;


     uint32_t totalTicks = 0;

     // Scan Meta events in Track 0 at time 0
     for (it = tracks[0].begin(); it != tracks[0].end(); ++it)
     {
         Event event = *it;
         Message message = (Message)event;
         uint32_t dt = event.Dt();
         totalTicks += dt;

         uint8_t status = message[0];    // Status byte

         if (event.Dt() != 0) continue;   // The following code is only for messages at time 0

         if (message.IsMeta(Message::MetaType::TrackName) && title.empty())
         {
             title = message.GetText();
         }
     }

     if (verses == 0) { verses = MidiPlay::DEFAULT_VERSES; }


     itintro = introSegments.begin();    // Reset intro iterator

     if (introSegments.size() == 0)      // If there are no intro markers, don't play intro
     {
         playIntro = false;      // Override default or command line option
     }


     size_t portCount = outport.GetPortCount();
 #if defined(DEBUG)
     for (size_t i = 0; i < portCount; i++)
     {
       std::cout << i << ": " << outport.GetPortName(i) << std::endl;
     }

     std::cout << std::endl;
 #endif

   for (int i = 0; true; i++)
   {
     if (i > MidiPlay::Device::CONNECTION_TIMEOUT)
     {
       std::cout << "Device connection timeout.  No device found.  Connect a MIDI device and try again.\n"
            << std::endl;
       exit(MidiPlay::EXIT_DEVICE_NOT_FOUND);  // Timeout connecting device.
     }

     if (portCount >= MidiPlay::Device::MIN_PORT_COUNT)
     {
       break;  // We have a device. Open it and play.
     }
     else
     {
       std::cout << "No device connected.  Connect a device."
            << std::endl;
       //usleep(2000000);
       sleep(MidiPlay::Device::POLL_SLEEP_SECONDS);
       portCount = outport.GetPortCount(); // Try again and see if there's a connection
     }
   }

   outport.OpenPort(MidiPlay::Device::OUTPUT_PORT_INDEX);

   std::string portName = outport.GetPortName(MidiPlay::Device::OUTPUT_PORT_INDEX);

   if (portName.find("CASIO USB") == 0)
   {
     // This is a Casio USB MIDI device.
     ctx3000* ctx = new ctx3000(outport);
     ctx->SetDefaults();
   }
   else if (portName.find("Digital Keyboard") == 0)
   {
     psr_ew425* psr = new psr_ew425(outport);
     psr->SetDefaults();
   }
   else 
   {
     // NOT Casio or Yamaha device, probably Allen Protege organ
     // TODO: check for "USB MIDI Interface" first.
     protege* p = new protege(outport);
     p->SetDefaults();
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
