/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "cmLocalUnixMakefileGenerator3.h"

#include "cmGeneratedFileStream.h"
#include "cmGlobalUnixMakefileGenerator3.h"
#include "cmMakefile.h"
#include "cmMakefileTargetGenerator.h"
#include "cmSourceFile.h"
#include "cmake.h"
#include "cmVersion.h"
#include "cmFileTimeComparison.h"
#include "cmCustomCommandGenerator.h"
#include "cmAlgorithms.h"

// Include dependency scanners for supported languages.  Only the
// C/C++ scanner is needed for bootstrapping CMake.
#include "cmDependsC.h"
#ifdef CMAKE_BUILD_WITH_CMAKE
# include "cmDependsFortran.h"
# include "cmDependsJava.h"
#endif

#include <cmsys/auto_ptr.hxx>
#include <cmsys/Terminal.h>

#include <queue>

//----------------------------------------------------------------------------
// Escape special characters in Makefile dependency lines
class cmMakeSafe
{
public:
  cmMakeSafe(const char* s): Data(s) {}
  cmMakeSafe(std::string const& s): Data(s.c_str()) {}
private:
  const char* Data;
  friend std::ostream& operator<<(std::ostream& os,
                                  cmMakeSafe const& self)
    {
    for(const char* c = self.Data; *c; ++c)
      {
      switch (*c)
        {
        case '=': os << "$(EQUALS)"; break;
        default: os << *c; break;
        }
      }
    return os;
    }
};

//----------------------------------------------------------------------------
// Helper function used below.
static std::string cmSplitExtension(std::string const& in, std::string& base)
{
  std::string ext;
  std::string::size_type dot_pos = in.rfind(".");
  if(dot_pos != std::string::npos)
    {
    // Remove the extension first in case &base == &in.
    ext = in.substr(dot_pos, std::string::npos);
    base = in.substr(0, dot_pos);
    }
  else
    {
    base = in;
    }
  return ext;
}

//----------------------------------------------------------------------------
cmLocalUnixMakefileGenerator3::
cmLocalUnixMakefileGenerator3(cmGlobalGenerator* gg, cmLocalGenerator* parent,
                              cmState::Snapshot snapshot)
  : cmLocalGenerator(gg, parent, snapshot)
{
  this->MakefileVariableSize = 0;
  this->ColorMakefile = false;
  this->SkipPreprocessedSourceRules = false;
  this->SkipAssemblySourceRules = false;
  this->MakeCommandEscapeTargetTwice = false;
  this->BorlandMakeCurlyHack = false;
}

//----------------------------------------------------------------------------
cmLocalUnixMakefileGenerator3::~cmLocalUnixMakefileGenerator3()
{
}

//----------------------------------------------------------------------------
void cmLocalUnixMakefileGenerator3::Configure()
{
  // Compute the path to use when referencing the current output
  // directory from the top output directory.
  this->HomeRelativeOutputPath =
    this->Convert(this->Makefile->GetCurrentBinaryDirectory(), HOME_OUTPUT);
  if(this->HomeRelativeOutputPath == ".")
    {
    this->HomeRelativeOutputPath = "";
    }
  if(!this->HomeRelativeOutputPath.empty())
    {
    this->HomeRelativeOutputPath += "/";
    }
  this->cmLocalGenerator::Configure();
}

//----------------------------------------------------------------------------
void cmLocalUnixMakefileGenerator3::Generate()
{
  // Store the configuration name that will be generated.
  if(const char* config = this->Makefile->GetDefinition("CMAKE_BUILD_TYPE"))
    {
    // Use the build type given by the user.
    this->ConfigurationName = config;
    }
  else
    {
    // No configuration type given.
    this->ConfigurationName = "";
    }

  // Record whether some options are enabled to avoid checking many
  // times later.
  if(!this->GetGlobalGenerator()->GetCMakeInstance()->GetIsInTryCompile())
    {
    this->ColorMakefile = this->Makefile->IsOn("CMAKE_COLOR_MAKEFILE");
    }
  this->SkipPreprocessedSourceRules =
    this->Makefile->IsOn("CMAKE_SKIP_PREPROCESSED_SOURCE_RULES");
  this->SkipAssemblySourceRules =
    this->Makefile->IsOn("CMAKE_SKIP_ASSEMBLY_SOURCE_RULES");

  // Generate the rule files for each target.
  cmGeneratorTargetsType targets = this->Makefile->GetGeneratorTargets();
  cmGlobalUnixMakefileGenerator3* gg =
    static_cast<cmGlobalUnixMakefileGenerator3*>(this->GlobalGenerator);
  for(cmGeneratorTargetsType::iterator t = targets.begin();
      t != targets.end(); ++t)
    {
    if (t->second->Target->GetType() == cmTarget::INTERFACE_LIBRARY
        || t->second->Target->IsImported())
      {
      continue;
      }
    cmsys::auto_ptr<cmMakefileTargetGenerator> tg(
      cmMakefileTargetGenerator::New(t->second));
    if (tg.get())
      {
      tg->WriteRuleFiles();
      gg->RecordTargetProgress(tg.get());
      }
    }

  // write the local Makefile
  this->WriteLocalMakefile();

  // Write the cmake file with information for this directory.
  this->WriteDirectoryInformationFile();
}

//----------------------------------------------------------------------------
void cmLocalUnixMakefileGenerator3::ComputeObjectFilenames(
                        std::map<cmSourceFile const*, std::string>& mapping,
                        cmGeneratorTarget const* gt)
{
  for(std::map<cmSourceFile const*, std::string>::iterator
      si = mapping.begin(); si != mapping.end(); ++si)
    {
    cmSourceFile const* sf = si->first;
    si->second = this->GetObjectFileNameWithoutTarget(*sf,
                                                      gt->ObjectDirectory);
    }
}

//----------------------------------------------------------------------------
void cmLocalUnixMakefileGenerator3::
GetLocalObjectFiles(std::map<std::string, LocalObjectInfo> &localObjectFiles)
{
  std::set<std::string> emitted;
  cmGeneratorTargetsType targets = this->Makefile->GetGeneratorTargets();
  for(cmGeneratorTargetsType::iterator ti = targets.begin();
      ti != targets.end(); ++ti)
    {
    cmGeneratorTarget* gt = ti->second;
    if (gt->GetType() == cmTarget::INTERFACE_LIBRARY)
      {
      continue;
      }
    std::vector<cmSourceFile const*> objectSources;
    gt->GetObjectSources(objectSources, this->Makefile
                                    ->GetSafeDefinition("CMAKE_BUILD_TYPE"));
    // Compute full path to object file directory for this target.
    std::string dir;
    dir += gt->Makefile->GetCurrentBinaryDirectory();
    dir += "/";
    dir += this->GetTargetDirectory(*gt->Target);
    dir += "/";
    // Compute the name of each object file.
    for(std::vector<cmSourceFile const*>::iterator
          si = objectSources.begin();
        si != objectSources.end(); ++si)
      {
      cmSourceFile const* sf = *si;
      bool hasSourceExtension = true;
      std::string objectName = this->GetObjectFileNameWithoutTarget(*sf,
                                                                    dir,
                                                        &hasSourceExtension);
      if(cmSystemTools::FileIsFullPath(objectName.c_str()))
        {
        objectName = cmSystemTools::GetFilenameName(objectName);
        }
      LocalObjectInfo& info = localObjectFiles[objectName];
      info.HasSourceExtension = hasSourceExtension;
      info.push_back(LocalObjectEntry(gt->Target, sf->GetLanguage()));
      }
    }
}

//----------------------------------------------------------------------------
void cmLocalUnixMakefileGenerator3::GetIndividualFileTargets
                                            (std::vector<std::string>& targets)
{
  std::map<std::string, LocalObjectInfo> localObjectFiles;
  this->GetLocalObjectFiles(localObjectFiles);
  for (std::map<std::string, LocalObjectInfo>::iterator lo =
         localObjectFiles.begin();
       lo != localObjectFiles.end(); ++lo)
    {
    targets.push_back(lo->first);

    std::string::size_type dot_pos = lo->first.rfind(".");
    std::string base = lo->first.substr(0, dot_pos);
    if(lo->second.HasPreprocessRule)
      {
      targets.push_back(base + ".i");
      }

    if(lo->second.HasAssembleRule)
      {
      targets.push_back(base + ".s");
      }
    }
}

