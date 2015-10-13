/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmFindPackageCommand_h
#define cmFindPackageCommand_h

#include "cmFindCommon.h"

class cmFindPackageFileList;

/** \class cmFindPackageCommand
 * \brief Load settings from an external project.
 *
 * cmFindPackageCommand
 */
class cmFindPackageCommand : public cmFindCommon
{
public:
  cmFindPackageCommand();

  /**
   * This is a virtual constructor for the command.
   */
  virtual cmCommand* Clone()
    {
    return new cmFindPackageCommand;
    }

  /**
   * This is called when the command is first encountered in
   * the CMakeLists.txt file.
   */
  virtual bool InitialPass(std::vector<std::string> const& args,
                           cmExecutionStatus &status);

  /**
   * This determines if the command is invoked when in script mode.
   */
  virtual bool IsScriptable() const { return true; }

  /**
   * The name of the command as specified in CMakeList.txt.
   */
  virtual std::string GetName() const { return "find_package";}

  cmTypeMacro(cmFindPackageCommand, cmFindCommon);
private:
  class PathLabel : public cmFindCommon::PathLabel
  {
  protected:
    PathLabel();
  public:
    PathLabel(const std::string& label) : cmFindCommon::PathLabel(label) { }
    static PathLabel UserRegistry;
    static PathLabel Builds;
    static PathLabel SystemRegistry;
  };

  // Add additional search path labels and groups not present in the
  // parent class
  void AppendSearchPathGroups();

  void AppendSuccessInformation();
  void AppendToFoundProperty(bool found);
  void SetModuleVariables(const std::string& components);
  bool FindModule(bool& found);
  void AddFindDefinition(const std::string& var, const char* val);
  void RestoreFindDefinitions();
  bool HandlePackageMode();
  bool FindConfig();
  bool FindPrefixedConfig();
  bool FindFrameworkConfig();
  bool FindAppBundleConfig();
  enum PolicyScopeRule { NoPolicyScope, DoPolicyScope };
  bool ReadListFile(const char* f, PolicyScopeRule psr);
  void StoreVersionFound();

  void ComputePrefixes();
  void FillPrefixesCMakeEnvironment();
  void FillPrefixesCMakeVariable();
  void FillPrefixesSystemEnvironment();
  void FillPrefixesUserRegistry();
  void FillPrefixesSystemRegistry();
  void FillPrefixesCMakeSystemVariable();
  void FillPrefixesUserGuess();
  void FillPrefixesUserHints();
  void LoadPackageRegistryDir(std::string const& dir, cmSearchPath& outPaths);
  void LoadPackageRegistryWinUser();
  void LoadPackageRegistryWinSystem();
  void LoadPackageRegistryWin(bool user, unsigned int view,
                              cmSearchPath& outPaths);
  bool CheckPackageRegistryEntry(const std::string& fname,
                                 cmSearchPath& outPaths);
  bool SearchDirectory(std::string const& dir);
  bool CheckDirectory(std::string const& dir);
  bool FindConfigFile(std::string const& dir, std::string& file);
  bool CheckVersion(std::string const& config_file);
  bool CheckVersionFile(std::string const& version_file,
                        std::string& result_version);
  bool SearchPrefix(std::string const& prefix);
  bool SearchFrameworkPrefix(std::string const& prefix_in);
  bool SearchAppBundlePrefix(std::string const& prefix_in);

  friend class cmFindPackageFileList;

  struct OriginalDef { bool exists; std::string value; };
  std::map<std::string, OriginalDef> OriginalDefs;

  std::string Name;
  std::string Variable;
  std::string Version;
  unsigned int VersionMajor;
  unsigned int VersionMinor;
  unsigned int VersionPatch;
  unsigned int VersionTweak;
  unsigned int VersionCount;
  bool VersionExact;
  std::string FileFound;
  std::string VersionFound;
  unsigned int VersionFoundMajor;
  unsigned int VersionFoundMinor;
  unsigned int VersionFoundPatch;
  unsigned int VersionFoundTweak;
  unsigned int VersionFoundCount;
  cmIML_INT_uint64_t RequiredCMakeVersion;
  bool Quiet;
  bool Required;
  bool UseConfigFiles;
  bool UseFindModules;
  bool NoUserRegistry;
  bool NoSystemRegistry;
  bool DebugMode;
  bool UseLib64Paths;
  bool PolicyScope;
  std::string LibraryArchitecture;
  std::vector<std::string> Names;
  std::vector<std::string> Configs;
  std::set<std::string> IgnoredPaths;

  struct ConfigFileInfo { std::string filename; std::string version; };
  std::vector<ConfigFileInfo> ConsideredConfigs;
};

#endif
