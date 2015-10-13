/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2013 Stephen Kelly <steveire@gmail.com>

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "cmTargetCompileOptionsCommand.h"

#include "cmAlgorithms.h"

bool cmTargetCompileOptionsCommand
::InitialPass(std::vector<std::string> const& args, cmExecutionStatus &)
{
  return this->HandleArguments(args, "COMPILE_OPTIONS", PROCESS_BEFORE);
}

void cmTargetCompileOptionsCommand
::HandleImportedTarget(const std::string &tgt)
{
  std::ostringstream e;
  e << "Cannot specify compile options for imported target \""
    << tgt << "\".";
  this->Makefile->IssueMessage(cmake::FATAL_ERROR, e.str());
}

void cmTargetCompileOptionsCommand
::HandleMissingTarget(const std::string &name)
{
  std::ostringstream e;
  e << "Cannot specify compile options for target \"" << name << "\" "
       "which is not built by this project.";
  this->Makefile->IssueMessage(cmake::FATAL_ERROR, e.str());
}

//----------------------------------------------------------------------------
std::string cmTargetCompileOptionsCommand
::Join(const std::vector<std::string> &content)
{
  return cmJoin(content, ";");
}

//----------------------------------------------------------------------------
bool cmTargetCompileOptionsCommand
::HandleDirectContent(cmTarget *tgt, const std::vector<std::string> &content,
                                   bool, bool)
{
  cmListFileBacktrace lfbt = this->Makefile->GetBacktrace();
  cmValueWithOrigin entry(this->Join(content), lfbt);
  tgt->InsertCompileOption(entry);
  return true;
}
