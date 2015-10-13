/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "cmGlobalVisualStudio6Generator.h"
#include "cmLocalVisualStudio6Generator.h"
#include "cmMakefile.h"
#include "cmake.h"
#include "cmGeneratedFileStream.h"
#include <cmsys/FStream.hxx>

// Utility function to make a valid VS6 *.dsp filename out
// of a CMake target name:
//
std::string GetVS6TargetName(const std::string& targetName)
{
  std::string name(targetName);

  // Eliminate hyphens. VS6 cannot handle hyphens in *.dsp filenames...
  // Replace them with underscores.
  //
  cmSystemTools::ReplaceString(name, "-", "_");

  return name;
}

cmGlobalVisualStudio6Generator::cmGlobalVisualStudio6Generator(cmake* cm)
  : cmGlobalVisualStudioGenerator(cm)
{
  this->MSDevCommandInitialized = false;
  this->Version = VS6;
}

void cmGlobalVisualStudio6Generator
::EnableLanguage(std::vector<std::string>const& lang,
                 cmMakefile *mf,
                 bool optional)
{
  mf->AddDefinition("CMAKE_GENERATOR_RC", "rc");
  mf->AddDefinition("CMAKE_GENERATOR_NO_COMPILER_ENV", "1");
  this->GenerateConfigurations(mf);
  this->cmGlobalGenerator::EnableLanguage(lang, mf, optional);
}

void cmGlobalVisualStudio6Generator::GenerateConfigurations(cmMakefile* mf)
{
  std::string fname= mf->GetRequiredDefinition("CMAKE_ROOT");
  const char* def= mf->GetDefinition( "MSPROJECT_TEMPLATE_DIRECTORY");
  if(def)
    {
    fname = def;
    }
  else
    {
    fname += "/Templates";
    }
  fname += "/CMakeVisualStudio6Configurations.cmake";
  if(!mf->ReadDependentFile(fname.c_str()))
    {
    cmSystemTools::Error("Cannot open ", fname.c_str(),
                         ".  Please copy this file from the main "
                         "CMake/Templates directory and edit it for "
                         "your build configurations.");
    }
  else if(!mf->GetDefinition("CMAKE_CONFIGURATION_TYPES"))
    {
    cmSystemTools::Error("CMAKE_CONFIGURATION_TYPES not set by ",
                         fname.c_str(),
                         ".  Please copy this file from the main "
                         "CMake/Templates directory and edit it for "
                         "your build configurations.");
    }
}

//----------------------------------------------------------------------------
void cmGlobalVisualStudio6Generator::FindMakeProgram(cmMakefile* mf)
{
  this->cmGlobalVisualStudioGenerator::FindMakeProgram(mf);
  mf->AddDefinition("CMAKE_VS_MSDEV_COMMAND",
                    this->GetMSDevCommand().c_str());
}

//----------------------------------------------------------------------------
std::string const& cmGlobalVisualStudio6Generator::GetMSDevCommand()
{
  if(!this->MSDevCommandInitialized)
    {
    this->MSDevCommandInitialized = true;
    this->MSDevCommand = this->FindMSDevCommand();
    }
  return this->MSDevCommand;
}

//----------------------------------------------------------------------------
std::string cmGlobalVisualStudio6Generator::FindMSDevCommand()
{
  std::string vscmd;
  std::string vskey = this->GetRegistryBase() + "\\Setup;VsCommonDir";
  if(cmSystemTools::ReadRegistryValue(vskey.c_str(), vscmd,
                                      cmSystemTools::KeyWOW64_32))
    {
    cmSystemTools::ConvertToUnixSlashes(vscmd);
    vscmd += "/MSDev98/Bin/";
    }
  vscmd += "msdev.exe";
  return vscmd;
}

//----------------------------------------------------------------------------
void
cmGlobalVisualStudio6Generator::GenerateBuildCommand(
  std::vector<std::string>& makeCommand,
  const std::string& makeProgram,
  const std::string& projectName,
  const std::string& /*projectDir*/,
  const std::string& targetName,
  const std::string& config,
  bool /*fast*/, bool /*verbose*/,
  std::vector<std::string> const& makeOptions
  )
{
  // now build the test
  makeCommand.push_back(
    this->SelectMakeProgram(makeProgram, this->GetMSDevCommand())
    );

  makeCommand.push_back(std::string(projectName)+".dsw");
  makeCommand.push_back("/MAKE");
  std::string targetArg;
  bool clean = false;
  std::string realTarget = targetName;
  if ( realTarget == "clean" )
    {
    clean = true;
    realTarget = "ALL_BUILD";
    }
  if (!realTarget.empty())
    {
    targetArg += realTarget;
    }
  else
    {
    targetArg += "ALL_BUILD";
    }
  targetArg += " - ";
  if(!config.empty())
    {
    targetArg += config;
    }
  else
    {
    targetArg += "Debug";
    }
  makeCommand.push_back(targetArg);
  if(clean)
    {
    makeCommand.push_back("/CLEAN");
    }
  else
    {
    makeCommand.push_back("/BUILD");
    }
  makeCommand.insert(makeCommand.end(),
                     makeOptions.begin(), makeOptions.end());
}

