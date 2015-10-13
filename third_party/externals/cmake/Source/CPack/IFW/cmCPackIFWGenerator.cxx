/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/

#include "cmCPackIFWGenerator.h"

#include "cmCPackIFWPackage.h"
#include "cmCPackIFWInstaller.h"

#include <CPack/cmCPackLog.h>
#include <CPack/cmCPackComponentGroup.h>

#include <cmsys/SystemTools.hxx>
#include <cmsys/Glob.hxx>
#include <cmsys/Directory.hxx>
#include <cmsys/RegularExpression.hxx>

#include <cmGlobalGenerator.h>
#include <cmLocalGenerator.h>
#include <cmSystemTools.h>
#include <cmMakefile.h>
#include <cmGeneratedFileStream.h>
#include <cmXMLSafe.h>
#include <cmVersionConfig.h>
#include <cmTimestamp.h>

//----------------------------------------------------------------------------
cmCPackIFWGenerator::cmCPackIFWGenerator()
{
}

//----------------------------------------------------------------------------
cmCPackIFWGenerator::~cmCPackIFWGenerator()
{
}

//----------------------------------------------------------------------------
bool cmCPackIFWGenerator::IsVersionLess(const char *version)
{
  return cmSystemTools::VersionCompare(cmSystemTools::OP_LESS,
    FrameworkVersion.data(), version);
}

//----------------------------------------------------------------------------
bool cmCPackIFWGenerator::IsVersionGreater(const char *version)
{
  return cmSystemTools::VersionCompare(cmSystemTools::OP_GREATER,
    FrameworkVersion.data(), version);
}

//----------------------------------------------------------------------------
bool cmCPackIFWGenerator::IsVersionEqual(const char *version)
{
  return cmSystemTools::VersionCompare(cmSystemTools::OP_EQUAL,
    FrameworkVersion.data(), version);
}

