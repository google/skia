/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2012-2015 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/

#include "cmCPackWIXGenerator.h"

#include <cmSystemTools.h>
#include <cmGeneratedFileStream.h>
#include <cmCryptoHash.h>
#include <cmInstalledFile.h>
#include <CPack/cmCPackLog.h>
#include <CPack/cmCPackComponentGroup.h>

#include "cmWIXSourceWriter.h"
#include "cmWIXDirectoriesSourceWriter.h"
#include "cmWIXFeaturesSourceWriter.h"
#include "cmWIXFilesSourceWriter.h"
#include "cmWIXRichTextFormatWriter.h"

#include <cmsys/SystemTools.hxx>
#include <cmsys/Directory.hxx>
#include <cmsys/Encoding.hxx>
#include <cmsys/FStream.hxx>

#include <rpc.h> // for GUID generation

cmCPackWIXGenerator::cmCPackWIXGenerator():
  Patch(0)
{

}

cmCPackWIXGenerator::~cmCPackWIXGenerator()
{
  if(this->Patch)
    {
    delete this->Patch;
    }
}

int cmCPackWIXGenerator::InitializeInternal()
{
  componentPackageMethod = ONE_PACKAGE;
  this->Patch = new cmWIXPatch(this->Logger);

  return this->Superclass::InitializeInternal();
}

bool cmCPackWIXGenerator::RunWiXCommand(std::string const& command)
{
  std::string logFileName = this->CPackTopLevel + "/wix.log";

  cmCPackLogger(cmCPackLog::LOG_DEBUG,
    "Running WiX command: " << command << std::endl);

  std::string output;

  int returnValue = 0;
  bool status = cmSystemTools::RunSingleCommand(
    command.c_str(), &output, &output,
    &returnValue, 0, cmSystemTools::OUTPUT_NONE);

  cmsys::ofstream logFile(logFileName.c_str(), std::ios::app);
  logFile << command << std::endl;
  logFile << output;
  logFile.close();

  if(!status || returnValue)
    {
    cmCPackLogger(cmCPackLog::LOG_ERROR,
      "Problem running WiX candle. "
      "Please check '" << logFileName << "' for errors." << std::endl);

    return false;
    }

  return true;
}

bool cmCPackWIXGenerator::RunCandleCommand(
  std::string const& sourceFile, std::string const& objectFile)
{
  std::string executable;
  if(!RequireOption("CPACK_WIX_CANDLE_EXECUTABLE", executable))
    {
    return false;
    }

  std::stringstream command;
  command << QuotePath(executable);
  command << " -nologo";
  command << " -arch " << GetArchitecture();
  command << " -out " << QuotePath(objectFile);

  for(extension_set_t::const_iterator i = CandleExtensions.begin();
      i != CandleExtensions.end(); ++i)
    {
    command << " -ext " << QuotePath(*i);
    }

  AddCustomFlags("CPACK_WIX_CANDLE_EXTRA_FLAGS", command);

  command << " " << QuotePath(sourceFile);

  return RunWiXCommand(command.str());
}

bool cmCPackWIXGenerator::RunLightCommand(std::string const& objectFiles)
{
  std::string executable;
  if(!RequireOption("CPACK_WIX_LIGHT_EXECUTABLE", executable))
    {
    return false;
    }

  std::stringstream command;
  command << QuotePath(executable);
  command << " -nologo";
  command << " -out " << QuotePath(packageFileNames.at(0));

  for(extension_set_t::const_iterator i = this->LightExtensions.begin();
      i != this->LightExtensions.end(); ++i)
    {
    command << " -ext " << QuotePath(*i);
    }

  const char* const cultures = GetOption("CPACK_WIX_CULTURES");
  if(cultures)
    {
    command << " -cultures:" << cultures;
    }

  AddCustomFlags("CPACK_WIX_LIGHT_EXTRA_FLAGS", command);

  command << " " << objectFiles;

  return RunWiXCommand(command.str());
}

int cmCPackWIXGenerator::PackageFiles()
{
  if(!PackageFilesImpl() || cmSystemTools::GetErrorOccuredFlag())
    {
    cmCPackLogger(cmCPackLog::LOG_ERROR,
      "Fatal WiX Generator Error" << std::endl);
    return false;
    }

  return true;
}