///! Create a local generator appropriate to this Global Generator
cmLocalGenerator *
cmGlobalVisualStudio6Generator::CreateLocalGenerator(cmLocalGenerator* parent,
                                                   cmState::Snapshot snapshot)
{
  return new cmLocalVisualStudio6Generator(this, parent, snapshot);
}


void cmGlobalVisualStudio6Generator::Generate()
{
  // first do the superclass method
  this->cmGlobalVisualStudioGenerator::Generate();

  // Now write out the DSW
  this->OutputDSWFile();

  if (!this->CMakeInstance->GetIsInTryCompile())
    {
    const char* cmakeWarnVS6 =
      this->CMakeInstance->GetState()->GetCacheEntryValue("CMAKE_WARN_VS6");
    if (!cmakeWarnVS6 || !cmSystemTools::IsOff(cmakeWarnVS6))
      {
      this->CMakeInstance->IssueMessage(
        cmake::WARNING,
        "The \"Visual Studio 6\" generator is deprecated "
        "and will be removed in a future version of CMake."
        "\n"
        "Add CMAKE_WARN_VS6=OFF to the cache to disable this warning."
        );
      }
    }
}

// Write a DSW file to the stream
void cmGlobalVisualStudio6Generator
::WriteDSWFile(std::ostream& fout,cmLocalGenerator* root,
               std::vector<cmLocalGenerator*>& generators)
{
  // Write out the header for a DSW file
  this->WriteDSWHeader(fout);

  // Collect all targets under this root generator and the transitive
  // closure of their dependencies.
  TargetDependSet projectTargets;
  TargetDependSet originalTargets;
  this->GetTargetSets(projectTargets, originalTargets, root, generators);
  OrderedTargetDependSet orderedProjectTargets(projectTargets);

  for(OrderedTargetDependSet::const_iterator
        tt = orderedProjectTargets.begin();
      tt != orderedProjectTargets.end(); ++tt)
    {
    cmTarget const* target = *tt;
    if(target->GetType() == cmTarget::INTERFACE_LIBRARY)
      {
      continue;
      }
    // Write the project into the DSW file
    const char* expath = target->GetProperty("EXTERNAL_MSPROJECT");
    if(expath)
      {
      std::string project = target->GetName();
      std::string location = expath;
      this->WriteExternalProject(fout, project.c_str(),
                                 location.c_str(), target->GetUtilities());
      }
    else
      {
      std::string dspname = GetVS6TargetName(target->GetName());
      std::string dir = target->GetMakefile()->GetCurrentBinaryDirectory();
      dir = root->Convert(dir.c_str(), cmLocalGenerator::START_OUTPUT);
      this->WriteProject(fout, dspname.c_str(), dir.c_str(), *target);
      }
    }

  // Write the footer for the DSW file
  this->WriteDSWFooter(fout);
}

void cmGlobalVisualStudio6Generator
::OutputDSWFile(cmLocalGenerator* root,
                std::vector<cmLocalGenerator*>& generators)
{
  if(generators.size() == 0)
    {
    return;
    }
  std::string fname = root->GetMakefile()->GetCurrentBinaryDirectory();
  fname += "/";
  fname += root->GetMakefile()->GetProjectName();
  fname += ".dsw";
  cmsys::ofstream fout(fname.c_str());
  if(!fout)
    {
    cmSystemTools::Error("Error can not open DSW file for write: ",
                         fname.c_str());
    cmSystemTools::ReportLastSystemError("");
    return;
    }
  this->WriteDSWFile(fout, root, generators);
}

// output the DSW file
void cmGlobalVisualStudio6Generator::OutputDSWFile()
{
  std::map<std::string, std::vector<cmLocalGenerator*> >::iterator it;
  for(it = this->ProjectMap.begin(); it!= this->ProjectMap.end(); ++it)
    {
    this->OutputDSWFile(it->second[0], it->second);
    }
}

// Write a dsp file into the DSW file,
// Note, that dependencies from executables to
// the libraries it uses are also done here
void cmGlobalVisualStudio6Generator::WriteProject(std::ostream& fout,
                                                  const std::string& dspname,
                                                  const char* dir,
                                                  cmTarget const& target)
{
  fout << "#########################################################"
    "######################\n\n";
  fout << "Project: \"" << dspname << "\"="
       << dir << "\\" << dspname << ".dsp - Package Owner=<4>\n\n";
  fout << "Package=<5>\n{{{\n}}}\n\n";
  fout << "Package=<4>\n";
  fout << "{{{\n";
  VSDependSet const& depends = this->VSTargetDepends[&target];
  for(VSDependSet::const_iterator di = depends.begin();
      di != depends.end(); ++di)
    {
    const char* name = di->c_str();
    fout << "Begin Project Dependency\n";
    fout << "Project_Dep_Name " << GetVS6TargetName(name) << "\n";
    fout << "End Project Dependency\n";
    }
  fout << "}}}\n\n";

  UtilityDependsMap::iterator ui = this->UtilityDepends.find(&target);
  if(ui != this->UtilityDepends.end())
    {
    const char* uname = ui->second.c_str();
    fout << "Project: \"" << uname << "\"="
         << dir << "\\" << uname << ".dsp - Package Owner=<4>\n\n";
    fout <<
      "Package=<5>\n{{{\n}}}\n\n"
      "Package=<4>\n"
      "{{{\n"
      "Begin Project Dependency\n"
      "Project_Dep_Name " << dspname << "\n"
      "End Project Dependency\n"
      "}}}\n\n";
      ;
    }
}