//----------------------------------------------------------------------------
void cmLocalUnixMakefileGenerator3::WriteLocalMakefile()
{
  // generate the includes
  std::string ruleFileName = "Makefile";

  // Open the rule file.  This should be copy-if-different because the
  // rules may depend on this file itself.
  std::string ruleFileNameFull = this->ConvertToFullPath(ruleFileName);
  cmGeneratedFileStream ruleFileStream(ruleFileNameFull.c_str());
  if(!ruleFileStream)
    {
    return;
    }
  // always write the top makefile
  if (this->Parent)
    {
    ruleFileStream.SetCopyIfDifferent(true);
    }

  // write the all rules
  this->WriteLocalAllRules(ruleFileStream);

  // only write local targets unless at the top Keep track of targets already
  // listed.
  std::set<std::string> emittedTargets;
  if (this->Parent)
    {
    // write our targets, and while doing it collect up the object
    // file rules
    this->WriteLocalMakefileTargets(ruleFileStream,emittedTargets);
    }
  else
    {
    cmGlobalUnixMakefileGenerator3 *gg =
      static_cast<cmGlobalUnixMakefileGenerator3*>(this->GlobalGenerator);
    gg->WriteConvenienceRules(ruleFileStream,emittedTargets);
    }

  bool do_preprocess_rules =
    this->GetCreatePreprocessedSourceRules();
  bool do_assembly_rules =
    this->GetCreateAssemblySourceRules();

  std::map<std::string, LocalObjectInfo> localObjectFiles;
  this->GetLocalObjectFiles(localObjectFiles);

  // now write out the object rules
  // for each object file name
  for (std::map<std::string, LocalObjectInfo>::iterator lo =
         localObjectFiles.begin();
       lo != localObjectFiles.end(); ++lo)
    {
    // Add a convenience rule for building the object file.
    this->WriteObjectConvenienceRule(ruleFileStream,
                                     "target to build an object file",
                                     lo->first.c_str(), lo->second);

    // Check whether preprocessing and assembly rules make sense.
    // They make sense only for C and C++ sources.
    bool lang_has_preprocessor = false;
    bool lang_has_assembly = false;

    for(std::vector<LocalObjectEntry>::const_iterator ei =
          lo->second.begin(); ei != lo->second.end(); ++ei)
      {
      if(ei->Language == "C" ||
         ei->Language == "CXX" ||
         ei->Language == "Fortran")
        {
        // Right now, C, C++ and Fortran have both a preprocessor and the
        // ability to generate assembly code
        lang_has_preprocessor = true;
        lang_has_assembly = true;
        break;
        }
      }

    // Add convenience rules for preprocessed and assembly files.
    if(lang_has_preprocessor && do_preprocess_rules)
      {
      std::string::size_type dot_pos = lo->first.rfind(".");
      std::string base = lo->first.substr(0, dot_pos);
      this->WriteObjectConvenienceRule(
        ruleFileStream, "target to preprocess a source file",
       (base + ".i").c_str(), lo->second);
      lo->second.HasPreprocessRule = true;
      }

    if(lang_has_assembly && do_assembly_rules)
      {
      std::string::size_type dot_pos = lo->first.rfind(".");
      std::string base = lo->first.substr(0, dot_pos);
      this->WriteObjectConvenienceRule(
       ruleFileStream, "target to generate assembly for a file",
       (base + ".s").c_str(), lo->second);
      lo->second.HasAssembleRule = true;
      }
    }

  // add a help target as long as there isn;t a real target named help
  if(emittedTargets.insert("help").second)
    {
    cmGlobalUnixMakefileGenerator3 *gg =
      static_cast<cmGlobalUnixMakefileGenerator3*>(this->GlobalGenerator);
    gg->WriteHelpRule(ruleFileStream,this);
    }

  this->WriteSpecialTargetsBottom(ruleFileStream);
}

//----------------------------------------------------------------------------
void
cmLocalUnixMakefileGenerator3
::WriteObjectConvenienceRule(std::ostream& ruleFileStream,
                             const char* comment, const char* output,
                             LocalObjectInfo const& info)
{
  // If the rule includes the source file extension then create a
  // version that has the extension removed.  The help should include
  // only the version without source extension.
  bool inHelp = true;
  if(info.HasSourceExtension)
    {
    // Remove the last extension.  This should be kept.
    std::string outBase1 = output;
    std::string outExt1 = cmSplitExtension(outBase1, outBase1);

    // Now remove the source extension and put back the last
    // extension.
    std::string outNoExt;
    cmSplitExtension(outBase1, outNoExt);
    outNoExt += outExt1;

    // Add a rule to drive the rule below.
    std::vector<std::string> depends;
    depends.push_back(output);
    std::vector<std::string> no_commands;
    this->WriteMakeRule(ruleFileStream, 0,
                        outNoExt, depends, no_commands, true, true);
    inHelp = false;
    }

  // Recursively make the rule for each target using the object file.
  std::vector<std::string> commands;
  for(std::vector<LocalObjectEntry>::const_iterator t = info.begin();
      t != info.end(); ++t)
    {
    std::string tgtMakefileName =
      this->GetRelativeTargetDirectory(*(t->Target));
    std::string targetName = tgtMakefileName;
    tgtMakefileName += "/build.make";
    targetName += "/";
    targetName += output;
    commands.push_back(
      this->GetRecursiveMakeCall(tgtMakefileName.c_str(), targetName)
      );
    }
  this->CreateCDCommand(commands,
                        this->Makefile->GetHomeOutputDirectory(),
                        cmLocalGenerator::START_OUTPUT);

  // Write the rule to the makefile.
  std::vector<std::string> no_depends;
  this->WriteMakeRule(ruleFileStream, comment,
                      output, no_depends, commands, true, inHelp);
}

//----------------------------------------------------------------------------
void cmLocalUnixMakefileGenerator3
::WriteLocalMakefileTargets(std::ostream& ruleFileStream,
                            std::set<std::string> &emitted)
{
  std::vector<std::string> depends;
  std::vector<std::string> commands;

  // for each target we just provide a rule to cd up to the top and do a make
  // on the target
  cmGeneratorTargetsType targets = this->Makefile->GetGeneratorTargets();
  std::string localName;
  for(cmGeneratorTargetsType::iterator t = targets.begin();
      t != targets.end(); ++t)
    {
    if((t->second->GetType() == cmTarget::EXECUTABLE) ||
       (t->second->GetType() == cmTarget::STATIC_LIBRARY) ||
       (t->second->GetType() == cmTarget::SHARED_LIBRARY) ||
       (t->second->GetType() == cmTarget::MODULE_LIBRARY) ||
       (t->second->GetType() == cmTarget::OBJECT_LIBRARY) ||
       (t->second->GetType() == cmTarget::UTILITY))
      {
      if (t->second->Target->IsImported())
        {
        continue;
        }

      emitted.insert(t->second->GetName());

      // for subdirs add a rule to build this specific target by name.
      localName = this->GetRelativeTargetDirectory(*t->second->Target);
      localName += "/rule";
      commands.clear();
      depends.clear();

      // Build the target for this pass.
      std::string makefile2 = cmake::GetCMakeFilesDirectoryPostSlash();
      makefile2 += "Makefile2";
      commands.push_back(this->GetRecursiveMakeCall
                         (makefile2.c_str(),localName));
      this->CreateCDCommand(commands,
                            this->Makefile->GetHomeOutputDirectory(),
                            cmLocalGenerator::START_OUTPUT);
      this->WriteMakeRule(ruleFileStream, "Convenience name for target.",
                          localName, depends, commands, true);

      // Add a target with the canonical name (no prefix, suffix or path).
      if(localName != t->second->GetName())
        {
        commands.clear();
        depends.push_back(localName);
        this->WriteMakeRule(ruleFileStream, "Convenience name for target.",
                            t->second->GetName(), depends, commands, true);
        }

      // Add a fast rule to build the target
      std::string makefileName =
                         this->GetRelativeTargetDirectory(*t->second->Target);
      makefileName += "/build.make";
      // make sure the makefile name is suitable for a makefile
      std::string makeTargetName =
        this->GetRelativeTargetDirectory(*t->second->Target);
      makeTargetName += "/build";
      localName = t->second->GetName();
      localName += "/fast";
      depends.clear();
      commands.clear();
      commands.push_back(this->GetRecursiveMakeCall
                         (makefileName.c_str(), makeTargetName));
      this->CreateCDCommand(commands,
                            this->Makefile->GetHomeOutputDirectory(),
                            cmLocalGenerator::START_OUTPUT);
      this->WriteMakeRule(ruleFileStream, "fast build rule for target.",
                          localName, depends, commands, true);

      // Add a local name for the rule to relink the target before
      // installation.
      if(t->second->Target
                  ->NeedRelinkBeforeInstall(this->ConfigurationName))
        {
        makeTargetName = this->GetRelativeTargetDirectory(*t->second->Target);
        makeTargetName += "/preinstall";
        localName = t->second->GetName();
        localName += "/preinstall";
        depends.clear();
        commands.clear();
        commands.push_back(this->GetRecursiveMakeCall
                           (makefile2.c_str(), makeTargetName));
        this->CreateCDCommand(commands,
                              this->Makefile->GetHomeOutputDirectory(),
                              cmLocalGenerator::START_OUTPUT);
        this->WriteMakeRule(ruleFileStream,
                            "Manual pre-install relink rule for target.",
                            localName, depends, commands, true);
        }
      }
    }
}

//----------------------------------------------------------------------------
void cmLocalUnixMakefileGenerator3::WriteDirectoryInformationFile()
{
  std::string infoFileName = this->Makefile->GetCurrentBinaryDirectory();
  infoFileName += cmake::GetCMakeFilesDirectory();
  infoFileName += "/CMakeDirectoryInformation.cmake";

  // Open the output file.
  cmGeneratedFileStream infoFileStream(infoFileName.c_str());
  if(!infoFileStream)
    {
    return;
    }

  infoFileStream.SetCopyIfDifferent(true);
  // Write the do not edit header.
  this->WriteDisclaimer(infoFileStream);

  // Setup relative path conversion tops.
  infoFileStream
    << "# Relative path conversion top directories.\n"
    << "set(CMAKE_RELATIVE_PATH_TOP_SOURCE \""
    << this->StateSnapshot.GetRelativePathTopSource()
    << "\")\n"
    << "set(CMAKE_RELATIVE_PATH_TOP_BINARY \""
    << this->StateSnapshot.GetRelativePathTopBinary()
    << "\")\n"
    << "\n";

  // Tell the dependency scanner to use unix paths if necessary.
  if(cmSystemTools::GetForceUnixPaths())
    {
    infoFileStream
      << "# Force unix paths in dependencies.\n"
      << "set(CMAKE_FORCE_UNIX_PATHS 1)\n"
      << "\n";
    }

  // Store the include regular expressions for this directory.
  infoFileStream
    << "\n"
    << "# The C and CXX include file regular expressions for "
    << "this directory.\n";
  infoFileStream
    << "set(CMAKE_C_INCLUDE_REGEX_SCAN ";
  this->WriteCMakeArgument(infoFileStream,
                           this->Makefile->GetIncludeRegularExpression());
  infoFileStream
    << ")\n";
  infoFileStream
    << "set(CMAKE_C_INCLUDE_REGEX_COMPLAIN ";
  this->WriteCMakeArgument(infoFileStream,
                           this->Makefile->GetComplainRegularExpression());
  infoFileStream
    << ")\n";
  infoFileStream
    << "set(CMAKE_CXX_INCLUDE_REGEX_SCAN ${CMAKE_C_INCLUDE_REGEX_SCAN})\n";
  infoFileStream
    << "set(CMAKE_CXX_INCLUDE_REGEX_COMPLAIN "
    "${CMAKE_C_INCLUDE_REGEX_COMPLAIN})\n";
}

