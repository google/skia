/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "cmDefinePropertyCommand.h"
#include "cmake.h"
#include "cmState.h"

bool cmDefinePropertyCommand
::InitialPass(std::vector<std::string> const& args, cmExecutionStatus &)
{
  if(args.size() < 1)
    {
    this->SetError("called with incorrect number of arguments");
    return false;
    }

  // Get the scope in which to define the property.
  cmProperty::ScopeType scope;
  if(args[0] == "GLOBAL")
    {
    scope = cmProperty::GLOBAL;
    }
  else if(args[0] == "DIRECTORY")
    {
    scope = cmProperty::DIRECTORY;
    }
  else if(args[0] == "TARGET")
    {
    scope = cmProperty::TARGET;
    }
  else if(args[0] == "SOURCE")
    {
    scope = cmProperty::SOURCE_FILE;
    }
  else if(args[0] == "TEST")
    {
    scope = cmProperty::TEST;
    }
  else if(args[0] == "VARIABLE")
    {
    scope = cmProperty::VARIABLE;
    }
  else if (args[0] == "CACHED_VARIABLE")
    {
    scope = cmProperty::CACHED_VARIABLE;
    }
  else
    {
    std::ostringstream e;
    e << "given invalid scope " << args[0] << ".  "
      << "Valid scopes are "
      << "GLOBAL, DIRECTORY, TARGET, SOURCE, "
      << "TEST, VARIABLE, CACHED_VARIABLE.";
    this->SetError(e.str());
    return false;
    }

  // Parse remaining arguments.
  bool inherited = false;
  enum Doing { DoingNone, DoingProperty, DoingBrief, DoingFull };
  Doing doing = DoingNone;
  for(unsigned int i=1; i < args.size(); ++i)
    {
    if(args[i] == "PROPERTY")
      {
      doing = DoingProperty;
      }
    else if(args[i] == "BRIEF_DOCS")
      {
      doing = DoingBrief;
      }
    else if(args[i] == "FULL_DOCS")
      {
      doing = DoingFull;
      }
    else if(args[i] == "INHERITED")
      {
      doing = DoingNone;
      inherited = true;
      }
    else if(doing == DoingProperty)
      {
      doing = DoingNone;
      this->PropertyName = args[i];
      }
    else if(doing == DoingBrief)
      {
      this->BriefDocs += args[i];
      }
    else if(doing == DoingFull)
      {
      this->FullDocs += args[i];
      }
    else
      {
      std::ostringstream e;
      e << "given invalid argument \"" << args[i] << "\".";
      this->SetError(e.str());
      return false;
      }
    }

  // Make sure a property name was found.
  if(this->PropertyName.empty())
    {
    this->SetError("not given a PROPERTY <name> argument.");
    return false;
    }

  // Make sure documentation was given.
  if(this->BriefDocs.empty())
    {
    this->SetError("not given a BRIEF_DOCS <brief-doc> argument.");
    return false;
    }
  if(this->FullDocs.empty())
    {
    this->SetError("not given a FULL_DOCS <full-doc> argument.");
    return false;
    }

  // Actually define the property.
  this->Makefile->GetState()->DefineProperty
    (this->PropertyName, scope,
     this->BriefDocs.c_str(), this->FullDocs.c_str(), inherited);

  return true;
}

