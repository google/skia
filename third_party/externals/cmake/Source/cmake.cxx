/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "cmake.h"
#include "cmCacheManager.h"
#include "cmMakefile.h"
#include "cmLocalGenerator.h"
#include "cmExternalMakefileProjectGenerator.h"
#include "cmCommands.h"
#include "cmCommand.h"
#include "cmFileTimeComparison.h"
#include "cmSourceFile.h"
#include "cmTest.h"
#include "cmDocumentationFormatter.h"
#include "cmAlgorithms.h"
#include "cmState.h"

#if defined(CMAKE_BUILD_WITH_CMAKE)
# include "cmGraphVizWriter.h"
# include "cmVariableWatch.h"
# include <cmsys/SystemInformation.hxx>
#endif

#include <cmsys/Glob.hxx>
#include <cmsys/RegularExpression.hxx>
#include <cmsys/FStream.hxx>

// only build kdevelop generator on non-windows platforms
// when not bootstrapping cmake
#if !defined(_WIN32)
# if defined(CMAKE_BUILD_WITH_CMAKE)
#   define CMAKE_USE_KDEVELOP
# endif
#endif

#if defined(CMAKE_BUILD_WITH_CMAKE)
#  define CMAKE_USE_ECLIPSE
#endif

#if defined(__MINGW32__) && !defined(CMAKE_BUILD_WITH_CMAKE)
# define CMAKE_BOOT_MINGW
#endif

// include the generator
#if defined(_WIN32) && !defined(__CYGWIN__)
#  if !defined(CMAKE_BOOT_MINGW)
#    include "cmGlobalVisualStudio6Generator.h"
#    include "cmGlobalVisualStudio7Generator.h"
#    include "cmGlobalVisualStudio71Generator.h"
#    include "cmGlobalVisualStudio8Generator.h"
#    include "cmGlobalVisualStudio9Generator.h"
#    include "cmGlobalVisualStudio10Generator.h"
#    include "cmGlobalVisualStudio11Generator.h"
#    include "cmGlobalVisualStudio12Generator.h"
#    include "cmGlobalVisualStudio14Generator.h"
#    include "cmGlobalBorlandMakefileGenerator.h"
#    include "cmGlobalNMakeMakefileGenerator.h"
#    include "cmGlobalJOMMakefileGenerator.h"
#    include "cmGlobalGhsMultiGenerator.h"
#    define CMAKE_HAVE_VS_GENERATORS
#  endif
#  include "cmGlobalMSYSMakefileGenerator.h"
#  include "cmGlobalMinGWMakefileGenerator.h"
#else
#endif
#if defined(CMAKE_USE_WMAKE)
# include "cmGlobalWatcomWMakeGenerator.h"
#endif
#include "cmGlobalUnixMakefileGenerator3.h"
#include "cmGlobalNinjaGenerator.h"
#include "cmExtraCodeLiteGenerator.h"

#if !defined(CMAKE_BOOT_MINGW)
# include "cmExtraCodeBlocksGenerator.h"
#endif
#include "cmExtraSublimeTextGenerator.h"
#include "cmExtraKateGenerator.h"

#ifdef CMAKE_USE_KDEVELOP
# include "cmGlobalKdevelopGenerator.h"
#endif

#ifdef CMAKE_USE_ECLIPSE
# include "cmExtraEclipseCDT4Generator.h"
#endif

#include <stdlib.h> // required for atoi

#if defined( __APPLE__ )
#  if defined(CMAKE_BUILD_WITH_CMAKE)
#    include "cmGlobalXCodeGenerator.h"
#    define CMAKE_USE_XCODE 1
#  endif
#  include <sys/types.h>
#  include <sys/time.h>
#  include <sys/resource.h>
#endif

#include <sys/stat.h> // struct stat

#include <list>

static bool cmakeCheckStampFile(const char* stampName);
static bool cmakeCheckStampList(const char* stampName);

void cmWarnUnusedCliWarning(const std::string& variable,
  int, void* ctx, const char*, const cmMakefile*)
{
  cmake* cm = reinterpret_cast<cmake*>(ctx);
  cm->MarkCliAsUsed(variable);
}

cmake::cmake()
{
  this->Trace = false;
  this->WarnUninitialized = false;
  this->WarnUnused = false;
  this->WarnUnusedCli = true;
  this->CheckSystemVars = false;
  this->SuppressDevWarnings = false;
  this->DoSuppressDevWarnings = false;
  this->DebugOutput = false;
  this->DebugTryCompile = false;
  this->ClearBuildSystem = false;
  this->FileComparison = new cmFileTimeComparison;

  this->Policies = new cmPolicies();
  this->State = new cmState(this);
  this->CurrentSnapshot = this->State->CreateSnapshot(cmState::Snapshot());

#ifdef __APPLE__
  struct rlimit rlp;
  if(!getrlimit(RLIMIT_STACK, &rlp))
    {
    if(rlp.rlim_cur != rlp.rlim_max)
      {
        rlp.rlim_cur = rlp.rlim_max;
         setrlimit(RLIMIT_STACK, &rlp);
      }
    }
#endif

  this->Verbose = false;
  this->CacheManager = new cmCacheManager(this);
  this->GlobalGenerator = 0;
  this->ProgressCallback = 0;
  this->ProgressCallbackClientData = 0;
  this->CurrentWorkingMode = NORMAL_MODE;

#ifdef CMAKE_BUILD_WITH_CMAKE
  this->VariableWatch = new cmVariableWatch;
#endif

  this->AddDefaultGenerators();
  this->AddDefaultExtraGenerators();
  this->AddDefaultCommands();

  // Make sure we can capture the build tool output.
  cmSystemTools::EnableVSConsoleOutput();
}

cmake::~cmake()
{
  delete this->CacheManager;
  delete this->Policies;
  delete this->State;
  if (this->GlobalGenerator)
    {
    delete this->GlobalGenerator;
    this->GlobalGenerator = 0;
    }
  cmDeleteAll(this->Generators);
#ifdef CMAKE_BUILD_WITH_CMAKE
  delete this->VariableWatch;
#endif
  delete this->FileComparison;
}

void cmake::CleanupCommandsAndMacros()
{
  this->State->Reset();
  this->State->RemoveUserDefinedCommands();
}

// Parse the args
bool cmake::SetCacheArgs(const std::vector<std::string>& args)
{
  bool findPackageMode = false;
  for(unsigned int i=1; i < args.size(); ++i)
    {
    std::string arg = args[i];
    if(arg.find("-D",0) == 0)
      {
      std::string entry = arg.substr(2);
      if(entry.empty())
        {
        ++i;
        if(i < args.size())
          {
          entry = args[i];
          }
        else
          {
          cmSystemTools::Error("-D must be followed with VAR=VALUE.");
          return false;
          }
        }
      std::string var, value;
      cmState::CacheEntryType type = cmState::UNINITIALIZED;
      if(cmCacheManager::ParseEntry(entry, var, value, type))
        {
        // The value is transformed if it is a filepath for example, so
        // we can't compare whether the value is already in the cache until
        // after we call AddCacheEntry.
        bool haveValue = false;
        std::string cachedValue;
        if(this->WarnUnusedCli)
          {
          if(const char *v = this->State->GetInitializedCacheValue(var))
            {
            haveValue = true;
            cachedValue = v;
            }
          }

        this->State->AddCacheEntry(var, value.c_str(),
          "No help, variable specified on the command line.", type);

        if(this->WarnUnusedCli)
          {
          if (!haveValue ||
              cachedValue != this->State->GetInitializedCacheValue(var))
            {
            this->WatchUnusedCli(var);
            }
          }
        }
      else
        {
        std::cerr << "Parse error in command line argument: " << arg << "\n"
                  << "Should be: VAR:type=value\n";
        cmSystemTools::Error("No cmake script provided.");
        return false;
        }
      }
    else if(arg.find("-Wno-dev",0) == 0)
      {
      this->SuppressDevWarnings = true;
      this->DoSuppressDevWarnings = true;
      }
    else if(arg.find("-Wdev",0) == 0)
      {
      this->SuppressDevWarnings = false;
      this->DoSuppressDevWarnings = true;
      }
    else if(arg.find("-U",0) == 0)
      {
      std::string entryPattern = arg.substr(2);
      if(entryPattern.empty())
        {
        ++i;
        if(i < args.size())
          {
          entryPattern = args[i];
          }
        else
          {
          cmSystemTools::Error("-U must be followed with VAR.");
          return false;
          }
        }
      cmsys::RegularExpression regex(
        cmsys::Glob::PatternToRegex(entryPattern, true, true).c_str());
      //go through all cache entries and collect the vars which will be removed
      std::vector<std::string> entriesToDelete;
      std::vector<std::string> cacheKeys = this->State->GetCacheEntryKeys();
      for (std::vector<std::string>::const_iterator it = cacheKeys.begin();
            it != cacheKeys.end(); ++it)
        {
        cmState::CacheEntryType t = this->State->GetCacheEntryType(*it);
        if(t != cmState::STATIC)
          {
          if (regex.find(it->c_str()))
            {
            entriesToDelete.push_back(*it);
            }
          }
        }

      // now remove them from the cache
      for(std::vector<std::string>::const_iterator currentEntry =
          entriesToDelete.begin();
          currentEntry != entriesToDelete.end();
          ++currentEntry)
        {
        this->State->RemoveCacheEntry(*currentEntry);
        }
      }
    else if(arg.find("-C",0) == 0)
      {
      std::string path = arg.substr(2);
      if (path.empty())
        {
        ++i;
        if(i < args.size())
          {
          path = args[i];
          }
        else
          {
          cmSystemTools::Error("-C must be followed by a file name.");
          return false;
          }
        }
      std::cout << "loading initial cache file " << path << "\n";
      this->ReadListFile(args, path.c_str());
      }
    else if(arg.find("-P",0) == 0)
      {
      i++;
      if(i >= args.size())
        {
        cmSystemTools::Error("-P must be followed by a file name.");
        return false;
        }
      std::string path = args[i];
      if (path.empty())
        {
        cmSystemTools::Error("No cmake script provided.");
        return false;
        }
      this->ReadListFile(args, path.c_str());
      }
    else if (arg.find("--find-package",0) == 0)
      {
      findPackageMode = true;
      }
    }

  if (findPackageMode)
    {
    return this->FindPackage(args);
    }

  return true;
}

