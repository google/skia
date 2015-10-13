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
#include "cmLocalNinjaGenerator.h"
#include "cmCustomCommandGenerator.h"
#include "cmMakefile.h"
#include "cmGlobalNinjaGenerator.h"
#include "cmNinjaTargetGenerator.h"
#include "cmGeneratedFileStream.h"
#include "cmSourceFile.h"
#include "cmake.h"
#include "cmState.h"

#include <assert.h>

cmLocalNinjaGenerator::cmLocalNinjaGenerator(cmGlobalGenerator* gg,
                                             cmLocalGenerator* parent,
                                             cmState::Snapshot snapshot)
  : cmLocalGenerator(gg, parent, snapshot)
  , ConfigName("")
  , HomeRelativeOutputPath("")
{
  this->TargetImplib = "$TARGET_IMPLIB";
}

//----------------------------------------------------------------------------
// Virtual public methods.

cmLocalNinjaGenerator::~cmLocalNinjaGenerator()
{
}

void cmLocalNinjaGenerator::Generate()
{
  this->SetConfigName();

  this->WriteProcessedMakefile(this->GetBuildFileStream());
#ifdef NINJA_GEN_VERBOSE_FILES
  this->WriteProcessedMakefile(this->GetRulesFileStream());
#endif

  // We do that only once for the top CMakeLists.txt file.
  if(this->IsRootMakefile())
    {
    this->WriteBuildFileTop();

    this->WritePools(this->GetRulesFileStream());

    const std::string showIncludesPrefix = this->GetMakefile()
             ->GetSafeDefinition("CMAKE_CL_SHOWINCLUDES_PREFIX");
    if (!showIncludesPrefix.empty())
      {
      cmGlobalNinjaGenerator::WriteComment(this->GetRulesFileStream(),
                                           "localized /showIncludes string");
      this->GetRulesFileStream()
            << "msvc_deps_prefix = " << showIncludesPrefix << "\n\n";
      }
    }

  cmGeneratorTargetsType targets = this->GetMakefile()->GetGeneratorTargets();
  for(cmGeneratorTargetsType::iterator t = targets.begin();
      t != targets.end(); ++t)
    {
    if (t->second->Target->GetType() == cmTarget::INTERFACE_LIBRARY
        || t->second->Target->IsImported())
      {
      continue;
      }
    cmNinjaTargetGenerator* tg = cmNinjaTargetGenerator::New(t->second);
    if(tg)
      {
      tg->Generate();
      // Add the target to "all" if required.
      if (!this->GetGlobalNinjaGenerator()->IsExcluded(
            this->GetGlobalNinjaGenerator()->GetLocalGenerators()[0],
            *t->second->Target))
        this->GetGlobalNinjaGenerator()->AddDependencyToAll(t->second->Target);
      delete tg;
      }
    }

  this->WriteCustomCommandBuildStatements();
}

// Implemented in:
//   cmLocalUnixMakefileGenerator3.
// Used in:
//   Source/cmMakefile.cxx
//   Source/cmGlobalGenerator.cxx
void cmLocalNinjaGenerator::Configure()
{
  // Compute the path to use when referencing the current output
  // directory from the top output directory.
  this->HomeRelativeOutputPath =
    this->Convert(this->Makefile->GetCurrentBinaryDirectory(), HOME_OUTPUT);
  if(this->HomeRelativeOutputPath == ".")
    {
    this->HomeRelativeOutputPath = "";
    }
  this->cmLocalGenerator::Configure();

}

// TODO: Picked up from cmLocalUnixMakefileGenerator3.  Refactor it.
std::string cmLocalNinjaGenerator
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
// Non-virtual public methods.

const cmGlobalNinjaGenerator*
cmLocalNinjaGenerator::GetGlobalNinjaGenerator() const
{
  return
    static_cast<const cmGlobalNinjaGenerator*>(this->GetGlobalGenerator());
}

cmGlobalNinjaGenerator* cmLocalNinjaGenerator::GetGlobalNinjaGenerator()
{
  return static_cast<cmGlobalNinjaGenerator*>(this->GetGlobalGenerator());
}

//----------------------------------------------------------------------------
// Virtual protected methods.

std::string
cmLocalNinjaGenerator::ConvertToLinkReference(std::string const& lib,
                                              OutputFormat format)
{
  return this->Convert(lib, HOME_OUTPUT, format);
}

