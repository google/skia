/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "cmInstallTargetsCommand.h"

// cmExecutableCommand
bool cmInstallTargetsCommand
::InitialPass(std::vector<std::string> const& args, cmExecutionStatus &)
{
  if(args.size() < 2 )
    {
    this->SetError("called with incorrect number of arguments");
    return false;
    }

  // Enable the install target.
  this->Makefile->GetGlobalGenerator()->EnableInstallTarget();

  cmTargets &tgts = this->Makefile->GetTargets();
  std::vector<std::string>::const_iterator s = args.begin();
  ++s;
  std::string runtime_dir = "/bin";
  for (;s != args.end(); ++s)
    {
    if (*s == "RUNTIME_DIRECTORY")
      {
      ++s;
      if ( s == args.end() )
        {
        this->SetError("called with RUNTIME_DIRECTORY but no actual "
                       "directory");
        return false;
        }

      runtime_dir = *s;
      }
    else if (tgts.find(*s) != tgts.end())
      {
      tgts[*s].SetInstallPath(args[0].c_str());
      tgts[*s].SetRuntimeInstallPath(runtime_dir.c_str());
      tgts[*s].SetHaveInstallRule(true);
      }
    else
      {
      std::string str = "Cannot find target: \"" + *s + "\" to install.";
      this->SetError(str);
      return false;
      }
    }

  this->Makefile->GetGlobalGenerator()
                       ->AddInstallComponent(this->Makefile->GetSafeDefinition(
                                      "CMAKE_INSTALL_DEFAULT_COMPONENT_NAME"));

  return true;
}

