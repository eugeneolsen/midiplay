#pragma once

#include <cxxmidi/output/default.hpp>
#include <cxxmidi/message.hpp>
#include <cxxmidi/event.hpp>

#include <cxxmidi/channel.hpp>

class protege
{
    private:
        cxxmidi::output::Default& _outport;

    public:
        protege(cxxmidi::output::Default& outport) : _outport(outport)
        {
        }

        void SetDefaults()
        {
            //Event e = Event(Channel8 | Message::kProgramChange, 0);     // General Setting 1
            //_outport.SendMessage(&e);
        }
};