std::string
cmLocalNinjaGenerator::ConvertToIncludeReference(std::string const& path,
                                                 OutputFormat format,
                                                 bool forceFullPaths)
{
  return this->Convert(path, forceFullPaths? FULL : HOME_OUTPUT, format);
}

//----------------------------------------------------------------------------
// Private methods.

cmGeneratedFileStream& cmLocalNinjaGenerator::GetBuildFileStream() const
{
  return *this->GetGlobalNinjaGenerator()->GetBuildFileStream();
}

cmGeneratedFileStream& cmLocalNinjaGenerator::GetRulesFileStream() const
{
  return *this->GetGlobalNinjaGenerator()->GetRulesFileStream();
}

const cmake* cmLocalNinjaGenerator::GetCMakeInstance() const
{
  return this->GetGlobalGenerator()->GetCMakeInstance();
}

cmake* cmLocalNinjaGenerator::GetCMakeInstance()
{
  return this->GetGlobalGenerator()->GetCMakeInstance();
}

void cmLocalNinjaGenerator::WriteBuildFileTop()
{
  // For the build file.
  this->WriteProjectHeader(this->GetBuildFileStream());
  this->WriteNinjaRequiredVersion(this->GetBuildFileStream());
  this->WriteNinjaFilesInclusion(this->GetBuildFileStream());

  // For the rule file.
  this->WriteProjectHeader(this->GetRulesFileStream());
}

void cmLocalNinjaGenerator::WriteProjectHeader(std::ostream& os)
{
  cmGlobalNinjaGenerator::WriteDivider(os);
  os
    << "# Project: " << this->GetMakefile()->GetProjectName() << std::endl
    << "# Configuration: " << this->ConfigName << std::endl
    ;
  cmGlobalNinjaGenerator::WriteDivider(os);
}

void cmLocalNinjaGenerator::WriteNinjaRequiredVersion(std::ostream& os)
{
  // Default required version
  // Ninja generator uses 'deps' and 'msvc_deps_prefix' introduced in 1.3
  std::string requiredVersion = "1.3";

  // Ninja generator uses the 'console' pool if available (>= 1.5)
  std::string usedVersion = this->GetGlobalNinjaGenerator()->ninjaVersion();
  if(cmSystemTools::VersionCompare(cmSystemTools::OP_LESS,
                                   usedVersion.c_str(),
                                   "1.5") ==  false)
    {
      requiredVersion = "1.5";
    }

  cmGlobalNinjaGenerator::WriteComment(os,
                          "Minimal version of Ninja required by this file");
  os
    << "ninja_required_version = "
    << requiredVersion
    << std::endl << std::endl
    ;
}

void cmLocalNinjaGenerator::WritePools(std::ostream& os)
{
  cmGlobalNinjaGenerator::WriteDivider(os);

  const char* jobpools = this->GetCMakeInstance()->GetState()
                             ->GetGlobalProperty("JOB_POOLS");
  if (jobpools)
    {
    cmGlobalNinjaGenerator::WriteComment(os,
                            "Pools defined by global property JOB_POOLS");
    std::vector<std::string> pools;
    cmSystemTools::ExpandListArgument(jobpools, pools);
    for (size_t i = 0; i < pools.size(); ++i)
      {
      const std::string pool = pools[i];
      const std::string::size_type eq = pool.find("=");
      unsigned int jobs;
      if (eq != std::string::npos &&
          sscanf(pool.c_str() + eq, "=%u", &jobs) == 1)
        {
        os << "pool " << pool.substr(0, eq) << std::endl;
        os << "  depth = " << jobs << std::endl;
        os << std::endl;
        }
      else
        {
        cmSystemTools::Error("Invalid pool defined by property 'JOB_POOLS': ",
                             pool.c_str());
        }
      }
    }
}

void cmLocalNinjaGenerator::WriteNinjaFilesInclusion(std::ostream& os)
{
  cmGlobalNinjaGenerator::WriteDivider(os);
  os
    << "# Include auxiliary files.\n"
    << "\n"
    ;
  cmGlobalNinjaGenerator::WriteInclude(os,
                                      cmGlobalNinjaGenerator::NINJA_RULES_FILE,
                                       "Include rules file.");
  os << "\n";
}

void cmLocalNinjaGenerator::SetConfigName()
{
  // Store the configuration name that will be generated.
  if(const char* config =
       this->GetMakefile()->GetDefinition("CMAKE_BUILD_TYPE"))
    {
    // Use the build type given by the user.
    this->ConfigName = config;
    }
  else
    {
    // No configuration type given.
    this->ConfigName = "";
    }
}

