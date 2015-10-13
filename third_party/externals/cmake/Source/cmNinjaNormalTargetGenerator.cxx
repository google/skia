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
#include "cmNinjaNormalTargetGenerator.h"
#include "cmLocalNinjaGenerator.h"
#include "cmGlobalNinjaGenerator.h"
#include "cmSourceFile.h"
#include "cmGeneratedFileStream.h"
#include "cmMakefile.h"
#include "cmOSXBundleGenerator.h"
#include "cmGeneratorTarget.h"
#include "cmCustomCommandGenerator.h"
#include "cmAlgorithms.h"

#include <assert.h>
#include <algorithm>
#include <limits>

#ifndef _WIN32
#include <unistd.h>
#endif


cmNinjaNormalTargetGenerator::
cmNinjaNormalTargetGenerator(cmGeneratorTarget* target)
  : cmNinjaTargetGenerator(target->Target)
  , TargetNameOut()
  , TargetNameSO()
  , TargetNameReal()
  , TargetNameImport()
  , TargetNamePDB()
  , TargetLinkLanguage("")
{
  this->TargetLinkLanguage = target->Target
                                   ->GetLinkerLanguage(this->GetConfigName());
  if (target->GetType() == cmTarget::EXECUTABLE)
    target->Target->GetExecutableNames(this->TargetNameOut,
                               this->TargetNameReal,
                               this->TargetNameImport,
                               this->TargetNamePDB,
                               GetLocalGenerator()->GetConfigName());
  else
    target->Target->GetLibraryNames(this->TargetNameOut,
                            this->TargetNameSO,
                            this->TargetNameReal,
                            this->TargetNameImport,
                            this->TargetNamePDB,
                            GetLocalGenerator()->GetConfigName());

  if(target->GetType() != cmTarget::OBJECT_LIBRARY)
    {
    // on Windows the output dir is already needed at compile time
    // ensure the directory exists (OutDir test)
    EnsureDirectoryExists(target->Target->GetDirectory(this->GetConfigName()));
    }

  this->OSXBundleGenerator = new cmOSXBundleGenerator(target,
                                                      this->GetConfigName());
  this->OSXBundleGenerator->SetMacContentFolders(&this->MacContentFolders);
}

cmNinjaNormalTargetGenerator::~cmNinjaNormalTargetGenerator()
{
  delete this->OSXBundleGenerator;
}

void cmNinjaNormalTargetGenerator::Generate()
{
  if (this->TargetLinkLanguage.empty()) {
    cmSystemTools::Error("CMake can not determine linker language for "
                         "target: ",
                         this->GetTarget()->GetName().c_str());
    return;
  }

  // Write the rules for each language.
  this->WriteLanguagesRules();

  // Write the build statements
  this->WriteObjectBuildStatements();

  if(this->GetTarget()->GetType() == cmTarget::OBJECT_LIBRARY)
    {
    this->WriteObjectLibStatement();
    }
  else
    {
    this->WriteLinkStatement();
    }
}

void cmNinjaNormalTargetGenerator::WriteLanguagesRules()
{
#ifdef NINJA_GEN_VERBOSE_FILES
  cmGlobalNinjaGenerator::WriteDivider(this->GetRulesFileStream());
  this->GetRulesFileStream()
    << "# Rules for each languages for "
    << cmTarget::GetTargetTypeName(this->GetTarget()->GetType())
    << " target "
    << this->GetTargetName()
    << "\n\n";
#endif

  // Write rules for languages compiled in this target.
  std::set<std::string> languages;
  std::vector<cmSourceFile*> sourceFiles;
  this->GetTarget()->GetSourceFiles(sourceFiles,
    this->GetMakefile()->GetSafeDefinition("CMAKE_BUILD_TYPE"));
  for(std::vector<cmSourceFile*>::const_iterator
        i = sourceFiles.begin(); i != sourceFiles.end(); ++i)
    {
    const std::string& lang = (*i)->GetLanguage();
    if(!lang.empty())
      {
      languages.insert(lang);
      }
    }
  for(std::set<std::string>::const_iterator l = languages.begin();
      l != languages.end();
      ++l)
    {
    this->WriteLanguageRules(*l);
    }
}

const char *cmNinjaNormalTargetGenerator::GetVisibleTypeName() const
{
  switch (this->GetTarget()->GetType()) {
    case cmTarget::STATIC_LIBRARY:
      return "static library";
    case cmTarget::SHARED_LIBRARY:
      return "shared library";
    case cmTarget::MODULE_LIBRARY:
      if (this->GetTarget()->IsCFBundleOnApple())
        return "CFBundle shared module";
      else
        return "shared module";
    case cmTarget::EXECUTABLE:
      return "executable";
    default:
      return 0;
  }
}

