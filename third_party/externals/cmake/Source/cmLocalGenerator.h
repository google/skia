/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmLocalGenerator_h
#define cmLocalGenerator_h

#include "cmStandardIncludes.h"
#include "cmState.h"

class cmMakefile;
class cmGlobalGenerator;
class cmGeneratorTarget;
class cmTarget;
class cmTargetManifest;
class cmSourceFile;
class cmCustomCommand;
class cmCustomCommandGenerator;

/** \class cmLocalGenerator
 * \brief Create required build files for a directory.
 *
 * Subclasses of this abstract class generate makefiles, DSP, etc for various
 * platforms. This class should never be constructed directly. A
 * GlobalGenerator will create it and invoke the appropriate commands on it.
 */
class cmLocalGenerator
{
public:
  cmLocalGenerator(cmGlobalGenerator* gg, cmLocalGenerator* parent,
                   cmState::Snapshot snapshot);
  virtual ~cmLocalGenerator();

  /// @return whether we are processing the top CMakeLists.txt file.
  bool IsRootMakefile() const;

  /**
   * Generate the makefile for this directory.
   */
  virtual void Generate() {}

  /**
   * Process the CMakeLists files for this directory to fill in the
   * Makefile ivar
   */
  virtual void Configure();

  /**
   * Calls TraceVSDependencies() on all targets of this generator.
   */
  void TraceDependencies();

  virtual void AddHelperCommands() {}

  /**
   * Perform any final calculations prior to generation
   */
  void ConfigureFinalPass();

  /**
   * Generate the install rules files in this directory.
   */
  void GenerateInstallRules();

  /**
   * Generate the test files for tests.
   */
  void GenerateTestFiles();

  /**
   * Generate a manifest of target files that will be built.
   */
  void GenerateTargetManifest();

  ///! Get the makefile for this generator
  cmMakefile *GetMakefile() {
    return this->Makefile; }

  ///! Get the makefile for this generator, const version
    const cmMakefile *GetMakefile() const {
      return this->Makefile; }

  ///! Get the GlobalGenerator this is associated with
  cmGlobalGenerator *GetGlobalGenerator() {
    return this->GlobalGenerator; }
  const cmGlobalGenerator *GetGlobalGenerator() const {
    return this->GlobalGenerator; }

  cmState* GetState() const;
  cmState::Snapshot GetStateSnapshot() const;

  /**
   * Convert something to something else. This is a centralized conversion
   * routine used by the generators to handle relative paths and the like.
   * The flags determine what is actually done.
   *
   * relative: treat the argument as a directory and convert it to make it
   * relative or full or unchanged. If relative (HOME, START etc) then that
   * specifies what it should be relative to.
   *
   * output: make the result suitable for output to a...
   *
   * optional: should any relative path operation be controlled by the rel
   * path setting
   */
  enum RelativeRoot { NONE, FULL, HOME, START, HOME_OUTPUT, START_OUTPUT };
  enum OutputFormat { UNCHANGED, MAKERULE, SHELL, WATCOMQUOTE, RESPONSE };
  std::string ConvertToOutputFormat(const std::string& source,
                                    OutputFormat output);
  std::string Convert(const std::string& remote, RelativeRoot local,
                      OutputFormat output = UNCHANGED,
                      bool optional = false);
  std::string Convert(RelativeRoot remote, const std::string& local,
                      OutputFormat output = UNCHANGED,
                      bool optional = false);

  /**
    * Get path for the specified relative root.
    */
  const char* GetRelativeRootPath(RelativeRoot relroot);

  /**
   * Convert the given path to an output path that is optionally
   * relative based on the cache option CMAKE_USE_RELATIVE_PATHS.  The
   * remote path must use forward slashes and not already be escaped
   * or quoted.
   */
  std::string ConvertToOptionallyRelativeOutputPath(const std::string& remote);

  ///! set/get the parent generator
  cmLocalGenerator* GetParent() const {return this->Parent;}

  ///! set/get the children
  void AddChild(cmLocalGenerator* g) { this->Children.push_back(g); }
  std::vector<cmLocalGenerator*>& GetChildren() { return this->Children; }


