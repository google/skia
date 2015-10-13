
/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "cmGlobalVisualStudioGenerator.h"

#include "cmCallVisualStudioMacro.h"
#include "cmGeneratorTarget.h"
#include "cmLocalVisualStudioGenerator.h"
#include "cmMakefile.h"
#include "cmSourceFile.h"
#include "cmTarget.h"
#include <cmsys/Encoding.hxx>

//----------------------------------------------------------------------------
cmGlobalVisualStudioGenerator::cmGlobalVisualStudioGenerator(cmake* cm)
  : cmGlobalGenerator(cm)
{
  cm->GetState()->SetWindowsShell(true);
  cm->GetState()->SetWindowsVSIDE(true);
}

//----------------------------------------------------------------------------
cmGlobalVisualStudioGenerator::~cmGlobalVisualStudioGenerator()
{
}

//----------------------------------------------------------------------------
cmGlobalVisualStudioGenerator::VSVersion
cmGlobalVisualStudioGenerator::GetVersion() const
{
  return this->Version;
}

//----------------------------------------------------------------------------
void cmGlobalVisualStudioGenerator::SetVersion(VSVersion v)
{
  this->Version = v;
}

//----------------------------------------------------------------------------
std::string cmGlobalVisualStudioGenerator::GetRegistryBase()
{
  return cmGlobalVisualStudioGenerator::GetRegistryBase(
    this->GetIDEVersion());
}

//----------------------------------------------------------------------------
std::string cmGlobalVisualStudioGenerator::GetRegistryBase(
  const char* version)
{
  std::string key = "HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\VisualStudio\\";
  return key + version;
}

//----------------------------------------------------------------------------
void cmGlobalVisualStudioGenerator::Generate()
{
  // Add a special target that depends on ALL projects for easy build
  // of one configuration only.
  const char* no_working_dir = 0;
  std::vector<std::string> no_depends;
  cmCustomCommandLines no_commands;
  std::map<std::string, std::vector<cmLocalGenerator*> >::iterator it;
  for(it = this->ProjectMap.begin(); it!= this->ProjectMap.end(); ++it)
    {
    std::vector<cmLocalGenerator*>& gen = it->second;
    // add the ALL_BUILD to the first local generator of each project
    if(gen.size())
      {
      // Use no actual command lines so that the target itself is not
      // considered always out of date.
      cmTarget* allBuild =
        gen[0]->GetMakefile()->
        AddUtilityCommand("ALL_BUILD", true, no_working_dir,
                          no_depends, no_commands, false,
                          "Build all projects");

#if 0
      // Can't activate this code because we want ALL_BUILD
      // selected as the default "startup project" when first
      // opened in Visual Studio... And if it's nested in a
      // folder, then that doesn't happen.
      //
      // Organize in the "predefined targets" folder:
      //
      if (this->UseFolderProperty())
        {
        allBuild->SetProperty("FOLDER", this->GetPredefinedTargetsFolder());
        }
#endif

      // Now make all targets depend on the ALL_BUILD target
      for(std::vector<cmLocalGenerator*>::iterator i = gen.begin();
          i != gen.end(); ++i)
        {
        cmTargets& targets = (*i)->GetMakefile()->GetTargets();
        for(cmTargets::iterator t = targets.begin();
            t != targets.end(); ++t)
          {
          if(!this->IsExcluded(gen[0], t->second))
            {
            allBuild->AddUtility(t->second.GetName());
            }
          }
        }
      }
    }

  // Configure CMake Visual Studio macros, for this user on this version
  // of Visual Studio.
  this->ConfigureCMakeVisualStudioMacros();

  // Add CMakeLists.txt with custom command to rerun CMake.
  for(std::vector<cmLocalGenerator*>::const_iterator
        lgi = this->LocalGenerators.begin();
      lgi != this->LocalGenerators.end(); ++lgi)
    {
    cmLocalVisualStudioGenerator* lg =
      static_cast<cmLocalVisualStudioGenerator*>(*lgi);
    lg->AddCMakeListsRules();
    }

  // Run all the local generators.
  this->cmGlobalGenerator::Generate();
}

