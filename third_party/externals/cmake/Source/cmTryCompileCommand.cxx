/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "cmTryCompileCommand.h"

// cmTryCompileCommand
bool cmTryCompileCommand
::InitialPass(std::vector<std::string> const& argv, cmExecutionStatus &)
{
  if(argv.size() < 3)
    {
    return false;
    }

  if(this->Makefile->GetCMakeInstance()->GetWorkingMode() ==
                                                      cmake::FIND_PACKAGE_MODE)
    {
    this->Makefile->IssueMessage(cmake::FATAL_ERROR,
         "The TRY_COMPILE() command is not supported in --find-package mode.");
    return false;
    }

  this->TryCompileCode(argv);

  // if They specified clean then we clean up what we can
  if (this->SrcFileSignature)
    {
    if(!this->Makefile->GetCMakeInstance()->GetDebugTryCompile())
      {
      this->CleanupFiles(this->BinaryDirectory.c_str());
      }
    }
  return true;
}
