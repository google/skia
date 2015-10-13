/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmGlobalUnixMakefileGenerator3_h
#define cmGlobalUnixMakefileGenerator3_h

#include "cmGlobalGenerator.h"
#include "cmGlobalGeneratorFactory.h"

class cmGeneratedFileStream;
class cmMakefileTargetGenerator;
class cmLocalUnixMakefileGenerator3;

/** \class cmGlobalUnixMakefileGenerator3
 * \brief Write a Unix makefiles.
 *
 * cmGlobalUnixMakefileGenerator3 manages UNIX build process for a tree


 The basic approach of this generator is to produce Makefiles that will all
 be run with the current working directory set to the Home Output
 directory. The one exception to this is the subdirectory Makefiles which are
 created as a convenience and just cd up to the Home Output directory and
 invoke the main Makefiles.

 The make process starts with Makefile. Makefile should only contain the
 targets the user is likely to invoke directly from a make command line. No
 internal targets should be in this file. Makefile2 contains the internal
 targets that are required to make the process work.

 Makefile2 in turn will recursively make targets in the correct order. Each
 target has its own directory \<target\>.dir and its own makefile build.make in
 that directory. Also in that directory is a couple makefiles per source file
 used by the target. Typically these are named source.obj.build.make and
 source.obj.build.depend.make. The source.obj.build.make contains the rules
 for building, cleaning, and computing dependencies for the given source
 file. The build.depend.make contains additional dependencies that were
 computed during dependency scanning. An additional file called
 source.obj.depend is used as a marker to indicate when dependencies must be
 rescanned.

 Rules for custom commands follow the same model as rules for source files.

 */

class cmGlobalUnixMakefileGenerator3 : public cmGlobalGenerator
{
public:
  cmGlobalUnixMakefileGenerator3(cmake* cm);
  static cmGlobalGeneratorFactory* NewFactory() {
    return new cmGlobalGeneratorSimpleFactory
      <cmGlobalUnixMakefileGenerator3>(); }

  ///! Get the name for the generator.
  virtual std::string GetName() const {
    return cmGlobalUnixMakefileGenerator3::GetActualName();}
  static std::string GetActualName() {return "Unix Makefiles";}

  /** Get the documentation entry for this generator.  */
  static void GetDocumentation(cmDocumentationEntry& entry);

  ///! Create a local generator appropriate to this Global Generator3
  virtual cmLocalGenerator *CreateLocalGenerator(cmLocalGenerator* parent,
                                                 cmState::Snapshot snapshot);

  /**
   * Try to determine system information such as shared library
   * extension, pthreads, byte order etc.
   */
  virtual void EnableLanguage(std::vector<std::string>const& languages,
                              cmMakefile *, bool optional);

  virtual void Configure();

  /**
   * Generate the all required files for building this project/tree. This
   * basically creates a series of LocalGenerators for each directory and
   * requests that they Generate.
   */
  virtual void Generate();


  void WriteMainCMakefileLanguageRules(cmGeneratedFileStream& cmakefileStream,
                                       std::vector<cmLocalGenerator *> &);

  // write out the help rule listing the valid targets
  void WriteHelpRule(std::ostream& ruleFileStream,
                     cmLocalUnixMakefileGenerator3 *);

  // write the top level target rules
  void WriteConvenienceRules(std::ostream& ruleFileStream,
                             std::set<std::string> &emitted);

  /** Get the command to use for a target that has no rule.  This is
      used for multiple output dependencies and for cmake_force.  */
  std::string GetEmptyRuleHackCommand() { return this->EmptyRuleHackCommand; }

  /** Get the fake dependency to use when a rule has no real commands
      or dependencies.  */
  std::string GetEmptyRuleHackDepends() { return this->EmptyRuleHackDepends; }

  // change the build command for speed
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

  /** Record per-target progress information.  */
  void RecordTargetProgress(cmMakefileTargetGenerator* tg);

  void AddCXXCompileCommand(const std::string &sourceFile,
                            const std::string &workingDirectory,
                            const std::string &compileCommand);

  /** Does the make tool tolerate .NOTPARALLEL? */
  virtual bool AllowNotParallel() const { return true; }

  /** Does the make tool tolerate .DELETE_ON_ERROR? */
  virtual bool AllowDeleteOnError() const { return true; }

  virtual void ComputeTargetObjectDirectory(cmGeneratorTarget* gt) const;

  std::string IncludeDirective;
  bool DefineWindowsNULL;
  bool PassMakeflags;
  bool UnixCD;
protected:
  void WriteMainMakefile2();
  void WriteMainCMakefile();

  void WriteConvenienceRules2(std::ostream& ruleFileStream,
                              cmLocalUnixMakefileGenerator3*);

  void WriteDirectoryRule2(std::ostream& ruleFileStream,
                           cmLocalUnixMakefileGenerator3* lg,
                           const char* pass, bool check_all,
                           bool check_relink);
  void WriteDirectoryRules2(std::ostream& ruleFileStream,
                            cmLocalUnixMakefileGenerator3* lg);

  void AppendGlobalTargetDepends(std::vector<std::string>& depends,
                                 cmTarget& target);

  // does this generator need a requires step for any of its targets
  bool NeedRequiresStep(cmTarget const&);

  // Target name hooks for superclass.
  const char* GetAllTargetName()           const { return "all"; }
  const char* GetInstallTargetName()       const { return "install"; }
  const char* GetInstallLocalTargetName()  const { return "install/local"; }
  const char* GetInstallStripTargetName()  const { return "install/strip"; }
  const char* GetPreinstallTargetName()    const { return "preinstall"; }
  const char* GetTestTargetName()          const { return "test"; }
  const char* GetPackageTargetName()       const { return "package"; }
  const char* GetPackageSourceTargetName() const { return "package_source"; }
  const char* GetEditCacheTargetName()     const { return "edit_cache"; }
  const char* GetRebuildCacheTargetName()  const { return "rebuild_cache"; }
  const char* GetCleanTargetName()         const { return "clean"; }

  virtual bool CheckALLOW_DUPLICATE_CUSTOM_TARGETS() const { return true; }

  // Some make programs (Borland) do not keep a rule if there are no
  // dependencies or commands.  This is a problem for creating rules
  // that might not do anything but might have other dependencies
  // added later.  If non-empty this variable holds a fake dependency
  // that can be added.
  std::string EmptyRuleHackDepends;

  // Some make programs (Watcom) do not like rules with no commands.
  // If non-empty this variable holds a bogus command that may be put
  // in the rule to satisfy the make program.
  std::string EmptyRuleHackCommand;

  // Store per-target progress counters.
  struct TargetProgress
  {
    TargetProgress(): NumberOfActions(0) {}
    unsigned long NumberOfActions;
    std::string VariableFile;
    std::vector<unsigned long> Marks;
    void WriteProgressVariables(unsigned long total, unsigned long& current);
  };
  typedef std::map<cmTarget const*, TargetProgress,
                   cmStrictTargetComparison> ProgressMapType;
  ProgressMapType ProgressMap;

  size_t CountProgressMarksInTarget(cmTarget const* target,
                                    std::set<cmTarget const*>& emitted);
  size_t CountProgressMarksInAll(cmLocalUnixMakefileGenerator3* lg);

  cmGeneratedFileStream *CommandDatabase;
private:
  virtual const char* GetBuildIgnoreErrorsFlag() const { return "-i"; }
  virtual std::string GetEditCacheCommand() const;
};

#endif
