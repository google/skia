/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "cmTargetLinkLibrariesCommand.h"

#include "cmGeneratorExpression.h"

const char* cmTargetLinkLibrariesCommand::LinkLibraryTypeNames[3] =
{
  "general",
  "debug",
  "optimized"
};

// cmTargetLinkLibrariesCommand
bool cmTargetLinkLibrariesCommand
::InitialPass(std::vector<std::string> const& args, cmExecutionStatus &)
{
  // must have one argument
  if(args.size() < 1)
    {
    this->SetError("called with incorrect number of arguments");
    return false;
    }

  if (this->Makefile->IsAlias(args[0]))
    {
    this->SetError("can not be used on an ALIAS target.");
    return false;
    }
  // Lookup the target for which libraries are specified.
  this->Target =
    this->Makefile->GetCMakeInstance()
    ->GetGlobalGenerator()->FindTarget(args[0]);
  if(!this->Target)
    {
    cmake::MessageType t = cmake::FATAL_ERROR;  // fail by default
    std::ostringstream e;
    e << "Cannot specify link libraries for target \"" << args[0] << "\" "
      << "which is not built by this project.";
    // The bad target is the only argument. Check how policy CMP0016 is set,
    // and accept, warn or fail respectively:
    if (args.size() < 2)
      {
      switch (this->Makefile->GetPolicyStatus(cmPolicies::CMP0016))
        {
        case cmPolicies::WARN:
          t = cmake::AUTHOR_WARNING;
          // Print the warning.
          e << "\n"
            << "CMake does not support this but it used to work accidentally "
            << "and is being allowed for compatibility."
            << "\n" << cmPolicies::GetPolicyWarning(cmPolicies::CMP0016);
           break;
        case cmPolicies::OLD:          // OLD behavior does not warn.
          t = cmake::MESSAGE;
          break;
        case cmPolicies::REQUIRED_IF_USED:
        case cmPolicies::REQUIRED_ALWAYS:
          e << "\n" << cmPolicies::GetRequiredPolicyError(cmPolicies::CMP0016);
          break;
        case cmPolicies::NEW:  // NEW behavior prints the error.
          break;
        }
      }

    // now actually print the message
    switch(t)
      {
      case cmake::AUTHOR_WARNING:
        this->Makefile->IssueMessage(cmake::AUTHOR_WARNING, e.str());
        break;
      case cmake::FATAL_ERROR:
        this->Makefile->IssueMessage(cmake::FATAL_ERROR, e.str());
        cmSystemTools::SetFatalErrorOccured();
        break;
      default:
        break;
      }
    return true;
    }

  if(this->Target->GetType() == cmTarget::OBJECT_LIBRARY)
    {
    std::ostringstream e;
    e << "Object library target \"" << args[0] << "\" "
      << "may not link to anything.";
    this->Makefile->IssueMessage(cmake::FATAL_ERROR, e.str());
    cmSystemTools::SetFatalErrorOccured();
    return true;
    }

  if (this->Target->GetType() == cmTarget::UTILITY)
    {
    std::ostringstream e;
    const char *modal = 0;
    cmake::MessageType messageType = cmake::AUTHOR_WARNING;
    switch(this->Makefile->GetPolicyStatus(cmPolicies::CMP0039))
      {
      case cmPolicies::WARN:
        e << cmPolicies::GetPolicyWarning(cmPolicies::CMP0039) << "\n";
        modal = "should";
      case cmPolicies::OLD:
        break;
      case cmPolicies::REQUIRED_ALWAYS:
      case cmPolicies::REQUIRED_IF_USED:
      case cmPolicies::NEW:
        modal = "must";
        messageType = cmake::FATAL_ERROR;
      }
    if (modal)
      {
      e <<
        "Utility target \"" << this->Target->GetName() << "\" " << modal
        << " not be used as the target of a target_link_libraries call.";
      this->Makefile->IssueMessage(messageType, e.str());
      if(messageType == cmake::FATAL_ERROR)
        {
        return false;
        }
      }
    }

  // but we might not have any libs after variable expansion
  if(args.size() < 2)
    {
    return true;
    }

  // Keep track of link configuration specifiers.
  cmTarget::LinkLibraryType llt = cmTarget::GENERAL;
  bool haveLLT = false;

  // Start with primary linking and switch to link interface
  // specification if the keyword is encountered as the first argument.
  this->CurrentProcessingState = ProcessingLinkLibraries;

  // add libraries, note that there is an optional prefix
  // of debug and optimized that can be used
  for(unsigned int i=1; i < args.size(); ++i)
    {
    if(args[i] == "LINK_INTERFACE_LIBRARIES")
      {
      this->CurrentProcessingState = ProcessingPlainLinkInterface;
      if(i != 1)
        {
        this->Makefile->IssueMessage(
          cmake::FATAL_ERROR,
          "The LINK_INTERFACE_LIBRARIES option must appear as the second "
          "argument, just after the target name."
          );
        return true;
        }
      }
    else if(args[i] == "INTERFACE")
      {
      if(i != 1
          && this->CurrentProcessingState != ProcessingKeywordPrivateInterface
          && this->CurrentProcessingState != ProcessingKeywordPublicInterface
          && this->CurrentProcessingState != ProcessingKeywordLinkInterface)
        {
        this->Makefile->IssueMessage(
          cmake::FATAL_ERROR,
          "The INTERFACE option must appear as the second "
          "argument, just after the target name."
          );
        return true;
        }
      this->CurrentProcessingState = ProcessingKeywordLinkInterface;
      }
    else if(args[i] == "LINK_PUBLIC")
      {
      if(i != 1
          && this->CurrentProcessingState != ProcessingPlainPrivateInterface
          && this->CurrentProcessingState != ProcessingPlainPublicInterface)
        {
        this->Makefile->IssueMessage(
          cmake::FATAL_ERROR,
          "The LINK_PUBLIC or LINK_PRIVATE option must appear as the second "
          "argument, just after the target name."
          );
        return true;
        }
      this->CurrentProcessingState = ProcessingPlainPublicInterface;
      }
    else if(args[i] == "PUBLIC")
      {
      if(i != 1
          && this->CurrentProcessingState != ProcessingKeywordPrivateInterface
          && this->CurrentProcessingState != ProcessingKeywordPublicInterface
          && this->CurrentProcessingState != ProcessingKeywordLinkInterface)
        {
        this->Makefile->IssueMessage(
          cmake::FATAL_ERROR,
          "The PUBLIC or PRIVATE option must appear as the second "
          "argument, just after the target name."
          );
        return true;
        }
      this->CurrentProcessingState = ProcessingKeywordPublicInterface;
      }
    else if(args[i] == "LINK_PRIVATE")
      {
      if(i != 1
          && this->CurrentProcessingState != ProcessingPlainPublicInterface
          && this->CurrentProcessingState != ProcessingPlainPrivateInterface)
        {
        this->Makefile->IssueMessage(
          cmake::FATAL_ERROR,
          "The LINK_PUBLIC or LINK_PRIVATE option must appear as the second "
          "argument, just after the target name."
          );
        return true;
        }
      this->CurrentProcessingState = ProcessingPlainPrivateInterface;
      }
    else if(args[i] == "PRIVATE")
      {
      if(i != 1
          && this->CurrentProcessingState != ProcessingKeywordPrivateInterface
          && this->CurrentProcessingState != ProcessingKeywordPublicInterface
          && this->CurrentProcessingState != ProcessingKeywordLinkInterface)
        {
        this->Makefile->IssueMessage(
          cmake::FATAL_ERROR,
          "The PUBLIC or PRIVATE option must appear as the second "
          "argument, just after the target name."
          );
        return true;
        }
      this->CurrentProcessingState = ProcessingKeywordPrivateInterface;
      }
    else if(args[i] == "debug")
      {
      if(haveLLT)
        {
        this->LinkLibraryTypeSpecifierWarning(llt, cmTarget::DEBUG);
        }
      llt = cmTarget::DEBUG;
      haveLLT = true;
      }
    else if(args[i] == "optimized")
      {
      if(haveLLT)
        {
        this->LinkLibraryTypeSpecifierWarning(llt, cmTarget::OPTIMIZED);
        }
      llt = cmTarget::OPTIMIZED;
      haveLLT = true;
      }
    else if(args[i] == "general")
      {
      if(haveLLT)
        {
        this->LinkLibraryTypeSpecifierWarning(llt, cmTarget::GENERAL);
        }
      llt = cmTarget::GENERAL;
      haveLLT = true;
      }
    else if(haveLLT)
      {
      // The link type was specified by the previous argument.
      haveLLT = false;
      if (!this->HandleLibrary(args[i], llt))
        {
        return false;
        }
      }
    else
      {
      // Lookup old-style cache entry if type is unspecified.  So if you
      // do a target_link_libraries(foo optimized bar) it will stay optimized
      // and not use the lookup.  As there maybe the case where someone has
      // specifed that a library is both debug and optimized.  (this check is
      // only there for backwards compatibility when mixing projects built
      // with old versions of CMake and new)
      llt = cmTarget::GENERAL;
      std::string linkType = args[0];
      linkType += "_LINK_TYPE";
      const char* linkTypeString =
        this->Makefile->GetDefinition( linkType );
      if(linkTypeString)
        {
        if(strcmp(linkTypeString, "debug") == 0)
          {
          llt = cmTarget::DEBUG;
          }
        if(strcmp(linkTypeString, "optimized") == 0)
          {
          llt = cmTarget::OPTIMIZED;
          }
        }
      if (!this->HandleLibrary(args[i], llt))
        {
        return false;
        }
      }
    }

  // Make sure the last argument was not a library type specifier.
  if(haveLLT)
    {
    std::ostringstream e;
    e << "The \"" << this->LinkLibraryTypeNames[llt]
      << "\" argument must be followed by a library.";
    this->Makefile->IssueMessage(cmake::FATAL_ERROR, e.str());
    cmSystemTools::SetFatalErrorOccured();
    }

  const cmPolicies::PolicyStatus policy22Status
                      = this->Target->GetPolicyStatusCMP0022();

  // If any of the LINK_ options were given, make sure the
  // LINK_INTERFACE_LIBRARIES target property exists.
  // Use of any of the new keywords implies awareness of
  // this property. And if no libraries are named, it should
  // result in an empty link interface.
  if((policy22Status == cmPolicies::OLD ||
      policy22Status == cmPolicies::WARN) &&
      this->CurrentProcessingState != ProcessingLinkLibraries &&
     !this->Target->GetProperty("LINK_INTERFACE_LIBRARIES"))
    {
    this->Target->SetProperty("LINK_INTERFACE_LIBRARIES", "");
    }

  return true;
}

