/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/

#include "cmCPackIFWInstaller.h"

#include "cmCPackIFWGenerator.h"

#include <CPack/cmCPackLog.h>

#include <cmGeneratedFileStream.h>
#include <cmXMLSafe.h>

#ifdef cmCPackLogger
# undef cmCPackLogger
#endif
#define cmCPackLogger(logType, msg)                                     \
  do {                                                                  \
  std::ostringstream cmCPackLog_msg;                                    \
  cmCPackLog_msg << msg;                                                \
  if(Generator) {                                                       \
  Generator->Logger->Log(logType, __FILE__, __LINE__,                   \
                         cmCPackLog_msg.str().c_str());                 \
  }                                                                     \
  } while ( 0 )

//----------------------------------------------------------------------------
cmCPackIFWInstaller::cmCPackIFWInstaller() :
  Generator(0)
{
}

//----------------------------------------------------------------------------
const char *cmCPackIFWInstaller::GetOption(const std::string &op) const
{
  return Generator ? Generator->GetOption(op) : 0;
}

//----------------------------------------------------------------------------
bool cmCPackIFWInstaller::IsOn(const std::string &op) const
{
  return Generator ? Generator->IsOn(op) : false;
}

//----------------------------------------------------------------------------
bool cmCPackIFWInstaller::IsVersionLess(const char *version)
{
  return Generator ? Generator->IsVersionLess(version) : false;
}

//----------------------------------------------------------------------------
bool cmCPackIFWInstaller::IsVersionGreater(const char *version)
{
  return Generator ? Generator->IsVersionGreater(version) : false;
}

//----------------------------------------------------------------------------
bool cmCPackIFWInstaller::IsVersionEqual(const char *version)
{
  return Generator ? Generator->IsVersionEqual(version) : false;
}


