#pragma once

#include <string>
#include <cctype>
#include <iostream>
#include <iomanip>

#include <boost/filesystem.hpp>

#include <cxxmidi/event.hpp>
#include <cxxmidi/message.hpp>

namespace fs = boost::filesystem;


bool isNumeric(const char *str)
{
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


std::string getFullPath(std::string fileName)
{
    fs::path home = getenv("HOME");
    fs::path dir = ("/Music/midihymns");
    std::string extension = ".mid";
    if (fileName.length() < extension.length())
    {
      fileName += extension;
    }

    std::string filenameEnd = fileName.substr(fileName.length() - extension.length(), extension.length());
    if (filenameEnd != extension)
    {
      fileName += extension;
    }

    fs::path file = (fileName);

    fs::path path = home / dir / file;

    return path.string();
}


void dumpEvent(cxxmidi::Event &event)
{
    cxxmidi::Message message = event;

        std::cout << "Event " << std::dec << event.Dt() << " ";

        for (cxxmidi::Message::iterator iter = message.begin(); iter < message.end(); iter++)
        {
            uint8_t byte = *iter;
            std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(byte);
        }

        std::cout << std::endl;
}