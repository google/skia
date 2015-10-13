/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2004-2009 Kitware, Inc.
  Copyright 2004 Alexander Neundorf (neundorf@kde.org)

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmExtraSublimeTextGenerator_h
#define cmExtraSublimeTextGenerator_h

#include "cmExternalMakefileProjectGenerator.h"
#include "cmSourceFile.h"

class cmLocalGenerator;
class cmMakefile;
class cmTarget;
class cmGeneratedFileStream;
class cmGeneratorTarget;

/** \class cmExtraSublimeTextGenerator
 * \brief Write Sublime Text 2 project files for Makefile based projects
 */
class cmExtraSublimeTextGenerator : public cmExternalMakefileProjectGenerator
{
public:
  typedef std::map<std::string, std::vector<std::string> > MapSourceFileFlags;
  cmExtraSublimeTextGenerator();

  virtual std::string GetName() const
                        { return cmExtraSublimeTextGenerator::GetActualName();}
  static std::string GetActualName()
                        { return "Sublime Text 2";}
  static cmExternalMakefileProjectGenerator* New()
                                    { return new cmExtraSublimeTextGenerator; }
  /** Get the documentation entry for this generator.  */
  virtual void GetDocumentation(cmDocumentationEntry& entry,
                                const std::string& fullName) const;

  virtual void Generate();
private:

  void CreateProjectFile(const std::vector<cmLocalGenerator*>& lgs);

  void CreateNewProjectFile(const std::vector<cmLocalGenerator*>& lgs,
                                const std::string& filename);

  /** Appends all targets as build systems to the project file and get all
   * include directories and compiler definitions used.
   */
  void AppendAllTargets(const std::vector<cmLocalGenerator*>& lgs,
                        const cmMakefile* mf,
                        cmGeneratedFileStream& fout,
                        MapSourceFileFlags& sourceFileFlags);
  /** Returns the build command that needs to be executed to build the
   *  specified target.
   */
  std::string BuildMakeCommand(const std::string& make, const char* makefile,
                               const std::string& target);
  /** Appends the specified target to the generated project file as a Sublime
   *  Text build system.
   */
  void AppendTarget(cmGeneratedFileStream& fout,
                    const std::string& targetName,
                    cmLocalGenerator* lg,
                    cmTarget* target,
                    const char* make,
                    const cmMakefile* makefile,
                    const char* compiler,
                    MapSourceFileFlags& sourceFileFlags, bool firstTarget);
  /**
   * Compute the flags for compilation of object files for a given @a language.
   * @note Generally it is the value of the variable whose name is computed
   *       by LanguageFlagsVarName().
   */
  std::string ComputeFlagsForObject(cmSourceFile *source,
                                    cmLocalGenerator* lg,
                                    cmTarget *target,
                                    cmGeneratorTarget* gtgt);

  std::string ComputeDefines(cmSourceFile *source, cmLocalGenerator* lg,
                             cmTarget *target, cmGeneratorTarget* gtgt);
};

#endif