//----------------------------------------------------------------------------
void cmCPackIFWInstaller::ConfigureFromOptions()
{
  // Name;
  if (const char* optIFW_PACKAGE_NAME =
      this->GetOption("CPACK_IFW_PACKAGE_NAME"))
    {
    Name = optIFW_PACKAGE_NAME;
    }
  else if (const char* optPACKAGE_NAME =
           this->GetOption("CPACK_PACKAGE_NAME"))
    {
    Name = optPACKAGE_NAME;
    }
  else
    {
    Name = "Your package";
    }

  // Title;
  if (const char* optIFW_PACKAGE_TITLE =
      GetOption("CPACK_IFW_PACKAGE_TITLE"))
    {
    Title = optIFW_PACKAGE_TITLE;
    }
  else if (const char* optPACKAGE_DESCRIPTION_SUMMARY =
           GetOption("CPACK_PACKAGE_DESCRIPTION_SUMMARY"))
    {
    Title = optPACKAGE_DESCRIPTION_SUMMARY;
    }
  else
    {
    Title = "Your package description";
    }

  // Version;
  if (const char* option = GetOption("CPACK_PACKAGE_VERSION"))
    {
    Version = option;
    }
  else
    {
    Version = "1.0.0";
    }

  // Publisher
  if(const char* optIFW_PACKAGE_PUBLISHER =
     GetOption("CPACK_IFW_PACKAGE_PUBLISHER"))
    {
    Publisher = optIFW_PACKAGE_PUBLISHER;
    }
  else if(const char* optPACKAGE_VENDOR = GetOption("CPACK_PACKAGE_VENDOR"))
    {
    Publisher = optPACKAGE_VENDOR;
    }

  // ProductUrl
  if(const char* option = GetOption("CPACK_IFW_PRODUCT_URL"))
    {
    ProductUrl = option;
    }

  // ApplicationIcon
  if(const char* option = GetOption("CPACK_IFW_PACKAGE_ICON"))
    {
    if(cmSystemTools::FileExists(option))
      {
      InstallerApplicationIcon = option;
      }
    else
      {
      // TODO: implement warning
      }
    }

  // WindowIcon
  if(const char* option = GetOption("CPACK_IFW_PACKAGE_WINDOW_ICON"))
    {
    if(cmSystemTools::FileExists(option))
      {
      InstallerWindowIcon = option;
      }
    else
      {
      // TODO: implement warning
      }
    }

  // Logo
  if(const char* option = GetOption("CPACK_IFW_PACKAGE_LOGO"))
    {
    if(cmSystemTools::FileExists(option))
      {
      Logo = option;
      }
    else
      {
      // TODO: implement warning
      }
    }

  // Start menu
  if (const char* optIFW_START_MENU_DIR =
      this->GetOption("CPACK_IFW_PACKAGE_START_MENU_DIRECTORY"))
    {
    StartMenuDir = optIFW_START_MENU_DIR;
    }
  else
    {
    StartMenuDir = Name;
    }

  // Default target directory for installation
  if (const char* optIFW_TARGET_DIRECTORY =
      GetOption("CPACK_IFW_TARGET_DIRECTORY"))
    {
    TargetDir = optIFW_TARGET_DIRECTORY;
    }
  else if (const char *optPACKAGE_INSTALL_DIRECTORY =
           GetOption("CPACK_PACKAGE_INSTALL_DIRECTORY"))
    {
    TargetDir = "@ApplicationsDir@/";
    TargetDir += optPACKAGE_INSTALL_DIRECTORY;
    }
  else
    {
    TargetDir = "@RootDir@/usr/local";
    }

  // Default target directory for installation with administrator rights
  if (const char* option = GetOption("CPACK_IFW_ADMIN_TARGET_DIRECTORY"))
    {
    AdminTargetDir = option;
    }

  // Repositories
  Repositories.clear();
  RepositoryStruct Repo;
  if(const char *site = this->GetOption("CPACK_DOWNLOAD_SITE"))
    {
    Repo.Url = site;
    Repositories.push_back(Repo);
    }
  if(const char *RepoAllStr = this->GetOption("CPACK_IFW_REPOSITORIES_ALL"))
    {
    std::vector<std::string> RepoAllVector;
    cmSystemTools::ExpandListArgument(RepoAllStr,
                                      RepoAllVector);
    for(std::vector<std::string>::iterator
          rit = RepoAllVector.begin(); rit != RepoAllVector.end(); ++rit)
      {
        std::string prefix = "CPACK_IFW_REPOSITORY_"
          + cmsys::SystemTools::UpperCase(*rit)
          + "_";
        // Url
        if (const char* url = GetOption(prefix + "URL"))
          {
          Repo.Url = url;
          }
        else
          {
          Repo.Url = "";
          }
        // Enabled
        if (IsOn(prefix + "DISABLED"))
          {
          Repo.Enabled = "0";
          }
        else
          {
          Repo.Enabled = "";
          }
        // Username
        if (const char* username = GetOption(prefix + "USERNAME"))
          {
          Repo.Username = username;
          }
        else
          {
          Repo.Username = "";
          }
        // Password
        if (const char* password = GetOption(prefix + "PASSWORD"))
          {
          Repo.Password = password;
          }
        else
          {
          Repo.Password = "";
          }
        // DisplayName
        if (const char* displayName = GetOption(prefix + "DISPLAY_NAME"))
          {
          Repo.DisplayName = displayName;
          }
        else
          {
          Repo.DisplayName = "";
          }

        if(!Repo.Url.empty())
          {
          Repositories.push_back(Repo);
          }
      }
    }

  // Maintenance tool
  if(const char* optIFW_MAINTENANCE_TOOL =
      this->GetOption("CPACK_IFW_PACKAGE_MAINTENANCE_TOOL_NAME"))
    {
    MaintenanceToolName = optIFW_MAINTENANCE_TOOL;
    }

  // Maintenance tool ini file
  if(const char* optIFW_MAINTENANCE_TOOL_INI =
      this->GetOption("CPACK_IFW_PACKAGE_MAINTENANCE_TOOL_INI_FILE"))
    {
    MaintenanceToolIniFile = optIFW_MAINTENANCE_TOOL_INI;
    }

  // Allow non-ASCII characters
  if(this->GetOption("CPACK_IFW_PACKAGE_ALLOW_NON_ASCII_CHARACTERS"))
    {
    if(IsOn("CPACK_IFW_PACKAGE_ALLOW_NON_ASCII_CHARACTERS"))
      {
      AllowNonAsciiCharacters = "true";
      }
    else
      {
      AllowNonAsciiCharacters = "false";
      }
    }

  // Space in path
  if(this->GetOption("CPACK_IFW_PACKAGE_ALLOW_SPACE_IN_PATH"))
    {
    if(IsOn("CPACK_IFW_PACKAGE_ALLOW_SPACE_IN_PATH"))
      {
      AllowSpaceInPath = "true";
      }
    else
      {
      AllowSpaceInPath = "false";
      }
    }

  // Control script
  if(const char* optIFW_CONTROL_SCRIPT =
      this->GetOption("CPACK_IFW_PACKAGE_CONTROL_SCRIPT"))
    {
    ControlScript = optIFW_CONTROL_SCRIPT;
    }
}