bool cmCPackWIXGenerator::InitializeWiXConfiguration()
{
  if(!ReadListFile("CPackWIX.cmake"))
    {
    cmCPackLogger(cmCPackLog::LOG_ERROR,
      "Error while executing CPackWIX.cmake" << std::endl);
    return false;
    }

  if(GetOption("CPACK_WIX_PRODUCT_GUID") == 0)
    {
    std::string guid = GenerateGUID();
    SetOption("CPACK_WIX_PRODUCT_GUID", guid.c_str());

    cmCPackLogger(cmCPackLog::LOG_VERBOSE,
      "CPACK_WIX_PRODUCT_GUID implicitly set to " << guid << " . "
      << std::endl);
    }

  if(GetOption("CPACK_WIX_UPGRADE_GUID") == 0)
    {
    std::string guid = GenerateGUID();
    SetOption("CPACK_WIX_UPGRADE_GUID", guid.c_str());

    cmCPackLogger(cmCPackLog::LOG_WARNING,
      "CPACK_WIX_UPGRADE_GUID implicitly set to " << guid << " . "
      "Please refer to the documentation on how and why "
      "you might want to set this explicitly." << std::endl);
    }

  if(!RequireOption("CPACK_TOPLEVEL_DIRECTORY", this->CPackTopLevel))
    {
    return false;
    }

  if(GetOption("CPACK_WIX_LICENSE_RTF") == 0)
    {
    std::string licenseFilename = this->CPackTopLevel + "/License.rtf";
    SetOption("CPACK_WIX_LICENSE_RTF", licenseFilename.c_str());

    if(!CreateLicenseFile())
      {
      return false;
      }
    }

  if(GetOption("CPACK_PACKAGE_VENDOR") == 0)
    {
    std::string defaultVendor = "Humanity";
    SetOption("CPACK_PACKAGE_VENDOR", defaultVendor.c_str());

    cmCPackLogger(cmCPackLog::LOG_VERBOSE,
      "CPACK_PACKAGE_VENDOR implicitly set to " << defaultVendor << " . "
      << std::endl);
    }

  if(GetOption("CPACK_WIX_UI_REF") == 0)
    {
    std::string defaultRef = "WixUI_InstallDir";

    if(this->Components.size())
      {
      defaultRef = "WixUI_FeatureTree";
      }

    SetOption("CPACK_WIX_UI_REF", defaultRef.c_str());
    }

  const char* packageContact = GetOption("CPACK_PACKAGE_CONTACT");
  if(packageContact != 0 &&
     GetOption("CPACK_WIX_PROPERTY_ARPCONTACT") == 0)
    {
    SetOption("CPACK_WIX_PROPERTY_ARPCONTACT", packageContact);
    }

  CollectExtensions("CPACK_WIX_EXTENSIONS", this->CandleExtensions);
  CollectExtensions("CPACK_WIX_CANDLE_EXTENSIONS", this->CandleExtensions);

  this->LightExtensions.insert("WixUIExtension");
  CollectExtensions("CPACK_WIX_EXTENSIONS", this->LightExtensions);
  CollectExtensions("CPACK_WIX_LIGHT_EXTENSIONS", this->LightExtensions);

  const char* patchFilePath = GetOption("CPACK_WIX_PATCH_FILE");
  if(patchFilePath)
    {
    this->Patch->LoadFragments(patchFilePath);
    }

  return true;
}

bool cmCPackWIXGenerator::PackageFilesImpl()
{
  if(!InitializeWiXConfiguration())
    {
    return false;
    }

  CreateWiXVariablesIncludeFile();
  CreateWiXPropertiesIncludeFile();
  CreateWiXProductFragmentIncludeFile();

  if(!CreateWiXSourceFiles())
    {
    return false;
    }

  AppendUserSuppliedExtraSources();

  std::set<std::string> usedBaseNames;

  std::stringstream objectFiles;
  for(size_t i = 0; i < this->WixSources.size(); ++i)
    {
    std::string const& sourceFilename = this->WixSources[i];

    std::string baseName =
      cmSystemTools::GetFilenameWithoutLastExtension(sourceFilename);

    unsigned int counter = 0;
    std::string uniqueBaseName = baseName;

    while(usedBaseNames.find(uniqueBaseName) != usedBaseNames.end())
      {
      std::stringstream tmp;
      tmp << baseName << ++counter;
      uniqueBaseName = tmp.str();
      }

    usedBaseNames.insert(uniqueBaseName);

    std::string objectFilename =
      this->CPackTopLevel + "/" + uniqueBaseName + ".wixobj";

    if(!RunCandleCommand(sourceFilename, objectFilename))
      {
      return false;
      }

    objectFiles << " " << QuotePath(objectFilename);
    }

  AppendUserSuppliedExtraObjects(objectFiles);

  return RunLightCommand(objectFiles.str());
}

void cmCPackWIXGenerator::AppendUserSuppliedExtraSources()
{
  const char *cpackWixExtraSources = GetOption("CPACK_WIX_EXTRA_SOURCES");
  if(!cpackWixExtraSources) return;

  cmSystemTools::ExpandListArgument(cpackWixExtraSources, this->WixSources);
}

