/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "cmSeparateArgumentsCommand.h"

// cmSeparateArgumentsCommand
bool cmSeparateArgumentsCommand
::InitialPass(std::vector<std::string> const& args, cmExecutionStatus &)
{
  if(args.empty())
    {
    this->SetError("must be given at least one argument.");
    return false;
    }

  std::string var;
  std::string command;
  enum Mode { ModeOld, ModeUnix, ModeWindows };
  Mode mode = ModeOld;
  enum Doing { DoingNone, DoingVariable, DoingMode, DoingCommand };
  Doing doing = DoingVariable;
  for(unsigned int i=0; i < args.size(); ++i)
    {
    if(doing == DoingVariable)
      {
      var = args[i];
      doing = DoingMode;
      }
    else if(doing == DoingMode && args[i] == "UNIX_COMMAND")
      {
      mode = ModeUnix;
      doing = DoingCommand;
      }
    else if(doing == DoingMode && args[i] == "WINDOWS_COMMAND")
      {
      mode = ModeWindows;
      doing = DoingCommand;
      }
    else if(doing == DoingCommand)
      {
      command = args[i];
      doing = DoingNone;
      }
    else
      {
      std::ostringstream e;
      e << "given unknown argument " << args[i];
      this->SetError(e.str());
      return false;
      }
    }

  if(mode == ModeOld)
    {
    // Original space-replacement version of command.
    if(const char* def = this->Makefile->GetDefinition(var))
      {
      std::string value = def;
      cmSystemTools::ReplaceString(value, " ", ";");
      this->Makefile->AddDefinition(var, value.c_str());
      }
    }
  else
    {
    // Parse the command line.
    std::vector<std::string> vec;
    if(mode == ModeUnix)
      {
      cmSystemTools::ParseUnixCommandLine(command.c_str(), vec);
      }
    else // if(mode == ModeWindows)
      {
      cmSystemTools::ParseWindowsCommandLine(command.c_str(), vec);
      }

    // Construct the result list value.
    std::string value;
    const char* sep = "";
    for(std::vector<std::string>::const_iterator vi = vec.begin();
        vi != vec.end(); ++vi)
      {
      // Separate from the previous argument.
      value += sep;
      sep = ";";

      // Preserve semicolons.
      for(std::string::const_iterator si = vi->begin();
          si != vi->end(); ++si)
        {
        if(*si == ';')
          {
          value += '\\';
          }
        value += *si;
        }
      }
    this->Makefile->AddDefinition(var, value.c_str());
    }

  return true;
}
