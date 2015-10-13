/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/

#ifndef cmCPackIFWGenerator_h
#define cmCPackIFWGenerator_h

#include <cmGeneratedFileStream.h>
#include <CPack/cmCPackGenerator.h>

#include "cmCPackIFWPackage.h"
#include "cmCPackIFWInstaller.h"

/** \class cmCPackIFWGenerator
 * \brief A generator for Qt Installer Framework tools
 *
 * http://qt-project.org/doc/qtinstallerframework/index.html
 */
class cmCPackIFWGenerator : public cmCPackGenerator
{
public:
  cmCPackTypeMacro(cmCPackIFWGenerator, cmCPackGenerator);

  typedef std::map<std::string, cmCPackIFWPackage> PackagesMap;
  typedef std::map<std::string, cmCPackComponent> ComponentsMap;
  typedef std::map<std::string, cmCPackComponentGroup> ComponentGoupsMap;
  typedef std::map<std::string, cmCPackIFWPackage::DependenceStruct>
    DependenceMap;

  /**
   * Construct IFW generator
   */
  cmCPackIFWGenerator();

  /**
   * Destruct IFW generator
   */
  virtual ~cmCPackIFWGenerator();

  /**
   * Compare \a version with QtIFW framework version
   */
  bool IsVersionLess(const char *version);

  /**
   * Compare \a version with QtIFW framework version
   */
  bool IsVersionGreater(const char *version);

  /**
   * Compare \a version with QtIFW framework version
   */
  bool IsVersionEqual(const char *version);

protected: // cmCPackGenerator reimplementation

  /**
   * @brief Initialize generator
   * @return 0 on failure
   */
  virtual int InitializeInternal();
  virtual int PackageFiles();
  virtual const char* GetPackagingInstallPrefix();

  /**
   * @brief Extension of binary installer
   * @return Executable suffix or value from default implementation
   */
  virtual const char* GetOutputExtension();

  virtual std::string GetComponentInstallDirNameSuffix(
    const std::string& componentName);

  /**
   * @brief Get Component
   * @param projectName Project name
   * @param componentName Component name
   *
   * This method calls the base implementation.
   *
   * @return Pointer to component
   */
  virtual cmCPackComponent* GetComponent(
    const std::string& projectName,
    const std::string& componentName);

  /**
   * @brief Get group of component
   * @param projectName Project name
   * @param groupName Component group name
   *
   * This method calls the base implementation.
   *
   * @return Pointer to component group
   */
  virtual cmCPackComponentGroup* GetComponentGroup(
    const std::string& projectName,
    const std::string& groupName);

  enum cmCPackGenerator::CPackSetDestdirSupport SupportsSetDestdir() const;
  virtual bool SupportsAbsoluteDestination() const;
  virtual bool SupportsComponentInstallation() const;

protected: // Methods

  bool IsOnePackage() const;

  std::string GetRootPackageName();

  std::string GetGroupPackageName(cmCPackComponentGroup *group) const;
  std::string GetComponentPackageName(cmCPackComponent *component) const;

  cmCPackIFWPackage* GetGroupPackage(cmCPackComponentGroup *group) const;
  cmCPackIFWPackage* GetComponentPackage(cmCPackComponent *component) const;

  void WriteGeneratedByToStrim(cmGeneratedFileStream& xout);

protected: // Data

  friend class cmCPackIFWPackage;
  friend class cmCPackIFWInstaller;

  // Installer
  cmCPackIFWInstaller Installer;
  // Collection of packages
  PackagesMap Packages;
  // Collection of binary packages
  std::set<cmCPackIFWPackage*> BinaryPackages;
  // Collection of downloaded packages
  std::set<cmCPackIFWPackage*> DownloadedPackages;
  // Dependent packages
  DependenceMap DependentPackages;
  std::map<cmCPackComponent*, cmCPackIFWPackage*> ComponentPackages;
  std::map<cmCPackComponentGroup*, cmCPackIFWPackage*> GroupPackages;

private:
  std::string RepoGen;
  std::string BinCreator;
  std::string FrameworkVersion;
  std::string ExecutableSuffix;

  bool OnlineOnly;
  bool ResolveDuplicateNames;
  std::vector<std::string> PkgsDirsVector;
};

#endif
