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
#include "cmExtraCodeBlocksGenerator.h"
#include "cmGlobalUnixMakefileGenerator3.h"
#include "cmLocalUnixMakefileGenerator3.h"
#include "cmMakefile.h"
#include "cmake.h"
#include "cmSourceFile.h"
#include "cmGeneratedFileStream.h"
#include "cmTarget.h"
#include "cmSystemTools.h"
#include "cmXMLSafe.h"

#include <cmsys/SystemTools.hxx>

/* Some useful URLs:
Homepage:
http://www.codeblocks.org

File format docs:
http://wiki.codeblocks.org/index.php?title=File_formats_description
http://wiki.codeblocks.org/index.php?title=Workspace_file
http://wiki.codeblocks.org/index.php?title=Project_file

Discussion:
http://forums.codeblocks.org/index.php/topic,6789.0.html
*/

//----------------------------------------------------------------------------
void cmExtraCodeBlocksGenerator
::GetDocumentation(cmDocumentationEntry& entry, const std::string&) const
{
  entry.Name = this->GetName();
  entry.Brief = "Generates CodeBlocks project files.";
}

cmExtraCodeBlocksGenerator::cmExtraCodeBlocksGenerator()
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


void cmExtraCodeBlocksGenerator::Generate()
{
  // for each sub project in the project create a codeblocks project
  for (std::map<std::string, std::vector<cmLocalGenerator*> >::const_iterator
       it = this->GlobalGenerator->GetProjectMap().begin();
      it!= this->GlobalGenerator->GetProjectMap().end();
      ++it)
    {
    // create a project file
    this->CreateProjectFile(it->second);
    }
}


/* create the project file */
void cmExtraCodeBlocksGenerator::CreateProjectFile(
                                     const std::vector<cmLocalGenerator*>& lgs)
{
  const cmMakefile* mf=lgs[0]->GetMakefile();
  std::string outputDir=mf->GetCurrentBinaryDirectory();
  std::string projectName=mf->GetProjectName();

  std::string filename=outputDir+"/";
  filename+=projectName+".cbp";
  std::string sessionFilename=outputDir+"/";
  sessionFilename+=projectName+".layout";

  this->CreateNewProjectFile(lgs, filename);
}


/* Tree is used to create a "Virtual Folder" in CodeBlocks, in which all
 CMake files this project depends on will be put. This means additionally
 to the "Sources" and "Headers" virtual folders of CodeBlocks, there will
 now also be a "CMake Files" virtual folder.
 Patch by Daniel Teske <daniel.teske AT nokia.com> (which use C::B project
 files in QtCreator).*/
struct Tree
{
  std::string path; //only one component of the path
  std::vector<Tree> folders;
  std::vector<std::string> files;
  void InsertPath(const std::vector<std::string>& splitted,
                  std::vector<std::string>::size_type start,
                  const std::string& fileName);
  void BuildVirtualFolder(std::string& virtualFolders) const;
  void BuildVirtualFolderImpl(std::string& virtualFolders,
                              const std::string& prefix) const;
  void BuildUnit(std::string& unitString, const std::string& fsPath) const;
  void BuildUnitImpl(std::string& unitString,
                     const std::string& virtualFolderPath,
                     const std::string& fsPath) const;
};


void Tree::InsertPath(const std::vector<std::string>& splitted,
                      std::vector<std::string>::size_type start,
                      const std::string& fileName)
{
  if (start == splitted.size())
    {
    files.push_back(fileName);
    return;
    }
  for (std::vector<Tree>::iterator
       it = folders.begin();
       it != folders.end();
       ++it)
    {
    if ((*it).path == splitted[start])
      {
      if (start + 1 <  splitted.size())
        {
        it->InsertPath(splitted, start + 1, fileName);
        return;
        }
      else
        {
        // last part of splitted
        it->files.push_back(fileName);
        return;
        }
      }
    }
  // Not found in folders, thus insert
  Tree newFolder;
  newFolder.path = splitted[start];
  if (start + 1 <  splitted.size())
    {
    newFolder.InsertPath(splitted, start + 1, fileName);
    folders.push_back(newFolder);
    return;
    }
  else
    {
    // last part of splitted
    newFolder.files.push_back(fileName);
    folders.push_back(newFolder);
    return;
    }
}


