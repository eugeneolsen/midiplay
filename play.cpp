#include <ecomidi/player_sync.hpp>    // modified copy of cxxmidi
#include <ecomidi/file.hpp>           // modified copy of cxxmidi
#include <cxxmidi/output/default.hpp>
#include <cxxmidi/note.hpp>
#include <signal.h>
#include <cstdlib>
#include <string>
#include <unistd.h>

#include <iostream>
#include <iomanip>

#include "timer.hpp"

#include "utility.hpp"
#include "options.hpp"

#include "ctx3000.hpp"
#include "protege.hpp"

#include <semaphore.h>


using namespace std;
using namespace cxxmidi;
namespace fs = boost::filesystem;

static string version = "1.2.0"; 

output::Default outport;

sem_t sem;

struct _timesig {
  uint8_t beatsPerMeasure;
  uint8_t denominator;
  uint8_t clocksPerClick;   // of metronome
  uint8_t n32ndNotesPerQuaver;
};

struct _introSegment {
  uint32_t start;
  uint32_t end;
};

vector<struct _introSegment> introSegments;
bool playIntro = false;    // Don't play intro unless MIDI file specifies verses with Meta event 0x10
bool playingIntro = false;
bool ritardando = false;
bool lastVerse = false;

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
        e = Event(0, channel | Message::kNoteOn, note, 0); // Note Off
        outport.SendMessage(&e);
      }
    }

    ret = sem_destroy(&sem);  // Clean up the semaphore

    cout << endl;

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

    int verses = options.getVerses();
    float speed = options.getSpeed();
    bool prepost = options.getPrePost();
    int bpm = options.getBpm();
    int uSecPerBeat = options.get_uSecPerBeat();
    string filename = options.getFileName();  


    string path = getFullPath(filename);

    File midifile(path.c_str());
    vector<Track> &tracks = (vector<Track> &)midifile;

    uint16_t ppq = midifile.TimeDivision();
    Track::iterator it;


    struct _timesig timesig;
    uint32_t totalTicks = 0;

    // Scan Meta events in Track 0 at time 0
    for (it = tracks[0].begin(); it != tracks[0].end(); ++it)
    {
        Event event = *it;
        Message message = (Message)event;
        uint32_t dt = event.Dt();
        totalTicks += dt;

        if (message[0] == Message::kMeta)
        {
            if (message[1] == Message::kMarker && message.size() == 3)
            {
                if (message[2] == '[')    // Beginning of introduction segment
                {
                    struct _introSegment seg;
                    seg.start = totalTicks;
                    seg.end = 0;

                    introSegments.push_back(seg);
                }

                if (message[2] == ']')    // End of introduction segment
                {
                    itintro = introSegments.end();
                    itintro--;
                    itintro->end = totalTicks;
                }
            }
        }

        if (event.Dt() != 0) continue;   // The following code is only for messages at time 0

        if (message[0] == Message::kMeta)
        {
            // Get Time Signature
            if (message[1] == Message::kTimeSignature && message.size() == 6)
            {
                timesig.beatsPerMeasure = message[2];
                timesig.denominator = message[3];
                timesig.clocksPerClick = message[4];
                timesig.n32ndNotesPerQuaver = message[5];
            }
            
            // Get Tempo
            if (message[1] == Message::kTempo && bpm == 0)
            {
                // The following works correctly when timesig.denominator == 2 - TODO: fix it to also work with 1 and 3
                int uSecFromFile = cxxmidi::utils::ExtractTempo(event[2], event[3], event[4]);

                if (uSecFromFile != 0) {
                    bpm = 60000000 / uSecFromFile;
                }

                if (uSecPerBeat != 0)
                {
                    speed = (float) uSecFromFile / (float) uSecPerBeat;
                }
            }

            // Get title
            if (message[1] == Message::kTrackName && title.empty())
            {
                title = string(message.begin() + 2, message.end());
            }

            // Find number of verses, if present, in a custom Meta event 0x10
            if (message[1] == 0x10 /* Default number of verses */) 
            {
                if (!prepost)
                {
                    char c = static_cast<char>(message[2]);

                    if (isdigit(c))
                    {
                        string sVerse{c};
                        verses = stoi(sVerse);
                        playIntro = true;
                    }
                }
            }
        }
    }

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
      cout << "Device connection timeout.  No device found.  Connect a MIDI device and try again.\n"
           << endl;
      exit(2);  // Timeout connecting device.
    }

    if (portCount >= 2)
    {
      break;  // We have a device. Open it and play.
    }
    else
    {
      cout << "No device connected.  Connect a device."
           << endl;
      usleep(2000000);
      portCount = outport.GetPortCount(); // Try again and see if there's a connection
    }
  }

  outport.OpenPort(1);

  if (outport.GetPortName(1).find("CASIO USB") == string::npos) 
  {
    // NOT Casio device, probably Allen Protege organ
    protege* p = new protege(outport);
    p->SetDefaults();
  }
  else
  {
    // This is a Casio USB MIDI device.
    ctx3000* ctx = new ctx3000(outport);
    ctx->SetDefaults();
  }

  player::PlayerSync player(&outport);
  player.SetFile(&midifile);

  float tempo = player.GetSpeed();
  player.SetSpeed(tempo * speed);

    cout << "Playing: \"" << title << "\"" << " - " << verses << " verse";
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

        if (playingIntro && introSegments.size() > 0 
            && message[0] == Message::kMeta && message[1] == Message::kMarker)
        {
            if (message[2] == ']')
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
                    player.Stop();
                    player.Finish();
                }
            }
        }

        if ((playingIntro || lastVerse) && message[0] == Message::kMeta && message[1] == Message::kMarker && message[2] == '\\')
        {
            // Start ritardando
            ritardando = true;
            cout << "  Ritardando" << endl;
        }

        return true;
    });

    // Start the elapsed time timer
    auto startTime = std::chrono::high_resolution_clock::now();

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
        ret = sem_wait(&sem);   // Wait on the semaphore

        playingIntro = false;

        player.GoTo(std::chrono::microseconds::zero());
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

        cout << endl;

        if (verse > 0) 
        {
            player.GoTo(std::chrono::microseconds::zero());

        }

        player.Play();
        ret = sem_wait(&sem);   // Wait on the semaphore
    }

    // End the timer
    auto endTime = std::chrono::high_resolution_clock::now();

    // Calculate the elapsed time
    std::chrono::duration<double> elapsed = endTime - startTime;

    // Convert the elapsed time to minutes and seconds
    int minutes = static_cast<int>(elapsed.count()) / 60;
    int seconds = static_cast<int>(elapsed.count()) % 60;

    cout << "Fine - elapsed time " << minutes << ":" << setw(2) << setfill('0') << seconds << endl << endl;

    ret = sem_destroy(&sem);  // Clean up the semaphore
}
