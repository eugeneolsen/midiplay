#pragma once

#include <string>
#include <unistd.h>
#include <iostream>

#include <getopt.h>

#include <ecocommon/utility.hpp>
#include "constants.hpp"

constexpr float DEFAULT_PRELUDE_SPEED = 0.90;   // 90% of default speed for hymn.
constexpr float PRELUDE_MIN_SPEED = 0.5;
constexpr float PRELUDE_MAX_SPEED = 2.0;
constexpr float PRELUDE_SPEED_DIVISOR = 10.0;   // Divide command line prelude speed by this to get float.

// Define the "long" command line options
static struct option long_options[] = {
    {"help", no_argument, NULL, 'h'},
    {"version", no_argument, NULL, 'v'},
    {"prelude", optional_argument, NULL, 'p'},
    {"goto", required_argument, NULL, 'g'},
    {"channel", required_argument, NULL, 'c'},  // Force channel to Swell (1) or Great (2)
    {"staging", no_argument, NULL, 's'},
    {"stops", required_argument, NULL, 'S'},    // --stops=<file name>  YAML file with stop definitions.
    {"tempo", required_argument, NULL, 't'},
    {"title", required_argument, NULL, 'T'},
    {"verbose", no_argument, NULL, 'V'},    // Verbose output
    {"warnings", no_argument, NULL, 'W'},   // Display warnings
    {NULL, 0, NULL, 0}};


class Options
{
private: 
    int _argc;
    char** _argv;

    int _bpm = 0;
    int _verses;    // Set in constructor
    int _uSecPerBeat = 0;

    float _speed = 1.0;
    bool _staging = false;
    bool _prepost = false;
    bool _playIntro = true;
    bool _verbose = false;
    bool _displayWarnings = false;
    std::string _filename;  // Provided as a command line argument
    std::string _urlName;    // Second command line argument
    std::string _title;      // Hymn title


public:
    // Constructor
    Options(int argc, char** argv) {
        _argc = argc;
        _argv = argv;

        _verses = 0;
    }

    int getBpm() const {
        return _bpm;
    }

    int getVerses() const {
        return _verses;
    }

    int get_uSecPerBeat() const {
        return _uSecPerBeat;
    }

    float getSpeed() const {
        return _speed;
    }

    bool isStaging() const {
        return _staging;
    }

    bool isPrePost() const {
        return _prepost;
    }

    bool isPlayIntro() const {
        return _playIntro;
    }

    bool isVerbose() const {
        return _verbose;
    }

    bool isDisplayWarnings() const {
        return _displayWarnings;
    }

    std::string getFileName() const {
        return _filename;
    }

    std::string getUrlName() const {
        return _urlName;
    }

    std::string getTitle() const {
        return _title;
    }

    int parse(std::string version)
    {
        int opt;
        int option_index = 0;

        // Loop until there are no more options
        while ((opt = getopt_long(_argc, _argv, "vVx:g:hn:p::st:wW?", long_options, &option_index)) != -1)
        {
            switch (opt)
            {
            case 'g':   // Goto measure or marker
                if (isNumeric(optarg))
                {
                    // TODO: Go to measure
                } else {
                    // TODO: Go to marker
                }

                std::cout << "Goto option not yet implemented. Starting at the beginning." << std::endl;
                break;
            case 'p':              // Prelude/Postlude
                _verses = 2;        // Play 2 verses
                _playIntro = false; // Don't play introduction

                if (optarg)
                {
                    // convert 2-digit number to float and divide by 10
                    if (isNumeric(optarg))
                    {
                        std::string s = optarg;
                        float speedOption = std::stof(s) / PRELUDE_SPEED_DIVISOR;

                        if (speedOption < PRELUDE_MIN_SPEED || speedOption > PRELUDE_MAX_SPEED)
                        {
                            _speed = 1;
                        }
                        else
                        {
                            _speed = speedOption;
                        }
                    }
                }
                else
                {
                    _speed = DEFAULT_PRELUDE_SPEED;
                }

                _prepost = true;
                break;
            case 'c':   // Channel override: -channel=n where n = 1 or 2 for Swell or Great
                // TO DO: Implement channel override
                break;

            case 'n':
                if (isNumeric(optarg))
                {
                    _verses = std::stoi(std::string(optarg));
                    _playIntro = true;
                }
                break;
            case 'x':
                if (isNumeric(optarg))
                {
                    _verses = std::stoi(std::string(optarg));
                    _playIntro = false;
                }
                break;
            case 's':   // Staging
                _staging = true;
                break;
            
            case 'S':   // stops=<file name>
                // TO DO: implement stops override
                break;

            case 't':
                if (isNumeric(optarg))
                {
                    _bpm = std::stoi(optarg);
                    _uSecPerBeat = MidiPlay::MICROSECONDS_PER_MINUTE / _bpm;
                }
                else
                {
                    std::cout << "Tempo must be numeric.  Exiting program." << std::endl;
                    exit(1);
                }
                break;
            case 'T':
                _title = optarg;
                break;
            case 'v':
                std::cout << "Organ Pi play MIDI file command\n";
                std::cout << "===============================\n";
                std::cout << "  Version " << version << "\n" << std::endl;
                return -2;
            case 'V':
                _verbose = true;
                break;
            case 'W':
                _displayWarnings = true;
                break;
            case 'h':
            case '?':
                std::cout << "Organ Pi play MIDI file command, version " << version << std::endl;
                std::cout << "===============================================\n" << std::endl;
                std::cout << "Usage:\n" << std::endl;
                std::cout << "play <filename> options\n" << std::endl;
                std::cout << "  --goto=<marker | measure>  -g<marker | measure>   If argument is numeric, start at the measure number; if has alpha, start at marker. (not yet implemented)" << std::endl;
                std::cout << "  --help -h -? This text." << std::endl;
                std::cout << "  -n<verses> Play the introduction followed by the specified number of verses." << std::endl;
                std::cout << "  --prelude=<speed> -p<speed> Prelude/postlude.  <speed> is optional, default is 9, which is 90%.  10 is 100%.  Plays 2 verses by default; can be modified by -x<verses>" << std::endl;
                std::cout << "  --staging   Play the file from the staging directory, if present." << std::endl;
                std::cout << "  --tempo=<bpm> -t<bpm>  Force tempo to the specified number of beats per minute." << std::endl;
                std::cout << "  --version -v  Version of this command" << std::endl;
                std::cout << "  -x<verses> Number of verses to play without introduction.\n" << std::endl;
                return 1;
            default:
                abort();
            }
        }

        // If there are still arguments left, they are positional arguments
        if (optind < _argc)
        { // optind is declared in <getopt.h> as the index of the next non-option
            _filename = _argv[optind];
#if defined(DEBUG)
            std::cout << "Filename: " << _argv[optind] << std::endl;
#endif
            optind++;
        }
        else
        {
            std::cerr << "No filename provided. You must pass an file name to play." << std::endl;
            return 1;
        }

        // Get urlName, if present
        if (optind < _argc)
        {
            _urlName = _argv[optind];
            optind++;
        }

        // Handle any remaining arguments (if necessary)
        while (optind < _argc)
        {
            std::cerr << "Unrecognized argument: " << _argv[optind] << std::endl;
            optind++;
        }

        return 0;
    }
};