void Tree::BuildVirtualFolder(std::string& virtualFolders) const
{
  virtualFolders += "<Option virtualFolders=\"CMake Files\\;";
  for (std::vector<Tree>::const_iterator it = folders.begin();
     it != folders.end();
     ++it)
    {
    it->BuildVirtualFolderImpl(virtualFolders, "");
    }
  virtualFolders += "\" />";
}


void Tree::BuildVirtualFolderImpl(std::string& virtualFolders,
                                  const std::string& prefix) const
{
  virtualFolders += "CMake Files\\" + prefix +  path + "\\;";
  for (std::vector<Tree>::const_iterator it = folders.begin();
       it != folders.end();
     ++it)
    {
    it->BuildVirtualFolderImpl(virtualFolders, prefix + path + "\\");
    }
}


void Tree::BuildUnit(std::string& unitString, const std::string& fsPath) const
{
  for (std::vector<std::string>::const_iterator it = files.begin();
       it != files.end();
       ++it)
    {
    unitString += "      <Unit filename=\"" + fsPath + *it + "\">\n";
    unitString += "          <Option virtualFolder=\"CMake Files\\\" />\n";
    unitString += "      </Unit>\n";
    }
  for (std::vector<Tree>::const_iterator it = folders.begin();
     it != folders.end();
     ++it)
    {
    it->BuildUnitImpl(unitString, "", fsPath);
    }
}


void Tree::BuildUnitImpl(std::string& unitString,
                         const std::string& virtualFolderPath,
                         const std::string& fsPath) const
{
  for (std::vector<std::string>::const_iterator it = files.begin();
       it != files.end();
       ++it)
    {
    unitString += "      <Unit filename=\"" +fsPath+path+ "/" + *it + "\">\n";
    unitString += "          <Option virtualFolder=\"CMake Files\\"
               + virtualFolderPath + path + "\\\" />\n";
    unitString += "      </Unit>\n";
    }
  for (std::vector<Tree>::const_iterator it = folders.begin();
     it != folders.end();
     ++it)
    {
    it->BuildUnitImpl(unitString,
                      virtualFolderPath + path + "\\", fsPath + path + "/");
    }
}


