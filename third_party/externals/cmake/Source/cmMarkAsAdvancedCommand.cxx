/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "cmMarkAsAdvancedCommand.h"

// cmMarkAsAdvancedCommand
bool cmMarkAsAdvancedCommand
::InitialPass(std::vector<std::string> const& args, cmExecutionStatus &)
{
  if(args.size() < 1 )
    {
    this->SetError("called with incorrect number of arguments");
    return false;
    }

  unsigned int i =0;
  const char* value = "1";
  bool overwrite = false;
  if(args[0] == "CLEAR" || args[0] == "FORCE")
    {
    overwrite = true;
    if(args[0] == "CLEAR")
      {
      value = "0";
      }
    i = 1;
    }
  for(; i < args.size(); ++i)
    {
    std::string variable = args[i];
    cmState* state = this->Makefile->GetState();
    if (!state->GetCacheEntryValue(variable))
      {
      state->AddCacheEntry(variable, 0, 0, cmState::UNINITIALIZED);
      overwrite = true;
      }
    if (!state->GetCacheEntryValue(variable))
      {
      cmSystemTools::Error("This should never happen...");
      return false;
      }
    if (!state->GetCacheEntryProperty(variable, "ADVANCED") || overwrite)
      {
      state->SetCacheEntryProperty(variable, "ADVANCED", value);
      }
    }
  return true;
}

