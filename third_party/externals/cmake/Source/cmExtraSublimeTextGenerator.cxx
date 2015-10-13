/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2004-2009 Kitware, Inc.
  Copyright 2004 Alexander Neundorf (neundorf@kde.org)

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "cmExtraSublimeTextGenerator.h"
#include "cmake.h"
#include "cmGeneratedFileStream.h"
#include "cmGeneratorTarget.h"
#include "cmGlobalUnixMakefileGenerator3.h"
#include "cmLocalGenerator.h"
#include "cmLocalUnixMakefileGenerator3.h"
#include "cmMakefile.h"
#include "cmSourceFile.h"
#include "cmSystemTools.h"
#include "cmTarget.h"
#include "cmXMLSafe.h"

#include <cmsys/SystemTools.hxx>

/*
Sublime Text 2 Generator
Author: Morné Chamberlain
This generator was initially based off of the CodeBlocks generator.

Some useful URLs:
Homepage:
http://www.sublimetext.com/

File format docs:
http://www.sublimetext.com/docs/2/projects.html
http://sublimetext.info/docs/en/reference/build_systems.html
*/

//----------------------------------------------------------------------------
void cmExtraSublimeTextGenerator
::GetDocumentation(cmDocumentationEntry& entry, const std::string&) const
{
  entry.Name = this->GetName();
  entry.Brief = "Generates Sublime Text 2 project files.";
}

cmExtraSublimeTextGenerator::cmExtraSublimeTextGenerator()
:cmExternalMakefileProjectGenerator()
{
#if defined(_WIN32)
  this->SupportedGlobalGenerators.push_back("MinGW Makefiles");
  this->SupportedGlobalGenerators.push_back("NMake Makefiles");
// disable until somebody actually tests it:
//  this->SupportedGlobalGenerators.push_back("MSYS Makefiles");
#endif
  this->SupportedGlobalGenerators.push_back("Ninja");
  this->SupportedGlobalGenerators.push_back("Unix Makefiles");
}


void cmExtraSublimeTextGenerator::Generate()
{
  // for each sub project in the project create a sublime text 2 project
  for (std::map<std::string, std::vector<cmLocalGenerator*> >::const_iterator
       it = this->GlobalGenerator->GetProjectMap().begin();
      it!= this->GlobalGenerator->GetProjectMap().end();
      ++it)
    {
    // create a project file
    this->CreateProjectFile(it->second);
    }
}


void cmExtraSublimeTextGenerator::CreateProjectFile(
                                     const std::vector<cmLocalGenerator*>& lgs)
{
  const cmMakefile* mf=lgs[0]->GetMakefile();
  std::string outputDir=mf->GetCurrentBinaryDirectory();
  std::string projectName=mf->GetProjectName();

  const std::string filename =
                     outputDir + "/" + projectName + ".sublime-project";

  this->CreateNewProjectFile(lgs, filename);
}

void cmExtraSublimeTextGenerator
  ::CreateNewProjectFile(const std::vector<cmLocalGenerator*>& lgs,
                         const std::string& filename)
{
  const cmMakefile* mf=lgs[0]->GetMakefile();
  cmGeneratedFileStream fout(filename.c_str());
  if(!fout)
    {
    return;
    }

  const std::string &sourceRootRelativeToOutput = cmSystemTools::RelativePath(
                     mf->GetHomeOutputDirectory(),
                     mf->GetHomeDirectory());
  // Write the folder entries to the project file
  fout << "{\n";
  fout << "\t\"folders\":\n\t[\n\t";
  if (!sourceRootRelativeToOutput.empty())
    {
      fout << "\t{\n\t\t\t\"path\": \"" << sourceRootRelativeToOutput << "\"";
      const std::string &outputRelativeToSourceRoot =
        cmSystemTools::RelativePath(mf->GetHomeDirectory(),
                                    mf->GetHomeOutputDirectory());
      if ((!outputRelativeToSourceRoot.empty()) &&
        ((outputRelativeToSourceRoot.length() < 3) ||
          (outputRelativeToSourceRoot.substr(0, 3) != "../")))
        {
        fout << ",\n\t\t\t\"folder_exclude_patterns\": [\"" <<
                outputRelativeToSourceRoot << "\"]";
        }
    }
  else
    {
      fout << "\t{\n\t\t\t\"path\": \"./\"";
    }
  fout << "\n\t\t}";
  // End of the folders section
  fout << "\n\t]";

  // Write the beginning of the build systems section to the project file
  fout << ",\n\t\"build_systems\":\n\t[\n\t";

  // Set of include directories over all targets (sublime text/sublimeclang
  // doesn't currently support these settings per build system, only project
  // wide
  MapSourceFileFlags sourceFileFlags;
  AppendAllTargets(lgs, mf, fout, sourceFileFlags);

  // End of build_systems
  fout << "\n\t]";
  fout << "\n\t}";
}


