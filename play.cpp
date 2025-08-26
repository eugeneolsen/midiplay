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

#include "ctx3000.hpp"
#include "psr-ew425.hpp"
#include "protege.hpp"

#include <semaphore.h>
#include <cmath>


using namespace std;
using namespace cxxmidi;
namespace fs = std::filesystem;

static string version = "1.4.0"; 

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
                              "G", "D", "A", "E", "B", "F#", "C#", "G#", "D#", "A#"};

const std::string INTRO_BEGIN = "[";
const std::string INTRO_END = "]";
const std::string RITARDANDO_INDICATOR = R"(\)";
const std::string D_C_AL_FINE = "D.C. al Fine";
const std::string FINE_INDICATOR = "Fine";

struct _introSegment {
  uint32_t start;
  uint32_t end;
};

vector<struct _introSegment> introSegments;
bool playIntro = false;    // Don't play intro unless MIDI file specifies verses with Meta event 0x10 or command line indicates
bool playingIntro = false;
bool ritardando = false;
bool lastVerse = false;
bool alFine = false;

bool firstTempo = true;

bool potentialStuckNote = false;
bool displayWarnings = false;

std::chrono::_V2::system_clock::time_point startTime;
std::chrono::_V2::system_clock::time_point endTime;

// Finished callback
//
void finished()
{
    int ret = sem_post(&sem);
}

inline bool exists(const std::string& name) {
  struct stat buffer;   
  return (stat (name.c_str(), &buffer) == 0); 
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
        e = Event(0, channel | Message::kNoteOn, note, 0); // Note Off
        outport.SendMessage(&e);
      }
    }

    ret = sem_destroy(&sem);  // Clean up the semaphore

    endTime = std::chrono::high_resolution_clock::now();

    // Calculate the elapsed time
    std::chrono::duration<double> elapsed = endTime - startTime;

    // Convert the elapsed time to minutes and seconds
    int minutes = static_cast<int>(elapsed.count()) / 60;
    int seconds = static_cast<int>(elapsed.count()) % 60;

    cout << "\nElapsed time " << minutes << ":" << setw(2) << setfill('0') << seconds << endl << endl;

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

    string title;

    vector<struct _introSegment>::iterator itintro;   // iterator for introduction segments

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
    string filename = options.getFileName();  
    displayWarnings = options.isDisplayWarnings();

    int uSecPerQuarter = 0;     // This comes from the Tempo Meta event from the file.
    int uSecPerTick = 0;        // Calculated from uSecPerQuarter / File::TimeDivision()

    MidiTicks ticksToPause;


    string path = getFullPath(filename, options.isStaging());

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

            if (message.IsMeta())
            {
                uint8_t type = message[1];

                if (event.Dt() == 0)    // Processing for Time Zero Meta events
                {
                    // Get Time Signature
                    if (Message::kTimeSignature == type && message.size() == 6)
                    {
                        timesig.beatsPerMeasure = message[2];
                        timesig.denominator = message[3];
                        timesig.clocksPerClick = message[4];
                        timesig.n32ndNotesPerQuaver = message[5];
                    }
                    
                    // Get Tempo
                    if (message.IsMeta(Message::kTempo))
                    {
                        // Get tempo from file
                        uSecPerQuarter = cxxmidi::utils::ExtractTempo(event[2], event[3], event[4]);

                        if (firstTempo) {
                            if (0 == bpm) {     // If BPM not overridden on command line
                                if (uSecPerQuarter != 0) {
                                    int qpm = 60000000 / uSecPerQuarter;  // Quarter notes per minute
                                    bpm = qpm * (pow(2.0, timesig.denominator) / 4);
                                }

                                if (uSecPerBeat != 0 && speed == 1.0)
                                {
                                    speed = (float) uSecPerQuarter / (float) uSecPerBeat;
                                }
                            }
                            else {
                                int qpm = 60000000 / uSecPerQuarter;  // Quarter notes per minute
                                bpm = qpm * (pow(2.0, timesig.denominator) / 4);

                                if (uSecPerBeat) {
                                    speed = (float) uSecPerQuarter / (float) uSecPerBeat;
                                }
                            }

                            firstTempo = false;
                        }
                    }

                    // Get Key Signature
                    if (message.IsMeta(Message::kKeySignature))
                    {
                        int sf = static_cast<int8_t>(static_cast<uint8_t>(message[2]));
                        int mi = (uint8_t)message[3];

                        if (mi == 0)
                        {
                            keySignature = keys[sf + 6];
                        }
                        else
                        {
                            keySignature = keys[sf + 9]; 
                            keySignature += " minor";
                        }
                    }

                    if (0x10 == type)   // Non-standard "number of verses" Meta event type for this sequencer
                    {
                        // Extract the number of verses, if the event is present in the file, and then throw the event away.
                        if (verses == 0)    // If verses not specified in command line
                        {
                            char c = static_cast<char>(message[2]);

                            if (isdigit(c))
                            {
                                string sVerse{c};
                                verses = stoi(sVerse);
                                playIntro = true;
                            }
                        }

                        return false;   // Don't load the non-standard event.
                    }

                    if (0x11 == type)   // Non-standard "pause between verses" Meta event type for this sequencer
                    {
                        ticksToPause = (static_cast<uint16_t>(message[2]) << 8) | message[3];

                        return false;   // Don't load the non-standard event.
                    }

                    if (0x7F == type) {     // Sequencer-Specific Meta Event
                        int index = 2;
                        if (message[index] != 0x7D) {
                            int len = message[index++];     // This does not conform to the MIDI standard
                        }

                        if (0x7D == message[index++]) {   // Prototyping, test, private use and experimentation
                            if (1 == message[index]){   // Number of verses
                                // Extract the number of verses, if the event is present in the file, and then throw the event away.
                                if (verses == 0)    // If verses not specified in command line
                                {
                                    char c = static_cast<char>(message[++index]);

                                    if (isdigit(c))
                                    {
                                        string sVerse{c};
                                        verses = stoi(sVerse);
                                        playIntro = true;
                                    }
                                }

                                return false;   // Don't load the non-standard event.
                            }

                            if (2 == message[index]) {  // Pause between verses
                                ticksToPause = (static_cast<uint16_t>(message[++index]) << 8) | message[++index];

                                return false;   // Don't load the non-standard event.
                            }
                        }
                    }
                }
            }   // Time 0

            if (track == 0) {   // Track 0-only messages
                if (event.IsMeta(Message::kMarker) && event.size() == 3) {
                    if (event.GetText() == INTRO_BEGIN)    // Beginning of introduction segment
                    {
                        struct _introSegment seg;
                        seg.start = totalTrackTicks;
                        seg.end = 0;

                        introSegments.push_back(seg);
                    }

                    if (event.GetText() == INTRO_END)    // End of introduction segment
                    {
                        itintro = introSegments.end();
                        itintro--;
                        itintro->end = totalTrackTicks;
                    }
                }
            }   // Track 0-only message handling

            if (event.IsVoiceCategory(Message::kNoteOn) && event[2] != 0) {
                lastNoteOn = totalTrackTicks;
            }

            if ((event.IsVoiceCategory(Message::kNoteOn) && event[2] == 0) || event.IsVoiceCategory(Message::kNoteOff)) {
                lastNoteOff = totalTrackTicks;
            }

            if (event.IsMeta(Message::MetaType::kEndOfTrack)) {
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

        exit(2);
    }

    midifile.Load(path.c_str());    // Load up the hymn file

    vector<Track> &tracks = (vector<Track> &)midifile;

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

        if (message.IsMeta(Message::kTrackName) && title.empty())
        {
            title = message.GetText();
        }
    }

    if (verses == 0) { verses = 1; }


    itintro = introSegments.begin();    // Reset intro iterator


    size_t portCount = outport.GetPortCount();
