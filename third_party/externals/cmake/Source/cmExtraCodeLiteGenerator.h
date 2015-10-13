/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2004-2009 Kitware, Inc.
  Copyright 2004 Alexander Neundorf (neundorf@kde.org)
  Copyright 2013 Eran Ifrah (eran.ifrah@gmail.com)

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmGlobalCodeLiteGenerator_h
#define cmGlobalCodeLiteGenerator_h

#include "cmExternalMakefileProjectGenerator.h"

class cmLocalGenerator;

class cmExtraCodeLiteGenerator : public cmExternalMakefileProjectGenerator
{
protected:
  std::string ConfigName;
  std::string WorkspacePath;
  unsigned int CpuCount;

protected:
  std::string GetCodeLiteCompilerName(const cmMakefile* mf) const;
  std::string GetConfigurationName( const cmMakefile* mf ) const;
  std::string GetBuildCommand(const cmMakefile* mf) const;
  std::string GetCleanCommand(const cmMakefile* mf) const;
  std::string GetRebuildCommand(const cmMakefile* mf) const;
  std::string GetSingleFileBuildCommand(const cmMakefile* mf) const;
public:
  cmExtraCodeLiteGenerator();

  virtual std::string GetName() const
                          { return cmExtraCodeLiteGenerator::GetActualName();}
  static std::string GetActualName()                     { return "CodeLite";}
  static cmExternalMakefileProjectGenerator* New()
                                      { return new cmExtraCodeLiteGenerator; }
  /** Get the documentation entry for this generator.  */
  virtual void GetDocumentation(cmDocumentationEntry& entry,
                                const std::string& fullName) const;

  virtual void Generate();
  void CreateProjectFile(const std::vector<cmLocalGenerator*>& lgs);

  void CreateNewProjectFile(const std::vector<cmLocalGenerator*>& lgs,
                                const std::string& filename);
};

#endif
