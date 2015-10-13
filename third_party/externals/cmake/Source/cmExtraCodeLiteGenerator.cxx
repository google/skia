/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2004-2009 Kitware, Inc.
  Copyright 2004 Alexander Neundorf (neundorf@kde.org)
  Copyright 2013 Eran Ifrah (eran.ifrah@gmail.com)

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "cmExtraCodeLiteGenerator.h"
#include "cmGlobalUnixMakefileGenerator3.h"
#include "cmLocalUnixMakefileGenerator3.h"
#include "cmMakefile.h"
#include "cmake.h"
#include "cmSourceFile.h"
#include "cmGeneratedFileStream.h"
#include "cmSystemTools.h"

#include <cmsys/SystemTools.hxx>
#include <cmsys/SystemInformation.hxx>
#include <cmsys/Directory.hxx>
#include "cmStandardIncludes.h"
#include "cmXMLSafe.h"

//----------------------------------------------------------------------------
void cmExtraCodeLiteGenerator::GetDocumentation(cmDocumentationEntry& entry,
                                                const std::string&) const
{
  entry.Name = this->GetName();
  entry.Brief = "Generates CodeLite project files.";
}

cmExtraCodeLiteGenerator::cmExtraCodeLiteGenerator()
  : cmExternalMakefileProjectGenerator()
  , ConfigName("NoConfig")
  , CpuCount(2)
{
#if defined(_WIN32)
  this->SupportedGlobalGenerators.push_back("MinGW Makefiles");
  this->SupportedGlobalGenerators.push_back("NMake Makefiles");
#endif
  this->SupportedGlobalGenerators.push_back("Ninja");
  this->SupportedGlobalGenerators.push_back("Unix Makefiles");
}

void cmExtraCodeLiteGenerator::Generate()
{
  // Hold root tree information for creating the workspace
  std::string workspaceProjectName;
  std::string workspaceOutputDir;
  std::string workspaceFileName;
  std::string workspaceSourcePath;
  std::string lprjdebug;

  cmGeneratedFileStream fout;

  // loop projects and locate the root project.
  // and extract the information for creating the worspace
  for (std::map<std::string, std::vector<cmLocalGenerator*> >::const_iterator
       it = this->GlobalGenerator->GetProjectMap().begin();
       it!= this->GlobalGenerator->GetProjectMap().end();
       ++it)
    {
    const cmMakefile* mf =it->second[0]->GetMakefile();
    this->ConfigName = GetConfigurationName( mf );

    if (strcmp(mf->GetCurrentBinaryDirectory(),
               mf->GetHomeOutputDirectory()) == 0)
      {
      workspaceOutputDir   = mf->GetCurrentBinaryDirectory();
      workspaceProjectName = mf->GetProjectName();
      workspaceSourcePath  = mf->GetHomeDirectory();
      workspaceFileName    = workspaceOutputDir+"/";
      workspaceFileName   += workspaceProjectName + ".workspace";
      this->WorkspacePath = mf->GetCurrentBinaryDirectory();;

      fout.Open(workspaceFileName.c_str(), false, false);
      fout << "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"
           "<CodeLite_Workspace Name=\"" << workspaceProjectName << "\" >\n";
      }
    }

  // for each sub project in the workspace create a codelite project
  for (std::map<std::string, std::vector<cmLocalGenerator*> >::const_iterator
       it = this->GlobalGenerator->GetProjectMap().begin();
       it!= this->GlobalGenerator->GetProjectMap().end();
       ++it)
    {
    // retrive project information
    const cmMakefile* mf    = it->second[0]->GetMakefile();
    std::string outputDir   = mf->GetCurrentBinaryDirectory();
    std::string projectName = mf->GetProjectName();
    std::string filename    = outputDir + "/" + projectName + ".project";

    // Make the project file relative to the workspace
    filename = cmSystemTools::RelativePath(this->WorkspacePath.c_str(),
                                          filename.c_str());

    // create a project file
    this->CreateProjectFile(it->second);
    fout << "  <Project Name=\"" << projectName << "\" Path=\""
    << filename << "\" Active=\"No\"/>\n";
    lprjdebug += "<Project Name=\"" + projectName
              + "\" ConfigName=\"" + this->ConfigName + "\"/>\n";
    }

  fout << "  <BuildMatrix>\n"
       "    <WorkspaceConfiguration Name=\""
       << this->ConfigName << "\" Selected=\"yes\">\n"
       "      " << lprjdebug << ""
       "    </WorkspaceConfiguration>\n"
       "  </BuildMatrix>\n"
       "</CodeLite_Workspace>\n";
}

