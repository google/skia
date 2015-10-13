/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2004-2009 Kitware, Inc.
  Copyright 2013 Alexander Neundorf (neundorf@kde.org)

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmExtraKateGenerator_h
#define cmExtraKateGenerator_h

#include "cmExternalMakefileProjectGenerator.h"

class cmLocalGenerator;
class cmMakefile;
class cmTarget;
class cmGeneratedFileStream;

/** \class cmExtraKateGenerator
 * \brief Write Kate project files for Makefile or ninja based projects
 */
class cmExtraKateGenerator : public cmExternalMakefileProjectGenerator
{
public:
  cmExtraKateGenerator();

  virtual std::string GetName() const
                         { return cmExtraKateGenerator::GetActualName();}
  static std::string GetActualName()                    { return "Kate";}
  static cmExternalMakefileProjectGenerator* New()
                                     { return new cmExtraKateGenerator; }
  /** Get the documentation entry for this generator.  */
  virtual void GetDocumentation(cmDocumentationEntry& entry,
                                const std::string& fullName) const;

  virtual void Generate();
private:
  void CreateKateProjectFile(const cmMakefile* mf) const;
  void CreateDummyKateProjectFile(const cmMakefile* mf) const;
  void WriteTargets(const cmMakefile* mf, cmGeneratedFileStream& fout) const;
  void AppendTarget(cmGeneratedFileStream& fout,
                    const std::string&     target,
                    const std::string&     make,
                    const std::string&     makeArgs,
                    const std::string&     path,
                    const char*            homeOutputDir) const;

  std::string GenerateFilesString(const cmMakefile* mf) const;
  std::string GetPathBasename(const std::string& path) const;
  std::string GenerateProjectName(const std::string& name,
                                  const std::string& type,
                                  const std::string& path) const;

  std::string ProjectName;
  bool UseNinja;
};

#endif
