/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/

#ifndef cmake_h
#define cmake_h

#include "cmListFileCache.h"
#include "cmSystemTools.h"
#include "cmInstalledFile.h"
#include "cmCacheManager.h"
#include "cmState.h"

class cmGlobalGeneratorFactory;
class cmGlobalGenerator;
class cmLocalGenerator;
class cmMakefile;
class cmVariableWatch;
class cmFileTimeComparison;
class cmExternalMakefileProjectGenerator;
class cmDocumentationSection;
class cmPolicies;
class cmTarget;
class cmGeneratedFileStream;

/** \brief Represents a cmake invocation.
 *
 * This class represents a cmake invocation. It is the top level class when
 * running cmake. Most cmake based GUIs should primarily create an instance
 * of this class and communicate with it.
 *
 * The basic process for a GUI is as follows:
 *
 * -# Create a cmake instance
 * -# Set the Home directories, generator, and cmake command. this
 *    can be done using the Set methods or by using SetArgs and passing in
 *    command line arguments.
 * -# Load the cache by calling LoadCache (duh)
 * -# if you are using command line arguments with -D or -C flags then
 *    call SetCacheArgs (or if for some other reason you want to modify the
 *    cache), do it now.
 * -# Finally call Configure
 * -# Let the user change values and go back to step 5
 * -# call Generate

 * If your GUI allows the user to change the home directories then
 * you must at a minimum redo steps 2 through 7.
 */

class cmake
{
 public:
  enum MessageType
  { AUTHOR_WARNING,
    FATAL_ERROR,
    INTERNAL_ERROR,
    MESSAGE,
    WARNING,
    LOG,
    DEPRECATION_ERROR,
    DEPRECATION_WARNING
  };


  /** \brief Describes the working modes of cmake */
  enum WorkingMode
  {
    NORMAL_MODE, ///< Cmake runs to create project files
    /** \brief Script mode (started by using -P).
     *
     * In script mode there is no generator and no cache. Also,
     * languages are not enabled, so add_executable and things do
     * nothing.
     */
    SCRIPT_MODE,
    /** \brief A pkg-config like mode
     *
     * In this mode cmake just searches for a package and prints the results to
     * stdout. This is similar to SCRIPT_MODE, but commands like add_library()
     * work too, since they may be used e.g. in exported target files. Started
     * via --find-package.
     */
    FIND_PACKAGE_MODE
  };
  typedef std::map<std::string, cmInstalledFile> InstalledFilesMap;

  /// Default constructor
  cmake();
  /// Destructor
  ~cmake();

  static const char *GetCMakeFilesDirectory() {return "/CMakeFiles";}
  static const char *GetCMakeFilesDirectoryPostSlash() {
    return "CMakeFiles/";}

  //@{
  /**
   * Set/Get the home directory (or output directory) in the project. The
   * home directory is the top directory of the project. It is the
   * path-to-source cmake was run with.
   */
  void SetHomeDirectory(const std::string& dir);
  const char* GetHomeDirectory() const;
  void SetHomeOutputDirectory(const std::string& dir);
  const char* GetHomeOutputDirectory() const;
  //@}

  /**
   * Handle a command line invocation of cmake.
   */
  int Run(const std::vector<std::string>&args)
    { return this->Run(args, false); }
  int Run(const std::vector<std::string>&args, bool noconfigure);

  /**
   * Run the global generator Generate step.
   */
  int Generate();

  /**
   * Configure the cmMakefiles. This routine will create a GlobalGenerator if
   * one has not already been set. It will then Call Configure on the
   * GlobalGenerator. This in turn will read in an process all the CMakeList
   * files for the tree. It will not produce any actual Makefiles, or
   * workspaces. Generate does that.  */
  int Configure();
  int ActualConfigure();

  ///! Break up a line like VAR:type="value" into var, type and value
  static bool ParseCacheEntry(const std::string& entry,
                         std::string& var,
                         std::string& value,
                         cmState::CacheEntryType& type);

  int LoadCache();
  bool LoadCache(const std::string& path);
  bool LoadCache(const std::string& path, bool internal,
                 std::set<std::string>& excludes,
                 std::set<std::string>& includes);
  bool SaveCache(const std::string& path);
  bool DeleteCache(const std::string& path);
  void PreLoadCMakeFiles();