//----------------------------------------------------------------------------
void cmGlobalVisualStudioGenerator
::ComputeTargetObjectDirectory(cmGeneratorTarget* gt) const
{
  std::string dir = gt->Makefile->GetCurrentBinaryDirectory();
  dir += "/";
  std::string tgtDir = gt->LocalGenerator->GetTargetDirectory(*gt->Target);
  if(!tgtDir.empty())
    {
    dir += tgtDir;
    dir += "/";
    }
  const char* cd = this->GetCMakeCFGIntDir();
  if(cd && *cd)
    {
    dir += cd;
    dir += "/";
    }
  gt->ObjectDirectory = dir;
}

//----------------------------------------------------------------------------
bool IsVisualStudioMacrosFileRegistered(const std::string& macrosFile,
  const std::string& regKeyBase,
  std::string& nextAvailableSubKeyName);

void RegisterVisualStudioMacros(const std::string& macrosFile,
  const std::string& regKeyBase);

//----------------------------------------------------------------------------
#define CMAKE_VSMACROS_FILENAME \
  "CMakeVSMacros2.vsmacros"

#define CMAKE_VSMACROS_RELOAD_MACRONAME \
  "Macros.CMakeVSMacros2.Macros.ReloadProjects"

#define CMAKE_VSMACROS_STOP_MACRONAME \
  "Macros.CMakeVSMacros2.Macros.StopBuild"

//----------------------------------------------------------------------------
void cmGlobalVisualStudioGenerator::ConfigureCMakeVisualStudioMacros()
{
  cmMakefile* mf = this->LocalGenerators[0]->GetMakefile();
  std::string dir = this->GetUserMacrosDirectory();

  if (mf != 0 && dir != "")
    {
    std::string src = mf->GetRequiredDefinition("CMAKE_ROOT");
    src += "/Templates/" CMAKE_VSMACROS_FILENAME;

    std::string dst = dir + "/CMakeMacros/" CMAKE_VSMACROS_FILENAME;

    // Copy the macros file to the user directory only if the
    // destination does not exist or the source location is newer.
    // This will allow the user to edit the macros for development
    // purposes but newer versions distributed with CMake will replace
    // older versions in user directories.
    int res;
    if(!cmSystemTools::FileTimeCompare(src.c_str(), dst.c_str(), &res) ||
       res > 0)
      {
      if (!cmSystemTools::CopyFileAlways(src.c_str(), dst.c_str()))
        {
        std::ostringstream oss;
        oss << "Could not copy from: " << src << std::endl;
        oss << "                 to: " << dst << std::endl;
        cmSystemTools::Message(oss.str().c_str(), "Warning");
        }
      }

    RegisterVisualStudioMacros(dst, this->GetUserMacrosRegKeyBase());
    }
}

//----------------------------------------------------------------------------
void
cmGlobalVisualStudioGenerator
::CallVisualStudioMacro(MacroName m,
                        const char* vsSolutionFile)
{
  // If any solution or project files changed during the generation,
  // tell Visual Studio to reload them...
  cmMakefile* mf = this->LocalGenerators[0]->GetMakefile();
  std::string dir = this->GetUserMacrosDirectory();

  // Only really try to call the macro if:
  //  - mf is non-NULL
  //  - there is a UserMacrosDirectory
  //  - the CMake vsmacros file exists
  //  - the CMake vsmacros file is registered
  //  - there were .sln/.vcproj files changed during generation
  //
  if (mf != 0 && dir != "")
    {
    std::string macrosFile = dir + "/CMakeMacros/" CMAKE_VSMACROS_FILENAME;
    std::string nextSubkeyName;
    if (cmSystemTools::FileExists(macrosFile.c_str()) &&
      IsVisualStudioMacrosFileRegistered(macrosFile,
        this->GetUserMacrosRegKeyBase(), nextSubkeyName)
      )
      {
      std::string topLevelSlnName;
      if(vsSolutionFile)
        {
        topLevelSlnName = vsSolutionFile;
        }
      else
        {
        topLevelSlnName = mf->GetCurrentBinaryDirectory();
        topLevelSlnName += "/";
        topLevelSlnName += mf->GetProjectName();
        topLevelSlnName += ".sln";
        }

      if(m == MacroReload)
        {
        std::vector<std::string> filenames;
        this->GetFilesReplacedDuringGenerate(filenames);
        if (filenames.size() > 0)
          {
          // Convert vector to semi-colon delimited string of filenames:
          std::string projects;
          std::vector<std::string>::iterator it = filenames.begin();
          if (it != filenames.end())
            {
            projects = *it;
            ++it;
            }
          for (; it != filenames.end(); ++it)
            {
            projects += ";";
            projects += *it;
            }
          cmCallVisualStudioMacro::CallMacro(topLevelSlnName,
            CMAKE_VSMACROS_RELOAD_MACRONAME, projects,
            this->GetCMakeInstance()->GetDebugOutput());
          }
        }
      else if(m == MacroStop)
        {
        cmCallVisualStudioMacro::CallMacro(topLevelSlnName,
          CMAKE_VSMACROS_STOP_MACRONAME, "",
          this->GetCMakeInstance()->GetDebugOutput());
        }
      }
    }
}

