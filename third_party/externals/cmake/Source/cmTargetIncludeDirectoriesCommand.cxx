/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2013 Stephen Kelly <steveire@gmail.com>

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "cmTargetIncludeDirectoriesCommand.h"

#include "cmGeneratorExpression.h"

//----------------------------------------------------------------------------
bool cmTargetIncludeDirectoriesCommand
::InitialPass(std::vector<std::string> const& args, cmExecutionStatus &)
{
  return this->HandleArguments(args, "INCLUDE_DIRECTORIES",
                               ArgumentFlags(PROCESS_BEFORE | PROCESS_SYSTEM));
}

//----------------------------------------------------------------------------
void cmTargetIncludeDirectoriesCommand
::HandleImportedTarget(const std::string &tgt)
{
  std::ostringstream e;
  e << "Cannot specify include directories for imported target \""
    << tgt << "\".";
  this->Makefile->IssueMessage(cmake::FATAL_ERROR, e.str());
}

//----------------------------------------------------------------------------
void cmTargetIncludeDirectoriesCommand
::HandleMissingTarget(const std::string &name)
{
  std::ostringstream e;
  e << "Cannot specify include directories for target \"" << name << "\" "
       "which is not built by this project.";
  this->Makefile->IssueMessage(cmake::FATAL_ERROR, e.str());
}

//----------------------------------------------------------------------------
std::string cmTargetIncludeDirectoriesCommand
::Join(const std::vector<std::string> &content)
{
  std::string dirs;
  std::string sep;
  std::string prefix =
      this->Makefile->GetCurrentSourceDirectory() + std::string("/");
  for(std::vector<std::string>::const_iterator it = content.begin();
    it != content.end(); ++it)
    {
    if (cmSystemTools::FileIsFullPath(it->c_str())
        || cmGeneratorExpression::Find(*it) == 0)
      {
      dirs += sep + *it;
      }
    else
      {
      dirs += sep + prefix + *it;
      }
    sep = ";";
    }
  return dirs;
}

//----------------------------------------------------------------------------
bool cmTargetIncludeDirectoriesCommand
::HandleDirectContent(cmTarget *tgt, const std::vector<std::string> &content,
                      bool prepend, bool system)
{
  cmListFileBacktrace lfbt = this->Makefile->GetBacktrace();
  cmValueWithOrigin entry(this->Join(content), lfbt);
  tgt->InsertInclude(entry, prepend);
  if (system)
    {
    tgt->AddSystemIncludeDirectories(content);
    }
  return true;
}

//----------------------------------------------------------------------------
void cmTargetIncludeDirectoriesCommand
::HandleInterfaceContent(cmTarget *tgt,
                         const std::vector<std::string> &content,
                         bool prepend, bool system)
{
  cmTargetPropCommandBase::HandleInterfaceContent(tgt, content,
                                                  prepend, system);

  if (system)
    {
    std::string joined = cmJoin(content, ";");
    tgt->AppendProperty("INTERFACE_SYSTEM_INCLUDE_DIRECTORIES",
                        joined.c_str());
    }
}