/* create the project file */
void cmExtraCodeLiteGenerator::CreateProjectFile(
  const std::vector<cmLocalGenerator*>& lgs)
{
  const cmMakefile* mf    = lgs[0]->GetMakefile();
  std::string outputDir   = mf->GetCurrentBinaryDirectory();
  std::string projectName = mf->GetProjectName();
  std::string filename    = outputDir + "/";

  filename += projectName + ".project";
  this->CreateNewProjectFile(lgs, filename);
}

void cmExtraCodeLiteGenerator
::CreateNewProjectFile(const std::vector<cmLocalGenerator*>& lgs,
                       const std::string& filename)
{
  const cmMakefile* mf=lgs[0]->GetMakefile();
  cmGeneratedFileStream fout(filename.c_str());
  if(!fout)
    {
    return;
    }

  // figure out the compiler
  //std::string compiler = this->GetCBCompilerId(mf);
  std::string workspaceSourcePath = mf->GetHomeDirectory();
  std::string workspaceOutputDir =  mf->GetHomeOutputDirectory();
  std::vector<std::string> outputFiles = mf->GetOutputFiles();
  std::string projectName = mf->GetProjectName();
  std::string incDirs;
  std::vector<cmValueWithOrigin> incDirsVec =
    mf->GetIncludeDirectoriesEntries();
  std::vector<cmValueWithOrigin>::const_iterator iterInc = incDirsVec.begin();

  //std::cout << "GetIncludeDirectories:" << std::endl;
  for(; iterInc != incDirsVec.end(); ++iterInc )
    {
    //std::cout << (*ItStrVec) << std::endl;
    incDirs += iterInc->Value + " ";
    }

  ////////////////////////////////////
  fout << "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"
       "<CodeLite_Project Name=\"" << mf->GetProjectName()
       << "\" InternalType=\"\">\n";

  // Collect all used source files in the project
  // Sort them into two containers, one for C/C++ implementation files
  // which may have an acompanying header, one for all other files
  std::string projectType;

  std::map<std::string, cmSourceFile*> cFiles;
  std::set<std::string> otherFiles;
  for (std::vector<cmLocalGenerator*>::const_iterator lg=lgs.begin();
       lg!=lgs.end(); lg++)
    {
    cmMakefile* makefile=(*lg)->GetMakefile();
    cmTargets& targets=makefile->GetTargets();
    for (cmTargets::iterator ti = targets.begin();
         ti != targets.end(); ti++)
      {

      switch(ti->second.GetType())
        {
        case cmTarget::EXECUTABLE:
          {
          projectType = "Executable";
          }
        break;
        case cmTarget::STATIC_LIBRARY:
          {
          projectType = "Static Library";
          }
        break;
        case cmTarget::SHARED_LIBRARY:
          {
          projectType = "Dynamic Library";
          }
        break;
        case cmTarget::MODULE_LIBRARY:
          {
          projectType = "Dynamic Library";
          }
        break;
        default:  // intended fallthrough
          break;
        }

      switch(ti->second.GetType())
        {
        case cmTarget::EXECUTABLE:
        case cmTarget::STATIC_LIBRARY:
        case cmTarget::SHARED_LIBRARY:
        case cmTarget::MODULE_LIBRARY:
          {
          std::vector<cmSourceFile*> sources;
          ti->second.GetSourceFiles(sources,
                            makefile->GetSafeDefinition("CMAKE_BUILD_TYPE"));
          for (std::vector<cmSourceFile*>::const_iterator si=sources.begin();
               si!=sources.end(); si++)
            {
            // check whether it is a C/C++ implementation file
            bool isCFile = false;
            std::string lang = (*si)->GetLanguage();
            if (lang == "C" || lang == "CXX")
              {
              std::string srcext = (*si)->GetExtension();
              for(std::vector<std::string>::const_iterator
                  ext = mf->GetSourceExtensions().begin();
                  ext !=  mf->GetSourceExtensions().end();
                  ++ext)
                {
                if (srcext == *ext)
                  {
                  isCFile = true;
                  break;
                  }
                }
              }

            // then put it accordingly into one of the two containers
            if (isCFile)
              {
              cFiles[(*si)->GetFullPath()] = *si ;
              }
            else
              {
              otherFiles.insert((*si)->GetFullPath());
              }
            }
          }
        default:  // intended fallthrough
          break;
        }
      }
    }

  // The following loop tries to add header files matching to implementation
  // files to the project. It does that by iterating over all source files,
  // replacing the file name extension with ".h" and checks whether such a
  // file exists. If it does, it is inserted into the map of files.
  // A very similar version of that code exists also in the kdevelop
  // project generator.
  for (std::map<std::string, cmSourceFile*>::const_iterator
       sit=cFiles.begin();
       sit!=cFiles.end();
       ++sit)
    {
    std::string headerBasename=cmSystemTools::GetFilenamePath(sit->first);
    headerBasename+="/";
    headerBasename+=cmSystemTools::GetFilenameWithoutExtension(sit->first);

    // check if there's a matching header around
    for(std::vector<std::string>::const_iterator
        ext = mf->GetHeaderExtensions().begin();
        ext !=  mf->GetHeaderExtensions().end();
        ++ext)
      {
      std::string hname=headerBasename;
      hname += ".";
      hname += *ext;
      // if it's already in the set, don't check if it exists on disk
      std::set<std::string>::const_iterator headerIt=otherFiles.find(hname);
      if (headerIt != otherFiles.end())
        {
        break;
        }

      if(cmSystemTools::FileExists(hname.c_str()))
        {
        otherFiles.insert(hname);
        break;
        }
      }
    }

  // Get the project path ( we need it later to convert files to
  // their relative path)
  std::string projectPath = cmSystemTools::GetFilenamePath(filename);

  // Create 2 virtual folders: src and include
  // and place all the implementation files into the src
  // folder, the rest goes to the include folder
  fout<< "  <VirtualDirectory Name=\"src\">\n";

  // insert all source files in the codelite project
  // first the C/C++ implementation files, then all others
  for (std::map<std::string, cmSourceFile*>::const_iterator
       sit=cFiles.begin();
       sit!=cFiles.end();
       ++sit)
    {
    std::string relativePath =
      cmSystemTools::RelativePath(projectPath.c_str(), sit->first.c_str());
    fout<< "    <File Name=\"" << relativePath << "\"/>\n";
    }
  fout<< "  </VirtualDirectory>\n";
  fout<< "  <VirtualDirectory Name=\"include\">\n";
  for (std::set<std::string>::const_iterator
       sit=otherFiles.begin();
       sit!=otherFiles.end();
       ++sit)
    {
    std::string relativePath =
      cmSystemTools::RelativePath(projectPath.c_str(), sit->c_str());
    fout << "    <File Name=\"" << relativePath << "\"/>\n";
    }
  fout << "  </VirtualDirectory>\n";

  // Get the number of CPUs. We use this information for the make -jN
  // command
  cmsys::SystemInformation info;
  info.RunCPUCheck();

  this->CpuCount = info.GetNumberOfLogicalCPU() *
                   info.GetNumberOfPhysicalCPU();

  std::string cleanCommand      = GetCleanCommand(mf);
  std::string buildCommand      = GetBuildCommand(mf);
  std::string rebuildCommand    = GetRebuildCommand(mf);
  std::string singleFileCommand = GetSingleFileBuildCommand(mf);

  std::string codeliteCompilerName = this->GetCodeLiteCompilerName(mf);

  fout << "\n"
     "  <Settings Type=\"" << projectType << "\">\n"
     "    <Configuration Name=\"" << this->ConfigName << "\" CompilerType=\""
     << codeliteCompilerName << "\" DebuggerType=\"GNU gdb debugger\" "
     "Type=\""
     << projectType << "\" BuildCmpWithGlobalSettings=\"append\" "
     "BuildLnkWithGlobalSettings=\"append\" "
     "BuildResWithGlobalSettings=\"append\">\n"
     "      <Compiler Options=\"-g\" "
     "Required=\"yes\" PreCompiledHeader=\"\">\n"
     "        <IncludePath Value=\".\"/>\n"
     "      </Compiler>\n"
     "      <Linker Options=\"\" Required=\"yes\"/>\n"
     "      <ResourceCompiler Options=\"\" Required=\"no\"/>\n"
     "      <General OutputFile=\"$(IntermediateDirectory)/$(ProjectName)\" "
     "IntermediateDirectory=\"./\" Command=\"./$(ProjectName)\" "
     "CommandArguments=\"\" WorkingDirectory=\"$(IntermediateDirectory)\" "
     "PauseExecWhenProcTerminates=\"yes\"/>\n"
     "      <Debugger IsRemote=\"no\" RemoteHostName=\"\" "
     "RemoteHostPort=\"\" DebuggerPath=\"\">\n"
     "        <PostConnectCommands/>\n"
     "        <StartupCommands/>\n"
     "      </Debugger>\n"
     "      <PreBuild/>\n"
     "      <PostBuild/>\n"
     "      <CustomBuild Enabled=\"yes\">\n"
     "        <RebuildCommand>" << rebuildCommand << "</RebuildCommand>\n"
     "        <CleanCommand>" << cleanCommand << "</CleanCommand>\n"
     "        <BuildCommand>" << buildCommand << "</BuildCommand>\n"
     "        <SingleFileCommand>" << singleFileCommand
     << "</SingleFileCommand>\n"
     "        <PreprocessFileCommand/>\n"
     "        <WorkingDirectory>$(WorkspacePath)</WorkingDirectory>\n"
     "      </CustomBuild>\n"
     "      <AdditionalRules>\n"
     "        <CustomPostBuild/>\n"
     "        <CustomPreBuild/>\n"
     "      </AdditionalRules>\n"
     "    </Configuration>\n"
     "    <GlobalSettings>\n"
     "      <Compiler Options=\"\">\n"
     "        <IncludePath Value=\".\"/>\n"
     "      </Compiler>\n"
     "      <Linker Options=\"\">\n"
     "        <LibraryPath Value=\".\"/>\n"
     "      </Linker>\n"
     "      <ResourceCompiler Options=\"\"/>\n"
     "    </GlobalSettings>\n"
     "  </Settings>\n"
     "</CodeLite_Project>\n";
}

