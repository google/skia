/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2014 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "cmContinueCommand.h"

// cmContinueCommand
bool cmContinueCommand::InitialPass(std::vector<std::string> const &args,
                                  cmExecutionStatus &status)
{
  if(!this->Makefile->IsLoopBlock())
    {
    this->Makefile->IssueMessage(cmake::FATAL_ERROR,
                                 "A CONTINUE command was found outside of a "
                                 "proper FOREACH or WHILE loop scope.");
    cmSystemTools::SetFatalErrorOccured();
    return true;
    }

  status.SetContinueInvoked(true);

  if(!args.empty())
    {
    this->Makefile->IssueMessage(cmake::FATAL_ERROR,
                                 "The CONTINUE command does not accept any "
                                 "arguments.");
    cmSystemTools::SetFatalErrorOccured();
    return true;
    }

  return true;
}