//----------------------------------------------------------------------------
std::string
cmLocalUnixMakefileGenerator3
::ConvertToFullPath(const std::string& localPath)
{
  std::string dir = this->Makefile->GetCurrentBinaryDirectory();
  dir += "/";
  dir += localPath;
  return dir;
}


const std::string &cmLocalUnixMakefileGenerator3::GetHomeRelativeOutputPath()
{
  return this->HomeRelativeOutputPath;
}

//----------------------------------------------------------------------------
void
cmLocalUnixMakefileGenerator3
::WriteMakeRule(std::ostream& os,
                const char* comment,
                const std::string& target,
                const std::vector<std::string>& depends,
                const std::vector<std::string>& commands,
                bool symbolic,
                bool in_help)
{
  // Make sure there is a target.
  if(target.empty())
    {
    cmSystemTools::Error("No target for WriteMakeRule! called with comment: ",
                         comment);
    return;
    }

  std::string replace;

  // Write the comment describing the rule in the makefile.
  if(comment)
    {
    replace = comment;
    std::string::size_type lpos = 0;
    std::string::size_type rpos;
    while((rpos = replace.find('\n', lpos)) != std::string::npos)
      {
      os << "# " << replace.substr(lpos, rpos-lpos) << "\n";
      lpos = rpos+1;
      }
    os << "# " << replace.substr(lpos) << "\n";
    }

  // Construct the left hand side of the rule.
  std::string tgt = this->Convert(target, HOME_OUTPUT, MAKERULE);

  const char* space = "";
  if(tgt.size() == 1)
    {
    // Add a space before the ":" to avoid drive letter confusion on
    // Windows.
    space = " ";
    }

  // Mark the rule as symbolic if requested.
  if(symbolic)
    {
    if(const char* sym =
       this->Makefile->GetDefinition("CMAKE_MAKE_SYMBOLIC_RULE"))
      {
      os << cmMakeSafe(tgt) << space << ": " << sym << "\n";
      }
    }

  // Write the rule.
  if(depends.empty())
    {
    // No dependencies.  The commands will always run.
    os << cmMakeSafe(tgt) << space << ":\n";
    }
  else
    {
    // Split dependencies into multiple rule lines.  This allows for
    // very long dependency lists even on older make implementations.
    for(std::vector<std::string>::const_iterator dep = depends.begin();
        dep != depends.end(); ++dep)
      {
      replace = *dep;
      replace = this->Convert(replace,HOME_OUTPUT,MAKERULE);
      os << cmMakeSafe(tgt) << space << ": " << cmMakeSafe(replace) << "\n";
      }
    }

  // Write the list of commands.
  os << cmWrap("\t", commands, "", "\n") << "\n";
  if(symbolic && !this->IsWatcomWMake())
    {
    os << ".PHONY : " << cmMakeSafe(tgt) << "\n";
    }
  os << "\n";
  // Add the output to the local help if requested.
  if(in_help)
    {
    this->LocalHelp.push_back(target);
    }
}

//----------------------------------------------------------------------------
std::string
cmLocalUnixMakefileGenerator3
::ConvertShellCommand(std::string const& cmd, RelativeRoot root)
{
  if(this->IsWatcomWMake() &&
     cmSystemTools::FileIsFullPath(cmd.c_str()) &&
     cmd.find_first_of("( )") != cmd.npos)
    {
    // On Watcom WMake use the windows short path for the command
    // name.  This is needed to avoid funny quoting problems on
    // lines with shell redirection operators.
    std::string scmd;
    if(cmSystemTools::GetShortPath(cmd, scmd))
      {
      return this->Convert(scmd, NONE, SHELL);
      }
    }
  return this->Convert(cmd, root, SHELL);
}

//----------------------------------------------------------------------------
void
cmLocalUnixMakefileGenerator3
::WriteMakeVariables(std::ostream& makefileStream)
{
  this->WriteDivider(makefileStream);
  makefileStream
    << "# Set environment variables for the build.\n"
    << "\n";
  cmGlobalUnixMakefileGenerator3* gg =
    static_cast<cmGlobalUnixMakefileGenerator3*>(this->GlobalGenerator);
  if(gg->DefineWindowsNULL)
    {
    makefileStream
      << "!IF \"$(OS)\" == \"Windows_NT\"\n"
      << "NULL=\n"
      << "!ELSE\n"
      << "NULL=nul\n"
      << "!ENDIF\n";
    }
  if(this->IsWindowsShell())
    {
     makefileStream
       << "SHELL = cmd.exe\n"
       << "\n";
    }
  else
    {
#if !defined(__VMS)
      makefileStream
        << "# The shell in which to execute make rules.\n"
        << "SHELL = /bin/sh\n"
        << "\n";
#endif
    }

  makefileStream
    << "# The CMake executable.\n"
    << "CMAKE_COMMAND = "
    << this->ConvertShellCommand(cmSystemTools::GetCMakeCommand(), FULL)
    << "\n"
    << "\n";
  makefileStream
    << "# The command to remove a file.\n"
    << "RM = "
    << this->ConvertShellCommand(cmSystemTools::GetCMakeCommand(), FULL)
    << " -E remove -f\n"
    << "\n";
  makefileStream
    << "# Escaping for special characters.\n"
    << "EQUALS = =\n"
    << "\n";
  makefileStream
    << "# The top-level source directory on which CMake was run.\n"
    << "CMAKE_SOURCE_DIR = "
    << this->Convert(this->Makefile->GetHomeDirectory(), FULL, SHELL)
    << "\n"
    << "\n";
  makefileStream
    << "# The top-level build directory on which CMake was run.\n"
    << "CMAKE_BINARY_DIR = "
    << this->Convert(this->Makefile->GetHomeOutputDirectory(), FULL, SHELL)
    << "\n"
    << "\n";
}

//----------------------------------------------------------------------------
void
cmLocalUnixMakefileGenerator3
::WriteSpecialTargetsTop(std::ostream& makefileStream)
{
  this->WriteDivider(makefileStream);
  makefileStream
    << "# Special targets provided by cmake.\n"
    << "\n";

  std::vector<std::string> no_commands;
  std::vector<std::string> no_depends;

  // Special target to cleanup operation of make tool.
  // This should be the first target except for the default_target in
  // the interface Makefile.
  this->WriteMakeRule(
    makefileStream, "Disable implicit rules so canonical targets will work.",
    ".SUFFIXES", no_depends, no_commands, false);

  if(!this->IsNMake()
      && !this->IsWatcomWMake() && !this->BorlandMakeCurlyHack)
    {
    // turn off RCS and SCCS automatic stuff from gmake
    makefileStream
      << "# Remove some rules from gmake that .SUFFIXES does not remove.\n"
      << "SUFFIXES =\n\n";
    }
  // Add a fake suffix to keep HP happy.  Must be max 32 chars for SGI make.
  std::vector<std::string> depends;
  depends.push_back(".hpux_make_needs_suffix_list");
  this->WriteMakeRule(makefileStream, 0,
                      ".SUFFIXES", depends, no_commands, false);
  if(this->IsWatcomWMake())
    {
    // Switch on WMake feature, if an error or interrupt occurs during
    // makefile processing, the current target being made may be deleted
    // without prompting (the same as command line -e option).
    makefileStream <<
      "\n"
      ".ERASE\n"
      "\n"
      ;
    }
  if(this->Makefile->IsOn("CMAKE_VERBOSE_MAKEFILE"))
    {
    makefileStream
      << "# Produce verbose output by default.\n"
      << "VERBOSE = 1\n"
      << "\n";
    }
  if(this->IsWatcomWMake())
    {
    makefileStream <<
      "!ifndef VERBOSE\n"
      ".SILENT\n"
      "!endif\n"
      "\n"
      ;
    }
  else
    {
    // Write special target to silence make output.  This must be after
    // the default target in case VERBOSE is set (which changes the
    // name).  The setting of CMAKE_VERBOSE_MAKEFILE to ON will cause a
    // "VERBOSE=1" to be added as a make variable which will change the
    // name of this special target.  This gives a make-time choice to
    // the user.
    this->WriteMakeRule(makefileStream,
                        "Suppress display of executed commands.",
                        "$(VERBOSE).SILENT",
                        no_depends,
                        no_commands, false);
    }

  // Work-around for makes that drop rules that have no dependencies
  // or commands.
  cmGlobalUnixMakefileGenerator3* gg =
    static_cast<cmGlobalUnixMakefileGenerator3*>(this->GlobalGenerator);
  std::string hack = gg->GetEmptyRuleHackDepends();
  if(!hack.empty())
    {
    no_depends.push_back(hack);
    }
  std::string hack_cmd = gg->GetEmptyRuleHackCommand();
  if(!hack_cmd.empty())
    {
    no_commands.push_back(hack_cmd);
    }

  // Special symbolic target that never exists to force dependers to
  // run their rules.
  this->WriteMakeRule
    (makefileStream,
     "A target that is always out of date.",
     "cmake_force", no_depends, no_commands, true);

  // Variables for reference by other rules.
  this->WriteMakeVariables(makefileStream);
}

