/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2013 Stephen Kelly <steveire@gmail.com>

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/

#include "cmTargetPropCommandBase.h"

#include "cmGlobalGenerator.h"

//----------------------------------------------------------------------------
bool cmTargetPropCommandBase
::HandleArguments(std::vector<std::string> const& args,
                  const std::string& prop,
                  ArgumentFlags flags)
{
  if(args.size() < 2)
    {
    this->SetError("called with incorrect number of arguments");
    return false;
    }

  // Lookup the target for which libraries are specified.
  if (this->Makefile->IsAlias(args[0]))
    {
    this->SetError("can not be used on an ALIAS target.");
    return false;
    }
  this->Target =
    this->Makefile->GetCMakeInstance()
    ->GetGlobalGenerator()->FindTarget(args[0]);
  if(!this->Target)
    {
    this->Target = this->Makefile->FindTargetToUse(args[0]);
    }
  if(!this->Target)
    {
    this->HandleMissingTarget(args[0]);
    return false;
    }
  if ((this->Target->GetType() != cmTarget::SHARED_LIBRARY)
    && (this->Target->GetType() != cmTarget::STATIC_LIBRARY)
    && (this->Target->GetType() != cmTarget::OBJECT_LIBRARY)
    && (this->Target->GetType() != cmTarget::MODULE_LIBRARY)
    && (this->Target->GetType() != cmTarget::INTERFACE_LIBRARY)
    && (this->Target->GetType() != cmTarget::EXECUTABLE))
    {
    this->SetError("called with non-compilable target type");
    return false;
    }

  bool system = false;
  unsigned int argIndex = 1;

  if ((flags & PROCESS_SYSTEM) && args[argIndex] == "SYSTEM")
    {
    if (args.size() < 3)
      {
      this->SetError("called with incorrect number of arguments");
      return false;
      }
    system = true;
    ++argIndex;
    }

  bool prepend = false;
  if ((flags & PROCESS_BEFORE) && args[argIndex] == "BEFORE")
    {
    if (args.size() < 3)
      {
      this->SetError("called with incorrect number of arguments");
      return false;
      }
    prepend = true;
    ++argIndex;
    }

  this->Property = prop;

  while (argIndex < args.size())
    {
    if (!this->ProcessContentArgs(args, argIndex, prepend, system))
      {
      return false;
      }
    }
  return true;
}

//----------------------------------------------------------------------------
bool cmTargetPropCommandBase
::ProcessContentArgs(std::vector<std::string> const& args,
                     unsigned int &argIndex, bool prepend, bool system)
{
  const std::string scope = args[argIndex];

  if(scope != "PUBLIC"
      && scope != "PRIVATE"
      && scope != "INTERFACE" )
    {
    this->SetError("called with invalid arguments");
    return false;
    }

  if(this->Target->IsImported())
    {
    this->HandleImportedTarget(args[0]);
    return false;
    }

  if (this->Target->GetType() == cmTarget::INTERFACE_LIBRARY
      && scope != "INTERFACE")
    {
    this->SetError("may only be set INTERFACE properties on INTERFACE "
      "targets");
    return false;
    }

  ++argIndex;

  std::vector<std::string> content;

  for(unsigned int i=argIndex; i < args.size(); ++i, ++argIndex)
    {
    if(args[i] == "PUBLIC"
        || args[i] == "PRIVATE"
        || args[i] == "INTERFACE" )
      {
      return this->PopulateTargetProperies(scope, content, prepend, system);
      }
    content.push_back(args[i]);
    }
  return this->PopulateTargetProperies(scope, content, prepend, system);
}

//----------------------------------------------------------------------------
bool cmTargetPropCommandBase
::PopulateTargetProperies(const std::string &scope,
                          const std::vector<std::string> &content,
                          bool prepend, bool system)
{
  if (scope == "PRIVATE" || scope == "PUBLIC")
    {
    if (!this->HandleDirectContent(this->Target, content, prepend, system))
      {
      return false;
      }
    }
  if (scope == "INTERFACE" || scope == "PUBLIC")
    {
    this->HandleInterfaceContent(this->Target, content, prepend, system);
    }
  return true;
}

//----------------------------------------------------------------------------
void cmTargetPropCommandBase::HandleInterfaceContent(cmTarget *tgt,
                                  const std::vector<std::string> &content,
                                  bool prepend, bool)
{
  if (prepend)
    {
    const std::string propName = std::string("INTERFACE_") + this->Property;
    const char *propValue = tgt->GetProperty(propName);
    const std::string totalContent = this->Join(content) + (propValue
                                              ? std::string(";") + propValue
                                              : std::string());
    tgt->SetProperty(propName, totalContent.c_str());
    }
  else
    {
    tgt->AppendProperty("INTERFACE_" + this->Property,
                          this->Join(content).c_str());
    }
}
