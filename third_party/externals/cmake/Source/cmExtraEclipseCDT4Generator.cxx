/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2004-2009 Kitware, Inc.
  Copyright 2004 Alexander Neundorf (neundorf@kde.org)
  Copyright 2007 Miguel A. Figueroa-Villanueva

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "cmExtraEclipseCDT4Generator.h"
#include "cmGlobalUnixMakefileGenerator3.h"
#include "cmLocalUnixMakefileGenerator3.h"
#include "cmMakefile.h"
#include "cmGeneratedFileStream.h"
#include "cmState.h"
#include "cmTarget.h"
#include "cmSourceFile.h"

#include "cmSystemTools.h"
#include <stdlib.h>
#include <assert.h>

//----------------------------------------------------------------------------
cmExtraEclipseCDT4Generator
::cmExtraEclipseCDT4Generator() : cmExternalMakefileProjectGenerator()
{
// TODO: Verify if __CYGWIN__ should be checked.
//#if defined(_WIN32) && !defined(__CYGWIN__)
#if defined(_WIN32)
  this->SupportedGlobalGenerators.push_back("NMake Makefiles");
  this->SupportedGlobalGenerators.push_back("MinGW Makefiles");
//  this->SupportedGlobalGenerators.push_back("MSYS Makefiles");
#endif
  this->SupportedGlobalGenerators.push_back("Ninja");
  this->SupportedGlobalGenerators.push_back("Unix Makefiles");

  this->SupportsVirtualFolders = true;
  this->GenerateLinkedResources = true;
  this->SupportsGmakeErrorParser = true;
  this->SupportsMachO64Parser = true;
}

//----------------------------------------------------------------------------
void cmExtraEclipseCDT4Generator
::GetDocumentation(cmDocumentationEntry& entry, const std::string&) const
{
  entry.Name = this->GetName();
  entry.Brief = "Generates Eclipse CDT 4.0 project files.";
}

//----------------------------------------------------------------------------
void cmExtraEclipseCDT4Generator
::EnableLanguage(std::vector<std::string> const& languages,
                 cmMakefile *, bool)
{
  for (std::vector<std::string>::const_iterator lit = languages.begin();
       lit != languages.end(); ++lit)
    {
    if (*lit == "CXX")
      {
      this->Natures.insert("org.eclipse.cdt.core.ccnature");
      this->Natures.insert("org.eclipse.cdt.core.cnature");
      }
    else if (*lit == "C")
      {
      this->Natures.insert("org.eclipse.cdt.core.cnature");
      }
    else if (*lit == "Java")
      {
      this->Natures.insert("org.eclipse.jdt.core.javanature");
      }
    }
}

//----------------------------------------------------------------------------
void cmExtraEclipseCDT4Generator::Generate()
{
  const cmMakefile* mf
    = this->GlobalGenerator->GetLocalGenerators()[0]->GetMakefile();

  std::string eclipseVersion = mf->GetSafeDefinition("CMAKE_ECLIPSE_VERSION");
  cmsys::RegularExpression regex(".*([0-9]+\\.[0-9]+).*");
  if (regex.find(eclipseVersion.c_str()))
    {
    unsigned int majorVersion = 0;
    unsigned int minorVersion = 0;
    int res=sscanf(regex.match(1).c_str(), "%u.%u", &majorVersion,
                                                    &minorVersion);
    if (res == 2)
      {
      int version = majorVersion * 1000 + minorVersion;
      if (version < 3006) // 3.6 is Helios
        {
        this->SupportsVirtualFolders = false;
        this->SupportsMachO64Parser = false;
        }
      if (version < 3007) // 3.7 is Indigo
        {
        this->SupportsGmakeErrorParser = false;
        }
      }
    }

  // TODO: Decide if these are local or member variables
  this->HomeDirectory       = mf->GetHomeDirectory();
  this->HomeOutputDirectory = mf->GetHomeOutputDirectory();

  this->GenerateLinkedResources = mf->IsOn(
                                    "CMAKE_ECLIPSE_GENERATE_LINKED_RESOURCES");

  this->IsOutOfSourceBuild = (this->HomeDirectory!=this->HomeOutputDirectory);

  this->GenerateSourceProject = (this->IsOutOfSourceBuild &&
                            mf->IsOn("CMAKE_ECLIPSE_GENERATE_SOURCE_PROJECT"));

  if ((this->GenerateSourceProject == false)
    && (mf->IsOn("ECLIPSE_CDT4_GENERATE_SOURCE_PROJECT")))
    {
    mf->IssueMessage(cmake::WARNING,
              "ECLIPSE_CDT4_GENERATE_SOURCE_PROJECT is set to TRUE, "
              "but this variable is not supported anymore since CMake 2.8.7.\n"
              "Enable CMAKE_ECLIPSE_GENERATE_SOURCE_PROJECT instead.");
    }

  if (cmSystemTools::IsSubDirectory(this->HomeOutputDirectory,
                                    this->HomeDirectory))
    {
    mf->IssueMessage(cmake::WARNING, "The build directory is a subdirectory "
                     "of the source directory.\n"
                     "This is not supported well by Eclipse. It is strongly "
                     "recommended to use a build directory which is a "
                     "sibling of the source directory.");
    }

  // NOTE: This is not good, since it pollutes the source tree. However,
  //       Eclipse doesn't allow CVS/SVN to work when the .project is not in
  //       the cvs/svn root directory. Hence, this is provided as an option.
  if (this->GenerateSourceProject)
    {
    // create .project file in the source tree
    this->CreateSourceProjectFile();
    }

  // create a .project file
  this->CreateProjectFile();

  // create a .cproject file
  this->CreateCProjectFile();
}

void cmExtraEclipseCDT4Generator::CreateSourceProjectFile()
{
  assert(this->HomeDirectory != this->HomeOutputDirectory);

  // set up the project name: <project>-Source@<baseSourcePathName>
  const cmMakefile* mf
     = this->GlobalGenerator->GetLocalGenerators()[0]->GetMakefile();
  std::string name = this->GenerateProjectName(mf->GetProjectName(), "Source",
                                   this->GetPathBasename(this->HomeDirectory));

  const std::string filename = this->HomeDirectory + "/.project";
  cmGeneratedFileStream fout(filename.c_str());
  if (!fout)
    {
    return;
    }

  fout <<
    "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
    "<projectDescription>\n"
    "\t<name>" << this->EscapeForXML(name) << "</name>\n"
    "\t<comment></comment>\n"
    "\t<projects>\n"
    "\t</projects>\n"
    "\t<buildSpec>\n"
    "\t</buildSpec>\n"
    "\t<natures>\n"
    "\t</natures>\n"
    "\t<linkedResources>\n";

  if (this->SupportsVirtualFolders)
    {
    this->CreateLinksToSubprojects(fout, this->HomeDirectory);
    this->SrcLinkedResources.clear();
    }

  fout <<
    "\t</linkedResources>\n"
    "</projectDescription>\n"
    ;
}