//----------------------------------------------------------------------------
void cmLocalUnixMakefileGenerator3
::WriteSpecialTargetsBottom(std::ostream& makefileStream)
{
  this->WriteDivider(makefileStream);
  makefileStream
    << "# Special targets to cleanup operation of make.\n"
    << "\n";

  // Write special "cmake_check_build_system" target to run cmake with
  // the --check-build-system flag.
  {
  // Build command to run CMake to check if anything needs regenerating.
  std::string cmakefileName = cmake::GetCMakeFilesDirectoryPostSlash();
  cmakefileName += "Makefile.cmake";
  std::string runRule =
    "$(CMAKE_COMMAND) -H$(CMAKE_SOURCE_DIR) -B$(CMAKE_BINARY_DIR)";
  runRule += " --check-build-system ";
  runRule += this->Convert(cmakefileName,NONE,SHELL);
  runRule += " 0";

  std::vector<std::string> no_depends;
  std::vector<std::string> commands;
  commands.push_back(runRule);
  if(this->Parent)
    {
    this->CreateCDCommand(commands,
                          this->Makefile->GetHomeOutputDirectory(),
                          cmLocalGenerator::START_OUTPUT);
    }
  this->WriteMakeRule(makefileStream,
                      "Special rule to run CMake to check the build system "
                      "integrity.\n"
                      "No rule that depends on this can have "
                      "commands that come from listfiles\n"
                      "because they might be regenerated.",
                      "cmake_check_build_system",
                      no_depends,
                      commands, true);
  }
}



//----------------------------------------------------------------------------
void
cmLocalUnixMakefileGenerator3
::WriteConvenienceRule(std::ostream& ruleFileStream,
                       const std::string& realTarget,
                       const std::string& helpTarget)
{
  // A rule is only needed if the names are different.
  if(realTarget != helpTarget)
    {
    // The helper target depends on the real target.
    std::vector<std::string> depends;
    depends.push_back(realTarget);

    // There are no commands.
    std::vector<std::string> no_commands;

    // Write the rule.
    this->WriteMakeRule(ruleFileStream, "Convenience name for target.",
                        helpTarget, depends, no_commands, true);
    }
}


//----------------------------------------------------------------------------
std::string
cmLocalUnixMakefileGenerator3
::GetRelativeTargetDirectory(cmTarget const& target)
{
  std::string dir = this->HomeRelativeOutputPath;
  dir += this->GetTargetDirectory(target);
  return this->Convert(dir,NONE,UNCHANGED);
}



//----------------------------------------------------------------------------
void cmLocalUnixMakefileGenerator3::AppendFlags(std::string& flags,
                                                const std::string& newFlags)
{
  if(this->IsWatcomWMake() && !newFlags.empty())
    {
    std::string newf = newFlags;
    if(newf.find("\\\"") != newf.npos)
      {
      cmSystemTools::ReplaceString(newf, "\\\"", "\"");
      this->cmLocalGenerator::AppendFlags(flags, newf);
      return;
      }
    }
  this->cmLocalGenerator::AppendFlags(flags, newFlags);
}

//----------------------------------------------------------------------------
void cmLocalUnixMakefileGenerator3::AppendFlags(std::string& flags,
                                                const char* newFlags)
{
  this->cmLocalGenerator::AppendFlags(flags, newFlags);
}

//----------------------------------------------------------------------------
void
cmLocalUnixMakefileGenerator3
::AppendRuleDepend(std::vector<std::string>& depends,
                   const char* ruleFileName)
{
  // Add a dependency on the rule file itself unless an option to skip
  // it is specifically enabled by the user or project.
  const char* nodep =
    this->Makefile->GetDefinition("CMAKE_SKIP_RULE_DEPENDENCY");
  if(!nodep || cmSystemTools::IsOff(nodep))
    {
    depends.push_back(ruleFileName);
    }
}

//----------------------------------------------------------------------------
void
cmLocalUnixMakefileGenerator3
::AppendRuleDepends(std::vector<std::string>& depends,
                    std::vector<std::string> const& ruleFiles)
{
  // Add a dependency on the rule file itself unless an option to skip
  // it is specifically enabled by the user or project.
  if(!this->Makefile->IsOn("CMAKE_SKIP_RULE_DEPENDENCY"))
    {
    depends.insert(depends.end(), ruleFiles.begin(), ruleFiles.end());
    }
}

//----------------------------------------------------------------------------
void
cmLocalUnixMakefileGenerator3
::AppendCustomDepends(std::vector<std::string>& depends,
                      const std::vector<cmCustomCommand>& ccs)
{
  for(std::vector<cmCustomCommand>::const_iterator i = ccs.begin();
      i != ccs.end(); ++i)
    {
    cmCustomCommandGenerator ccg(*i, this->ConfigurationName,
                                 this->Makefile);
    this->AppendCustomDepend(depends, ccg);
    }
}

//----------------------------------------------------------------------------
void
cmLocalUnixMakefileGenerator3
::AppendCustomDepend(std::vector<std::string>& depends,
                     cmCustomCommandGenerator const& ccg)
{
  for(std::vector<std::string>::const_iterator d = ccg.GetDepends().begin();
      d != ccg.GetDepends().end(); ++d)
    {
    // Lookup the real name of the dependency in case it is a CMake target.
    std::string dep;
    if(this->GetRealDependency(*d, this->ConfigurationName,
                               dep))
      {
      depends.push_back(dep);
      }
    }
}

//----------------------------------------------------------------------------
void
cmLocalUnixMakefileGenerator3
::AppendCustomCommands(std::vector<std::string>& commands,
                       const std::vector<cmCustomCommand>& ccs,
                       cmTarget* target,
                       cmLocalGenerator::RelativeRoot relative)
{
  for(std::vector<cmCustomCommand>::const_iterator i = ccs.begin();
      i != ccs.end(); ++i)
    {
    cmCustomCommandGenerator ccg(*i, this->ConfigurationName,
                                 this->Makefile);
    this->AppendCustomCommand(commands, ccg, target, true, relative);
    }
}

//----------------------------------------------------------------------------
void
cmLocalUnixMakefileGenerator3
::AppendCustomCommand(std::vector<std::string>& commands,
                      cmCustomCommandGenerator const& ccg,
                      cmTarget* target,
                      bool echo_comment,
                      cmLocalGenerator::RelativeRoot relative,
                      std::ostream* content)
{
  // Optionally create a command to display the custom command's
  // comment text.  This is used for pre-build, pre-link, and
  // post-build command comments.  Custom build step commands have
  // their comments generated elsewhere.
  if(echo_comment)
    {
    const char* comment = ccg.GetComment();
    if(comment && *comment)
      {
      this->AppendEcho(commands, comment,
                       cmLocalUnixMakefileGenerator3::EchoGenerate);
      }
    }

  // if the command specified a working directory use it.
  std::string dir  = this->Makefile->GetCurrentBinaryDirectory();
  std::string workingDir = ccg.GetWorkingDirectory();
  if(!workingDir.empty())
    {
    dir = workingDir;
    }
  if(content)
    {
    *content << dir;
    }

  // Add each command line to the set of commands.
  std::vector<std::string> commands1;
  for(unsigned int c = 0; c < ccg.GetNumberOfCommands(); ++c)
    {
    // Build the command line in a single string.
    std::string cmd = ccg.GetCommand(c);
    if (!cmd.empty())
      {
      // Use "call " before any invocations of .bat or .cmd files
      // invoked as custom commands in the WindowsShell.
      //
      bool useCall = false;

      if (this->IsWindowsShell())
        {
        std::string suffix;
        if (cmd.size() > 4)
          {
          suffix = cmSystemTools::LowerCase(cmd.substr(cmd.size()-4));
          if (suffix == ".bat" || suffix == ".cmd")
            {
            useCall = true;
            }
          }
        }

      cmSystemTools::ReplaceString(cmd, "/./", "/");
      // Convert the command to a relative path only if the current
      // working directory will be the start-output directory.
      bool had_slash = cmd.find("/") != cmd.npos;
      if(workingDir.empty())
        {
        cmd = this->Convert(cmd,START_OUTPUT);
        }
      bool has_slash = cmd.find("/") != cmd.npos;
      if(had_slash && !has_slash)
        {
        // This command was specified as a path to a file in the
        // current directory.  Add a leading "./" so it can run
        // without the current directory being in the search path.
        cmd = "./" + cmd;
        }
      std::string launcher =
        this->MakeLauncher(ccg, target,
                           workingDir.empty()? START_OUTPUT : NONE);
      cmd = launcher + this->ConvertShellCommand(cmd, NONE);

      ccg.AppendArguments(c, cmd);
      if(content)
        {
        // Rule content does not include the launcher.
        *content << (cmd.c_str()+launcher.size());
        }
      if(this->BorlandMakeCurlyHack)
        {
        // Borland Make has a very strange bug.  If the first curly
        // brace anywhere in the command string is a left curly, it
        // must be written {{} instead of just {.  Otherwise some
        // curly braces are removed.  The hack can be skipped if the
        // first curly brace is the last character.
        std::string::size_type lcurly = cmd.find("{");
        if(lcurly != cmd.npos && lcurly < (cmd.size()-1))
          {
          std::string::size_type rcurly = cmd.find("}");
          if(rcurly == cmd.npos || rcurly > lcurly)
            {
            // The first curly is a left curly.  Use the hack.
            std::string hack_cmd = cmd.substr(0, lcurly);
            hack_cmd += "{{}";
            hack_cmd += cmd.substr(lcurly+1);
            cmd = hack_cmd;
            }
          }
        }
      if (launcher.empty())
        {
        if (useCall)
          {
          cmd = "call " + cmd;
          }
        else if (this->IsNMake() && cmd[0]=='"')
          {
          cmd = "echo >nul && " + cmd;
          }
        }
      commands1.push_back(cmd);
      }
    }

  // Setup the proper working directory for the commands.
  this->CreateCDCommand(commands1, dir.c_str(), relative);

  // push back the custom commands
  commands.insert(commands.end(), commands1.begin(), commands1.end());
}

