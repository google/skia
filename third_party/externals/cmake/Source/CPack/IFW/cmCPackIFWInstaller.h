/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/

#ifndef cmCPackIFWInstaller_h
#define cmCPackIFWInstaller_h

#include <cmGeneratedFileStream.h>
#include <cmStandardIncludes.h>

class cmCPackIFWPackage;
class cmCPackIFWGenerator;

/** \class cmCPackIFWInstaller
 * \brief A binary installer to be created CPack IFW generator
 */
class cmCPackIFWInstaller
{
public: // Types

  typedef std::map<std::string, cmCPackIFWPackage*> PackagesMap;

  struct RepositoryStruct
  {
    std::string Url;
    std::string Enabled;
    std::string Username;
    std::string Password;
    std::string DisplayName;
  };

public: // Constructor

  /**
   * Construct installer
   */
  cmCPackIFWInstaller();

public: // Configuration

  /// Name of the product being installed
  std::string Name;

  /// Version number of the product being installed
  std::string Version;

  /// Name of the installer as displayed on the title bar
  std::string Title;

  /// Publisher of the software (as shown in the Windows Control Panel)
  std::string Publisher;

  /// URL to a page that contains product information on your web site
  std::string ProductUrl;

  /// Filename for a custom installer icon
  std::string InstallerApplicationIcon;

  /// Filename for a custom window icon
  std::string InstallerWindowIcon;

  /// Filename for a logo
  std::string Logo;

  /// Name of the default program group in the Windows Start menu
  std::string StartMenuDir;

  /// Default target directory for installation
  std::string TargetDir;

  /// Default target directory for installation with administrator rights
  std::string AdminTargetDir;

  /// Filename of the generated maintenance tool
  std::string MaintenanceToolName;

  /// Filename for the configuration of the generated maintenance tool
  std::string MaintenanceToolIniFile;

  /// Set to true if the installation path can contain non-ASCII characters
  std::string AllowNonAsciiCharacters;

  /// Set to false if the installation path cannot contain space characters
  std::string AllowSpaceInPath;

  /// Filename for a custom installer control script
  std::string ControlScript;

public: // Internal implementation

  const char* GetOption(const std::string& op) const;
  bool IsOn(const std::string& op) const;

  bool IsVersionLess(const char *version);
  bool IsVersionGreater(const char *version);
  bool IsVersionEqual(const char *version);

  void ConfigureFromOptions();

  void GenerateInstallerFile();

  void GeneratePackageFiles();

  cmCPackIFWGenerator* Generator;
  PackagesMap Packages;
  std::vector<RepositoryStruct> Repositories;
  std::string Directory;

protected:
  void WriteGeneratedByToStrim(cmGeneratedFileStream& xout);
};

#endif // cmCPackIFWInstaller_h