//----------------------------------------------------------------------------
void cmExtraEclipseCDT4Generator::AddEnvVar(cmGeneratedFileStream& fout,
                                            const char* envVar, cmMakefile* mf)
{
  // get the variables from the environment and from the cache and then
  // figure out which one to use:

  const char* envVarValue = getenv(envVar);

  std::string cacheEntryName = "CMAKE_ECLIPSE_ENVVAR_";
  cacheEntryName += envVar;
  const char* cacheValue = mf->GetState()->GetInitializedCacheValue(
                                                       cacheEntryName);

  // now we have both, decide which one to use
  std::string valueToUse;
  if (envVarValue==0 && cacheValue==0)
    {
    // nothing known, do nothing
    valueToUse = "";
    }
  else if (envVarValue!=0 && cacheValue==0)
    {
    // The variable is in the env, but not in the cache. Use it and put it
    // in the cache
    valueToUse = envVarValue;
    mf->AddCacheDefinition(cacheEntryName, valueToUse.c_str(),
                           cacheEntryName.c_str(), cmState::STRING,
                           true);
    mf->GetCMakeInstance()->SaveCache(mf->GetHomeOutputDirectory());
    }
  else if (envVarValue==0 && cacheValue!=0)
    {
    // It is already in the cache, but not in the env, so use it from the cache
    valueToUse = cacheValue;
    }
  else
    {
    // It is both in the cache and in the env.
    // Use the version from the env. except if the value from the env is
    // completely contained in the value from the cache (for the case that we
    // now have a PATH without MSVC dirs in the env. but had the full PATH with
    // all MSVC dirs during the cmake run which stored the var in the cache:
    valueToUse = cacheValue;
    if (valueToUse.find(envVarValue) == std::string::npos)
      {
      valueToUse = envVarValue;
      mf->AddCacheDefinition(cacheEntryName, valueToUse.c_str(),
                             cacheEntryName.c_str(), cmState::STRING,
                             true);
      mf->GetCMakeInstance()->SaveCache(mf->GetHomeOutputDirectory());
      }
    }

  if (!valueToUse.empty())
    {
    fout << envVar << "=" << valueToUse << "|";
    }
}