//----------------------------------------------------------------------------
std::string
cmLocalUnixMakefileGenerator3::MakeLauncher(
  cmCustomCommandGenerator const& ccg,
  cmTarget* target, RelativeRoot relative)
{
  // Short-circuit if there is no launcher.
  const char* prop = "RULE_LAUNCH_CUSTOM";
  const char* val = this->GetRuleLauncher(target, prop);
  if(!(val && *val))
    {
    return "";
    }

  // Expand rules in the empty string.  It may insert the launcher and
  // perform replacements.
  RuleVariables vars;
  vars.RuleLauncher = prop;
  vars.CMTarget = target;
  std::string output;
  const std::vector<std::string>& outputs = ccg.GetOutputs();
  if(!outputs.empty())
    {
    output = this->Convert(outputs[0], relative, SHELL);
    }
  vars.Output = output.c_str();

  std::string launcher;
  this->ExpandRuleVariables(launcher, vars);
  if(!launcher.empty())
    {
    launcher += " ";
    }
  return launcher;
}

//----------------------------------------------------------------------------
void
cmLocalUnixMakefileGenerator3
::AppendCleanCommand(std::vector<std::string>& commands,
                     const std::vector<std::string>& files,
                     cmTarget& target, const char* filename)
{
  std::string cleanfile = this->Makefile->GetCurrentBinaryDirectory();
  cleanfile += "/";
  cleanfile += this->GetTargetDirectory(target);
  cleanfile += "/cmake_clean";
  if(filename)
    {
    cleanfile += "_";
    cleanfile += filename;
    }
  cleanfile += ".cmake";
  std::string cleanfilePath = this->Convert(cleanfile, FULL);
  cmsys::ofstream fout(cleanfilePath.c_str());
  if(!fout)
    {
    cmSystemTools::Error("Could not create ", cleanfilePath.c_str());
    }
  if(!files.empty())
    {
    fout << "file(REMOVE_RECURSE\n";
    for(std::vector<std::string>::const_iterator f = files.begin();
        f != files.end(); ++f)
      {
      std::string fc = this->Convert(*f,START_OUTPUT,UNCHANGED);
      fout << "  " << cmLocalGenerator::EscapeForCMake(fc) << "\n";
      }
    fout << ")\n";
    }
  std::string remove = "$(CMAKE_COMMAND) -P ";
  remove += this->Convert(cleanfile, START_OUTPUT, SHELL);
  commands.push_back(remove);

  // For the main clean rule add per-language cleaning.
  if(!filename)
    {
    // Get the set of source languages in the target.
    std::set<std::string> languages;
    target.GetLanguages(languages,
                      this->Makefile->GetSafeDefinition("CMAKE_BUILD_TYPE"));
    fout << "\n"
         << "# Per-language clean rules from dependency scanning.\n"
         << "foreach(lang " << cmJoin(languages, " ") << ")\n"
         << "  include(" << this->GetTargetDirectory(target)
         << "/cmake_clean_${lang}.cmake OPTIONAL)\n"
         << "endforeach()\n";
    }
}

//----------------------------------------------------------------------------
void
cmLocalUnixMakefileGenerator3::AppendEcho(std::vector<std::string>& commands,
                                          std::string const& text,
                                          EchoColor color,
                                          EchoProgress const* progress)
{
  // Choose the color for the text.
  std::string color_name;
  if(this->GlobalGenerator->GetToolSupportsColor() && this->ColorMakefile)
    {
    // See cmake::ExecuteEchoColor in cmake.cxx for these options.
    // This color set is readable on both black and white backgrounds.
    switch(color)
      {
      case EchoNormal:
        break;
      case EchoDepend:
        color_name = "--magenta --bold ";
        break;
      case EchoBuild:
        color_name = "--green ";
        break;
      case EchoLink:
        color_name = "--green --bold ";
        break;
      case EchoGenerate:
        color_name = "--blue --bold ";
        break;
      case EchoGlobal:
        color_name = "--cyan ";
        break;
      }
    }

  // Echo one line at a time.
  std::string line;
  line.reserve(200);
  for(const char* c = text.c_str();; ++c)
    {
    if(*c == '\n' || *c == '\0')
      {
      // Avoid writing a blank last line on end-of-string.
      if(*c != '\0' || !line.empty())
        {
        // Add a command to echo this line.
        std::string cmd;
        if(color_name.empty() && !progress)
          {
          // Use the native echo command.
          cmd = "@echo ";
          cmd += this->EscapeForShell(line, false, true);
          }
        else
          {
          // Use cmake to echo the text in color.
          cmd = "@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) ";
          cmd += color_name;
          if (progress)
            {
            cmd += "--progress-dir=";
            cmd += this->Convert(progress->Dir,
                                 cmLocalGenerator::FULL,
                                 cmLocalGenerator::SHELL);
            cmd += " ";
            cmd += "--progress-num=";
            cmd += progress->Arg;
            cmd += " ";
            }
          cmd += this->EscapeForShell(line);
          }
        commands.push_back(cmd);
        }

      // Reset the line to emtpy.
      line = "";

      // Progress appears only on first line.
      progress = 0;

      // Terminate on end-of-string.
      if(*c == '\0')
        {
        return;
        }
      }
    else if(*c != '\r')
      {
      // Append this character to the current line.
      line += *c;
      }
    }
}

//----------------------------------------------------------------------------
std::string
cmLocalUnixMakefileGenerator3
::CreateMakeVariable(const std::string& sin, const std::string& s2in)
{
  std::string s = sin;
  std::string s2 = s2in;
  std::string unmodified = s;
  unmodified += s2;
  // if there is no restriction on the length of make variables
  // and there are no "." characters in the string, then return the
  // unmodified combination.
  if((!this->MakefileVariableSize && unmodified.find('.') == s.npos)
     && (!this->MakefileVariableSize && unmodified.find('+') == s.npos)
     && (!this->MakefileVariableSize && unmodified.find('-') == s.npos))
    {
    return unmodified;
    }

  // see if the variable has been defined before and return
  // the modified version of the variable
  std::map<std::string, std::string>::iterator i =
    this->MakeVariableMap.find(unmodified);
  if(i != this->MakeVariableMap.end())
    {
    return i->second;
    }
  // start with the unmodified variable
  std::string ret = unmodified;
  // if this there is no value for this->MakefileVariableSize then
  // the string must have bad characters in it
  if(!this->MakefileVariableSize)
    {
    cmSystemTools::ReplaceString(ret, ".", "_");
    cmSystemTools::ReplaceString(ret, "-", "__");
    cmSystemTools::ReplaceString(ret, "+", "___");
    int ni = 0;
    char buffer[5];
    // make sure the _ version is not already used, if
    // it is used then add number to the end of the variable
    while(this->ShortMakeVariableMap.count(ret) && ni < 1000)
      {
      ++ni;
      sprintf(buffer, "%04d", ni);
      ret = unmodified + buffer;
      }
    this->ShortMakeVariableMap[ret] = "1";
    this->MakeVariableMap[unmodified] = ret;
    return ret;
    }

  // if the string is greater than 32 chars it is an invalid variable name
  // for borland make
  if(static_cast<int>(ret.size()) > this->MakefileVariableSize)
    {
    int keep = this->MakefileVariableSize - 8;
    int size = keep + 3;
    std::string str1 = s;
    std::string str2 = s2;
    // we must shorten the combined string by 4 characters
    // keep no more than 24 characters from the second string
    if(static_cast<int>(str2.size()) > keep)
      {
      str2 = str2.substr(0, keep);
      }
    if(static_cast<int>(str1.size()) + static_cast<int>(str2.size()) > size)
      {
      str1 = str1.substr(0, size - str2.size());
      }
    char buffer[5];
    int ni = 0;
    sprintf(buffer, "%04d", ni);
    ret = str1 + str2 + buffer;
    while(this->ShortMakeVariableMap.count(ret) && ni < 1000)
      {
      ++ni;
      sprintf(buffer, "%04d", ni);
      ret = str1 + str2 + buffer;
      }
    if(ni == 1000)
      {
      cmSystemTools::Error("Borland makefile variable length too long");
      return unmodified;
      }
    // once an unused variable is found
    this->ShortMakeVariableMap[ret] = "1";
    }
  // always make an entry into the unmodified to variable map
  this->MakeVariableMap[unmodified] = ret;
  return ret;
}

