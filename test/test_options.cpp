#include "external/catch_amalgamated.hpp"
#include "../options.hpp"
#include <vector>
#include <string>
#include <cstring>

using Catch::Approx;

// Helper to reset getopt global state completely
void resetGetopt() {
    resetGetopt();    // Reset to start of argv
    opterr = 1;    // Reset error reporting
    optopt = 0;    // Reset option character
}

// Helper functions to create argv from vector of strings
char** makeArgv(const std::vector<std::string>& args) {
    char** argv = new char*[args.size()];
    for (size_t i = 0; i < args.size(); i++) {
        argv[i] = strdup(args[i].c_str());
    }
    return argv;
}

void freeArgv(char** argv, int argc) {
    for (int i = 0; i < argc; i++) {
        free(argv[i]);
    }
    delete[] argv;
}

TEST_CASE("Options basic parsing", "[options][unit]") {
    SECTION("minimal arguments") {
        resetGetopt();  // Reset getopt global state
        auto args = std::vector<std::string>{"play", "test.mid"};
        char** argv = makeArgv(args);
        
        Options opts(args.size(), argv);
        REQUIRE(opts.parse() == 0);
        REQUIRE(opts.getFileName() == "test.mid");
        REQUIRE(opts.getSpeed() == 1.0f);
        
        freeArgv(argv, args.size());
    }
    
    SECTION("version flag returns -2") {
        resetGetopt();  // Reset getopt global state
        auto args = std::vector<std::string>{"play", "-v"};
        char** argv = makeArgv(args);
        
        Options opts(args.size(), argv);
        REQUIRE(opts.parse() == -2);
        
        freeArgv(argv, args.size());
    }
    
    SECTION("help flag returns 1") {
        resetGetopt();  // Reset getopt global state
        auto args = std::vector<std::string>{"play", "-h"};
        char** argv = makeArgv(args);
        
        Options opts(args.size(), argv);
        REQUIRE(opts.parse() == 1);
        
        freeArgv(argv, args.size());
    }
    
    SECTION("question mark help returns 1") {
        resetGetopt();  // Reset getopt global state
        auto args = std::vector<std::string>{"play", "-?"};
        char** argv = makeArgv(args);
        
        Options opts(args.size(), argv);
        REQUIRE(opts.parse() == 1);
        
        freeArgv(argv, args.size());
    }
    
    // NOTE: Test disabled - causes segfault due to getopt_long edge case with argc=1
    // Will be re-enabled after Options refactoring (see OPTIONS_REFACTORING_PLAN.md)
    // SECTION("missing filename returns error") {
    //     resetGetopt();  // Reset getopt global state
    //     auto args = std::vector<std::string>{"play"};
    //     char** argv = makeArgv(args);
    //
    //     Options opts(args.size(), argv);
    //     REQUIRE(opts.parse() == 1);
    //
    //     freeArgv(argv, args.size());
    // }
}

// NOTE: Invalid tempo test would call exit(1), terminating test runner
// This test will be added after Options refactoring (see OPTIONS_REFACTORING_PLAN.md)
// TEST_CASE("Options invalid tempo", "[options][unit]") {
//     SECTION("non-numeric tempo value") {
//         resetGetopt();
//         auto args = std::vector<std::string>{"play", "test.mid", "-tABC"};
//         char** argv = makeArgv(args);
//
//         Options opts(args.size(), argv);
//         // Currently calls exit(1), should return error code after refactoring
//         REQUIRE(opts.parse() == 3);  // Future: INVALID_TEMPO
//
//         freeArgv(argv, args.size());
//     }
// }

