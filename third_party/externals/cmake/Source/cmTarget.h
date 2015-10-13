/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmTarget_h
#define cmTarget_h

#include "cmCustomCommand.h"
#include "cmPropertyMap.h"
#include "cmPolicies.h"
#include "cmListFileCache.h"

#include <cmsys/auto_ptr.hxx>
#if defined(CMAKE_BUILD_WITH_CMAKE)
# ifdef CMake_HAVE_CXX11_UNORDERED_MAP
#  include <unordered_map>
# else
#  include <cmsys/hash_map.hxx>
# endif
#endif

#define CM_FOR_EACH_TARGET_POLICY(F) \
  F(CMP0003) \
  F(CMP0004) \
  F(CMP0008) \
  F(CMP0020) \
  F(CMP0021) \
  F(CMP0022) \
  F(CMP0027) \
  F(CMP0038) \
  F(CMP0041) \
  F(CMP0042) \
  F(CMP0046) \
  F(CMP0052) \
  F(CMP0060) \
  F(CMP0063)

class cmake;
class cmMakefile;
class cmSourceFile;
class cmGlobalGenerator;
class cmComputeLinkInformation;
class cmListFileBacktrace;
class cmTarget;
class cmGeneratorTarget;
class cmTargetTraceDependencies;

// Basic information about each link item.
class cmLinkItem: public std::string
{
  typedef std::string std_string;
public:
  cmLinkItem(): std_string(), Target(0) {}
  cmLinkItem(const std_string& n,
             cmTarget const* t): std_string(n), Target(t) {}
  cmLinkItem(cmLinkItem const& r): std_string(r), Target(r.Target) {}
  cmTarget const* Target;
};
class cmLinkImplItem: public cmLinkItem
{
public:
  cmLinkImplItem(): cmLinkItem(), Backtrace(0), FromGenex(false) {}
  cmLinkImplItem(std::string const& n,
                 cmTarget const* t,
                 cmListFileBacktrace const& bt,
                 bool fromGenex):
    cmLinkItem(n, t), Backtrace(bt), FromGenex(fromGenex) {}
  cmLinkImplItem(cmLinkImplItem const& r):
    cmLinkItem(r), Backtrace(r.Backtrace), FromGenex(r.FromGenex) {}
  cmListFileBacktrace Backtrace;
  bool FromGenex;
};

struct cmTargetLinkInformationMap:
  public std::map<std::string, cmComputeLinkInformation*>
{
  typedef std::map<std::string, cmComputeLinkInformation*> derived;
  cmTargetLinkInformationMap() {}
  cmTargetLinkInformationMap(cmTargetLinkInformationMap const& r);
  ~cmTargetLinkInformationMap();
};

class cmTargetInternals;
class cmTargetInternalPointer
{
public:
  cmTargetInternalPointer();
  cmTargetInternalPointer(cmTargetInternalPointer const& r);
  ~cmTargetInternalPointer();
  cmTargetInternalPointer& operator=(cmTargetInternalPointer const& r);
  cmTargetInternals* operator->() const { return this->Pointer; }
  cmTargetInternals* Get() const { return this->Pointer; }
private:
  cmTargetInternals* Pointer;
};

/** \class cmTarget
 * \brief Represent a library or executable target loaded from a makefile.
 *
 * cmTarget represents a target loaded from
 * a makefile.
 */
class cmTarget
{
public:
  cmTarget();
  enum TargetType { EXECUTABLE, STATIC_LIBRARY,
                    SHARED_LIBRARY, MODULE_LIBRARY,
                    OBJECT_LIBRARY, UTILITY, GLOBAL_TARGET,
                    INTERFACE_LIBRARY,
                    UNKNOWN_LIBRARY};
  static const char* GetTargetTypeName(TargetType targetType);
  enum CustomCommandType { PRE_BUILD, PRE_LINK, POST_BUILD };

  /**
   * Return the type of target.
   */
  TargetType GetType() const
    {
    return this->TargetTypeValue;
    }