void cmExtraCodeBlocksGenerator
  ::CreateNewProjectFile(const std::vector<cmLocalGenerator*>& lgs,
                         const std::string& filename)
{
  const cmMakefile* mf=lgs[0]->GetMakefile();
  cmGeneratedFileStream fout(filename.c_str());
  if(!fout)
    {
    return;
    }

  Tree tree;

  // build tree of virtual folders
  for (std::map<std::string, std::vector<cmLocalGenerator*> >::const_iterator
          it = this->GlobalGenerator->GetProjectMap().begin();
         it != this->GlobalGenerator->GetProjectMap().end();
         ++it)
    {
    // Collect all files
    std::vector<std::string> listFiles;
    for (std::vector<cmLocalGenerator *>::const_iterator
         jt = it->second.begin();
         jt != it->second.end();
         ++jt)
      {
      const std::vector<std::string> & files =
                                          (*jt)->GetMakefile()->GetListFiles();
      listFiles.insert(listFiles.end(), files.begin(), files.end());
      }

    // Convert
    const char* cmakeRoot = mf->GetDefinition("CMAKE_ROOT");
    for (std::vector<std::string>::const_iterator jt = listFiles.begin();
         jt != listFiles.end();
         ++jt)
      {
      // don't put cmake's own files into the project (#12110):
      if (jt->find(cmakeRoot) == 0)
        {
        continue;
        }

      const std::string &relative = cmSystemTools::RelativePath(
                         it->second[0]->GetMakefile()->GetHomeDirectory(),
                         jt->c_str());
      std::vector<std::string> splitted;
      cmSystemTools::SplitPath(relative, splitted, false);
      // Split filename from path
      std::string fileName = *(splitted.end()-1);
      splitted.erase(splitted.end() - 1, splitted.end());

      // We don't want paths with CMakeFiles in them
      // or do we?
      // In speedcrunch those where purely internal
      if (splitted.size() >= 1
          && relative.find("CMakeFiles") == std::string::npos)
        {
        tree.InsertPath(splitted, 1, fileName);
        }
      }
    }

  // Now build a virtual tree string
  std::string virtualFolders;
  tree.BuildVirtualFolder(virtualFolders);
  // And one for <Unit>
  std::string unitFiles;
  tree.BuildUnit(unitFiles, std::string(mf->GetHomeDirectory()) + "/");

  // figure out the compiler
  std::string compiler = this->GetCBCompilerId(mf);
  std::string make = mf->GetRequiredDefinition("CMAKE_MAKE_PROGRAM");

  fout<<"<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\" ?>\n"
        "<CodeBlocks_project_file>\n"
        "   <FileVersion major=\"1\" minor=\"6\" />\n"
        "   <Project>\n"
        "      <Option title=\"" << mf->GetProjectName()<<"\" />\n"
        "      <Option makefile_is_custom=\"1\" />\n"
        "      <Option compiler=\"" << compiler << "\" />\n"
        "      "<<virtualFolders<<"\n"
        "      <Build>\n";

  this->AppendTarget(fout, "all", 0, make.c_str(), mf, compiler.c_str());

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
            this->AppendTarget(fout, ti->first, 0,
                               make.c_str(), makefile, compiler.c_str());
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

          this->AppendTarget(fout, ti->first, 0,
                                 make.c_str(), makefile, compiler.c_str());
          break;
        case cmTarget::EXECUTABLE:
        case cmTarget::STATIC_LIBRARY:
        case cmTarget::SHARED_LIBRARY:
        case cmTarget::MODULE_LIBRARY:
        case cmTarget::OBJECT_LIBRARY:
          {
          this->AppendTarget(fout, ti->first, &ti->second,
                             make.c_str(), makefile, compiler.c_str());
          std::string fastTarget = ti->first;
          fastTarget += "/fast";
          this->AppendTarget(fout, fastTarget, &ti->second,
                             make.c_str(), makefile, compiler.c_str());
          }
          break;
        default:
          break;
        }
      }
    }

  fout<<"      </Build>\n";


  // Collect all used source files in the project.
  // Keep a list of C/C++ source files which might have an acompanying header
  // that should be looked for.
  typedef std::map<std::string, CbpUnit> all_files_map_t;
  all_files_map_t allFiles;
  std::vector<std::string> cFiles;

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
        case cmTarget::STATIC_LIBRARY:
        case cmTarget::SHARED_LIBRARY:
        case cmTarget::MODULE_LIBRARY:
        case cmTarget::OBJECT_LIBRARY:
        case cmTarget::UTILITY: // can have sources since 2.6.3
          {
          std::vector<cmSourceFile*> sources;
          ti->second.GetSourceFiles(sources,
                            makefile->GetSafeDefinition("CMAKE_BUILD_TYPE"));
          for (std::vector<cmSourceFile*>::const_iterator si=sources.begin();
               si!=sources.end(); si++)
            {
            // don't add source files which have the GENERATED property set:
            if ((*si)->GetPropertyAsBool("GENERATED"))
              {
              continue;
              }

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

            std::string fullPath = (*si)->GetFullPath();

            if(isCFile)
              {
              cFiles.push_back(fullPath);
              }

            CbpUnit &cbpUnit = allFiles[fullPath];
            cbpUnit.Targets.push_back(&(ti->second));
            }
          }
        default:  // intended fallthrough
          break;
        }
      }
    }

  // The following loop tries to add header files matching to implementation
  // files to the project. It does that by iterating over all
  // C/C++ source files,
  // replacing the file name extension with ".h" and checks whether such a
  // file exists. If it does, it is inserted into the map of files.
  // A very similar version of that code exists also in the kdevelop
  // project generator.
  for (std::vector<std::string>::const_iterator
       sit=cFiles.begin();
       sit!=cFiles.end();
       ++sit)
    {
    std::string const& fileName = *sit;
    std::string headerBasename=cmSystemTools::GetFilenamePath(fileName);
    headerBasename+="/";
    headerBasename+=cmSystemTools::GetFilenameWithoutExtension(fileName);

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
      if (allFiles.find(hname) != allFiles.end())
        {
        break;
        }

      if(cmSystemTools::FileExists(hname.c_str()))
        {
        allFiles[hname].Targets = allFiles[fileName].Targets;
        break;
        }
      }
    }

  // insert all source files in the CodeBlocks project
  for (all_files_map_t::const_iterator
       sit=allFiles.begin();
       sit!=allFiles.end();
       ++sit)
    {
    std::string const& unitFilename = sit->first;
    CbpUnit const& unit = sit->second;

    fout<<"      <Unit filename=\""<< cmXMLSafe(unitFilename) <<"\">\n";

    for(std::vector<const cmTarget*>::const_iterator ti = unit.Targets.begin();
      ti != unit.Targets.end(); ++ti)
      {
      std::string const& targetName = (*ti)->GetName();
      fout<<"         <Option target=\""<< cmXMLSafe(targetName) <<"\"/>\n";
      }

    fout<<"      </Unit>\n";
    }

  // Add CMakeLists.txt
  fout<<unitFiles;

  fout<<"   </Project>\n"
        "</CodeBlocks_project_file>\n";
}


