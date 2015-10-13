/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "cmBuildCommand.h"

#include "cmLocalGenerator.h"
#include "cmGlobalGenerator.h"

//----------------------------------------------------------------------
bool cmBuildCommand
::InitialPass(std::vector<std::string> const& args, cmExecutionStatus &)
{
  // Support the legacy signature of the command:
  //
  if(2 == args.size())
    {
    return this->TwoArgsSignature(args);
    }

  return this->MainSignature(args);
}

//----------------------------------------------------------------------
bool cmBuildCommand
::MainSignature(std::vector<std::string> const& args)
{
  if(args.size() < 1)
    {
    this->SetError("requires at least one argument naming a CMake variable");
    return false;
    }

  // The cmake variable in which to store the result.
  const char* variable = args[0].c_str();

  // Parse remaining arguments.
  const char* configuration = 0;
  const char* project_name = 0;
  std::string target;
  enum Doing { DoingNone, DoingConfiguration, DoingProjectName, DoingTarget };
  Doing doing = DoingNone;
  for(unsigned int i=1; i < args.size(); ++i)
    {
    if(args[i] == "CONFIGURATION")
      {
      doing = DoingConfiguration;
      }
    else if(args[i] == "PROJECT_NAME")
      {
      doing = DoingProjectName;
      }
    else if(args[i] == "TARGET")
      {
      doing = DoingTarget;
      }
    else if(doing == DoingConfiguration)
      {
      doing = DoingNone;
      configuration = args[i].c_str();
      }
    else if(doing == DoingProjectName)
      {
      doing = DoingNone;
      project_name = args[i].c_str();
      }
    else if(doing == DoingTarget)
      {
      doing = DoingNone;
      target = args[i];
      }
    else
      {
      std::ostringstream e;
      e << "unknown argument \"" << args[i] << "\"";
      this->SetError(e.str());
      return false;
      }
    }

  // If null/empty CONFIGURATION argument, cmake --build uses 'Debug'
  // in the currently implemented multi-configuration global generators...
  // so we put this code here to end up with the same default configuration
  // as the original 2-arg build_command signature:
  //
  if(!configuration || !*configuration)
    {
    configuration = getenv("CMAKE_CONFIG_TYPE");
    }
  if(!configuration || !*configuration)
    {
    configuration = "Release";
    }

  if(project_name && *project_name)
    {
    this->Makefile->IssueMessage(cmake::AUTHOR_WARNING,
      "Ignoring PROJECT_NAME option because it has no effect.");
    }

  std::string makecommand = this->Makefile->GetGlobalGenerator()
    ->GenerateCMakeBuildCommand(target, configuration, "",
                                this->Makefile->IgnoreErrorsCMP0061());

  this->Makefile->AddDefinition(variable, makecommand.c_str());

  return true;
}

//----------------------------------------------------------------------
bool cmBuildCommand
::TwoArgsSignature(std::vector<std::string> const& args)
{
  if(args.size() < 2 )
    {
    this->SetError("called with less than two arguments");
    return false;
    }

  const char* define = args[0].c_str();
  const char* cacheValue
    = this->Makefile->GetDefinition(define);

  std::string configType = "Release";
  const char* cfg = getenv("CMAKE_CONFIG_TYPE");
  if ( cfg && *cfg )
    {
    configType = cfg;
    }

  std::string makecommand = this->Makefile->GetGlobalGenerator()
    ->GenerateCMakeBuildCommand("", configType, "",
                                this->Makefile->IgnoreErrorsCMP0061());

  if(cacheValue)
    {
    return true;
    }
  this->Makefile->AddCacheDefinition(define,
                                 makecommand.c_str(),
                                 "Command used to build entire project "
                                 "from the command line.",
                                 cmState::STRING);
  return true;
}
