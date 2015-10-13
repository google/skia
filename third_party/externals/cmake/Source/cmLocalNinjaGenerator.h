/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2011 Peter Collingbourne <peter@pcc.me.uk>
  Copyright 2011 Nicolas Despres <nicolas.despres@gmail.com>

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmLocalNinjaGenerator_h
#  define cmLocalNinjaGenerator_h

#  include "cmLocalGenerator.h"
#  include "cmNinjaTypes.h"

class cmCustomCommandGenerator;
class cmGlobalNinjaGenerator;
class cmGeneratedFileStream;
class cmake;

/**
 * \class cmLocalNinjaGenerator
 * \brief Write a local build.ninja file.
 *
 * cmLocalNinjaGenerator produces a local build.ninja file from its
 * member Makefile.
 */
class cmLocalNinjaGenerator : public cmLocalGenerator
{
public:
  cmLocalNinjaGenerator(cmGlobalGenerator* gg, cmLocalGenerator* parent,
                        cmState::Snapshot snapshot);

  virtual ~cmLocalNinjaGenerator();

  virtual void Generate();

  virtual void Configure();

  virtual std::string GetTargetDirectory(cmTarget const& target) const;

  const cmGlobalNinjaGenerator* GetGlobalNinjaGenerator() const;
  cmGlobalNinjaGenerator* GetGlobalNinjaGenerator();

  const cmake* GetCMakeInstance() const;
  cmake* GetCMakeInstance();

  std::string const& GetConfigName() const
  { return this->ConfigName; }

  /// @returns the relative path between the HomeOutputDirectory and this
  /// local generators StartOutputDirectory.
  std::string GetHomeRelativeOutputPath() const
  { return this->HomeRelativeOutputPath; }

  std::string ConvertToNinjaPath(const std::string& path);

  struct map_to_ninja_path {
    cmLocalNinjaGenerator *LocalGenerator;
    map_to_ninja_path(cmLocalNinjaGenerator *LocalGen)
      : LocalGenerator(LocalGen) {}
    std::string operator()(const std::string &path) {
      return LocalGenerator->ConvertToNinjaPath(path);
    }
  };

  map_to_ninja_path MapToNinjaPath() {
    return map_to_ninja_path(this);
  }

  void ExpandRuleVariables(std::string& string,
                           const RuleVariables& replaceValues) {
    cmLocalGenerator::ExpandRuleVariables(string, replaceValues);
  }

  std::string BuildCommandLine(const std::vector<std::string> &cmdLines);

  void AppendTargetOutputs(cmTarget* target, cmNinjaDeps& outputs);
  void AppendTargetDepends(cmTarget* target, cmNinjaDeps& outputs);

  void AddCustomCommandTarget(cmCustomCommand const* cc, cmTarget* target);
  void AppendCustomCommandLines(cmCustomCommandGenerator const& ccg,
                                std::vector<std::string> &cmdLines);
  void AppendCustomCommandDeps(cmCustomCommandGenerator const& ccg,
                               cmNinjaDeps &ninjaDeps);

  virtual std::string ConvertToLinkReference(std::string const& lib,
                                             OutputFormat format = SHELL);

  virtual void ComputeObjectFilenames(
                        std::map<cmSourceFile const*, std::string>& mapping,
                        cmGeneratorTarget const* gt = 0);


protected:
  virtual std::string ConvertToIncludeReference(std::string const& path,
                                                OutputFormat format = SHELL,
                                                bool forceFullPaths = false);


private:
  cmGeneratedFileStream& GetBuildFileStream() const;
  cmGeneratedFileStream& GetRulesFileStream() const;

  void WriteBuildFileTop();
  void WriteProjectHeader(std::ostream& os);
  void WriteNinjaRequiredVersion(std::ostream& os);
  void WriteNinjaFilesInclusion(std::ostream& os);
  void WriteProcessedMakefile(std::ostream& os);
  void WritePools(std::ostream& os);

  void SetConfigName();

  void WriteCustomCommandRule();
  void WriteCustomCommandBuildStatement(cmCustomCommand const *cc,
                                        const cmNinjaDeps& orderOnlyDeps);

  void WriteCustomCommandBuildStatements();

  std::string MakeCustomLauncher(cmCustomCommandGenerator const& ccg);

  std::string ConfigName;
  std::string HomeRelativeOutputPath;

  typedef std::map<cmCustomCommand const*, std::set<cmTarget*> >
    CustomCommandTargetMap;
  CustomCommandTargetMap CustomCommandTargets;
};

#endif // ! cmLocalNinjaGenerator_h
