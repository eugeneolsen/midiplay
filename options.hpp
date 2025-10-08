#pragma once

#include <regex>
#include <string>
#include <unistd.h>
#include <iostream>

#include <getopt.h>

#include "i18n.hpp"
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
    int argc_;
    char** argv_;

    int bpm_ = 0;
    int verses_;    // Set in constructor
    int usec_per_beat_ = 0;

    float speed_ = 1.0;
    bool staging_ = false;
    bool prepost_ = false;
    bool play_intro_ = true;
    bool verbose_ = false;
    bool display_warnings_ = false;
    std::string filename_;  // Provided as a command line argument
    std::string url_name_;    // Second command line argument
    std::string title_;      // Hymn title

    // Private helper methods for option handling
    void displayVersion() const {
        std::cout << _("Organ Pi play MIDI file command\n");
        std::cout << _("===============================\n");
        std::cout << _("  Version ") << getSemanticVersion() << "\n" << std::endl;
    }
    
    void displayHelp() const {
        std::cout << _("Organ Pi play MIDI file command, version ") << getSemanticVersion() << std::endl;
        std::cout << _("===============================================\n") << std::endl;
        std::cout << _("Usage:\n") << std::endl;
        std::cout << "play <filename> options\n" << std::endl;
        std::cout << "  --goto=<marker | measure>  -g<marker | measure>   " << _("If argument is numeric, start at the measure number; if has alpha, start at marker. (not yet implemented)") << std::endl;
        std::cout << "  --help -h -? " << _("This text.") << std::endl;
        std::cout << "  -n<verses> " << _("Play the introduction followed by the specified number of verses.") << std::endl;
        std::cout << "  --prelude=<speed> -p<speed> " << _("Prelude/postlude.  <speed> is optional, default is 9, which is 90%.  10 is 100%.  Plays 2 verses by default; can be modified by -x<verses>") << std::endl;
        std::cout << "  --staging   " << _("Play the file from the staging directory, if present.") << std::endl;
        std::cout << "  --tempo=<bpm> -t<bpm>  " << _("Force tempo to the specified number of beats per minute.") << std::endl;
        std::cout << "  --version -v  " << _("Version of this command") << std::endl;
        std::cout << "  -x<verses> " << _("Number of verses to play without introduction.\n") << std::endl;
    }
    
    void handlePreludeOption(const char* optarg) {
        verses_ = 2;        // Play 2 verses
        play_intro_ = false; // Don't play introduction

        if (optarg) {
            // convert 2-digit number to float and divide by 10
            if (isNumeric(optarg)) {
                std::string s = optarg;
                float speedOption = std::stof(s) / PRELUDE_SPEED_DIVISOR;

                if (speedOption < PRELUDE_MIN_SPEED || speedOption > PRELUDE_MAX_SPEED) {
                    speed_ = 1;
                } else {
                    speed_ = speedOption;
                }
            }
        } else {
            speed_ = DEFAULT_PRELUDE_SPEED;
        }

        prepost_ = true;
    }
    
    void handleTempoOption(const char* optarg) {
        if (isNumeric(optarg)) {
            bpm_ = std::stoi(optarg);
            usec_per_beat_ = MidiPlay::MICROSECONDS_PER_MINUTE / bpm_;
        } else {
            std::cout << _("Tempo must be numeric.  Exiting program.") << std::endl;
            exit(1);
        }
    }
    
    void handleVersesOption(const char* optarg, bool playIntro) {
        if (isNumeric(optarg)) {
            verses_ = std::stoi(std::string(optarg));
            play_intro_ = playIntro;
        }
    }

public:
    static std::string getSemanticVersion() {
        static const std::regex pattern{R"((?:^|-|n|v|V)([0-9]+\.[0-9]+\.[0-9]+))"};
        std::smatch match;
        const std::string tag{APP_VERSION};
        if (std::regex_search(tag, match, pattern) && match.size() > 1) {
            return match[1].str();
        }
        return "not found";
    }

    // Constructor
    Options(int argc, char** argv) {
        argc_ = argc;
        argv_ = argv;

        verses_ = 0;
    }

    int getBpm() const {
        return bpm_;
    }

    int getVerses() const {
        return verses_;
    }

    int getUsecPerBeat() const {
        return usec_per_beat_;
    }

    float getSpeed() const {
        return speed_;
    }

    bool isStaging() const {
        return staging_;
    }

    bool isPrePost() const {
        return prepost_;
    }

    bool isPlayIntro() const {
        return play_intro_;
    }

    bool isVerbose() const {
        return verbose_;
    }

    bool isDisplayWarnings() const {
        return display_warnings_;
    }

    std::string getFileName() const {
        return filename_;
    }

    std::string getUrlName() const {
        return url_name_;
    }

    std::string getTitle() const {
        return title_;
    }

    int parse()
    {
        int opt;
        int option_index = 0;

        // Loop until there are no more options
        while ((opt = getopt_long(argc_, argv_, "vVx:g:hn:p::st:W?", long_options, &option_index)) != -1)
        {
            switch (opt)
            {
            case 'g':   // Goto measure or marker
                if (isNumeric(optarg)) {
                    // TODO: Go to measure
                } else {
                    // TODO: Go to marker
                }
                std::cout << _("Goto option not yet implemented. Starting at the beginning.") << std::endl;
                break;
                
            case 'p':   // Prelude/Postlude
                handlePreludeOption(optarg);
                break;
                
            case 'c':   // Channel override: -channel=n where n = 1 or 2 for Swell or Great
                // TO DO: Implement channel override
                break;

            case 'n':   // Verses with introduction
                handleVersesOption(optarg, true);
                break;
                
            case 'x':   // Verses without introduction
                handleVersesOption(optarg, false);
                break;
                
            case 's':   // Staging
                staging_ = true;
                break;
            
            case 'S':   // stops=<file name>
                // TO DO: implement stops override
                break;

            case 't':   // Tempo
                handleTempoOption(optarg);
                break;
                
            case 'T':   // Title
                title_ = optarg;
                break;
                
            case 'v':   // Version
                displayVersion();
                return -2;
                
            case 'V':   // Verbose
                verbose_ = true;
                break;
                
            case 'W':   // Warnings
                display_warnings_ = true;
                break;
                
            case 'h':   // Help
            case '?':
                displayHelp();
                return 1;
                
            default:
                abort();
            }
        }

        // If there are still arguments left, they are positional arguments
        if (optind < argc_) {
            // optind is declared in <getopt.h> as the index of the next non-option
            filename_ = argv_[optind];
#if defined(DEBUG)
            std::cout << "Filename: " << argv_[optind] << std::endl;
#endif
            optind++;
        } else {
            std::cerr << _("No filename provided. You must pass an file name to play.") << std::endl;
            return 1;
        }

        // Get urlName, if present
        if (optind < argc_) {
            url_name_ = argv_[optind];
            optind++;
        }

        // Handle any remaining arguments (if necessary)
        while (optind < argc_) {
            std::cerr << _("Unrecognized argument: ") << argv_[optind] << std::endl;
            optind++;
        }

        return 0;
    }
};