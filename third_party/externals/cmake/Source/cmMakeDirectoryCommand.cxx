/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "cmMakeDirectoryCommand.h"

// cmMakeDirectoryCommand
bool cmMakeDirectoryCommand
::InitialPass(std::vector<std::string> const& args, cmExecutionStatus &)
{
  if(args.size() != 1 )
    {
    this->SetError("called with incorrect number of arguments");
    return false;
    }
    if ( !this->Makefile->CanIWriteThisFile(args[0].c_str()) )
      {
      std::string e = "attempted to create a directory: " + args[0]
        + " into a source directory.";
      this->SetError(e);
      cmSystemTools::SetFatalErrorOccured();
      return false;
      }
  cmSystemTools::MakeDirectory(args[0].c_str());
  return true;
}