//----------------------------------------------------------------------------
void cmExtraEclipseCDT4Generator::CreateProjectFile()
{
  cmMakefile* mf
    = this->GlobalGenerator->GetLocalGenerators()[0]->GetMakefile();

  const std::string filename = this->HomeOutputDirectory + "/.project";

  cmGeneratedFileStream fout(filename.c_str());
  if (!fout)
    {
    return;
    }

  std::string compilerId = mf->GetSafeDefinition("CMAKE_C_COMPILER_ID");
  if (compilerId.empty())  // no C compiler, try the C++ compiler:
    {
    compilerId = mf->GetSafeDefinition("CMAKE_CXX_COMPILER_ID");
    }

  fout <<
    "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
    "<projectDescription>\n"
    "\t<name>" <<
    this->GenerateProjectName(mf->GetProjectName(),
                              mf->GetSafeDefinition("CMAKE_BUILD_TYPE"),
                              this->GetPathBasename(this->HomeOutputDirectory))
    << "</name>\n"
    "\t<comment></comment>\n"
    "\t<projects>\n"
    "\t</projects>\n"
    "\t<buildSpec>\n"
    "\t\t<buildCommand>\n"
    "\t\t\t<name>org.eclipse.cdt.make.core.makeBuilder</name>\n"
    "\t\t\t<triggers>clean,full,incremental,</triggers>\n"
    "\t\t\t<arguments>\n"
    ;

  // use clean target
  fout <<
    "\t\t\t\t<dictionary>\n"
    "\t\t\t\t\t<key>org.eclipse.cdt.make.core.cleanBuildTarget</key>\n"
    "\t\t\t\t\t<value>clean</value>\n"
    "\t\t\t\t</dictionary>\n"
    "\t\t\t\t<dictionary>\n"
    "\t\t\t\t\t<key>org.eclipse.cdt.make.core.enableCleanBuild</key>\n"
    "\t\t\t\t\t<value>true</value>\n"
    "\t\t\t\t</dictionary>\n"
    "\t\t\t\t<dictionary>\n"
    "\t\t\t\t\t<key>org.eclipse.cdt.make.core.append_environment</key>\n"
    "\t\t\t\t\t<value>true</value>\n"
    "\t\t\t\t</dictionary>\n"
    "\t\t\t\t<dictionary>\n"
    "\t\t\t\t\t<key>org.eclipse.cdt.make.core.stopOnError</key>\n"
    "\t\t\t\t\t<value>true</value>\n"
    "\t\t\t\t</dictionary>\n"
    ;

  // set the make command
  std::string make = mf->GetRequiredDefinition("CMAKE_MAKE_PROGRAM");
  const std::string makeArgs = mf->GetSafeDefinition(
                                               "CMAKE_ECLIPSE_MAKE_ARGUMENTS");

  fout <<
    "\t\t\t\t<dictionary>\n"
    "\t\t\t\t\t<key>org.eclipse.cdt.make.core.enabledIncrementalBuild</key>\n"
    "\t\t\t\t\t<value>true</value>\n"
    "\t\t\t\t</dictionary>\n"
    "\t\t\t\t<dictionary>\n"
    "\t\t\t\t\t<key>org.eclipse.cdt.make.core.build.command</key>\n"
    "\t\t\t\t\t<value>" << this->GetEclipsePath(make) << "</value>\n"
    "\t\t\t\t</dictionary>\n"
    "\t\t\t\t<dictionary>\n"
    "\t\t\t\t\t<key>org.eclipse.cdt.make.core.contents</key>\n"
    "\t\t\t\t\t<value>org.eclipse.cdt.make.core.activeConfigSettings</value>\n"
    "\t\t\t\t</dictionary>\n"
    "\t\t\t\t<dictionary>\n"
    "\t\t\t\t\t<key>org.eclipse.cdt.make.core.build.target.inc</key>\n"
    "\t\t\t\t\t<value>all</value>\n"
    "\t\t\t\t</dictionary>\n"
    "\t\t\t\t<dictionary>\n"
    "\t\t\t\t\t<key>org.eclipse.cdt.make.core.build.arguments</key>\n"
    "\t\t\t\t\t<value>" << makeArgs << "</value>\n"
    "\t\t\t\t</dictionary>\n"
    "\t\t\t\t<dictionary>\n"
    "\t\t\t\t\t<key>org.eclipse.cdt.make.core.buildLocation</key>\n"
    "\t\t\t\t\t<value>"
     << this->GetEclipsePath(this->HomeOutputDirectory) << "</value>\n"
    "\t\t\t\t</dictionary>\n"
    "\t\t\t\t<dictionary>\n"
    "\t\t\t\t\t<key>org.eclipse.cdt.make.core.useDefaultBuildCmd</key>\n"
    "\t\t\t\t\t<value>false</value>\n"
    "\t\t\t\t</dictionary>\n"
    ;

  // set project specific environment
  fout <<
    "\t\t\t\t<dictionary>\n"
    "\t\t\t\t\t<key>org.eclipse.cdt.make.core.environment</key>\n"
    "\t\t\t\t\t<value>VERBOSE=1|CMAKE_NO_VERBOSE=1|"  //verbose Makefile output
    ;
  // set vsvars32.bat environment available at CMake time,
  //   but not necessarily when eclipse is open
  if (compilerId == "MSVC")
    {
    AddEnvVar(fout, "PATH", mf);
    AddEnvVar(fout, "INCLUDE", mf);
    AddEnvVar(fout, "LIB", mf);
    AddEnvVar(fout, "LIBPATH", mf);
    }
  else if (compilerId == "Intel")
    {
    // if the env.var is set, use this one and put it in the cache
    // if the env.var is not set, but the value is in the cache,
    // use it from the cache:
    AddEnvVar(fout, "INTEL_LICENSE_FILE", mf);
    }
  fout <<
    "</value>\n"
    "\t\t\t\t</dictionary>\n"
    ;

  fout <<
    "\t\t\t\t<dictionary>\n"
    "\t\t\t\t\t<key>org.eclipse.cdt.make.core.enableFullBuild</key>\n"
    "\t\t\t\t\t<value>true</value>\n"
    "\t\t\t\t</dictionary>\n"
    "\t\t\t\t<dictionary>\n"
    "\t\t\t\t\t<key>org.eclipse.cdt.make.core.build.target.auto</key>\n"
    "\t\t\t\t\t<value>all</value>\n"
    "\t\t\t\t</dictionary>\n"
    "\t\t\t\t<dictionary>\n"
    "\t\t\t\t\t<key>org.eclipse.cdt.make.core.enableAutoBuild</key>\n"
    "\t\t\t\t\t<value>false</value>\n"
    "\t\t\t\t</dictionary>\n"
    "\t\t\t\t<dictionary>\n"
    "\t\t\t\t\t<key>org.eclipse.cdt.make.core.build.target.clean</key>\n"
    "\t\t\t\t\t<value>clean</value>\n"
    "\t\t\t\t</dictionary>\n"
    "\t\t\t\t<dictionary>\n"
    "\t\t\t\t\t<key>org.eclipse.cdt.make.core.fullBuildTarget</key>\n"
    "\t\t\t\t\t<value>all</value>\n"
    "\t\t\t\t</dictionary>\n"
    "\t\t\t\t<dictionary>\n"
    "\t\t\t\t\t<key>org.eclipse.cdt.make.core.buildArguments</key>\n"
    "\t\t\t\t\t<value></value>\n"
    "\t\t\t\t</dictionary>\n"
    "\t\t\t\t<dictionary>\n"
    "\t\t\t\t\t<key>org.eclipse.cdt.make.core.build.location</key>\n"
    "\t\t\t\t\t<value>"
    << this->GetEclipsePath(this->HomeOutputDirectory) << "</value>\n"
    "\t\t\t\t</dictionary>\n"
    "\t\t\t\t<dictionary>\n"
    "\t\t\t\t\t<key>org.eclipse.cdt.make.core.autoBuildTarget</key>\n"
    "\t\t\t\t\t<value>all</value>\n"
    "\t\t\t\t</dictionary>\n"
    ;

  // set error parsers
  fout <<
    "\t\t\t\t<dictionary>\n"
    "\t\t\t\t\t<key>org.eclipse.cdt.core.errorOutputParser</key>\n"
    "\t\t\t\t\t<value>"
    ;
  if (compilerId == "MSVC")
    {
    fout << "org.eclipse.cdt.core.VCErrorParser;";
    }
  else if (compilerId == "Intel")
    {
    fout << "org.eclipse.cdt.core.ICCErrorParser;";
    }

  if (this->SupportsGmakeErrorParser)
    {
    fout << "org.eclipse.cdt.core.GmakeErrorParser;";
    }
  else
    {
    fout << "org.eclipse.cdt.core.MakeErrorParser;";
    }

  fout <<
    "org.eclipse.cdt.core.GCCErrorParser;"
    "org.eclipse.cdt.core.GASErrorParser;"
    "org.eclipse.cdt.core.GLDErrorParser;"
    "</value>\n"
    "\t\t\t\t</dictionary>\n"
    ;

  fout <<
    "\t\t\t</arguments>\n"
    "\t\t</buildCommand>\n"
    "\t\t<buildCommand>\n"
    "\t\t\t<name>org.eclipse.cdt.make.core.ScannerConfigBuilder</name>\n"
    "\t\t\t<arguments>\n"
    "\t\t\t</arguments>\n"
    "\t\t</buildCommand>\n"
    "\t</buildSpec>\n"
    ;

  // set natures for c/c++ projects
  fout <<
    "\t<natures>\n"
    "\t\t<nature>org.eclipse.cdt.make.core.makeNature</nature>\n"
    "\t\t<nature>org.eclipse.cdt.make.core.ScannerConfigNature</nature>\n";

  for (std::set<std::string>::const_iterator nit=this->Natures.begin();
       nit != this->Natures.end(); ++nit)
    {
    fout << "\t\t<nature>" << *nit << "</nature>\n";
    }

  if (const char *extraNaturesProp = mf->GetState()
        ->GetGlobalProperty("ECLIPSE_EXTRA_NATURES"))
    {
    std::vector<std::string> extraNatures;
    cmSystemTools::ExpandListArgument(extraNaturesProp, extraNatures);
    for (std::vector<std::string>::const_iterator nit = extraNatures.begin();
         nit != extraNatures.end(); ++nit)
      {
      fout << "\t\t<nature>" << *nit << "</nature>\n";
      }
    }

  fout << "\t</natures>\n";

  fout << "\t<linkedResources>\n";
  // create linked resources
  if (this->IsOutOfSourceBuild)
    {
    // create a linked resource to CMAKE_SOURCE_DIR
    // (this is not done anymore for each project because of
    // http://public.kitware.com/Bug/view.php?id=9978 and because I found it
    // actually quite confusing in bigger projects with many directories and
    // projects, Alex

    std::string sourceLinkedResourceName = "[Source directory]";
    std::string linkSourceDirectory = this->GetEclipsePath(
                                             mf->GetCurrentSourceDirectory());
    // .project dir can't be subdir of a linked resource dir
    if (!cmSystemTools::IsSubDirectory(this->HomeOutputDirectory,
                                         linkSourceDirectory))
      {
      this->AppendLinkedResource(fout, sourceLinkedResourceName,
                                 this->GetEclipsePath(linkSourceDirectory),
                                 LinkToFolder);
      this->SrcLinkedResources.push_back(sourceLinkedResourceName);
      }

    }

  if (this->SupportsVirtualFolders)
    {
    this->CreateLinksToSubprojects(fout, this->HomeOutputDirectory);

    this->CreateLinksForTargets(fout);
    }

  fout << "\t</linkedResources>\n";

  fout << "</projectDescription>\n";
}


