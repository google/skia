/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2015 Geoffrey Viola <geoffrey.viola@asirobots.com>

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmGhsMultiTargetGenerator_h
#define cmGhsMultiTargetGenerator_h

#include "cmStandardIncludes.h"
#include "cmTarget.h"
#include "cmGhsMultiGpj.h"

class cmGeneratedFileStream;
class cmGlobalGhsMultiGenerator;
class cmLocalGhsMultiGenerator;
class cmMakefile;
class cmSourceFile;
class cmGeneratedFileStream;
class cmCustomCommand;

class cmGhsMultiTargetGenerator
{
public:
  cmGhsMultiTargetGenerator(cmTarget *target);

  virtual ~cmGhsMultiTargetGenerator();

  virtual void Generate();

  bool IncludeThisTarget();
  std::vector<cmSourceFile *> GetSources() const;
  GhsMultiGpj::Types GetGpjTag() const;
  static GhsMultiGpj::Types GetGpjTag(const cmTarget *target);
  const char *GetAbsBuildFilePath() const
  {
    return this->AbsBuildFilePath.c_str();
  }
  const char *GetRelBuildFileName() const
  {
    return this->RelBuildFileName.c_str();
  }
  const char *GetAbsBuildFileName() const
  {
    return this->AbsBuildFileName.c_str();
  }
  const char *GetAbsOutputFileName() const
  {
    return this->AbsOutputFileName.c_str();
  }

  static std::string GetRelBuildFilePath(const cmTarget *target);
  static std::string GetAbsPathToRoot(const cmTarget *target);
  static std::string GetAbsBuildFilePath(const cmTarget *target);
  static std::string GetRelBuildFileName(const cmTarget *target);
  static std::string GetBuildFileName(const cmTarget *target);
  static std::string AddSlashIfNeededToPath(std::string const &input);

private:
  cmGlobalGhsMultiGenerator *GetGlobalGenerator() const;
  cmGeneratedFileStream *GetFolderBuildStreams()
  {
    return this->FolderBuildStreams[""];
  };
  bool IsTargetGroup() const { return this->TargetGroup; }

  void WriteTypeSpecifics(const std::string &config, bool notKernel);
  void WriteCompilerFlags(const std::string &config,
    const std::string &language);
  void WriteCompilerDefinitions(const std::string &config,
                                const std::string &language);

  void SetCompilerFlags(std::string const &config, const std::string &language,
                        bool const notKernel);
  std::string GetDefines(const std::string &langugae,
                         std::string const &config);

  void WriteIncludes(const std::string &config, const std::string &language);
  void WriteTargetLinkLibraries();
  void WriteCustomCommands();
  void
  WriteCustomCommandsHelper(std::vector<cmCustomCommand> const &commandsSet,
                            cmTarget::CustomCommandType commandType);
  void WriteSources(std::vector<cmSourceFile *> const &objectSources);
  static void WriteObjectLangOverride(cmGeneratedFileStream *fileStream,
                                      cmSourceFile *sourceFile);
  static void WriteObjectDir(cmGeneratedFileStream *fileStream,
                             std::string const &dir);
  std::string GetOutputDirectory(const std::string &config) const;
  std::string GetOutputFilename(const std::string &config) const;

  bool IsNotKernel(std::string const &config, const std::string &language);
  static bool DetermineIfTargetGroup(const cmTarget *target);
  bool DetermineIfDynamicDownload(std::string const &config,
                                  const std::string &language);

  cmTarget *Target;
  cmLocalGhsMultiGenerator *LocalGenerator;
  cmMakefile *Makefile;
  std::string AbsBuildFilePath;
  std::string RelBuildFilePath;
  std::string AbsBuildFileName;
  std::string RelBuildFileName;
  std::string RelOutputFileName;
  std::string AbsOutputFileName;
  std::map<std::string, cmGeneratedFileStream *> FolderBuildStreams;
  bool TargetGroup;
  bool DynamicDownload;
  static std::string const DDOption;
  std::map<std::string, std::string> FlagsByLanguage;
  std::map<std::string, std::string> DefinesByLanguage;
};

#endif // ! cmGhsMultiTargetGenerator_h
