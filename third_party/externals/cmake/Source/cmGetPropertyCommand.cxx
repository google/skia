/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "cmGetPropertyCommand.h"

#include "cmake.h"
#include "cmState.h"
#include "cmTest.h"
#include "cmGlobalGenerator.h"
#include "cmLocalGenerator.h"
#include "cmSourceFile.h"
#include "cmPropertyDefinition.h"

//----------------------------------------------------------------------------
cmGetPropertyCommand::cmGetPropertyCommand()
{
  this->InfoType = OutValue;
}

//----------------------------------------------------------------------------
bool cmGetPropertyCommand
::InitialPass(std::vector<std::string> const& args, cmExecutionStatus &)
{
  if(args.size() < 3 )
    {
    this->SetError("called with incorrect number of arguments");
    return false;
    }

  // The cmake variable in which to store the result.
  this->Variable = args[0];

  // Get the scope from which to get the property.
  cmProperty::ScopeType scope;
  if(args[1] == "GLOBAL")
    {
    scope = cmProperty::GLOBAL;
    }
  else if(args[1] == "DIRECTORY")
    {
    scope = cmProperty::DIRECTORY;
    }
  else if(args[1] == "TARGET")
    {
    scope = cmProperty::TARGET;
    }
  else if(args[1] == "SOURCE")
    {
    scope = cmProperty::SOURCE_FILE;
    }
  else if(args[1] == "TEST")
    {
    scope = cmProperty::TEST;
    }
  else if(args[1] == "VARIABLE")
    {
    scope = cmProperty::VARIABLE;
    }
  else if(args[1] == "CACHE")
    {
    scope = cmProperty::CACHE;
    }
  else if(args[1] == "INSTALL")
    {
    scope = cmProperty::INSTALL;
    }
  else
    {
    std::ostringstream e;
    e << "given invalid scope " << args[1] << ".  "
      << "Valid scopes are "
      << "GLOBAL, DIRECTORY, TARGET, SOURCE, TEST, VARIABLE, CACHE, INSTALL.";
    this->SetError(e.str());
    return false;
    }

  // Parse remaining arguments.
  enum Doing { DoingNone, DoingName, DoingProperty, DoingType };
  Doing doing = DoingName;
  for(unsigned int i=2; i < args.size(); ++i)
    {
    if(args[i] == "PROPERTY")
      {
      doing = DoingProperty;
      }
    else if(args[i] == "BRIEF_DOCS")
      {
      doing = DoingNone;
      this->InfoType = OutBriefDoc;
      }
    else if(args[i] == "FULL_DOCS")
      {
      doing = DoingNone;
      this->InfoType = OutFullDoc;
      }
    else if(args[i] == "SET")
      {
      doing = DoingNone;
      this->InfoType = OutSet;
      }
    else if(args[i] == "DEFINED")
      {
      doing = DoingNone;
      this->InfoType = OutDefined;
      }
    else if(doing == DoingName)
      {
      doing = DoingNone;
      this->Name = args[i];
      }
    else if(doing == DoingProperty)
      {
      doing = DoingNone;
      this->PropertyName = args[i];
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

  // Compute requested output.
  if(this->InfoType == OutBriefDoc)
    {
    // Lookup brief documentation.
    std::string output;
    if(cmPropertyDefinition* def =
       this->Makefile->GetState()->
       GetPropertyDefinition(this->PropertyName, scope))
      {
      output = def->GetShortDescription();
      }
    else
      {
      output = "NOTFOUND";
      }
    this->Makefile->AddDefinition(this->Variable, output.c_str());
    }
  else if(this->InfoType == OutFullDoc)
    {
    // Lookup full documentation.
    std::string output;
    if(cmPropertyDefinition* def =
       this->Makefile->GetState()->
       GetPropertyDefinition(this->PropertyName, scope))
      {
      output = def->GetFullDescription();
      }
    else
      {
      output = "NOTFOUND";
      }
    this->Makefile->AddDefinition(this->Variable, output.c_str());
    }
  else if(this->InfoType == OutDefined)
    {
    // Lookup if the property is defined
    if(this->Makefile->GetState()->
       GetPropertyDefinition(this->PropertyName, scope))
      {
      this->Makefile->AddDefinition(this->Variable, "1");
      }
    else
      {
      this->Makefile->AddDefinition(this->Variable, "0");
      }
    }
  else
    {
    // Dispatch property getting.
    switch(scope)
      {
      case cmProperty::GLOBAL:      return this->HandleGlobalMode();
      case cmProperty::DIRECTORY:   return this->HandleDirectoryMode();
      case cmProperty::TARGET:      return this->HandleTargetMode();
      case cmProperty::SOURCE_FILE: return this->HandleSourceMode();
      case cmProperty::TEST:        return this->HandleTestMode();
      case cmProperty::VARIABLE:    return this->HandleVariableMode();
      case cmProperty::CACHE:       return this->HandleCacheMode();
      case cmProperty::INSTALL:       return this->HandleInstallMode();

      case cmProperty::CACHED_VARIABLE:
        break; // should never happen
      }
    }

  return true;
}

//----------------------------------------------------------------------------
bool cmGetPropertyCommand::StoreResult(const char* value)
{
  if(this->InfoType == OutSet)
    {
    this->Makefile->AddDefinition(this->Variable, value? "1":"0");
    }
  else // if(this->InfoType == OutValue)
    {
    if(value)
      {
      this->Makefile->AddDefinition(this->Variable, value);
      }
    else
      {
      this->Makefile->RemoveDefinition(this->Variable);
      }
    }
  return true;
}

//----------------------------------------------------------------------------
bool cmGetPropertyCommand::HandleGlobalMode()
{
  if(!this->Name.empty())
    {
    this->SetError("given name for GLOBAL scope.");
    return false;
    }

  // Get the property.
  cmake* cm = this->Makefile->GetCMakeInstance();
  return this->StoreResult(cm->GetState()
             ->GetGlobalProperty(this->PropertyName));
}

//----------------------------------------------------------------------------
bool cmGetPropertyCommand::HandleDirectoryMode()
{
  // Default to the current directory.
  cmMakefile* mf = this->Makefile;

  // Lookup the directory if given.
  if(!this->Name.empty())
    {
    // Construct the directory name.  Interpret relative paths with
    // respect to the current directory.
    std::string dir = this->Name;
    if(!cmSystemTools::FileIsFullPath(dir.c_str()))
      {
      dir = this->Makefile->GetCurrentSourceDirectory();
      dir += "/";
      dir += this->Name;
      }

    // The local generators are associated with collapsed paths.
    dir = cmSystemTools::CollapseFullPath(dir);

    // Lookup the generator.
    if(cmLocalGenerator* lg =
       (this->Makefile->GetGlobalGenerator()->FindLocalGenerator(dir)))
      {
      // Use the makefile for the directory found.
      mf = lg->GetMakefile();
      }
    else
      {
      // Could not find the directory.
      this->SetError
        ("DIRECTORY scope provided but requested directory was not found. "
         "This could be because the directory argument was invalid or, "
         "it is valid but has not been processed yet.");
      return false;
      }
    }

  // Get the property.
  return this->StoreResult(mf->GetProperty(this->PropertyName));
}

//----------------------------------------------------------------------------
bool cmGetPropertyCommand::HandleTargetMode()
{
  if(this->Name.empty())
    {
    this->SetError("not given name for TARGET scope.");
    return false;
    }

  if(this->PropertyName == "ALIASED_TARGET")
    {
    if(this->Makefile->IsAlias(this->Name))
      {
      if(cmTarget* target =
                          this->Makefile->FindTargetToUse(this->Name))
        {
        return this->StoreResult(target->GetName().c_str());
        }
      }
    return this->StoreResult((this->Variable + "-NOTFOUND").c_str());
    }
  if(cmTarget* target = this->Makefile->FindTargetToUse(this->Name))
    {
    return this->StoreResult(target->GetProperty(this->PropertyName,
                                                 this->Makefile));
    }
  else
    {
    std::ostringstream e;
    e << "could not find TARGET " << this->Name
      << ".  Perhaps it has not yet been created.";
    this->SetError(e.str());
    return false;
    }
}

//----------------------------------------------------------------------------
bool cmGetPropertyCommand::HandleSourceMode()
{
  if(this->Name.empty())
    {
    this->SetError("not given name for SOURCE scope.");
    return false;
    }

  // Get the source file.
  if(cmSourceFile* sf =
     this->Makefile->GetOrCreateSource(this->Name))
    {
    return
      this->StoreResult(sf->GetPropertyForUser(this->PropertyName));
    }
  else
    {
    std::ostringstream e;
    e << "given SOURCE name that could not be found or created: "
      << this->Name;
    this->SetError(e.str());
    return false;
    }
}

//----------------------------------------------------------------------------
bool cmGetPropertyCommand::HandleTestMode()
{
  if(this->Name.empty())
    {
    this->SetError("not given name for TEST scope.");
    return false;
    }

  // Loop over all tests looking for matching names.
  if(cmTest* test = this->Makefile->GetTest(this->Name))
    {
    return this->StoreResult(test->GetProperty(this->PropertyName));
    }

  // If not found it is an error.
  std::ostringstream e;
  e << "given TEST name that does not exist: " << this->Name;
  this->SetError(e.str());
  return false;
}

//----------------------------------------------------------------------------
bool cmGetPropertyCommand::HandleVariableMode()
{
  if(!this->Name.empty())
    {
    this->SetError("given name for VARIABLE scope.");
    return false;
    }

  return this->StoreResult
    (this->Makefile->GetDefinition(this->PropertyName));
}

//----------------------------------------------------------------------------
bool cmGetPropertyCommand::HandleCacheMode()
{
  if(this->Name.empty())
    {
    this->SetError("not given name for CACHE scope.");
    return false;
    }

  const char* value = 0;
  if(this->Makefile->GetState()->GetCacheEntryValue(this->Name))
    {
    value = this->Makefile->GetState()
                ->GetCacheEntryProperty(this->Name, this->PropertyName);
    }
  this->StoreResult(value);
  return true;
}

//----------------------------------------------------------------------------
bool cmGetPropertyCommand::HandleInstallMode()
{
  if(this->Name.empty())
    {
    this->SetError("not given name for INSTALL scope.");
    return false;
    }

  // Get the installed file.
  cmake* cm = this->Makefile->GetCMakeInstance();

  if(cmInstalledFile* file = cm->GetOrCreateInstalledFile(
    this->Makefile, this->Name))
    {
    std::string value;
    bool isSet = file->GetProperty(this->PropertyName, value);

    return this->StoreResult(isSet ? value.c_str() : 0);
    }
  else
    {
    std::ostringstream e;
    e << "given INSTALL name that could not be found or created: "
      << this->Name;
    this->SetError(e.str());
    return false;
    }
}
