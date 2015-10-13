/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "cmSourceGroupCommand.h"

// cmSourceGroupCommand
bool cmSourceGroupCommand
::InitialPass(std::vector<std::string> const& args, cmExecutionStatus &)
{
  if(args.size() < 1)
    {
    this->SetError("called with incorrect number of arguments");
    return false;
    }

  std::string delimiter = "\\";
  if(this->Makefile->GetDefinition("SOURCE_GROUP_DELIMITER"))
    {
    delimiter = this->Makefile->GetDefinition("SOURCE_GROUP_DELIMITER");
    }

  std::vector<std::string> folders =
    cmSystemTools::tokenize(args[0], delimiter);

  cmSourceGroup* sg = 0;
  sg = this->Makefile->GetSourceGroup(folders);
  if(!sg)
    {
    this->Makefile->AddSourceGroup(folders);
    sg = this->Makefile->GetSourceGroup(folders);
    }

  if(!sg)
    {
    this->SetError("Could not create or find source group");
    return false;
    }
  // If only two arguments are given, the pre-1.8 version of the
  // command is being invoked.
  if(args.size() == 2  && args[1] != "FILES")
    {
    sg->SetGroupRegex(args[1].c_str());
    return true;
    }

  // Process arguments.
  bool doingFiles = false;
  for(unsigned int i=1; i < args.size(); ++i)
    {
    if(args[i] == "REGULAR_EXPRESSION")
      {
      // Next argument must specify the regex.
      if(i+1 < args.size())
        {
        ++i;
        sg->SetGroupRegex(args[i].c_str());
        }
      else
        {
        this->SetError("REGULAR_EXPRESSION argument given without a regex.");
        return false;
        }
      doingFiles = false;
      }
    else if(args[i] == "FILES")
      {
      // Next arguments will specify files.
      doingFiles = true;
      }
    else if(doingFiles)
      {
      // Convert name to full path and add to the group's list.
      std::string src = args[i];
      if(!cmSystemTools::FileIsFullPath(src.c_str()))
        {
        src = this->Makefile->GetCurrentSourceDirectory();
        src += "/";
        src += args[i];
        }
      src = cmSystemTools::CollapseFullPath(src.c_str());
      sg->AddGroupFile(src);
      }
    else
      {
      std::ostringstream err;
      err << "Unknown argument \"" << args[i] << "\".  "
          << "Perhaps the FILES keyword is missing.\n";
      this->SetError(err.str());
      return false;
      }
    }

  return true;
}
