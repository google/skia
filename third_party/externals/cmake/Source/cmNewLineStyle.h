/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2011 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmNewLineStyle_h
#define cmNewLineStyle_h

#include "cmStandardIncludes.h"

class cmNewLineStyle
{
public:

  cmNewLineStyle();

  enum Style
  {
    Invalid,
               // LF = '\n', 0x0A, 10
               // CR = '\r', 0x0D, 13
    LF,        // Unix
    CRLF       // Dos
  };

  void SetStyle(Style);
  Style GetStyle() const;

  bool IsValid() const;

  bool ReadFromArguments(const std::vector<std::string>& args,
                         std::string &errorString);

  const std::string GetCharacters() const;

private:
  Style NewLineStyle;
};

#endif
