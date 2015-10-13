/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "cmExportBuildFileGenerator.h"

#include "cmLocalGenerator.h"
#include "cmGlobalGenerator.h"
#include "cmExportSet.h"
#include "cmTargetExport.h"

//----------------------------------------------------------------------------
cmExportBuildFileGenerator::cmExportBuildFileGenerator()
  : Backtrace(NULL)
{
  this->Makefile = 0;
  this->ExportSet = 0;
}

//----------------------------------------------------------------------------
bool cmExportBuildFileGenerator::GenerateMainFile(std::ostream& os)
{
  {
  std::string expectedTargets;
  std::string sep;
  std::vector<std::string> targets;
  this->GetTargets(targets);
  for(std::vector<std::string>::const_iterator
        tei = targets.begin();
      tei != targets.end(); ++tei)
    {
    cmTarget *te = this->Makefile->FindTargetToUse(*tei);
    expectedTargets += sep + this->Namespace + te->GetExportName();
    sep = " ";
    if(this->ExportedTargets.insert(te).second)
      {
      this->Exports.push_back(te);
      }
    else
      {
      std::ostringstream e;
      e << "given target \"" << te->GetName() << "\" more than once.";
      this->Makefile->GetCMakeInstance()
          ->IssueMessage(cmake::FATAL_ERROR, e.str(), this->Backtrace);
      return false;
      }
    if (te->GetType() == cmTarget::INTERFACE_LIBRARY)
      {
      this->GenerateRequiredCMakeVersion(os, "3.0.0");
      }
    }

  this->GenerateExpectedTargetsCode(os, expectedTargets);
  }

  std::vector<std::string> missingTargets;

  // Create all the imported targets.
  for(std::vector<cmTarget*>::const_iterator
        tei = this->Exports.begin();
      tei != this->Exports.end(); ++tei)
    {
    cmTarget* te = *tei;
    this->GenerateImportTargetCode(os, te);

    te->AppendBuildInterfaceIncludes();

    ImportPropertyMap properties;

    this->PopulateInterfaceProperty("INTERFACE_INCLUDE_DIRECTORIES", te,
                                    cmGeneratorExpression::BuildInterface,
                                    properties, missingTargets);
    this->PopulateInterfaceProperty("INTERFACE_SOURCES", te,
                                    cmGeneratorExpression::BuildInterface,
                                    properties, missingTargets);
    this->PopulateInterfaceProperty("INTERFACE_COMPILE_DEFINITIONS", te,
                                    cmGeneratorExpression::BuildInterface,
                                    properties, missingTargets);
    this->PopulateInterfaceProperty("INTERFACE_COMPILE_OPTIONS", te,
                                    cmGeneratorExpression::BuildInterface,
                                    properties, missingTargets);
    this->PopulateInterfaceProperty("INTERFACE_AUTOUIC_OPTIONS", te,
                                    cmGeneratorExpression::BuildInterface,
                                    properties, missingTargets);
    this->PopulateInterfaceProperty("INTERFACE_COMPILE_FEATURES", te,
                                    cmGeneratorExpression::BuildInterface,
                                    properties, missingTargets);
    this->PopulateInterfaceProperty("INTERFACE_POSITION_INDEPENDENT_CODE",
                                  te, properties);
    const bool newCMP0022Behavior =
                              te->GetPolicyStatusCMP0022() != cmPolicies::WARN
                           && te->GetPolicyStatusCMP0022() != cmPolicies::OLD;
    if (newCMP0022Behavior)
      {
      this->PopulateInterfaceLinkLibrariesProperty(te,
                                    cmGeneratorExpression::BuildInterface,
                                    properties, missingTargets);
      }
    this->PopulateCompatibleInterfaceProperties(te, properties);

    this->GenerateInterfaceProperties(te, os, properties);
    }

  // Generate import file content for each configuration.
  for(std::vector<std::string>::const_iterator
        ci = this->Configurations.begin();
      ci != this->Configurations.end(); ++ci)
    {
    this->GenerateImportConfig(os, *ci, missingTargets);
    }

  this->GenerateMissingTargetsCheckCode(os, missingTargets);

  return true;
}

//----------------------------------------------------------------------------
void
cmExportBuildFileGenerator
::GenerateImportTargetsConfig(std::ostream& os,
                              const std::string& config,
                              std::string const& suffix,
                              std::vector<std::string> &missingTargets)
{
  for(std::vector<cmTarget*>::const_iterator
        tei = this->Exports.begin();
      tei != this->Exports.end(); ++tei)
    {
    // Collect import properties for this target.
    cmTarget* target = *tei;
    ImportPropertyMap properties;

    if (target->GetType() != cmTarget::INTERFACE_LIBRARY)
      {
      this->SetImportLocationProperty(config, suffix, target, properties);
      }
    if(!properties.empty())
      {
      // Get the rest of the target details.
      if (target->GetType() != cmTarget::INTERFACE_LIBRARY)
        {
        this->SetImportDetailProperties(config, suffix,
                                        target, properties, missingTargets);
        this->SetImportLinkInterface(config, suffix,
                                    cmGeneratorExpression::BuildInterface,
                                    target, properties, missingTargets);
        }

      // TOOD: PUBLIC_HEADER_LOCATION
      // This should wait until the build feature propagation stuff
      // is done.  Then this can be a propagated include directory.
      // this->GenerateImportProperty(config, te->HeaderGenerator,
      //                              properties);

      // Generate code in the export file.
      this->GenerateImportPropertyCode(os, config, target, properties);
      }
    }
}

