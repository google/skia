/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "cmAddDependenciesCommand.h"
#include "cmLocalGenerator.h"
#include "cmGlobalGenerator.h"

// cmDependenciesCommand
bool cmAddDependenciesCommand
::InitialPass(std::vector<std::string> const& args, cmExecutionStatus &)
{
  if(args.size() < 2 )
    {
    this->SetError("called with incorrect number of arguments");
    return false;
    }

  std::string target_name = args[0];
  if(this->Makefile->IsAlias(target_name))
    {
    std::ostringstream e;
    e << "Cannot add target-level dependencies to alias target \""
      << target_name << "\".\n";
    this->Makefile->IssueMessage(cmake::FATAL_ERROR, e.str());
    }
  if(cmTarget* target = this->Makefile->FindTargetToUse(target_name))
    {
    std::vector<std::string>::const_iterator s = args.begin();
    ++s; // skip over target_name
    for (; s != args.end(); ++s)
      {
      target->AddUtility(*s, this->Makefile);
      }
    }
  else
    {
    std::ostringstream e;
    e << "Cannot add target-level dependencies to non-existent target \""
      << target_name << "\".\n"
      << "The add_dependencies works for top-level logical targets created "
      << "by the add_executable, add_library, or add_custom_target commands.  "
      << "If you want to add file-level dependencies see the DEPENDS option "
      << "of the add_custom_target and add_custom_command commands.";
    this->Makefile->IssueMessage(cmake::FATAL_ERROR, e.str());
    }

  return true;
}