  void AddArchitectureFlags(std::string& flags,
                            cmGeneratorTarget const* target,
                            const std::string&lang, const std::string& config);

  void AddLanguageFlags(std::string& flags, const std::string& lang,
                        const std::string& config);
  void AddCMP0018Flags(std::string &flags, cmTarget const* target,
                       std::string const& lang, const std::string& config);
  void AddVisibilityPresetFlags(std::string &flags, cmTarget const* target,
                                const std::string& lang);
  void AddConfigVariableFlags(std::string& flags, const std::string& var,
                              const std::string& config);
  void AddCompilerRequirementFlag(std::string &flags, cmTarget const* target,
                                  const std::string& lang);
  ///! Append flags to a string.
  virtual void AppendFlags(std::string& flags, const std::string& newFlags);
  virtual void AppendFlags(std::string& flags, const char* newFlags);
  virtual void AppendFlagEscape(std::string& flags,
                                const std::string& rawFlag);
  ///! Get the include flags for the current makefile and language
  std::string GetIncludeFlags(const std::vector<std::string> &includes,
                              cmGeneratorTarget* target,
                              const std::string& lang,
                              bool forceFullPaths = false,
                              bool forResponseFile = false,
                              const std::string& config = "");

  /**
   * Encode a list of preprocessor definitions for the compiler
   * command line.
   */
  void AppendDefines(std::set<std::string>& defines,
                     const char* defines_list);
  void AppendDefines(std::set<std::string>& defines,
                     std::string defines_list)
  {
    this->AppendDefines(defines, defines_list.c_str());
  }
  void AppendDefines(std::set<std::string>& defines,
                     const std::vector<std::string> &defines_vec);

  /**
   * Join a set of defines into a definesString with a space separator.
   */
  void JoinDefines(const std::set<std::string>& defines,
                   std::string &definesString,
                   const std::string& lang);

  /** Lookup and append options associated with a particular feature.  */
  void AppendFeatureOptions(std::string& flags, const std::string& lang,
                            const char* feature);

  /** \brief Get absolute path to dependency \a name
   *
   * Translate a dependency as given in CMake code to the name to
   * appear in a generated build file.
   * - If \a name is a utility target, returns false.
   * - If \a name is a CMake target, it will be transformed to the real output
   *   location of that target for the given configuration.
   * - If \a name is the full path to a file, it will be returned.
   * - Otherwise \a name is treated as a relative path with respect to
   *   the source directory of this generator.  This should only be
   *   used for dependencies of custom commands.
   */
  bool GetRealDependency(const std::string& name, const std::string& config,
                         std::string& dep);

  ///! for existing files convert to output path and short path if spaces
  std::string ConvertToOutputForExisting(const std::string& remote,
                                         RelativeRoot local = START_OUTPUT,
                                         OutputFormat format = SHELL);

  /** For existing path identified by RelativeRoot convert to output
      path and short path if spaces.  */
  std::string ConvertToOutputForExisting(RelativeRoot remote,
                                         const std::string& local = "",
                                         OutputFormat format = SHELL);

  virtual std::string ConvertToIncludeReference(std::string const& path,
                                                OutputFormat format = SHELL,
                                                bool forceFullPaths = false);

  /** Called from command-line hook to clear dependencies.  */
  virtual void ClearDependencies(cmMakefile* /* mf */,
                                 bool /* verbose */) {}

  /** Called from command-line hook to update dependencies.  */
  virtual bool UpdateDependencies(const char* /* tgtInfo */,
                                  bool /*verbose*/,
                                  bool /*color*/)
    { return true; }

  /** Get the include flags for the current makefile and language.  */
  void GetIncludeDirectories(std::vector<std::string>& dirs,
                             cmGeneratorTarget* target,
                             const std::string& lang = "C",
                             const std::string& config = "",
                             bool stripImplicitInclDirs = true);
  void AddCompileOptions(std::string& flags, cmTarget* target,
                         const std::string& lang, const std::string& config);
  void AddCompileDefinitions(std::set<std::string>& defines,
                             cmTarget const* target,
                             const std::string& config,
                             const std::string& lang);

