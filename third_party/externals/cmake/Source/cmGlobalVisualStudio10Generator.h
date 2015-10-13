/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmGlobalVisualStudio10Generator_h
#define cmGlobalVisualStudio10Generator_h

#include "cmGlobalVisualStudio8Generator.h"


/** \class cmGlobalVisualStudio10Generator
 * \brief Write a Unix makefiles.
 *
 * cmGlobalVisualStudio10Generator manages UNIX build process for a tree
 */
class cmGlobalVisualStudio10Generator :
  public cmGlobalVisualStudio8Generator
{
public:
  cmGlobalVisualStudio10Generator(cmake* cm, const std::string& name,
    const std::string& platformName);
  static cmGlobalGeneratorFactory* NewFactory();

  virtual bool MatchesGeneratorName(const std::string& name) const;

  virtual bool SetSystemName(std::string const& s, cmMakefile* mf);
  virtual bool SetGeneratorPlatform(std::string const& p, cmMakefile* mf);
  virtual bool SetGeneratorToolset(std::string const& ts, cmMakefile* mf);

  virtual void GenerateBuildCommand(
    std::vector<std::string>& makeCommand,
    const std::string& makeProgram,
    const std::string& projectName,
    const std::string& projectDir,
    const std::string& targetName,
    const std::string& config,
    bool fast, bool verbose,
    std::vector<std::string> const& makeOptions = std::vector<std::string>()
    );

  ///! create the correct local generator
  virtual cmLocalGenerator *CreateLocalGenerator(cmLocalGenerator* parent,
                                                 cmState::Snapshot snapshot);

  /**
   * Try to determine system information such as shared library
   * extension, pthreads, byte order etc.
   */
  virtual void EnableLanguage(std::vector<std::string>const& languages,
                              cmMakefile *, bool optional);
  virtual void WriteSLNHeader(std::ostream& fout);

  /** Is the installed VS an Express edition?  */
  bool IsExpressEdition() const { return this->ExpressEdition; }

  /** Generating for Nsight Tegra VS plugin?  */
  bool IsNsightTegra() const;
  std::string GetNsightTegraVersion() const;

  /** The toolset name for the target platform.  */
  const char* GetPlatformToolset() const;

  /** Return the CMAKE_SYSTEM_NAME.  */
  std::string const& GetSystemName() const { return this->SystemName; }

  /** Return the CMAKE_SYSTEM_VERSION.  */
  std::string const& GetSystemVersion() const { return this->SystemVersion; }

  /** Return true if building for WindowsCE */
  bool TargetsWindowsCE() const
    { return this->SystemIsWindowsCE; }

  /** Return true if building for WindowsPhone */
  bool TargetsWindowsPhone() const
    { return this->SystemIsWindowsPhone; }

  /** Return true if building for WindowsStore */
  bool TargetsWindowsStore() const
    { return this->SystemIsWindowsStore; }

  virtual const char* GetCMakeCFGIntDir() const
    { return "$(Configuration)";}
  bool Find64BitTools(cmMakefile* mf);

  /** Generate an <output>.rule file path for a given command output.  */
  virtual std::string GenerateRuleFile(std::string const& output) const;

  void PathTooLong(cmTarget* target, cmSourceFile const* sf,
                   std::string const& sfRel);

  virtual const char* GetToolsVersion() { return "4.0"; }

  virtual void FindMakeProgram(cmMakefile*);

  static std::string GetInstalledNsightTegraVersion();

protected:
  virtual void Generate();
  virtual bool InitializeSystem(cmMakefile* mf);
  virtual bool InitializeWindowsCE(cmMakefile* mf);
  virtual bool InitializeWindowsPhone(cmMakefile* mf);
  virtual bool InitializeWindowsStore(cmMakefile* mf);

  virtual std::string SelectWindowsCEToolset() const;
  virtual bool SelectWindowsPhoneToolset(std::string& toolset) const;
  virtual bool SelectWindowsStoreToolset(std::string& toolset) const;

  virtual const char* GetIDEVersion() { return "10.0"; }

  std::string const& GetMSBuildCommand();

  std::string GeneratorToolset;
  std::string DefaultPlatformToolset;
  std::string SystemName;
  std::string SystemVersion;
  std::string NsightTegraVersion;
  bool SystemIsWindowsCE;
  bool SystemIsWindowsPhone;
  bool SystemIsWindowsStore;
  bool ExpressEdition;

  bool UseFolderProperty();

private:
  class Factory;
  struct LongestSourcePath
  {
    LongestSourcePath(): Length(0), Target(0), SourceFile(0) {}
    size_t Length;
    cmTarget* Target;
    cmSourceFile const* SourceFile;
    std::string SourceRel;
  };
  LongestSourcePath LongestSource;

  std::string MSBuildCommand;
  bool MSBuildCommandInitialized;
  virtual std::string FindMSBuildCommand();
  virtual std::string FindDevEnvCommand();
  virtual std::string GetVSMakeProgram() { return this->GetMSBuildCommand(); }

  // We do not use the reload macros for VS >= 10.
  virtual std::string GetUserMacrosDirectory() { return ""; }
};
#endif