//----------------------------------------------------------------------------
bool cmLocalUnixMakefileGenerator3::UpdateDependencies(const char* tgtInfo,
                                                       bool verbose,
                                                       bool color)
{
  // read in the target info file
  if(!this->Makefile->ReadListFile(tgtInfo) ||
     cmSystemTools::GetErrorOccuredFlag())
    {
    cmSystemTools::Error("Target DependInfo.cmake file not found");
    }

  // Check if any multiple output pairs have a missing file.
  this->CheckMultipleOutputs(verbose);

  std::string dir = cmSystemTools::GetFilenamePath(tgtInfo);
  std::string internalDependFile = dir + "/depend.internal";
  std::string dependFile = dir + "/depend.make";

  // If the target DependInfo.cmake file has changed since the last
  // time dependencies were scanned then force rescanning.  This may
  // happen when a new source file is added and CMake regenerates the
  // project but no other sources were touched.
  bool needRescanDependInfo = false;
  cmFileTimeComparison* ftc =
    this->GlobalGenerator->GetCMakeInstance()->GetFileComparison();
  {
  int result;
  if(!ftc->FileTimeCompare(internalDependFile.c_str(), tgtInfo, &result) ||
     result < 0)
    {
    if(verbose)
      {
      std::ostringstream msg;
      msg << "Dependee \"" << tgtInfo
          << "\" is newer than depender \""
          << internalDependFile << "\"." << std::endl;
      cmSystemTools::Stdout(msg.str().c_str());
      }
    needRescanDependInfo = true;
    }
  }

  // If the directory information is newer than depend.internal, include dirs
  // may have changed. In this case discard all old dependencies.
  bool needRescanDirInfo = false;
  std::string dirInfoFile = this->Makefile->GetCurrentBinaryDirectory();
  dirInfoFile += cmake::GetCMakeFilesDirectory();
  dirInfoFile += "/CMakeDirectoryInformation.cmake";
  {
  int result;
  if(!ftc->FileTimeCompare(internalDependFile.c_str(),
                           dirInfoFile.c_str(), &result) || result < 0)
    {
    if(verbose)
      {
      std::ostringstream msg;
      msg << "Dependee \"" << dirInfoFile
          << "\" is newer than depender \""
          << internalDependFile << "\"." << std::endl;
      cmSystemTools::Stdout(msg.str().c_str());
      }
    needRescanDirInfo = true;
    }
  }

  // Check the implicit dependencies to see if they are up to date.
  // The build.make file may have explicit dependencies for the object
  // files but these will not affect the scanning process so they need
  // not be considered.
  std::map<std::string, cmDepends::DependencyVector> validDependencies;
  bool needRescanDependencies = false;
  if (needRescanDirInfo == false)
    {
    cmDependsC checker;
    checker.SetVerbose(verbose);
    checker.SetFileComparison(ftc);
    // cmDependsC::Check() fills the vector validDependencies() with the
    // dependencies for those files where they are still valid, i.e. neither
    // the files themselves nor any files they depend on have changed.
    // We don't do that if the CMakeDirectoryInformation.cmake file has
    // changed, because then potentially all dependencies have changed.
    // This information is given later on to cmDependsC, which then only
    // rescans the files where it did not get valid dependencies via this
    // dependency vector. This means that in the normal case, when only
    // few or one file have been edited, then also only this one file is
    // actually scanned again, instead of all files for this target.
    needRescanDependencies = !checker.Check(dependFile.c_str(),
                                            internalDependFile.c_str(),
                                            validDependencies);
    }

  if(needRescanDependInfo || needRescanDirInfo || needRescanDependencies)
    {
    // The dependencies must be regenerated.
    std::string targetName = cmSystemTools::GetFilenameName(dir);
    targetName = targetName.substr(0, targetName.length()-4);
    std::string message = "Scanning dependencies of target ";
    message += targetName;
    cmSystemTools::MakefileColorEcho(
      cmsysTerminal_Color_ForegroundMagenta |
      cmsysTerminal_Color_ForegroundBold,
      message.c_str(), true, color);

    return this->ScanDependencies(dir.c_str(), validDependencies);
    }

  // The dependencies are already up-to-date.
  return true;
}

//----------------------------------------------------------------------------
bool
cmLocalUnixMakefileGenerator3
::ScanDependencies(const char* targetDir,
                 std::map<std::string, cmDepends::DependencyVector>& validDeps)
{
  // Read the directory information file.
  cmMakefile* mf = this->Makefile;
  bool haveDirectoryInfo = false;
  std::string dirInfoFile = this->Makefile->GetCurrentBinaryDirectory();
  dirInfoFile += cmake::GetCMakeFilesDirectory();
  dirInfoFile += "/CMakeDirectoryInformation.cmake";
  if(mf->ReadListFile(dirInfoFile.c_str()) &&
     !cmSystemTools::GetErrorOccuredFlag())
    {
    haveDirectoryInfo = true;
    }

  // Lookup useful directory information.
  if(haveDirectoryInfo)
    {
    // Test whether we need to force Unix paths.
    if(const char* force = mf->GetDefinition("CMAKE_FORCE_UNIX_PATHS"))
      {
      if(!cmSystemTools::IsOff(force))
        {
        cmSystemTools::SetForceUnixPaths(true);
        }
      }

    // Setup relative path top directories.
    if(const char* relativePathTopSource =
       mf->GetDefinition("CMAKE_RELATIVE_PATH_TOP_SOURCE"))
      {
      this->StateSnapshot.SetRelativePathTopSource(relativePathTopSource);
      }
    if(const char* relativePathTopBinary =
       mf->GetDefinition("CMAKE_RELATIVE_PATH_TOP_BINARY"))
      {
      this->StateSnapshot.SetRelativePathTopBinary(relativePathTopBinary);
      }
    }
  else
    {
    cmSystemTools::Error("Directory Information file not found");
    }

  // create the file stream for the depends file
  std::string dir = targetDir;

  // Open the make depends file.  This should be copy-if-different
  // because the make tool may try to reload it needlessly otherwise.
  std::string ruleFileNameFull = dir;
  ruleFileNameFull += "/depend.make";
  cmGeneratedFileStream ruleFileStream(ruleFileNameFull.c_str());
  ruleFileStream.SetCopyIfDifferent(true);
  if(!ruleFileStream)
    {
    return false;
    }

  // Open the cmake dependency tracking file.  This should not be
  // copy-if-different because dependencies are re-scanned when it is
  // older than the DependInfo.cmake.
  std::string internalRuleFileNameFull = dir;
  internalRuleFileNameFull += "/depend.internal";
  cmGeneratedFileStream
    internalRuleFileStream(internalRuleFileNameFull.c_str());
  if(!internalRuleFileStream)
    {
    return false;
    }

  this->WriteDisclaimer(ruleFileStream);
  this->WriteDisclaimer(internalRuleFileStream);

  // for each language we need to scan, scan it
  const char *langStr = mf->GetSafeDefinition("CMAKE_DEPENDS_LANGUAGES");
  std::vector<std::string> langs;
  cmSystemTools::ExpandListArgument(langStr, langs);
  for (std::vector<std::string>::iterator li =
         langs.begin(); li != langs.end(); ++li)
    {
    // construct the checker
    std::string lang = *li;

    // Create the scanner for this language
    cmDepends *scanner = 0;
    if(lang == "C" || lang == "CXX" || lang == "RC" || lang == "ASM")
      {
      // TODO: Handle RC (resource files) dependencies correctly.
      scanner = new cmDependsC(this, targetDir, lang, &validDeps);
      }
#ifdef CMAKE_BUILD_WITH_CMAKE
    else if(lang == "Fortran")
      {
      scanner = new cmDependsFortran(this);
      }
    else if(lang == "Java")
      {
      scanner = new cmDependsJava();
      }
#endif

    if (scanner)
      {
      scanner->SetLocalGenerator(this);
      scanner->SetFileComparison
        (this->GlobalGenerator->GetCMakeInstance()->GetFileComparison());
      scanner->SetLanguage(lang);
      scanner->SetTargetDirectory(dir.c_str());
      scanner->Write(ruleFileStream, internalRuleFileStream);

      // free the scanner for this language
      delete scanner;
      }
    }

  return true;
}

//----------------------------------------------------------------------------
void cmLocalUnixMakefileGenerator3::CheckMultipleOutputs(bool verbose)
{
  cmMakefile* mf = this->Makefile;

  // Get the string listing the multiple output pairs.
  const char* pairs_string = mf->GetDefinition("CMAKE_MULTIPLE_OUTPUT_PAIRS");
  if(!pairs_string)
    {
    return;
    }

  // Convert the string to a list and preserve empty entries.
  std::vector<std::string> pairs;
  cmSystemTools::ExpandListArgument(pairs_string, pairs, true);
  for(std::vector<std::string>::const_iterator i = pairs.begin();
      i != pairs.end() && (i+1) != pairs.end();)
    {
    const std::string& depender = *i++;
    const std::string& dependee = *i++;

    // If the depender is missing then delete the dependee to make
    // sure both will be regenerated.
    if(cmSystemTools::FileExists(dependee.c_str()) &&
       !cmSystemTools::FileExists(depender.c_str()))
      {
      if(verbose)
        {
        std::ostringstream msg;
        msg << "Deleting primary custom command output \"" << dependee
            << "\" because another output \""
            << depender << "\" does not exist." << std::endl;
        cmSystemTools::Stdout(msg.str().c_str());
        }
      cmSystemTools::RemoveFile(dependee);
      }
    }
}