//----------------------------------------------------------------------------
int cmCPackIFWGenerator::PackageFiles()
{
  cmCPackLogger(cmCPackLog::LOG_OUTPUT, "- Configuration" << std::endl);

  // Installer configuragion
  Installer.GenerateInstallerFile();

  // Packages configuration
  Installer.GeneratePackageFiles();

  std::string ifwTLD = this->GetOption("CPACK_TOPLEVEL_DIRECTORY");
  std::string ifwTmpFile = ifwTLD;
  ifwTmpFile += "/IFWOutput.log";

  // Run repogen
  if (!Installer.Repositories.empty())
    {
    std::string ifwCmd = RepoGen;

    if(IsVersionLess("2.0.0"))
      {
      ifwCmd += " -c " + this->toplevel + "/config/config.xml";
      }

    ifwCmd += " -p " + this->toplevel + "/packages";

    if(!PkgsDirsVector.empty())
      {
      for(std::vector<std::string>::iterator it = PkgsDirsVector.begin();
          it != PkgsDirsVector.end(); ++it)
        {
        ifwCmd += " -p " + *it;
        }
      }

    if (!OnlineOnly && !DownloadedPackages.empty())
      {
      ifwCmd += " -i ";
      std::set<cmCPackIFWPackage*>::iterator it
        = DownloadedPackages.begin();
      ifwCmd += (*it)->Name;
      ++it;
      while(it != DownloadedPackages.end())
        {
        ifwCmd += "," + (*it)->Name;
        ++it;
        }
      }
    ifwCmd += " " + this->toplevel + "/repository";
    cmCPackLogger(cmCPackLog::LOG_VERBOSE, "Execute: " << ifwCmd
                  << std::endl);
    std::string output;
    int retVal = 1;
    cmCPackLogger(cmCPackLog::LOG_OUTPUT,
                  "- Generate repository" << std::endl);
    bool res = cmSystemTools::RunSingleCommand(
      ifwCmd.c_str(), &output, &output,
      &retVal, 0, this->GeneratorVerbose, 0);
    if ( !res || retVal )
      {
      cmGeneratedFileStream ofs(ifwTmpFile.c_str());
      ofs << "# Run command: " << ifwCmd << std::endl
          << "# Output:" << std::endl
          << output << std::endl;
      cmCPackLogger(cmCPackLog::LOG_ERROR, "Problem running IFW command: "
                    << ifwCmd << std::endl
                    << "Please check " << ifwTmpFile << " for errors"
                    << std::endl);
      return 0;
      }
    cmCPackLogger(cmCPackLog::LOG_OUTPUT, "- repository: " << this->toplevel
                  << "/repository generated" << std::endl);
    }

  // Run binary creator
  {
  std::string ifwCmd = BinCreator;
  ifwCmd += " -c " + this->toplevel + "/config/config.xml";
  ifwCmd += " -p " + this->toplevel + "/packages";

  if(!PkgsDirsVector.empty())
    {
    for(std::vector<std::string>::iterator it = PkgsDirsVector.begin();
        it != PkgsDirsVector.end(); ++it)
      {
      ifwCmd += " -p " + *it;
      }
    }

  if (OnlineOnly)
    {
    ifwCmd += " --online-only";
    }
  else if (!DownloadedPackages.empty() && !Installer.Repositories.empty())
    {
    ifwCmd += " -e ";
    std::set<cmCPackIFWPackage*>::iterator it
      = DownloadedPackages.begin();
    ifwCmd += (*it)->Name;
    ++it;
    while(it != DownloadedPackages.end())
      {
      ifwCmd += "," + (*it)->Name;
      ++it;
      }
    }
  else if (!DependentPackages.empty())
    {
    ifwCmd += " -i ";
    // Binary
    std::set<cmCPackIFWPackage*>::iterator bit = BinaryPackages.begin();
    while(bit != BinaryPackages.end())
      {
      ifwCmd += (*bit)->Name + ",";
      ++bit;
      }
    // Depend
    DependenceMap::iterator it = DependentPackages.begin();
    ifwCmd += it->second.Name;
    ++it;
    while(it != DependentPackages.end())
      {
      ifwCmd += "," + it->second.Name;
      ++it;
      }
    }
  // TODO: set correct name for multipackages
  if (this->packageFileNames.size() > 0)
    {
    ifwCmd += " " + packageFileNames[0];
    }
  else
    {
    ifwCmd += " installer";
    }
  cmCPackLogger(cmCPackLog::LOG_VERBOSE, "Execute: " << ifwCmd
                << std::endl);
  std::string output;
  int retVal = 1;
  cmCPackLogger(cmCPackLog::LOG_OUTPUT, "- Generate package" << std::endl);
  bool res = cmSystemTools::RunSingleCommand(
    ifwCmd.c_str(), &output, &output,
    &retVal, 0, this->GeneratorVerbose, 0);
  if ( !res || retVal )
    {
    cmGeneratedFileStream ofs(ifwTmpFile.c_str());
    ofs << "# Run command: " << ifwCmd << std::endl
        << "# Output:" << std::endl
        << output << std::endl;
    cmCPackLogger(cmCPackLog::LOG_ERROR, "Problem running IFW command: "
                  << ifwCmd << std::endl
                  << "Please check " << ifwTmpFile << " for errors"
                  << std::endl);
    return 0;
    }
  }

  return 1;
}

//----------------------------------------------------------------------------
const char *cmCPackIFWGenerator::GetPackagingInstallPrefix()
{
  const char *defPrefix = cmCPackGenerator::GetPackagingInstallPrefix();

  std::string tmpPref = defPrefix ? defPrefix : "";

  if(this->Components.empty())
    {
    tmpPref += "packages/" + GetRootPackageName() + "/data";
    }

  this->SetOption("CPACK_IFW_PACKAGING_INSTALL_PREFIX", tmpPref.c_str());

  return this->GetOption("CPACK_IFW_PACKAGING_INSTALL_PREFIX");
}

