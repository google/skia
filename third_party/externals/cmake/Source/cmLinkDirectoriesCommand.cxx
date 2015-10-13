/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "cmLinkDirectoriesCommand.h"

// cmLinkDirectoriesCommand
bool cmLinkDirectoriesCommand
::InitialPass(std::vector<std::string> const& args, cmExecutionStatus &)
{
 if(args.size() < 1 )
    {
    return true;
    }

  for(std::vector<std::string>::const_iterator i = args.begin();
      i != args.end(); ++i)
    {
    this->AddLinkDir(*i);
    }
  return true;
}

//----------------------------------------------------------------------------
void cmLinkDirectoriesCommand::AddLinkDir(std::string const& dir)
{
  std::string unixPath = dir;
  cmSystemTools::ConvertToUnixSlashes(unixPath);
  if(!cmSystemTools::FileIsFullPath(unixPath.c_str()))
    {
    bool convertToAbsolute = false;
    std::ostringstream e;
    e << "This command specifies the relative path\n"
      << "  " << unixPath << "\n"
      << "as a link directory.\n";
    switch (this->Makefile->GetPolicyStatus(cmPolicies::CMP0015))
      {
      case cmPolicies::WARN:
        e << cmPolicies::GetPolicyWarning(cmPolicies::CMP0015);
        this->Makefile->IssueMessage(cmake::AUTHOR_WARNING, e.str());
      case cmPolicies::OLD:
        // OLD behavior does not convert
        break;
      case cmPolicies::REQUIRED_IF_USED:
      case cmPolicies::REQUIRED_ALWAYS:
        e << cmPolicies::GetRequiredPolicyError(cmPolicies::CMP0015);
        this->Makefile->IssueMessage(cmake::FATAL_ERROR, e.str());
      case cmPolicies::NEW:
        // NEW behavior converts
        convertToAbsolute = true;
        break;
      }
    if (convertToAbsolute)
      {
      std::string tmp = this->Makefile->GetCurrentSourceDirectory();
      tmp += "/";
      tmp += unixPath;
      unixPath = tmp;
      }
    }
  this->Makefile->AddLinkDirectory(unixPath);
}