void cmake::ReadListFile(const std::vector<std::string>& args,
                         const char *path)
{
  // if a generator was not yet created, temporarily create one
  cmGlobalGenerator *gg = this->GetGlobalGenerator();
  bool created = false;

  // if a generator was not specified use a generic one
  if (!gg)
    {
    gg = new cmGlobalGenerator(this);
    created = true;
    }

  // read in the list file to fill the cache
  if(path)
    {
    std::string homeDir = this->GetHomeDirectory();
    std::string homeOutputDir = this->GetHomeOutputDirectory();
    this->SetHomeDirectory(cmSystemTools::GetCurrentWorkingDirectory());
    this->SetHomeOutputDirectory(cmSystemTools::GetCurrentWorkingDirectory());
    cmsys::auto_ptr<cmLocalGenerator> lg(gg->MakeLocalGenerator());
    lg->GetMakefile()->SetCurrentBinaryDirectory
      (cmSystemTools::GetCurrentWorkingDirectory());
    lg->GetMakefile()->SetCurrentSourceDirectory
      (cmSystemTools::GetCurrentWorkingDirectory());
    if (this->GetWorkingMode() != NORMAL_MODE)
      {
      std::string file(cmSystemTools::CollapseFullPath(path));
      cmSystemTools::ConvertToUnixSlashes(file);
      lg->GetMakefile()->SetScriptModeFile(file.c_str());

      lg->GetMakefile()->SetArgcArgv(args);
      }
    if (!lg->GetMakefile()->ReadListFile(path))
      {
      cmSystemTools::Error("Error processing file: ", path);
      }
    this->SetHomeDirectory(homeDir);
    this->SetHomeOutputDirectory(homeOutputDir);
    }

  // free generic one if generated
  if (created)
    {
    delete gg;
    }
}


bool cmake::FindPackage(const std::vector<std::string>& args)
{
  this->SetHomeDirectory
    (cmSystemTools::GetCurrentWorkingDirectory());
  this->SetHomeOutputDirectory
    (cmSystemTools::GetCurrentWorkingDirectory());

  // if a generator was not yet created, temporarily create one
  cmGlobalGenerator *gg = new cmGlobalGenerator(this);
  this->SetGlobalGenerator(gg);

  // read in the list file to fill the cache
  cmsys::auto_ptr<cmLocalGenerator> lg(gg->MakeLocalGenerator());
  cmMakefile* mf = lg->GetMakefile();
  mf->SetCurrentBinaryDirectory
    (cmSystemTools::GetCurrentWorkingDirectory());
  mf->SetCurrentSourceDirectory
    (cmSystemTools::GetCurrentWorkingDirectory());

  mf->SetArgcArgv(args);

  std::string systemFile = mf->GetModulesFile("CMakeFindPackageMode.cmake");
  mf->ReadListFile(systemFile.c_str());

  std::string language = mf->GetSafeDefinition("LANGUAGE");
  std::string mode = mf->GetSafeDefinition("MODE");
  std::string packageName = mf->GetSafeDefinition("NAME");
  bool packageFound = mf->IsOn("PACKAGE_FOUND");
  bool quiet = mf->IsOn("PACKAGE_QUIET");

  if (!packageFound)
    {
    if (!quiet)
      {
      printf("%s not found.\n", packageName.c_str());
      }
    }
  else if (mode == "EXIST")
    {
    if (!quiet)
      {
      printf("%s found.\n", packageName.c_str());
      }
    }
  else if (mode == "COMPILE")
    {
    std::string includes = mf->GetSafeDefinition("PACKAGE_INCLUDE_DIRS");
    std::vector<std::string> includeDirs;
    cmSystemTools::ExpandListArgument(includes, includeDirs);

    std::string includeFlags = lg->GetIncludeFlags(includeDirs, 0, language);

    std::string definitions = mf->GetSafeDefinition("PACKAGE_DEFINITIONS");
    printf("%s %s\n", includeFlags.c_str(), definitions.c_str());
    }
  else if (mode == "LINK")
    {
    const char* targetName = "dummy";
    std::vector<std::string> srcs;
    cmTarget* tgt = mf->AddExecutable(targetName, srcs, true);
    tgt->SetProperty("LINKER_LANGUAGE", language.c_str());

    std::string libs = mf->GetSafeDefinition("PACKAGE_LIBRARIES");
    std::vector<std::string> libList;
    cmSystemTools::ExpandListArgument(libs, libList);
    for(std::vector<std::string>::const_iterator libIt=libList.begin();
            libIt != libList.end();
            ++libIt)
      {
      mf->AddLinkLibraryForTarget(targetName, *libIt,
                                  cmTarget::GENERAL);
      }


    std::string linkLibs;
    std::string frameworkPath;
    std::string linkPath;
    std::string flags;
    std::string linkFlags;
    gg->CreateGeneratorTargets(mf);
    cmGeneratorTarget *gtgt = gg->GetGeneratorTarget(tgt);
    lg->GetTargetFlags(linkLibs, frameworkPath, linkPath, flags, linkFlags,
                       gtgt, false);
    linkLibs = frameworkPath + linkPath + linkLibs;

    printf("%s\n", linkLibs.c_str() );

/*    if ( use_win32 )
      {
      tgt->SetProperty("WIN32_EXECUTABLE", "ON");
      }
    if ( use_macbundle)
      {
      tgt->SetProperty("MACOSX_BUNDLE", "ON");
      }*/
    }

  // free generic one if generated
//  this->SetGlobalGenerator(0); // setting 0-pointer is not possible
//  delete gg; // this crashes inside the cmake instance

  return packageFound;
}


// Parse the args
void cmake::SetArgs(const std::vector<std::string>& args,
                    bool directoriesSetBefore)
{
  bool directoriesSet = directoriesSetBefore;
  bool haveToolset = false;
  bool havePlatform = false;
  for(unsigned int i=1; i < args.size(); ++i)
    {
    std::string arg = args[i];
    if(arg.find("-H",0) == 0)
      {
      directoriesSet = true;
      std::string path = arg.substr(2);
      path = cmSystemTools::CollapseFullPath(path);
      cmSystemTools::ConvertToUnixSlashes(path);
      this->SetHomeDirectory(path);
      }
    else if(arg.find("-S",0) == 0)
      {
      // There is no local generate anymore.  Ignore -S option.
      }
    else if(arg.find("-O",0) == 0)
      {
      // There is no local generate anymore.  Ignore -O option.
      }
    else if(arg.find("-B",0) == 0)
      {
      directoriesSet = true;
      std::string path = arg.substr(2);
      path = cmSystemTools::CollapseFullPath(path);
      cmSystemTools::ConvertToUnixSlashes(path);
      this->SetHomeOutputDirectory(path);
      }
    else if((i < args.size()-2) && (arg.find("--check-build-system",0) == 0))
      {
      this->CheckBuildSystemArgument = args[++i];
      this->ClearBuildSystem = (atoi(args[++i].c_str()) > 0);
      }
    else if((i < args.size()-1) && (arg.find("--check-stamp-file",0) == 0))
      {
      this->CheckStampFile = args[++i];
      }
    else if((i < args.size()-1) && (arg.find("--check-stamp-list",0) == 0))
      {
      this->CheckStampList = args[++i];
      }
#if defined(CMAKE_HAVE_VS_GENERATORS)
    else if((i < args.size()-1) && (arg.find("--vs-solution-file",0) == 0))
      {
      this->VSSolutionFile = args[++i];
      }
#endif
    else if(arg.find("-V",0) == 0)
      {
        this->Verbose = true;
      }
    else if(arg.find("-D",0) == 0)
      {
      // skip for now
      }
    else if(arg.find("-U",0) == 0)
      {
      // skip for now
      }
    else if(arg.find("-C",0) == 0)
      {
      // skip for now
      }
    else if(arg.find("-P",0) == 0)
      {
      // skip for now
      i++;
      }
    else if(arg.find("--find-package",0) == 0)
      {
      // skip for now
      i++;
      }
    else if(arg.find("-Wno-dev",0) == 0)
      {
      // skip for now
      }
    else if(arg.find("-Wdev",0) == 0)
      {
      // skip for now
      }
    else if(arg.find("--graphviz=",0) == 0)
      {
      std::string path = arg.substr(strlen("--graphviz="));
      path = cmSystemTools::CollapseFullPath(path);
      cmSystemTools::ConvertToUnixSlashes(path);
      this->GraphVizFile = path;
      if ( this->GraphVizFile.empty() )
        {
        cmSystemTools::Error("No file specified for --graphviz");
        }
      }
    else if(arg.find("--debug-trycompile",0) == 0)
      {
      std::cout << "debug trycompile on\n";
      this->DebugTryCompileOn();
      }
    else if(arg.find("--debug-output",0) == 0)
      {
      std::cout << "Running with debug output on.\n";
      this->SetDebugOutputOn(true);
      }
    else if(arg.find("--trace",0) == 0)
      {
      std::cout << "Running with trace output on.\n";
      this->SetTrace(true);
      }
    else if(arg.find("--warn-uninitialized",0) == 0)
      {
      std::cout << "Warn about uninitialized values.\n";
      this->SetWarnUninitialized(true);
      }
    else if(arg.find("--warn-unused-vars",0) == 0)
      {
      std::cout << "Finding unused variables.\n";
      this->SetWarnUnused(true);
      }
    else if(arg.find("--no-warn-unused-cli",0) == 0)
      {
      std::cout << "Not searching for unused variables given on the " <<
                   "command line.\n";
      this->SetWarnUnusedCli(false);
      }
    else if(arg.find("--check-system-vars",0) == 0)
      {
      std::cout << "Also check system files when warning about unused and " <<
                   "uninitialized variables.\n";
      this->SetCheckSystemVars(true);
      }
    else if(arg.find("-A",0) == 0)
      {
      std::string value = arg.substr(2);
      if(value.empty())
        {
        ++i;
        if(i >= args.size())
          {
          cmSystemTools::Error("No platform specified for -A");
          return;
          }
        value = args[i];
        }
      if(havePlatform)
        {
        cmSystemTools::Error("Multiple -A options not allowed");
        return;
        }
      this->GeneratorPlatform = value;
      havePlatform = true;
      }
    else if(arg.find("-T",0) == 0)
      {
      std::string value = arg.substr(2);
      if(value.empty())
        {
        ++i;
        if(i >= args.size())
          {
          cmSystemTools::Error("No toolset specified for -T");
          return;
          }
        value = args[i];
        }
      if(haveToolset)
        {
        cmSystemTools::Error("Multiple -T options not allowed");
        return;
        }
      this->GeneratorToolset = value;
      haveToolset = true;
      }
    else if(arg.find("-G",0) == 0)
      {
      std::string value = arg.substr(2);
      if(value.empty())
        {
        ++i;
        if(i >= args.size())
          {
          cmSystemTools::Error("No generator specified for -G");
          this->PrintGeneratorList();
          return;
          }
        value = args[i];
        }
      cmGlobalGenerator* gen =
        this->CreateGlobalGenerator(value);
      if(!gen)
        {
        cmSystemTools::Error("Could not create named generator ",
                             value.c_str());
        this->PrintGeneratorList();
        }
      else
        {
        this->SetGlobalGenerator(gen);
        }
      }
    // no option assume it is the path to the source
    else
      {
      directoriesSet = true;
      this->SetDirectoriesFromFile(arg.c_str());
      }
    }
  if(!directoriesSet)
    {
    this->SetHomeOutputDirectory
      (cmSystemTools::GetCurrentWorkingDirectory());
    this->SetHomeDirectory
      (cmSystemTools::GetCurrentWorkingDirectory());
    }
}