//----------------------------------------------------------------------------
void cmExtraEclipseCDT4Generator::CreateLinksForTargets(
                                                   cmGeneratedFileStream& fout)
{
  std::string linkName = "[Targets]";
  this->AppendLinkedResource(fout, linkName, "virtual:/virtual",VirtualFolder);

  for (std::vector<cmLocalGenerator*>::const_iterator
       lgIt = this->GlobalGenerator->GetLocalGenerators().begin();
       lgIt != this->GlobalGenerator->GetLocalGenerators().end();
       ++lgIt)
    {
    cmMakefile* makefile = (*lgIt)->GetMakefile();
    const cmTargets& targets = makefile->GetTargets();

    for(cmTargets::const_iterator ti=targets.begin(); ti!=targets.end();++ti)
      {
      std::string linkName2 = linkName;
      linkName2 += "/";
      switch(ti->second.GetType())
        {
        case cmTarget::EXECUTABLE:
        case cmTarget::STATIC_LIBRARY:
        case cmTarget::SHARED_LIBRARY:
        case cmTarget::MODULE_LIBRARY:
        case cmTarget::OBJECT_LIBRARY:
          {
          const char* prefix = (ti->second.GetType()==cmTarget::EXECUTABLE ?
                                                          "[exe] " : "[lib] ");
          linkName2 += prefix;
          linkName2 += ti->first;
          this->AppendLinkedResource(fout, linkName2, "virtual:/virtual",
                                     VirtualFolder);
          if (!this->GenerateLinkedResources)
            {
            break; // skip generating the linked resources to the source files
            }
          std::vector<cmSourceGroup> sourceGroups=makefile->GetSourceGroups();
          // get the files from the source lists then add them to the groups
          cmTarget* tgt = const_cast<cmTarget*>(&ti->second);
          std::vector<cmSourceFile*> files;
          tgt->GetSourceFiles(files,
                            makefile->GetSafeDefinition("CMAKE_BUILD_TYPE"));
          for(std::vector<cmSourceFile*>::const_iterator sfIt = files.begin();
              sfIt != files.end();
              sfIt++)
            {
            // Add the file to the list of sources.
            std::string source = (*sfIt)->GetFullPath();
            cmSourceGroup* sourceGroup =
                       makefile->FindSourceGroup(source.c_str(), sourceGroups);
            sourceGroup->AssignSource(*sfIt);
            }

          for(std::vector<cmSourceGroup>::iterator sgIt = sourceGroups.begin();
              sgIt != sourceGroups.end();
              ++sgIt)
            {
            std::string linkName3 = linkName2;
            linkName3 += "/";
            linkName3 += sgIt->GetFullName();
            this->AppendLinkedResource(fout, linkName3, "virtual:/virtual",
                                       VirtualFolder);

            std::vector<const cmSourceFile*> sFiles = sgIt->GetSourceFiles();
            for(std::vector<const cmSourceFile*>::const_iterator fileIt =
                                                                sFiles.begin();
                fileIt != sFiles.end();
                ++fileIt)
              {
              std::string fullPath = (*fileIt)->GetFullPath();
              if (!cmSystemTools::FileIsDirectory(fullPath))
                {
                std::string linkName4 = linkName3;
                linkName4 += "/";
                linkName4 += cmSystemTools::GetFilenameName(fullPath);
                this->AppendLinkedResource(fout, linkName4,
                                           fullPath, LinkToFile);
                }
              }
            }
          }
          break;
        // ignore all others:
        default:
          break;
        }
      }
    }
}


//----------------------------------------------------------------------------
void cmExtraEclipseCDT4Generator::CreateLinksToSubprojects(
                       cmGeneratedFileStream& fout, const std::string& baseDir)
{
  if (!this->GenerateLinkedResources)
    {
    return;
    }

  // for each sub project create a linked resource to the source dir
  // - only if it is an out-of-source build
  this->AppendLinkedResource(fout, "[Subprojects]",
                             "virtual:/virtual", VirtualFolder);

  for (std::map<std::string, std::vector<cmLocalGenerator*> >::const_iterator
       it = this->GlobalGenerator->GetProjectMap().begin();
       it != this->GlobalGenerator->GetProjectMap().end();
       ++it)
    {
    std::string linkSourceDirectory = this->GetEclipsePath(
                   it->second[0]->GetMakefile()->GetCurrentSourceDirectory());
    // a linked resource must not point to a parent directory of .project or
    // .project itself
    if ((baseDir != linkSourceDirectory) &&
        !cmSystemTools::IsSubDirectory(baseDir,
                                       linkSourceDirectory))
      {
      std::string linkName = "[Subprojects]/";
      linkName += it->first;
      this->AppendLinkedResource(fout, linkName,
                                 this->GetEclipsePath(linkSourceDirectory),
                                 LinkToFolder
                                );
      // Don't add it to the srcLinkedResources, because listing multiple
      // directories confuses the Eclipse indexer (#13596).
      }
    }
}