#if defined(DEBUG)
    for (size_t i = 0; i < portCount; i++)
    {
      cout << i << ": " << outport.GetPortName(i) << endl;
    }

    cout << endl;
#endif

  for (int i = 0; true; i++)
  {
    if (i > 300) 
    {
      std::cout << "Device connection timeout.  No device found.  Connect a MIDI device and try again.\n"
           << std::endl;
      exit(2);  // Timeout connecting device.
    }

    if (portCount >= 2)
    {
      break;  // We have a device. Open it and play.
    }
    else
    {
      std::cout << "No device connected.  Connect a device."
           << std::endl;
      //usleep(2000000);
      sleep(2);
      portCount = outport.GetPortCount(); // Try again and see if there's a connection
    }
  }

  outport.OpenPort(1);

  std::string portName = outport.GetPortName(1);

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
    protege* p = new protege(outport);
    p->SetDefaults();
  }

  player::PlayerSync player(&outport);
  player.SetFile(&midifile);

  float tempo = player.GetSpeed();
  player.SetSpeed(tempo * speed);

    cout << "Playing: \"" << title << "\"" << " in " << keySignature << " - " << verses << " verse";
    if (verses > 1) {
        cout << "s";
    }
    cout << " at " << bpm * player.GetSpeed() << " bpm" << endl;


    // Heartbeat callback
    //
    player.SetCallbackHeartbeat(
        [&]() { 
                  if (ritardando)   // Diminish speed gradually
                  {
                      int64_t count = player.CurrentTimePos().count();
                      if (count % 100000 == 0)
                      {
                          float t = player.GetSpeed();
                          t -= .002;
                          //cout << "Speed: " << t << endl;
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
            if (message.IsMeta(Message::kMarker) && message.GetText() == INTRO_END)
            {
                itintro++;

                if (itintro < introSegments.end())
                {
                    uint32_t start = itintro->start;

                    //cout << "Jump to " << dec << start << endl;

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

        if ((playingIntro || lastVerse) && message.IsMeta(Message::kMarker) && message.GetText() == RITARDANDO_INDICATOR) {
            // Start ritardando
            ritardando = true;
            std::cout << "  Ritardando" << std::endl;
        }

        if (lastVerse) {
            if (message.IsMeta(Message::kMarker) &&  message.GetText() == D_C_AL_FINE) {
                std::cout << " " << message.GetText() << std::endl;
                alFine = true;
                player.Stop();
                player.Finish();
                return false;   // Don't send event to output device
            }
        }

        if (alFine && message.IsMeta(Message::kMarker) && message.GetText() == FINE_INDICATOR) {
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

        cout << " Playing introduction" << endl;

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

        cout <<  " Playing verse " << verse + 1;

        if (verse == verses - 1)
        {
            lastVerse = true;
            cout << ", last verse";
        }

        //cout << " at speed " << speed;

        cout << endl;


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
    int minutes = static_cast<int>(elapsed.count()) / 60;
    int seconds = static_cast<int>(elapsed.count()) % 60;

    cout << "Fine - elapsed time " << minutes << ":" << setw(2) << setfill('0') << seconds << endl << endl;

    ret = sem_destroy(&sem);  // Clean up the semaphore
}