  ///! Create a GlobalGenerator
  cmGlobalGenerator* CreateGlobalGenerator(const std::string& name);

  ///! Return the global generator assigned to this instance of cmake
  cmGlobalGenerator* GetGlobalGenerator()     { return this->GlobalGenerator; }
  ///! Return the global generator assigned to this instance of cmake, const
  const cmGlobalGenerator* GetGlobalGenerator() const
                                              { return this->GlobalGenerator; }

  ///! Return the global generator assigned to this instance of cmake
  void SetGlobalGenerator(cmGlobalGenerator *);

  ///! Get the names of the current registered generators
  void GetRegisteredGenerators(std::vector<std::string>& names);

  ///! Set the name of the selected generator-specific platform.
  void SetGeneratorPlatform(std::string const& ts)
    { this->GeneratorPlatform = ts; }

  ///! Get the name of the selected generator-specific platform.
  std::string const& GetGeneratorPlatform() const
    { return this->GeneratorPlatform; }

  ///! Set the name of the selected generator-specific toolset.
  void SetGeneratorToolset(std::string const& ts)
    { this->GeneratorToolset = ts; }

  ///! Get the name of the selected generator-specific toolset.
  std::string const& GetGeneratorToolset() const
    { return this->GeneratorToolset; }

  ///! get the cmCachemManager used by this invocation of cmake
  cmCacheManager *GetCacheManager() { return this->CacheManager; }

  /**
   * Given a variable name, return its value (as a string).
   */
  const char* GetCacheDefinition(const std::string&) const;
  ///! Add an entry into the cache
  void AddCacheEntry(const std::string& key, const char* value,
                     const char* helpString,
                     int type);

  /**
   * Get the system information and write it to the file specified
   */
  int GetSystemInformation(std::vector<std::string>&);

  ///! Parse command line arguments
  void SetArgs(const std::vector<std::string>&,
               bool directoriesSetBefore = false);

  ///! Is this cmake running as a result of a TRY_COMPILE command
  bool GetIsInTryCompile() const;
  void SetIsInTryCompile(bool b);

  ///! Parse command line arguments that might set cache values
  bool SetCacheArgs(const std::vector<std::string>&);

  typedef  void (*ProgressCallbackType)
    (const char*msg, float progress, void *);
  /**
   *  Set the function used by GUIs to receive progress updates
   *  Function gets passed: message as a const char*, a progress
   *  amount ranging from 0 to 1.0 and client data. The progress
   *  number provided may be negative in cases where a message is
   *  to be displayed without any progress percentage.
   */
  void SetProgressCallback(ProgressCallbackType f, void* clientData=0);

  ///! this is called by generators to update the progress
  void UpdateProgress(const char *msg, float prog);

  ///!  get the cmake policies instance
  cmPolicies *GetPolicies() {return this->Policies;}

  ///! Get the variable watch object
  cmVariableWatch* GetVariableWatch() { return this->VariableWatch; }

  void GetGeneratorDocumentation(std::vector<cmDocumentationEntry>&);

  ///! Set/Get a property of this target file
  void SetProperty(const std::string& prop, const char *value);
  void AppendProperty(const std::string& prop,
                      const char *value,bool asString=false);
  const char *GetProperty(const std::string& prop);
  bool GetPropertyAsBool(const std::string& prop);

  ///! Get or create an cmInstalledFile instance and return a pointer to it
  cmInstalledFile *GetOrCreateInstalledFile(
    cmMakefile* mf, const std::string& name);

  cmInstalledFile const* GetInstalledFile(const std::string& name) const;

  InstalledFilesMap const& GetInstalledFiles() const
    { return this->InstalledFiles; }

  ///! Do all the checks before running configure
  int DoPreConfigureChecks();

  void SetWorkingMode(WorkingMode mode) { this->CurrentWorkingMode = mode; }
  WorkingMode GetWorkingMode() { return this->CurrentWorkingMode; }

  ///! Debug the try compile stuff by not deleting the files
  bool GetDebugTryCompile(){return this->DebugTryCompile;}
  void DebugTryCompileOn(){this->DebugTryCompile = true;}

  /**
   * Generate CMAKE_ROOT and CMAKE_COMMAND cache entries
   */
  int AddCMakePaths();

  /**
   * Get the file comparison class
   */
  cmFileTimeComparison* GetFileComparison() { return this->FileComparison; }