//----------------------------------------------------------------------------
std::string cmGlobalVisualStudioGenerator::GetUserMacrosDirectory()
{
  return "";
}

//----------------------------------------------------------------------------
std::string cmGlobalVisualStudioGenerator::GetUserMacrosRegKeyBase()
{
  return "";
}

//----------------------------------------------------------------------------
void cmGlobalVisualStudioGenerator::FillLinkClosure(cmTarget const* target,
                                                    TargetSet& linked)
{
  if(linked.insert(target).second)
    {
    TargetDependSet const& depends = this->GetTargetDirectDepends(*target);
    for(TargetDependSet::const_iterator di = depends.begin();
        di != depends.end(); ++di)
      {
      if(di->IsLink())
        {
        this->FillLinkClosure(*di, linked);
        }
      }
    }
}

//----------------------------------------------------------------------------
cmGlobalVisualStudioGenerator::TargetSet const&
cmGlobalVisualStudioGenerator::GetTargetLinkClosure(cmTarget* target)
{
  TargetSetMap::iterator i = this->TargetLinkClosure.find(target);
  if(i == this->TargetLinkClosure.end())
    {
    TargetSetMap::value_type entry(target, TargetSet());
    i = this->TargetLinkClosure.insert(entry).first;
    this->FillLinkClosure(target, i->second);
    }
  return i->second;
}

//----------------------------------------------------------------------------
void cmGlobalVisualStudioGenerator::FollowLinkDepends(
  cmTarget const* target, std::set<cmTarget const*>& linked)
{
  if(target->GetType() == cmTarget::INTERFACE_LIBRARY)
    {
    return;
    }
  if(linked.insert(target).second &&
     target->GetType() == cmTarget::STATIC_LIBRARY)
    {
    // Static library targets do not list their link dependencies so
    // we must follow them transitively now.
    TargetDependSet const& depends = this->GetTargetDirectDepends(*target);
    for(TargetDependSet::const_iterator di = depends.begin();
        di != depends.end(); ++di)
      {
      if(di->IsLink())
        {
        this->FollowLinkDepends(*di, linked);
        }
      }
    }
}

//----------------------------------------------------------------------------
bool cmGlobalVisualStudioGenerator::ComputeTargetDepends()
{
  if(!this->cmGlobalGenerator::ComputeTargetDepends())
    {
    return false;
    }
  std::map<std::string, std::vector<cmLocalGenerator*> >::iterator it;
  for(it = this->ProjectMap.begin(); it!= this->ProjectMap.end(); ++it)
    {
    std::vector<cmLocalGenerator*>& gen = it->second;
    for(std::vector<cmLocalGenerator*>::iterator i = gen.begin();
        i != gen.end(); ++i)
      {
      cmTargets& targets = (*i)->GetMakefile()->GetTargets();
      for(cmTargets::iterator ti = targets.begin();
          ti != targets.end(); ++ti)
        {
        this->ComputeVSTargetDepends(ti->second);
        }
      }
    }
  return true;
}

//----------------------------------------------------------------------------
static bool VSLinkable(cmTarget const* t)
{
  return t->IsLinkable() || t->GetType() == cmTarget::OBJECT_LIBRARY;
}

