#pragma once

#include <cstdint>
#include <cxxmidi/message.hpp>

namespace midiplay {

    class CustomMessage : public cxxmidi::Message {
      public:
        enum Type {
            Private = 0x7D
        };

        enum PrivateType {
            NumberOfVerses = 0x01,
            PauseBetweenVerses = 0x02
        };
    };
}