//----------------------------------------------------------------------------
const char *cmCPackIFWGenerator::GetOutputExtension()
{
  return ExecutableSuffix.c_str();
}

//----------------------------------------------------------------------------
int cmCPackIFWGenerator::InitializeInternal()
{
  // Search Qt Installer Framework tools

  const std::string BinCreatorOpt = "CPACK_IFW_BINARYCREATOR_EXECUTABLE";
  const std::string RepoGenOpt = "CPACK_IFW_REPOGEN_EXECUTABLE";
  const std::string FrameworkVersionOpt = "CPACK_IFW_FRAMEWORK_VERSION";

  if(!this->IsSet(BinCreatorOpt) ||
     !this->IsSet(RepoGenOpt) ||
     !this->IsSet(FrameworkVersionOpt))
    {
    this->ReadListFile("CPackIFW.cmake");
    }

  // Look 'binarycreator' executable (needs)

  const char *BinCreatorStr = this->GetOption(BinCreatorOpt);
  if(!BinCreatorStr || cmSystemTools::IsNOTFOUND(BinCreatorStr))
    {
    BinCreator = "";
    }
  else
    {
    BinCreator = BinCreatorStr;
    }

  if (BinCreator.empty())
    {
    cmCPackLogger(cmCPackLog::LOG_ERROR,
                  "Cannot find QtIFW compiler \"binarycreator\": "
                  "likely it is not installed, or not in your PATH"
                  << std::endl);
    return 0;
    }

  // Look 'repogen' executable (optional)

  const char *RepoGenStr = this->GetOption(RepoGenOpt);
  if(!RepoGenStr || cmSystemTools::IsNOTFOUND(RepoGenStr))
    {
    RepoGen = "";
    }
  else
    {
    RepoGen = RepoGenStr;
    }

  // Framework version
  if(const char* FrameworkVersionSrt =
      this->GetOption(FrameworkVersionOpt))
    {
    FrameworkVersion = FrameworkVersionSrt;
    }
  else
    {
    FrameworkVersion = "1.9.9";
    }

  // Variables that Change Behavior

  // Resolve duplicate names
  ResolveDuplicateNames = this->IsOn("CPACK_IFW_RESOLVE_DUPLICATE_NAMES");

  // Additional packages dirs
  PkgsDirsVector.clear();
  if(const char* dirs = this->GetOption("CPACK_IFW_PACKAGES_DIRECTORIES"))
    {
    cmSystemTools::ExpandListArgument(dirs,
                                      PkgsDirsVector);
    }

  // Installer
  Installer.Generator = this;
  Installer.ConfigureFromOptions();

  if (const char* ifwDownloadAll =
      this->GetOption("CPACK_IFW_DOWNLOAD_ALL"))
    {
    OnlineOnly = cmSystemTools::IsOn(ifwDownloadAll);
    }
  else if (const char* cpackDownloadAll =
           this->GetOption("CPACK_DOWNLOAD_ALL"))
    {
    OnlineOnly = cmSystemTools::IsOn(cpackDownloadAll);
    }
  else
    {
    OnlineOnly = false;
    }

  if (!Installer.Repositories.empty() && RepoGen.empty()) {
  cmCPackLogger(cmCPackLog::LOG_ERROR,
                "Cannot find QtIFW repository generator \"repogen\": "
                "likely it is not installed, or not in your PATH"
                << std::endl);
  return 0;
  }

  // Executable suffix
  if(const char *optExeSuffix = this->GetOption("CMAKE_EXECUTABLE_SUFFIX"))
    {
    ExecutableSuffix = optExeSuffix;
    if(ExecutableSuffix.empty())
      {
      std::string sysName(this->GetOption("CMAKE_SYSTEM_NAME"));
      if(sysName == "Linux")
        {
        ExecutableSuffix = ".run";
        }
      }
    }
  else
    {
    ExecutableSuffix = cmCPackGenerator::GetOutputExtension();
    }

  return this->Superclass::InitializeInternal();
}