std::string
cmNinjaNormalTargetGenerator
::LanguageLinkerRule() const
{
  return this->TargetLinkLanguage
    + "_"
    + cmTarget::GetTargetTypeName(this->GetTarget()->GetType())
    + "_LINKER__"
    + cmGlobalNinjaGenerator::EncodeRuleName(this->GetTarget()->GetName())
    ;
}

void
cmNinjaNormalTargetGenerator
::WriteLinkRule(bool useResponseFile)
{
  cmTarget::TargetType targetType = this->GetTarget()->GetType();
  std::string ruleName = this->LanguageLinkerRule();

  // Select whether to use a response file for objects.
  std::string rspfile;
  std::string rspcontent;

  if (!this->GetGlobalGenerator()->HasRule(ruleName)) {
    cmLocalGenerator::RuleVariables vars;
    vars.RuleLauncher = "RULE_LAUNCH_LINK";
    vars.CMTarget = this->GetTarget();
    vars.Language = this->TargetLinkLanguage.c_str();

    std::string responseFlag;
    if (!useResponseFile) {
      vars.Objects = "$in";
      vars.LinkLibraries = "$LINK_PATH $LINK_LIBRARIES";
    } else {
        std::string cmakeVarLang = "CMAKE_";
        cmakeVarLang += this->TargetLinkLanguage;

        // build response file name
        std::string cmakeLinkVar =  cmakeVarLang + "_RESPONSE_FILE_LINK_FLAG";
        const char * flag = GetMakefile()->GetDefinition(cmakeLinkVar);
        if(flag) {
          responseFlag = flag;
        } else {
          responseFlag = "@";
        }
        rspfile = "$RSP_FILE";
        responseFlag += rspfile;

        // build response file content
        if (this->GetGlobalGenerator()->IsGCCOnWindows()) {
          rspcontent = "$in";
        } else {
          rspcontent = "$in_newline";
        }
        rspcontent += " $LINK_PATH $LINK_LIBRARIES";
        vars.Objects = responseFlag.c_str();
        vars.LinkLibraries = "";
    }

    vars.ObjectDir = "$OBJECT_DIR";

    vars.Target = "$TARGET_FILE";

    vars.SONameFlag = "$SONAME_FLAG";
    vars.TargetSOName = "$SONAME";
    vars.TargetInstallNameDir = "$INSTALLNAME_DIR";
    vars.TargetPDB = "$TARGET_PDB";

    // Setup the target version.
    std::string targetVersionMajor;
    std::string targetVersionMinor;
    {
    std::ostringstream majorStream;
    std::ostringstream minorStream;
    int major;
    int minor;
    this->GetTarget()->GetTargetVersion(major, minor);
    majorStream << major;
    minorStream << minor;
    targetVersionMajor = majorStream.str();
    targetVersionMinor = minorStream.str();
    }
    vars.TargetVersionMajor = targetVersionMajor.c_str();
    vars.TargetVersionMinor = targetVersionMinor.c_str();

    vars.Flags = "$FLAGS";
    vars.LinkFlags = "$LINK_FLAGS";

    std::string langFlags;
    if (targetType != cmTarget::EXECUTABLE)
      {
      langFlags += "$LANGUAGE_COMPILE_FLAGS $ARCH_FLAGS";
      vars.LanguageCompileFlags = langFlags.c_str();
      }

    // Rule for linking library/executable.
    std::vector<std::string> linkCmds = this->ComputeLinkCmd();
    for(std::vector<std::string>::iterator i = linkCmds.begin();
        i != linkCmds.end();
        ++i)
      {
      this->GetLocalGenerator()->ExpandRuleVariables(*i, vars);
      }
    linkCmds.insert(linkCmds.begin(), "$PRE_LINK");
    linkCmds.push_back("$POST_BUILD");
    std::string linkCmd =
      this->GetLocalGenerator()->BuildCommandLine(linkCmds);

    // Write the linker rule with response file if needed.
    std::ostringstream comment;
    comment << "Rule for linking " << this->TargetLinkLanguage << " "
            << this->GetVisibleTypeName() << ".";
    std::ostringstream description;
    description << "Linking " << this->TargetLinkLanguage << " "
                << this->GetVisibleTypeName() << " $TARGET_FILE";
    this->GetGlobalGenerator()->AddRule(ruleName,
                                        linkCmd,
                                        description.str(),
                                        comment.str(),
                                        /*depfile*/ "",
                                        /*deptype*/ "",
                                        rspfile,
                                        rspcontent,
                                        /*restat*/ "$RESTAT",
                                        /*generator*/ false);
  }

  if (this->TargetNameOut != this->TargetNameReal &&
    !this->GetTarget()->IsFrameworkOnApple()) {
    std::string cmakeCommand =
      this->GetLocalGenerator()->ConvertToOutputFormat(
        cmSystemTools::GetCMakeCommand(), cmLocalGenerator::SHELL);
    if (targetType == cmTarget::EXECUTABLE)
      this->GetGlobalGenerator()->AddRule("CMAKE_SYMLINK_EXECUTABLE",
                                          cmakeCommand +
                                          " -E cmake_symlink_executable"
                                          " $in $out && $POST_BUILD",
                                          "Creating executable symlink $out",
                                          "Rule for creating "
                                          "executable symlink.",
                                          /*depfile*/ "",
                                          /*deptype*/ "",
                                          /*rspfile*/ "",
                                          /*rspcontent*/ "",
                                          /*restat*/ "",
                                          /*generator*/ false);
    else
      this->GetGlobalGenerator()->AddRule("CMAKE_SYMLINK_LIBRARY",
                                          cmakeCommand +
                                          " -E cmake_symlink_library"
                                          " $in $SONAME $out && $POST_BUILD",
                                          "Creating library symlink $out",
                                          "Rule for creating "
                                          "library symlink.",
                                          /*depfile*/ "",
                                          /*deptype*/ "",
                                          /*rspfile*/ "",
                                          /*rspcontent*/ "",
                                          /*restat*/ "",
                                          /*generator*/ false);
  }
}