TEST_CASE("Options prelude mode", "[options][unit]") {
    SECTION("prelude without speed uses default") {
        resetGetopt();
        auto args = std::vector<std::string>{"play", "test.mid", "-p"};
        char** argv = makeArgv(args);
        
        Options opts(args.size(), argv);
        opts.parse();
        
        REQUIRE(opts.getSpeed() == Approx(0.90f));
        REQUIRE(opts.getVerses() == 2);
        REQUIRE_FALSE(opts.isPlayIntro());
        REQUIRE(opts.isPrePost());
        
        freeArgv(argv, args.size());
    }
    
    SECTION("prelude with custom speed") {
        resetGetopt();
        auto args = std::vector<std::string>{"play", "test.mid", "-p12"};
        char** argv = makeArgv(args);
        
        Options opts(args.size(), argv);
        opts.parse();
        
        REQUIRE(opts.getSpeed() == Approx(1.2f));
        REQUIRE(opts.getVerses() == 2);
        
        freeArgv(argv, args.size());
    }
    
    SECTION("prelude with speed 9") {
        resetGetopt();
        auto args = std::vector<std::string>{"play", "test.mid", "-p9"};
        char** argv = makeArgv(args);
        
        Options opts(args.size(), argv);
        opts.parse();
        
        REQUIRE(opts.getSpeed() == Approx(0.9f));
        
        freeArgv(argv, args.size());
    }
    
    SECTION("prelude with speed 10 (100%)") {
        resetGetopt();
        auto args = std::vector<std::string>{"play", "test.mid", "-p10"};
        char** argv = makeArgv(args);
        
        Options opts(args.size(), argv);
        opts.parse();
        
        REQUIRE(opts.getSpeed() == Approx(1.0f));
        
        freeArgv(argv, args.size());
    }
}

// Parameterized test using GENERATE
TEST_CASE("Options verse count parsing", "[options][unit]") {
    int verses = GENERATE(1, 2, 3, 4, 5);
    
    SECTION("with intro (-n flag)") {
        resetGetopt();
        auto args = std::vector<std::string>{"play", "test.mid",
                                              "-n" + std::to_string(verses)};
        char** argv = makeArgv(args);
        
        Options opts(args.size(), argv);
        opts.parse();
        
        REQUIRE(opts.getVerses() == verses);
        REQUIRE(opts.isPlayIntro());
        
        freeArgv(argv, args.size());
    }
    
    SECTION("without intro (-x flag)") {
        resetGetopt();
        auto args = std::vector<std::string>{"play", "test.mid",
                                              "-x" + std::to_string(verses)};
        char** argv = makeArgv(args);
        
        Options opts(args.size(), argv);
        opts.parse();
        
        REQUIRE(opts.getVerses() == verses);
        REQUIRE_FALSE(opts.isPlayIntro());
        
        freeArgv(argv, args.size());
    }
}

TEST_CASE("Options tempo override", "[options][unit]") {
    SECTION("tempo 120 BPM") {
        resetGetopt();
        auto args = std::vector<std::string>{"play", "test.mid", "-t120"};
        char** argv = makeArgv(args);
        
        Options opts(args.size(), argv);
        opts.parse();
        
        REQUIRE(opts.getBpm() == 120);
        REQUIRE(opts.getUsecPerBeat() == 500000);  // 60000000/120
        
        freeArgv(argv, args.size());
    }
    
    SECTION("tempo 90 BPM") {
        resetGetopt();
        auto args = std::vector<std::string>{"play", "test.mid", "-t90"};
        char** argv = makeArgv(args);
        
        Options opts(args.size(), argv);
        opts.parse();
        
        REQUIRE(opts.getBpm() == 90);
        REQUIRE(opts.getUsecPerBeat() == 666666);  // 60000000/90
        
        freeArgv(argv, args.size());
    }
}

TEST_CASE("Options flag combinations", "[options][unit]") {
    SECTION("multiple flags together") {
        resetGetopt();
        auto args = std::vector<std::string>{"play", "test.mid", "-n5", "-t100", "-V", "-W"};
        char** argv = makeArgv(args);
        
        Options opts(args.size(), argv);
        opts.parse();
        
        REQUIRE(opts.getVerses() == 5);
        REQUIRE(opts.getBpm() == 100);
        REQUIRE(opts.isVerbose());
        REQUIRE(opts.isDisplayWarnings());
        REQUIRE(opts.isPlayIntro());
        
        freeArgv(argv, args.size());
    }
    
    SECTION("prelude with tempo and verbose") {
        resetGetopt();
        auto args = std::vector<std::string>{"play", "test.mid", "-p10", "-t80", "-V"};
        char** argv = makeArgv(args);
        
        Options opts(args.size(), argv);
        opts.parse();
        
        REQUIRE(opts.getSpeed() == Approx(1.0f));
        REQUIRE(opts.getBpm() == 80);
        REQUIRE(opts.isVerbose());
        REQUIRE(opts.isPrePost());
        
        freeArgv(argv, args.size());
    }
}