void cmCPackWIXGenerator::AppendUserSuppliedExtraObjects(std::ostream& stream)
{
  const char *cpackWixExtraObjects = GetOption("CPACK_WIX_EXTRA_OBJECTS");
  if(!cpackWixExtraObjects) return;

  std::vector<std::string> expandedExtraObjects;

  cmSystemTools::ExpandListArgument(
    cpackWixExtraObjects, expandedExtraObjects);

  for(size_t i = 0; i < expandedExtraObjects.size(); ++i)
    {
    stream << " " << QuotePath(expandedExtraObjects[i]);
    }
}

void cmCPackWIXGenerator::CreateWiXVariablesIncludeFile()
{
  std::string includeFilename =
    this->CPackTopLevel + "/cpack_variables.wxi";

  cmWIXSourceWriter includeFile(
    this->Logger, includeFilename, true);

  CopyDefinition(includeFile, "CPACK_WIX_PRODUCT_GUID");
  CopyDefinition(includeFile, "CPACK_WIX_UPGRADE_GUID");
  CopyDefinition(includeFile, "CPACK_PACKAGE_VENDOR");
  CopyDefinition(includeFile, "CPACK_PACKAGE_NAME");
  CopyDefinition(includeFile, "CPACK_PACKAGE_VERSION");
  CopyDefinition(includeFile, "CPACK_WIX_LICENSE_RTF");
  CopyDefinition(includeFile, "CPACK_WIX_PRODUCT_ICON");
  CopyDefinition(includeFile, "CPACK_WIX_UI_BANNER");
  CopyDefinition(includeFile, "CPACK_WIX_UI_DIALOG");
  SetOptionIfNotSet("CPACK_WIX_PROGRAM_MENU_FOLDER",
    GetOption("CPACK_PACKAGE_NAME"));
  CopyDefinition(includeFile, "CPACK_WIX_PROGRAM_MENU_FOLDER");
  CopyDefinition(includeFile, "CPACK_WIX_UI_REF");
}

void cmCPackWIXGenerator::CreateWiXPropertiesIncludeFile()
{
  std::string includeFilename =
    this->CPackTopLevel + "/properties.wxi";

  cmWIXSourceWriter includeFile(
    this->Logger, includeFilename, true);

  std::string prefix = "CPACK_WIX_PROPERTY_";
  std::vector<std::string> options = GetOptions();

  for(size_t i = 0; i < options.size(); ++i)
    {
    std::string const& name = options[i];

    if(name.length() > prefix.length() &&
       name.substr(0, prefix.length()) == prefix)
      {
      std::string id = name.substr(prefix.length());
      std::string value = GetOption(name.c_str());

      includeFile.BeginElement("Property");
      includeFile.AddAttribute("Id", id);
      includeFile.AddAttribute("Value", value);
      includeFile.EndElement("Property");
      }
    }

  if(GetOption("CPACK_WIX_PROPERTY_ARPINSTALLLOCATION") == 0)
    {
    includeFile.BeginElement("Property");
    includeFile.AddAttribute("Id", "INSTALL_ROOT");
    includeFile.AddAttribute("Secure", "yes");

    includeFile.BeginElement("RegistrySearch");
    includeFile.AddAttribute("Id", "FindInstallLocation");
    includeFile.AddAttribute("Root", "HKLM");
    includeFile.AddAttribute("Key", "Software\\Microsoft\\Windows\\"
      "CurrentVersion\\Uninstall\\[WIX_UPGRADE_DETECTED]");
    includeFile.AddAttribute("Name", "InstallLocation");
    includeFile.AddAttribute("Type", "raw");
    includeFile.EndElement("RegistrySearch");
    includeFile.EndElement("Property");

    includeFile.BeginElement("SetProperty");
    includeFile.AddAttribute("Id", "ARPINSTALLLOCATION");
    includeFile.AddAttribute("Value", "[INSTALL_ROOT]");
    includeFile.AddAttribute("After", "CostFinalize");
    includeFile.EndElement("SetProperty");
    }
}

void cmCPackWIXGenerator::CreateWiXProductFragmentIncludeFile()
{
    std::string includeFilename =
      this->CPackTopLevel + "/product_fragment.wxi";

    cmWIXSourceWriter includeFile(
      this->Logger, includeFilename, true);

    this->Patch->ApplyFragment("#PRODUCT", includeFile);
}

void cmCPackWIXGenerator::CopyDefinition(
  cmWIXSourceWriter &source, std::string const& name)
{
  const char* value = GetOption(name.c_str());
  if(value)
    {
    AddDefinition(source, name, value);
    }
}

void cmCPackWIXGenerator::AddDefinition(cmWIXSourceWriter& source,
  std::string const& name, std::string const& value)
{
  std::stringstream tmp;
  tmp << name << "=\"" << value << '"';

  source.AddProcessingInstruction("define",
    cmWIXSourceWriter::CMakeEncodingToUtf8(tmp.str()));
}

