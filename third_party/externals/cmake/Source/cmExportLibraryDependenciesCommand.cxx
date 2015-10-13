/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "cmExportLibraryDependenciesCommand.h"
#include "cmGlobalGenerator.h"
#include "cmLocalGenerator.h"
#include "cmGeneratedFileStream.h"
#include "cmake.h"
#include "cmVersion.h"

#include <cmsys/auto_ptr.hxx>

bool cmExportLibraryDependenciesCommand
::InitialPass(std::vector<std::string> const& args, cmExecutionStatus &)
{
  if(this->Disallowed(cmPolicies::CMP0033,
      "The export_library_dependencies command should not be called; "
      "see CMP0033."))
    { return true; }
  if(args.size() < 1 )
    {
    this->SetError("called with incorrect number of arguments");
    return false;
    }

  // store the arguments for the final pass
  this->Filename = args[0];
  this->Append = false;
  if(args.size() > 1)
    {
    if(args[1] == "APPEND")
      {
      this->Append = true;
      }
    }
  return true;
}


void cmExportLibraryDependenciesCommand::FinalPass()
{
  // export_library_dependencies() shouldn't modify anything
  // ensure this by calling a const method
  this->ConstFinalPass();
}

void cmExportLibraryDependenciesCommand::ConstFinalPass() const
{
  // Use copy-if-different if not appending.
  cmsys::auto_ptr<cmsys::ofstream> foutPtr;
  if(this->Append)
    {
    cmsys::auto_ptr<cmsys::ofstream> ap(
      new cmsys::ofstream(this->Filename.c_str(), std::ios::app));
    foutPtr = ap;
    }
  else
    {
    cmsys::auto_ptr<cmGeneratedFileStream> ap(
      new cmGeneratedFileStream(this->Filename.c_str(), true));
    ap->SetCopyIfDifferent(true);
    foutPtr = ap;
    }
  std::ostream& fout = *foutPtr.get();

  if (!fout)
    {
    cmSystemTools::Error("Error Writing ", this->Filename.c_str());
    cmSystemTools::ReportLastSystemError("");
    return;
    }

  // Collect dependency information about all library targets built in
  // the project.
  cmake* cm = this->Makefile->GetCMakeInstance();
  cmGlobalGenerator* global = cm->GetGlobalGenerator();
  const std::vector<cmLocalGenerator *>& locals = global->GetLocalGenerators();
  std::map<std::string, std::string> libDepsOld;
  std::map<std::string, std::string> libDepsNew;
  std::map<std::string, std::string> libTypes;
  for(std::vector<cmLocalGenerator *>::const_iterator i = locals.begin();
      i != locals.end(); ++i)
    {
    const cmLocalGenerator* gen = *i;
    const cmTargets &tgts = gen->GetMakefile()->GetTargets();
    for(cmTargets::const_iterator l = tgts.begin();
        l != tgts.end(); ++l)
      {
      // Get the current target.
      cmTarget const& target = l->second;

      // Skip non-library targets.
      if(target.GetType() < cmTarget::STATIC_LIBRARY
         || target.GetType() > cmTarget::MODULE_LIBRARY)
        {
        continue;
        }

      // Construct the dependency variable name.
      std::string targetEntry = target.GetName();
      targetEntry += "_LIB_DEPENDS";

      // Construct the dependency variable value with the direct link
      // dependencies.
      std::string valueOld;
      std::string valueNew;
      cmTarget::LinkLibraryVectorType const& libs =
        target.GetOriginalLinkLibraries();
      for(cmTarget::LinkLibraryVectorType::const_iterator li = libs.begin();
          li != libs.end(); ++li)
        {
        std::string ltVar = li->first;
        ltVar += "_LINK_TYPE";
        std::string ltValue;
        switch(li->second)
          {
          case cmTarget::GENERAL:
            valueNew += "general;";
            ltValue = "general";
            break;
          case cmTarget::DEBUG:
            valueNew += "debug;";
            ltValue = "debug";
            break;
          case cmTarget::OPTIMIZED:
            valueNew += "optimized;";
            ltValue = "optimized";
            break;
          }
        std::string lib = li->first;
        if(cmTarget* libtgt = global->FindTarget(lib))
          {
          // Handle simple output name changes.  This command is
          // deprecated so we do not support full target name
          // translation (which requires per-configuration info).
          if(const char* outname = libtgt->GetProperty("OUTPUT_NAME"))
            {
            lib = outname;
            }
          }
        valueOld += lib;
        valueOld += ";";
        valueNew += lib;
        valueNew += ";";

        std::string& ltEntry = libTypes[ltVar];
        if(ltEntry.empty())
          {
          ltEntry = ltValue;
          }
        else if(ltEntry != ltValue)
          {
          ltEntry = "general";
          }
        }
      libDepsNew[targetEntry] = valueNew;
      libDepsOld[targetEntry] = valueOld;
      }
    }

  // Generate dependency information for both old and new style CMake
  // versions.
  const char* vertest =
    "\"${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION}\" GREATER 2.4";
  fout << "# Generated by CMake " <<  cmVersion::GetCMakeVersion() << "\n\n";
  fout << "if(" << vertest << ")\n";
  fout << "  # Information for CMake 2.6 and above.\n";
  for(std::map<std::string, std::string>::const_iterator
        i = libDepsNew.begin();
      i != libDepsNew.end(); ++i)
    {
    if(!i->second.empty())
      {
      fout << "  set(\"" << i->first << "\" \"" << i->second << "\")\n";
      }
    }
  fout << "else()\n";
  fout << "  # Information for CMake 2.4 and lower.\n";
  for(std::map<std::string, std::string>::const_iterator
        i = libDepsOld.begin();
      i != libDepsOld.end(); ++i)
    {
    if(!i->second.empty())
      {
      fout << "  set(\"" << i->first << "\" \"" << i->second << "\")\n";
      }
    }
  for(std::map<std::string, std::string>::const_iterator i = libTypes.begin();
      i != libTypes.end(); ++i)
    {
    if(i->second != "general")
      {
      fout << "  set(\"" << i->first << "\" \"" << i->second << "\")\n";
      }
    }
  fout << "endif()\n";
  return;
}
