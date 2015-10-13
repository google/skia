/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "cmIncludeRegularExpressionCommand.h"

// cmIncludeRegularExpressionCommand
bool cmIncludeRegularExpressionCommand
::InitialPass(std::vector<std::string> const& args, cmExecutionStatus &)
{
  if((args.size() < 1) || (args.size() > 2))
    {
    this->SetError("called with incorrect number of arguments");
    return false;
    }
  this->Makefile->SetIncludeRegularExpression(args[0].c_str());

  if(args.size() > 1)
    {
    this->Makefile->SetComplainRegularExpression(args[1]);
    }

  return true;
}