bool cmCPackWIXGenerator::CreateWiXSourceFiles()
{
  std::string directoryDefinitionsFilename =
    this->CPackTopLevel + "/directories.wxs";

  this->WixSources.push_back(directoryDefinitionsFilename);

  cmWIXDirectoriesSourceWriter directoryDefinitions(
    this->Logger, directoryDefinitionsFilename);
  directoryDefinitions.BeginElement("Fragment");

  std::string installRoot;
  if(!RequireOption("CPACK_PACKAGE_INSTALL_DIRECTORY", installRoot))
    {
    return false;
    }

  directoryDefinitions.BeginElement("Directory");
  directoryDefinitions.AddAttribute("Id", "TARGETDIR");
  directoryDefinitions.AddAttribute("Name", "SourceDir");

  size_t installRootSize =
    directoryDefinitions.BeginInstallationPrefixDirectory(
      GetProgramFilesFolderId(), installRoot);

  std::string fileDefinitionsFilename =
    this->CPackTopLevel + "/files.wxs";

  this->WixSources.push_back(fileDefinitionsFilename);

  cmWIXFilesSourceWriter fileDefinitions(
    this->Logger, fileDefinitionsFilename);

  fileDefinitions.BeginElement("Fragment");

  std::string featureDefinitionsFilename =
      this->CPackTopLevel +"/features.wxs";

  this->WixSources.push_back(featureDefinitionsFilename);

  cmWIXFeaturesSourceWriter featureDefinitions(
    this->Logger, featureDefinitionsFilename);

  featureDefinitions.BeginElement("Fragment");

  featureDefinitions.BeginElement("Feature");
  featureDefinitions.AddAttribute("Id", "ProductFeature");
  featureDefinitions.AddAttribute("Display", "expand");
  featureDefinitions.AddAttribute("ConfigurableDirectory", "INSTALL_ROOT");

  std::string cpackPackageName;
  if(!RequireOption("CPACK_PACKAGE_NAME", cpackPackageName))
    {
    return false;
    }

  featureDefinitions.AddAttribute("Title", cpackPackageName);
  featureDefinitions.AddAttribute("Level", "1");
  this->Patch->ApplyFragment("#PRODUCTFEATURE", featureDefinitions);

  const char* package = GetOption("CPACK_WIX_CMAKE_PACKAGE_REGISTRY");
  if(package)
    {
    featureDefinitions.CreateCMakePackageRegistryEntry(
        package, GetOption("CPACK_WIX_UPGRADE_GUID"));
    }

  if(!CreateFeatureHierarchy(featureDefinitions))
    {
    return false;
    }

  featureDefinitions.EndElement("Feature");

  std::set<cmWIXShortcuts::Type> emittedShortcutTypes;

  cmWIXShortcuts globalShortcuts;
  if(Components.empty())
    {
    AddComponentsToFeature(toplevel, "ProductFeature",
      directoryDefinitions, fileDefinitions, featureDefinitions,
      globalShortcuts);

    globalShortcuts.AddShortcutTypes(emittedShortcutTypes);
    }
  else
    {
    for(std::map<std::string, cmCPackComponent>::const_iterator
      i = this->Components.begin(); i != this->Components.end(); ++i)
      {
      cmCPackComponent const& component = i->second;

      std::string componentPath = toplevel;
      componentPath += "/";
      componentPath += component.Name;

      std::string componentFeatureId = "CM_C_" + component.Name;

      cmWIXShortcuts featureShortcuts;
      AddComponentsToFeature(componentPath, componentFeatureId,
        directoryDefinitions, fileDefinitions,
        featureDefinitions, featureShortcuts);

      featureShortcuts.AddShortcutTypes(emittedShortcutTypes);

      if(!CreateShortcuts(component.Name, componentFeatureId,
        featureShortcuts, false, fileDefinitions, featureDefinitions))
        {
        return false;
        }
      }
    }

  bool emitUninstallShortcut = emittedShortcutTypes.find(
    cmWIXShortcuts::START_MENU) != emittedShortcutTypes.end();

  if(!CreateShortcuts(std::string(), "ProductFeature",
      globalShortcuts, emitUninstallShortcut,
      fileDefinitions, featureDefinitions))
    {
    return false;
    }

  featureDefinitions.EndElement("Fragment");
  fileDefinitions.EndElement("Fragment");

  directoryDefinitions.EndInstallationPrefixDirectory(
    installRootSize);

  if(emittedShortcutTypes.find(cmWIXShortcuts::START_MENU) !=
      emittedShortcutTypes.end())
    {
    directoryDefinitions.EmitStartMenuFolder(
      GetOption("CPACK_WIX_PROGRAM_MENU_FOLDER"));
    }

  if(emittedShortcutTypes.find(cmWIXShortcuts::DESKTOP) !=
      emittedShortcutTypes.end())
    {
    directoryDefinitions.EmitDesktopFolder();
    }

  if(emittedShortcutTypes.find(cmWIXShortcuts::STARTUP) !=
      emittedShortcutTypes.end())
    {
    directoryDefinitions.EmitStartupFolder();
    }

  directoryDefinitions.EndElement("Directory");
  directoryDefinitions.EndElement("Fragment");

  if(!GenerateMainSourceFileFromTemplate())
    {
    return false;
    }

  return this->Patch->CheckForUnappliedFragments();
}