  /** Compute the language used to compile the given source file.  */
  std::string GetSourceFileLanguage(const cmSourceFile& source);

  // Fill the vector with the target names for the object files,
  // preprocessed files and assembly files.
  virtual void GetIndividualFileTargets(std::vector<std::string>&) {}

  // Create a struct to hold the varibles passed into
  // ExpandRuleVariables
  struct RuleVariables
  {
    RuleVariables()
      {
        memset(this, 0,  sizeof(*this));
      }
    cmTarget* CMTarget;
    const char* TargetPDB;
    const char* TargetCompilePDB;
    const char* TargetVersionMajor;
    const char* TargetVersionMinor;
    const char* Language;
    const char* Objects;
    const char* Target;
    const char* LinkLibraries;
    const char* Source;
    const char* AssemblySource;
    const char* PreprocessedSource;
    const char* Output;
    const char* Object;
    const char* ObjectDir;
    const char* ObjectFileDir;
    const char* Flags;
    const char* ObjectsQuoted;
    const char* SONameFlag;
    const char* TargetSOName;
    const char* TargetInstallNameDir;
    const char* LinkFlags;
    const char* LanguageCompileFlags;
    const char* Defines;
    const char* RuleLauncher;
    const char* DependencyFile;
    const char* FilterPrefix;
  };

  /** Set whether to treat conversions to SHELL as a link script shell.  */
  void SetLinkScriptShell(bool b) { this->LinkScriptShell = b; }

  /** Escape the given string to be used as a command line argument in
      the native build system shell.  Optionally allow the build
      system to replace make variable references.  Optionally adjust
      escapes for the special case of passing to the native echo
      command.  */
  std::string EscapeForShell(const std::string& str, bool makeVars = false,
                             bool forEcho = false,
                             bool useWatcomQuote = false);

  /** Escape the given string as an argument in a CMake script.  */
  static std::string EscapeForCMake(const std::string& str);

  enum FortranFormat
    {
    FortranFormatNone,
    FortranFormatFixed,
    FortranFormatFree
    };
  FortranFormat GetFortranFormat(const char* value);

  /**
   * Convert the given remote path to a relative path with respect to
   * the given local path.  The local path must be given in component
   * form (see SystemTools::SplitPath) without a trailing slash.  The
   * remote path must use forward slashes and not already be escaped
   * or quoted.
   */
  std::string ConvertToRelativePath(const std::vector<std::string>& local,
                                    const std::string& remote,
                                    bool force=false);

  /**
   * Get the relative path from the generator output directory to a
   * per-target support directory.
   */
  virtual std::string GetTargetDirectory(cmTarget const& target) const;

  /**
   * Get the level of backwards compatibility requested by the project
   * in this directory.  This is the value of the CMake variable
   * CMAKE_BACKWARDS_COMPATIBILITY whose format is
   * "major.minor[.patch]".  The returned integer is encoded as
   *
   *   CMake_VERSION_ENCODE(major, minor, patch)
   *
   * and is monotonically increasing with the CMake version.
   */
  cmIML_INT_uint64_t GetBackwardsCompatibility();

  /**
   * Test whether compatibility is set to a given version or lower.
   */
  bool NeedBackwardsCompatibility_2_4();

  /**
   * Generate a Mac OS X application bundle Info.plist file.
   */
  void GenerateAppleInfoPList(cmTarget* target, const std::string& targetName,
                              const char* fname);

  /**
   * Generate a Mac OS X framework Info.plist file.
   */
  void GenerateFrameworkInfoPList(cmTarget* target,
                                  const std::string& targetName,
                                  const char* fname);
  /** Construct a comment for a custom command.  */
  std::string ConstructComment(cmCustomCommandGenerator const& ccg,
                               const char* default_comment = "");
  // Compute object file names.
  std::string GetObjectFileNameWithoutTarget(const cmSourceFile& source,
                                             std::string const& dir_max,
                                             bool* hasSourceExtension = 0);

  /** Fill out the static linker flags for the given target.  */
  void GetStaticLibraryFlags(std::string& flags,
                             std::string const& config,
                             cmTarget* target);

