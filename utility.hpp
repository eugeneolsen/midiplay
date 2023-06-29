#pragma once

#include <string>
#include <cctype>

#include <boost/filesystem.hpp>

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