//----------------------------------------------------------------------------
void cmGlobalVisualStudioGenerator::ComputeVSTargetDepends(cmTarget& target)
{
  if(this->VSTargetDepends.find(&target) != this->VSTargetDepends.end())
    {
    return;
    }
  VSDependSet& vsTargetDepend = this->VSTargetDepends[&target];
  // VS <= 7.1 has two behaviors that affect solution dependencies.
  //
  // (1) Solution-level dependencies between a linkable target and a
  // library cause that library to be linked.  We use an intermedite
  // empty utility target to express the dependency.  (VS 8 and above
  // provide a project file "LinkLibraryDependencies" setting to
  // choose whether to activate this behavior.  We disable it except
  // when linking external project files.)
  //
  // (2) We cannot let static libraries depend directly on targets to
  // which they "link" because the librarian tool will copy the
  // targets into the static library.  While the work-around for
  // behavior (1) would also avoid this, it would create a large
  // number of extra utility targets for little gain.  Instead, use
  // the above work-around only for dependencies explicitly added by
  // the add_dependencies() command.  Approximate link dependencies by
  // leaving them out for the static library itself but following them
  // transitively for other targets.

  bool allowLinkable = (target.GetType() != cmTarget::STATIC_LIBRARY &&
                        target.GetType() != cmTarget::SHARED_LIBRARY &&
                        target.GetType() != cmTarget::MODULE_LIBRARY &&
                        target.GetType() != cmTarget::EXECUTABLE);

  TargetDependSet const& depends = this->GetTargetDirectDepends(target);

  // Collect implicit link dependencies (target_link_libraries).
  // Static libraries cannot depend on their link implementation
  // due to behavior (2), but they do not really need to.
  std::set<cmTarget const*> linkDepends;
  if(target.GetType() != cmTarget::STATIC_LIBRARY)
    {
    for(TargetDependSet::const_iterator di = depends.begin();
        di != depends.end(); ++di)
      {
      cmTargetDepend dep = *di;
      if(dep.IsLink())
        {
        this->FollowLinkDepends(dep, linkDepends);
        }
      }
    }

  // Collect explicit util dependencies (add_dependencies).
  std::set<cmTarget const*> utilDepends;
  for(TargetDependSet::const_iterator di = depends.begin();
      di != depends.end(); ++di)
    {
    cmTargetDepend dep = *di;
    if(dep.IsUtil())
      {
      this->FollowLinkDepends(dep, utilDepends);
      }
    }

  // Collect all targets linked by this target so we can avoid
  // intermediate targets below.
  TargetSet linked;
  if(target.GetType() != cmTarget::STATIC_LIBRARY)
    {
    linked = this->GetTargetLinkClosure(&target);
    }

  // Emit link dependencies.
  for(std::set<cmTarget const*>::iterator di = linkDepends.begin();
      di != linkDepends.end(); ++di)
    {
    cmTarget const* dep = *di;
    vsTargetDepend.insert(dep->GetName());
    }

  // Emit util dependencies.  Possibly use intermediate targets.
  for(std::set<cmTarget const*>::iterator di = utilDepends.begin();
      di != utilDepends.end(); ++di)
    {
    cmTarget const* dep = *di;
    if(allowLinkable || !VSLinkable(dep) || linked.count(dep))
      {
      // Direct dependency allowed.
      vsTargetDepend.insert(dep->GetName());
      }
    else
      {
      // Direct dependency on linkable target not allowed.
      // Use an intermediate utility target.
      vsTargetDepend.insert(this->GetUtilityDepend(dep));
      }
    }
}

//----------------------------------------------------------------------------
void cmGlobalVisualStudioGenerator::FindMakeProgram(cmMakefile* mf)
{
  // Visual Studio generators know how to lookup their build tool
  // directly instead of needing a helper module to do it, so we
  // do not actually need to put CMAKE_MAKE_PROGRAM into the cache.
  if(cmSystemTools::IsOff(mf->GetDefinition("CMAKE_MAKE_PROGRAM")))
    {
    mf->AddDefinition("CMAKE_MAKE_PROGRAM",
                      this->GetVSMakeProgram().c_str());
    }
}

//----------------------------------------------------------------------------
std::string
cmGlobalVisualStudioGenerator::GetUtilityDepend(cmTarget const* target)
{
  UtilityDependsMap::iterator i = this->UtilityDepends.find(target);
  if(i == this->UtilityDepends.end())
    {
    std::string name = this->WriteUtilityDepend(target);
    UtilityDependsMap::value_type entry(target, name);
    i = this->UtilityDepends.insert(entry).first;
    }
  return i->second;
}

//----------------------------------------------------------------------------
#include <windows.h>