std::string cmCPackWIXGenerator::GetProgramFilesFolderId() const
{
  if(GetArchitecture() == "x86")
    {
    return "ProgramFilesFolder";
    }
  else
    {
    return "ProgramFiles64Folder";
    }
}

bool cmCPackWIXGenerator::GenerateMainSourceFileFromTemplate()
{
  std::string wixTemplate = FindTemplate("WIX.template.in");
  if(GetOption("CPACK_WIX_TEMPLATE") != 0)
    {
    wixTemplate = GetOption("CPACK_WIX_TEMPLATE");
    }

  if(wixTemplate.empty())
    {
    cmCPackLogger(cmCPackLog::LOG_ERROR,
      "Could not find CPack WiX template file WIX.template.in" << std::endl);
    return false;
    }

  std::string mainSourceFilePath = this->CPackTopLevel + "/main.wxs";

  if(!ConfigureFile(wixTemplate.c_str(), mainSourceFilePath .c_str()))
    {
    cmCPackLogger(cmCPackLog::LOG_ERROR,
      "Failed creating '" << mainSourceFilePath  <<
      "'' from template." << std::endl);

    return false;
    }

  this->WixSources.push_back(mainSourceFilePath);

  return true;
}

bool cmCPackWIXGenerator::CreateFeatureHierarchy(
  cmWIXFeaturesSourceWriter& featureDefinitions)
{
  for(std::map<std::string, cmCPackComponentGroup>::const_iterator
    i = ComponentGroups.begin(); i != ComponentGroups.end(); ++i)
    {
    cmCPackComponentGroup const& group = i->second;
    if(group.ParentGroup == 0)
      {
      featureDefinitions.EmitFeatureForComponentGroup(group);
      }
    }

  for(std::map<std::string, cmCPackComponent>::const_iterator
    i = this->Components.begin(); i != this->Components.end(); ++i)
    {
    cmCPackComponent const& component = i->second;

    if(!component.Group)
      {
      featureDefinitions.EmitFeatureForComponent(component);
      }
    }

  return true;
}

bool cmCPackWIXGenerator::AddComponentsToFeature(
  std::string const& rootPath,
  std::string const& featureId,
  cmWIXDirectoriesSourceWriter& directoryDefinitions,
  cmWIXFilesSourceWriter& fileDefinitions,
  cmWIXFeaturesSourceWriter& featureDefinitions,
  cmWIXShortcuts& shortcuts)
{
  featureDefinitions.BeginElement("FeatureRef");
  featureDefinitions.AddAttribute("Id", featureId);

  std::vector<std::string> cpackPackageExecutablesList;
  const char *cpackPackageExecutables = GetOption("CPACK_PACKAGE_EXECUTABLES");
  if(cpackPackageExecutables)
    {
      cmSystemTools::ExpandListArgument(cpackPackageExecutables,
        cpackPackageExecutablesList);
      if(cpackPackageExecutablesList.size() % 2 != 0 )
        {
        cmCPackLogger(cmCPackLog::LOG_ERROR,
          "CPACK_PACKAGE_EXECUTABLES should contain pairs of <executable> and "
          "<text label>." << std::endl);
        return false;
        }
    }

  std::vector<std::string> cpackPackageDesktopLinksList;
  const char *cpackPackageDesktopLinks =
    GetOption("CPACK_CREATE_DESKTOP_LINKS");
  if(cpackPackageDesktopLinks)
    {
      cmSystemTools::ExpandListArgument(cpackPackageDesktopLinks,
        cpackPackageDesktopLinksList);
    }

  AddDirectoryAndFileDefinitons(
    rootPath, "INSTALL_ROOT",
    directoryDefinitions, fileDefinitions, featureDefinitions,
    cpackPackageExecutablesList, cpackPackageDesktopLinksList,
    shortcuts);

  featureDefinitions.EndElement("FeatureRef");

  return true;
}