//----------------------------------------------------------------------------
void cmExtraEclipseCDT4Generator::AppendIncludeDirectories(
                            cmGeneratedFileStream& fout,
                            const std::vector<std::string>& includeDirs,
                            std::set<std::string>& emittedDirs)
{
  for(std::vector<std::string>::const_iterator inc = includeDirs.begin();
      inc != includeDirs.end();
      ++inc)
    {
    if (!inc->empty())
      {
      std::string dir = cmSystemTools::CollapseFullPath(*inc);

      // handle framework include dirs on OSX, the remainder after the
      // Frameworks/ part has to be stripped
      //   /System/Library/Frameworks/GLUT.framework/Headers
      cmsys::RegularExpression frameworkRx("(.+/Frameworks)/.+\\.framework/");
      if(frameworkRx.find(dir.c_str()))
        {
        dir = frameworkRx.match(1);
        }

      if(emittedDirs.find(dir) == emittedDirs.end())
        {
        emittedDirs.insert(dir);
        fout << "<pathentry include=\""
             << cmExtraEclipseCDT4Generator::EscapeForXML(
                              cmExtraEclipseCDT4Generator::GetEclipsePath(dir))
             << "\" kind=\"inc\" path=\"\" system=\"true\"/>\n";
        }
      }
    }
}