//----------------------------------------------------------------------------
std::string
cmCPackIFWGenerator::GetComponentInstallDirNameSuffix(
  const std::string& componentName)
{
  const std::string prefix = "packages/";
  const std::string suffix = "/data";

  if (componentPackageMethod == ONE_PACKAGE) {
  return std::string(prefix + GetRootPackageName() + suffix);
  }

  return prefix
    + GetComponentPackageName(&Components[componentName])
    + suffix;
}

//----------------------------------------------------------------------------
cmCPackComponent*
cmCPackIFWGenerator::GetComponent(const std::string &projectName,
                                  const std::string &componentName)
{
  ComponentsMap::iterator cit = Components.find(componentName);
  if ( cit != Components.end() ) return &(cit->second);

  cmCPackComponent* component
    = cmCPackGenerator::GetComponent(projectName, componentName);
  if(!component) return component;

  std::string name = GetComponentPackageName(component);
  PackagesMap::iterator pit = Packages.find(name);
  if(pit != Packages.end()) return component;

  cmCPackIFWPackage *package = &Packages[name];
  package->Name = name;
  package->Generator = this;
  if(package->ConfigureFromComponent(component))
    {
    package->Installer = &Installer;
    Installer.Packages.insert(
      std::pair<std::string, cmCPackIFWPackage*>(
        name, package));
    ComponentPackages.insert(
      std::pair<cmCPackComponent*, cmCPackIFWPackage*>(
        component, package));
    if(component->IsDownloaded)
      {
      DownloadedPackages.insert(package);
      }
    else
      {
      BinaryPackages.insert(package);
      }
    }
  else
    {
    Packages.erase(name);
    cmCPackLogger(cmCPackLog::LOG_ERROR,
                  "Cannot configure package \"" << name <<
                  "\" for component \"" << component->Name << "\""
                  << std::endl);
    }

  return component;
}

//----------------------------------------------------------------------------
cmCPackComponentGroup*
cmCPackIFWGenerator::GetComponentGroup(const std::string &projectName,
                                       const std::string &groupName)
{
  cmCPackComponentGroup* group
    = cmCPackGenerator::GetComponentGroup(projectName, groupName);
  if(!group) return group;

  std::string name = GetGroupPackageName(group);
  PackagesMap::iterator pit = Packages.find(name);
  if(pit != Packages.end()) return group;

  cmCPackIFWPackage *package = &Packages[name];
  package->Name = name;
  package->Generator = this;
  if(package->ConfigureFromGroup(group))
    {
    package->Installer = &Installer;
    Installer.Packages.insert(
      std::pair<std::string, cmCPackIFWPackage*>(
        name, package));
    GroupPackages.insert(
      std::pair<cmCPackComponentGroup*, cmCPackIFWPackage*>(
        group, package));
    BinaryPackages.insert(package);
    }
  else
    {
    Packages.erase(name);
    cmCPackLogger(cmCPackLog::LOG_ERROR,
                  "Cannot configure package \"" << name <<
                  "\" for component group \"" << group->Name << "\""
                  << std::endl);
    }
  return group;
}

//----------------------------------------------------------------------------
enum cmCPackGenerator::CPackSetDestdirSupport
cmCPackIFWGenerator::SupportsSetDestdir() const
{
  return cmCPackGenerator::SETDESTDIR_SHOULD_NOT_BE_USED;
}

//----------------------------------------------------------------------------
bool cmCPackIFWGenerator::SupportsAbsoluteDestination() const
{
  return false;
}

//----------------------------------------------------------------------------
bool cmCPackIFWGenerator::SupportsComponentInstallation() const
{
  return true;
}

//----------------------------------------------------------------------------
bool cmCPackIFWGenerator::IsOnePackage() const
{
  return componentPackageMethod == ONE_PACKAGE;
}