  /**
   * Set the target type
   */
  void SetType(TargetType f, const std::string& name);

  void MarkAsImported();

  ///! Set/Get the name of the target
  const std::string& GetName() const {return this->Name;}
  std::string GetExportName() const;

  ///! Set the cmMakefile that owns this target
  void SetMakefile(cmMakefile *mf);
  cmMakefile *GetMakefile() const { return this->Makefile;}

#define DECLARE_TARGET_POLICY(POLICY) \
  cmPolicies::PolicyStatus GetPolicyStatus ## POLICY () const \
    { return this->PolicyStatus ## POLICY; }

  CM_FOR_EACH_TARGET_POLICY(DECLARE_TARGET_POLICY)

#undef DECLARE_TARGET_POLICY

  /**
   * Get the list of the custom commands for this target
   */
  std::vector<cmCustomCommand> const &GetPreBuildCommands() const
    {return this->PreBuildCommands;}
  std::vector<cmCustomCommand> const &GetPreLinkCommands() const
    {return this->PreLinkCommands;}
  std::vector<cmCustomCommand> const &GetPostBuildCommands() const
    {return this->PostBuildCommands;}
  void AddPreBuildCommand(cmCustomCommand const &cmd)
    {this->PreBuildCommands.push_back(cmd);}
  void AddPreLinkCommand(cmCustomCommand const &cmd)
    {this->PreLinkCommands.push_back(cmd);}
  void AddPostBuildCommand(cmCustomCommand const &cmd)
    {this->PostBuildCommands.push_back(cmd);}

  /**
   * Get the list of the source files used by this target
   */
  void GetSourceFiles(std::vector<cmSourceFile*> &files,
                      const std::string& config) const;
  bool GetConfigCommonSourceFiles(std::vector<cmSourceFile*>& files) const;

  /**
   * Add sources to the target.
   */
  void AddSources(std::vector<std::string> const& srcs);
  void AddTracedSources(std::vector<std::string> const& srcs);
  cmSourceFile* AddSourceCMP0049(const std::string& src);
  cmSourceFile* AddSource(const std::string& src);

  enum LinkLibraryType {GENERAL, DEBUG, OPTIMIZED};

  //* how we identify a library, by name and type
  typedef std::pair<std::string, LinkLibraryType> LibraryID;

  typedef std::vector<LibraryID > LinkLibraryVectorType;
  const LinkLibraryVectorType &GetOriginalLinkLibraries() const
    {return this->OriginalLinkLibraries;}

  /** Compute the link type to use for the given configuration.  */
  LinkLibraryType ComputeLinkType(const std::string& config) const;

  /**
   * Clear the dependency information recorded for this target, if any.
   */
  void ClearDependencyInformation(cmMakefile& mf, const std::string& target);

  // Check to see if a library is a framework and treat it different on Mac
  bool NameResolvesToFramework(const std::string& libname) const;
  void AddLinkLibrary(cmMakefile& mf,
                      const std::string& target, const std::string& lib,
                      LinkLibraryType llt);
  enum TLLSignature {
    KeywordTLLSignature,
    PlainTLLSignature
  };
  bool PushTLLCommandTrace(TLLSignature signature,
                           cmListFileContext const& lfc);
  void GetTllSignatureTraces(std::ostringstream &s, TLLSignature sig) const;

  void MergeLinkLibraries( cmMakefile& mf, const std::string& selfname,
                           const LinkLibraryVectorType& libs );

  const std::vector<std::string>& GetLinkDirectories() const;

  void AddLinkDirectory(const std::string& d);

  /**
   * Set the path where this target should be installed. This is relative to
   * INSTALL_PREFIX
   */
  std::string GetInstallPath() const {return this->InstallPath;}
  void SetInstallPath(const char *name) {this->InstallPath = name;}

  /**
   * Set the path where this target (if it has a runtime part) should be
   * installed. This is relative to INSTALL_PREFIX
   */
  std::string GetRuntimeInstallPath() const {return this->RuntimeInstallPath;}
  void SetRuntimeInstallPath(const char *name) {
    this->RuntimeInstallPath = name; }

  /**
   * Get/Set whether there is an install rule for this target.
   */
  bool GetHaveInstallRule() const { return this->HaveInstallRule; }
  void SetHaveInstallRule(bool h) { this->HaveInstallRule = h; }

  /** Add a utility on which this project depends. A utility is an executable
   * name as would be specified to the ADD_EXECUTABLE or UTILITY_SOURCE
   * commands. It is not a full path nor does it have an extension.
   */
  void AddUtility(const std::string& u, cmMakefile *makefile = 0);
  ///! Get the utilities used by this target
  std::set<std::string>const& GetUtilities() const { return this->Utilities; }
  std::set<cmLinkItem>const& GetUtilityItems() const;
  cmListFileBacktrace const* GetUtilityBacktrace(const std::string& u) const;

  /** Finalize the target at the end of the Configure step.  */
  void FinishConfigure();

  ///! Set/Get a property of this target file
  void SetProperty(const std::string& prop, const char *value);
  void AppendProperty(const std::string&  prop, const char* value,
          bool asString=false);
  const char *GetProperty(const std::string& prop) const;
  const char *GetProperty(const std::string& prop, cmMakefile* context) const;
  bool GetPropertyAsBool(const std::string& prop) const;
  void CheckProperty(const std::string& prop, cmMakefile* context) const;

  const char* GetFeature(const std::string& feature,
                         const std::string& config) const;
  bool GetFeatureAsBool(const std::string& feature,
                        const std::string& config) const;

  bool IsImported() const {return this->IsImportedTarget;}

  void GetObjectLibrariesCMP0026(std::vector<cmTarget*>& objlibs) const;

  /** The link interface specifies transitive library dependencies and
      other information needed by targets that link to this target.  */
  struct LinkInterfaceLibraries
  {
    // Libraries listed in the interface.
    std::vector<cmLinkItem> Libraries;
  };
  struct LinkInterface: public LinkInterfaceLibraries
  {
    // Languages whose runtime libraries must be linked.
    std::vector<std::string> Languages;

    // Shared library dependencies needed for linking on some platforms.
    std::vector<cmLinkItem> SharedDeps;

    // Number of repetitions of a strongly connected component of two
    // or more static libraries.
    int Multiplicity;

    // Libraries listed for other configurations.
    // Needed only for OLD behavior of CMP0003.
    std::vector<cmLinkItem> WrongConfigLibraries;

    bool ImplementationIsInterface;

    LinkInterface(): Multiplicity(0), ImplementationIsInterface(false) {}
  };

  /** Get the link interface for the given configuration.  Returns 0
      if the target cannot be linked.  */
  LinkInterface const* GetLinkInterface(const std::string& config,
                                        cmTarget const* headTarget) const;
  LinkInterfaceLibraries const*
    GetLinkInterfaceLibraries(const std::string& config,
                              cmTarget const* headTarget,
                              bool usage_requirements_only) const;

  std::vector<cmTarget const*> const&
    GetLinkImplementationClosure(const std::string& config) const;

  struct CompatibleInterfaces
  {
    std::set<std::string> PropsBool;
    std::set<std::string> PropsString;
    std::set<std::string> PropsNumberMax;
    std::set<std::string> PropsNumberMin;
  };
  CompatibleInterfaces const&
    GetCompatibleInterfaces(std::string const& config) const;

  /** The link implementation specifies the direct library
      dependencies needed by the object files of the target.  */
  struct LinkImplementationLibraries
  {
    // Libraries linked directly in this configuration.
    std::vector<cmLinkImplItem> Libraries;

    // Libraries linked directly in other configurations.
    // Needed only for OLD behavior of CMP0003.
    std::vector<cmLinkItem> WrongConfigLibraries;
  };
  struct LinkImplementation: public LinkImplementationLibraries
  {
    // Languages whose runtime libraries must be linked.
    std::vector<std::string> Languages;
  };
  LinkImplementation const*
    GetLinkImplementation(const std::string& config) const;

  LinkImplementationLibraries const*
    GetLinkImplementationLibraries(const std::string& config) const;

  /** Link information from the transitive closure of the link
      implementation and the interfaces of its dependencies.  */
  struct LinkClosure
  {
    // The preferred linker language.
    std::string LinkerLanguage;

    // Languages whose runtime libraries must be linked.
    std::vector<std::string> Languages;
  };
  LinkClosure const* GetLinkClosure(const std::string& config) const;

  cmTarget const* FindTargetToLink(std::string const& name) const;

  /** Strip off leading and trailing whitespace from an item named in
      the link dependencies of this target.  */
  std::string CheckCMP0004(std::string const& item) const;

  /** Get the directory in which this target will be built.  If the
      configuration name is given then the generator will add its
      subdirectory for that configuration.  Otherwise just the canonical
      output directory is given.  */
  std::string GetDirectory(const std::string& config = "",
                           bool implib = false) const;

  /** Get the directory in which this targets .pdb files will be placed.
      If the configuration name is given then the generator will add its
      subdirectory for that configuration.  Otherwise just the canonical
      pdb output directory is given.  */
  std::string GetPDBDirectory(const std::string& config) const;

  /** Get the directory in which to place the target compiler .pdb file.
      If the configuration name is given then the generator will add its
      subdirectory for that configuration.  Otherwise just the canonical
      compiler pdb output directory is given.  */
  std::string GetCompilePDBDirectory(const std::string& config = "") const;

  /** Get the location of the target in the build tree for the given
      configuration.  */
  const char* GetLocation(const std::string& config) const;

  /** Get the location of the target in the build tree with a placeholder
      referencing the configuration in the native build system.  This
      location is suitable for use as the LOCATION target property.  */
  const char* GetLocationForBuild() const;

  /** Get the target major and minor version numbers interpreted from
      the VERSION property.  Version 0 is returned if the property is
      not set or cannot be parsed.  */
  void GetTargetVersion(int& major, int& minor) const;

  /** Get the target major, minor, and patch version numbers
      interpreted from the VERSION or SOVERSION property.  Version 0
      is returned if the property is not set or cannot be parsed.  */
  void
  GetTargetVersion(bool soversion, int& major, int& minor, int& patch) const;

  ///! Return the preferred linker language for this target
  std::string GetLinkerLanguage(const std::string& config = "") const;

  /** Get the full name of the target according to the settings in its
      makefile.  */
  std::string GetFullName(const std::string& config="",
                          bool implib = false) const;
  void GetFullNameComponents(std::string& prefix,
                             std::string& base, std::string& suffix,
                             const std::string& config="",
                             bool implib = false) const;

  /** Get the name of the pdb file for the target.  */
  std::string GetPDBName(const std::string& config) const;

  /** Get the name of the compiler pdb file for the target.  */
  std::string GetCompilePDBName(const std::string& config="") const;

  /** Get the path for the MSVC /Fd option for this target.  */
  std::string GetCompilePDBPath(const std::string& config="") const;

  /** Whether this library has soname enabled and platform supports it.  */
  bool HasSOName(const std::string& config) const;

  /** Get the soname of the target.  Allowed only for a shared library.  */
  std::string GetSOName(const std::string& config) const;

  /** Whether this library has \@rpath and platform supports it.  */
  bool HasMacOSXRpathInstallNameDir(const std::string& config) const;

  /** Whether this library defaults to \@rpath.  */
  bool MacOSXRpathInstallNameDirDefault() const;

  /** Test for special case of a third-party shared library that has
      no soname at all.  */
  bool IsImportedSharedLibWithoutSOName(const std::string& config) const;

  /** Get the full path to the target according to the settings in its
      makefile and the configuration type.  */
  std::string GetFullPath(const std::string& config="", bool implib = false,
                          bool realname = false) const;

  /** Get the names of the library needed to generate a build rule
      that takes into account shared library version numbers.  This
      should be called only on a library target.  */
  void GetLibraryNames(std::string& name, std::string& soName,
                       std::string& realName, std::string& impName,
                       std::string& pdbName, const std::string& config) const;

  /** Get the names of the executable needed to generate a build rule
      that takes into account executable version numbers.  This should
      be called only on an executable target.  */
  void GetExecutableNames(std::string& name, std::string& realName,
                          std::string& impName,
                          std::string& pdbName,
                          const std::string& config) const;

  /** Does this target have a GNU implib to convert to MS format?  */
  bool HasImplibGNUtoMS() const;

  /** Convert the given GNU import library name (.dll.a) to a name with a new
      extension (.lib or ${CMAKE_IMPORT_LIBRARY_SUFFIX}).  */
  bool GetImplibGNUtoMS(std::string const& gnuName, std::string& out,
                        const char* newExt = 0) const;

  /**
   * Compute whether this target must be relinked before installing.
   */
  bool NeedRelinkBeforeInstall(const std::string& config) const;

  bool HaveBuildTreeRPATH(const std::string& config) const;
  bool HaveInstallTreeRPATH() const;

  /** Return true if builtin chrpath will work for this target */
  bool IsChrpathUsed(const std::string& config) const;

  /** Return the install name directory for the target in the
    * build tree.  For example: "\@rpath/", "\@loader_path/",
    * or "/full/path/to/library".  */
  std::string GetInstallNameDirForBuildTree(const std::string& config) const;

  /** Return the install name directory for the target in the
    * install tree.  For example: "\@rpath/" or "\@loader_path/". */
  std::string GetInstallNameDirForInstallTree() const;

  cmComputeLinkInformation*
    GetLinkInformation(const std::string& config) const;

  // Get the properties
  cmPropertyMap &GetProperties() const { return this->Properties; }

  bool GetMappedConfig(std::string const& desired_config,
                       const char** loc,
                       const char** imp,
                       std::string& suffix) const;

  /** Get the macro to define when building sources in this target.
      If no macro should be defined null is returned.  */
  const char* GetExportMacro() const;

  void GetCompileDefinitions(std::vector<std::string> &result,
                             const std::string& config,
                             const std::string& language) const;

  // Compute the set of languages compiled by the target.  This is
  // computed every time it is called because the languages can change
  // when source file properties are changed and we do not have enough
  // information to forward these property changes to the targets
  // until we have per-target object file properties.
  void GetLanguages(std::set<std::string>& languages,
                    std::string const& config) const;

  /** Return whether this target is an executable with symbol exports
      enabled.  */
  bool IsExecutableWithExports() const;

  /** Return whether this target may be used to link another target.  */
  bool IsLinkable() const;

  /** Return whether or not the target is for a DLL platform.  */
  bool IsDLLPlatform() const { return this->DLLPlatform; }

  /** Return whether or not the target has a DLL import library.  */
  bool HasImportLibrary() const;

  /** Return whether this target is a shared library Framework on
      Apple.  */
  bool IsFrameworkOnApple() const;

  /** Return whether this target is a CFBundle (plugin) on Apple.  */
  bool IsCFBundleOnApple() const;

  /** Return whether this target is a XCTest on Apple.  */
  bool IsXCTestOnApple() const;

  /** Return whether this target is an executable Bundle on Apple.  */
  bool IsAppBundleOnApple() const;

  /** Return whether this target is an executable Bundle, a framework
      or CFBundle on Apple.  */
  bool IsBundleOnApple() const;

  /** Return the framework version string.  Undefined if
      IsFrameworkOnApple returns false.  */
  std::string GetFrameworkVersion() const;

  /** Get a backtrace from the creation of the target.  */
  cmListFileBacktrace const& GetBacktrace() const;

  /** Get a build-tree directory in which to place target support files.  */
  std::string GetSupportDirectory() const;

  /** Return whether this target uses the default value for its output
      directory.  */
  bool UsesDefaultOutputDir(const std::string& config, bool implib) const;

  /** @return the mac content directory for this target. */
  std::string GetMacContentDirectory(const std::string& config,
                                     bool implib) const;

  /** @return whether this target have a well defined output file name. */
  bool HaveWellDefinedOutputFiles() const;

  /** @return the Mac framework directory without the base. */
  std::string GetFrameworkDirectory(const std::string& config,
                                    bool rootDir) const;

  /** @return the Mac CFBundle directory without the base */
  std::string GetCFBundleDirectory(const std::string& config,
                                   bool contentOnly) const;

  /** @return the Mac App directory without the base */
  std::string GetAppBundleDirectory(const std::string& config,
                                    bool contentOnly) const;

  std::vector<std::string> GetIncludeDirectories(
                     const std::string& config,
                     const std::string& language) const;
  void InsertInclude(const cmValueWithOrigin &entry,
                     bool before = false);
  void InsertCompileOption(const cmValueWithOrigin &entry,
                     bool before = false);
  void InsertCompileDefinition(const cmValueWithOrigin &entry);

  void AppendBuildInterfaceIncludes();

  void GetCompileOptions(std::vector<std::string> &result,
                         const std::string& config,
                         const std::string& language) const;
  void GetAutoUicOptions(std::vector<std::string> &result,
                         const std::string& config) const;
  void GetCompileFeatures(std::vector<std::string> &features,
                          const std::string& config) const;

  bool IsNullImpliedByLinkLibraries(const std::string &p) const;
  bool IsLinkInterfaceDependentBoolProperty(const std::string &p,
                         const std::string& config) const;
  bool IsLinkInterfaceDependentStringProperty(const std::string &p,
                         const std::string& config) const;
  bool IsLinkInterfaceDependentNumberMinProperty(const std::string &p,
                         const std::string& config) const;
  bool IsLinkInterfaceDependentNumberMaxProperty(const std::string &p,
                         const std::string& config) const;

  bool GetLinkInterfaceDependentBoolProperty(const std::string &p,
                                             const std::string& config) const;

  const char *GetLinkInterfaceDependentStringProperty(const std::string &p,
                         const std::string& config) const;
  const char *GetLinkInterfaceDependentNumberMinProperty(const std::string &p,
                         const std::string& config) const;
  const char *GetLinkInterfaceDependentNumberMaxProperty(const std::string &p,
                         const std::string& config) const;

  std::string GetDebugGeneratorExpressions(const std::string &value,
                                  cmTarget::LinkLibraryType llt) const;

  void AddSystemIncludeDirectories(const std::set<std::string> &incs);
  void AddSystemIncludeDirectories(const std::vector<std::string> &incs);
  std::set<std::string> const & GetSystemIncludeDirectories() const
    { return this->SystemIncludeDirectories; }

  bool LinkLanguagePropagatesToDependents() const
  { return this->TargetTypeValue == STATIC_LIBRARY; }

  void ReportPropertyOrigin(const std::string &p,
                            const std::string &result,
                            const std::string &report,
                            const std::string &compatibilityType) const;

  std::map<std::string, std::string> const&
  GetMaxLanguageStandards() const
  {
    return this->MaxLanguageStandards;
  }

#if defined(_WIN32) && !defined(__CYGWIN__)
  const LinkLibraryVectorType &GetLinkLibrariesForVS6() const {
  return this->LinkLibrariesForVS6;}
#endif

private:
  bool HandleLocationPropertyPolicy(cmMakefile* context) const;

  // The set of include directories that are marked as system include
  // directories.
  std::set<std::string> SystemIncludeDirectories;

  std::vector<std::pair<TLLSignature, cmListFileContext> > TLLCommands;

#if defined(_WIN32) && !defined(__CYGWIN__)
  /**
   * A list of direct dependencies. Use in conjunction with DependencyMap.
   */
  typedef std::vector< LibraryID > DependencyList;

  /**
   * This map holds the dependency graph. map[x] returns a set of
   * direct dependencies of x. Note that the direct depenencies are
   * ordered. This is necessary to handle direct dependencies that
   * themselves have no dependency information.
   */
  typedef std::map< LibraryID, DependencyList > DependencyMap;

  /**
   * Inserts \a dep at the end of the dependency list of \a lib.
   */
  void InsertDependencyForVS6( DependencyMap& depMap,
                               const LibraryID& lib,
                               const LibraryID& dep);

  /*
   * Deletes \a dep from the dependency list of \a lib.
   */
  void DeleteDependencyForVS6( DependencyMap& depMap,
                               const LibraryID& lib,
                               const LibraryID& dep);

  /**
   * Emits the library \a lib and all its dependencies into link_line.
   * \a emitted keeps track of the libraries that have been emitted to
   * avoid duplicates--it is more efficient than searching
   * link_line. \a visited is used detect cycles. Note that \a
   * link_line is in reverse order, in that the dependencies of a
   * library are listed before the library itself.
   */
  void EmitForVS6( const LibraryID lib,
                   const DependencyMap& dep_map,
                   std::set<LibraryID>& emitted,
                   std::set<LibraryID>& visited,
                   DependencyList& link_line);

  /**
   * Finds the dependencies for \a lib and inserts them into \a
   * dep_map.
   */
  void GatherDependenciesForVS6( const cmMakefile& mf,
                                 const LibraryID& lib,
                                 DependencyMap& dep_map);

  void AnalyzeLibDependenciesForVS6( const cmMakefile& mf );
#endif

  const char* GetSuffixVariableInternal(bool implib) const;
  const char* GetPrefixVariableInternal(bool implib) const;
  std::string GetFullNameInternal(const std::string& config,
                                  bool implib) const;
  void GetFullNameInternal(const std::string& config, bool implib,
                           std::string& outPrefix, std::string& outBase,
                           std::string& outSuffix) const;

  // Use a makefile variable to set a default for the given property.
  // If the variable is not defined use the given default instead.
  void SetPropertyDefault(const std::string& property,
                          const char* default_value);

  // Returns ARCHIVE, LIBRARY, or RUNTIME based on platform and type.
  const char* GetOutputTargetType(bool implib) const;

  // Get the target base name.
  std::string GetOutputName(const std::string& config, bool implib) const;

  std::string GetFullNameImported(const std::string& config,
                                  bool implib) const;

  std::string ImportedGetFullPath(const std::string& config,
                                  bool implib) const;
  std::string NormalGetFullPath(const std::string& config, bool implib,
                                bool realname) const;

  /** Get the real name of the target.  Allowed only for non-imported
      targets.  When a library or executable file is versioned this is
      the full versioned name.  If the target is not versioned this is
      the same as GetFullName.  */
  std::string NormalGetRealName(const std::string& config) const;

  /** Append to @a base the mac content directory and return it. */
  std::string BuildMacContentDirectory(const std::string& base,
                                       const std::string& config,
                                       bool contentOnly) const;

  void GetSourceFiles(std::vector<std::string> &files,
                      const std::string& config) const;
private:
  std::string Name;
  std::vector<cmCustomCommand> PreBuildCommands;
  std::vector<cmCustomCommand> PreLinkCommands;
  std::vector<cmCustomCommand> PostBuildCommands;
  TargetType TargetTypeValue;
  LinkLibraryVectorType PrevLinkedLibraries;
#if defined(_WIN32) && !defined(__CYGWIN__)
  LinkLibraryVectorType LinkLibrariesForVS6;
  bool LinkLibrariesForVS6Analyzed;
#endif
  std::vector<std::string> LinkDirectories;
  std::set<std::string> LinkDirectoriesEmmitted;
  bool HaveInstallRule;
  std::string InstallPath;
  std::string RuntimeInstallPath;
  mutable std::string ExportMacro;
  std::set<std::string> Utilities;
  std::map<std::string, cmListFileBacktrace> UtilityBacktraces;
  bool RecordDependencies;
  mutable cmPropertyMap Properties;
  LinkLibraryVectorType OriginalLinkLibraries;
  bool DLLPlatform;
  bool IsAndroid;
  bool IsApple;
  bool IsImportedTarget;
  mutable bool DebugIncludesDone;
  mutable std::map<std::string, bool> DebugCompatiblePropertiesDone;
  mutable bool DebugCompileOptionsDone;
  mutable bool DebugCompileDefinitionsDone;
  mutable bool DebugSourcesDone;
  mutable bool DebugCompileFeaturesDone;
  mutable std::set<std::string> LinkImplicitNullProperties;
  mutable std::map<std::string, std::string> MaxLanguageStandards;
  bool BuildInterfaceIncludesAppended;

  // Cache target output paths for each configuration.
  struct OutputInfo;
  OutputInfo const* GetOutputInfo(const std::string& config) const;
  bool
  ComputeOutputDir(const std::string& config,
                   bool implib, std::string& out) const;
  bool ComputePDBOutputDir(const std::string& kind, const std::string& config,
                           std::string& out) const;

  // Cache import information from properties for each configuration.
  struct ImportInfo;
  ImportInfo const* GetImportInfo(const std::string& config) const;
  void ComputeImportInfo(std::string const& desired_config,
                         ImportInfo& info) const;

  // Cache target compile paths for each configuration.
  struct CompileInfo;
  CompileInfo const* GetCompileInfo(const std::string& config) const;

  mutable cmTargetLinkInformationMap LinkInformation;
  void CheckPropertyCompatibility(cmComputeLinkInformation *info,
                                  const std::string& config) const;

  LinkInterface const*
    GetImportLinkInterface(const std::string& config, cmTarget const* head,
                           bool usage_requirements_only) const;

  LinkImplementationLibraries const*
    GetLinkImplementationLibrariesInternal(const std::string& config,
                                           cmTarget const* head) const;
  void ComputeLinkClosure(const std::string& config, LinkClosure& lc) const;

  void ExpandLinkItems(std::string const& prop, std::string const& value,
                       std::string const& config, cmTarget const* headTarget,
                       bool usage_requirements_only,
                       std::vector<cmLinkItem>& items,
                       bool& hadHeadSensitiveCondition) const;
  void LookupLinkItems(std::vector<std::string> const& names,
                       std::vector<cmLinkItem>& items) const;

  std::string ProcessSourceItemCMP0049(const std::string& s);

  void ClearLinkMaps();

  void MaybeInvalidatePropertyCache(const std::string& prop);

  void ProcessSourceExpression(std::string const& expr);

  // The cmMakefile instance that owns this target.  This should
  // always be set.
  cmMakefile* Makefile;

  // Policy status recorded when target was created.
#define TARGET_POLICY_MEMBER(POLICY) \
  cmPolicies::PolicyStatus PolicyStatus ## POLICY;

  CM_FOR_EACH_TARGET_POLICY(TARGET_POLICY_MEMBER)

#undef TARGET_POLICY_MEMBER

  // Internal representation details.
  friend class cmTargetInternals;
  friend class cmGeneratorTarget;
  friend class cmTargetTraceDependencies;
  cmTargetInternalPointer Internal;

  void ComputeVersionedName(std::string& vName,
                            std::string const& prefix,
                            std::string const& base,
                            std::string const& suffix,
                            std::string const& name,
                            const char* version) const;

  mutable bool LinkImplementationLanguageIsContextDependent;
};

#ifdef CMAKE_BUILD_WITH_CMAKE
#ifdef CMake_HAVE_CXX11_UNORDERED_MAP
typedef std::unordered_map<std::string, cmTarget> cmTargets;
#else
typedef cmsys::hash_map<std::string, cmTarget> cmTargets;
#endif
#else
typedef std::map<std::string,cmTarget> cmTargets;
#endif

class cmTargetSet: public std::set<std::string> {};
class cmTargetManifest: public std::map<std::string, cmTargetSet> {};

#endif