//----------------------------------------------------------------------------
void
cmTargetLinkLibrariesCommand
::LinkLibraryTypeSpecifierWarning(int left, int right)
{
  std::ostringstream w;
  w << "Link library type specifier \""
    << this->LinkLibraryTypeNames[left] << "\" is followed by specifier \""
    << this->LinkLibraryTypeNames[right] << "\" instead of a library name.  "
    << "The first specifier will be ignored.";
  this->Makefile->IssueMessage(cmake::AUTHOR_WARNING, w.str());
}

//----------------------------------------------------------------------------
bool
cmTargetLinkLibrariesCommand::HandleLibrary(const std::string& lib,
                                            cmTarget::LinkLibraryType llt)
{
  if(this->Target->GetType() == cmTarget::INTERFACE_LIBRARY
      && this->CurrentProcessingState != ProcessingKeywordLinkInterface)
    {
    this->Makefile->IssueMessage(cmake::FATAL_ERROR,
      "INTERFACE library can only be used with the INTERFACE keyword of "
      "target_link_libraries");
    return false;
    }

  cmTarget::TLLSignature sig =
        (this->CurrentProcessingState == ProcessingPlainPrivateInterface
      || this->CurrentProcessingState == ProcessingPlainPublicInterface
      || this->CurrentProcessingState == ProcessingKeywordPrivateInterface
      || this->CurrentProcessingState == ProcessingKeywordPublicInterface
      || this->CurrentProcessingState == ProcessingKeywordLinkInterface)
        ? cmTarget::KeywordTLLSignature : cmTarget::PlainTLLSignature;
  if (!this->Target->PushTLLCommandTrace(
        sig, this->Makefile->GetExecutionContext()))
    {
    std::ostringstream e;
    const char *modal = 0;
    cmake::MessageType messageType = cmake::AUTHOR_WARNING;
    switch(this->Makefile->GetPolicyStatus(cmPolicies::CMP0023))
      {
      case cmPolicies::WARN:
        e << cmPolicies::GetPolicyWarning(cmPolicies::CMP0023) << "\n";
        modal = "should";
      case cmPolicies::OLD:
        break;
      case cmPolicies::REQUIRED_ALWAYS:
      case cmPolicies::REQUIRED_IF_USED:
      case cmPolicies::NEW:
        modal = "must";
        messageType = cmake::FATAL_ERROR;
      }

      if(modal)
        {
        // If the sig is a keyword form and there is a conflict, the existing
        // form must be the plain form.
        const char *existingSig
                    = (sig == cmTarget::KeywordTLLSignature ? "plain"
                                                            : "keyword");
          e <<
            "The " << existingSig << " signature for target_link_libraries "
            "has already been used with the target \""
          << this->Target->GetName() << "\".  All uses of "
             "target_link_libraries with a target " << modal << " be either "
             "all-keyword or all-plain.\n";
        this->Target->GetTllSignatureTraces(e,
                                          sig == cmTarget::KeywordTLLSignature
                                            ? cmTarget::PlainTLLSignature
                                            : cmTarget::KeywordTLLSignature);
        this->Makefile->IssueMessage(messageType, e.str());
        if(messageType == cmake::FATAL_ERROR)
          {
          return false;
          }
        }
    }

  // Handle normal case first.
  if(this->CurrentProcessingState != ProcessingKeywordLinkInterface
      && this->CurrentProcessingState != ProcessingPlainLinkInterface)
    {
    this->Makefile
      ->AddLinkLibraryForTarget(this->Target->GetName(), lib, llt);
    if(this->CurrentProcessingState == ProcessingLinkLibraries)
      {
      this->Target->AppendProperty("INTERFACE_LINK_LIBRARIES",
        this->Target->GetDebugGeneratorExpressions(lib, llt).c_str());
      return true;
      }
    else if(this->CurrentProcessingState != ProcessingKeywordPublicInterface
            && this->CurrentProcessingState != ProcessingPlainPublicInterface)
      {
      if (this->Target->GetType() == cmTarget::STATIC_LIBRARY)
        {
        std::string configLib = this->Target
                                     ->GetDebugGeneratorExpressions(lib, llt);
        if (cmGeneratorExpression::IsValidTargetName(lib)
            || cmGeneratorExpression::Find(lib) != std::string::npos)
          {
          configLib = "$<LINK_ONLY:" + configLib + ">";
          }
        this->Target->AppendProperty("INTERFACE_LINK_LIBRARIES",
                                     configLib.c_str());
        }
      // Not a 'public' or 'interface' library. Do not add to interface
      // property.
      return true;
      }
    }

  this->Target->AppendProperty("INTERFACE_LINK_LIBRARIES",
              this->Target->GetDebugGeneratorExpressions(lib, llt).c_str());

  const cmPolicies::PolicyStatus policy22Status
                      = this->Target->GetPolicyStatusCMP0022();

  if (policy22Status != cmPolicies::OLD
      && policy22Status != cmPolicies::WARN)
    {
    return true;
    }

  if (this->Target->GetType() == cmTarget::INTERFACE_LIBRARY)
    {
    return true;
    }

  // Get the list of configurations considered to be DEBUG.
  std::vector<std::string> debugConfigs =
    this->Makefile->GetCMakeInstance()->GetDebugConfigs();
  std::string prop;

  // Include this library in the link interface for the target.
  if(llt == cmTarget::DEBUG || llt == cmTarget::GENERAL)
    {
    // Put in the DEBUG configuration interfaces.
    for(std::vector<std::string>::const_iterator i = debugConfigs.begin();
        i != debugConfigs.end(); ++i)
      {
      prop = "LINK_INTERFACE_LIBRARIES_";
      prop += *i;
      this->Target->AppendProperty(prop, lib.c_str());
      }
    }
  if(llt == cmTarget::OPTIMIZED || llt == cmTarget::GENERAL)
    {
    // Put in the non-DEBUG configuration interfaces.
    this->Target->AppendProperty("LINK_INTERFACE_LIBRARIES", lib.c_str());

    // Make sure the DEBUG configuration interfaces exist so that the
    // general one will not be used as a fall-back.
    for(std::vector<std::string>::const_iterator i = debugConfigs.begin();
        i != debugConfigs.end(); ++i)
      {
      prop = "LINK_INTERFACE_LIBRARIES_";
      prop += *i;
      if(!this->Target->GetProperty(prop))
        {
        this->Target->SetProperty(prop, "");
        }
      }
    }
  return true;
}