//----------------------------------------------------------------------------
void cmCPackIFWInstaller::GenerateInstallerFile()
{
  // Lazy directory initialization
  if(Directory.empty() && Generator)
    {
    Directory = Generator->toplevel;
    }

  // Output stream
  cmGeneratedFileStream xout((Directory + "/config/config.xml").data());

  xout << "<?xml version=\"1.0\"?>" << std::endl;

  WriteGeneratedByToStrim(xout);

  xout << "<Installer>" << std::endl;

  xout << "    <Name>" << cmXMLSafe(Name).str() << "</Name>" << std::endl;

  xout << "    <Version>" << Version << "</Version>" << std::endl;

  xout << "    <Title>" << cmXMLSafe(Title).str() << "</Title>"
       << std::endl;

  if(!Publisher.empty())
    {
    xout << "    <Publisher>" << cmXMLSafe(Publisher).str()
         << "</Publisher>" << std::endl;
    }

  if(!ProductUrl.empty())
    {
    xout << "    <ProductUrl>" << ProductUrl << "</ProductUrl>" << std::endl;
    }

  // ApplicationIcon
  if(!InstallerApplicationIcon.empty())
    {
    std::string name =
      cmSystemTools::GetFilenameName(InstallerApplicationIcon);
    std::string path = Directory + "/config/" + name;
    name = cmSystemTools::GetFilenameWithoutExtension(name);
    cmsys::SystemTools::CopyFileIfDifferent(
      InstallerApplicationIcon.data(), path.data());
    xout << "    <InstallerApplicationIcon>" << name
         << "</InstallerApplicationIcon>" << std::endl;
    }

  // WindowIcon
  if(!InstallerWindowIcon.empty())
    {
    std::string name = cmSystemTools::GetFilenameName(InstallerWindowIcon);
    std::string path = Directory + "/config/" + name;
    cmsys::SystemTools::CopyFileIfDifferent(
      InstallerWindowIcon.data(), path.data());
    xout << "    <InstallerWindowIcon>" << name
         << "</InstallerWindowIcon>" << std::endl;
    }

  // Logo
  if(!Logo.empty())
    {
    std::string name = cmSystemTools::GetFilenameName(Logo);
    std::string path = Directory + "/config/" + name;
    cmsys::SystemTools::CopyFileIfDifferent(Logo.data(), path.data());
    xout << "    <Logo>" << name << "</Logo>" << std::endl;
    }

  // Start menu
  if(!IsVersionLess("2.0"))
    {
    xout << "    <StartMenuDir>" << StartMenuDir
         << "</StartMenuDir>" << std::endl;
    }

  // Target dir
  if(!TargetDir.empty())
    {
    xout << "    <TargetDir>" << TargetDir << "</TargetDir>" << std::endl;
    }

  // Admin target dir
  if(!AdminTargetDir.empty())
    {
    xout << "    <AdminTargetDir>" << AdminTargetDir
         << "</AdminTargetDir>" << std::endl;
    }

  // Remote repositories
  if (!Repositories.empty())
    {
    xout << "    <RemoteRepositories>" << std::endl;
    for(std::vector<RepositoryStruct>::iterator
        rit = Repositories.begin(); rit != Repositories.end(); ++rit)
      {
      xout << "        <Repository>" << std::endl;
      // Url
      xout << "            <Url>" << rit->Url
           << "</Url>" << std::endl;
      // Enabled
      if(!rit->Enabled.empty())
        {
        xout << "            <Enabled>" << rit->Enabled
             << "</Enabled>" << std::endl;
        }
      // Username
      if(!rit->Username.empty())
        {
        xout << "            <Username>" << rit->Username
             << "</Username>" << std::endl;
        }
      // Password
      if(!rit->Password.empty())
        {
        xout << "            <Password>" << rit->Password
             << "</Password>" << std::endl;
        }
      // DisplayName
      if(!rit->DisplayName.empty())
        {
        xout << "            <DisplayName>" << rit->DisplayName
             << "</DisplayName>" << std::endl;
        }
      xout << "        </Repository>" << std::endl;
      }
    xout << "    </RemoteRepositories>" << std::endl;
    }

  // Maintenance tool
  if(!IsVersionLess("2.0") && !MaintenanceToolName.empty())
    {
    xout << "    <MaintenanceToolName>" << MaintenanceToolName
         << "</MaintenanceToolName>" << std::endl;
    }

  // Maintenance tool ini file
  if(!IsVersionLess("2.0") && !MaintenanceToolIniFile.empty())
    {
    xout << "    <MaintenanceToolIniFile>" << MaintenanceToolIniFile
         << "</MaintenanceToolIniFile>" << std::endl;
    }

  // Different allows
  if(IsVersionLess("2.0"))
    {
    // CPack IFW default policy
    xout << "    <!-- CPack IFW default policy for QtIFW less 2.0 -->"
         << std::endl;
    xout << "    <AllowNonAsciiCharacters>true</AllowNonAsciiCharacters>"
         << std::endl;
    xout << "    <AllowSpaceInPath>true</AllowSpaceInPath>" << std::endl;
    }
  else
    {
      if(!AllowNonAsciiCharacters.empty())
        {
        xout << "    <AllowNonAsciiCharacters>" << AllowNonAsciiCharacters
             << "</AllowNonAsciiCharacters>" << std::endl;
        }
      if(!AllowSpaceInPath.empty())
        {
        xout << "    <AllowAllowSpaceInPath>" << AllowSpaceInPath
             << "</AllowSpaceInPath>" << std::endl;
        }
    }

  // Control script (copy to config dir)
  if(!IsVersionLess("2.0") && !ControlScript.empty())
    {
    std::string name = cmSystemTools::GetFilenameName(ControlScript);
    std::string path = Directory + "/config/" + name;
    cmsys::SystemTools::CopyFileIfDifferent(ControlScript.data(), path.data());
    xout << "    <ControlScript>" << name << "</ControlScript>" << std::endl;
    }

  xout << "</Installer>" << std::endl;
}

//----------------------------------------------------------------------------
void cmCPackIFWInstaller::GeneratePackageFiles()
{
  if (Packages.empty() || Generator->IsOnePackage())
    {
    // Generate default package
    cmCPackIFWPackage package;
    package.Generator = Generator;
    package.Installer = this;
    // Check package group
    if (const char* option = GetOption("CPACK_IFW_PACKAGE_GROUP"))
      {
      package.ConfigureFromGroup(option);
      package.ForcedInstallation = "true";
      }
    else
      {
      package.ConfigureFromOptions();
      }
    package.GeneratePackageFile();
    return;
    }

  // Generate packages meta information
  for(PackagesMap::iterator pit = Packages.begin();
      pit != Packages.end(); ++pit)
    {
    cmCPackIFWPackage* package = pit->second;
    package->GeneratePackageFile();
  }
}

void cmCPackIFWInstaller::WriteGeneratedByToStrim(cmGeneratedFileStream &xout)
{
  if(Generator) Generator->WriteGeneratedByToStrim(xout);
}