  /** Fill out these strings for the given target.  Libraries to link,
   *  flags, and linkflags. */
  void GetTargetFlags(std::string& linkLibs,
                      std::string& flags,
                      std::string& linkFlags,
                      std::string& frameworkPath,
                      std::string& linkPath,
                      cmGeneratorTarget* target,
                      bool useWatcomQuote);

  virtual void ComputeObjectFilenames(
                        std::map<cmSourceFile const*, std::string>& mapping,
                        cmGeneratorTarget const* gt = 0);

  bool IsWindowsShell() const;
  bool IsWatcomWMake() const;
  bool IsMinGWMake() const;
  bool IsNMake() const;

  void SetConfiguredCMP0014(bool configured);

protected:
  ///! put all the libraries for a target on into the given stream
  void OutputLinkLibraries(std::string& linkLibraries,
                                   std::string& frameworkPath,
                                   std::string& linkPath,
                                   cmGeneratorTarget &,
                                   bool relink,
                                   bool forResponseFile,
                                   bool useWatcomQuote);

  // Expand rule variables in CMake of the type found in language rules
  void ExpandRuleVariables(std::string& string,
                           const RuleVariables& replaceValues);
  // Expand rule variables in a single string
  std::string ExpandRuleVariable(std::string const& variable,
                                 const RuleVariables& replaceValues);

  const char* GetRuleLauncher(cmTarget* target, const std::string& prop);
  void InsertRuleLauncher(std::string& s, cmTarget* target,
                          const std::string& prop);


  /** Convert a target to a utility target for unsupported
   *  languages of a generator */
  void AddBuildTargetRule(const std::string& llang,
                          cmGeneratorTarget& target);
  ///! add a custom command to build a .o file that is part of a target
  void AddCustomCommandToCreateObject(const char* ofname,
                                      const std::string& lang,
                                      cmSourceFile& source,
                                      cmGeneratorTarget& target);
  // Create Custom Targets and commands for unsupported languages
  // The set passed in should contain the languages supported by the
  // generator directly.  Any targets containing files that are not
  // of the types listed will be compiled as custom commands and added
  // to a custom target.
  void CreateCustomTargetsAndCommands(std::set<std::string> const&);

  // Handle old-style install rules stored in the targets.
  void GenerateTargetInstallRules(
    std::ostream& os, const std::string& config,
    std::vector<std::string> const& configurationTypes);

  std::string& CreateSafeUniqueObjectFileName(const std::string& sin,
                                              std::string const& dir_max);
  void ComputeObjectMaxPath();

  virtual std::string ConvertToLinkReference(std::string const& lib,
                                             OutputFormat format = SHELL);

  /** Check whether the native build system supports the given
      definition.  Issues a warning.  */
  virtual bool CheckDefinition(std::string const& define) const;

  cmMakefile *Makefile;
  cmState::Snapshot StateSnapshot;
  cmGlobalGenerator *GlobalGenerator;
  cmLocalGenerator* Parent;
  std::vector<cmLocalGenerator*> Children;
  std::map<std::string, std::string> UniqueObjectNamesMap;
  std::string::size_type ObjectPathMax;
  std::set<std::string> ObjectMaxPathViolations;

  std::set<cmTarget const*> WarnCMP0063;

  bool LinkScriptShell;
  bool UseRelativePaths;
  bool Configured;
  bool EmitUniversalBinaryFlags;

  // Hack for ExpandRuleVariable until object-oriented version is
  // committed.
  std::string TargetImplib;

  cmIML_INT_uint64_t BackwardsCompatibility;
  bool BackwardsCompatibilityFinal;
private:
  std::string ConvertToOutputForExistingCommon(const std::string& remote,
                                               std::string const& result,
                                               OutputFormat format);

  void AddSharedFlags(std::string& flags, const std::string& lang,
                      bool shared);
  bool GetShouldUseOldFlags(bool shared, const std::string &lang) const;
  void AddPositionIndependentFlags(std::string& flags, std::string const& l,
                                   int targetType);
};

#endif