// Write a dummy file for OBJECT libraries, so C::B can reference some file
std::string cmExtraCodeBlocksGenerator::CreateDummyTargetFile(
                                        cmMakefile* mf, cmTarget* target) const
{
  // this file doesn't seem to be used by C::B in custom makefile mode,
  // but we generate a unique file for each OBJECT library so in case
  // C::B uses it in some way, the targets don't interfere with each other.
  std::string filename = mf->GetCurrentBinaryDirectory();
  filename += "/";
  filename += mf->GetLocalGenerator()->GetTargetDirectory(*target);
  filename += "/";
  filename += target->GetName();
  filename += ".objlib";
  cmGeneratedFileStream fout(filename.c_str());
  if(fout)
    {
    fout << "# This is a dummy file for the OBJECT library "
         << target->GetName()
         << " for the CMake CodeBlocks project generator.\n"
         << "# Don't edit, this file will be overwritten.\n";
    }
  return filename;
}


// Generate the xml code for one target.
void cmExtraCodeBlocksGenerator::AppendTarget(cmGeneratedFileStream& fout,
                                              const std::string& targetName,
                                              cmTarget* target,
                                              const char* make,
                                              const cmMakefile* makefile,
                                              const char* compiler)
{
  std::string makefileName = makefile->GetCurrentBinaryDirectory();
  makefileName += "/Makefile";

  fout<<"      <Target title=\"" << targetName << "\">\n";
  if (target!=0)
    {
    int cbTargetType = this->GetCBTargetType(target);
    std::string workingDir = makefile->GetCurrentBinaryDirectory();
    if ( target->GetType()==cmTarget::EXECUTABLE)
      {
      // Determine the directory where the executable target is created, and
      // set the working directory to this dir.
      const char* runtimeOutputDir = makefile->GetDefinition(
                                             "CMAKE_RUNTIME_OUTPUT_DIRECTORY");
      if (runtimeOutputDir != 0)
        {
        workingDir = runtimeOutputDir;
        }
      else
        {
        const char* executableOutputDir = makefile->GetDefinition(
                                                     "EXECUTABLE_OUTPUT_PATH");
        if (executableOutputDir != 0)
          {
          workingDir = executableOutputDir;
          }
        }
      }

    std::string buildType = makefile->GetSafeDefinition("CMAKE_BUILD_TYPE");
    std::string location;
    if ( target->GetType()==cmTarget::OBJECT_LIBRARY)
      {
      location = this->CreateDummyTargetFile(const_cast<cmMakefile*>(makefile),
                                             target);
      }
    else
      {
      location = target->GetLocation(buildType);
      }

    fout<<"         <Option output=\"" << location
                            << "\" prefix_auto=\"0\" extension_auto=\"0\" />\n"
          "         <Option working_dir=\"" << workingDir << "\" />\n"
          "         <Option object_output=\"./\" />\n"
          "         <Option type=\"" << cbTargetType << "\" />\n"
          "         <Option compiler=\"" << compiler << "\" />\n"
          "         <Compiler>\n";

    cmGeneratorTarget *gtgt = this->GlobalGenerator
                                  ->GetGeneratorTarget(target);

    // the compilerdefines for this target
    std::vector<std::string> cdefs;
    target->GetCompileDefinitions(cdefs, buildType, "C");

    // Expand the list.
    for(std::vector<std::string>::const_iterator di = cdefs.begin();
        di != cdefs.end(); ++di)
      {
      cmXMLSafe safedef(di->c_str());
      fout <<"            <Add option=\"-D" << safedef.str() << "\" />\n";
      }

    // the include directories for this target
    std::set<std::string> uniqIncludeDirs;

    std::vector<std::string> includes;
    target->GetMakefile()->GetLocalGenerator()->
      GetIncludeDirectories(includes, gtgt, "C", buildType);

    uniqIncludeDirs.insert(includes.begin(), includes.end());

    std::string systemIncludeDirs = makefile->GetSafeDefinition(
                              "CMAKE_EXTRA_GENERATOR_C_SYSTEM_INCLUDE_DIRS");
    if (!systemIncludeDirs.empty())
      {
      std::vector<std::string> dirs;
      cmSystemTools::ExpandListArgument(systemIncludeDirs, dirs);
      uniqIncludeDirs.insert(dirs.begin(), dirs.end());
      }

    systemIncludeDirs = makefile->GetSafeDefinition(
                            "CMAKE_EXTRA_GENERATOR_CXX_SYSTEM_INCLUDE_DIRS");
    if (!systemIncludeDirs.empty())
      {
      std::vector<std::string> dirs;
      cmSystemTools::ExpandListArgument(systemIncludeDirs, dirs);
      uniqIncludeDirs.insert(dirs.begin(), dirs.end());
      }

    for(std::set<std::string>::const_iterator dirIt=uniqIncludeDirs.begin();
        dirIt != uniqIncludeDirs.end();
        ++dirIt)
      {
      fout <<"            <Add directory=\"" << *dirIt << "\" />\n";
      }

    fout<<"         </Compiler>\n";
    }
  else // e.g. all and the GLOBAL and UTILITY targets
    {
    fout<<"         <Option working_dir=\""
        << makefile->GetCurrentBinaryDirectory() << "\" />\n"
        <<"         <Option type=\"" << 4 << "\" />\n";
    }

  fout<<"         <MakeCommands>\n"
        "            <Build command=\""
      << this->BuildMakeCommand(make, makefileName.c_str(), targetName)
      << "\" />\n"
        "            <CompileFile command=\""
      << this->BuildMakeCommand(make, makefileName.c_str(),"&quot;$file&quot;")
      << "\" />\n"
        "            <Clean command=\""
      << this->BuildMakeCommand(make, makefileName.c_str(), "clean")
      << "\" />\n"
        "            <DistClean command=\""
      << this->BuildMakeCommand(make, makefileName.c_str(), "clean")
      << "\" />\n"
        "         </MakeCommands>\n"
        "      </Target>\n";

}


