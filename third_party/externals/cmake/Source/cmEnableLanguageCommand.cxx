/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "cmEnableLanguageCommand.h"

// cmEnableLanguageCommand
bool cmEnableLanguageCommand
::InitialPass(std::vector<std::string> const& args, cmExecutionStatus &)
{
  bool optional = false;
  std::vector<std::string> languages;
  if(args.size() < 1 )
    {
    this->SetError
      ("called with incorrect number of arguments");
    return false;
    }
  for (std::vector<std::string>::const_iterator it = args.begin();
       it != args.end();
       ++it)
    {
    if ((*it) == "OPTIONAL")
      {
      optional = true;
      }
    else
      {
      languages.push_back(*it);
      }
    }

  this->Makefile->EnableLanguage(languages, optional);
  return true;
}