void cmExtraSublimeTextGenerator::
  AppendAllTargets(const std::vector<cmLocalGenerator*>& lgs,
                   const cmMakefile* mf,
                   cmGeneratedFileStream& fout,
                   MapSourceFileFlags& sourceFileFlags)
{
  std::string make = mf->GetRequiredDefinition("CMAKE_MAKE_PROGRAM");
  std::string compiler = "";
  if (!lgs.empty())
    {
      this->AppendTarget(fout, "all", lgs[0], 0, make.c_str(), mf,
                         compiler.c_str(), sourceFileFlags, true);
      this->AppendTarget(fout, "clean", lgs[0], 0, make.c_str(), mf,
                         compiler.c_str(), sourceFileFlags, false);
    }

  // add all executable and library targets and some of the GLOBAL
  // and UTILITY targets
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
        case cmTarget::GLOBAL_TARGET:
          {
          // Only add the global targets from CMAKE_BINARY_DIR,
          // not from the subdirs
          if (strcmp(makefile->GetCurrentBinaryDirectory(),
                     makefile->GetHomeOutputDirectory())==0)
            {
            this->AppendTarget(fout, ti->first, *lg, 0,
                               make.c_str(), makefile, compiler.c_str(),
                               sourceFileFlags, false);
            }
          }
          break;
        case cmTarget::UTILITY:
          // Add all utility targets, except the Nightly/Continuous/
          // Experimental-"sub"targets as e.g. NightlyStart
          if (((ti->first.find("Nightly")==0)   &&(ti->first!="Nightly"))
             || ((ti->first.find("Continuous")==0)&&(ti->first!="Continuous"))
             || ((ti->first.find("Experimental")==0)
                                               && (ti->first!="Experimental")))
            {
            break;
            }

          this->AppendTarget(fout, ti->first, *lg, 0,
                             make.c_str(), makefile, compiler.c_str(),
                             sourceFileFlags, false);
          break;
        case cmTarget::EXECUTABLE:
        case cmTarget::STATIC_LIBRARY:
        case cmTarget::SHARED_LIBRARY:
        case cmTarget::MODULE_LIBRARY:
        case cmTarget::OBJECT_LIBRARY:
          {
          this->AppendTarget(fout, ti->first, *lg, &ti->second,
                             make.c_str(), makefile, compiler.c_str(),
                             sourceFileFlags, false);
          std::string fastTarget = ti->first;
          fastTarget += "/fast";
          this->AppendTarget(fout, fastTarget, *lg, &ti->second,
                             make.c_str(), makefile, compiler.c_str(),
                             sourceFileFlags, false);
          }
          break;
        default:
          break;
        }
      }
    }
}

void cmExtraSublimeTextGenerator::
  AppendTarget(cmGeneratedFileStream& fout,
               const std::string& targetName,
               cmLocalGenerator* lg,
               cmTarget* target,
               const char* make,
               const cmMakefile* makefile,
               const char*, //compiler
               MapSourceFileFlags& sourceFileFlags,
               bool firstTarget)
{

  if (target != 0)
    {
      cmGeneratorTarget *gtgt = this->GlobalGenerator
                                    ->GetGeneratorTarget(target);
      std::vector<cmSourceFile*> sourceFiles;
      target->GetSourceFiles(sourceFiles,
                             makefile->GetSafeDefinition("CMAKE_BUILD_TYPE"));
      std::vector<cmSourceFile*>::const_iterator sourceFilesEnd =
        sourceFiles.end();
      for (std::vector<cmSourceFile*>::const_iterator iter =
        sourceFiles.begin(); iter != sourceFilesEnd; ++iter)
        {
          cmSourceFile* sourceFile = *iter;
          MapSourceFileFlags::iterator sourceFileFlagsIter =
            sourceFileFlags.find(sourceFile->GetFullPath());
          if (sourceFileFlagsIter == sourceFileFlags.end())
            {
            sourceFileFlagsIter =
              sourceFileFlags.insert(MapSourceFileFlags::value_type(
                sourceFile->GetFullPath(), std::vector<std::string>())).first;
            }
          std::vector<std::string>& flags = sourceFileFlagsIter->second;
          std::string flagsString =
            this->ComputeFlagsForObject(*iter, lg, target, gtgt);
          std::string definesString =
            this->ComputeDefines(*iter, lg, target, gtgt);
          flags.clear();
          cmsys::RegularExpression flagRegex;
          // Regular expression to extract compiler flags from a string
          // https://gist.github.com/3944250
          const char* regexString =
            "(^|[ ])-[DIOUWfgs][^= ]+(=\\\"[^\"]+\\\"|=[^\"][^ ]+)?";
          flagRegex.compile(regexString);
          std::string workString = flagsString + " " + definesString;
          while (flagRegex.find(workString))
            {
              std::string::size_type start = flagRegex.start();
              if (workString[start] == ' ')
                {
                  start++;
                }
              flags.push_back(workString.substr(start,
                flagRegex.end() - start));
              if (flagRegex.end() < workString.size())
                {
                workString = workString.substr(flagRegex.end());
                }
                else
                {
                workString = "";
                }
            }
        }
    }

  // Ninja uses ninja.build files (look for a way to get the output file name
  // from cmMakefile or something)
  std::string makefileName;
  if (this->GlobalGenerator->GetName() == "Ninja")
    {
      makefileName = "build.ninja";
    }
    else
    {
      makefileName = "Makefile";
    }
  if (!firstTarget)
    {
    fout << ",\n\t";
    }
  fout << "\t{\n\t\t\t\"name\": \"" << makefile->GetProjectName() << " - " <<
          targetName << "\",\n";
  fout << "\t\t\t\"cmd\": [" <<
          this->BuildMakeCommand(make, makefileName.c_str(), targetName) <<
          "],\n";
  fout << "\t\t\t\"working_dir\": \"${project_path}\",\n";
  fout << "\t\t\t\"file_regex\": \"^(..[^:]*):([0-9]+):?([0-9]+)?:? (.*)$\"\n";
  fout << "\t\t}";
}