// Write a dsp file into the DSW file,
// Note, that dependencies from executables to
// the libraries it uses are also done here
void cmGlobalVisualStudio6Generator::WriteExternalProject(std::ostream& fout,
                               const std::string& name,
                               const char* location,
                               const std::set<std::string>& dependencies)
{
 fout << "#########################################################"
    "######################\n\n";
  fout << "Project: \"" << name << "\"="
       << location << " - Package Owner=<4>\n\n";
  fout << "Package=<5>\n{{{\n}}}\n\n";
  fout << "Package=<4>\n";
  fout << "{{{\n";


  std::set<std::string>::const_iterator i, end;
  // write dependencies.
  i = dependencies.begin();
  end = dependencies.end();
  for(;i!= end; ++i)
  {
    fout << "Begin Project Dependency\n";
    fout << "Project_Dep_Name " << GetVS6TargetName(*i) << "\n";
    fout << "End Project Dependency\n";
  }
  fout << "}}}\n\n";
}



// Standard end of dsw file
void cmGlobalVisualStudio6Generator::WriteDSWFooter(std::ostream& fout)
{
  fout << "######################################################"
    "#########################\n\n";
  fout << "Global:\n\n";
  fout << "Package=<5>\n{{{\n}}}\n\n";
  fout << "Package=<3>\n{{{\n}}}\n\n";
  fout << "#####################################################"
    "##########################\n\n";
}


// ouput standard header for dsw file
void cmGlobalVisualStudio6Generator::WriteDSWHeader(std::ostream& fout)
{
  fout << "Microsoft Developer Studio Workspace File, Format Version 6.00\n";
  fout << "# WARNING: DO NOT EDIT OR DELETE THIS WORKSPACE FILE!\n\n";
}

//----------------------------------------------------------------------------
std::string
cmGlobalVisualStudio6Generator::WriteUtilityDepend(cmTarget const* target)
{
  std::string pname = target->GetName();
  pname += "_UTILITY";
  pname = GetVS6TargetName(pname.c_str());
  std::string fname = target->GetMakefile()->GetCurrentBinaryDirectory();
  fname += "/";
  fname += pname;
  fname += ".dsp";
  cmGeneratedFileStream fout(fname.c_str());
  fout.SetCopyIfDifferent(true);
  fout <<
    "# Microsoft Developer Studio Project File - Name=\""
       << pname << "\" - Package Owner=<4>\n"
    "# Microsoft Developer Studio Generated Build File, Format Version 6.00\n"
    "# ** DO NOT EDIT **\n"
    "\n"
    "# TARGTYPE \"Win32 (x86) Generic Project\" 0x010a\n"
    "\n"
    "CFG=" << pname << " - Win32 Debug\n"
    "!MESSAGE \"" << pname << " - Win32 Debug\""
    " (based on \"Win32 (x86) Generic Project\")\n"
    "!MESSAGE \"" << pname << " - Win32 Release\" "
    "(based on \"Win32 (x86) Generic Project\")\n"
    "!MESSAGE \"" << pname << " - Win32 MinSizeRel\" "
    "(based on \"Win32 (x86) Generic Project\")\n"
    "!MESSAGE \"" << pname << " - Win32 RelWithDebInfo\" "
    "(based on \"Win32 (x86) Generic Project\")\n"
    "\n"
    "# Begin Project\n"
    "# Begin Target\n"
    "# Name \"" << pname << " - Win32 Debug\"\n"
    "# Name \"" << pname << " - Win32 Release\"\n"
    "# Name \"" << pname << " - Win32 MinSizeRel\"\n"
    "# Name \"" << pname << " - Win32 RelWithDebInfo\"\n"
    "# End Target\n"
    "# End Project\n"
    ;
  return pname;
}

//----------------------------------------------------------------------------
void cmGlobalVisualStudio6Generator
::GetDocumentation(cmDocumentationEntry& entry)
{
  entry.Name = cmGlobalVisualStudio6Generator::GetActualName();
  entry.Brief = "Deprecated. Generates Visual Studio 6 project files.";
}

//----------------------------------------------------------------------------
void
cmGlobalVisualStudio6Generator
::AppendDirectoryForConfig(const std::string& prefix,
                           const std::string& config,
                           const std::string& suffix,
                           std::string& dir)
{
  if(!config.empty())
    {
    dir += prefix;
    dir += config;
    dir += suffix;
    }
}
