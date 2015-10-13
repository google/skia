/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmGlobalVisualStudio7Generator_h
#define cmGlobalVisualStudio7Generator_h

#include "cmGlobalVisualStudioGenerator.h"
#include "cmGlobalGeneratorFactory.h"

class cmTarget;
struct cmIDEFlagTable;

/** \class cmGlobalVisualStudio7Generator
 * \brief Write a Unix makefiles.
 *
 * cmGlobalVisualStudio7Generator manages UNIX build process for a tree
 */
class cmGlobalVisualStudio7Generator : public cmGlobalVisualStudioGenerator
{
public:
  cmGlobalVisualStudio7Generator(cmake* cm,
                                 const std::string& platformName = "");
  ~cmGlobalVisualStudio7Generator();

  static cmGlobalGeneratorFactory* NewFactory() {
    return new cmGlobalGeneratorSimpleFactory
      <cmGlobalVisualStudio7Generator>(); }

  ///! Get the name for the generator.
  virtual std::string GetName() const {
    return cmGlobalVisualStudio7Generator::GetActualName();}
  static std::string GetActualName() {return "Visual Studio 7";}

  ///! Get the name for the platform.
  std::string const& GetPlatformName() const;

  ///! Create a local generator appropriate to this Global Generator
  virtual cmLocalGenerator *CreateLocalGenerator(cmLocalGenerator* parent,
                                                 cmState::Snapshot snapshot);

  virtual bool SetSystemName(std::string const& s, cmMakefile* mf);

  virtual bool SetGeneratorPlatform(std::string const& p, cmMakefile* mf);

  /** Get the documentation entry for this generator.  */
  static void GetDocumentation(cmDocumentationEntry& entry);

  /**
   * Try to determine system information such as shared library
   * extension, pthreads, byte order etc.
   */
  virtual void EnableLanguage(std::vector<std::string>const& languages,
                              cmMakefile *, bool optional);

  /**
   * Try running cmake and building a file. This is used for dynamically
   * loaded commands, not as part of the usual build process.
   */
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

  /**
   * Generate the DSW workspace file.
   */
  virtual void OutputSLNFile();

  ///! Create a GUID or get an existing one.
  void CreateGUID(const std::string& name);
  std::string GetGUID(const std::string& name);

  /** Append the subdirectory for the given configuration.  */
  virtual void AppendDirectoryForConfig(const std::string& prefix,
                                        const std::string& config,
                                        const std::string& suffix,
                                        std::string& dir);

  ///! What is the configurations directory variable called?
  virtual const char* GetCMakeCFGIntDir() const
    { return "$(ConfigurationName)"; }

  /** Return true if the target project file should have the option
      LinkLibraryDependencies and link to .sln dependencies. */
  virtual bool NeedLinkLibraryDependencies(cmTarget&) { return false; }

  const char* GetIntelProjectVersion();

  virtual void FindMakeProgram(cmMakefile*);

  /** Is the Microsoft Assembler enabled?  */
  bool IsMasmEnabled() const { return this->MasmEnabled; }

  // Encoding for Visual Studio files
  virtual std::string Encoding();

  cmIDEFlagTable const* ExtraFlagTable;

protected:
  virtual void Generate();
  virtual const char* GetIDEVersion() { return "7.0"; }

  std::string const& GetDevEnvCommand();
  virtual std::string FindDevEnvCommand();

  static const char* ExternalProjectType(const char* location);

  virtual void OutputSLNFile(cmLocalGenerator* root,
                             std::vector<cmLocalGenerator*>& generators);
  virtual void WriteSLNFile(std::ostream& fout, cmLocalGenerator* root,
                            std::vector<cmLocalGenerator*>& generators);
  virtual void WriteProject(std::ostream& fout,
                            const std::string& name, const char* path,
                            cmTarget const& t);
  virtual void WriteProjectDepends(std::ostream& fout,
                           const std::string& name, const char* path,
                           cmTarget const&t);
  virtual void WriteProjectConfigurations(
    std::ostream& fout, const std::string& name, cmTarget::TargetType type,
    std::vector<std::string> const& configs,
    const std::set<std::string>& configsPartOfDefaultBuild,
    const std::string& platformMapping = "");
  virtual void WriteSLNGlobalSections(std::ostream& fout,
                                      cmLocalGenerator* root);
  virtual void WriteSLNFooter(std::ostream& fout);
  virtual void WriteSLNHeader(std::ostream& fout);
  virtual std::string WriteUtilityDepend(cmTarget const* target);

  virtual void WriteTargetsToSolution(
    std::ostream& fout,
    cmLocalGenerator* root,
    OrderedTargetDependSet const& projectTargets);
  virtual void WriteTargetDepends(
    std::ostream& fout,
    OrderedTargetDependSet const& projectTargets);
  virtual void WriteTargetConfigurations(
    std::ostream& fout,
    std::vector<std::string> const& configs,
    OrderedTargetDependSet const& projectTargets);

  virtual void WriteExternalProject(std::ostream& fout,
                                    const std::string& name,
                                    const char* path,
                                    const char* typeGuid,
                                    const std::set<std::string>&
                                    dependencies);

  std::string ConvertToSolutionPath(const char* path);

  std::set<std::string>
    IsPartOfDefaultBuild(std::vector<std::string> const& configs,
                         OrderedTargetDependSet const& projectTargets,
                         cmTarget const* target);
  bool IsDependedOn(OrderedTargetDependSet const& projectTargets,
                    cmTarget const* target);
  std::map<std::string, std::string> GUIDMap;

  virtual void WriteFolders(std::ostream& fout);
  virtual void WriteFoldersContent(std::ostream& fout);
  std::map<std::string,std::set<std::string> > VisualStudioFolders;

  // Set during OutputSLNFile with the name of the current project.
  // There is one SLN file per project.
  std::string CurrentProject;
  std::string GeneratorPlatform;
  std::string DefaultPlatformName;
  bool MasmEnabled;

private:
  char* IntelProjectVersion;
  std::string DevEnvCommand;
  bool DevEnvCommandInitialized;
  virtual std::string GetVSMakeProgram() { return this->GetDevEnvCommand(); }
};

#define CMAKE_CHECK_BUILD_SYSTEM_TARGET "ZERO_CHECK"

#endif