//----------------------------------------------------------------------------
void cmake::SetDirectoriesFromFile(const char* arg)
{
  // Check if the argument refers to a CMakeCache.txt or
  // CMakeLists.txt file.
  std::string listPath;
  std::string cachePath;
  bool argIsFile = false;
  if(cmSystemTools::FileIsDirectory(arg))
    {
    std::string path = cmSystemTools::CollapseFullPath(arg);
    cmSystemTools::ConvertToUnixSlashes(path);
    std::string cacheFile = path;
    cacheFile += "/CMakeCache.txt";
    std::string listFile = path;
    listFile += "/CMakeLists.txt";
    if(cmSystemTools::FileExists(cacheFile.c_str()))
      {
      cachePath = path;
      }
    if(cmSystemTools::FileExists(listFile.c_str()))
      {
      listPath = path;
      }
    }
  else if(cmSystemTools::FileExists(arg))
    {
    argIsFile = true;
    std::string fullPath = cmSystemTools::CollapseFullPath(arg);
    std::string name = cmSystemTools::GetFilenameName(fullPath);
    name = cmSystemTools::LowerCase(name);
    if(name == "cmakecache.txt")
      {
      cachePath = cmSystemTools::GetFilenamePath(fullPath);
      }
    else if(name == "cmakelists.txt")
      {
      listPath = cmSystemTools::GetFilenamePath(fullPath);
      }
    }
  else
    {
    // Specified file or directory does not exist.  Try to set things
    // up to produce a meaningful error message.
    std::string fullPath = cmSystemTools::CollapseFullPath(arg);
    std::string name = cmSystemTools::GetFilenameName(fullPath);
    name = cmSystemTools::LowerCase(name);
    if(name == "cmakecache.txt" || name == "cmakelists.txt")
      {
      argIsFile = true;
      listPath = cmSystemTools::GetFilenamePath(fullPath);
      }
    else
      {
      listPath = fullPath;
      }
    }

  // If there is a CMakeCache.txt file, use its settings.
  if(!cachePath.empty())
    {
    if(this->LoadCache(cachePath))
      {
      const char* existingValue =
          this->State->GetCacheEntryValue("CMAKE_HOME_DIRECTORY");
      if (existingValue)
        {
        this->SetHomeOutputDirectory(cachePath);
        this->SetHomeDirectory(existingValue);
        return;
        }
      }
    }

  // If there is a CMakeLists.txt file, use it as the source tree.
  if(!listPath.empty())
    {
    this->SetHomeDirectory(listPath);

    if(argIsFile)
      {
      // Source CMakeLists.txt file given.  It was probably dropped
      // onto the executable in a GUI.  Default to an in-source build.
      this->SetHomeOutputDirectory(listPath);
      }
    else
      {
      // Source directory given on command line.  Use current working
      // directory as build tree.
      std::string cwd = cmSystemTools::GetCurrentWorkingDirectory();
      this->SetHomeOutputDirectory(cwd);
      }
    return;
    }

  // We didn't find a CMakeLists.txt or CMakeCache.txt file from the
  // argument.  Assume it is the path to the source tree, and use the
  // current working directory as the build tree.
  std::string full = cmSystemTools::CollapseFullPath(arg);
  std::string cwd = cmSystemTools::GetCurrentWorkingDirectory();
  this->SetHomeDirectory(full);
  this->SetHomeOutputDirectory(cwd);
}

// at the end of this CMAKE_ROOT and CMAKE_COMMAND should be added to the
// cache
int cmake::AddCMakePaths()
{
  // Save the value in the cache
  this->CacheManager->AddCacheEntry
    ("CMAKE_COMMAND", cmSystemTools::GetCMakeCommand().c_str(),
     "Path to CMake executable.", cmState::INTERNAL);
#ifdef CMAKE_BUILD_WITH_CMAKE
  this->CacheManager->AddCacheEntry
    ("CMAKE_CTEST_COMMAND", cmSystemTools::GetCTestCommand().c_str(),
     "Path to ctest program executable.", cmState::INTERNAL);
  this->CacheManager->AddCacheEntry
    ("CMAKE_CPACK_COMMAND", cmSystemTools::GetCPackCommand().c_str(),
     "Path to cpack program executable.", cmState::INTERNAL);
#endif
  if(!cmSystemTools::FileExists(
       (cmSystemTools::GetCMakeRoot()+"/Modules/CMake.cmake").c_str()))
    {
    // couldn't find modules
    cmSystemTools::Error("Could not find CMAKE_ROOT !!!\n"
      "CMake has most likely not been installed correctly.\n"
      "Modules directory not found in\n",
      cmSystemTools::GetCMakeRoot().c_str());
    return 0;
    }
  this->CacheManager->AddCacheEntry
    ("CMAKE_ROOT", cmSystemTools::GetCMakeRoot().c_str(),
     "Path to CMake installation.", cmState::INTERNAL);

  return 1;
}

void cmake::AddExtraGenerator(const std::string& name,
                              CreateExtraGeneratorFunctionType newFunction)
{
  cmExternalMakefileProjectGenerator* extraGenerator = newFunction();
  const std::vector<std::string>& supportedGlobalGenerators =
                                extraGenerator->GetSupportedGlobalGenerators();

  for(std::vector<std::string>::const_iterator
      it = supportedGlobalGenerators.begin();
      it != supportedGlobalGenerators.end();
      ++it )
    {
    std::string fullName = cmExternalMakefileProjectGenerator::
                                    CreateFullGeneratorName(*it, name);
    this->ExtraGenerators[fullName] = newFunction;
    }
  delete extraGenerator;
}

void cmake::AddDefaultExtraGenerators()
{
#if defined(CMAKE_BUILD_WITH_CMAKE)
#if defined(_WIN32) && !defined(__CYGWIN__)
  // e.g. kdevelop4 ?
#endif

  this->AddExtraGenerator(cmExtraCodeBlocksGenerator::GetActualName(),
                          &cmExtraCodeBlocksGenerator::New);
  this->AddExtraGenerator(cmExtraCodeLiteGenerator::GetActualName(),
                          &cmExtraCodeLiteGenerator::New);
  this->AddExtraGenerator(cmExtraSublimeTextGenerator::GetActualName(),
                          &cmExtraSublimeTextGenerator::New);
  this->AddExtraGenerator(cmExtraKateGenerator::GetActualName(),
                          &cmExtraKateGenerator::New);

#ifdef CMAKE_USE_ECLIPSE
  this->AddExtraGenerator(cmExtraEclipseCDT4Generator::GetActualName(),
                          &cmExtraEclipseCDT4Generator::New);
#endif

#ifdef CMAKE_USE_KDEVELOP
  this->AddExtraGenerator(cmGlobalKdevelopGenerator::GetActualName(),
                          &cmGlobalKdevelopGenerator::New);
  // for kdevelop also add the generator with just the name of the
  // extra generator, since it was this way since cmake 2.2
  this->ExtraGenerators[cmGlobalKdevelopGenerator::GetActualName()]
                                             = &cmGlobalKdevelopGenerator::New;
#endif

#endif
}


//----------------------------------------------------------------------------
void cmake::GetRegisteredGenerators(std::vector<std::string>& names)
{
  for(RegisteredGeneratorsVector::const_iterator i = this->Generators.begin();
      i != this->Generators.end(); ++i)
    {
    (*i)->GetGenerators(names);
    }
  for(RegisteredExtraGeneratorsMap::const_iterator
      i = this->ExtraGenerators.begin();
      i != this->ExtraGenerators.end(); ++i)
    {
    names.push_back(i->first);
    }
}

cmGlobalGenerator* cmake::CreateGlobalGenerator(const std::string& gname)
{
  cmExternalMakefileProjectGenerator* extraGenerator = 0;
  std::string name = gname;
  RegisteredExtraGeneratorsMap::const_iterator extraGenIt =
                                            this->ExtraGenerators.find(name);
  if (extraGenIt != this->ExtraGenerators.end())
    {
    extraGenerator = (extraGenIt->second)();
    name = extraGenerator->GetGlobalGeneratorName(name);
    }

  cmGlobalGenerator* generator = 0;
  for (RegisteredGeneratorsVector::const_iterator i =
    this->Generators.begin(); i != this->Generators.end(); ++i)
    {
    generator = (*i)->CreateGlobalGenerator(name, this);
    if (generator)
      {
      break;
      }
    }

  if (generator)
    {
    generator->SetExternalMakefileProjectGenerator(extraGenerator);
    }
  else
    {
    delete extraGenerator;
    }

  return generator;
}

void cmake::SetHomeDirectory(const std::string& dir)
{
  this->State->SetSourceDirectory(dir);
}

const char* cmake::GetHomeDirectory() const
{
  return this->State->GetSourceDirectory();
}

void cmake::SetHomeOutputDirectory(const std::string& dir)
{
  this->State->SetBinaryDirectory(dir);
}

const char* cmake::GetHomeOutputDirectory() const
{
  return this->State->GetBinaryDirectory();
}