std::vector<std::string>
cmNinjaNormalTargetGenerator
::ComputeLinkCmd()
{
  std::vector<std::string> linkCmds;
  cmMakefile* mf = this->GetMakefile();
  {
  std::string linkCmdVar = this->GetGeneratorTarget()
    ->GetCreateRuleVariable(this->TargetLinkLanguage, this->GetConfigName());
  const char *linkCmd = mf->GetDefinition(linkCmdVar);
  if (linkCmd)
    {
    cmSystemTools::ExpandListArgument(linkCmd, linkCmds);
    return linkCmds;
    }
  }
  switch (this->GetTarget()->GetType()) {
    case cmTarget::STATIC_LIBRARY: {
      // We have archive link commands set. First, delete the existing archive.
      {
      std::string cmakeCommand =
        this->GetLocalGenerator()->ConvertToOutputFormat(
          cmSystemTools::GetCMakeCommand(), cmLocalGenerator::SHELL);
      linkCmds.push_back(cmakeCommand + " -E remove $TARGET_FILE");
      }
      // TODO: Use ARCHIVE_APPEND for archives over a certain size.
      {
      std::string linkCmdVar = "CMAKE_";
      linkCmdVar += this->TargetLinkLanguage;
      linkCmdVar += "_ARCHIVE_CREATE";
      const char *linkCmd = mf->GetRequiredDefinition(linkCmdVar);
      cmSystemTools::ExpandListArgument(linkCmd, linkCmds);
      }
      {
      std::string linkCmdVar = "CMAKE_";
      linkCmdVar += this->TargetLinkLanguage;
      linkCmdVar += "_ARCHIVE_FINISH";
      const char *linkCmd = mf->GetRequiredDefinition(linkCmdVar);
      cmSystemTools::ExpandListArgument(linkCmd, linkCmds);
      }
      return linkCmds;
    }
    case cmTarget::SHARED_LIBRARY:
    case cmTarget::MODULE_LIBRARY:
    case cmTarget::EXECUTABLE:
      break;
    default:
      assert(0 && "Unexpected target type");
  }
  return std::vector<std::string>();
}


static int calculateCommandLineLengthLimit(int linkRuleLength)
{
  static int const limits[] = {
#ifdef _WIN32
    8000,
#endif
#if defined(__APPLE__) || defined(__HAIKU__) || defined(__linux)
    // for instance ARG_MAX is 2096152 on Ubuntu or 262144 on Mac
    ((int)sysconf(_SC_ARG_MAX)) - 1000,
#endif
#if defined(__linux)
    // #define MAX_ARG_STRLEN (PAGE_SIZE * 32) in Linux's binfmts.h
    ((int)sysconf(_SC_PAGESIZE) * 32) - 1000,
#endif
    std::numeric_limits<int>::max()
  };

  size_t const arrSz = cmArraySize(limits);
  int const sz = *std::min_element(limits, limits + arrSz);
  if (sz == std::numeric_limits<int>::max())
    {
    return -1;
    }

  return sz - linkRuleLength;
}