// Create the command line for building the given target using the selected
// make
std::string cmExtraSublimeTextGenerator::BuildMakeCommand(
             const std::string& make, const char* makefile,
             const std::string& target)
{
  std::string command = "\"";
  command += make + "\"";
  std::string generator = this->GlobalGenerator->GetName();
  if (generator == "NMake Makefiles")
    {
    std::string makefileName = cmSystemTools::ConvertToOutputPath(makefile);
    command += ", \"/NOLOGO\", \"/f\", \"";
    command += makefileName + "\"";
    command += ", \"VERBOSE=1\", \"";
    command += target;
    command += "\"";
    }
  else if (generator == "Ninja")
    {
    std::string makefileName = cmSystemTools::ConvertToOutputPath(makefile);
    command += ", \"-f\", \"";
    command += makefileName + "\"";
    command += ", \"-v\", \"";
    command += target;
    command += "\"";
    }
  else
    {
    std::string makefileName;
    if (generator == "MinGW Makefiles")
      {
        // no escaping of spaces in this case, see
        // http://public.kitware.com/Bug/view.php?id=10014
        makefileName = makefile;
      }
      else
      {
        makefileName = cmSystemTools::ConvertToOutputPath(makefile);
      }
    command += ", \"-f\", \"";
    command += makefileName + "\"";
    command += ", \"VERBOSE=1\", \"";
    command += target;
    command += "\"";
    }
  return command;
}

// TODO: Most of the code is picked up from the Ninja generator, refactor it.
std::string
cmExtraSublimeTextGenerator::ComputeFlagsForObject(cmSourceFile* source,
                                                   cmLocalGenerator* lg,
                                                   cmTarget *target,
                                                   cmGeneratorTarget* gtgt)
{
  std::string flags;

  cmMakefile *makefile = lg->GetMakefile();
  std::string language = source->GetLanguage();
  if (language.empty())
   {
   language = "C";
   }
  const std::string& config = makefile->GetSafeDefinition("CMAKE_BUILD_TYPE");
  // Add language-specific flags.
  lg->AddLanguageFlags(flags, language, config);

  lg->AddArchitectureFlags(flags, gtgt, language, config);

  // TODO: Fortran support.
  // // Fortran-specific flags computed for this target.
  // if(*l == "Fortran")
  //   {
  //   this->AddFortranFlags(flags);
  //   }

  // Add shared-library flags if needed.
  lg->AddCMP0018Flags(flags, target, language, config);

  // Add include directory flags.
  {
  std::vector<std::string> includes;
  lg->GetIncludeDirectories(includes, gtgt, language, config);
  std::string includeFlags =
    lg->GetIncludeFlags(includes, gtgt, language, true); // full include paths
  lg->AppendFlags(flags, includeFlags);
  }

  // Append old-style preprocessor definition flags.
  lg->AppendFlags(flags, makefile->GetDefineFlags());

  // Add target-specific flags.
  lg->AddCompileOptions(flags, target, language, config);

  // Add source file specific flags.
  lg->AppendFlags(flags, source->GetProperty("COMPILE_FLAGS"));

  // TODO: Handle Apple frameworks.

  return flags;
}

// TODO: Refactor with
// void cmMakefileTargetGenerator::WriteTargetLanguageFlags().
std::string
cmExtraSublimeTextGenerator::
ComputeDefines(cmSourceFile *source, cmLocalGenerator* lg, cmTarget *target,
               cmGeneratorTarget*)

{
  std::set<std::string> defines;
  cmMakefile *makefile = lg->GetMakefile();
  const std::string& language = source->GetLanguage();
  const std::string& config = makefile->GetSafeDefinition("CMAKE_BUILD_TYPE");

  // Add the export symbol definition for shared library objects.
  if(const char* exportMacro = target->GetExportMacro())
    {
    lg->AppendDefines(defines, exportMacro);
    }

  // Add preprocessor definitions for this target and configuration.
  lg->AddCompileDefinitions(defines, target, config, language);
  lg->AppendDefines(defines, source->GetProperty("COMPILE_DEFINITIONS"));
  {
  std::string defPropName = "COMPILE_DEFINITIONS_";
  defPropName += cmSystemTools::UpperCase(config);
  lg->AppendDefines(defines, source->GetProperty(defPropName));
  }

  std::string definesString;
  lg->JoinDefines(defines, definesString, language);

  return definesString;
}