//----------------------------------------------------------------------------
void cmExtraEclipseCDT4Generator::CreateCProjectFile() const
{
  std::set<std::string> emmited;

  const cmMakefile* mf
    = this->GlobalGenerator->GetLocalGenerators()[0]->GetMakefile();

  const std::string filename = this->HomeOutputDirectory + "/.cproject";

  cmGeneratedFileStream fout(filename.c_str());
  if (!fout)
    {
    return;
    }

  // add header
  fout <<
    "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>\n"
    "<?fileVersion 4.0.0?>\n\n"
    "<cproject>\n"
    "<storageModule moduleId=\"org.eclipse.cdt.core.settings\">\n"
    ;

  fout << "<cconfiguration id=\"org.eclipse.cdt.core.default.config.1\">\n";

  // Configuration settings...
  fout <<
    "<storageModule"
    " buildSystemId=\"org.eclipse.cdt.core.defaultConfigDataProvider\""
    " id=\"org.eclipse.cdt.core.default.config.1\""
    " moduleId=\"org.eclipse.cdt.core.settings\" name=\"Configuration\">\n"
    "<externalSettings/>\n"
    "<extensions>\n"
    ;
  // TODO: refactor this out...
  std::string executableFormat = mf->GetSafeDefinition(
                                                    "CMAKE_EXECUTABLE_FORMAT");
  if (executableFormat == "ELF")
    {
    fout << "<extension id=\"org.eclipse.cdt.core.ELF\""
            " point=\"org.eclipse.cdt.core.BinaryParser\"/>\n"
            ;
    fout << "<extension id=\"org.eclipse.cdt.core.GNU_ELF\""
            " point=\"org.eclipse.cdt.core.BinaryParser\">\n"
            "<attribute key=\"addr2line\" value=\"addr2line\"/>\n"
            "<attribute key=\"c++filt\" value=\"c++filt\"/>\n"
            "</extension>\n"
            ;
    }
  else
    {
    std::string systemName = mf->GetSafeDefinition("CMAKE_SYSTEM_NAME");
    if (systemName == "CYGWIN")
      {
      fout << "<extension id=\"org.eclipse.cdt.core.Cygwin_PE\""
              " point=\"org.eclipse.cdt.core.BinaryParser\">\n"
              "<attribute key=\"addr2line\" value=\"addr2line\"/>\n"
              "<attribute key=\"c++filt\" value=\"c++filt\"/>\n"
              "<attribute key=\"cygpath\" value=\"cygpath\"/>\n"
              "<attribute key=\"nm\" value=\"nm\"/>\n"
              "</extension>\n"
              ;
      }
    else if (systemName == "Windows")
      {
      fout << "<extension id=\"org.eclipse.cdt.core.PE\""
              " point=\"org.eclipse.cdt.core.BinaryParser\"/>\n"
              ;
      }
    else if (systemName == "Darwin")
      {
      fout << "<extension id=\"" <<
           (this->SupportsMachO64Parser ? "org.eclipse.cdt.core.MachO64"
                                        : "org.eclipse.cdt.core.MachO") << "\""
              " point=\"org.eclipse.cdt.core.BinaryParser\">\n"
              "<attribute key=\"c++filt\" value=\"c++filt\"/>\n"
              "</extension>\n"
              ;
      }
    else
      {
      // *** Should never get here ***
      fout << "<error_toolchain_type/>\n";
      }
    }

  fout << "</extensions>\n"
          "</storageModule>\n"
          ;

  // ???
  fout <<
    "<storageModule moduleId=\"org.eclipse.cdt.core.language.mapping\">\n"
    "<project-mappings/>\n"
    "</storageModule>\n"
    ;

  // ???
  fout<<"<storageModule moduleId=\"org.eclipse.cdt.core.externalSettings\"/>\n"
          ;

  // set the path entries (includes, libs, source dirs, etc.)
  fout << "<storageModule moduleId=\"org.eclipse.cdt.core.pathentry\">\n"
          ;
  // for each sub project with a linked resource to the source dir:
  // - make it type 'src'
  // - and exclude it from type 'out'
  std::string excludeFromOut;
/* I don't know what the pathentry kind="src" are good for, e.g. autocompletion
 * works also without them. Done wrong, the indexer complains, see #12417
 * and #12213.
 * According to #13596, this entry at least limits the directories the
 * indexer is searching for files. So now the "src" entry contains only
 * the linked resource to CMAKE_SOURCE_DIR.
 * The CDT documentation is very terse on that:
 * "CDT_SOURCE: Entry kind constant describing a path entry identifying a
 * folder containing source code to be compiled."
 * Also on the cdt-dev list didn't bring any information:
 * http://web.archiveorange.com/archive/v/B4NlJDNIpYoOS1SbxFNy
 * Alex */

  for (std::vector<std::string>::const_iterator
       it = this->SrcLinkedResources.begin();
       it != this->SrcLinkedResources.end();
       ++it)
    {
    fout << "<pathentry kind=\"src\" path=\"" << this->EscapeForXML(*it)
         << "\"/>\n";

    // exlude source directory from output search path
    // - only if not named the same as an output directory
    if (!cmSystemTools::FileIsDirectory(
           std::string(this->HomeOutputDirectory + "/" + *it)))
      {
      excludeFromOut += this->EscapeForXML(*it) + "/|";
      }
    }

  excludeFromOut += "**/CMakeFiles/";
  fout << "<pathentry excluding=\"" << excludeFromOut
       << "\" kind=\"out\" path=\"\"/>\n";

  // add pre-processor definitions to allow eclipse to gray out sections
  emmited.clear();
  for (std::vector<cmLocalGenerator*>::const_iterator
        it = this->GlobalGenerator->GetLocalGenerators().begin();
       it != this->GlobalGenerator->GetLocalGenerators().end();
       ++it)
    {

    if(const char* cdefs = (*it)->GetMakefile()->GetProperty(
                                                        "COMPILE_DEFINITIONS"))
      {
      // Expand the list.
      std::vector<std::string> defs;
      cmGeneratorExpression::Split(cdefs, defs);

      for(std::vector<std::string>::const_iterator di = defs.begin();
          di != defs.end(); ++di)
        {
        if (cmGeneratorExpression::Find(*di) != std::string::npos)
          {
          continue;
          }

        std::string::size_type equals = di->find('=', 0);
        std::string::size_type enddef = di->length();

        std::string def;
        std::string val;
        if (equals != std::string::npos && equals < enddef)
          {
          // we have -DFOO=BAR
          def = di->substr(0, equals);
          val = di->substr(equals + 1, enddef - equals + 1);
          }
        else
          {
          // we have -DFOO
          def = *di;
          }

        // insert the definition if not already added.
        if(emmited.find(def) == emmited.end())
          {
          emmited.insert(def);
          fout << "<pathentry kind=\"mac\" name=\"" << def
               << "\" path=\"\" value=\"" << this->EscapeForXML(val)
               << "\"/>\n";
          }
        }
      }
    }
  // add system defined c macros
  const char* cDefs=mf->GetDefinition(
                              "CMAKE_EXTRA_GENERATOR_C_SYSTEM_DEFINED_MACROS");
  if(cDefs)
    {
    // Expand the list.
    std::vector<std::string> defs;
    cmSystemTools::ExpandListArgument(cDefs, defs, true);

    // the list must contain only definition-value pairs:
    if ((defs.size() % 2) == 0)
      {
      std::vector<std::string>::const_iterator di = defs.begin();
      while (di != defs.end())
        {
        std::string def = *di;
        ++di;
        std::string val;
        if (di != defs.end())
          {
          val = *di;
          ++di;
          }

        // insert the definition if not already added.
        if(emmited.find(def) == emmited.end())
          {
          emmited.insert(def);
          fout << "<pathentry kind=\"mac\" name=\"" << def
               << "\" path=\"\" value=\"" << this->EscapeForXML(val)
               << "\"/>\n";
          }
        }
      }
    }
  // add system defined c++ macros
  const char* cxxDefs = mf->GetDefinition(
                            "CMAKE_EXTRA_GENERATOR_CXX_SYSTEM_DEFINED_MACROS");
  if(cxxDefs)
    {
    // Expand the list.
    std::vector<std::string> defs;
    cmSystemTools::ExpandListArgument(cxxDefs, defs, true);

    // the list must contain only definition-value pairs:
    if ((defs.size() % 2) == 0)
      {
      std::vector<std::string>::const_iterator di = defs.begin();
      while (di != defs.end())
        {
        std::string def = *di;
        ++di;
        std::string val;
        if (di != defs.end())
          {
          val = *di;
          ++di;
          }

        // insert the definition if not already added.
        if(emmited.find(def) == emmited.end())
          {
          emmited.insert(def);
          fout << "<pathentry kind=\"mac\" name=\"" << def
               << "\" path=\"\" value=\"" << this->EscapeForXML(val)
               << "\"/>\n";
          }
        }
      }
    }

  // include dirs
  emmited.clear();
  for (std::vector<cmLocalGenerator*>::const_iterator
        it = this->GlobalGenerator->GetLocalGenerators().begin();
       it != this->GlobalGenerator->GetLocalGenerators().end();
       ++it)
    {
    cmGeneratorTargetsType targets = (*it)->GetMakefile()
                                        ->GetGeneratorTargets();
    for (cmGeneratorTargetsType::iterator l = targets.begin();
         l != targets.end(); ++l)
      {
      if (l->first->IsImported())
        {
        continue;
        }
      std::vector<std::string> includeDirs;
      std::string config = mf->GetSafeDefinition("CMAKE_BUILD_TYPE");
      (*it)->GetIncludeDirectories(includeDirs, l->second, "C", config);
      this->AppendIncludeDirectories(fout, includeDirs, emmited);
      }
    }
  // now also the system include directories, in case we found them in
  // CMakeSystemSpecificInformation.cmake. This makes Eclipse find the
  // standard headers.
  std::string compiler = mf->GetSafeDefinition("CMAKE_C_COMPILER");
  if (!compiler.empty())
    {
    std::string systemIncludeDirs = mf->GetSafeDefinition(
                                "CMAKE_EXTRA_GENERATOR_C_SYSTEM_INCLUDE_DIRS");
    std::vector<std::string> dirs;
    cmSystemTools::ExpandListArgument(systemIncludeDirs, dirs);
    this->AppendIncludeDirectories(fout, dirs, emmited);
    }
  compiler = mf->GetSafeDefinition("CMAKE_CXX_COMPILER");
  if (!compiler.empty())
    {
    std::string systemIncludeDirs = mf->GetSafeDefinition(
                              "CMAKE_EXTRA_GENERATOR_CXX_SYSTEM_INCLUDE_DIRS");
    std::vector<std::string> dirs;
    cmSystemTools::ExpandListArgument(systemIncludeDirs, dirs);
    this->AppendIncludeDirectories(fout, dirs, emmited);
    }

  fout << "</storageModule>\n";

  // add build targets
  fout <<
    "<storageModule moduleId=\"org.eclipse.cdt.make.core.buildtargets\">\n"
    "<buildTargets>\n"
    ;
  emmited.clear();
  const std::string make = mf->GetRequiredDefinition("CMAKE_MAKE_PROGRAM");
  const std::string makeArgs = mf->GetSafeDefinition(
                                               "CMAKE_ECLIPSE_MAKE_ARGUMENTS");

  cmGlobalGenerator* generator
    = const_cast<cmGlobalGenerator*>(this->GlobalGenerator);

  std::string allTarget;
  std::string cleanTarget;
  if (generator->GetAllTargetName())
    {
    allTarget = generator->GetAllTargetName();
    }
  if (generator->GetCleanTargetName())
    {
    cleanTarget = generator->GetCleanTargetName();
    }

  // add all executable and library targets and some of the GLOBAL
  // and UTILITY targets
  for (std::vector<cmLocalGenerator*>::const_iterator
        it = this->GlobalGenerator->GetLocalGenerators().begin();
       it != this->GlobalGenerator->GetLocalGenerators().end();
       ++it)
    {
    const cmTargets& targets = (*it)->GetMakefile()->GetTargets();
    cmMakefile* makefile=(*it)->GetMakefile();
    std::string subdir = (*it)->Convert(makefile->GetCurrentBinaryDirectory(),
                           cmLocalGenerator::HOME_OUTPUT);
    if (subdir == ".")
      {
      subdir = "";
      }

    for(cmTargets::const_iterator ti=targets.begin(); ti!=targets.end(); ++ti)
      {
      switch(ti->second.GetType())
        {
        case cmTarget::GLOBAL_TARGET:
          {
          // Only add the global targets from CMAKE_BINARY_DIR,
          // not from the subdirs
          if (subdir.empty())
           {
           this->AppendTarget(fout, ti->first, make, makeArgs, subdir, ": ");
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

         this->AppendTarget(fout, ti->first, make, makeArgs, subdir, ": ");
         break;
       case cmTarget::EXECUTABLE:
       case cmTarget::STATIC_LIBRARY:
       case cmTarget::SHARED_LIBRARY:
       case cmTarget::MODULE_LIBRARY:
       case cmTarget::OBJECT_LIBRARY:
         {
         const char* prefix = (ti->second.GetType()==cmTarget::EXECUTABLE ?
                                                          "[exe] " : "[lib] ");
         this->AppendTarget(fout, ti->first, make, makeArgs, subdir, prefix);
         std::string fastTarget = ti->first;
         fastTarget += "/fast";
         this->AppendTarget(fout, fastTarget, make, makeArgs, subdir, prefix);

         // Add Build and Clean targets in the virtual folder of targets:
         if (this->SupportsVirtualFolders)
          {
          std::string virtDir = "[Targets]/";
          virtDir += prefix;
          virtDir += ti->first;
          std::string buildArgs = "-C \"";
          buildArgs += makefile->GetHomeOutputDirectory();
          buildArgs += "\" ";
          buildArgs += makeArgs;
          this->AppendTarget(fout, "Build", make, buildArgs, virtDir, "",
                             ti->first.c_str());

          std::string cleanArgs = "-E chdir \"";
          cleanArgs += makefile->GetCurrentBinaryDirectory();
          cleanArgs += "\" \"";
          cleanArgs += cmSystemTools::GetCMakeCommand();
          cleanArgs += "\" -P \"";
          cleanArgs += (*it)->GetTargetDirectory(ti->second);
          cleanArgs += "/cmake_clean.cmake\"";
          this->AppendTarget(fout, "Clean", cmSystemTools::GetCMakeCommand(),
                             cleanArgs, virtDir, "", "");
          }
         }
         break;
        default:
          break;
        }
      }

    // insert the all and clean targets in every subdir
    if (!allTarget.empty())
      {
      this->AppendTarget(fout, allTarget, make, makeArgs, subdir, ": ");
      }
    if (!cleanTarget.empty())
      {
      this->AppendTarget(fout, cleanTarget, make, makeArgs, subdir, ": ");
      }

    //insert rules for compiling, preprocessing and assembling individual files
    std::vector<std::string> objectFileTargets;
    (*it)->GetIndividualFileTargets(objectFileTargets);
    for(std::vector<std::string>::const_iterator fit=objectFileTargets.begin();
        fit != objectFileTargets.end();
        ++fit)
      {
      const char* prefix = "[obj] ";
      if ((*fit)[fit->length()-1] == 's')
        {
        prefix = "[to asm] ";
        }
      else if ((*fit)[fit->length()-1] == 'i')
        {
        prefix = "[pre] ";
        }
      this->AppendTarget(fout, *fit, make, makeArgs, subdir, prefix);
      }
    }

  fout << "</buildTargets>\n"
          "</storageModule>\n"
          ;

  this->AppendStorageScanners(fout, *mf);

  fout << "</cconfiguration>\n"
          "</storageModule>\n"
          "<storageModule moduleId=\"cdtBuildSystem\" version=\"4.0.0\">\n"
          "<project id=\"" << this->EscapeForXML(mf->GetProjectName())
       << ".null.1\" name=\"" << this->EscapeForXML(mf->GetProjectName())
       << "\"/>\n"
          "</storageModule>\n"
          "</cproject>\n"
          ;
}

//----------------------------------------------------------------------------
std::string
cmExtraEclipseCDT4Generator::GetEclipsePath(const std::string& path)
{
#if defined(__CYGWIN__)
  std::string cmd = "cygpath -m " + path;
  std::string out;
  if (!cmSystemTools::RunSingleCommand(cmd.c_str(), &out, &out))
    {
    return path;
    }
  else
    {
    out.erase(out.find_last_of('\n'));
    return out;
    }
#else
  return path;
#endif
}

std::string
cmExtraEclipseCDT4Generator::GetPathBasename(const std::string& path)
{
  std::string outputBasename = path;
  while (!outputBasename.empty() &&
         (outputBasename[outputBasename.size() - 1] == '/' ||
          outputBasename[outputBasename.size() - 1] == '\\'))
    {
    outputBasename.resize(outputBasename.size() - 1);
    }
  std::string::size_type loc = outputBasename.find_last_of("/\\");
  if (loc != std::string::npos)
    {
    outputBasename = outputBasename.substr(loc + 1);
    }

  return outputBasename;
}

std::string
cmExtraEclipseCDT4Generator::GenerateProjectName(const std::string& name,
                                                 const std::string& type,
                                                 const std::string& path)
{
  return cmExtraEclipseCDT4Generator::EscapeForXML(name)
                                +(type.empty() ? "" : "-") + type + "@" + path;
}

std::string cmExtraEclipseCDT4Generator::EscapeForXML(const std::string& value)
{
  std::string str = value;
  cmSystemTools::ReplaceString(str, "&", "&amp;");
  cmSystemTools::ReplaceString(str, "<", "&lt;");
  cmSystemTools::ReplaceString(str, ">", "&gt;");
  cmSystemTools::ReplaceString(str, "\"", "&quot;");
  // NOTE: This one is not necessary, since as of Eclipse CDT4 it will
  //       automatically change this to the original value (').
  //cmSystemTools::ReplaceString(str, "'", "&apos;");
  return str;
}

//----------------------------------------------------------------------------
// Helper functions
//----------------------------------------------------------------------------
void cmExtraEclipseCDT4Generator
::AppendStorageScanners(cmGeneratedFileStream& fout,
                        const cmMakefile& makefile)
{
  // we need the "make" and the C (or C++) compiler which are used, Alex
  std::string make = makefile.GetRequiredDefinition("CMAKE_MAKE_PROGRAM");
  std::string compiler = makefile.GetSafeDefinition("CMAKE_C_COMPILER");
  std::string arg1 = makefile.GetSafeDefinition("CMAKE_C_COMPILER_ARG1");
  if (compiler.empty())
    {
    compiler = makefile.GetSafeDefinition("CMAKE_CXX_COMPILER");
    arg1 = makefile.GetSafeDefinition("CMAKE_CXX_COMPILER_ARG1");
    }
  if (compiler.empty())  //Hmm, what to do now ?
    {
    compiler = "gcc";
    }

  // the following right now hardcodes gcc behaviour :-/
  std::string compilerArgs =
                         "-E -P -v -dD ${plugin_state_location}/${specs_file}";
  if (!arg1.empty())
    {
    arg1 += " ";
    compilerArgs = arg1 + compilerArgs;
    }

  fout <<
    "<storageModule moduleId=\"scannerConfiguration\">\n"
    "<autodiscovery enabled=\"true\" problemReportingEnabled=\"true\""
    " selectedProfileId="
    "\"org.eclipse.cdt.make.core.GCCStandardMakePerProjectProfile\"/>\n"
    ;
  cmExtraEclipseCDT4Generator::AppendScannerProfile(fout,
    "org.eclipse.cdt.make.core.GCCStandardMakePerProjectProfile",
    true, "", true, "specsFile",
    compilerArgs,
    compiler, true, true);
  cmExtraEclipseCDT4Generator::AppendScannerProfile(fout,
    "org.eclipse.cdt.make.core.GCCStandardMakePerFileProfile",
    true, "", true, "makefileGenerator",
    "-f ${project_name}_scd.mk",
    make, true, true);

  fout << "</storageModule>\n";
}

// The prefix is prepended before the actual name of the target. The purpose
// of that is to sort the targets in the view of Eclipse, so that at first
// the global/utility/all/clean targets appear ": ", then the executable
// targets "[exe] ", then the libraries "[lib]", then the rules for the
// object files "[obj]", then for preprocessing only "[pre] " and
// finally the assembly files "[to asm] ". Note the "to" in "to asm",
// without it, "asm" would be the first targets in the list, with the "to"
// they are the last targets, which makes more sense.
void cmExtraEclipseCDT4Generator::AppendTarget(cmGeneratedFileStream& fout,
                                               const std::string&     target,
                                               const std::string&     make,
                                               const std::string&     makeArgs,
                                               const std::string&     path,
                                               const char* prefix,
                                               const char* makeTarget
                                              )
{
  std::string targetXml = cmExtraEclipseCDT4Generator::EscapeForXML(target);
  std::string makeTargetXml = targetXml;
  if (makeTarget != NULL)
    {
    makeTargetXml = cmExtraEclipseCDT4Generator::EscapeForXML(makeTarget);
    }
  cmExtraEclipseCDT4Generator::EscapeForXML(target);
  std::string pathXml = cmExtraEclipseCDT4Generator::EscapeForXML(path);
  fout <<
    "<target name=\"" << prefix << targetXml << "\""
    " path=\"" << pathXml << "\""
    " targetID=\"org.eclipse.cdt.make.MakeTargetBuilder\">\n"
    "<buildCommand>"
    << cmExtraEclipseCDT4Generator::GetEclipsePath(make)
    << "</buildCommand>\n"
    "<buildArguments>"  << makeArgs << "</buildArguments>\n"
    "<buildTarget>" << makeTargetXml << "</buildTarget>\n"
    "<stopOnError>true</stopOnError>\n"
    "<useDefaultCommand>false</useDefaultCommand>\n"
    "</target>\n"
    ;
}

void cmExtraEclipseCDT4Generator
::AppendScannerProfile(cmGeneratedFileStream& fout,
                       const std::string&     profileID,
                       bool                   openActionEnabled,
                       const std::string&     openActionFilePath,
                       bool                   pParserEnabled,
                       const std::string&     scannerInfoProviderID,
                       const std::string&     runActionArguments,
                       const std::string&     runActionCommand,
                       bool                   runActionUseDefault,
                       bool                   sipParserEnabled)
{
  fout <<
    "<profile id=\"" << profileID << "\">\n"
    "<buildOutputProvider>\n"
    "<openAction enabled=\"" << (openActionEnabled ? "true" : "false")
    << "\" filePath=\"" << openActionFilePath << "\"/>\n"
    "<parser enabled=\"" << (pParserEnabled ? "true" : "false") << "\"/>\n"
    "</buildOutputProvider>\n"
    "<scannerInfoProvider id=\"" << scannerInfoProviderID << "\">\n"
    "<runAction arguments=\"" << runActionArguments << "\""
    " command=\"" << runActionCommand
    << "\" useDefault=\"" << (runActionUseDefault ? "true":"false") << "\"/>\n"
    "<parser enabled=\"" << (sipParserEnabled ? "true" : "false") << "\"/>\n"
    "</scannerInfoProvider>\n"
    "</profile>\n"
    ;
}

void cmExtraEclipseCDT4Generator
::AppendLinkedResource (cmGeneratedFileStream& fout,
                        const std::string&     name,
                        const std::string&     path,
                        LinkType linkType)
{
  const char* locationTag = "location";
  const char* typeTag = "2";
  if (linkType == VirtualFolder) // ... and not a linked folder
    {
    locationTag = "locationURI";
    }
  if (linkType == LinkToFile)
    {
    typeTag = "1";
    }

  fout <<
    "\t\t<link>\n"
    "\t\t\t<name>"
    << cmExtraEclipseCDT4Generator::EscapeForXML(name)
    << "</name>\n"
    "\t\t\t<type>" << typeTag << "</type>\n"
    "\t\t\t<" << locationTag << ">"
    << cmExtraEclipseCDT4Generator::EscapeForXML(path)
    << "</" << locationTag << ">\n"
    "\t\t</link>\n"
    ;
}