bool cmCPackWIXGenerator::CreateShortcuts(
  std::string const& cpackComponentName,
  std::string const& featureId,
  cmWIXShortcuts const& shortcuts,
  bool emitUninstallShortcut,
  cmWIXFilesSourceWriter& fileDefinitions,
  cmWIXFeaturesSourceWriter& featureDefinitions)
{
  if(!shortcuts.empty(cmWIXShortcuts::START_MENU))
    {
    if(!this->CreateShortcutsOfSpecificType(cmWIXShortcuts::START_MENU,
      cpackComponentName, featureId, "",
      shortcuts, emitUninstallShortcut,
      fileDefinitions, featureDefinitions))
      {
      return false;
      }
    }

  if(!shortcuts.empty(cmWIXShortcuts::DESKTOP))
    {
    if(!this->CreateShortcutsOfSpecificType(cmWIXShortcuts::DESKTOP,
      cpackComponentName, featureId, "DESKTOP",
      shortcuts, false,
      fileDefinitions, featureDefinitions))
      {
      return false;
      }
    }

  if(!shortcuts.empty(cmWIXShortcuts::STARTUP))
    {
    if(!this->CreateShortcutsOfSpecificType(cmWIXShortcuts::STARTUP,
      cpackComponentName, featureId, "STARTUP",
      shortcuts, false,
      fileDefinitions, featureDefinitions))
      {
      return false;
      }
    }

  return true;
}

bool cmCPackWIXGenerator::CreateShortcutsOfSpecificType(
  cmWIXShortcuts::Type type,
  std::string const& cpackComponentName,
  std::string const& featureId,
  std::string const& idPrefix,
  cmWIXShortcuts const& shortcuts,
  bool emitUninstallShortcut,
  cmWIXFilesSourceWriter& fileDefinitions,
  cmWIXFeaturesSourceWriter& featureDefinitions)
{
  std::string directoryId;
  switch(type)
    {
    case cmWIXShortcuts::START_MENU:
      directoryId = "PROGRAM_MENU_FOLDER";
      break;
    case cmWIXShortcuts::DESKTOP:
      directoryId = "DesktopFolder";
      break;
    case cmWIXShortcuts::STARTUP:
      directoryId = "StartupFolder";
      break;
    default:
      return false;
    }

  featureDefinitions.BeginElement("FeatureRef");
  featureDefinitions.AddAttribute("Id", featureId);

  std::string cpackVendor;
  if(!RequireOption("CPACK_PACKAGE_VENDOR", cpackVendor))
    {
    return false;
    }

  std::string cpackPackageName;
  if(!RequireOption("CPACK_PACKAGE_NAME", cpackPackageName))
    {
    return false;
    }

  std::string idSuffix;
  if(!cpackComponentName.empty())
    {
      idSuffix += "_";
      idSuffix += cpackComponentName;
    }

  std::string componentId = "CM_SHORTCUT";
  if(idPrefix.size())
    {
    componentId += "_" + idPrefix;
    }

  componentId += idSuffix;

  fileDefinitions.BeginElement("DirectoryRef");
  fileDefinitions.AddAttribute("Id", directoryId);

  fileDefinitions.BeginElement("Component");
  fileDefinitions.AddAttribute("Id", componentId);
  fileDefinitions.AddAttribute("Guid", "*");

  std::string registryKey = std::string("Software\\") +
    cpackVendor + "\\" + cpackPackageName;

  shortcuts.EmitShortcuts(type, registryKey,
    cpackComponentName, fileDefinitions);

  if(type == cmWIXShortcuts::START_MENU)
    {
    fileDefinitions.EmitRemoveFolder(
      "CM_REMOVE_PROGRAM_MENU_FOLDER" + idSuffix);
    }

  if(emitUninstallShortcut)
    {
    fileDefinitions.EmitUninstallShortcut(cpackPackageName);
    }

  fileDefinitions.EndElement("Component");
  fileDefinitions.EndElement("DirectoryRef");

  featureDefinitions.EmitComponentRef(componentId);
  featureDefinitions.EndElement("FeatureRef");

  return true;
}

bool cmCPackWIXGenerator::CreateLicenseFile()
{
  std::string licenseSourceFilename;
  if(!RequireOption("CPACK_RESOURCE_FILE_LICENSE", licenseSourceFilename))
    {
    return false;
    }

  std::string licenseDestinationFilename;
  if(!RequireOption("CPACK_WIX_LICENSE_RTF", licenseDestinationFilename))
    {
    return false;
    }

  std::string extension = GetRightmostExtension(licenseSourceFilename);

  if(extension == ".rtf")
    {
    cmSystemTools::CopyAFile(
      licenseSourceFilename.c_str(),
      licenseDestinationFilename.c_str());
    }
  else if(extension == ".txt")
    {
    cmWIXRichTextFormatWriter rtfWriter(licenseDestinationFilename);

    cmsys::ifstream licenseSource(licenseSourceFilename.c_str());

    std::string line;
    while(std::getline(licenseSource, line))
      {
      rtfWriter.AddText(line);
      rtfWriter.AddText("\n");
      }
    }
  else
    {
    cmCPackLogger(cmCPackLog::LOG_ERROR,
      "unsupported WiX License file extension '" <<
      extension << "'" << std::endl);

    return false;
    }

  return true;
}

