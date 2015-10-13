/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "cmVariableRequiresCommand.h"
#include "cmState.h"

// cmLibraryCommand
bool cmVariableRequiresCommand
::InitialPass(std::vector<std::string>const& args, cmExecutionStatus &)
{
  if(this->Disallowed(cmPolicies::CMP0035,
      "The variable_requires command should not be called; see CMP0035."))
    { return true; }
  if(args.size() < 3 )
    {
    this->SetError("called with incorrect number of arguments");
    return false;
    }

  std::string testVariable = args[0];
  if(!this->Makefile->IsOn(testVariable))
    {
    return true;
    }
  std::string resultVariable = args[1];
  bool requirementsMet = true;
  std::string notSet;
  bool hasAdvanced = false;
  cmState* state = this->Makefile->GetState();
  for(unsigned int i = 2; i < args.size(); ++i)
    {
    if(!this->Makefile->IsOn(args[i]))
      {
      requirementsMet = false;
      notSet += args[i];
      notSet += "\n";
      if(state->GetCacheEntryValue(args[i]) &&
          state->GetCacheEntryPropertyAsBool(args[i], "ADVANCED"))
        {
        hasAdvanced = true;
        }
      }
    }
  const char* reqVar = this->Makefile->GetDefinition(resultVariable);
  // if reqVar is unset, then set it to requirementsMet
  // if reqVar is set to true, but requirementsMet is false , then
  // set reqVar to false.
  if(!reqVar || (!requirementsMet && this->Makefile->IsOn(reqVar)))
    {
    this->Makefile->AddDefinition(resultVariable, requirementsMet);
    }

  if(!requirementsMet)
    {
    std::string message = "Variable assertion failed:\n";
    message += testVariable +
      " Requires that the following unset variables are set:\n";
    message += notSet;
    message += "\nPlease set them, or set ";
    message += testVariable + " to false, and re-configure.\n";
    if(hasAdvanced)
      {
      message +=
        "One or more of the required variables is advanced."
        "  To set the variable, you must turn on advanced mode in cmake.";
      }
    cmSystemTools::Error(message.c_str());
    }

  return true;
}