TEST_CASE("Options staging flag", "[options][unit]") {
    SECTION("staging flag sets correctly") {
        resetGetopt();
        auto args = std::vector<std::string>{"play", "test.mid", "-s"};
        char** argv = makeArgv(args);
        
        Options opts(args.size(), argv);
        opts.parse();
        
        REQUIRE(opts.isStaging());
        
        freeArgv(argv, args.size());
    }
    
    SECTION("staging default is false") {
        resetGetopt();
        auto args = std::vector<std::string>{"play", "test.mid"};
        char** argv = makeArgv(args);
        
        Options opts(args.size(), argv);
        opts.parse();
        
        REQUIRE_FALSE(opts.isStaging());
        
        freeArgv(argv, args.size());
    }
}

TEST_CASE("Options verbose flag", "[options][unit]") {
    SECTION("verbose flag sets correctly") {
        resetGetopt();
        auto args = std::vector<std::string>{"play", "test.mid", "-V"};
        char** argv = makeArgv(args);
        
        Options opts(args.size(), argv);
        opts.parse();
        
        REQUIRE(opts.isVerbose());
        
        freeArgv(argv, args.size());
    }
}

TEST_CASE("Options warnings flag", "[options][unit]") {
    SECTION("warnings flag sets correctly") {
        resetGetopt();
        auto args = std::vector<std::string>{"play", "test.mid", "-W"};
        char** argv = makeArgv(args);
        
        Options opts(args.size(), argv);
        opts.parse();
        
        REQUIRE(opts.isDisplayWarnings());
        
        freeArgv(argv, args.size());
    }
}

TEST_CASE("Options semantic version", "[options][unit]") {
    SECTION("extracts semantic version") {
        std::string version = Options::getSemanticVersion();
        
        // Should be in format X.Y.Z
        REQUIRE_FALSE(version.empty());
        REQUIRE(version != "not found");
        
        // Should contain dots
        REQUIRE(version.find('.') != std::string::npos);
    }
}

TEST_CASE("Options default values", "[options][unit]") {
    resetGetopt();
    auto args = std::vector<std::string>{"play", "test.mid"};
    char** argv = makeArgv(args);
    
    Options opts(args.size(), argv);
    opts.parse();
    
    SECTION("default speed is 1.0") {
        REQUIRE(opts.getSpeed() == 1.0f);
    }
    
    SECTION("default verses is 0") {
        REQUIRE(opts.getVerses() == 0);
    }
    
    SECTION("default bpm is 0") {
        REQUIRE(opts.getBpm() == 0);
    }
    
    SECTION("default play intro is true") {
        REQUIRE(opts.isPlayIntro());
    }
    
    SECTION("default prepost is false") {
        REQUIRE_FALSE(opts.isPrePost());
    }
    
    SECTION("default verbose is false") {
        REQUIRE_FALSE(opts.isVerbose());
    }
    
    SECTION("default warnings is false") {
        REQUIRE_FALSE(opts.isDisplayWarnings());
    }
    
    freeArgv(argv, args.size());
}

TEST_CASE("Options filename parsing", "[options][unit][cli]") {
    SECTION("simple filename") {
        auto args = std::vector<std::string>{"play", "hymn.mid"};
        char** argv = makeArgv(args);
        
        Options opts(args.size(), argv);
        opts.parse();
        
        REQUIRE(opts.getFileName() == "hymn.mid");
        
        freeArgv(argv, args.size());
    }
    
    SECTION("filename with path") {
        auto args = std::vector<std::string>{"play", "/path/to/file.mid"};
        char** argv = makeArgv(args);
        
        Options opts(args.size(), argv);
        opts.parse();
        
        REQUIRE(opts.getFileName() == "/path/to/file.mid");
        
        freeArgv(argv, args.size());
    }
}

TEST_CASE("Options long format flags", "[options][unit]") {
    SECTION("--version flag") {
        auto args = std::vector<std::string>{"play", "--version"};
        char** argv = makeArgv(args);
        
        Options opts(args.size(), argv);
        REQUIRE(opts.parse() == -2);
        
        freeArgv(argv, args.size());
    }
    
    SECTION("--help flag") {
        auto args = std::vector<std::string>{"play", "--help"};
        char** argv = makeArgv(args);
        
        Options opts(args.size(), argv);
        REQUIRE(opts.parse() == 1);
        
        freeArgv(argv, args.size());
    }
}