void cmCPackWIXGenerator::AddDirectoryAndFileDefinitons(
  std::string const& topdir,
  std::string const& directoryId,
  cmWIXDirectoriesSourceWriter& directoryDefinitions,
  cmWIXFilesSourceWriter& fileDefinitions,
  cmWIXFeaturesSourceWriter& featureDefinitions,
  std::vector<std::string> const& packageExecutables,
  std::vector<std::string> const& desktopExecutables,
  cmWIXShortcuts& shortcuts)
{
  cmsys::Directory dir;
  dir.Load(topdir.c_str());

  std::string relativeDirectoryPath =
    cmSystemTools::RelativePath(toplevel.c_str(), topdir.c_str());

  if(relativeDirectoryPath.empty())
    {
    relativeDirectoryPath = ".";
    }

  cmInstalledFile const* directoryInstalledFile =
    this->GetInstalledFile(relativeDirectoryPath);

  bool emptyDirectory = dir.GetNumberOfFiles() == 2;
  bool createDirectory = false;

  if(emptyDirectory)
    {
    createDirectory = true;
    }

  if(directoryInstalledFile)
    {
    if(directoryInstalledFile->HasProperty("CPACK_WIX_ACL"))
      {
      createDirectory = true;
      }
    }

  if(createDirectory)
    {
    std::string componentId = fileDefinitions.EmitComponentCreateFolder(
      directoryId, GenerateGUID(), directoryInstalledFile);
    featureDefinitions.EmitComponentRef(componentId);
    }

  if(emptyDirectory)
    {
    return;
    }

  for(size_t i = 0; i < dir.GetNumberOfFiles(); ++i)
    {
    std::string fileName = dir.GetFile(static_cast<unsigned long>(i));

    if(fileName == "." || fileName == "..")
      {
      continue;
      }

    std::string fullPath = topdir + "/" + fileName;

    std::string relativePath = cmSystemTools::RelativePath(
      toplevel.c_str(), fullPath.c_str());

    std::string id = PathToId(relativePath);

    if(cmSystemTools::FileIsDirectory(fullPath.c_str()))
      {
      std::string subDirectoryId = std::string("CM_D") + id;

      directoryDefinitions.BeginElement("Directory");
      directoryDefinitions.AddAttribute("Id", subDirectoryId);
      directoryDefinitions.AddAttribute("Name", fileName);

      AddDirectoryAndFileDefinitons(
        fullPath, subDirectoryId,
        directoryDefinitions,
        fileDefinitions,
        featureDefinitions,
        packageExecutables,
        desktopExecutables,
        shortcuts);

      this->Patch->ApplyFragment(subDirectoryId, directoryDefinitions);
      directoryDefinitions.EndElement("Directory");
      }
    else
      {
      cmInstalledFile const* installedFile =
        this->GetInstalledFile(relativePath);

      if(installedFile)
        {
        shortcuts.CreateFromProperties(id, directoryId, *installedFile);
        }

      std::string componentId = fileDefinitions.EmitComponentFile(
        directoryId, id, fullPath, *(this->Patch), installedFile);

      featureDefinitions.EmitComponentRef(componentId);

      for(size_t j = 0; j < packageExecutables.size(); ++j)
        {
        std::string const& executableName = packageExecutables[j++];
        std::string const& textLabel = packageExecutables[j];

        if(cmSystemTools::LowerCase(fileName) ==
            cmSystemTools::LowerCase(executableName) + ".exe")
          {
          cmWIXShortcut shortcut;
          shortcut.label= textLabel;
          shortcut.workingDirectoryId = directoryId;
          shortcuts.insert(cmWIXShortcuts::START_MENU, id, shortcut);

          if(desktopExecutables.size() &&
             std::find(desktopExecutables.begin(),
                       desktopExecutables.end(),
                       executableName)
             != desktopExecutables.end())
            {
              shortcuts.insert(cmWIXShortcuts::DESKTOP, id, shortcut);
            }
          }
        }
      }
    }
}

bool cmCPackWIXGenerator::RequireOption(
  std::string const& name, std::string &value) const
{
  const char* tmp = GetOption(name.c_str());
  if(tmp)
    {
    value = tmp;

    return true;
    }
  else
    {
    cmCPackLogger(cmCPackLog::LOG_ERROR,
      "Required variable " << name << " not set" << std::endl);

    return false;
    }
}

std::string cmCPackWIXGenerator::GetArchitecture() const
{
  std::string void_p_size;
  RequireOption("CPACK_WIX_SIZEOF_VOID_P", void_p_size);

  if(void_p_size == "8")
    {
    return "x64";
    }
  else
    {
    return "x86";
    }
}