//----------------------------------------------------------------------------
void cmLocalUnixMakefileGenerator3
::WriteLocalAllRules(std::ostream& ruleFileStream)
{
  this->WriteDisclaimer(ruleFileStream);

  // Write the main entry point target.  This must be the VERY first
  // target so that make with no arguments will run it.
  {
  // Just depend on the all target to drive the build.
  std::vector<std::string> depends;
  std::vector<std::string> no_commands;
  depends.push_back("all");

  // Write the rule.
  this->WriteMakeRule(ruleFileStream,
                      "Default target executed when no arguments are "
                      "given to make.",
                      "default_target",
                      depends,
                      no_commands, true);

  // Help out users that try "gmake target1 target2 -j".
  cmGlobalUnixMakefileGenerator3* gg =
    static_cast<cmGlobalUnixMakefileGenerator3*>(this->GlobalGenerator);
  if(gg->AllowNotParallel())
    {
    std::vector<std::string> no_depends;
    this->WriteMakeRule(ruleFileStream,
      "Allow only one \"make -f Makefile2\" at a time, but pass parallelism.",
      ".NOTPARALLEL", no_depends, no_commands, false);
    }
  }

  this->WriteSpecialTargetsTop(ruleFileStream);

  // Include the progress variables for the target.
  // Write all global targets
  this->WriteDivider(ruleFileStream);
  ruleFileStream
    << "# Targets provided globally by CMake.\n"
    << "\n";
  cmTargets* targets = &(this->Makefile->GetTargets());
  cmTargets::iterator glIt;
  for ( glIt = targets->begin(); glIt != targets->end(); ++ glIt )
    {
    if ( glIt->second.GetType() == cmTarget::GLOBAL_TARGET )
      {
      std::string targetString = "Special rule for the target " + glIt->first;
      std::vector<std::string> commands;
      std::vector<std::string> depends;

      const char* text = glIt->second.GetProperty("EchoString");
      if ( !text )
        {
        text = "Running external command ...";
        }
      depends.insert(depends.end(), glIt->second.GetUtilities().begin(),
                     glIt->second.GetUtilities().end());
      this->AppendEcho(commands, text,
                       cmLocalUnixMakefileGenerator3::EchoGlobal);

      // Global targets store their rules in pre- and post-build commands.
      this->AppendCustomDepends(depends,
                                glIt->second.GetPreBuildCommands());
      this->AppendCustomDepends(depends,
                                glIt->second.GetPostBuildCommands());
      this->AppendCustomCommands(commands,
                                 glIt->second.GetPreBuildCommands(),
                                 &glIt->second,
                                 cmLocalGenerator::START_OUTPUT);
      this->AppendCustomCommands(commands,
                                 glIt->second.GetPostBuildCommands(),
                                 &glIt->second,
                                 cmLocalGenerator::START_OUTPUT);
      std::string targetName = glIt->second.GetName();
      this->WriteMakeRule(ruleFileStream, targetString.c_str(),
                          targetName, depends, commands, true);

      // Provide a "/fast" version of the target.
      depends.clear();
      if((targetName == "install")
          || (targetName == "install_local")
          || (targetName == "install_strip"))
        {
        // Provide a fast install target that does not depend on all
        // but has the same command.
        depends.push_back("preinstall/fast");
        }
      else
        {
        // Just forward to the real target so at least it will work.
        depends.push_back(targetName);
        commands.clear();
        }
      targetName += "/fast";
      this->WriteMakeRule(ruleFileStream, targetString.c_str(),
                          targetName, depends, commands, true);
      }
    }

  std::vector<std::string> depends;
  std::vector<std::string> commands;

  // Write the all rule.
  std::string dir;
  std::string recursiveTarget = this->Makefile->GetCurrentBinaryDirectory();
  recursiveTarget += "/all";

  depends.push_back("cmake_check_build_system");

  std::string progressDir = this->Makefile->GetHomeOutputDirectory();
  progressDir += cmake::GetCMakeFilesDirectory();
    {
    std::ostringstream progCmd;
    progCmd <<
      "$(CMAKE_COMMAND) -E cmake_progress_start ";
    progCmd << this->Convert(progressDir,
                             cmLocalGenerator::FULL,
                             cmLocalGenerator::SHELL);

    std::string progressFile = cmake::GetCMakeFilesDirectory();
    progressFile += "/progress.marks";
    std::string progressFileNameFull =
      this->ConvertToFullPath(progressFile);
    progCmd << " " << this->Convert(progressFileNameFull,
                                    cmLocalGenerator::FULL,
                                    cmLocalGenerator::SHELL);
    commands.push_back(progCmd.str());
    }
  std::string mf2Dir = cmake::GetCMakeFilesDirectoryPostSlash();
  mf2Dir += "Makefile2";
  commands.push_back(this->GetRecursiveMakeCall(mf2Dir.c_str(),
                                                recursiveTarget));
  this->CreateCDCommand(commands,
                        this->Makefile->GetHomeOutputDirectory(),
                        cmLocalGenerator::START_OUTPUT);
    {
    std::ostringstream progCmd;
    progCmd << "$(CMAKE_COMMAND) -E cmake_progress_start "; // # 0
    progCmd << this->Convert(progressDir,
                             cmLocalGenerator::FULL,
                             cmLocalGenerator::SHELL);
    progCmd << " 0";
    commands.push_back(progCmd.str());
    }
  this->WriteMakeRule(ruleFileStream, "The main all target", "all",
                      depends, commands, true);

  // Write the clean rule.
  recursiveTarget = this->Makefile->GetCurrentBinaryDirectory();
  recursiveTarget += "/clean";
  commands.clear();
  depends.clear();
  commands.push_back(this->GetRecursiveMakeCall(mf2Dir.c_str(),
                                                recursiveTarget));
  this->CreateCDCommand(commands,
                                this->Makefile->GetHomeOutputDirectory(),
                                cmLocalGenerator::START_OUTPUT);
  this->WriteMakeRule(ruleFileStream, "The main clean target", "clean",
                      depends, commands, true);
  commands.clear();
  depends.clear();
  depends.push_back("clean");
  this->WriteMakeRule(ruleFileStream, "The main clean target", "clean/fast",
                      depends, commands, true);

  // Write the preinstall rule.
  recursiveTarget = this->Makefile->GetCurrentBinaryDirectory();
  recursiveTarget += "/preinstall";
  commands.clear();
  depends.clear();
  const char* noall =
    this->Makefile->GetDefinition("CMAKE_SKIP_INSTALL_ALL_DEPENDENCY");
  if(!noall || cmSystemTools::IsOff(noall))
    {
    // Drive the build before installing.
    depends.push_back("all");
    }
  else
    {
    // At least make sure the build system is up to date.
    depends.push_back("cmake_check_build_system");
    }
  commands.push_back
    (this->GetRecursiveMakeCall(mf2Dir.c_str(), recursiveTarget));
  this->CreateCDCommand(commands,
                        this->Makefile->GetHomeOutputDirectory(),
                        cmLocalGenerator::START_OUTPUT);
  this->WriteMakeRule(ruleFileStream, "Prepare targets for installation.",
                      "preinstall", depends, commands, true);
  depends.clear();
  this->WriteMakeRule(ruleFileStream, "Prepare targets for installation.",
                      "preinstall/fast", depends, commands, true);

  // write the depend rule, really a recompute depends rule
  depends.clear();
  commands.clear();
  std::string cmakefileName = cmake::GetCMakeFilesDirectoryPostSlash();
  cmakefileName += "Makefile.cmake";
  std::string runRule =
    "$(CMAKE_COMMAND) -H$(CMAKE_SOURCE_DIR) -B$(CMAKE_BINARY_DIR)";
  runRule += " --check-build-system ";
  runRule += this->Convert(cmakefileName,cmLocalGenerator::NONE,
                           cmLocalGenerator::SHELL);
  runRule += " 1";
  commands.push_back(runRule);
  this->CreateCDCommand(commands,
                        this->Makefile->GetHomeOutputDirectory(),
                        cmLocalGenerator::START_OUTPUT);
  this->WriteMakeRule(ruleFileStream, "clear depends",
                      "depend",
                      depends, commands, true);
}


//----------------------------------------------------------------------------
void cmLocalUnixMakefileGenerator3::ClearDependencies(cmMakefile* mf,
                                                      bool verbose)
{
  // Get the list of target files to check
  const char* infoDef = mf->GetDefinition("CMAKE_DEPEND_INFO_FILES");
  if(!infoDef)
    {
    return;
    }
  std::vector<std::string> files;
  cmSystemTools::ExpandListArgument(infoDef, files);

  // Each depend information file corresponds to a target.  Clear the
  // dependencies for that target.
  cmDepends clearer;
  clearer.SetVerbose(verbose);
  for(std::vector<std::string>::iterator l = files.begin();
      l != files.end(); ++l)
    {
    std::string dir = cmSystemTools::GetFilenamePath(*l);

    // Clear the implicit dependency makefile.
    std::string dependFile = dir + "/depend.make";
    clearer.Clear(dependFile.c_str());

    // Remove the internal dependency check file to force
    // regeneration.
    std::string internalDependFile = dir + "/depend.internal";
    cmSystemTools::RemoveFile(internalDependFile);
    }
}