//----------------------------------------------------------------------------
bool IsVisualStudioMacrosFileRegistered(const std::string& macrosFile,
  const std::string& regKeyBase,
  std::string& nextAvailableSubKeyName)
{
  bool macrosRegistered = false;

  std::string s1;
  std::string s2;

  // Make lowercase local copies, convert to Unix slashes, and
  // see if the resulting strings are the same:
  s1 = cmSystemTools::LowerCase(macrosFile);
  cmSystemTools::ConvertToUnixSlashes(s1);

  std::string keyname;
  HKEY hkey = NULL;
  LONG result = ERROR_SUCCESS;
  DWORD index = 0;

  keyname = regKeyBase + "\\OtherProjects7";
  hkey = NULL;
  result = RegOpenKeyExW(HKEY_CURRENT_USER,
                         cmsys::Encoding::ToWide(keyname).c_str(),
                         0, KEY_READ, &hkey);
  if (ERROR_SUCCESS == result)
    {
    // Iterate the subkeys and look for the values of interest in each subkey:
    wchar_t subkeyname[256];
    DWORD cch_subkeyname = sizeof(subkeyname)*sizeof(subkeyname[0]);
    wchar_t keyclass[256];
    DWORD cch_keyclass = sizeof(keyclass)*sizeof(keyclass[0]);
    FILETIME lastWriteTime;
    lastWriteTime.dwHighDateTime = 0;
    lastWriteTime.dwLowDateTime = 0;

    while (ERROR_SUCCESS == RegEnumKeyExW(hkey, index, subkeyname,
                                         &cch_subkeyname,
      0, keyclass, &cch_keyclass, &lastWriteTime))
      {
      // Open the subkey and query the values of interest:
      HKEY hsubkey = NULL;
      result = RegOpenKeyExW(hkey, subkeyname, 0, KEY_READ, &hsubkey);
      if (ERROR_SUCCESS == result)
        {
        DWORD valueType = REG_SZ;
        wchar_t data1[256];
        DWORD cch_data1 = sizeof(data1)*sizeof(data1[0]);
        RegQueryValueExW(hsubkey, L"Path", 0, &valueType,
                        (LPBYTE) &data1[0], &cch_data1);

        DWORD data2 = 0;
        DWORD cch_data2 = sizeof(data2);
        RegQueryValueExW(hsubkey, L"Security", 0, &valueType,
                        (LPBYTE) &data2, &cch_data2);

        DWORD data3 = 0;
        DWORD cch_data3 = sizeof(data3);
        RegQueryValueExW(hsubkey, L"StorageFormat", 0, &valueType,
                        (LPBYTE) &data3, &cch_data3);

        s2 = cmSystemTools::LowerCase(cmsys::Encoding::ToNarrow(data1));
        cmSystemTools::ConvertToUnixSlashes(s2);
        if (s2 == s1)
          {
          macrosRegistered = true;
          }

        std::string fullname = cmsys::Encoding::ToNarrow(data1);
        std::string filename;
        std::string filepath;
        std::string filepathname;
        std::string filepathpath;
        if (cmSystemTools::FileExists(fullname.c_str()))
          {
          filename = cmSystemTools::GetFilenameName(fullname);
          filepath = cmSystemTools::GetFilenamePath(fullname);
          filepathname = cmSystemTools::GetFilenameName(filepath);
          filepathpath = cmSystemTools::GetFilenamePath(filepath);
          }

        //std::cout << keyname << "\\" << subkeyname << ":" << std::endl;
        //std::cout << "  Path: " << data1 << std::endl;
        //std::cout << "  Security: " << data2 << std::endl;
        //std::cout << "  StorageFormat: " << data3 << std::endl;
        //std::cout << "  filename: " << filename << std::endl;
        //std::cout << "  filepath: " << filepath << std::endl;
        //std::cout << "  filepathname: " << filepathname << std::endl;
        //std::cout << "  filepathpath: " << filepathpath << std::endl;
        //std::cout << std::endl;

        RegCloseKey(hsubkey);
        }
      else
        {
        std::cout << "error opening subkey: " << subkeyname << std::endl;
        std::cout << std::endl;
        }

      ++index;
      cch_subkeyname = sizeof(subkeyname)*sizeof(subkeyname[0]);
      cch_keyclass = sizeof(keyclass)*sizeof(keyclass[0]);
      lastWriteTime.dwHighDateTime = 0;
      lastWriteTime.dwLowDateTime = 0;
      }

    RegCloseKey(hkey);
    }
  else
    {
    std::cout << "error opening key: " << keyname << std::endl;
    std::cout << std::endl;
    }


  // Pass back next available sub key name, assuming sub keys always
  // follow the expected naming scheme. Expected naming scheme is that
  // the subkeys of OtherProjects7 is 0 to n-1, so it's ok to use "n"
  // as the name of the next subkey.
  std::ostringstream ossNext;
  ossNext << index;
  nextAvailableSubKeyName = ossNext.str();


  keyname = regKeyBase + "\\RecordingProject7";
  hkey = NULL;
  result = RegOpenKeyExW(HKEY_CURRENT_USER,
                         cmsys::Encoding::ToWide(keyname).c_str(),
                         0, KEY_READ, &hkey);
  if (ERROR_SUCCESS == result)
    {
    DWORD valueType = REG_SZ;
    wchar_t data1[256];
    DWORD cch_data1 = sizeof(data1)*sizeof(data1[0]);
    RegQueryValueExW(hkey, L"Path", 0, &valueType,
                    (LPBYTE) &data1[0], &cch_data1);

    DWORD data2 = 0;
    DWORD cch_data2 = sizeof(data2);
    RegQueryValueExW(hkey, L"Security", 0, &valueType,
                    (LPBYTE) &data2, &cch_data2);

    DWORD data3 = 0;
    DWORD cch_data3 = sizeof(data3);
    RegQueryValueExW(hkey, L"StorageFormat", 0, &valueType,
                    (LPBYTE) &data3, &cch_data3);

    s2 = cmSystemTools::LowerCase(cmsys::Encoding::ToNarrow(data1));
    cmSystemTools::ConvertToUnixSlashes(s2);
    if (s2 == s1)
      {
      macrosRegistered = true;
      }

    //std::cout << keyname << ":" << std::endl;
    //std::cout << "  Path: " << data1 << std::endl;
    //std::cout << "  Security: " << data2 << std::endl;
    //std::cout << "  StorageFormat: " << data3 << std::endl;
    //std::cout << std::endl;

    RegCloseKey(hkey);
    }
  else
    {
    std::cout << "error opening key: " << keyname << std::endl;
    std::cout << std::endl;
    }

  return macrosRegistered;
}