void cmake::SetGlobalGenerator(cmGlobalGenerator *gg)
{
  if(!gg)
    {
    cmSystemTools::Error("Error SetGlobalGenerator called with null");
    return;
    }
  // delete the old generator
  if (this->GlobalGenerator)
    {
    delete this->GlobalGenerator;
    // restore the original environment variables CXX and CC
    // Restore CC
    std::string env = "CC=";
    if(!this->CCEnvironment.empty())
      {
      env += this->CCEnvironment;
      }
    cmSystemTools::PutEnv(env);
    env = "CXX=";
    if(!this->CXXEnvironment.empty())
      {
      env += this->CXXEnvironment;
      }
    cmSystemTools::PutEnv(env);
    }

  // set the new
  this->GlobalGenerator = gg;

  // set the global flag for unix style paths on cmSystemTools as soon as
  // the generator is set.  This allows gmake to be used on windows.
  cmSystemTools::SetForceUnixPaths
    (this->GlobalGenerator->GetForceUnixPaths());

  // Save the environment variables CXX and CC
  const char* cxx = getenv("CXX");
  const char* cc = getenv("CC");
  if(cxx)
    {
    this->CXXEnvironment = cxx;
    }
  else
    {
    this->CXXEnvironment = "";
    }
  if(cc)
    {
    this->CCEnvironment = cc;
    }
  else
    {
    this->CCEnvironment = "";
    }
}

int cmake::DoPreConfigureChecks()
{
  // Make sure the Source directory contains a CMakeLists.txt file.
  std::string srcList = this->GetHomeDirectory();
  srcList += "/CMakeLists.txt";
  if(!cmSystemTools::FileExists(srcList.c_str()))
    {
    std::ostringstream err;
    if(cmSystemTools::FileIsDirectory(this->GetHomeDirectory()))
      {
      err << "The source directory \"" << this->GetHomeDirectory()
          << "\" does not appear to contain CMakeLists.txt.\n";
      }
    else if(cmSystemTools::FileExists(this->GetHomeDirectory()))
      {
      err << "The source directory \"" << this->GetHomeDirectory()
          << "\" is a file, not a directory.\n";
      }
    else
      {
      err << "The source directory \"" << this->GetHomeDirectory()
          << "\" does not exist.\n";
      }
    err << "Specify --help for usage, or press the help button on the CMake "
      "GUI.";
    cmSystemTools::Error(err.str().c_str());
    return -2;
    }

  // do a sanity check on some values
  if(this->CacheManager->GetInitializedCacheValue("CMAKE_HOME_DIRECTORY"))
    {
    std::string cacheStart =
      this->CacheManager->GetInitializedCacheValue("CMAKE_HOME_DIRECTORY");
    cacheStart += "/CMakeLists.txt";
    std::string currentStart = this->GetHomeDirectory();
    currentStart += "/CMakeLists.txt";
    if(!cmSystemTools::SameFile(cacheStart, currentStart))
      {
      std::string message = "The source \"";
      message += currentStart;
      message += "\" does not match the source \"";
      message += cacheStart;
      message += "\" used to generate cache.  ";
      message += "Re-run cmake with a different source directory.";
      cmSystemTools::Error(message.c_str());
      return -2;
      }
    }
  else
    {
    return 0;
    }
  return 1;
}
struct SaveCacheEntry
{
  std::string key;
  std::string value;
  std::string help;
  cmState::CacheEntryType type;
};

int cmake::HandleDeleteCacheVariables(const std::string& var)
{
  std::vector<std::string> argsSplit;
  cmSystemTools::ExpandListArgument(std::string(var), argsSplit, true);
  // erase the property to avoid infinite recursion
  this->State
      ->SetGlobalProperty("__CMAKE_DELETE_CACHE_CHANGE_VARS_", "");
  if(this->State->GetIsInTryCompile())
    {
    return 0;
    }
  std::vector<SaveCacheEntry> saved;
  std::ostringstream warning;
  warning
    << "You have changed variables that require your cache to be deleted.\n"
    << "Configure will be re-run and you may have to reset some variables.\n"
    << "The following variables have changed:\n";
  for(std::vector<std::string>::iterator i = argsSplit.begin();
      i != argsSplit.end(); ++i)
    {
    SaveCacheEntry save;
    save.key = *i;
    warning << *i << "= ";
    i++;
    save.value = *i;
    warning << *i << "\n";
    const char* existingValue =
        this->CacheManager->GetCacheEntryValue(save.key);
    if(existingValue)
      {
      save.type = this->CacheManager->GetCacheEntryType(save.key);
      if(const char* help =
            this->CacheManager->GetCacheEntryProperty(save.key, "HELPSTRING"))
        {
        save.help = help;
        }
      }
    saved.push_back(save);
    }

  // remove the cache
  this->CacheManager->DeleteCache(this->GetHomeOutputDirectory());
  // load the empty cache
  this->LoadCache();
  // restore the changed compilers
  for(std::vector<SaveCacheEntry>::iterator i = saved.begin();
      i != saved.end(); ++i)
    {
    this->AddCacheEntry(i->key, i->value.c_str(),
                        i->help.c_str(), i->type);
    }
  cmSystemTools::Message(warning.str().c_str());
  // avoid reconfigure if there were errors
  if(!cmSystemTools::GetErrorOccuredFlag())
    {
    // re-run configure
    return this->Configure();
    }
  return 0;
}

int cmake::Configure()
{
  if(this->DoSuppressDevWarnings)
    {
    if(this->SuppressDevWarnings)
      {
      this->CacheManager->
        AddCacheEntry("CMAKE_SUPPRESS_DEVELOPER_WARNINGS", "TRUE",
                      "Suppress Warnings that are meant for"
                      " the author of the CMakeLists.txt files.",
                      cmState::INTERNAL);
      }
    else
      {
      this->CacheManager->
        AddCacheEntry("CMAKE_SUPPRESS_DEVELOPER_WARNINGS", "FALSE",
                      "Suppress Warnings that are meant for"
                      " the author of the CMakeLists.txt files.",
                      cmState::INTERNAL);
      }
    }
  int ret = this->ActualConfigure();
  const char* delCacheVars = this->State
                    ->GetGlobalProperty("__CMAKE_DELETE_CACHE_CHANGE_VARS_");
  if(delCacheVars && delCacheVars[0] != 0)
    {
    return this->HandleDeleteCacheVariables(delCacheVars);
    }
  return ret;

}