void cmLocalUnixMakefileGenerator3
::WriteDependLanguageInfo(std::ostream& cmakefileStream, cmTarget &target)
{
  ImplicitDependLanguageMap const& implicitLangs =
    this->GetImplicitDepends(target);

  // list the languages
  cmakefileStream
    << "# The set of languages for which implicit dependencies are needed:\n";
  cmakefileStream
    << "set(CMAKE_DEPENDS_LANGUAGES\n";
  for(ImplicitDependLanguageMap::const_iterator
        l = implicitLangs.begin(); l != implicitLangs.end(); ++l)
    {
    cmakefileStream << "  \"" << l->first << "\"\n";
    }
  cmakefileStream << "  )\n";

  // now list the files for each language
  cmakefileStream
    << "# The set of files for implicit dependencies of each language:\n";
  for(ImplicitDependLanguageMap::const_iterator
        l = implicitLangs.begin(); l != implicitLangs.end(); ++l)
    {
    cmakefileStream
      << "set(CMAKE_DEPENDS_CHECK_" << l->first << "\n";
    ImplicitDependFileMap const& implicitPairs = l->second;

    // for each file pair
    for(ImplicitDependFileMap::const_iterator pi = implicitPairs.begin();
        pi != implicitPairs.end(); ++pi)
      {
      for(cmDepends::DependencyVector::const_iterator di = pi->second.begin();
          di != pi->second.end(); ++ di)
        {
        cmakefileStream << "  \"" << *di << "\" ";
        cmakefileStream << "\"" << pi->first << "\"\n";
        }
      }
    cmakefileStream << "  )\n";

    // Tell the dependency scanner what compiler is used.
    std::string cidVar = "CMAKE_";
    cidVar += l->first;
    cidVar += "_COMPILER_ID";
    const char* cid = this->Makefile->GetDefinition(cidVar);
    if(cid && *cid)
      {
      cmakefileStream
        << "set(CMAKE_" << l->first << "_COMPILER_ID \""
        << cid << "\")\n";
      }

    // Build a list of preprocessor definitions for the target.
    std::set<std::string> defines;
    this->AddCompileDefinitions(defines, &target,
                                this->ConfigurationName, l->first);
    if(!defines.empty())
      {
      cmakefileStream
        << "\n"
        << "# Preprocessor definitions for this target.\n"
        << "set(CMAKE_TARGET_DEFINITIONS_" << l->first << "\n";
      for(std::set<std::string>::const_iterator di = defines.begin();
          di != defines.end(); ++di)
        {
        cmakefileStream
          << "  " << cmLocalGenerator::EscapeForCMake(*di) << "\n";
        }
      cmakefileStream
        << "  )\n";
      }

    // Target-specific include directories:
    cmakefileStream
      << "\n"
      << "# The include file search paths:\n";
    cmakefileStream
      << "set(CMAKE_" << l->first << "_TARGET_INCLUDE_PATH\n";
    std::vector<std::string> includes;

    cmGeneratorTarget* gt = this->GetGlobalGenerator()
                                ->GetGeneratorTarget(&target);

    const std::string& config =
      this->Makefile->GetSafeDefinition("CMAKE_BUILD_TYPE");
    this->GetIncludeDirectories(includes, gt,
                                l->first, config);
    for(std::vector<std::string>::iterator i = includes.begin();
        i != includes.end(); ++i)
      {
      cmakefileStream
        << "  \""
        << this->Convert(*i, cmLocalGenerator::HOME_OUTPUT)
        << "\"\n";
      }
    cmakefileStream
      << "  )\n";
    }

  // Store include transform rule properties.  Write the directory
  // rules first because they may be overridden by later target rules.
  std::vector<std::string> transformRules;
  if(const char* xform =
     this->Makefile->GetProperty("IMPLICIT_DEPENDS_INCLUDE_TRANSFORM"))
    {
    cmSystemTools::ExpandListArgument(xform, transformRules);
    }
  if(const char* xform =
     target.GetProperty("IMPLICIT_DEPENDS_INCLUDE_TRANSFORM"))
    {
    cmSystemTools::ExpandListArgument(xform, transformRules);
    }
  if(!transformRules.empty())
    {
    cmakefileStream
      << "set(CMAKE_INCLUDE_TRANSFORMS\n";
    for(std::vector<std::string>::const_iterator tri = transformRules.begin();
        tri != transformRules.end(); ++tri)
      {
      cmakefileStream << "  "
          << cmLocalGenerator::EscapeForCMake(*tri) << "\n";
      }
    cmakefileStream
      << "  )\n";
    }
}

//----------------------------------------------------------------------------
void cmLocalUnixMakefileGenerator3::WriteDisclaimer(std::ostream& os)
{
  os
    << "# CMAKE generated file: DO NOT EDIT!\n"
    << "# Generated by \"" << this->GlobalGenerator->GetName() << "\""
    << " Generator, CMake Version "
    << cmVersion::GetMajorVersion() << "."
    << cmVersion::GetMinorVersion() << "\n\n";
}

//----------------------------------------------------------------------------
std::string
cmLocalUnixMakefileGenerator3
::GetRecursiveMakeCall(const char *makefile, const std::string& tgt)
{
  // Call make on the given file.
  std::string cmd;
  cmd += "$(MAKE) -f ";
  cmd += this->Convert(makefile,NONE,SHELL);
  cmd += " ";

  cmGlobalUnixMakefileGenerator3* gg =
    static_cast<cmGlobalUnixMakefileGenerator3*>(this->GlobalGenerator);
  // Pass down verbosity level.
  if(!gg->MakeSilentFlag.empty())
    {
    cmd += gg->MakeSilentFlag;
    cmd += " ";
    }

  // Most unix makes will pass the command line flags to make down to
  // sub-invoked makes via an environment variable.  However, some
  // makes do not support that, so you have to pass the flags
  // explicitly.
  if(gg->PassMakeflags)
    {
    cmd += "-$(MAKEFLAGS) ";
    }

  // Add the target.
  if (!tgt.empty())
    {
    // The make target is always relative to the top of the build tree.
    std::string tgt2 = this->Convert(tgt, HOME_OUTPUT);

    // The target may have been written with windows paths.
    cmSystemTools::ConvertToOutputSlashes(tgt2);

    // Escape one extra time if the make tool requires it.
    if(this->MakeCommandEscapeTargetTwice)
      {
      tgt2 = this->EscapeForShell(tgt2, true, false);
      }

    // The target name is now a string that should be passed verbatim
    // on the command line.
    cmd += this->EscapeForShell(tgt2, true, false);
    }
  return cmd;
}

//----------------------------------------------------------------------------
void cmLocalUnixMakefileGenerator3::WriteDivider(std::ostream& os)
{
  os
    << "#======================================"
    << "=======================================\n";
}

//----------------------------------------------------------------------------
void
cmLocalUnixMakefileGenerator3
::WriteCMakeArgument(std::ostream& os, const char* s)
{
  // Write the given string to the stream with escaping to get it back
  // into CMake through the lexical scanner.
  os << "\"";
  for(const char* c = s; *c; ++c)
    {
    if(*c == '\\')
      {
      os << "\\\\";
      }
    else if(*c == '"')
      {
      os << "\\\"";
      }
    else
      {
      os << *c;
      }
    }
  os << "\"";
}

//----------------------------------------------------------------------------
std::string
cmLocalUnixMakefileGenerator3::ConvertToQuotedOutputPath(const char* p,
                                                         bool useWatcomQuote)
{
  // Split the path into its components.
  std::vector<std::string> components;
  cmSystemTools::SplitPath(p, components);

  // Open the quoted result.
  std::string result;
  if(useWatcomQuote)
    {
#if defined(_WIN32) && !defined(__CYGWIN__)
    result = "'";
#else
    result = "\"'";
#endif
    }
  else
    {
    result = "\"";
    }

  // Return an empty path if there are no components.
  if(!components.empty())
    {
    // Choose a slash direction and fix root component.
    const char* slash = "/";
#if defined(_WIN32) && !defined(__CYGWIN__)
    if(!cmSystemTools::GetForceUnixPaths())
      {
      slash = "\\";
      for(std::string::iterator i = components[0].begin();
          i != components[0].end(); ++i)
        {
        if(*i == '/')
          {
          *i = '\\';
          }
        }
      }
#endif

    // Begin the quoted result with the root component.
    result += components[0];

    if (components.size() > 1)
      {
      // Now add the rest of the components separated by the proper slash
      // direction for this platform.
      std::vector<std::string>::const_iterator compEnd
          = std::remove(components.begin() + 1, components.end() - 1,
                          std::string());
      std::vector<std::string>::const_iterator compStart
          = components.begin() + 1;
      result += cmJoin(cmRange(compStart, compEnd), slash);
      // Only the last component can be empty to avoid double slashes.
      result += slash;
      result += components.back();
      }
    }

  // Close the quoted result.
  if(useWatcomQuote)
    {
#if defined(_WIN32) && !defined(__CYGWIN__)
    result += "'";
#else
    result += "'\"";
#endif
    }
  else
    {
    result += "\"";
    }

  return result;
}

//----------------------------------------------------------------------------
std::string
cmLocalUnixMakefileGenerator3
::GetTargetDirectory(cmTarget const& target) const
{
  std::string dir = cmake::GetCMakeFilesDirectoryPostSlash();
  dir += target.GetName();
#if defined(__VMS)
  dir += "_dir";
#else
  dir += ".dir";
#endif
  return dir;
}

//----------------------------------------------------------------------------
cmLocalUnixMakefileGenerator3::ImplicitDependLanguageMap const&
cmLocalUnixMakefileGenerator3::GetImplicitDepends(cmTarget const& tgt)
{
  return this->ImplicitDepends[tgt.GetName()];
}

//----------------------------------------------------------------------------
void
cmLocalUnixMakefileGenerator3::AddImplicitDepends(cmTarget const& tgt,
                                                  const std::string& lang,
                                                  const char* obj,
                                                  const char* src)
{
  this->ImplicitDepends[tgt.GetName()][lang][obj].push_back(src);
}

//----------------------------------------------------------------------------
void cmLocalUnixMakefileGenerator3
::CreateCDCommand(std::vector<std::string>& commands, const char *tgtDir,
                  cmLocalGenerator::RelativeRoot relRetDir)
{
  const char* retDir = this->GetRelativeRootPath(relRetDir);

  // do we need to cd?
  if (!strcmp(tgtDir,retDir))
    {
    return;
    }

  // In a Windows shell we must change drive letter too.  The shell
  // used by NMake and Borland make does not support "cd /d" so this
  // feature simply cannot work with them (Borland make does not even
  // support changing the drive letter with just "d:").
  const char* cd_cmd = this->IsMinGWMake() ? "cd /d " : "cd ";

  cmGlobalUnixMakefileGenerator3* gg =
    static_cast<cmGlobalUnixMakefileGenerator3*>(this->GlobalGenerator);
  if(!gg->UnixCD)
    {
    // On Windows we must perform each step separately and then change
    // back because the shell keeps the working directory between
    // commands.
    std::string cmd = cd_cmd;
    cmd += this->ConvertToOutputForExisting(tgtDir, relRetDir);
    commands.insert(commands.begin(),cmd);

    // Change back to the starting directory.
    cmd = cd_cmd;
    cmd += this->ConvertToOutputForExisting(relRetDir, tgtDir);
    commands.push_back(cmd);
    }
  else
    {
    // On UNIX we must construct a single shell command to change
    // directory and build because make resets the directory between
    // each command.
    std::string outputForExisting =
                          this->ConvertToOutputForExisting(tgtDir, relRetDir);
    std::string prefix = cd_cmd + outputForExisting + " && ";
    std::transform(commands.begin(), commands.end(), commands.begin(),
                   std::bind1st(std::plus<std::string>(), prefix));
    }
}
