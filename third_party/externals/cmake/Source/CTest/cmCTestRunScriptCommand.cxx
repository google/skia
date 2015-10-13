/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "cmCTestRunScriptCommand.h"

#include "cmCTestScriptHandler.h"

bool cmCTestRunScriptCommand
::InitialPass(std::vector<std::string> const& args, cmExecutionStatus &)
{
  if(args.size() < 1 )
    {
    this->CTestScriptHandler->RunCurrentScript();
    return true;
    }

  bool np = false;
  unsigned int i = 0;
  if (args[i] == "NEW_PROCESS")
    {
    np = true;
    i++;
    }
  int start = i;
  // run each script
  std::string returnVariable;
  for (i = start; i < args.size(); ++i)
    {
    if(args[i] == "RETURN_VALUE")
      {
      ++i;
      if(i < args.size())
        {
        returnVariable = args[i];
        }
      }
    }
  for (i = start; i < args.size(); ++i)
    {
    if(args[i] == "RETURN_VALUE")
      {
      ++i;
      }
    else
      {
      int ret;
      cmCTestScriptHandler::RunScript(this->CTest, args[i].c_str(), !np,
        &ret);
      std::ostringstream str;
      str << ret;
      this->Makefile->AddDefinition(returnVariable, str.str().c_str());
      }
    }
  return true;
}