int cmake::ActualConfigure()
{
  // Construct right now our path conversion table before it's too late:
  this->UpdateConversionPathTable();
  this->CleanupCommandsAndMacros();

  int res = 0;
  if ( this->GetWorkingMode() == NORMAL_MODE )
    {
    res = this->DoPreConfigureChecks();
    }
  if ( res < 0 )
    {
    return -2;
    }
  if ( !res )
    {
    this->CacheManager->AddCacheEntry
      ("CMAKE_HOME_DIRECTORY",
       this->GetHomeDirectory(),
       "Source directory with the top level CMakeLists.txt file for this "
       "project",
       cmState::INTERNAL);
    }

  // no generator specified on the command line
  if(!this->GlobalGenerator)
    {
    const char* genName =
      this->CacheManager->GetInitializedCacheValue("CMAKE_GENERATOR");
    const char* extraGenName =
      this->CacheManager->GetInitializedCacheValue("CMAKE_EXTRA_GENERATOR");
    if(genName)
      {
      std::string fullName = cmExternalMakefileProjectGenerator::
                                CreateFullGeneratorName(genName,
                                    extraGenName ? extraGenName : "");
      this->GlobalGenerator = this->CreateGlobalGenerator(fullName);
      }
    if(this->GlobalGenerator)
      {
      // set the global flag for unix style paths on cmSystemTools as
      // soon as the generator is set.  This allows gmake to be used
      // on windows.
      cmSystemTools::SetForceUnixPaths
        (this->GlobalGenerator->GetForceUnixPaths());
      }
    else
      {
#if defined(_WIN32) && !defined(__CYGWIN__) && !defined(CMAKE_BOOT_MINGW)
      std::string installedCompiler;
      // Try to find the newest VS installed on the computer and
      // use that as a default if -G is not specified
      const std::string vsregBase =
        "[HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\";
      std::vector<std::string> vsVerions;
      vsVerions.push_back("VisualStudio\\");
      vsVerions.push_back("VCExpress\\");
      vsVerions.push_back("WDExpress\\");
      struct VSRegistryEntryName
      {
        const char* MSVersion;
        const char* GeneratorName;
      };
      VSRegistryEntryName version[] = {
        {"6.0", "Visual Studio 6"},
        {"7.0", "Visual Studio 7"},
        {"7.1", "Visual Studio 7 .NET 2003"},
        {"8.0", "Visual Studio 8 2005"},
        {"9.0", "Visual Studio 9 2008"},
        {"10.0", "Visual Studio 10 2010"},
        {"11.0", "Visual Studio 11 2012"},
        {"12.0", "Visual Studio 12 2013"},
        {"14.0", "Visual Studio 14 2015"},
        {0, 0}};
      for(int i=0; version[i].MSVersion != 0; i++)
        {
        for(size_t b=0; b < vsVerions.size(); b++)
          {
          std::string reg = vsregBase + vsVerions[b] + version[i].MSVersion;
          reg += ";InstallDir]";
          cmSystemTools::ExpandRegistryValues(reg,
                                              cmSystemTools::KeyWOW64_32);
          if (!(reg == "/registry"))
            {
            installedCompiler = version[i].GeneratorName;
            break;
            }
          }
        }
      cmGlobalGenerator* gen
        = this->CreateGlobalGenerator(installedCompiler.c_str());
      if(!gen)
        {
        gen = new cmGlobalNMakeMakefileGenerator(this);
        }
      this->SetGlobalGenerator(gen);
      std::cout << "-- Building for: " << gen->GetName() << "\n";
#else
      this->SetGlobalGenerator(new cmGlobalUnixMakefileGenerator3(this));
#endif
      }
    if(!this->GlobalGenerator)
      {
      cmSystemTools::Error("Could not create generator");
      return -1;
      }
    }

  const char* genName = this->CacheManager
                            ->GetInitializedCacheValue("CMAKE_GENERATOR");
  if(genName)
    {
    if(!this->GlobalGenerator->MatchesGeneratorName(genName))
      {
      std::string message = "Error: generator : ";
      message += this->GlobalGenerator->GetName();
      message += "\nDoes not match the generator used previously: ";
      message += genName;
      message +=
        "\nEither remove the CMakeCache.txt file and CMakeFiles "
        "directory or choose a different binary directory.";
      cmSystemTools::Error(message.c_str());
      return -2;
      }
    }
  if(!this->CacheManager->GetInitializedCacheValue("CMAKE_GENERATOR"))
    {
    this->CacheManager->AddCacheEntry("CMAKE_GENERATOR",
                                      this->GlobalGenerator->GetName().c_str(),
                                      "Name of generator.",
                                      cmState::INTERNAL);
    this->CacheManager->AddCacheEntry("CMAKE_EXTRA_GENERATOR",
                        this->GlobalGenerator->GetExtraGeneratorName().c_str(),
                        "Name of external makefile project generator.",
                        cmState::INTERNAL);
    }

  if(const char* platformName =
     this->CacheManager->GetInitializedCacheValue("CMAKE_GENERATOR_PLATFORM"))
    {
    if(this->GeneratorPlatform.empty())
      {
      this->GeneratorPlatform = platformName;
      }
    else if(this->GeneratorPlatform != platformName)
      {
      std::string message = "Error: generator platform: ";
      message += this->GeneratorPlatform;
      message += "\nDoes not match the platform used previously: ";
      message += platformName;
      message +=
        "\nEither remove the CMakeCache.txt file and CMakeFiles "
        "directory or choose a different binary directory.";
      cmSystemTools::Error(message.c_str());
      return -2;
      }
    }
  else
    {
    this->CacheManager->AddCacheEntry("CMAKE_GENERATOR_PLATFORM",
                                      this->GeneratorPlatform.c_str(),
                                      "Name of generator platform.",
                                      cmState::INTERNAL);
    }

  if(const char* tsName =
     this->CacheManager->GetInitializedCacheValue("CMAKE_GENERATOR_TOOLSET"))
    {
    if(this->GeneratorToolset.empty())
      {
      this->GeneratorToolset = tsName;
      }
    else if(this->GeneratorToolset != tsName)
      {
      std::string message = "Error: generator toolset: ";
      message += this->GeneratorToolset;
      message += "\nDoes not match the toolset used previously: ";
      message += tsName;
      message +=
        "\nEither remove the CMakeCache.txt file and CMakeFiles "
        "directory or choose a different binary directory.";
      cmSystemTools::Error(message.c_str());
      return -2;
      }
    }
  else
    {
    this->CacheManager->AddCacheEntry("CMAKE_GENERATOR_TOOLSET",
                                      this->GeneratorToolset.c_str(),
                                      "Name of generator toolset.",
                                      cmState::INTERNAL);
    }

  // reset any system configuration information, except for when we are
  // InTryCompile. With TryCompile the system info is taken from the parent's
  // info to save time
  if (!this->State->GetIsInTryCompile())
    {
    this->GlobalGenerator->ClearEnabledLanguages();

    this->TruncateOutputLog("CMakeOutput.log");
    this->TruncateOutputLog("CMakeError.log");
    }

  // actually do the configure
  this->GlobalGenerator->Configure();
  // Before saving the cache
  // if the project did not define one of the entries below, add them now
  // so users can edit the values in the cache:

  // We used to always present LIBRARY_OUTPUT_PATH and
  // EXECUTABLE_OUTPUT_PATH.  They are now documented as old-style and
  // should no longer be used.  Therefore we present them only if the
  // project requires compatibility with CMake 2.4.  We detect this
  // here by looking for the old CMAKE_BACKWARDS_COMPATIBILITY
  // variable created when CMP0001 is not set to NEW.
  if(this->State
         ->GetInitializedCacheValue("CMAKE_BACKWARDS_COMPATIBILITY"))
    {
    if(!this->State->GetInitializedCacheValue("LIBRARY_OUTPUT_PATH"))
      {
      this->State->AddCacheEntry
        ("LIBRARY_OUTPUT_PATH", "",
         "Single output directory for building all libraries.",
         cmState::PATH);
      }
    if(!this->State
            ->GetInitializedCacheValue("EXECUTABLE_OUTPUT_PATH"))
      {
      this->State->AddCacheEntry
        ("EXECUTABLE_OUTPUT_PATH", "",
         "Single output directory for building all executables.",
         cmState::PATH);
      }
    }
  if(!this->State
          ->GetInitializedCacheValue("CMAKE_USE_RELATIVE_PATHS"))
    {
    this->State->AddCacheEntry
      ("CMAKE_USE_RELATIVE_PATHS", "OFF",
       "If true, cmake will use relative paths in makefiles and projects.",
       cmState::BOOL);
    if (!this->State->GetCacheEntryProperty("CMAKE_USE_RELATIVE_PATHS",
                                                    "ADVANCED"))
      {
      this->State->SetCacheEntryProperty("CMAKE_USE_RELATIVE_PATHS",
                                                 "ADVANCED", "1");
      }
    }

  if(cmSystemTools::GetFatalErrorOccured())
    {
    const char* makeProgram =
        this->State->GetInitializedCacheValue("CMAKE_MAKE_PROGRAM");
    if (!makeProgram || cmSystemTools::IsOff(makeProgram))
      {
      // We must have a bad generator selection.  Wipe the cache entry so the
      // user can select another.
      this->State->RemoveCacheEntry("CMAKE_GENERATOR");
      this->State->RemoveCacheEntry("CMAKE_EXTRA_GENERATOR");
      }
    }

  cmMakefile* mf=this->GlobalGenerator->GetLocalGenerators()[0]->GetMakefile();
  if (mf->IsOn("CTEST_USE_LAUNCHERS")
              && !this->State->GetGlobalProperty("RULE_LAUNCH_COMPILE"))
    {
    cmSystemTools::Error("CTEST_USE_LAUNCHERS is enabled, but the "
                        "RULE_LAUNCH_COMPILE global property is not defined.\n"
                        "Did you forget to include(CTest) in the toplevel "
                         "CMakeLists.txt ?");
    }

  // only save the cache if there were no fatal errors
  if ( this->GetWorkingMode() == NORMAL_MODE )
    {
    this->CacheManager->SaveCache(this->GetHomeOutputDirectory());
    }
  if(cmSystemTools::GetErrorOccuredFlag())
    {
    return -1;
    }
  return 0;
}

void cmake::PreLoadCMakeFiles()
{
  std::vector<std::string> args;
  std::string pre_load = this->GetHomeDirectory();
  if (!pre_load.empty())
    {
    pre_load += "/PreLoad.cmake";
    if ( cmSystemTools::FileExists(pre_load.c_str()) )
      {
      this->ReadListFile(args, pre_load.c_str());
      }
    }
  pre_load = this->GetHomeOutputDirectory();
  if (!pre_load.empty())
    {
    pre_load += "/PreLoad.cmake";
    if ( cmSystemTools::FileExists(pre_load.c_str()) )
      {
      this->ReadListFile(args, pre_load.c_str());
      }
    }
}

// handle a command line invocation
int cmake::Run(const std::vector<std::string>& args, bool noconfigure)
{
  // Process the arguments
  this->SetArgs(args);
  if(cmSystemTools::GetErrorOccuredFlag())
    {
    return -1;
    }

  // If we are given a stamp list file check if it is really out of date.
  if(!this->CheckStampList.empty() &&
     cmakeCheckStampList(this->CheckStampList.c_str()))
    {
    return 0;
    }

  // If we are given a stamp file check if it is really out of date.
  if(!this->CheckStampFile.empty() &&
     cmakeCheckStampFile(this->CheckStampFile.c_str()))
    {
    return 0;
    }

  if ( this->GetWorkingMode() == NORMAL_MODE )
    {
    // load the cache
    if(this->LoadCache() < 0)
      {
      cmSystemTools::Error("Error executing cmake::LoadCache(). Aborting.\n");
      return -1;
      }
    }
  else
    {
    this->AddCMakePaths();
    }
  // Add any cache args
  if ( !this->SetCacheArgs(args) )
    {
    cmSystemTools::Error("Problem processing arguments. Aborting.\n");
    return -1;
    }

  // In script mode we terminate after running the script.
  if(this->GetWorkingMode() != NORMAL_MODE)
    {
    if(cmSystemTools::GetErrorOccuredFlag())
      {
      return -1;
      }
    else
      {
      return 0;
      }
    }

  // If MAKEFLAGS are given in the environment, remove the environment
  // variable.  This will prevent try-compile from succeeding when it
  // should fail (if "-i" is an option).  We cannot simply test
  // whether "-i" is given and remove it because some make programs
  // encode the MAKEFLAGS variable in a strange way.
  if(getenv("MAKEFLAGS"))
    {
    cmSystemTools::PutEnv("MAKEFLAGS=");
    }

  this->PreLoadCMakeFiles();

  if ( noconfigure )
    {
    return 0;
    }

  // now run the global generate
  // Check the state of the build system to see if we need to regenerate.
  if(!this->CheckBuildSystem())
    {
    return 0;
    }

  int ret = this->Configure();
  if (ret || this->GetWorkingMode() != NORMAL_MODE)
    {
#if defined(CMAKE_HAVE_VS_GENERATORS)
    if(!this->VSSolutionFile.empty() && this->GlobalGenerator)
      {
      // CMake is running to regenerate a Visual Studio build tree
      // during a build from the VS IDE.  The build files cannot be
      // regenerated, so we should stop the build.
      cmSystemTools::Message(
        "CMake Configure step failed.  "
        "Build files cannot be regenerated correctly.  "
        "Attempting to stop IDE build.");
      cmGlobalVisualStudioGenerator* gg =
        static_cast<cmGlobalVisualStudioGenerator*>(this->GlobalGenerator);
      gg->CallVisualStudioMacro(cmGlobalVisualStudioGenerator::MacroStop,
                                this->VSSolutionFile.c_str());
      }
#endif
    return ret;
    }
  ret = this->Generate();
  std::string message = "Build files have been written to: ";
  message += this->GetHomeOutputDirectory();
  this->UpdateProgress(message.c_str(), -1);
  return ret;
}

int cmake::Generate()
{
  if(!this->GlobalGenerator)
    {
    return -1;
    }
  this->GlobalGenerator->DoGenerate();
  if ( !this->GraphVizFile.empty() )
    {
    std::cout << "Generate graphviz: " << this->GraphVizFile << std::endl;
    this->GenerateGraphViz(this->GraphVizFile.c_str());
    }
  if(this->WarnUnusedCli)
    {
    this->RunCheckForUnusedVariables();
    }
  if(cmSystemTools::GetErrorOccuredFlag())
    {
    return -1;
    }
  // Save the cache again after a successful Generate so that any internal
  // variables created during Generate are saved. (Specifically target GUIDs
  // for the Visual Studio and Xcode generators.)
  if ( this->GetWorkingMode() == NORMAL_MODE )
    {
    this->CacheManager->SaveCache(this->GetHomeOutputDirectory());
    }
  return 0;
}

