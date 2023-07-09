#pragma once

#include <string>
#include <unistd.h>
#include <iostream>

#include <getopt.h>

#include "utility.hpp"

using namespace std;

// Define the "long" command line options
static struct option long_options[] = {
    {"version", no_argument, NULL, 'v'},
    {"prelude", optional_argument, NULL, 'p'},
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

    bool getPrePost() {
        return _prepost;
    }

    bool getPlayIntro() {
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
        while ((opt = getopt_long(_argc, _argv, "vx:n:p::t:", long_options, &option_index)) != -1)
        {
            switch (opt)
            {
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
            case '?':
                std::cerr << "Unknown flag. Valid flags are --prelude= (or -p) optionally followed by a number and --version (or -v)" << std::endl;
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