#include <cxxmidi/output/default.hpp>
#include <cxxmidi/message.hpp>
#include <cxxmidi/event.hpp>

#include <ecomidi/channel.hpp>

#pragma once

using namespace cxxmidi;

class protege
{
    private:
        output::Default& _outport;

    public: 
        protege(output::Default& outport) : _outport(outport)
        {
        }

        void SetDefaults()
        {
            //Event e = Event(Channel8 | Message::kProgramChange, 0);     // General Setting 1
            //_outport.SendMessage(&e);
        }
};