std::string cmCPackWIXGenerator::GenerateGUID()
{
  UUID guid;
  UuidCreate(&guid);

  unsigned short *tmp = 0;
  UuidToStringW(&guid, &tmp);

  std::string result =
    cmsys::Encoding::ToNarrow(reinterpret_cast<wchar_t*>(tmp));
  RpcStringFreeW(&tmp);

  return cmSystemTools::UpperCase(result);
}

std::string cmCPackWIXGenerator::QuotePath(std::string const& path)
{
  return std::string("\"") + path + '"';
}

std::string cmCPackWIXGenerator::GetRightmostExtension(
  std::string const& filename)
{
  std::string extension;

  std::string::size_type i = filename.rfind(".");
  if(i != std::string::npos)
    {
    extension = filename.substr(i);
    }

  return cmSystemTools::LowerCase(extension);
}

std::string cmCPackWIXGenerator::PathToId(std::string const& path)
{
  id_map_t::const_iterator i = PathToIdMap.find(path);
  if(i != PathToIdMap.end()) return i->second;

  std::string id = CreateNewIdForPath(path);
  return id;
}

std::string cmCPackWIXGenerator::CreateNewIdForPath(std::string const& path)
{
  std::vector<std::string> components;
  cmSystemTools::SplitPath(path.c_str(), components, false);

  size_t replacementCount = 0;

  std::string identifier;
  std::string currentComponent;

  for(size_t i = 1; i < components.size(); ++i)
    {
    if(i != 1) identifier += '.';

    currentComponent = NormalizeComponentForId(
      components[i], replacementCount);

    identifier += currentComponent;
    }

  std::string idPrefix = "P";
  size_t replacementPercent = replacementCount * 100 / identifier.size();
  if(replacementPercent > 33 || identifier.size() > 60)
    {
    identifier = CreateHashedId(path, currentComponent);
    idPrefix = "H";
    }

  std::stringstream result;
  result << idPrefix << "_" << identifier;

  size_t ambiguityCount = ++IdAmbiguityCounter[identifier];

  if(ambiguityCount > 999)
    {
    cmCPackLogger(cmCPackLog::LOG_ERROR,
      "Error while trying to generate a unique Id for '" <<
      path << "'" << std::endl);

    return std::string();
    }
  else if(ambiguityCount > 1)
    {
    result << "_" << ambiguityCount;
    }

  std::string resultString = result.str();

  PathToIdMap[path] = resultString;

  return resultString;
}

std::string cmCPackWIXGenerator::CreateHashedId(
  std::string const& path, std::string const& normalizedFilename)
{
  cmsys::auto_ptr<cmCryptoHash> sha1 = cmCryptoHash::New("SHA1");
  std::string hash = sha1->HashString(path.c_str());

  std::string identifier;
  identifier += hash.substr(0, 7) + "_";

  const size_t maxFileNameLength = 52;
  if(normalizedFilename.length() > maxFileNameLength)
    {
    identifier += normalizedFilename.substr(0, maxFileNameLength - 3);
    identifier += "...";
    }
  else
    {
    identifier += normalizedFilename;
    }

  return identifier;
}

std::string cmCPackWIXGenerator::NormalizeComponentForId(
  std::string const& component, size_t& replacementCount)
{
  std::string result;
  result.resize(component.size());

  for(size_t i = 0; i < component.size(); ++i)
    {
    char c = component[i];
    if(IsLegalIdCharacter(c))
      {
      result[i] = c;
      }
    else
      {
      result[i] = '_';
      ++ replacementCount;
      }
    }

  return result;
}

bool cmCPackWIXGenerator::IsLegalIdCharacter(char c)
{
  return (c >= '0' && c <= '9') ||
      (c >= 'a' && c <= 'z') ||
      (c >= 'A' && c <= 'Z') ||
      c == '_' || c == '.';
}

void cmCPackWIXGenerator::CollectExtensions(
     std::string const& variableName, extension_set_t& extensions)
{
  const char *variableContent = GetOption(variableName.c_str());
  if(!variableContent) return;

  std::vector<std::string> list;
  cmSystemTools::ExpandListArgument(variableContent, list);
  extensions.insert(list.begin(), list.end());
}

void cmCPackWIXGenerator::AddCustomFlags(
  std::string const& variableName, std::ostream& stream)
{
  const char *variableContent = GetOption(variableName.c_str());
  if(!variableContent) return;

  std::vector<std::string> list;
  cmSystemTools::ExpandListArgument(variableContent, list);

  for(std::vector<std::string>::const_iterator i = list.begin();
    i != list.end(); ++i)
    {
      stream << " " << QuotePath(*i);
    }
}
