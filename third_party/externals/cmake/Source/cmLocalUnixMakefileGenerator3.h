/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmLocalUnixMakefileGenerator3_h
#define cmLocalUnixMakefileGenerator3_h

#include "cmLocalGenerator.h"

// for cmDepends::DependencyVector
#include "cmDepends.h"

class cmCustomCommand;
class cmCustomCommandGenerator;
class cmDependInformation;
class cmDepends;
class cmMakefileTargetGenerator;
class cmTarget;
class cmSourceFile;

/** \class cmLocalUnixMakefileGenerator3
 * \brief Write a LocalUnix makefiles.
 *
 * cmLocalUnixMakefileGenerator3 produces a LocalUnix makefile from its
 * member Makefile.
 */
class cmLocalUnixMakefileGenerator3 : public cmLocalGenerator
{
public:
  cmLocalUnixMakefileGenerator3(cmGlobalGenerator* gg,
                                cmLocalGenerator* parent,
                                cmState::Snapshot snapshot);
  virtual ~cmLocalUnixMakefileGenerator3();

  /**
   * Process the CMakeLists files for this directory to fill in the
   * Makefile ivar
   */
  virtual void Configure();

  /**
   * Generate the makefile for this directory.
   */
  virtual void Generate();


  // this returns the relative path between the HomeOutputDirectory and this
  // local generators StartOutputDirectory
  const std::string &GetHomeRelativeOutputPath();

  // Write out a make rule
  void WriteMakeRule(std::ostream& os,
                     const char* comment,
                     const std::string& target,
                     const std::vector<std::string>& depends,
                     const std::vector<std::string>& commands,
                     bool symbolic,
                     bool in_help = false);

  // write the main variables used by the makefiles
  void WriteMakeVariables(std::ostream& makefileStream);

  /**
   * Set max makefile variable size, default is 0 which means unlimited.
   */
  void SetMakefileVariableSize(int s) { this->MakefileVariableSize = s; }

  /**
   * Set whether passing a make target on a command line requires an
   * extra level of escapes.
   */
  void SetMakeCommandEscapeTargetTwice(bool b)
    { this->MakeCommandEscapeTargetTwice = b; }

  /**
   * Set whether the Borland curly brace command line hack should be
   * applied.
   */
  void SetBorlandMakeCurlyHack(bool b)
    { this->BorlandMakeCurlyHack = b; }

  // used in writing out Cmake files such as WriteDirectoryInformation
  static void WriteCMakeArgument(std::ostream& os, const char* s);

  /** creates the common disclaimer text at the top of each makefile */
  void WriteDisclaimer(std::ostream& os);

  // write a  comment line #====... in the stream
  void WriteDivider(std::ostream& os);

  /** used to create a recursive make call */
  std::string GetRecursiveMakeCall(const char *makefile,
                                   const std::string& tgt);

  // append flags to a string
  virtual void AppendFlags(std::string& flags, const std::string& newFlags);
  virtual void AppendFlags(std::string& flags, const char* newFlags);

  // append an echo command
  enum EchoColor { EchoNormal, EchoDepend, EchoBuild, EchoLink,
                   EchoGenerate, EchoGlobal };
  struct EchoProgress { std::string Dir; std::string Arg; };
  void AppendEcho(std::vector<std::string>& commands, std::string const& text,
                  EchoColor color = EchoNormal, EchoProgress const* = 0);

  /** Get whether the makefile is to have color.  */
  bool GetColorMakefile() const { return this->ColorMakefile; }

  virtual std::string GetTargetDirectory(cmTarget const& target) const;

    // create a command that cds to the start dir then runs the commands
  void CreateCDCommand(std::vector<std::string>& commands,
                       const char *targetDir,
                       cmLocalGenerator::RelativeRoot returnDir);

  static std::string ConvertToQuotedOutputPath(const char* p,
                                               bool useWatcomQuote);

  std::string CreateMakeVariable(const std::string& sin,
                                 const std::string& s2in);

  /** Called from command-line hook to bring dependencies up to date
      for a target.  */
  virtual bool UpdateDependencies(const char* tgtInfo,
                                  bool verbose, bool color);

  /** Called from command-line hook to clear dependencies.  */
  virtual void ClearDependencies(cmMakefile* mf, bool verbose);

  /** write some extra rules such as make test etc */
  void WriteSpecialTargetsTop(std::ostream& makefileStream);
  void WriteSpecialTargetsBottom(std::ostream& makefileStream);

  std::string GetRelativeTargetDirectory(cmTarget const& target);

  // File pairs for implicit dependency scanning.  The key of the map
  // is the depender and the value is the explicit dependee.
  struct ImplicitDependFileMap:
    public std::map<std::string, cmDepends::DependencyVector> {};
  struct ImplicitDependLanguageMap:
    public std::map<std::string, ImplicitDependFileMap> {};
  struct ImplicitDependTargetMap:
    public std::map<std::string, ImplicitDependLanguageMap> {};
  ImplicitDependLanguageMap const& GetImplicitDepends(cmTarget const& tgt);

  void AddImplicitDepends(cmTarget const& tgt, const std::string& lang,
                          const char* obj, const char* src);

  void AppendGlobalTargetDepends(std::vector<std::string>& depends,
                                 cmTarget& target);

  // write the target rules for the local Makefile into the stream
  void WriteLocalAllRules(std::ostream& ruleFileStream);

  std::vector<std::string> const& GetLocalHelp() { return this->LocalHelp; }

