#pragma once

#include <string>
#include <unistd.h>
#include <iostream>

#include <getopt.h>

#include <ecocommon/utility.hpp>

using namespace std;

// Define the "long" command line options
static struct option long_options[] = {
    {"help", no_argument, NULL, 'h'},
    {"version", no_argument, NULL, 'v'},
    {"prelude", optional_argument, NULL, 'p'},
    {"goto", required_argument, NULL, 'g'},
    {"staging", no_argument, NULL, 's'},
    {"tempo", required_argument, NULL, 't'},
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
    bool _playIntro = false;
    string _filename;  // Provided as a command line argument


public:
    // Constructor
    Options(int argc, char** argv) {
        _argc = argc;
        _argv = argv;

        _verses = 0;
    }

    int getBpm() {
        return _bpm;
    }

    int getVerses() {
        return _verses;
    }

    int get_uSecPerBeat() {
        return _uSecPerBeat;
    }

    float getSpeed() {
        return _speed;
    }

    bool isStaging() {
        return _staging;
    }

    bool isPrePost() {
        return _prepost;
    }

    bool isPlayIntro() {
        return _playIntro;
    }

    std::string getFileName() {
        return _filename;
    }

    int parse(std::string version)
    {
        int opt;
        int option_index = 0;

        // Loop until there are no more options
        while ((opt = getopt_long(_argc, _argv, "vx:g:hn:p::st:?", long_options, &option_index)) != -1)
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
                break;
            case 'p':              // Prelude/Postlude
                _verses = 2;        // Play 2 verses
                _playIntro = false; // Don't play introduction

                if (optarg)
                {
                    // convert 2-digit number to float and divide by 10
                    if (isNumeric(optarg))
                    {
                        string s = optarg;
                        float speedOption = stof(s) / 10.0;

                        if (speedOption < 0.5 || speedOption > 2.0)
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
                    _speed = 0.8; // Default 80%
                }

                _prepost = true;
                break;
            case 'n':
                if (isNumeric(optarg))
                {
                    _verses = stoi(string(optarg));
                    _playIntro = true;
                }
                break;
            case 'x':
                if (isNumeric(optarg))
                {
                    _verses = stoi(string(optarg));
                    _playIntro = false;
                }
                break;
            case 's':   // Staging
                _staging = true;
                break;

            case 't':
                if (isNumeric(optarg))
                {
                    _bpm = stoi(optarg);
                    _uSecPerBeat = 60000000 / _bpm;
                }
                else
                {
                    cout << "Tempo must be numeric.  Exiting program." << endl;
                    exit(1);
                }
                break;
            case 'v':
                std::cout << "Version " << version << std::endl;
                return -2;
            case 'h':
            case '?':
                std::cout << "Play MIDI file command\n" << std::endl;
                std::cout << "Usage:\n" << endl;
                std::cout << "play <filename> options\n" << std::endl;
                std::cout << "  --goto=<marker | measure>  -g<marker | measure>   If argument is numeric, start at the measure number; if has alpha, start at marker." << endl;
                std::cout << "  --help -h -? This text." << endl;
                std::cout << "  -n<verses> Number of verses to play after introduction." << endl;
                std::cout << "  --prelude=<speed> -p<speed> Prelude/postlude.  <speed> is optional, default is 8, which is 80%.  10 is 100%.  Plays 2 verses by default; can be modified by -n<verses>" << endl;
                std::cout << "  --staging   Play the file from the staging directory." << std::endl;
                std::cout << "  --tempo=<bpm> -t<bpm>  Force tempo to the specified number of beats per minute." << endl;
                std::cout << "  --version -v  Version of this command" << endl;
                std::cout << "  -x<verses> Number of verses to play without introduction.\n" << endl;
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
        while (optind < _argc)
        {
            std::cerr << "Unrecognized argument: " << _argv[optind] << std::endl;
            optind++;
        }

        return 0;
    }
};