//----------------------------------------------------------------------------
void cmLocalNinjaGenerator::ComputeObjectFilenames(
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

void cmLocalNinjaGenerator::WriteProcessedMakefile(std::ostream& os)
{
  cmGlobalNinjaGenerator::WriteDivider(os);
  os
    << "# Write statements declared in CMakeLists.txt:" << std::endl
    << "# "
    << this->Makefile->GetDefinition("CMAKE_CURRENT_LIST_FILE") << std::endl;
  if(this->IsRootMakefile())
    os << "# Which is the root file." << std::endl;
  cmGlobalNinjaGenerator::WriteDivider(os);
  os << std::endl;
}

std::string cmLocalNinjaGenerator::ConvertToNinjaPath(const std::string& path)
{
  std::string convPath = this->Convert(path, cmLocalGenerator::HOME_OUTPUT);
#ifdef _WIN32
  cmSystemTools::ReplaceString(convPath, "/", "\\");
#endif
  return convPath;
}

void
cmLocalNinjaGenerator
::AppendTargetOutputs(cmTarget* target, cmNinjaDeps& outputs)
{
  this->GetGlobalNinjaGenerator()->AppendTargetOutputs(target, outputs);
}

void
cmLocalNinjaGenerator
::AppendTargetDepends(cmTarget* target, cmNinjaDeps& outputs)
{
  this->GetGlobalNinjaGenerator()->AppendTargetDepends(target, outputs);
}

void cmLocalNinjaGenerator::AppendCustomCommandDeps(
  cmCustomCommandGenerator const& ccg,
  cmNinjaDeps &ninjaDeps)
{
  const std::vector<std::string> &deps = ccg.GetDepends();
  for (std::vector<std::string>::const_iterator i = deps.begin();
       i != deps.end(); ++i) {
    std::string dep;
    if (this->GetRealDependency(*i, this->GetConfigName(), dep))
      ninjaDeps.push_back(ConvertToNinjaPath(dep));
  }
}

std::string cmLocalNinjaGenerator::BuildCommandLine(
                                    const std::vector<std::string> &cmdLines)
{
  // If we have no commands but we need to build a command anyway, use ":".
  // This happens when building a POST_BUILD value for link targets that
  // don't use POST_BUILD.
  if (cmdLines.empty())
#ifdef _WIN32
    return "cd .";
#else
    return ":";
#endif

  std::ostringstream cmd;
  for (std::vector<std::string>::const_iterator li = cmdLines.begin();
       li != cmdLines.end(); ++li)
#ifdef _WIN32
    {
    if (li != cmdLines.begin())
      {
      cmd << " && ";
      }
    else if (cmdLines.size() > 1)
      {
      cmd << "cmd.exe /C \"";
      }
    cmd << *li;
    }
  if (cmdLines.size() > 1)
    {
    cmd << "\"";
    }
#else
    {
    if (li != cmdLines.begin())
      {
      cmd << " && ";
      }
    cmd << *li;
    }
#endif
  return cmd.str();
}

void cmLocalNinjaGenerator::AppendCustomCommandLines(
  cmCustomCommandGenerator const& ccg,
  std::vector<std::string> &cmdLines)
{
  if (ccg.GetNumberOfCommands() > 0) {
    std::string wd = ccg.GetWorkingDirectory();
    if (wd.empty())
      wd = this->GetMakefile()->GetCurrentBinaryDirectory();

    std::ostringstream cdCmd;
#ifdef _WIN32
        std::string cdStr = "cd /D ";
#else
        std::string cdStr = "cd ";
#endif
    cdCmd << cdStr << this->ConvertToOutputFormat(wd, SHELL);
    cmdLines.push_back(cdCmd.str());
  }

  std::string launcher = this->MakeCustomLauncher(ccg);

  for (unsigned i = 0; i != ccg.GetNumberOfCommands(); ++i) {
    cmdLines.push_back(launcher +
      this->ConvertToOutputFormat(ccg.GetCommand(i), SHELL));

    std::string& cmd = cmdLines.back();
    ccg.AppendArguments(i, cmd);
  }
}

void
cmLocalNinjaGenerator::WriteCustomCommandBuildStatement(
  cmCustomCommand const *cc, const cmNinjaDeps& orderOnlyDeps)
{
  if (this->GetGlobalNinjaGenerator()->SeenCustomCommand(cc))
    return;

  cmCustomCommandGenerator ccg(*cc, this->GetConfigName(), this->Makefile);

  const std::vector<std::string> &outputs = ccg.GetOutputs();
  const std::vector<std::string> &byproducts = ccg.GetByproducts();
  cmNinjaDeps ninjaOutputs(outputs.size()+byproducts.size()), ninjaDeps;

#if 0
#error TODO: Once CC in an ExternalProject target must provide the \
    file of each imported target that has an add_dependencies pointing \
    at us.  How to know which ExternalProject step actually provides it?
#endif
  std::transform(outputs.begin(), outputs.end(),
                 ninjaOutputs.begin(), MapToNinjaPath());
  std::transform(byproducts.begin(), byproducts.end(),
                 ninjaOutputs.begin() + outputs.size(), MapToNinjaPath());
  this->AppendCustomCommandDeps(ccg, ninjaDeps);

  for (cmNinjaDeps::iterator i = ninjaOutputs.begin(); i != ninjaOutputs.end();
       ++i)
    this->GetGlobalNinjaGenerator()->SeenCustomCommandOutput(*i);

  std::vector<std::string> cmdLines;
  this->AppendCustomCommandLines(ccg, cmdLines);

  if (cmdLines.empty()) {
    this->GetGlobalNinjaGenerator()->WritePhonyBuild(
      this->GetBuildFileStream(),
      "Phony custom command for " +
      ninjaOutputs[0],
      ninjaOutputs,
      ninjaDeps,
      cmNinjaDeps(),
      orderOnlyDeps,
      cmNinjaVars());
  } else {
    this->GetGlobalNinjaGenerator()->WriteCustomCommandBuild(
      this->BuildCommandLine(cmdLines),
      this->ConstructComment(ccg),
      "Custom command for " + ninjaOutputs[0],
      cc->GetUsesTerminal(),
      ninjaOutputs,
      ninjaDeps,
      orderOnlyDeps);
  }
}

void cmLocalNinjaGenerator::AddCustomCommandTarget(cmCustomCommand const* cc,
                                                   cmTarget* target)
{
  this->CustomCommandTargets[cc].insert(target);
}

void cmLocalNinjaGenerator::WriteCustomCommandBuildStatements()
{
  for (CustomCommandTargetMap::iterator i = this->CustomCommandTargets.begin();
       i != this->CustomCommandTargets.end(); ++i) {
    // A custom command may appear on multiple targets.  However, some build
    // systems exist where the target dependencies on some of the targets are
    // overspecified, leading to a dependency cycle.  If we assume all target
    // dependencies are a superset of the true target dependencies for this
    // custom command, we can take the set intersection of all target
    // dependencies to obtain a correct dependency list.
    //
    // FIXME: This won't work in certain obscure scenarios involving indirect
    // dependencies.
    std::set<cmTarget*>::iterator j = i->second.begin();
    assert(j != i->second.end());
    std::vector<std::string> ccTargetDeps;
    this->AppendTargetDepends(*j, ccTargetDeps);
    std::sort(ccTargetDeps.begin(), ccTargetDeps.end());
    ++j;

    for (; j != i->second.end(); ++j) {
      std::vector<std::string> jDeps, depsIntersection;
      this->AppendTargetDepends(*j, jDeps);
      std::sort(jDeps.begin(), jDeps.end());
      std::set_intersection(ccTargetDeps.begin(), ccTargetDeps.end(),
                            jDeps.begin(), jDeps.end(),
                            std::back_inserter(depsIntersection));
      ccTargetDeps = depsIntersection;
    }

    this->WriteCustomCommandBuildStatement(i->first, ccTargetDeps);
  }
}

std::string cmLocalNinjaGenerator::MakeCustomLauncher(
  cmCustomCommandGenerator const& ccg)
{
  const char* property = "RULE_LAUNCH_CUSTOM";
  const char* property_value = this->Makefile->GetProperty(property);

  if(!property_value || !*property_value)
  {
    return std::string();
  }

  // Expand rules in the empty string.  It may insert the launcher and
  // perform replacements.
  RuleVariables vars;
  vars.RuleLauncher = property;
  std::string output;
  const std::vector<std::string>& outputs = ccg.GetOutputs();
  if(!outputs.empty())
  {
    RelativeRoot relative_root =
      ccg.GetWorkingDirectory().empty() ? START_OUTPUT : NONE;

    output = this->Convert(outputs[0], relative_root, SHELL);
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