//----------------------------------------------------------------------------
void WriteVSMacrosFileRegistryEntry(
  const std::string& nextAvailableSubKeyName,
  const std::string& macrosFile,
  const std::string& regKeyBase)
{
  std::string keyname = regKeyBase + "\\OtherProjects7";
  HKEY hkey = NULL;
  LONG result = RegOpenKeyExW(HKEY_CURRENT_USER,
    cmsys::Encoding::ToWide(keyname).c_str(), 0,
    KEY_READ|KEY_WRITE, &hkey);
  if (ERROR_SUCCESS == result)
    {
    // Create the subkey and set the values of interest:
    HKEY hsubkey = NULL;
    wchar_t lpClass[] = L"";
    result = RegCreateKeyExW(hkey,
      cmsys::Encoding::ToWide(nextAvailableSubKeyName).c_str(), 0,
      lpClass, 0, KEY_READ|KEY_WRITE, 0, &hsubkey, 0);
    if (ERROR_SUCCESS == result)
      {
      DWORD dw = 0;

      std::string s(macrosFile);
      cmSystemTools::ReplaceString(s, "/", "\\");
      std::wstring ws = cmsys::Encoding::ToWide(s);

      result = RegSetValueExW(hsubkey, L"Path", 0, REG_SZ, (LPBYTE)ws.c_str(),
        static_cast<DWORD>(ws.size() + 1)*sizeof(wchar_t));
      if (ERROR_SUCCESS != result)
        {
        std::cout << "error result 1: " << result << std::endl;
        std::cout << std::endl;
        }

      // Security value is always "1" for sample macros files (seems to be "2"
      // if you put the file somewhere outside the standard VSMacros folder)
      dw = 1;
      result = RegSetValueExW(hsubkey, L"Security",
                             0, REG_DWORD, (LPBYTE) &dw, sizeof(DWORD));
      if (ERROR_SUCCESS != result)
        {
        std::cout << "error result 2: " << result << std::endl;
        std::cout << std::endl;
        }

      // StorageFormat value is always "0" for sample macros files
      dw = 0;
      result = RegSetValueExW(hsubkey, L"StorageFormat",
                             0, REG_DWORD, (LPBYTE) &dw, sizeof(DWORD));
      if (ERROR_SUCCESS != result)
        {
        std::cout << "error result 3: " << result << std::endl;
        std::cout << std::endl;
        }

      RegCloseKey(hsubkey);
      }
    else
      {
      std::cout << "error creating subkey: "
                << nextAvailableSubKeyName << std::endl;
      std::cout << std::endl;
      }
    RegCloseKey(hkey);
    }
  else
    {
    std::cout << "error opening key: " << keyname << std::endl;
    std::cout << std::endl;
    }
}

