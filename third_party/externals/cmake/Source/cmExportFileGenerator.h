/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmExportFileGenerator_h
#define cmExportFileGenerator_h

#include "cmCommand.h"
#include "cmGeneratorExpression.h"

#include "cmVersionMacros.h"
#include "cmVersion.h"

#define STRINGIFY_HELPER(X) #X
#define STRINGIFY(X) STRINGIFY_HELPER(X)

#define DEVEL_CMAKE_VERSION(major, minor) ( \
  CMake_VERSION_ENCODE(major, minor, 0) > \
  CMake_VERSION_ENCODE(CMake_VERSION_MAJOR, CMake_VERSION_MINOR, 0) ?  \
    STRINGIFY(CMake_VERSION_MAJOR) "." STRINGIFY(CMake_VERSION_MINOR) "." \
    STRINGIFY(CMake_VERSION_PATCH) \
  : #major "." #minor ".0" \
  )

class cmTargetExport;

/** \class cmExportFileGenerator
 * \brief Generate a file exporting targets from a build or install tree.
 *
 * cmExportFileGenerator is the superclass for
 * cmExportBuildFileGenerator and cmExportInstallFileGenerator.  It
 * contains common code generation routines for the two kinds of
 * export implementations.
 */
class cmExportFileGenerator
{
public:
  cmExportFileGenerator();
  virtual ~cmExportFileGenerator() {}

  /** Set the full path to the export file to generate.  */
  void SetExportFile(const char* mainFile);
  const char *GetMainExportFileName() const;

  /** Set the namespace in which to place exported target names.  */
  void SetNamespace(const std::string& ns) { this->Namespace = ns; }
  std::string GetNamespace() const { return this->Namespace; }

  void SetExportOld(bool exportOld) { this->ExportOld = exportOld; }

  /** Add a configuration to be exported.  */
  void AddConfiguration(const std::string& config);

  /** Actually generate the export file.  Returns whether there was an
      error.  */
  bool GenerateImportFile();
protected:

  typedef std::map<std::string, std::string> ImportPropertyMap;

  // Generate per-configuration target information to the given output
  // stream.
  void GenerateImportConfig(std::ostream& os, const std::string& config,
                            std::vector<std::string> &missingTargets);

  // Methods to implement export file code generation.
  void GenerateImportHeaderCode(std::ostream& os,
                                const std::string& config = "");
  void GenerateImportFooterCode(std::ostream& os);
  void GenerateImportVersionCode(std::ostream& os);
  void GenerateImportTargetCode(std::ostream& os, cmTarget const* target);
  void GenerateImportPropertyCode(std::ostream& os, const std::string& config,
                                  cmTarget const* target,
                                  ImportPropertyMap const& properties);
  void GenerateImportedFileChecksCode(std::ostream& os, cmTarget* target,
                                      ImportPropertyMap const& properties,
                               const std::set<std::string>& importedLocations);
  void GenerateImportedFileCheckLoop(std::ostream& os);
  void GenerateMissingTargetsCheckCode(std::ostream& os,
                               const std::vector<std::string>& missingTargets);

  void GenerateExpectedTargetsCode(std::ostream& os,
                                          const std::string &expectedTargets);

  // Collect properties with detailed information about targets beyond
  // their location on disk.
  void SetImportDetailProperties(const std::string& config,
                                 std::string const& suffix, cmTarget* target,
                                 ImportPropertyMap& properties,
                                 std::vector<std::string>& missingTargets);

  template <typename T>
  void SetImportLinkProperty(std::string const& suffix,
                             cmTarget* target, const std::string& propName,
                             std::vector<T> const& entries,
                             ImportPropertyMap& properties,
                             std::vector<std::string>& missingTargets);

  /** Each subclass knows how to generate its kind of export file.  */
  virtual bool GenerateMainFile(std::ostream& os) = 0;

  /** Each subclass knows where the target files are located.  */
  virtual void GenerateImportTargetsConfig(std::ostream& os,
                                           const std::string& config,
                                           std::string const& suffix,
                            std::vector<std::string> &missingTargets) = 0;

  /** Each subclass knows how to deal with a target that is  missing from an
   *  export set.  */
  virtual void HandleMissingTarget(std::string& link_libs,
                                   std::vector<std::string>& missingTargets,
                                   cmMakefile* mf,
                                   cmTarget* depender,
                                   cmTarget* dependee) = 0;
  void PopulateInterfaceProperty(const std::string&,
                                 cmTarget *target,
                                 cmGeneratorExpression::PreprocessContext,
                                 ImportPropertyMap &properties,
                                 std::vector<std::string> &missingTargets);
  bool PopulateInterfaceLinkLibrariesProperty(cmTarget *target,
                                 cmGeneratorExpression::PreprocessContext,
                                 ImportPropertyMap &properties,
                                 std::vector<std::string> &missingTargets);
  void PopulateInterfaceProperty(const std::string& propName, cmTarget *target,
                                 ImportPropertyMap &properties);
  void PopulateCompatibleInterfaceProperties(cmTarget *target,
                                 ImportPropertyMap &properties);
  void GenerateInterfaceProperties(cmTarget const* target, std::ostream& os,
                                   const ImportPropertyMap &properties);
  void PopulateIncludeDirectoriesInterface(
                      cmTargetExport *target,
                      cmGeneratorExpression::PreprocessContext preprocessRule,
                      ImportPropertyMap &properties,
                      std::vector<std::string> &missingTargets);
  void PopulateSourcesInterface(
                      cmTargetExport *target,
                      cmGeneratorExpression::PreprocessContext preprocessRule,
                      ImportPropertyMap &properties,
                      std::vector<std::string> &missingTargets);

  void SetImportLinkInterface(const std::string& config,
                    std::string const& suffix,
                    cmGeneratorExpression::PreprocessContext preprocessRule,
                    cmTarget* target, ImportPropertyMap& properties,
                    std::vector<std::string>& missingTargets);

  enum FreeTargetsReplace {
    ReplaceFreeTargets,
    NoReplaceFreeTargets
  };

  void ResolveTargetsInGeneratorExpressions(std::string &input,
                          cmTarget* target,
                          std::vector<std::string> &missingTargets,
                          FreeTargetsReplace replace = NoReplaceFreeTargets);

  void GenerateRequiredCMakeVersion(std::ostream& os,
                                    const char *versionString);

  // The namespace in which the exports are placed in the generated file.
  std::string Namespace;

  bool ExportOld;

  // The set of configurations to export.
  std::vector<std::string> Configurations;

  // The file to generate.
  std::string MainImportFile;
  std::string FileDir;
  std::string FileBase;
  std::string FileExt;
  bool AppendMode;

  // The set of targets included in the export.
  std::set<cmTarget*> ExportedTargets;

private:
  void PopulateInterfaceProperty(const std::string&, const std::string&,
                                 cmTarget *target,
                                 cmGeneratorExpression::PreprocessContext,
                                 ImportPropertyMap &properties,
                                 std::vector<std::string> &missingTargets);

  bool AddTargetNamespace(std::string &input, cmTarget* target,
                          std::vector<std::string> &missingTargets);

  void ResolveTargetsInGeneratorExpression(std::string &input,
                                    cmTarget* target,
                                    std::vector<std::string> &missingTargets);

  virtual void ReplaceInstallPrefix(std::string &input);

  virtual std::string InstallNameDir(cmTarget* target,
                                     const std::string& config) = 0;
};

#endif
