/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2014 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmExpandedCommandArgument_h
#define cmExpandedCommandArgument_h

#include "cmStandardIncludes.h"

/** \class cmExpandedCommandArgument
 * \brief Represents an expanded command argument
 *
 * cmCommandArgument stores a string representing an expanded
 * command argument and context information.
 */

class cmExpandedCommandArgument
{
public:
  cmExpandedCommandArgument();
  cmExpandedCommandArgument(std::string const& value, bool quoted);

  std::string const& GetValue() const;

  bool WasQuoted() const;

  bool operator== (std::string const& value) const;

  bool empty() const;

  const char* c_str() const;

private:
  std::string Value;
  bool Quoted;
};

#endif