std::string
cmExtraCodeLiteGenerator::GetCodeLiteCompilerName(const cmMakefile* mf) const
{
  // figure out which language to use
  // for now care only for C and C++
  std::string compilerIdVar = "CMAKE_CXX_COMPILER_ID";
  if (this->GlobalGenerator->GetLanguageEnabled("CXX") == false)
    {
    compilerIdVar = "CMAKE_C_COMPILER_ID";
    }

  std::string compilerId = mf->GetSafeDefinition(compilerIdVar);
  std::string compiler = "gnu g++"; // default to g++

  // Since we need the compiler for parsing purposes only
  // it does not matter if we use clang or clang++, same as
  // "gnu gcc" vs "gnu g++"
  if (compilerId == "MSVC")
    {
    compiler = "VC++";
    }
  else if (compilerId == "Clang")
    {
    compiler = "clang++";
    }
  else if (compilerId == "GNU")
    {
    compiler = "gnu g++";
    }
  return compiler;
}

std::string
cmExtraCodeLiteGenerator::GetConfigurationName(const cmMakefile* mf) const
{
  std::string confName = mf->GetSafeDefinition("CMAKE_BUILD_TYPE");
  // Trim the configuration name from whitespaces (left and right)
  confName.erase(0, confName.find_first_not_of(" \t\r\v\n"));
  confName.erase(confName.find_last_not_of(" \t\r\v\n")+1);
  if ( confName.empty() )
    {
    confName = "NoConfig";
    }
  return confName;
}

