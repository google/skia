/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2014 Stephen Kelly <steveire@gmail.com>

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "cmTargetSourcesCommand.h"

#include "cmGeneratorExpression.h"

//----------------------------------------------------------------------------
bool cmTargetSourcesCommand
::InitialPass(std::vector<std::string> const& args, cmExecutionStatus &)
{
  return this->HandleArguments(args, "SOURCES");
}

//----------------------------------------------------------------------------
void cmTargetSourcesCommand
::HandleImportedTarget(const std::string &tgt)
{
  std::ostringstream e;
  e << "Cannot specify sources for imported target \""
    << tgt << "\".";
  this->Makefile->IssueMessage(cmake::FATAL_ERROR, e.str());
}

//----------------------------------------------------------------------------
void cmTargetSourcesCommand
::HandleMissingTarget(const std::string &name)
{
  std::ostringstream e;
  e << "Cannot specify sources for target \"" << name << "\" "
       "which is not built by this project.";
  this->Makefile->IssueMessage(cmake::FATAL_ERROR, e.str());
}

//----------------------------------------------------------------------------
std::string cmTargetSourcesCommand
::Join(const std::vector<std::string> &content)
{
  return cmJoin(content, ";");
}

//----------------------------------------------------------------------------
bool cmTargetSourcesCommand
::HandleDirectContent(cmTarget *tgt, const std::vector<std::string> &content,
                      bool, bool)
{
  tgt->AppendProperty("SOURCES", this->Join(content).c_str());
  return true;
}