void cmake::AddCacheEntry(const std::string& key, const char* value,
                          const char* helpString,
                          int type)
{
  this->CacheManager->AddCacheEntry(key, value,
                                    helpString,
                                    cmState::CacheEntryType(type));
}

const char* cmake::GetCacheDefinition(const std::string& name) const
{
  return this->CacheManager->GetInitializedCacheValue(name);
}

void cmake::AddDefaultCommands()
{
  std::vector<cmCommand*> commands;
  GetBootstrapCommands1(commands);
  GetBootstrapCommands2(commands);
  GetPredefinedCommands(commands);
  for(std::vector<cmCommand*>::iterator i = commands.begin();
      i != commands.end(); ++i)
    {
    this->State->AddCommand(*i);
    }
}

void cmake::AddDefaultGenerators()
{
#if defined(_WIN32) && !defined(__CYGWIN__)
# if !defined(CMAKE_BOOT_MINGW)
  this->Generators.push_back(
    cmGlobalVisualStudio14Generator::NewFactory());
  this->Generators.push_back(
    cmGlobalVisualStudio12Generator::NewFactory());
  this->Generators.push_back(
    cmGlobalVisualStudio11Generator::NewFactory());
  this->Generators.push_back(
    cmGlobalVisualStudio10Generator::NewFactory());
  this->Generators.push_back(
    cmGlobalVisualStudio9Generator::NewFactory());
  this->Generators.push_back(
    cmGlobalVisualStudio8Generator::NewFactory());
  this->Generators.push_back(
    cmGlobalVisualStudio71Generator::NewFactory());
  this->Generators.push_back(
    cmGlobalVisualStudio7Generator::NewFactory());
  this->Generators.push_back(
    cmGlobalVisualStudio6Generator::NewFactory());
  this->Generators.push_back(
    cmGlobalBorlandMakefileGenerator::NewFactory());
  this->Generators.push_back(
    cmGlobalNMakeMakefileGenerator::NewFactory());
  this->Generators.push_back(
    cmGlobalJOMMakefileGenerator::NewFactory());
  this->Generators.push_back(
    cmGlobalGhsMultiGenerator::NewFactory());
# endif
  this->Generators.push_back(
    cmGlobalMSYSMakefileGenerator::NewFactory());
  this->Generators.push_back(
    cmGlobalMinGWMakefileGenerator::NewFactory());
#endif
  this->Generators.push_back(
    cmGlobalUnixMakefileGenerator3::NewFactory());
  this->Generators.push_back(
    cmGlobalNinjaGenerator::NewFactory());
#if defined(CMAKE_USE_WMAKE)
  this->Generators.push_back(
    cmGlobalWatcomWMakeGenerator::NewFactory());
#endif
#ifdef CMAKE_USE_XCODE
  this->Generators.push_back(
    cmGlobalXCodeGenerator::NewFactory());
#endif
}

bool cmake::ParseCacheEntry(const std::string& entry,
                            std::string& var,
                            std::string& value,
                            cmState::CacheEntryType& type)
{
  return cmCacheManager::ParseEntry(entry, var, value, type);
}

int cmake::LoadCache()
{
  // could we not read the cache
  if (!this->LoadCache(this->GetHomeOutputDirectory()))
    {
    // if it does exist, but isn't readable then warn the user
    std::string cacheFile = this->GetHomeOutputDirectory();
    cacheFile += "/CMakeCache.txt";
    if(cmSystemTools::FileExists(cacheFile.c_str()))
      {
      cmSystemTools::Error(
        "There is a CMakeCache.txt file for the current binary tree but "
        "cmake does not have permission to read it. Please check the "
        "permissions of the directory you are trying to run CMake on.");
      return -1;
      }
    }

  // setup CMAKE_ROOT and CMAKE_COMMAND
  if(!this->AddCMakePaths())
    {
    return -3;
    }
  return 0;
}

bool cmake::LoadCache(const std::string& path)
{
  return this->CacheManager->LoadCache(path);
}

bool cmake::LoadCache(const std::string& path, bool internal,
                std::set<std::string>& excludes,
                std::set<std::string>& includes)
{
  return this->CacheManager->LoadCache(path, internal, excludes, includes);
}

bool cmake::SaveCache(const std::string& path)
{
  return this->CacheManager->SaveCache(path);
}

bool cmake::DeleteCache(const std::string& path)
{
  return this->CacheManager->DeleteCache(path);
}

void cmake::SetProgressCallback(ProgressCallbackType f, void *cd)
{
  this->ProgressCallback = f;
  this->ProgressCallbackClientData = cd;
}

void cmake::UpdateProgress(const char *msg, float prog)
{
  if(this->ProgressCallback && !this->State->GetIsInTryCompile())
    {
    (*this->ProgressCallback)(msg, prog, this->ProgressCallbackClientData);
    return;
    }
}

bool cmake::GetIsInTryCompile() const
{
  return this->State->GetIsInTryCompile();
}

void cmake::SetIsInTryCompile(bool b)
{
  this->State->SetIsInTryCompile(b);
}

void cmake::GetGeneratorDocumentation(std::vector<cmDocumentationEntry>& v)
{
  for(RegisteredGeneratorsVector::const_iterator i =
      this->Generators.begin(); i != this->Generators.end(); ++i)
    {
    cmDocumentationEntry e;
    (*i)->GetDocumentation(e);
    v.push_back(e);
    }
  for(RegisteredExtraGeneratorsMap::const_iterator i =
      this->ExtraGenerators.begin(); i != this->ExtraGenerators.end(); ++i)
    {
    cmDocumentationEntry e;
    cmExternalMakefileProjectGenerator* generator = (i->second)();
    generator->GetDocumentation(e, i->first);
    e.Name = i->first;
    delete generator;
    v.push_back(e);
    }
}

void cmake::PrintGeneratorList()
{
#ifdef CMAKE_BUILD_WITH_CMAKE
  cmDocumentation doc;
  std::vector<cmDocumentationEntry> generators;
  this->GetGeneratorDocumentation(generators);
  doc.AppendSection("Generators",generators);
  std::cerr << "\n";
  doc.PrintDocumentation(cmDocumentation::ListGenerators, std::cerr);
#endif
}

void cmake::UpdateConversionPathTable()
{
  // Update the path conversion table with any specified file:
  const char* tablepath =
    this->CacheManager
        ->GetInitializedCacheValue("CMAKE_PATH_TRANSLATION_FILE");

  if(tablepath)
    {
    cmsys::ifstream table( tablepath );
    if(!table)
      {
      cmSystemTools::Error("CMAKE_PATH_TRANSLATION_FILE set to ", tablepath,
        ". CMake can not open file.");
      cmSystemTools::ReportLastSystemError("CMake can not open file.");
      }
    else
      {
      std::string a, b;
      while(!table.eof())
        {
        // two entries per line
        table >> a; table >> b;
        cmSystemTools::AddTranslationPath( a, b);
        }
      }
    }
}

//----------------------------------------------------------------------------
int cmake::CheckBuildSystem()
{
  // We do not need to rerun CMake.  Check dependency integrity.  Use
  // the make system's VERBOSE environment variable to enable verbose
  // output. This can be skipped by setting CMAKE_NO_VERBOSE (which is set
  // by the Eclipse and KDevelop generators).
  bool verbose = ((cmSystemTools::GetEnv("VERBOSE") != 0)
                   && (cmSystemTools::GetEnv("CMAKE_NO_VERBOSE") == 0));

  // This method will check the integrity of the build system if the
  // option was given on the command line.  It reads the given file to
  // determine whether CMake should rerun.

  // If no file is provided for the check, we have to rerun.
  if(this->CheckBuildSystemArgument.empty())
    {
    if(verbose)
      {
      std::ostringstream msg;
      msg << "Re-run cmake no build system arguments\n";
      cmSystemTools::Stdout(msg.str().c_str());
      }
    return 1;
    }

  // If the file provided does not exist, we have to rerun.
  if(!cmSystemTools::FileExists(this->CheckBuildSystemArgument.c_str()))
    {
    if(verbose)
      {
      std::ostringstream msg;
      msg << "Re-run cmake missing file: "
          << this->CheckBuildSystemArgument << "\n";
      cmSystemTools::Stdout(msg.str().c_str());
      }
    return 1;
    }

  // Read the rerun check file and use it to decide whether to do the
  // global generate.
  cmake cm;
  cm.SetHomeDirectory("");
  cm.SetHomeOutputDirectory("");
  cmGlobalGenerator gg(&cm);
  cmsys::auto_ptr<cmLocalGenerator> lg(gg.MakeLocalGenerator());
  cmMakefile* mf = lg->GetMakefile();
  if(!mf->ReadListFile(this->CheckBuildSystemArgument.c_str()) ||
     cmSystemTools::GetErrorOccuredFlag())
    {
    if(verbose)
      {
      std::ostringstream msg;
      msg << "Re-run cmake error reading : "
          << this->CheckBuildSystemArgument << "\n";
      cmSystemTools::Stdout(msg.str().c_str());
      }
    // There was an error reading the file.  Just rerun.
    return 1;
    }

  if(this->ClearBuildSystem)
    {
    // Get the generator used for this build system.
    const char* genName = mf->GetDefinition("CMAKE_DEPENDS_GENERATOR");
    if(!genName || genName[0] == '\0')
      {
      genName = "Unix Makefiles";
      }

    // Create the generator and use it to clear the dependencies.
    cmsys::auto_ptr<cmGlobalGenerator>
      ggd(this->CreateGlobalGenerator(genName));
    if(ggd.get())
      {
      cmsys::auto_ptr<cmLocalGenerator> lgd(ggd->MakeLocalGenerator());
      lgd->ClearDependencies(mf, verbose);
      }
    }

  // If any byproduct of makefile generation is missing we must re-run.
  std::vector<std::string> products;
  if(const char* productStr = mf->GetDefinition("CMAKE_MAKEFILE_PRODUCTS"))
    {
    cmSystemTools::ExpandListArgument(productStr, products);
    }
  for(std::vector<std::string>::const_iterator pi = products.begin();
      pi != products.end(); ++pi)
    {
    if(!(cmSystemTools::FileExists(pi->c_str()) ||
         cmSystemTools::FileIsSymlink(*pi)))
      {
      if(verbose)
        {
        std::ostringstream msg;
        msg << "Re-run cmake, missing byproduct: " << *pi << "\n";
        cmSystemTools::Stdout(msg.str().c_str());
        }
      return 1;
      }
    }

  // Get the set of dependencies and outputs.
  std::vector<std::string> depends;
  std::vector<std::string> outputs;
  const char* dependsStr = mf->GetDefinition("CMAKE_MAKEFILE_DEPENDS");
  const char* outputsStr = mf->GetDefinition("CMAKE_MAKEFILE_OUTPUTS");
  if(dependsStr && outputsStr)
    {
    cmSystemTools::ExpandListArgument(dependsStr, depends);
    cmSystemTools::ExpandListArgument(outputsStr, outputs);
    }
  if(depends.empty() || outputs.empty())
    {
    // Not enough information was provided to do the test.  Just rerun.
    if(verbose)
      {
      std::ostringstream msg;
      msg << "Re-run cmake no CMAKE_MAKEFILE_DEPENDS "
        "or CMAKE_MAKEFILE_OUTPUTS :\n";
      cmSystemTools::Stdout(msg.str().c_str());
      }
    return 1;
    }

  // Find the newest dependency.
  std::vector<std::string>::iterator dep = depends.begin();
  std::string dep_newest = *dep++;
  for(;dep != depends.end(); ++dep)
    {
    int result = 0;
    if(this->FileComparison->FileTimeCompare(dep_newest.c_str(),
                                             dep->c_str(), &result))
      {
      if(result < 0)
        {
        dep_newest = *dep;
        }
      }
    else
      {
      if(verbose)
        {
        std::ostringstream msg;
        msg << "Re-run cmake: build system dependency is missing\n";
        cmSystemTools::Stdout(msg.str().c_str());
        }
      return 1;
      }
    }

  // Find the oldest output.
  std::vector<std::string>::iterator out = outputs.begin();
  std::string out_oldest = *out++;
  for(;out != outputs.end(); ++out)
    {
    int result = 0;
    if(this->FileComparison->FileTimeCompare(out_oldest.c_str(),
                                             out->c_str(), &result))
      {
      if(result > 0)
        {
        out_oldest = *out;
        }
      }
    else
      {
      if(verbose)
        {
        std::ostringstream msg;
        msg << "Re-run cmake: build system output is missing\n";
        cmSystemTools::Stdout(msg.str().c_str());
        }
      return 1;
      }
    }

  // If any output is older than any dependency then rerun.
  {
  int result = 0;
  if(!this->FileComparison->FileTimeCompare(out_oldest.c_str(),
                                            dep_newest.c_str(),
                                            &result) ||
     result < 0)
    {
    if(verbose)
      {
      std::ostringstream msg;
      msg << "Re-run cmake file: " << out_oldest
          << " older than: " << dep_newest << "\n";
      cmSystemTools::Stdout(msg.str().c_str());
      }
    return 1;
    }
  }

  // No need to rerun.
  return 0;
}

