/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "cmSubdirCommand.h"

// cmSubdirCommand
bool cmSubdirCommand
::InitialPass(std::vector<std::string> const& args, cmExecutionStatus &)
{
  if(args.size() < 1 )
    {
    this->SetError("called with incorrect number of arguments");
    return false;
    }
  bool res = true;
  bool excludeFromAll = false;

  for(std::vector<std::string>::const_iterator i = args.begin();
      i != args.end(); ++i)
    {
    if(*i == "EXCLUDE_FROM_ALL")
      {
      excludeFromAll = true;
      continue;
      }
    if(*i == "PREORDER")
      {
      // Ignored
      continue;
      }

    // if they specified a relative path then compute the full
    std::string srcPath =
      std::string(this->Makefile->GetCurrentSourceDirectory()) +
        "/" + i->c_str();
    if (cmSystemTools::FileIsDirectory(srcPath))
      {
      std::string binPath =
        std::string(this->Makefile->GetCurrentBinaryDirectory()) +
        "/" + i->c_str();
      this->Makefile->AddSubDirectory(srcPath, binPath,
                                  excludeFromAll, false);
      }
    // otherwise it is a full path
    else if ( cmSystemTools::FileIsDirectory(*i) )
      {
      // we must compute the binPath from the srcPath, we just take the last
      // element from the source path and use that
      std::string binPath =
        std::string(this->Makefile->GetCurrentBinaryDirectory()) +
        "/" + cmSystemTools::GetFilenameName(*i);
      this->Makefile->AddSubDirectory(*i, binPath,
                                  excludeFromAll, false);
      }
    else
      {
      std::string error = "Incorrect SUBDIRS command. Directory: ";
      error += *i + " does not exist.";
      this->SetError(error);
      res = false;
      }
    }
  return res;
}

