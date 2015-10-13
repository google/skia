/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/

#ifndef cmCPackIFWPackage_h
#define cmCPackIFWPackage_h

#include <cmStandardIncludes.h>
#include <cmGeneratedFileStream.h>

class cmCPackComponent;
class cmCPackComponentGroup;
class cmCPackIFWInstaller;
class cmCPackIFWGenerator;

/** \class cmCPackIFWPackage
 * \brief A single component to be installed by CPack IFW generator
 */
class cmCPackIFWPackage
{
public: // Types
  enum CompareTypes
  {
    CompareNone           = 0x0,
    CompareEqual          = 0x1,
    CompareLess           = 0x2,
    CompareLessOrEqual    = 0x3,
    CompareGreater        = 0x4,
    CompareGreaterOrEqual = 0x5
  };

  struct CompareStruct
  {
    CompareStruct();

    unsigned int Type;
    std::string Value;
  };

  struct DependenceStruct
  {
    DependenceStruct();
    DependenceStruct(const std::string &dependence);

    std::string Name;
    CompareStruct Compare;

    std::string NameWithCompare() const;

    bool operator < (const DependenceStruct &other) const
      {
      return Name < other.Name;
      }
  };

public: // [Con|De]structor

  /**
   * Construct package
   */
  cmCPackIFWPackage();

public: // Configuration

  /// Human-readable name of the component
  std::string DisplayName;

  /// Human-readable description of the component
  std::string Description;

  /// Version number of the component
  std::string Version;

  /// Date when this component version was released
  std::string ReleaseDate;

  /// Domain-like identification for this component
  std::string Name;

  /// File name of a script being loaded
  std::string Script;

  /// List of license agreements to be accepted by the installing user
  std::vector<std::string> Licenses;

  /// Priority of the component in the tree
  std::string SortingPriority;

  /// Set to true to preselect the component in the installer
  std::string Default;

  /// Set to true to hide the component from the installer
  std::string Virtual;

  /// Determines that the package must always be installed
  std::string ForcedInstallation;

public: // Internal implementation

  const char* GetOption(const std::string& op) const;
  bool IsOn(const std::string& op) const;

  bool IsVersionLess(const char *version);
  bool IsVersionGreater(const char *version);
  bool IsVersionEqual(const char *version);

  std::string GetComponentName(cmCPackComponent *component);

  void DefaultConfiguration();

  int ConfigureFromOptions();
  int ConfigureFromComponent(cmCPackComponent *component);
  int ConfigureFromGroup(cmCPackComponentGroup *group);
  int ConfigureFromGroup(const std::string &groupName);

  void GeneratePackageFile();

  // Pointer to generator
  cmCPackIFWGenerator* Generator;
  // Pointer to installer
  cmCPackIFWInstaller* Installer;
  // Collection of dependencies
  std::set<cmCPackIFWPackage*> Dependencies;
  // Collection of unresolved dependencies
  std::set<DependenceStruct*> AlienDependencies;
  // Patch to package directory
  std::string Directory;

protected:
  void WriteGeneratedByToStrim(cmGeneratedFileStream& xout);
};

#endif // cmCPackIFWPackage_h
