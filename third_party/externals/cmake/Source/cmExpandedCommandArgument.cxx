/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2014 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/

#include "cmExpandedCommandArgument.h"

cmExpandedCommandArgument::cmExpandedCommandArgument():
  Quoted(false)
{

}

cmExpandedCommandArgument::cmExpandedCommandArgument(
  std::string const& value, bool quoted):
    Value(value), Quoted(quoted)
{

}

std::string const& cmExpandedCommandArgument::GetValue() const
{
  return this->Value;
}

bool cmExpandedCommandArgument::WasQuoted() const
{
  return this->Quoted;
}

bool cmExpandedCommandArgument::operator== (std::string const& value) const
{
  return this->Value == value;
}

bool cmExpandedCommandArgument::empty() const
{
  return this->Value.empty();
}

const char* cmExpandedCommandArgument::c_str() const
{
  return this->Value.c_str();
}