  // Do we want debug output during the cmake run.
  bool GetDebugOutput() { return this->DebugOutput; }
  void SetDebugOutputOn(bool b) { this->DebugOutput = b;}

  // Do we want trace output during the cmake run.
  bool GetTrace() { return this->Trace;}
  void SetTrace(bool b) {  this->Trace = b;}
  bool GetWarnUninitialized() { return this->WarnUninitialized;}
  void SetWarnUninitialized(bool b) {  this->WarnUninitialized = b;}
  bool GetWarnUnused() { return this->WarnUnused;}
  void SetWarnUnused(bool b) {  this->WarnUnused = b;}
  bool GetWarnUnusedCli() { return this->WarnUnusedCli;}
  void SetWarnUnusedCli(bool b) {  this->WarnUnusedCli = b;}
  bool GetCheckSystemVars() { return this->CheckSystemVars;}
  void SetCheckSystemVars(bool b) {  this->CheckSystemVars = b;}

  void MarkCliAsUsed(const std::string& variable);

  /** Get the list of configurations (in upper case) considered to be
      debugging configurations.*/
  std::vector<std::string> GetDebugConfigs();

  void SetCMakeEditCommand(std::string const& s)
    { this->CMakeEditCommand = s; }
  std::string const& GetCMakeEditCommand() const
    { return this->CMakeEditCommand; }

  void SetSuppressDevWarnings(bool v)
    {
      this->SuppressDevWarnings = v;
      this->DoSuppressDevWarnings = true;
    }

  /** Display a message to the user.  */
  void IssueMessage(cmake::MessageType t, std::string const& text,
        cmListFileBacktrace const& backtrace = cmListFileBacktrace(NULL));
  void IssueMessage(cmake::MessageType t, std::string const& text,
        cmListFileContext const& lfc);

  ///! run the --build option
  int Build(const std::string& dir,
            const std::string& target,
            const std::string& config,
            const std::vector<std::string>& nativeOptions,
            bool clean);

  void UnwatchUnusedCli(const std::string& var);
  void WatchUnusedCli(const std::string& var);

  cmState* GetState() const { return this->State; }
  void SetCurrentSnapshot(cmState::Snapshot snapshot)
  { this->CurrentSnapshot = snapshot; }
  cmState::Snapshot GetCurrentSnapshot() const
  { return this->CurrentSnapshot; }

protected:
  void RunCheckForUnusedVariables();
  void InitializeProperties();
  int HandleDeleteCacheVariables(const std::string& var);

  typedef
     cmExternalMakefileProjectGenerator* (*CreateExtraGeneratorFunctionType)();
  typedef std::map<std::string,
                CreateExtraGeneratorFunctionType> RegisteredExtraGeneratorsMap;
  typedef std::vector<cmGlobalGeneratorFactory*> RegisteredGeneratorsVector;
  RegisteredGeneratorsVector Generators;
  RegisteredExtraGeneratorsMap ExtraGenerators;
  void AddDefaultCommands();
  void AddDefaultGenerators();
  void AddDefaultExtraGenerators();
  void AddExtraGenerator(const std::string& name,
                         CreateExtraGeneratorFunctionType newFunction);

  cmPolicies *Policies;
  cmGlobalGenerator *GlobalGenerator;
  cmCacheManager *CacheManager;
  bool SuppressDevWarnings;
  bool DoSuppressDevWarnings;
  std::string GeneratorPlatform;
  std::string GeneratorToolset;

  ///! read in a cmake list file to initialize the cache
  void ReadListFile(const std::vector<std::string>& args, const char *path);
  bool FindPackage(const std::vector<std::string>& args);

  ///! Check if CMAKE_CACHEFILE_DIR is set. If it is not, delete the log file.
  ///  If it is set, truncate it to 50kb
  void TruncateOutputLog(const char* fname);

  /**
   * Method called to check build system integrity at build time.
   * Returns 1 if CMake should rerun and 0 otherwise.
   */
  int CheckBuildSystem();

  void SetDirectoriesFromFile(const char* arg);

  //! Make sure all commands are what they say they are and there is no
  /// macros.
  void CleanupCommandsAndMacros();

  void GenerateGraphViz(const char* fileName) const;

