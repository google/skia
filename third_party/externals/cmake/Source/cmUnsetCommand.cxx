/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "cmUnsetCommand.h"

// cmUnsetCommand
bool cmUnsetCommand::InitialPass(std::vector<std::string> const& args,
                                 cmExecutionStatus &)
{
  if(args.size() < 1 || args.size() > 2)
    {
    this->SetError("called with incorrect number of arguments");
    return false;
    }

  const char* variable = args[0].c_str();

  // unset(ENV{VAR})
  if (cmHasLiteralPrefix(variable, "ENV{") && strlen(variable) > 5)
    {
    // what is the variable name
    char *envVarName = new char [strlen(variable)];
    strncpy(envVarName,variable+4,strlen(variable)-5);
    envVarName[strlen(variable)-5] = '\0';

#ifdef CMAKE_BUILD_WITH_CMAKE
    cmSystemTools::UnsetEnv(envVarName);
#endif
    delete[] envVarName;
    return true;
    }
  // unset(VAR)
  else if (args.size() == 1)
    {
    this->Makefile->RemoveDefinition(variable);
    return true;
    }
  // unset(VAR CACHE)
  else if ((args.size() == 2) && (args[1] == "CACHE"))
    {
    this->Makefile->RemoveCacheDefinition(variable);
    return true;
    }
  // unset(VAR PARENT_SCOPE)
  else if ((args.size() == 2) && (args[1] == "PARENT_SCOPE"))
    {
    this->Makefile->RaiseScope(variable, 0);
    return true;
    }
  // ERROR: second argument isn't CACHE or PARENT_SCOPE
  else
    {
    this->SetError("called with an invalid second argument");
    return false;
    }
}

