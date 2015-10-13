/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmExportInstallFileGenerator_h
#define cmExportInstallFileGenerator_h

#include "cmExportFileGenerator.h"

class cmInstallExportGenerator;
class cmInstallTargetGenerator;

/** \class cmExportInstallFileGenerator
 * \brief Generate a file exporting targets from an install tree.
 *
 * cmExportInstallFileGenerator generates files exporting targets from
 * install an installation tree.  The files are placed in a temporary
 * location for installation by cmInstallExportGenerator.  One main
 * file is generated that creates the imported targets and loads
 * per-configuration files.  Target locations and settings for each
 * configuration are written to these per-configuration files.  After
 * installation the main file loads the configurations that have been
 * installed.
 *
 * This is used to implement the INSTALL(EXPORT) command.
 */
class cmExportInstallFileGenerator: public cmExportFileGenerator
{
public:
  /** Construct with the export installer that will install the
      files.  */
  cmExportInstallFileGenerator(cmInstallExportGenerator* iegen);

  /** Get the per-config file generated for each configuraiton.  This
      maps from the configuration name to the file temporary location
      for installation.  */
  std::map<std::string, std::string> const& GetConfigImportFiles()
    { return this->ConfigImportFiles; }

  /** Compute the globbing expression used to load per-config import
      files from the main file.  */
  std::string GetConfigImportFileGlob();
protected:

  // Implement virtual methods from the superclass.
  virtual bool GenerateMainFile(std::ostream& os);
  virtual void GenerateImportTargetsConfig(std::ostream& os,
                                           const std::string& config,
                                           std::string const& suffix,
                            std::vector<std::string> &missingTargets);
  virtual void HandleMissingTarget(std::string& link_libs,
                                   std::vector<std::string>& missingTargets,
                                   cmMakefile* mf,
                                   cmTarget* depender,
                                   cmTarget* dependee);

  virtual void ReplaceInstallPrefix(std::string &input);

  void ComplainAboutMissingTarget(cmTarget* depender,
                                  cmTarget* dependee,
                                  int occurrences);

  std::vector<std::string> FindNamespaces(cmMakefile* mf,
                                          const std::string& name);


  /** Generate a per-configuration file for the targets.  */
  bool GenerateImportFileConfig(const std::string& config,
                            std::vector<std::string> &missingTargets);

  /** Fill in properties indicating installed file locations.  */
  void SetImportLocationProperty(const std::string& config,
                                 std::string const& suffix,
                                 cmInstallTargetGenerator* itgen,
                                 ImportPropertyMap& properties,
                                 std::set<std::string>& importedLocations
                                );

  std::string InstallNameDir(cmTarget* target, const std::string& config);

  cmInstallExportGenerator* IEGen;

  // The import file generated for each configuration.
  std::map<std::string, std::string> ConfigImportFiles;
};

#endif