  /** Get whether to create rules to generate preprocessed and
      assembly sources.  This could be converted to a variable lookup
      later.  */
  bool GetCreatePreprocessedSourceRules()
    {
    return !this->SkipPreprocessedSourceRules;
    }
  bool GetCreateAssemblySourceRules()
    {
    return !this->SkipAssemblySourceRules;
    }

  // Fill the vector with the target names for the object files,
  // preprocessed files and assembly files. Currently only used by the
  // Eclipse generator.
  void GetIndividualFileTargets(std::vector<std::string>& targets);

protected:
  void WriteLocalMakefile();


  // write the target rules for the local Makefile into the stream
  void WriteLocalMakefileTargets(std::ostream& ruleFileStream,
                                 std::set<std::string> &emitted);

  // this method Writes the Directory information files
  void WriteDirectoryInformationFile();


  // write the depend info
  void WriteDependLanguageInfo(std::ostream& cmakefileStream, cmTarget &tgt);

  // write the local help rule
  void WriteHelpRule(std::ostream& ruleFileStream);

  // this converts a file name that is relative to the StartOuputDirectory
  // into a full path
  std::string ConvertToFullPath(const std::string& localPath);


  void WriteConvenienceRule(std::ostream& ruleFileStream,
                            const std::string& realTarget,
                            const std::string& helpTarget);

  void WriteTargetDependRule(std::ostream& ruleFileStream,
                             cmTarget& target);
  void WriteTargetCleanRule(std::ostream& ruleFileStream,
                            cmTarget& target,
                            const std::vector<std::string>& files);
  void WriteTargetRequiresRule(std::ostream& ruleFileStream,
                               cmTarget& target,
                               const std::vector<std::string>& objects);

  void AppendRuleDepend(std::vector<std::string>& depends,
                        const char* ruleFileName);
  void AppendRuleDepends(std::vector<std::string>& depends,
                         std::vector<std::string> const& ruleFiles);
  void AppendCustomDepends(std::vector<std::string>& depends,
                           const std::vector<cmCustomCommand>& ccs);
  void AppendCustomDepend(std::vector<std::string>& depends,
                          cmCustomCommandGenerator const& cc);
  void AppendCustomCommands(std::vector<std::string>& commands,
                            const std::vector<cmCustomCommand>& ccs,
                            cmTarget* target,
                            cmLocalGenerator::RelativeRoot relative =
                            cmLocalGenerator::HOME_OUTPUT);
  void AppendCustomCommand(std::vector<std::string>& commands,
                           cmCustomCommandGenerator const& ccg,
                           cmTarget* target,
                           bool echo_comment=false,
                           cmLocalGenerator::RelativeRoot relative =
                           cmLocalGenerator::HOME_OUTPUT,
                           std::ostream* content = 0);
  void AppendCleanCommand(std::vector<std::string>& commands,
                          const std::vector<std::string>& files,
                          cmTarget& target, const char* filename =0);

  // Helper methods for dependeny updates.
  bool ScanDependencies(const char* targetDir,
                std::map<std::string, cmDepends::DependencyVector>& validDeps);
  void CheckMultipleOutputs(bool verbose);

private:
  std::string ConvertShellCommand(std::string const& cmd, RelativeRoot root);
  std::string MakeLauncher(cmCustomCommandGenerator const& ccg,
                           cmTarget* target, RelativeRoot relative);

  virtual void ComputeObjectFilenames(
                        std::map<cmSourceFile const*, std::string>& mapping,
                        cmGeneratorTarget const* gt = 0);

  friend class cmMakefileTargetGenerator;
  friend class cmMakefileExecutableTargetGenerator;
  friend class cmMakefileLibraryTargetGenerator;
  friend class cmMakefileUtilityTargetGenerator;
  friend class cmGlobalUnixMakefileGenerator3;

  ImplicitDependTargetMap ImplicitDepends;

  //==========================================================================
  // Configuration settings.
  int MakefileVariableSize;
  std::string ConfigurationName;
  bool MakeCommandEscapeTargetTwice;
  bool BorlandMakeCurlyHack;
  //==========================================================================

  std::string HomeRelativeOutputPath;

  /* Copy the setting of CMAKE_COLOR_MAKEFILE from the makefile at the
     beginning of generation to avoid many duplicate lookups.  */
  bool ColorMakefile;

  /* Copy the setting of CMAKE_SKIP_PREPROCESSED_SOURCE_RULES and
     CMAKE_SKIP_ASSEMBLY_SOURCE_RULES at the beginning of generation to
     avoid many duplicate lookups.  */
  bool SkipPreprocessedSourceRules;
  bool SkipAssemblySourceRules;

  struct LocalObjectEntry
  {
    cmTarget* Target;
    std::string Language;
    LocalObjectEntry(): Target(0), Language() {}
    LocalObjectEntry(cmTarget* t, const std::string& lang):
      Target(t), Language(lang) {}
  };
  struct LocalObjectInfo: public std::vector<LocalObjectEntry>
  {
    bool HasSourceExtension;
    bool HasPreprocessRule;
    bool HasAssembleRule;
    LocalObjectInfo():HasSourceExtension(false), HasPreprocessRule(false),
                      HasAssembleRule(false) {}
  };
  void GetLocalObjectFiles(
                    std::map<std::string, LocalObjectInfo> &localObjectFiles);

  void WriteObjectConvenienceRule(std::ostream& ruleFileStream,
                                  const char* comment, const char* output,
                                  LocalObjectInfo const& info);

  std::vector<std::string> LocalHelp;

  /* does the work for each target */
  std::map<std::string, std::string> MakeVariableMap;
  std::map<std::string, std::string> ShortMakeVariableMap;
};

#endif