//----------------------------------------------------------------------------
void cmExportBuildFileGenerator::SetExportSet(cmExportSet *exportSet)
{
  this->ExportSet = exportSet;
}

//----------------------------------------------------------------------------
void
cmExportBuildFileGenerator
::SetImportLocationProperty(const std::string& config,
                            std::string const& suffix,
                            cmTarget* target, ImportPropertyMap& properties)
{
  // Get the makefile in which to lookup target information.
  cmMakefile* mf = target->GetMakefile();

  // Add the main target file.
  {
  std::string prop = "IMPORTED_LOCATION";
  prop += suffix;
  std::string value;
  if(target->IsAppBundleOnApple())
    {
    value = target->GetFullPath(config, false);
    }
  else
    {
    value = target->GetFullPath(config, false, true);
    }
  properties[prop] = value;
  }

  // Check whether this is a DLL platform.
  bool dll_platform =
    (mf->IsOn("WIN32") || mf->IsOn("CYGWIN") || mf->IsOn("MINGW"));

  // Add the import library for windows DLLs.
  if(dll_platform &&
     (target->GetType() == cmTarget::SHARED_LIBRARY ||
      target->IsExecutableWithExports()) &&
     mf->GetDefinition("CMAKE_IMPORT_LIBRARY_SUFFIX"))
    {
    std::string prop = "IMPORTED_IMPLIB";
    prop += suffix;
    std::string value = target->GetFullPath(config, true);
    target->GetImplibGNUtoMS(value, value,
                             "${CMAKE_IMPORT_LIBRARY_SUFFIX}");
    properties[prop] = value;
    }
}

//----------------------------------------------------------------------------
void
cmExportBuildFileGenerator::HandleMissingTarget(
  std::string& link_libs, std::vector<std::string>& missingTargets,
  cmMakefile* mf, cmTarget* depender, cmTarget* dependee)
{
  // The target is not in the export.
  if(!this->AppendMode)
    {
    const std::string name = dependee->GetName();
    std::vector<std::string> namespaces = this->FindNamespaces(mf, name);

    int targetOccurrences = (int)namespaces.size();
    if (targetOccurrences == 1)
      {
      std::string missingTarget = namespaces[0];

      missingTarget += dependee->GetExportName();
      link_libs += missingTarget;
      missingTargets.push_back(missingTarget);
      return;
      }
    else
      {
      // We are not appending, so all exported targets should be
      // known here.  This is probably user-error.
      this->ComplainAboutMissingTarget(depender, dependee, targetOccurrences);
      }
    }
  // Assume the target will be exported by another command.
  // Append it with the export namespace.
  link_libs += this->Namespace;
  link_libs += dependee->GetExportName();
}

//----------------------------------------------------------------------------
void cmExportBuildFileGenerator
::GetTargets(std::vector<std::string> &targets) const
{
  if (this->ExportSet)
    {
    for(std::vector<cmTargetExport*>::const_iterator
          tei = this->ExportSet->GetTargetExports()->begin();
          tei != this->ExportSet->GetTargetExports()->end(); ++tei)
      {
      targets.push_back((*tei)->Target->GetName());
      }
    return;
    }
  targets = this->Targets;
}

//----------------------------------------------------------------------------
std::vector<std::string>
cmExportBuildFileGenerator
::FindNamespaces(cmMakefile* mf, const std::string& name)
{
  std::vector<std::string> namespaces;
  cmGlobalGenerator* gg = mf->GetGlobalGenerator();

  std::map<std::string, cmExportBuildFileGenerator*>& exportSets
                                                  = gg->GetBuildExportSets();

  for(std::map<std::string, cmExportBuildFileGenerator*>::const_iterator
      expIt = exportSets.begin(); expIt != exportSets.end(); ++expIt)
    {
    const cmExportBuildFileGenerator* exportSet = expIt->second;
    std::vector<std::string> targets;
    exportSet->GetTargets(targets);
    if (std::find(targets.begin(), targets.end(), name) != targets.end())
      {
      namespaces.push_back(exportSet->GetNamespace());
      }
    }

  return namespaces;
}

//----------------------------------------------------------------------------
void
cmExportBuildFileGenerator
::ComplainAboutMissingTarget(cmTarget* depender,
                             cmTarget* dependee,
                             int occurrences)
{
  if(cmSystemTools::GetErrorOccuredFlag())
    {
    return;
    }

  std::ostringstream e;
  e << "export called with target \"" << depender->GetName()
    << "\" which requires target \"" << dependee->GetName() << "\" ";
  if (occurrences == 0)
    {
    e << "that is not in the export set.\n";
    }
  else
    {
    e << "that is not in this export set, but " << occurrences
    << " times in others.\n";
    }
  e << "If the required target is not easy to reference in this call, "
    << "consider using the APPEND option with multiple separate calls.";

  this->Makefile->GetCMakeInstance()
      ->IssueMessage(cmake::FATAL_ERROR, e.str(), this->Backtrace);
}

std::string
cmExportBuildFileGenerator::InstallNameDir(cmTarget* target,
                                           const std::string& config)
{
  std::string install_name_dir;

  cmMakefile* mf = target->GetMakefile();
  if(mf->IsOn("CMAKE_PLATFORM_HAS_INSTALLNAME"))
    {
    install_name_dir =
      target->GetInstallNameDirForBuildTree(config);
    }

  return install_name_dir;
}