void cmNinjaNormalTargetGenerator::WriteLinkStatement()
{
  cmTarget& target = *this->GetTarget();
  const std::string cfgName = this->GetConfigName();
  std::string targetOutput = ConvertToNinjaPath(
                               target.GetFullPath(cfgName));
  std::string targetOutputReal = ConvertToNinjaPath(
                                   target.GetFullPath(cfgName,
                                      /*implib=*/false,
                                      /*realpath=*/true));
  std::string targetOutputImplib = ConvertToNinjaPath(
                                     target.GetFullPath(cfgName,
                                       /*implib=*/true));

  if (target.IsAppBundleOnApple())
    {
    // Create the app bundle
    std::string outpath = target.GetDirectory(cfgName);
    this->OSXBundleGenerator->CreateAppBundle(this->TargetNameOut, outpath);

    // Calculate the output path
    targetOutput = outpath;
    targetOutput += "/";
    targetOutput += this->TargetNameOut;
    targetOutput = this->ConvertToNinjaPath(targetOutput);
    targetOutputReal = outpath;
    targetOutputReal += "/";
    targetOutputReal += this->TargetNameReal;
    targetOutputReal = this->ConvertToNinjaPath(targetOutputReal);
    }
  else if (target.IsFrameworkOnApple())
    {
    // Create the library framework.
    this->OSXBundleGenerator->CreateFramework(this->TargetNameOut,
                                              target.GetDirectory(cfgName));
    }
  else if(target.IsCFBundleOnApple())
    {
    // Create the core foundation bundle.
    this->OSXBundleGenerator->CreateCFBundle(this->TargetNameOut,
                                             target.GetDirectory(cfgName));
    }

  // Write comments.
  cmGlobalNinjaGenerator::WriteDivider(this->GetBuildFileStream());
  const cmTarget::TargetType targetType = target.GetType();
  this->GetBuildFileStream()
    << "# Link build statements for "
    << cmTarget::GetTargetTypeName(targetType)
    << " target "
    << this->GetTargetName()
    << "\n\n";

  cmNinjaDeps emptyDeps;
  cmNinjaVars vars;

  // Compute the comment.
  std::ostringstream comment;
  comment <<
    "Link the " << this->GetVisibleTypeName() << " " << targetOutputReal;

  // Compute outputs.
  cmNinjaDeps outputs;
  outputs.push_back(targetOutputReal);

  // Compute specific libraries to link with.
  cmNinjaDeps explicitDeps = this->GetObjects();
  cmNinjaDeps implicitDeps = this->ComputeLinkDeps();

  cmMakefile* mf = this->GetMakefile();

  std::string frameworkPath;
  std::string linkPath;
  cmGeneratorTarget& genTarget = *this->GetGeneratorTarget();

  std::string createRule =
    genTarget.GetCreateRuleVariable(this->TargetLinkLanguage,
                                    this->GetConfigName());
  bool useWatcomQuote = mf->IsOn(createRule+"_USE_WATCOM_QUOTE");
  cmLocalNinjaGenerator& localGen = *this->GetLocalGenerator();

  vars["TARGET_FILE"] =
    localGen.ConvertToOutputFormat(targetOutputReal, cmLocalGenerator::SHELL);

  localGen.GetTargetFlags(vars["LINK_LIBRARIES"],
                          vars["FLAGS"],
                          vars["LINK_FLAGS"],
                          frameworkPath,
                          linkPath,
                          &genTarget,
                          useWatcomQuote);

  this->addPoolNinjaVariable("JOB_POOL_LINK", &target, vars);

  this->AddModuleDefinitionFlag(vars["LINK_FLAGS"]);
  vars["LINK_FLAGS"] = cmGlobalNinjaGenerator
                        ::EncodeLiteral(vars["LINK_FLAGS"]);

  vars["LINK_PATH"] = frameworkPath + linkPath;

  // Compute architecture specific link flags.  Yes, these go into a different
  // variable for executables, probably due to a mistake made when duplicating
  // code between the Makefile executable and library generators.
  if (targetType == cmTarget::EXECUTABLE)
    {
    std::string t = vars["FLAGS"];
    localGen.AddArchitectureFlags(t, &genTarget, TargetLinkLanguage, cfgName);
    vars["FLAGS"] = t;
    }
  else
    {
    std::string t = vars["ARCH_FLAGS"];
    localGen.AddArchitectureFlags(t, &genTarget, TargetLinkLanguage, cfgName);
    vars["ARCH_FLAGS"] = t;
    t = "";
    localGen.AddLanguageFlags(t, TargetLinkLanguage, cfgName);
    vars["LANGUAGE_COMPILE_FLAGS"] = t;
    }

  if (target.HasSOName(cfgName))
    {
    vars["SONAME_FLAG"] = mf->GetSONameFlag(this->TargetLinkLanguage);
    vars["SONAME"] = this->TargetNameSO;
    if (targetType == cmTarget::SHARED_LIBRARY)
      {
      std::string install_dir = target.GetInstallNameDirForBuildTree(cfgName);
      if (!install_dir.empty())
        {
        vars["INSTALLNAME_DIR"] = localGen.Convert(install_dir,
                                                   cmLocalGenerator::NONE,
                                                   cmLocalGenerator::SHELL);
        }
      }
    }

  if (!this->TargetNameImport.empty())
    {
    const std::string impLibPath = localGen.ConvertToOutputFormat(
                                              targetOutputImplib,
                                              cmLocalGenerator::SHELL);
    vars["TARGET_IMPLIB"] = impLibPath;
    EnsureParentDirectoryExists(impLibPath);
    if(target.HasImportLibrary())
      {
      outputs.push_back(targetOutputImplib);
      }
    }

  if (!this->SetMsvcTargetPdbVariable(vars))
    {
    // It is common to place debug symbols at a specific place,
    // so we need a plain target name in the rule available.
    std::string prefix;
    std::string base;
    std::string suffix;
    target.GetFullNameComponents(prefix, base, suffix);
    std::string dbg_suffix = ".dbg";
    // TODO: Where to document?
    if (mf->GetDefinition("CMAKE_DEBUG_SYMBOL_SUFFIX"))
      {
      dbg_suffix = mf->GetDefinition("CMAKE_DEBUG_SYMBOL_SUFFIX");
      }
    vars["TARGET_PDB"] = base + suffix + dbg_suffix;
    }

  if (this->GetGlobalGenerator()->IsGCCOnWindows())
    {
    const std::string objPath = GetTarget()->GetSupportDirectory();
    vars["OBJECT_DIR"] = ConvertToNinjaPath(objPath);
    EnsureDirectoryExists(objPath);
    // ar.exe can't handle backslashes in rsp files (implicitly used by gcc)
    std::string& linkLibraries = vars["LINK_LIBRARIES"];
    std::replace(linkLibraries.begin(), linkLibraries.end(), '\\', '/');
    std::string& link_path = vars["LINK_PATH"];
    std::replace(link_path.begin(), link_path.end(), '\\', '/');
    }

  const std::vector<cmCustomCommand> *cmdLists[3] = {
    &target.GetPreBuildCommands(),
    &target.GetPreLinkCommands(),
    &target.GetPostBuildCommands()
  };

  std::vector<std::string> preLinkCmdLines, postBuildCmdLines;
  std::vector<std::string> *cmdLineLists[3] = {
    &preLinkCmdLines,
    &preLinkCmdLines,
    &postBuildCmdLines
  };

  cmNinjaDeps byproducts;
  for (unsigned i = 0; i != 3; ++i)
    {
    for (std::vector<cmCustomCommand>::const_iterator
         ci = cmdLists[i]->begin();
         ci != cmdLists[i]->end(); ++ci)
      {
      cmCustomCommandGenerator ccg(*ci, cfgName, mf);
      localGen.AppendCustomCommandLines(ccg, *cmdLineLists[i]);
      std::vector<std::string> const& ccByproducts = ccg.GetByproducts();
      std::transform(ccByproducts.begin(), ccByproducts.end(),
                     std::back_inserter(byproducts), MapToNinjaPath());
      }
    }

  // If we have any PRE_LINK commands, we need to go back to HOME_OUTPUT for
  // the link commands.
  if (!preLinkCmdLines.empty())
    {
    const std::string homeOutDir = localGen.ConvertToOutputFormat(
                                              mf->GetHomeOutputDirectory(),
                                              cmLocalGenerator::SHELL);
    preLinkCmdLines.push_back("cd " + homeOutDir);
    }

  vars["PRE_LINK"] = localGen.BuildCommandLine(preLinkCmdLines);
  std::string postBuildCmdLine = localGen.BuildCommandLine(postBuildCmdLines);

  cmNinjaVars symlinkVars;
  if (targetOutput == targetOutputReal)
    {
    vars["POST_BUILD"] = postBuildCmdLine;
    }
  else
    {
    vars["POST_BUILD"] = ":";
    symlinkVars["POST_BUILD"] = postBuildCmdLine;
    }

  cmGlobalNinjaGenerator& globalGen = *this->GetGlobalGenerator();

  int commandLineLengthLimit = 1;
  const char* forceRspFile = "CMAKE_NINJA_FORCE_RESPONSE_FILE";
  if (!mf->IsDefinitionSet(forceRspFile) &&
      cmSystemTools::GetEnv(forceRspFile) == 0)
    {
    commandLineLengthLimit = calculateCommandLineLengthLimit(
                globalGen.GetRuleCmdLength(this->LanguageLinkerRule()));
    }

  const std::string rspfile =
      std::string(cmake::GetCMakeFilesDirectoryPostSlash())
      + target.GetName() + ".rsp";

  // Gather order-only dependencies.
  cmNinjaDeps orderOnlyDeps;
  this->GetLocalGenerator()->AppendTargetDepends(this->GetTarget(),
    orderOnlyDeps);

  // Ninja should restat after linking if and only if there are byproducts.
  vars["RESTAT"] = byproducts.empty()? "" : "1";

  for (cmNinjaDeps::const_iterator oi = byproducts.begin(),
         oe = byproducts.end();
       oi != oe; ++oi)
    {
    this->GetGlobalGenerator()->SeenCustomCommandOutput(*oi);
    outputs.push_back(*oi);
    }

  // Write the build statement for this target.
  bool usedResponseFile = false;
  globalGen.WriteBuild(this->GetBuildFileStream(),
                        comment.str(),
                        this->LanguageLinkerRule(),
                        outputs,
                        explicitDeps,
                        implicitDeps,
                        orderOnlyDeps,
                        vars,
                        rspfile,
                        commandLineLengthLimit,
                        &usedResponseFile);
  this->WriteLinkRule(usedResponseFile);

  if (targetOutput != targetOutputReal && !target.IsFrameworkOnApple())
    {
    if (targetType == cmTarget::EXECUTABLE)
      {
      globalGen.WriteBuild(this->GetBuildFileStream(),
                            "Create executable symlink " + targetOutput,
                            "CMAKE_SYMLINK_EXECUTABLE",
                            cmNinjaDeps(1, targetOutput),
                            cmNinjaDeps(1, targetOutputReal),
                            emptyDeps,
                            emptyDeps,
                            symlinkVars);
      }
   else
     {
      cmNinjaDeps symlinks;
      const std::string soName = this->GetTargetFilePath(this->TargetNameSO);
      // If one link has to be created.
      if (targetOutputReal == soName || targetOutput == soName)
        {
        symlinkVars["SONAME"] = soName;
        }
      else
        {
        symlinkVars["SONAME"] = "";
        symlinks.push_back(soName);
        }
      symlinks.push_back(targetOutput);
      globalGen.WriteBuild(this->GetBuildFileStream(),
                                  "Create library symlink " + targetOutput,
                                     "CMAKE_SYMLINK_LIBRARY",
                                  symlinks,
                                  cmNinjaDeps(1, targetOutputReal),
                                  emptyDeps,
                                  emptyDeps,
                                  symlinkVars);
      }
    }

  // Add aliases for the file name and the target name.
  globalGen.AddTargetAlias(this->TargetNameOut, &target);
  globalGen.AddTargetAlias(this->GetTargetName(), &target);
}

//----------------------------------------------------------------------------
void cmNinjaNormalTargetGenerator::WriteObjectLibStatement()
{
  // Write a phony output that depends on all object files.
  cmNinjaDeps outputs;
  this->GetLocalGenerator()->AppendTargetOutputs(this->GetTarget(), outputs);
  cmNinjaDeps depends = this->GetObjects();
  this->GetGlobalGenerator()->WritePhonyBuild(this->GetBuildFileStream(),
                                              "Object library "
                                                + this->GetTargetName(),
                                              outputs,
                                              depends);

  // Add aliases for the target name.
  this->GetGlobalGenerator()->AddTargetAlias(this->GetTargetName(),
                                             this->GetTarget());
}