// Translate the cmake compiler id into the CodeBlocks compiler id
std::string cmExtraCodeBlocksGenerator::GetCBCompilerId(const cmMakefile* mf)
{
  // figure out which language to use
  // for now care only for C and C++
  std::string compilerIdVar = "CMAKE_CXX_COMPILER_ID";
  if (this->GlobalGenerator->GetLanguageEnabled("CXX") == false)
    {
    compilerIdVar = "CMAKE_C_COMPILER_ID";
    }

  std::string hostSystemName = mf->GetSafeDefinition("CMAKE_HOST_SYSTEM_NAME");
  std::string systemName = mf->GetSafeDefinition("CMAKE_SYSTEM_NAME");
  std::string compilerId = mf->GetSafeDefinition(compilerIdVar);
  std::string compiler = "gcc";  // default to gcc
  if (compilerId == "MSVC")
    {
    compiler = "msvc8";
    }
  else if (compilerId == "Borland")
    {
    compiler = "bcc";
    }
  else if (compilerId == "SDCC")
    {
    compiler = "sdcc";
    }
  else if (compilerId == "Intel")
    {
    compiler = "icc";
    }
  else if (compilerId == "Watcom" || compilerId == "OpenWatcom")
    {
    compiler = "ow";
    }
  else if (compilerId == "GNU")
    {
    compiler = "gcc";
    }
  return compiler;
}


