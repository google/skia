/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "cmGetCMakePropertyCommand.h"

#include "cmGlobalGenerator.h"
#include "cmLocalGenerator.h"
#include "cmake.h"
#include "cmState.h"
#include "cmAlgorithms.h"

// cmGetCMakePropertyCommand
bool cmGetCMakePropertyCommand
::InitialPass(std::vector<std::string> const& args, cmExecutionStatus &)
{
  if(args.size() < 2 )
    {
    this->SetError("called with incorrect number of arguments");
    return false;
    }

  std::string variable = args[0];
  std::string output = "NOTFOUND";

  if ( args[1] == "VARIABLES" )
    {
    int cacheonly = 0;
    std::vector<std::string> vars = this->Makefile->GetDefinitions(cacheonly);
    if (!vars.empty())
      {
      output = cmJoin(vars, ";");
      }
    }
  else if ( args[1] == "MACROS" )
    {
    output.clear();
    this->Makefile->GetListOfMacros(output);
    }
  else if ( args[1] == "COMPONENTS" )
    {
    const std::set<std::string>* components
      = this->Makefile->GetGlobalGenerator()->GetInstallComponents();
    output = cmJoin(*components, ";");
    }
  else
    {
    const char *prop =
      this->Makefile->GetState()
          ->GetGlobalProperty(args[1]);
    if (prop)
      {
      output = prop;
      }
    }

  this->Makefile->AddDefinition(variable, output.c_str());

  return true;
}