//----------------------------------------------------------------------------
void RegisterVisualStudioMacros(const std::string& macrosFile,
  const std::string& regKeyBase)
{
  bool macrosRegistered;
  std::string nextAvailableSubKeyName;

  macrosRegistered = IsVisualStudioMacrosFileRegistered(macrosFile,
    regKeyBase, nextAvailableSubKeyName);

  if (!macrosRegistered)
    {
    int count = cmCallVisualStudioMacro::
      GetNumberOfRunningVisualStudioInstances("ALL");

    // Only register the macros file if there are *no* instances of Visual
    // Studio running. If we register it while one is running, first, it has
    // no effect on the running instance; second, and worse, Visual Studio
    // removes our newly added registration entry when it quits. Instead,
    // emit a warning asking the user to exit all running Visual Studio
    // instances...
    //
    if (0 != count)
      {
      std::ostringstream oss;
      oss << "Could not register CMake's Visual Studio macros file '"
        << CMAKE_VSMACROS_FILENAME "' while Visual Studio is running."
        << " Please exit all running instances of Visual Studio before"
        << " continuing." << std::endl
        << std::endl
        << "CMake needs to register Visual Studio macros when its macros"
        << " file is updated or when it detects that its current macros file"
        << " is no longer registered with Visual Studio."
        << std::endl;
      cmSystemTools::Message(oss.str().c_str(), "Warning");

      // Count them again now that the warning is over. In the case of a GUI
      // warning, the user may have gone to close Visual Studio and then come
      // back to the CMake GUI and clicked ok on the above warning. If so,
      // then register the macros *now* if the count is *now* 0...
      //
      count = cmCallVisualStudioMacro::
        GetNumberOfRunningVisualStudioInstances("ALL");

      // Also re-get the nextAvailableSubKeyName in case Visual Studio
      // wrote out new registered macros information as it was exiting:
      //
      if (0 == count)
        {
        IsVisualStudioMacrosFileRegistered(macrosFile, regKeyBase,
          nextAvailableSubKeyName);
        }
      }

    // Do another if check - 'count' may have changed inside the above if:
    //
    if (0 == count)
      {
      WriteVSMacrosFileRegistryEntry(nextAvailableSubKeyName, macrosFile,
        regKeyBase);
      }
    }
}
bool
cmGlobalVisualStudioGenerator::TargetIsFortranOnly(cmTarget const& target)
{
  // check to see if this is a fortran build
  std::set<std::string> languages;
  {
  // Issue diagnostic if the source files depend on the config.
  std::vector<cmSourceFile*> sources;
  if (!target.GetConfigCommonSourceFiles(sources))
    {
    return false;
    }
  }
  target.GetLanguages(languages, "");
  if(languages.size() == 1)
    {
    if(*languages.begin() == "Fortran")
      {
      return true;
      }
    }
  return false;
}

//----------------------------------------------------------------------------
bool
cmGlobalVisualStudioGenerator::TargetCompare
::operator()(cmTarget const* l, cmTarget const* r) const
{
  // Make sure ALL_BUILD is first so it is the default active project.
  if(r->GetName() == "ALL_BUILD")
    {
    return false;
    }
  if(l->GetName() == "ALL_BUILD")
    {
    return true;
    }
  return strcmp(l->GetName().c_str(), r->GetName().c_str()) < 0;
}

//----------------------------------------------------------------------------
cmGlobalVisualStudioGenerator::OrderedTargetDependSet
::OrderedTargetDependSet(TargetDependSet const& targets)
{
  this->insert(targets.begin(), targets.end());
}

//----------------------------------------------------------------------------
cmGlobalVisualStudioGenerator::OrderedTargetDependSet
::OrderedTargetDependSet(TargetSet const& targets)
{
  this->insert(targets.begin(), targets.end());
}

std::string cmGlobalVisualStudioGenerator::ExpandCFGIntDir(
  const std::string& str,
  const std::string& config) const
{
  std::string replace = GetCMakeCFGIntDir();

  std::string tmp = str;
  for(std::string::size_type i = tmp.find(replace);
      i != std::string::npos;
      i = tmp.find(replace, i))
    {
    tmp.replace(i, replace.size(), config);
    i += config.size();
    }
  return tmp;
}