std::string
cmExtraCodeLiteGenerator::GetBuildCommand(const cmMakefile* mf) const
{
  std::string generator = mf->GetSafeDefinition("CMAKE_GENERATOR");
  std::string make = mf->GetRequiredDefinition("CMAKE_MAKE_PROGRAM");
  std::string buildCommand = make; // Default
  if ( generator == "NMake Makefiles" ||
       generator == "Ninja" )
    {
    buildCommand = make;
    }
  else if ( generator == "MinGW Makefiles" ||
            generator == "Unix Makefiles" )
    {
    std::ostringstream ss;
    ss << make << " -j " << this->CpuCount;
    buildCommand = ss.str();
    }
  return buildCommand;
}

std::string
cmExtraCodeLiteGenerator::GetCleanCommand(const cmMakefile* mf) const
{
  return GetBuildCommand(mf) + " clean";
}

std::string
cmExtraCodeLiteGenerator::GetRebuildCommand(const cmMakefile* mf) const
{
  return GetCleanCommand(mf) + cmXMLSafe(" && ").str() + GetBuildCommand(mf);
}

std::string
cmExtraCodeLiteGenerator::GetSingleFileBuildCommand
(const cmMakefile* mf) const
{
  std::string buildCommand;
  std::string make = mf->GetRequiredDefinition("CMAKE_MAKE_PROGRAM");
  std::string generator = mf->GetSafeDefinition("CMAKE_GENERATOR");
  if ( generator == "Unix Makefiles" || generator == "MinGW Makefiles" )
    {
    std::ostringstream ss;
    ss << make << " -f$(ProjectPath)/Makefile $(CurrentFileName).cpp.o";
    buildCommand = ss.str();
    }
  return buildCommand;
}
