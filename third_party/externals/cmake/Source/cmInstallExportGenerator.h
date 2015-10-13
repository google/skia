/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmInstallExportGenerator_h
#define cmInstallExportGenerator_h

#include "cmInstallGenerator.h"

class cmExportInstallFileGenerator;
class cmInstallFilesGenerator;
class cmInstallTargetGenerator;
class cmExportSet;
class cmMakefile;

/** \class cmInstallExportGenerator
 * \brief Generate rules for creating an export files.
 */
class cmInstallExportGenerator: public cmInstallGenerator
{
public:
  cmInstallExportGenerator(cmExportSet* exportSet,
                           const char* dest, const char* file_permissions,
                           const std::vector<std::string>& configurations,
                           const char* component,
                           MessageLevel message,
                           const char* filename, const char* name_space,
                           bool exportOld, cmMakefile* mf);
  ~cmInstallExportGenerator();

  cmExportSet* GetExportSet() {return this->ExportSet;}

  cmMakefile* GetMakefile() const { return this->Makefile; }

  const std::string& GetNamespace() const { return this->Namespace; }

  std::string const& GetDestination() const
    { return this->Destination; }

protected:
  virtual void GenerateScript(std::ostream& os);
  virtual void GenerateScriptConfigs(std::ostream& os, Indent const& indent);
  virtual void GenerateScriptActions(std::ostream& os, Indent const& indent);
  void GenerateImportFile(cmExportSet const* exportSet);
  void GenerateImportFile(const char* config, cmExportSet const* exportSet);
  void ComputeTempDir();

  cmExportSet* ExportSet;
  std::string FilePermissions;
  std::string FileName;
  std::string Namespace;
  bool ExportOld;
  cmMakefile* Makefile;

  std::string TempDir;
  std::string MainImportFile;
  cmExportInstallFileGenerator* EFGen;
};

#endif