//----------------------------------------------------------------------------
std::string cmCPackIFWGenerator::GetRootPackageName()
{
  // Default value
  std::string name = "root";
  if (const char* optIFW_PACKAGE_GROUP =
      this->GetOption("CPACK_IFW_PACKAGE_GROUP"))
    {
    // Configure from root group
    cmCPackIFWPackage package;
    package.Generator = this;
    package.ConfigureFromGroup(optIFW_PACKAGE_GROUP);
    name = package.Name;
    }
  else if (const char* optIFW_PACKAGE_NAME =
           this->GetOption("CPACK_IFW_PACKAGE_NAME"))
    {
    // Configure from root package name
    name = optIFW_PACKAGE_NAME;
    }
  else if (const char* optPACKAGE_NAME =
           this->GetOption("CPACK_PACKAGE_NAME"))
    {
    // Configure from package name
    name = optPACKAGE_NAME;
    }
  return name;
}

//----------------------------------------------------------------------------
std::string
cmCPackIFWGenerator::GetGroupPackageName(cmCPackComponentGroup *group) const
{
  std::string name;
  if (!group) return name;
  if (cmCPackIFWPackage* package = GetGroupPackage(group))
    {
    return package->Name;
    }
  const char* option = GetOption(
    "CPACK_IFW_COMPONENT_GROUP_"
    + cmsys::SystemTools::UpperCase(group->Name)
    + "_NAME");
  name = option ? option : group->Name;
  if(group->ParentGroup)
    {
    cmCPackIFWPackage* package = GetGroupPackage(group->ParentGroup);
    bool dot = !ResolveDuplicateNames;
    if(dot && name.substr(0, package->Name.size()) == package->Name)
      {
      dot = false;
      }
    if(dot)
      {
      name = package->Name + "." + name;
      }
    }
  return name;
}

//----------------------------------------------------------------------------
std::string cmCPackIFWGenerator::GetComponentPackageName(
  cmCPackComponent *component) const
{
  std::string name;
  if (!component) return name;
  if (cmCPackIFWPackage* package = GetComponentPackage(component))
    {
    return package->Name;
    }
  std::string prefix = "CPACK_IFW_COMPONENT_"
    + cmsys::SystemTools::UpperCase(component->Name)
    + "_";
  const char* option = GetOption(prefix + "NAME");
  name = option ? option : component->Name;
  if(component->Group)
    {
    cmCPackIFWPackage* package = GetGroupPackage(component->Group);
    if((componentPackageMethod == ONE_PACKAGE_PER_GROUP)
       || IsOn(prefix + "COMMON"))
      {
      return package->Name;
      }
    bool dot = !ResolveDuplicateNames;
    if(dot && name.substr(0, package->Name.size()) == package->Name)
      {
      dot = false;
      }
    if(dot)
      {
      name = package->Name + "." + name;
      }
    }
  return name;
}

//----------------------------------------------------------------------------
cmCPackIFWPackage* cmCPackIFWGenerator::GetGroupPackage(
  cmCPackComponentGroup *group) const
{
  std::map<cmCPackComponentGroup*, cmCPackIFWPackage*>::const_iterator pit
    = GroupPackages.find(group);
  return pit != GroupPackages.end() ? pit->second : 0;
}

//----------------------------------------------------------------------------
cmCPackIFWPackage* cmCPackIFWGenerator::GetComponentPackage(
  cmCPackComponent *component) const
{
  std::map<cmCPackComponent*, cmCPackIFWPackage*>::const_iterator pit
    = ComponentPackages.find(component);
  return pit != ComponentPackages.end() ? pit->second : 0;
}

//----------------------------------------------------------------------------
void cmCPackIFWGenerator::WriteGeneratedByToStrim(cmGeneratedFileStream &xout)
{
  xout << "<!-- Generated by CPack " << CMake_VERSION << " IFW generator "
       << "for QtIFW ";
  if(IsVersionLess("2.0"))
    {
    xout << "less 2.0";
    }
  else
    {
    xout << FrameworkVersion;
    }
  xout << " tools at " << cmTimestamp().CurrentTime("", true) << " -->"
       << std::endl;
}
