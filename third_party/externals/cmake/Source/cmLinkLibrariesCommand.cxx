/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "cmLinkLibrariesCommand.h"

// cmLinkLibrariesCommand
bool cmLinkLibrariesCommand
::InitialPass(std::vector<std::string> const& args, cmExecutionStatus &)
{
  if(args.size() < 1 )
    {
    return true;
    }
  // add libraries, nothe that there is an optional prefix
  // of debug and optimized than can be used
  for(std::vector<std::string>::const_iterator i = args.begin();
      i != args.end(); ++i)
    {
    if (*i == "debug")
      {
      ++i;
      if(i == args.end())
        {
        this->SetError("The \"debug\" argument must be followed by "
                       "a library");
        return false;
        }
      this->Makefile->AddLinkLibrary(*i,
                                 cmTarget::DEBUG);
      }
    else if (*i == "optimized")
      {
      ++i;
      if(i == args.end())
        {
        this->SetError("The \"optimized\" argument must be followed by "
                       "a library");
        return false;
        }
      this->Makefile->AddLinkLibrary(*i,
                                 cmTarget::OPTIMIZED);
      }
    else
      {
      this->Makefile->AddLinkLibrary(*i);
      }
    }

  return true;
}