// Translate the cmake target type into the CodeBlocks target type id
int cmExtraCodeBlocksGenerator::GetCBTargetType(cmTarget* target)
{
  if ( target->GetType()==cmTarget::EXECUTABLE)
    {
    if ((target->GetPropertyAsBool("WIN32_EXECUTABLE"))
        || (target->GetPropertyAsBool("MACOSX_BUNDLE")))
      {
      return 0;
      }
    else
      {
      return 1;
      }
    }
  else if (( target->GetType()==cmTarget::STATIC_LIBRARY)
        || (target->GetType()==cmTarget::OBJECT_LIBRARY))
    {
    return 2;
    }
  else if ((target->GetType()==cmTarget::SHARED_LIBRARY)
           || (target->GetType()==cmTarget::MODULE_LIBRARY))
    {
    return 3;
    }
  return 4;
}

// Create the command line for building the given target using the selected
// make
std::string cmExtraCodeBlocksGenerator::BuildMakeCommand(
             const std::string& make, const char* makefile,
             const std::string& target)
{
  std::string command = make;
  std::string generator = this->GlobalGenerator->GetName();
  if (generator == "NMake Makefiles")
    {
    // For Windows ConvertToOutputPath already adds quotes when required.
    // These need to be escaped, see
    // http://public.kitware.com/Bug/view.php?id=13952
    std::string makefileName = cmSystemTools::ConvertToOutputPath(makefile);
    command += " /NOLOGO /f ";
    command += cmXMLSafe(makefileName).str();
    command += " VERBOSE=1 ";
    command += target;
    }
  else if (generator == "MinGW Makefiles")
    {
    // no escaping of spaces in this case, see
    // http://public.kitware.com/Bug/view.php?id=10014
    std::string makefileName = makefile;
    command += " -f &quot;";
    command += makefileName;
    command += "&quot; ";
    command += " VERBOSE=1 ";
    command += target;
    }
  else if (generator == "Ninja")
    {
    command += " -v ";
    command += target;
    }
  else
    {
    std::string makefileName = cmSystemTools::ConvertToOutputPath(makefile);
    command += " -f &quot;";
    command += makefileName;
    command += "&quot; ";
    command += " VERBOSE=1 ";
    command += target;
    }
  return command;
}