//----------------------------------------------------------------------------
void cmake::TruncateOutputLog(const char* fname)
{
  std::string fullPath = this->GetHomeOutputDirectory();
  fullPath += "/";
  fullPath += fname;
  struct stat st;
  if ( ::stat(fullPath.c_str(), &st) )
    {
    return;
    }
  if (!this->State->GetInitializedCacheValue("CMAKE_CACHEFILE_DIR"))
    {
    cmSystemTools::RemoveFile(fullPath);
    return;
    }
  off_t fsize = st.st_size;
  const off_t maxFileSize = 50 * 1024;
  if ( fsize < maxFileSize )
    {
    //TODO: truncate file
    return;
    }
}

inline std::string removeQuotes(const std::string& s)
{
  if(s[0] == '\"' && s[s.size()-1] == '\"')
    {
    return s.substr(1, s.size()-2);
    }
  return s;
}

void cmake::MarkCliAsUsed(const std::string& variable)
{
  this->UsedCliVariables[variable] = true;
}

void cmake::GenerateGraphViz(const char* fileName) const
{
#ifdef CMAKE_BUILD_WITH_CMAKE
  cmsys::auto_ptr<cmGraphVizWriter> gvWriter(
       new cmGraphVizWriter(this->GetGlobalGenerator()->GetLocalGenerators()));

  std::string settingsFile = this->GetHomeOutputDirectory();
  settingsFile += "/CMakeGraphVizOptions.cmake";
  std::string fallbackSettingsFile = this->GetHomeDirectory();
  fallbackSettingsFile += "/CMakeGraphVizOptions.cmake";

  gvWriter->ReadSettings(settingsFile.c_str(), fallbackSettingsFile.c_str());

  gvWriter->WritePerTargetFiles(fileName);
  gvWriter->WriteTargetDependersFiles(fileName);
  gvWriter->WriteGlobalFile(fileName);

#endif
}

void cmake::SetProperty(const std::string& prop, const char* value)
{
  this->State->SetGlobalProperty(prop, value);
}

void cmake::AppendProperty(const std::string& prop,
                           const char* value, bool asString)
{
  this->State->AppendGlobalProperty(prop, value, asString);
}

const char *cmake::GetProperty(const std::string& prop)
{
  return this->State->GetGlobalProperty(prop);
}

bool cmake::GetPropertyAsBool(const std::string& prop)
{
  return this->State->GetGlobalPropertyAsBool(prop);
}

cmInstalledFile *cmake::GetOrCreateInstalledFile(
  cmMakefile* mf, const std::string& name)
{
  std::map<std::string, cmInstalledFile>::iterator i =
    this->InstalledFiles.find(name);

  if(i != this->InstalledFiles.end())
    {
    cmInstalledFile &file = i->second;
    return &file;
    }
  else
    {
    cmInstalledFile &file = this->InstalledFiles[name];
    file.SetName(mf, name);
    return &file;
    }
}

cmInstalledFile const* cmake::GetInstalledFile(const std::string& name) const
{
  std::map<std::string, cmInstalledFile>::const_iterator i =
    this->InstalledFiles.find(name);

  if(i != this->InstalledFiles.end())
    {
    cmInstalledFile const& file = i->second;
    return &file;
    }
  else
    {
    return 0;
    }
}

int cmake::GetSystemInformation(std::vector<std::string>& args)
{
  // so create the directory
  std::string resultFile;
  std::string cwd = cmSystemTools::GetCurrentWorkingDirectory();
  std::string destPath = cwd + "/__cmake_systeminformation";
  cmSystemTools::RemoveADirectory(destPath);
  if (!cmSystemTools::MakeDirectory(destPath.c_str()))
    {
    std::cerr << "Error: --system-information must be run from a "
      "writable directory!\n";
    return 1;
    }

  // process the arguments
  bool writeToStdout = true;
  for(unsigned int i=1; i < args.size(); ++i)
    {
    std::string arg = args[i];
    if(arg.find("-V",0) == 0)
      {
      this->Verbose = true;
      }
    else if(arg.find("-G",0) == 0)
      {
      std::string value = arg.substr(2);
      if(value.empty())
        {
        ++i;
        if(i >= args.size())
          {
          cmSystemTools::Error("No generator specified for -G");
          this->PrintGeneratorList();
          return -1;
          }
        value = args[i];
        }
      cmGlobalGenerator* gen =
        this->CreateGlobalGenerator(value);
      if(!gen)
        {
        cmSystemTools::Error("Could not create named generator ",
                             value.c_str());
        this->PrintGeneratorList();
        }
      else
        {
        this->SetGlobalGenerator(gen);
        }
      }
    // no option assume it is the output file
    else
      {
      if (!cmSystemTools::FileIsFullPath(arg.c_str()))
        {
        resultFile = cwd;
        resultFile += "/";
        }
      resultFile += arg;
      writeToStdout = false;
      }
    }


  // we have to find the module directory, so we can copy the files
  this->AddCMakePaths();
  std::string modulesPath =
    this->State->GetInitializedCacheValue("CMAKE_ROOT");
  modulesPath += "/Modules";
  std::string inFile = modulesPath;
  inFile += "/SystemInformation.cmake";
  std::string outFile = destPath;
  outFile += "/CMakeLists.txt";

  // Copy file
  if(!cmSystemTools::cmCopyFile(inFile.c_str(), outFile.c_str()))
    {
    std::cerr << "Error copying file \"" << inFile
              << "\" to \"" << outFile << "\".\n";
    return 1;
    }

  // do we write to a file or to stdout?
  if (resultFile.empty())
    {
    resultFile = cwd;
    resultFile += "/__cmake_systeminformation/results.txt";
    }

  // now run cmake on the CMakeLists file
  cmSystemTools::ChangeDirectory(destPath);
  std::vector<std::string> args2;
  args2.push_back(args[0]);
  args2.push_back(destPath);
  std::string resultArg = "-DRESULT_FILE=";
  resultArg += resultFile;
  args2.push_back(resultArg);
  int res = this->Run(args2, false);

  if (res != 0)
    {
    std::cerr << "Error: --system-information failed on internal CMake!\n";
    return res;
    }

  // change back to the original directory
  cmSystemTools::ChangeDirectory(cwd);

  // echo results to stdout if needed
  if (writeToStdout)
    {
    FILE* fin = cmsys::SystemTools::Fopen(resultFile, "r");
    if(fin)
      {
      const int bufferSize = 4096;
      char buffer[bufferSize];
      size_t n;
      while((n = fread(buffer, 1, bufferSize, fin)) > 0)
        {
        for(char* c = buffer; c < buffer+n; ++c)
          {
          putc(*c, stdout);
          }
        fflush(stdout);
        }
      fclose(fin);
      }
    }

  // clean up the directory
  cmSystemTools::RemoveADirectory(destPath);
  return 0;
}

