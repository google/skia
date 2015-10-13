/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "cmGetTestPropertyCommand.h"

#include "cmake.h"
#include "cmTest.h"

// cmGetTestPropertyCommand
bool cmGetTestPropertyCommand
::InitialPass(std::vector<std::string> const& args, cmExecutionStatus &)
{
  if(args.size() < 3 )
    {
    this->SetError("called with incorrect number of arguments");
    return false;
    }

  std::string testName = args[0];
  std::string var = args[2];
  cmTest *test = this->Makefile->GetTest(testName);
  if (test)
    {
    const char *prop = test->GetProperty(args[1]);
    if (prop)
      {
      this->Makefile->AddDefinition(var, prop);
      return true;
      }
    }
  this->Makefile->AddDefinition(var, "NOTFOUND");
  return true;
}