  cmVariableWatch* VariableWatch;

private:
  cmake(const cmake&);  // Not implemented.
  void operator=(const cmake&);  // Not implemented.
  ProgressCallbackType ProgressCallback;
  void* ProgressCallbackClientData;
  bool Verbose;
  bool InTryCompile;
  WorkingMode CurrentWorkingMode;
  bool DebugOutput;
  bool Trace;
  bool WarnUninitialized;
  bool WarnUnused;
  bool WarnUnusedCli;
  bool CheckSystemVars;
  std::map<std::string, bool> UsedCliVariables;
  std::string CMakeEditCommand;
  std::string CXXEnvironment;
  std::string CCEnvironment;
  std::string CheckBuildSystemArgument;
  std::string CheckStampFile;
  std::string CheckStampList;
  std::string VSSolutionFile;
  bool ClearBuildSystem;
  bool DebugTryCompile;
  cmFileTimeComparison* FileComparison;
  std::string GraphVizFile;
  InstalledFilesMap InstalledFiles;

  cmState* State;
  cmState::Snapshot CurrentSnapshot;

  void UpdateConversionPathTable();

  // Print a list of valid generators to stderr.
  void PrintGeneratorList();

  bool PrintMessagePreamble(cmake::MessageType t, std::ostream& msg);
};

#define CMAKE_STANDARD_OPTIONS_TABLE \
  {"-C <initial-cache>", "Pre-load a script to populate the cache."}, \
  {"-D <var>[:<type>]=<value>", "Create a cmake cache entry."}, \
  {"-U <globbing_expr>", "Remove matching entries from CMake cache."}, \
  {"-G <generator-name>", "Specify a build system generator."},\
  {"-T <toolset-name>", "Specify toolset name if supported by generator."}, \
  {"-A <platform-name>", "Specify platform name if supported by generator."}, \
  {"-Wno-dev", "Suppress developer warnings."},\
  {"-Wdev", "Enable developer warnings."}

#define FOR_EACH_C_FEATURE(F) \
  F(c_function_prototypes) \
  F(c_restrict) \
  F(c_static_assert) \
  F(c_variadic_macros)

#define FOR_EACH_CXX_FEATURE(F) \
  F(cxx_aggregate_default_initializers) \
  F(cxx_alias_templates) \
  F(cxx_alignas) \
  F(cxx_alignof) \
  F(cxx_attributes) \
  F(cxx_attribute_deprecated) \
  F(cxx_auto_type) \
  F(cxx_binary_literals) \
  F(cxx_constexpr) \
  F(cxx_contextual_conversions) \
  F(cxx_decltype) \
  F(cxx_decltype_auto) \
  F(cxx_decltype_incomplete_return_types) \
  F(cxx_default_function_template_args) \
  F(cxx_defaulted_functions) \
  F(cxx_defaulted_move_initializers) \
  F(cxx_delegating_constructors) \
  F(cxx_deleted_functions) \
  F(cxx_digit_separators) \
  F(cxx_enum_forward_declarations) \
  F(cxx_explicit_conversions) \
  F(cxx_extended_friend_declarations) \
  F(cxx_extern_templates) \
  F(cxx_final) \
  F(cxx_func_identifier) \
  F(cxx_generalized_initializers) \
  F(cxx_generic_lambdas) \
  F(cxx_inheriting_constructors) \
  F(cxx_inline_namespaces) \
  F(cxx_lambdas) \
  F(cxx_lambda_init_captures) \
  F(cxx_local_type_template_args) \
  F(cxx_long_long_type) \
  F(cxx_noexcept) \
  F(cxx_nonstatic_member_init) \
  F(cxx_nullptr) \
  F(cxx_override) \
  F(cxx_range_for) \
  F(cxx_raw_string_literals) \
  F(cxx_reference_qualified_functions) \
  F(cxx_relaxed_constexpr) \
  F(cxx_return_type_deduction) \
  F(cxx_right_angle_brackets) \
  F(cxx_rvalue_references) \
  F(cxx_sizeof_member) \
  F(cxx_static_assert) \
  F(cxx_strong_enums) \
  F(cxx_template_template_parameters) \
  F(cxx_thread_local) \
  F(cxx_trailing_return_types) \
  F(cxx_unicode_literals) \
  F(cxx_uniform_initialization) \
  F(cxx_unrestricted_unions) \
  F(cxx_user_literals) \
  F(cxx_variable_templates) \
  F(cxx_variadic_macros) \
  F(cxx_variadic_templates)

#endif
