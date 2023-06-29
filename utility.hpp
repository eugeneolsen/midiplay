#pragma once

#include <cctype>

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