//----------------------------------------------------------------------------
static bool cmakeCheckStampFile(const char* stampName)
{
  // The stamp file does not exist.  Use the stamp dependencies to
  // determine whether it is really out of date.  This works in
  // conjunction with cmLocalVisualStudio7Generator to avoid
  // repeatedly re-running CMake when the user rebuilds the entire
  // solution.
  std::string stampDepends = stampName;
  stampDepends += ".depend";
#if defined(_WIN32) || defined(__CYGWIN__)
  cmsys::ifstream fin(stampDepends.c_str(), std::ios::in | std::ios::binary);
#else
  cmsys::ifstream fin(stampDepends.c_str(), std::ios::in);
#endif
  if(!fin)
    {
    // The stamp dependencies file cannot be read.  Just assume the
    // build system is really out of date.
    std::cout << "CMake is re-running because " << stampName
              << " dependency file is missing.\n";
    return false;
    }

  // Compare the stamp dependencies against the dependency file itself.
  cmFileTimeComparison ftc;
  std::string dep;
  while(cmSystemTools::GetLineFromStream(fin, dep))
    {
    int result;
    if(!dep.empty() && dep[0] != '#' &&
       (!ftc.FileTimeCompare(stampDepends.c_str(), dep.c_str(), &result)
        || result < 0))
      {
      // The stamp depends file is older than this dependency.  The
      // build system is really out of date.
      std::cout << "CMake is re-running because " << stampName
                << " is out-of-date.\n";
      std::cout << "  the file '" << dep << "'\n";
      std::cout << "  is newer than '" << stampDepends << "'\n";
      std::cout << "  result='" << result << "'\n";
      return false;
      }
    }

  // The build system is up to date.  The stamp file has been removed
  // by the VS IDE due to a "rebuild" request.  Restore it atomically.
  std::ostringstream stampTempStream;
  stampTempStream << stampName << ".tmp" << cmSystemTools::RandomSeed();
  std::string stampTempString = stampTempStream.str();
  const char* stampTemp = stampTempString.c_str();
  {
  // TODO: Teach cmGeneratedFileStream to use a random temp file (with
  // multiple tries in unlikely case of conflict) and use that here.
  cmsys::ofstream stamp(stampTemp);
  stamp << "# CMake generation timestamp file for this directory.\n";
  }
  if(cmSystemTools::RenameFile(stampTemp, stampName))
    {
    // Notify the user why CMake is not re-running.  It is safe to
    // just print to stdout here because this code is only reachable
    // through an undocumented flag used by the VS generator.
    std::cout << "CMake does not need to re-run because "
              << stampName << " is up-to-date.\n";
    return true;
    }
  else
    {
    cmSystemTools::RemoveFile(stampTemp);
    cmSystemTools::Error("Cannot restore timestamp ", stampName);
    return false;
    }
}

//----------------------------------------------------------------------------
static bool cmakeCheckStampList(const char* stampList)
{
  // If the stamp list does not exist CMake must rerun to generate it.
  if(!cmSystemTools::FileExists(stampList))
    {
    std::cout << "CMake is re-running because generate.stamp.list "
              << "is missing.\n";
    return false;
    }
  cmsys::ifstream fin(stampList);
  if(!fin)
    {
    std::cout << "CMake is re-running because generate.stamp.list "
              << "could not be read.\n";
    return false;
    }

  // Check each stamp.
  std::string stampName;
  while(cmSystemTools::GetLineFromStream(fin, stampName))
    {
    if(!cmakeCheckStampFile(stampName.c_str()))
      {
      return false;
      }
    }
  return true;
}

bool cmake::PrintMessagePreamble(cmake::MessageType t, std::ostream& msg)
{
  // Construct the message header.
  if(t == cmake::FATAL_ERROR)
    {
    msg << "CMake Error";
    }
  else if(t == cmake::INTERNAL_ERROR)
    {
    msg << "CMake Internal Error (please report a bug)";
    }
  else if(t == cmake::LOG)
    {
    msg << "CMake Debug Log";
    }
  else if(t == cmake::DEPRECATION_ERROR)
    {
    msg << "CMake Deprecation Error";
    }
  else if (t == cmake::DEPRECATION_WARNING)
    {
    msg << "CMake Deprecation Warning";
    }
  else
    {
    msg << "CMake Warning";
    if(t == cmake::AUTHOR_WARNING)
      {
      // Allow suppression of these warnings.
      const char* suppress = this->State->GetCacheEntryValue(
                                        "CMAKE_SUPPRESS_DEVELOPER_WARNINGS");
      if(suppress && cmSystemTools::IsOn(suppress))
        {
        return false;
        }
      msg << " (dev)";
      }
    }
  return true;
}

void printMessageText(std::ostream& msg, std::string const& text)
{
   msg << ":\n";
   cmDocumentationFormatter formatter;
   formatter.SetIndent("  ");
   formatter.PrintFormatted(msg, text.c_str());
}

void displayMessage(cmake::MessageType t, std::ostringstream& msg)
{

  // Add a note about warning suppression.
  if(t == cmake::AUTHOR_WARNING)
    {
    msg <<
      "This warning is for project developers.  Use -Wno-dev to suppress it.";
    }

  // Add a terminating blank line.
  msg << "\n";

#if defined(CMAKE_BUILD_WITH_CMAKE)
  // Add a C++ stack trace to internal errors.
  if(t == cmake::INTERNAL_ERROR)
    {
    std::string stack = cmsys::SystemInformation::GetProgramStack(0,0);
    if(!stack.empty())
      {
      if(cmHasLiteralPrefix(stack, "WARNING:"))
        {
        stack = "Note:" + stack.substr(8);
        }
      msg << stack << "\n";
      }
    }
#endif

  // Output the message.
  if(t == cmake::FATAL_ERROR
     || t == cmake::INTERNAL_ERROR
     || t == cmake::DEPRECATION_ERROR)
    {
    cmSystemTools::SetErrorOccured();
    cmSystemTools::Message(msg.str().c_str(), "Error");
    }
  else
    {
    cmSystemTools::Message(msg.str().c_str(), "Warning");
    }
}

//----------------------------------------------------------------------------
void cmake::IssueMessage(cmake::MessageType t, std::string const& text,
                         cmListFileBacktrace const& bt)
{
  cmListFileBacktrace backtrace = bt;
  backtrace.MakeRelative();

  std::ostringstream msg;
  if (!this->PrintMessagePreamble(t, msg))
    {
    return;
    }

  // Add the immediate context.
  backtrace.PrintTitle(msg);

  printMessageText(msg, text);

  // Add the rest of the context.
  backtrace.PrintCallStack(msg);

  displayMessage(t, msg);
}

//----------------------------------------------------------------------------
void cmake::IssueMessage(cmake::MessageType t, std::string const& text,
                         cmListFileContext const& lfc)
{
  std::ostringstream msg;
  if (!this->PrintMessagePreamble(t, msg))
    {
    return;
    }

  // Add the immediate context.
  msg << (lfc.Line ? " at " : " in ") << lfc;

  printMessageText(msg, text);

  displayMessage(t, msg);
}

//----------------------------------------------------------------------------
std::vector<std::string> cmake::GetDebugConfigs()
{
  std::vector<std::string> configs;
  if(const char* config_list =
                      this->State->GetGlobalProperty("DEBUG_CONFIGURATIONS"))
    {
    // Expand the specified list and convert to upper-case.
    cmSystemTools::ExpandListArgument(config_list, configs);
    std::transform(configs.begin(),
                   configs.end(),
                   configs.begin(),
                   cmSystemTools::UpperCase);
    }
  // If no configurations were specified, use a default list.
  if(configs.empty())
    {
    configs.push_back("DEBUG");
    }
  return configs;
}


int cmake::Build(const std::string& dir,
                 const std::string& target,
                 const std::string& config,
                 const std::vector<std::string>& nativeOptions,
                 bool clean)
{

  this->SetHomeDirectory("");
  this->SetHomeOutputDirectory("");
  if(!cmSystemTools::FileIsDirectory(dir))
    {
    std::cerr << "Error: " << dir << " is not a directory\n";
    return 1;
    }
  std::string cachePath = dir;
  cmSystemTools::ConvertToUnixSlashes(cachePath);
  if(!this->LoadCache(cachePath))
    {
    std::cerr << "Error: could not load cache\n";
    return 1;
    }
  const char* cachedGenerator =
      this->State->GetCacheEntryValue("CMAKE_GENERATOR");
  if(!cachedGenerator)
    {
    std::cerr << "Error: could not find CMAKE_GENERATOR in Cache\n";
    return 1;
    }
  cmsys::auto_ptr<cmGlobalGenerator> gen(
    this->CreateGlobalGenerator(cachedGenerator));
  if(!gen.get())
    {
    std::cerr << "Error: could create CMAKE_GENERATOR \""
              << cachedGenerator << "\"\n";
    return 1;
    }
  std::string output;
  std::string projName;
  const char* cachedProjectName =
      this->State->GetCacheEntryValue("CMAKE_PROJECT_NAME");
  if(!cachedProjectName)
    {
    std::cerr << "Error: could not find CMAKE_PROJECT_NAME in Cache\n";
    return 1;
    }
  projName = cachedProjectName;
  bool verbose = false;
  const char* cachedVerbose =
      this->State->GetCacheEntryValue("CMAKE_VERBOSE_MAKEFILE");
  if(cachedVerbose)
    {
    verbose = cmSystemTools::IsOn(cachedVerbose);
    }
  return gen->Build("", dir,
                    projName, target,
                    output,
                    "",
                    config, clean, false, verbose, 0,
                    cmSystemTools::OUTPUT_PASSTHROUGH,
                    nativeOptions);
}

void cmake::WatchUnusedCli(const std::string& var)
{
#ifdef CMAKE_BUILD_WITH_CMAKE
  this->VariableWatch->AddWatch(var, cmWarnUnusedCliWarning, this);
  if(this->UsedCliVariables.find(var) == this->UsedCliVariables.end())
    {
    this->UsedCliVariables[var] = false;
    }
#endif
}

void cmake::UnwatchUnusedCli(const std::string& var)
{
#ifdef CMAKE_BUILD_WITH_CMAKE
  this->VariableWatch->RemoveWatch(var, cmWarnUnusedCliWarning);
  this->UsedCliVariables.erase(var);
#endif
}

void cmake::RunCheckForUnusedVariables()
{
#ifdef CMAKE_BUILD_WITH_CMAKE
  bool haveUnused = false;
  std::ostringstream msg;
  msg << "Manually-specified variables were not used by the project:";
  for(std::map<std::string, bool>::const_iterator
        it = this->UsedCliVariables.begin();
      it != this->UsedCliVariables.end(); ++it)
    {
    if(!it->second)
      {
      haveUnused = true;
      msg << "\n  " << it->first;
      }
    }
  if(haveUnused)
    {
    this->IssueMessage(cmake::WARNING, msg.str());
    }
#endif
}
