/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2012 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmGeneratorTarget_h
#define cmGeneratorTarget_h

#include "cmStandardIncludes.h"

class cmCustomCommand;
class cmGlobalGenerator;
class cmLocalGenerator;
class cmMakefile;
class cmSourceFile;
class cmTarget;

class cmGeneratorTarget
{
public:
  cmGeneratorTarget(cmTarget*);

  int GetType() const;
  std::string GetName() const;
  const char *GetProperty(const std::string& prop) const;
  bool GetPropertyAsBool(const std::string& prop) const;
  void GetSourceFiles(std::vector<cmSourceFile*>& files,
                      const std::string& config) const;

  void GetObjectSources(std::vector<cmSourceFile const*> &,
                        const std::string& config) const;
  const std::string& GetObjectName(cmSourceFile const* file);

  bool HasExplicitObjectName(cmSourceFile const* file) const;
  void AddExplicitObjectName(cmSourceFile const* sf);

  void GetResxSources(std::vector<cmSourceFile const*>&,
                      const std::string& config) const;
  void GetIDLSources(std::vector<cmSourceFile const*>&,
                     const std::string& config) const;
  void GetExternalObjects(std::vector<cmSourceFile const*>&,
                          const std::string& config) const;
  void GetHeaderSources(std::vector<cmSourceFile const*>&,
                        const std::string& config) const;
  void GetExtraSources(std::vector<cmSourceFile const*>&,
                       const std::string& config) const;
  void GetCustomCommands(std::vector<cmSourceFile const*>&,
                         const std::string& config) const;
  void GetExpectedResxHeaders(std::set<std::string>&,
                              const std::string& config) const;
  void GetAppManifest(std::vector<cmSourceFile const*>&,
                      const std::string& config) const;
  void GetCertificates(std::vector<cmSourceFile const*>&,
                       const std::string& config) const;
  void GetXamlSources(std::vector<cmSourceFile const*>&,
                      const std::string& config) const;
  void GetExpectedXamlHeaders(std::set<std::string>&,
                              const std::string& config) const;
  void GetExpectedXamlSources(std::set<std::string>&,
                              const std::string& config) const;

  void ComputeObjectMapping();

  cmTarget* Target;
  cmMakefile* Makefile;
  cmLocalGenerator* LocalGenerator;
  cmGlobalGenerator const* GlobalGenerator;

  std::string GetModuleDefinitionFile(const std::string& config) const;

  /** Full path with trailing slash to the top-level directory
      holding object files for this target.  Includes the build
      time config name placeholder if needed for the generator.  */
  std::string ObjectDirectory;

  void UseObjectLibraries(std::vector<std::string>& objs,
                          const std::string& config) const;

  void GetAppleArchs(const std::string& config,
                     std::vector<std::string>& archVec) const;

  /** Return the rule variable used to create this type of target.  */
  std::string GetCreateRuleVariable(std::string const& lang,
                                    std::string const& config) const;

  /** Get the include directories for this target.  */
  std::vector<std::string> GetIncludeDirectories(
      const std::string& config, const std::string& lang) const;

  bool IsSystemIncludeDirectory(const std::string& dir,
                                const std::string& config) const;

  /** Add the target output files to the global generator manifest.  */
  void GenerateTargetManifest(const std::string& config) const;

  /**
   * Trace through the source files in this target and add al source files
   * that they depend on, used by all generators
   */
  void TraceDependencies();

  /** Get sources that must be built before the given source.  */
  std::vector<cmSourceFile*> const*
  GetSourceDepends(cmSourceFile const* sf) const;

  /**
   * Flags for a given source file as used in this target. Typically assigned
   * via SET_TARGET_PROPERTIES when the property is a list of source files.
   */
  enum SourceFileType
  {
    SourceFileTypeNormal,
    SourceFileTypePrivateHeader, // is in "PRIVATE_HEADER" target property
    SourceFileTypePublicHeader,  // is in "PUBLIC_HEADER" target property
    SourceFileTypeResource,      // is in "RESOURCE" target property *or*
                                 // has MACOSX_PACKAGE_LOCATION=="Resources"
    SourceFileTypeMacContent     // has MACOSX_PACKAGE_LOCATION!="Resources"
  };
  struct SourceFileFlags
  {
    SourceFileFlags(): Type(SourceFileTypeNormal), MacFolder(0) {}
    SourceFileFlags(SourceFileFlags const& r):
      Type(r.Type), MacFolder(r.MacFolder) {}
    SourceFileType Type;
    const char* MacFolder; // location inside Mac content folders
  };

  struct SourceFileFlags
  GetTargetSourceFileFlags(const cmSourceFile* sf) const;

  struct ResxData {
    mutable std::set<std::string> ExpectedResxHeaders;
    mutable std::vector<cmSourceFile const*> ResxSources;
  };

  struct XamlData {
    std::set<std::string> ExpectedXamlHeaders;
    std::set<std::string> ExpectedXamlSources;
    std::vector<cmSourceFile const*> XamlSources;
  };

private:
  friend class cmTargetTraceDependencies;
  struct SourceEntry { std::vector<cmSourceFile*> Depends; };
  typedef std::map<cmSourceFile const*, SourceEntry> SourceEntriesType;
  SourceEntriesType SourceEntries;

  mutable std::map<cmSourceFile const*, std::string> Objects;
  std::set<cmSourceFile const*> ExplicitObjectName;
  mutable std::map<std::string, std::vector<std::string> > SystemIncludesCache;

  void ConstructSourceFileFlags() const;
  mutable bool SourceFileFlagsConstructed;
  mutable std::map<cmSourceFile const*, SourceFileFlags> SourceFlagsMap;

  cmGeneratorTarget(cmGeneratorTarget const&);
  void operator=(cmGeneratorTarget const&);
};

struct cmStrictTargetComparison {
  bool operator()(cmTarget const* t1, cmTarget const* t2) const;
};

typedef std::map<cmTarget const*,
                 cmGeneratorTarget*,
                 cmStrictTargetComparison> cmGeneratorTargetsType;

#endif
