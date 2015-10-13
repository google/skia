/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "cmGetDirectoryPropertyCommand.h"

#include "cmake.h"

// cmGetDirectoryPropertyCommand
bool cmGetDirectoryPropertyCommand
::InitialPass(std::vector<std::string> const& args, cmExecutionStatus &)
{
  if(args.size() < 2 )
    {
    this->SetError("called with incorrect number of arguments");
    return false;
    }

  std::vector<std::string>::const_iterator i = args.begin();
  std::string variable = *i;
  ++i;
  std::string output = "";

  // get the directory argument if there is one
  cmMakefile *dir = this->Makefile;
  if (*i == "DIRECTORY")
    {
    ++i;
    if (i == args.end())
      {
      this->SetError
        ("DIRECTORY argument provided without subsequent arguments");
      return false;
      }
    std::string sd = *i;
    // make sure the start dir is a full path
    if (!cmSystemTools::FileIsFullPath(sd.c_str()))
      {
      sd = this->Makefile->GetCurrentSourceDirectory();
      sd += "/";
      sd += *i;
      }

    // The local generators are associated with collapsed paths.
    sd = cmSystemTools::CollapseFullPath(sd);

    // lookup the makefile from the directory name
    cmLocalGenerator *lg =
      this->Makefile->GetGlobalGenerator()->
      FindLocalGenerator(sd);
    if (!lg)
      {
      this->SetError
        ("DIRECTORY argument provided but requested directory not found. "
         "This could be because the directory argument was invalid or, "
         "it is valid but has not been processed yet.");
      return false;
      }
    dir = lg->GetMakefile();
    ++i;
    }

  // OK, now we have the directory to process, we just get the requested
  // information out of it

  if ( *i == "DEFINITION" )
    {
    ++i;
    if (i == args.end())
      {
      this->SetError("A request for a variable definition was made without "
                     "providing the name of the variable to get.");
      return false;
      }
    output = dir->GetSafeDefinition(*i);
    this->Makefile->AddDefinition(variable, output.c_str());
    return true;
    }

  const char *prop = dir->GetProperty(*i);
  if (prop)
    {
    this->Makefile->AddDefinition(variable, prop);
    return true;
    }
  this->Makefile->AddDefinition(variable, "");
  return true;
}

