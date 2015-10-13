/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "cmAddLibraryCommand.h"

#include "cmake.h"
#include "cmState.h"

// cmLibraryCommand
bool cmAddLibraryCommand
::InitialPass(std::vector<std::string> const& args, cmExecutionStatus &)
{
  if(args.size() < 1 )
    {
    this->SetError("called with incorrect number of arguments");
    return false;
    }
  // Library type defaults to value of BUILD_SHARED_LIBS, if it exists,
  // otherwise it defaults to static library.
  cmTarget::TargetType type = cmTarget::SHARED_LIBRARY;
  if (cmSystemTools::IsOff(this->Makefile->GetDefinition("BUILD_SHARED_LIBS")))
    {
    type = cmTarget::STATIC_LIBRARY;
    }
  bool excludeFromAll = false;
  bool importTarget = false;
  bool importGlobal = false;

  std::vector<std::string>::const_iterator s = args.begin();

  std::string libName = *s;

  ++s;

  // If the second argument is "SHARED" or "STATIC", then it controls
  // the type of library.  Otherwise, it is treated as a source or
  // source list name. There may be two keyword arguments, check for them
  bool haveSpecifiedType = false;
  bool isAlias = false;
  while ( s != args.end() )
    {
    std::string libType = *s;
    if(libType == "STATIC")
      {
      if (type == cmTarget::INTERFACE_LIBRARY)
        {
        std::ostringstream e;
        e << "INTERFACE library specified with conflicting STATIC type.";
        this->SetError(e.str());
        return false;
        }
      ++s;
      type = cmTarget::STATIC_LIBRARY;
      haveSpecifiedType = true;
      }
    else if(libType == "SHARED")
      {
      if (type == cmTarget::INTERFACE_LIBRARY)
        {
        std::ostringstream e;
        e << "INTERFACE library specified with conflicting SHARED type.";
        this->SetError(e.str());
        return false;
        }
      ++s;
      type = cmTarget::SHARED_LIBRARY;
      haveSpecifiedType = true;
      }
    else if(libType == "MODULE")
      {
      if (type == cmTarget::INTERFACE_LIBRARY)
        {
        std::ostringstream e;
        e << "INTERFACE library specified with conflicting MODULE type.";
        this->SetError(e.str());
        return false;
        }
      ++s;
      type = cmTarget::MODULE_LIBRARY;
      haveSpecifiedType = true;
      }
    else if(libType == "OBJECT")
      {
      if (type == cmTarget::INTERFACE_LIBRARY)
        {
        std::ostringstream e;
        e << "INTERFACE library specified with conflicting OBJECT type.";
        this->SetError(e.str());
        return false;
        }
      ++s;
      type = cmTarget::OBJECT_LIBRARY;
      haveSpecifiedType = true;
      }
    else if(libType == "UNKNOWN")
      {
      if (type == cmTarget::INTERFACE_LIBRARY)
        {
        std::ostringstream e;
        e << "INTERFACE library specified with conflicting UNKNOWN type.";
        this->SetError(e.str());
        return false;
        }
      ++s;
      type = cmTarget::UNKNOWN_LIBRARY;
      haveSpecifiedType = true;
      }
    else if(libType == "ALIAS")
      {
      if (type == cmTarget::INTERFACE_LIBRARY)
        {
        std::ostringstream e;
        e << "INTERFACE library specified with conflicting ALIAS type.";
        this->SetError(e.str());
        return false;
        }
      ++s;
      isAlias = true;
      }
    else if(libType == "INTERFACE")
      {
      if (haveSpecifiedType)
        {
        std::ostringstream e;
        e << "INTERFACE library specified with conflicting/multiple types.";
        this->SetError(e.str());
        return false;
        }
      if (isAlias)
        {
        std::ostringstream e;
        e << "INTERFACE library specified with conflicting ALIAS type.";
        this->SetError(e.str());
        return false;
        }
      if (excludeFromAll)
        {
        std::ostringstream e;
        e << "INTERFACE library may not be used with EXCLUDE_FROM_ALL.";
        this->SetError(e.str());
        return false;
        }
      ++s;
      type = cmTarget::INTERFACE_LIBRARY;
      haveSpecifiedType = true;
      }
    else if(*s == "EXCLUDE_FROM_ALL")
      {
      if (type == cmTarget::INTERFACE_LIBRARY)
        {
        std::ostringstream e;
        e << "INTERFACE library may not be used with EXCLUDE_FROM_ALL.";
        this->SetError(e.str());
        return false;
        }
      ++s;
      excludeFromAll = true;
      }
    else if(*s == "IMPORTED")
      {
      ++s;
      importTarget = true;
      }
    else if(importTarget && *s == "GLOBAL")
      {
      ++s;
      importGlobal = true;
      }
    else if(type == cmTarget::INTERFACE_LIBRARY && *s == "GLOBAL")
      {
      std::ostringstream e;
      e << "GLOBAL option may only be used with IMPORTED libraries.";
      this->SetError(e.str());
      return false;
      }
    else
      {
      break;
      }
    }

  if (type == cmTarget::INTERFACE_LIBRARY)
    {
    if (s != args.end())
      {
      std::ostringstream e;
      e << "INTERFACE library requires no source arguments.";
      this->SetError(e.str());
      return false;
      }
    if (importGlobal && !importTarget)
      {
      std::ostringstream e;
      e << "INTERFACE library specified as GLOBAL, but not as IMPORTED.";
      this->SetError(e.str());
      return false;
      }
    }

  bool nameOk = cmGeneratorExpression::IsValidTargetName(libName) &&
    !cmGlobalGenerator::IsReservedTarget(libName);

  if (nameOk && !importTarget && !isAlias)
    {
    nameOk = libName.find(":") == std::string::npos;
    }
  if (!nameOk)
    {
    cmake::MessageType messageType = cmake::AUTHOR_WARNING;
    std::ostringstream e;
    bool issueMessage = false;
    switch(this->Makefile->GetPolicyStatus(cmPolicies::CMP0037))
      {
      case cmPolicies::WARN:
        if(type != cmTarget::INTERFACE_LIBRARY)
          {
          e << cmPolicies::GetPolicyWarning(cmPolicies::CMP0037) << "\n";
          issueMessage = true;
          }
      case cmPolicies::OLD:
        break;
      case cmPolicies::NEW:
      case cmPolicies::REQUIRED_IF_USED:
      case cmPolicies::REQUIRED_ALWAYS:
        issueMessage = true;
        messageType = cmake::FATAL_ERROR;
      }
    if (issueMessage)
      {
      e << "The target name \"" << libName <<
          "\" is reserved or not valid for certain "
          "CMake features, such as generator expressions, and may result "
          "in undefined behavior.";
      this->Makefile->IssueMessage(messageType, e.str());

      if (messageType == cmake::FATAL_ERROR)
        {
        return false;
        }
      }
    }

  if (isAlias)
    {
    if(!cmGeneratorExpression::IsValidTargetName(libName))
      {
      this->SetError("Invalid name for ALIAS: " + libName);
      return false;
      }
    if(excludeFromAll)
      {
      this->SetError("EXCLUDE_FROM_ALL with ALIAS makes no sense.");
      return false;
      }
    if(importTarget || importGlobal)
      {
      this->SetError("IMPORTED with ALIAS is not allowed.");
      return false;
      }
    if(args.size() != 3)
      {
      std::ostringstream e;
      e << "ALIAS requires exactly one target argument.";
      this->SetError(e.str());
      return false;
      }

    const char *aliasedName = s->c_str();
    if(this->Makefile->IsAlias(aliasedName))
      {
      std::ostringstream e;
      e << "cannot create ALIAS target \"" << libName
        << "\" because target \"" << aliasedName << "\" is itself an ALIAS.";
      this->SetError(e.str());
      return false;
      }
    cmTarget *aliasedTarget =
                    this->Makefile->FindTargetToUse(aliasedName, true);
    if(!aliasedTarget)
      {
      std::ostringstream e;
      e << "cannot create ALIAS target \"" << libName
        << "\" because target \"" << aliasedName << "\" does not already "
        "exist.";
      this->SetError(e.str());
      return false;
      }
    cmTarget::TargetType aliasedType = aliasedTarget->GetType();
    if(aliasedType != cmTarget::SHARED_LIBRARY
        && aliasedType != cmTarget::STATIC_LIBRARY
        && aliasedType != cmTarget::MODULE_LIBRARY
        && aliasedType != cmTarget::OBJECT_LIBRARY
        && aliasedType != cmTarget::INTERFACE_LIBRARY)
      {
      std::ostringstream e;
      e << "cannot create ALIAS target \"" << libName
        << "\" because target \"" << aliasedName << "\" is not a library.";
      this->SetError(e.str());
      return false;
      }
    if(aliasedTarget->IsImported())
      {
      std::ostringstream e;
      e << "cannot create ALIAS target \"" << libName
        << "\" because target \"" << aliasedName << "\" is IMPORTED.";
      this->SetError(e.str());
      return false;
      }
    this->Makefile->AddAlias(libName, aliasedTarget);
    return true;
    }

  if(importTarget && excludeFromAll)
    {
    this->SetError("excludeFromAll with IMPORTED target makes no sense.");
    return false;
    }

  /* ideally we should check whether for the linker language of the target
    CMAKE_${LANG}_CREATE_SHARED_LIBRARY is defined and if not default to
    STATIC. But at this point we know only the name of the target, but not
    yet its linker language. */
  if ((type == cmTarget::SHARED_LIBRARY ||
       type == cmTarget::MODULE_LIBRARY) &&
       (this->Makefile->GetState()->GetGlobalPropertyAsBool(
                                      "TARGET_SUPPORTS_SHARED_LIBS") == false))
    {
    std::ostringstream w;
    w <<
      "ADD_LIBRARY called with " <<
      (type==cmTarget::SHARED_LIBRARY ? "SHARED" : "MODULE") <<
      " option but the target platform does not support dynamic linking. "
      "Building a STATIC library instead. This may lead to problems.";
    this->Makefile->IssueMessage(cmake::AUTHOR_WARNING, w.str());
    type = cmTarget::STATIC_LIBRARY;
    }

  // Handle imported target creation.
  if(importTarget)
    {
    // The IMPORTED signature requires a type to be specified explicitly.
    if (!haveSpecifiedType)
      {
      this->SetError("called with IMPORTED argument but no library type.");
      return false;
      }
    if(type == cmTarget::OBJECT_LIBRARY)
      {
      this->Makefile->IssueMessage(
        cmake::FATAL_ERROR,
        "The OBJECT library type may not be used for IMPORTED libraries."
        );
      return true;
      }
    if(type == cmTarget::INTERFACE_LIBRARY)
      {
      if (!cmGeneratorExpression::IsValidTargetName(libName))
        {
        std::ostringstream e;
        e << "Invalid name for IMPORTED INTERFACE library target: " << libName;
        this->SetError(e.str());
        return false;
        }
      }

    // Make sure the target does not already exist.
    if(this->Makefile->FindTargetToUse(libName))
      {
      std::ostringstream e;
      e << "cannot create imported target \"" << libName
        << "\" because another target with the same name already exists.";
      this->SetError(e.str());
      return false;
      }

    // Create the imported target.
    this->Makefile->AddImportedTarget(libName, type, importGlobal);
    return true;
    }

  // A non-imported target may not have UNKNOWN type.
  if(type == cmTarget::UNKNOWN_LIBRARY)
    {
    this->Makefile->IssueMessage(
      cmake::FATAL_ERROR,
      "The UNKNOWN library type may be used only for IMPORTED libraries."
      );
    return true;
    }

  // Enforce name uniqueness.
  {
  std::string msg;
  if(!this->Makefile->EnforceUniqueName(libName, msg))
    {
    this->SetError(msg);
    return false;
    }
  }

  std::vector<std::string> srclists;

  if(type == cmTarget::INTERFACE_LIBRARY)
    {
    if (!cmGeneratorExpression::IsValidTargetName(libName)
        || libName.find("::") != std::string::npos)
      {
      std::ostringstream e;
      e << "Invalid name for INTERFACE library target: " << libName;
      this->SetError(e.str());
      return false;
      }

    this->Makefile->AddLibrary(libName,
                               type,
                               srclists,
                               excludeFromAll);
    return true;
    }

  if (s == args.end())
    {
    std::string msg = "You have called ADD_LIBRARY for library ";
    msg += args[0];
    msg += " without any source files. This typically indicates a problem ";
    msg += "with your CMakeLists.txt file";
    cmSystemTools::Message(msg.c_str() ,"Warning");
    }

  srclists.insert(srclists.end(), s, args.end());

  this->Makefile->AddLibrary(libName, type, srclists, excludeFromAll);

  return true;
}


