#include "./player_sync.hpp"    // Local, modified copy of cxxmidi
#include "./file.hpp"           // local, modified copy of cxxmidi
#include <cxxmidi/output/default.hpp>
#include <cxxmidi/note.hpp>
#include <signal.h>
#include <cstdlib>
#include <string>
#include <unistd.h>

#include <getopt.h>
#include <iostream>
#include <iomanip>
#include <boost/filesystem.hpp>

#include "timer.hpp"

#include "ctx3000.hpp"
#include "protege.hpp"

#include <semaphore.h>


using namespace std;
using namespace cxxmidi;
namespace fs = boost::filesystem;

static string version = "1.1.3"; 

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
bool playIntro = false;
bool playingIntro = false;
bool ritardando = false;
bool lastVerse = false;

void heartbeat()
{
    cout << "thump-thump\n";
}

void finished()
{
    int ret = sem_post(&sem);
}


void control_c(int signum)
{
    int ret = sem_post(&sem);

    // Turn all notes off.
    Event e;

    for (int channel = Channel1; channel <= Channel3; channel++)
    {
      for (int note = Note::kC0; note <= Note::kG9; note++)
      {
        e = Event(0, channel | Message::kNoteOn, note, 0); // Note Off
        outport.SendMessage(&e);
      }
    }

    ret = sem_destroy(&sem);  // Clean up the semaphore

    cout << endl;

    exit(signum);
}

