/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2013 Stephen Kelly <steveire@gmail.com>

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

class cmExportTryCompileFileGenerator: public cmExportFileGenerator
{
public:
  /** Set the list of targets to export.  */
  void SetExports(const std::vector<cmTarget const*> &exports)
    { this->Exports = exports; }
  void SetConfig(const std::string& config) { this->Config = config; }
protected:

  // Implement virtual methods from the superclass.
  virtual bool GenerateMainFile(std::ostream& os);

  virtual void GenerateImportTargetsConfig(std::ostream&,
                                           const std::string&,
                                           std::string const&,
                            std::vector<std::string>&) {}
  virtual void HandleMissingTarget(std::string&,
                                   std::vector<std::string>&,
                                   cmMakefile*,
                                   cmTarget*,
                                   cmTarget*) {}

  void PopulateProperties(cmTarget const* target,
                          ImportPropertyMap& properties,
                          std::set<cmTarget const*> &emitted);

  std::string InstallNameDir(cmTarget* target,
                             const std::string& config);
private:
  std::string FindTargets(const std::string& prop, cmTarget const* tgt,
                   std::set<cmTarget const*> &emitted);


  std::vector<cmTarget const*> Exports;
  std::string Config;
};

#endif