bool isNumeric(const char *str)
{
    //char *s = const_cast<char *>(str);

    while (*str != '\0')
    {
      if (!isdigit(*str))
      {
        return false;
      }

      str++;
    }

    return true;
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

    //cout << argv[0] << endl;

    string title;
    int verses = 1;
    float speed = 1.0;
    bool prepost = false;   // Is this prelude or postlude?

    vector<struct _introSegment>::iterator itintro;   // iterator for introduction segments

    // Get command line arguments
    //
    int opt;
    string filename;  // Provided as a command line argument

    // Define the "long" command line options
    static struct option long_options[] = {
        {"version", no_argument, NULL, 'v'},
        {"prelude", optional_argument, NULL, 'p'},
        {NULL, 0, NULL, 0}};

    int option_index = 0;

    // Loop until there are no more options
    while ((opt = getopt_long(argc, argv, "vp::", long_options, &option_index)) != -1)
    {
      switch (opt)
      {
      case 'p':   // Prelude/Postlude
        verses = 2;           // Play 2 verses
        playIntro = false;    // Don't play introduction

        if (optarg) 
        {
            // convert 2-digit number to float and divide by 10
            if (isNumeric(optarg))
            {
                string s = optarg;
                float speedOption = stof(s) / 10.0;

                if (speedOption < 0.5 || speedOption > 2.0)
                {
                    speed = 1;
                }
                else
                {
                    speed = speedOption;
                }
            }
        }
        else
        {
            speed = 0.7;   // Default 70%
        }

        prepost = true;
        break;
      case 'v':
        std::cout << "Version " << version << std::endl;
        return 0;
      case '?':
        std::cerr << "Unknown flag. Valid flags are --prelude= (or -p) optionally followed by a number and --version (or -v)" << std::endl;
        return 1;
      default:
        abort();
      }
    }

    // If there are still arguments left, they are positional arguments
    if (optind < argc)
    { // optind is declared in <getopt.h> as the index of the next non-option
      filename = argv[optind];
#if defined(DEBUG)
      std::cout << "Filename: " << argv[optind] << std::endl;
#endif
      optind++;
    }
    else
    {
      std::cerr << "No filename provided. You must pass an file name to play." << std::endl;
      return 1;
    }

    // Handle any remaining arguments (if necessary)
    while (optind < argc)
    {
      std::cerr << "Unrecognized argument: " << argv[optind] << std::endl;
      optind++;
    }

    fs::path dir = ("/home/pi/Music/midihymns");
    string extension = ".mid";
    if (filename.length() < extension.length())
    {
      filename += extension;
    }

    string filenameEnd = filename.substr(filename.length() - extension.length(), extension.length());
    if (filenameEnd != extension)
    {
      filename += extension;
    }

    fs::path file = (filename);

    fs::path path = dir / file;

    File midifile(path.c_str());
    vector<Track> &tracks = (vector<Track> &)midifile;

    uint16_t ppq = midifile.TimeDivision();
    Track::iterator it;

#if defined(DEBUG)
  cout << ppq << " ticks per quarter note" << endl;

  Timer timer;
  int nEvents = 0;
  int nTracks = 0;

  // Iterate through all events of all tracks
  for (vector<Track>::iterator ti = tracks.begin(); ti != tracks.end(); ti++)
  {
    // TODO: Look for introduction and ritardando markers in Track 1
    //       and set up for introduction and ritard at the end of
    //       the intro and the end of the hymn.
    for (it = ti->begin(); it != ti->end(); ++it)
    {
      Event& event = *it;
      uint32_t dt = event.Dt();
      Message& message = static_cast<Message&>(*it);

      /* The following code is to change piano technique to organ technique
      if ((message[0] & 0xF0) == Message::kNoteOn && message[2] != 0x00) 
      {
        if (dt == 0x07) 
        {
          it->SetDt(0);
        }
      }

      if ((message[0] & 0xF0) == Message::kNoteOn && message[2] == 0x00) 
      {
        it->SetDt(dt + 7);
      }
      */

      nEvents++;
    }

    nTracks++;
  }

  double elapsed = timer.elapsed();

  std::string s = std::to_string(elapsed);
  cout << "Iterated through " << nEvents << " events in " << nTracks << " tracks in " << s << " seconds." << endl;
#endif

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
                if (!prepost)
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
        }

        if (event.Dt() != 0) continue;   // The following code is only for messages at time 0

        if (message[0] == Message::kMeta)
        {
            // Get title
            if (message[1] == Message::kTrackName && title.empty())
            {
                title = string(message.begin() + 2, message.end());

                cout << "Playing: \"" << title << "\"" << endl;
            }

            // Find number of verses, if present, in a custom Meta event 0x10
            if (message[1] == 0x10 /* Default number of verses */) 
            {
                if (!prepost)
                {
                    char c = static_cast<char>(message[2]);

                    if (isdigit(c))
                    {
                        verses = atoi(&c);
                        playIntro = true;
                    }
                }
            }

            // Get Time Signature
            if (message[1] == Message::kTimeSignature && message.size() == 6)
            {
                timesig.beatsPerMeasure = message[2];
                timesig.denominator = message[3];
                timesig.clocksPerClick = message[4];
                timesig.n32ndNotesPerQuaver = message[5];
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
  //midifile.SaveAs(".~/Music/125o.mid");
  //return 1;

  float tempo = player.GetSpeed();
  player.SetSpeed(tempo * speed);

// TODO: Diminish speed gradually when ritardando
//       At end of introduction and on last verse.
  player.SetCallbackHeartbeat(
      [&]() { 
                if (ritardando)
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
  //player.SetCallbackHeartbeat(heartbeat);

  player.SetCallbackFinished(finished);


  // Event callback
  //
  player.SetCallbackEvent(
    [&](Event& event)->bool 
    {
        Message message = event;
#ifdef DEBUG
        cout << "Event " << dec << event.Dt() << " ";

        for (Message::iterator iter = message.begin(); iter < message.end(); iter++)
        {
            uint8_t byte = *iter;
            cout << hex << std::setw(2) << std::setfill('0') << static_cast<int>(byte);
        }

        cout << endl;
#endif        

        if (playingIntro && message[0] == Message::kMeta && message[1] == Message::kMarker)
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


    // Play intro
    if (playIntro)
    {
        playingIntro = true;
        ritardando = false;

        cout << " Playing introduction" << endl;

        player.Play();
        ret = sem_wait(&sem);   // Wait on the semaphore

        playingIntro = false;
        cout << " Introduction ended" << endl;

        player.GoTo(std::chrono::microseconds::zero());
        sleep(2);
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

            sleep(2);
        }

        player.Play();
        ret = sem_wait(&sem);   // Wait on the semaphore
    }

    cout << "Fine" << endl << endl;

    ret = sem_destroy(&sem);  // Clean up the semaphore
}
