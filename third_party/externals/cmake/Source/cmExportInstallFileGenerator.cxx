/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "cmExportInstallFileGenerator.h"

#include "cmExportSet.h"
#include "cmExportSetMap.h"
#include "cmGeneratedFileStream.h"
#include "cmGlobalGenerator.h"
#include "cmLocalGenerator.h"
#include "cmInstallExportGenerator.h"
#include "cmInstallTargetGenerator.h"
#include "cmTargetExport.h"
#include "cmAlgorithms.h"

//----------------------------------------------------------------------------
cmExportInstallFileGenerator
::cmExportInstallFileGenerator(cmInstallExportGenerator* iegen):
  IEGen(iegen)
{
}

//----------------------------------------------------------------------------
std::string cmExportInstallFileGenerator::GetConfigImportFileGlob()
{
  std::string glob = this->FileBase;
  glob += "-*";
  glob += this->FileExt;
  return glob;
}

//----------------------------------------------------------------------------
bool cmExportInstallFileGenerator::GenerateMainFile(std::ostream& os)
{
  std::vector<cmTargetExport*> allTargets;
  {
  std::string expectedTargets;
  std::string sep;
  for(std::vector<cmTargetExport*>::const_iterator
        tei = this->IEGen->GetExportSet()->GetTargetExports()->begin();
      tei != this->IEGen->GetExportSet()->GetTargetExports()->end(); ++tei)
    {
    expectedTargets += sep + this->Namespace + (*tei)->Target->GetExportName();
    sep = " ";
    cmTargetExport * te = *tei;
    if(this->ExportedTargets.insert(te->Target).second)
      {
      allTargets.push_back(te);
      }
    else
      {
      std::ostringstream e;
      e << "install(EXPORT \""
        << this->IEGen->GetExportSet()->GetName()
        << "\" ...) " << "includes target \"" << te->Target->GetName()
        << "\" more than once in the export set.";
      cmSystemTools::Error(e.str().c_str());
      return false;
      }
    }

  this->GenerateExpectedTargetsCode(os, expectedTargets);
  }

  // Set an _IMPORT_PREFIX variable for import location properties
  // to reference if they are relative to the install prefix.
  std::string installPrefix =
    this->IEGen->GetMakefile()->GetSafeDefinition("CMAKE_INSTALL_PREFIX");
  std::string const& expDest = this->IEGen->GetDestination();
  if(cmSystemTools::FileIsFullPath(expDest))
    {
    // The export file is being installed to an absolute path so the
    // package is not relocatable.  Use the configured install prefix.
    os <<
      "# The installation prefix configured by this project.\n"
      "set(_IMPORT_PREFIX \"" << installPrefix << "\")\n"
      "\n";
    }
  else
    {
    // Add code to compute the installation prefix relative to the
    // import file location.
    std::string absDest = installPrefix + "/" + expDest;
    std::string absDestS = absDest + "/";
    os << "# Compute the installation prefix relative to this file.\n"
       << "get_filename_component(_IMPORT_PREFIX"
       << " \"${CMAKE_CURRENT_LIST_FILE}\" PATH)\n";
    if(cmHasLiteralPrefix(absDestS.c_str(), "/lib/") ||
       cmHasLiteralPrefix(absDestS.c_str(), "/lib64/") ||
       cmHasLiteralPrefix(absDestS.c_str(), "/usr/lib/") ||
       cmHasLiteralPrefix(absDestS.c_str(), "/usr/lib64/"))
      {
      // Handle "/usr move" symlinks created by some Linux distros.
      os <<
        "# Use original install prefix when loaded through a\n"
        "# cross-prefix symbolic link such as /lib -> /usr/lib.\n"
        "get_filename_component(_realCurr \"${_IMPORT_PREFIX}\" REALPATH)\n"
        "get_filename_component(_realOrig \"" << absDest << "\" REALPATH)\n"
        "if(_realCurr STREQUAL _realOrig)\n"
        "  set(_IMPORT_PREFIX \"" << absDest << "\")\n"
        "endif()\n"
        "unset(_realOrig)\n"
        "unset(_realCurr)\n";
      }
    std::string dest = expDest;
    while(!dest.empty())
      {
      os <<
        "get_filename_component(_IMPORT_PREFIX \"${_IMPORT_PREFIX}\" PATH)\n";
      dest = cmSystemTools::GetFilenamePath(dest);
      }
    os << "\n";
    }

  std::vector<std::string> missingTargets;

  bool require2_8_12 = false;
  bool require3_0_0 = false;
  bool require3_1_0 = false;
  bool requiresConfigFiles = false;
  // Create all the imported targets.
  for(std::vector<cmTargetExport*>::const_iterator
        tei = allTargets.begin();
      tei != allTargets.end(); ++tei)
    {
    cmTarget* te = (*tei)->Target;

    requiresConfigFiles = requiresConfigFiles
                              || te->GetType() != cmTarget::INTERFACE_LIBRARY;

    this->GenerateImportTargetCode(os, te);

    ImportPropertyMap properties;

    this->PopulateIncludeDirectoriesInterface(*tei,
                                  cmGeneratorExpression::InstallInterface,
                                  properties, missingTargets);
    this->PopulateSourcesInterface(*tei,
                                  cmGeneratorExpression::InstallInterface,
                                  properties, missingTargets);
    this->PopulateInterfaceProperty("INTERFACE_SYSTEM_INCLUDE_DIRECTORIES",
                                  te,
                                  cmGeneratorExpression::InstallInterface,
                                  properties, missingTargets);
    this->PopulateInterfaceProperty("INTERFACE_COMPILE_DEFINITIONS",
                                  te,
                                  cmGeneratorExpression::InstallInterface,
                                  properties, missingTargets);
    this->PopulateInterfaceProperty("INTERFACE_COMPILE_OPTIONS",
                                  te,
                                  cmGeneratorExpression::InstallInterface,
                                  properties, missingTargets);
    this->PopulateInterfaceProperty("INTERFACE_AUTOUIC_OPTIONS",
                                  te,
                                  cmGeneratorExpression::InstallInterface,
                                  properties, missingTargets);
    this->PopulateInterfaceProperty("INTERFACE_COMPILE_FEATURES",
                                  te,
                                  cmGeneratorExpression::InstallInterface,
                                  properties, missingTargets);

    const bool newCMP0022Behavior =
                              te->GetPolicyStatusCMP0022() != cmPolicies::WARN
                           && te->GetPolicyStatusCMP0022() != cmPolicies::OLD;
    if (newCMP0022Behavior)
      {
      if (this->PopulateInterfaceLinkLibrariesProperty(te,
                                    cmGeneratorExpression::InstallInterface,
                                    properties, missingTargets)
          && !this->ExportOld)
        {
        require2_8_12 = true;
        }
      }
    if (te->GetType() == cmTarget::INTERFACE_LIBRARY)
      {
      require3_0_0 = true;
      }
    if(te->GetProperty("INTERFACE_SOURCES"))
      {
      // We can only generate INTERFACE_SOURCES in CMake 3.3, but CMake 3.1
      // can consume them.
      require3_1_0 = true;
      }

    this->PopulateInterfaceProperty("INTERFACE_POSITION_INDEPENDENT_CODE",
                                  te, properties);
    this->PopulateCompatibleInterfaceProperties(te, properties);

    this->GenerateInterfaceProperties(te, os, properties);
    }

  if (require3_1_0)
    {
    this->GenerateRequiredCMakeVersion(os, "3.1.0");
    }
  else if (require3_0_0)
    {
    this->GenerateRequiredCMakeVersion(os, "3.0.0");
    }
  else if (require2_8_12)
    {
    this->GenerateRequiredCMakeVersion(os, "2.8.12");
    }

  // Now load per-configuration properties for them.
  os << "# Load information for each installed configuration.\n"
     << "get_filename_component(_DIR \"${CMAKE_CURRENT_LIST_FILE}\" PATH)\n"
     << "file(GLOB CONFIG_FILES \"${_DIR}/"
     << this->GetConfigImportFileGlob() << "\")\n"
     << "foreach(f ${CONFIG_FILES})\n"
     << "  include(${f})\n"
     << "endforeach()\n"
     << "\n";

  // Cleanup the import prefix variable.
  os << "# Cleanup temporary variables.\n"
     << "set(_IMPORT_PREFIX)\n"
     << "\n";
  this->GenerateImportedFileCheckLoop(os);

  bool result = true;
  // Generate an import file for each configuration.
  // Don't do this if we only export INTERFACE_LIBRARY targets.
  if (requiresConfigFiles)
    {
    for(std::vector<std::string>::const_iterator
          ci = this->Configurations.begin();
        ci != this->Configurations.end(); ++ci)
      {
      if(!this->GenerateImportFileConfig(*ci, missingTargets))
        {
        result = false;
        }
      }
    }

  this->GenerateMissingTargetsCheckCode(os, missingTargets);

  return result;
}

//----------------------------------------------------------------------------
void
cmExportInstallFileGenerator::ReplaceInstallPrefix(std::string &input)
{
  std::string::size_type pos = 0;
  std::string::size_type lastPos = pos;

  while((pos = input.find("$<INSTALL_PREFIX>", lastPos)) != input.npos)
    {
    std::string::size_type endPos = pos + sizeof("$<INSTALL_PREFIX>") - 1;
    input.replace(pos, endPos - pos, "${_IMPORT_PREFIX}");
    lastPos = endPos;
    }
}

//----------------------------------------------------------------------------
bool
cmExportInstallFileGenerator::GenerateImportFileConfig(
                                    const std::string& config,
                                    std::vector<std::string> &missingTargets)
{
  // Skip configurations not enabled for this export.
  if(!this->IEGen->InstallsForConfig(config))
    {
    return true;
    }

  // Construct the name of the file to generate.
  std::string fileName = this->FileDir;
  fileName += "/";
  fileName += this->FileBase;
  fileName += "-";
  if(!config.empty())
    {
    fileName += cmSystemTools::LowerCase(config);
    }
  else
    {
    fileName += "noconfig";
    }
  fileName += this->FileExt;

  // Open the output file to generate it.
  cmGeneratedFileStream exportFileStream(fileName.c_str(), true);
  if(!exportFileStream)
    {
    std::string se = cmSystemTools::GetLastSystemError();
    std::ostringstream e;
    e << "cannot write to file \"" << fileName
      << "\": " << se;
    cmSystemTools::Error(e.str().c_str());
    return false;
    }
  std::ostream& os = exportFileStream;

  // Start with the import file header.
  this->GenerateImportHeaderCode(os, config);

  // Generate the per-config target information.
  this->GenerateImportConfig(os, config, missingTargets);

  // End with the import file footer.
  this->GenerateImportFooterCode(os);

  // Record this per-config import file.
  this->ConfigImportFiles[config] = fileName;

  return true;
}

//----------------------------------------------------------------------------
void
cmExportInstallFileGenerator
::GenerateImportTargetsConfig(std::ostream& os,
                              const std::string& config,
                              std::string const& suffix,
                              std::vector<std::string> &missingTargets)
{
  // Add each target in the set to the export.
  for(std::vector<cmTargetExport*>::const_iterator
        tei = this->IEGen->GetExportSet()->GetTargetExports()->begin();
      tei != this->IEGen->GetExportSet()->GetTargetExports()->end(); ++tei)
    {
    // Collect import properties for this target.
    cmTargetExport const* te = *tei;
    if (te->Target->GetType() == cmTarget::INTERFACE_LIBRARY)
      {
      continue;
      }

    ImportPropertyMap properties;
    std::set<std::string> importedLocations;

    this->SetImportLocationProperty(config, suffix, te->ArchiveGenerator,
                                    properties, importedLocations);
    this->SetImportLocationProperty(config, suffix, te->LibraryGenerator,
                                    properties, importedLocations);
    this->SetImportLocationProperty(config, suffix,
                                    te->RuntimeGenerator, properties,
                                    importedLocations);
    this->SetImportLocationProperty(config, suffix, te->FrameworkGenerator,
                                    properties, importedLocations);
    this->SetImportLocationProperty(config, suffix, te->BundleGenerator,
                                    properties, importedLocations);

    // If any file location was set for the target add it to the
    // import file.
    if(!properties.empty())
      {
      // Get the rest of the target details.
      this->SetImportDetailProperties(config, suffix,
                                      te->Target, properties, missingTargets);

      this->SetImportLinkInterface(config, suffix,
                                   cmGeneratorExpression::InstallInterface,
                                   te->Target, properties, missingTargets);

      // TOOD: PUBLIC_HEADER_LOCATION
      // This should wait until the build feature propagation stuff
      // is done.  Then this can be a propagated include directory.
      // this->GenerateImportProperty(config, te->HeaderGenerator,
      //                              properties);

      // Generate code in the export file.
      this->GenerateImportPropertyCode(os, config, te->Target, properties);
      this->GenerateImportedFileChecksCode(os, te->Target, properties,
                                           importedLocations);
      }
    }
}

//----------------------------------------------------------------------------
void
cmExportInstallFileGenerator
::SetImportLocationProperty(const std::string& config,
                            std::string const& suffix,
                            cmInstallTargetGenerator* itgen,
                            ImportPropertyMap& properties,
                            std::set<std::string>& importedLocations
                           )
{
  // Skip rules that do not match this configuration.
  if(!(itgen && itgen->InstallsForConfig(config)))
    {
    return;
    }

  // Get the target to be installed.
  cmTarget* target = itgen->GetTarget();

  // Construct the installed location of the target.
  std::string dest = itgen->GetDestination(config);
  std::string value;
  if(!cmSystemTools::FileIsFullPath(dest.c_str()))
    {
    // The target is installed relative to the installation prefix.
    value = "${_IMPORT_PREFIX}/";
    }
  value += dest;
  value += "/";

  if(itgen->IsImportLibrary())
    {
    // Construct the property name.
    std::string prop = "IMPORTED_IMPLIB";
    prop += suffix;

    // Append the installed file name.
    value += itgen->GetInstallFilename(target, config,
                                       cmInstallTargetGenerator::NameImplib);

    // Store the property.
    properties[prop] = value;
    importedLocations.insert(prop);
    }
  else
    {
    // Construct the property name.
    std::string prop = "IMPORTED_LOCATION";
    prop += suffix;

    // Append the installed file name.
    if(target->IsAppBundleOnApple())
      {
      value += itgen->GetInstallFilename(target, config);
      value += ".app/Contents/MacOS/";
      value += itgen->GetInstallFilename(target, config);
      }
    else
      {
      value += itgen->GetInstallFilename(target, config,
                                         cmInstallTargetGenerator::NameReal);
      }

    // Store the property.
    properties[prop] = value;
    importedLocations.insert(prop);
    }
}

//----------------------------------------------------------------------------
void
cmExportInstallFileGenerator::HandleMissingTarget(
  std::string& link_libs, std::vector<std::string>& missingTargets,
  cmMakefile* mf, cmTarget* depender, cmTarget* dependee)
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
    }
  else
    {
    // All exported targets should be known here and should be unique.
    // This is probably user-error.
    this->ComplainAboutMissingTarget(depender, dependee, targetOccurrences);
    }
}

//----------------------------------------------------------------------------
std::vector<std::string>
cmExportInstallFileGenerator
::FindNamespaces(cmMakefile* mf, const std::string& name)
{
  std::vector<std::string> namespaces;
  cmGlobalGenerator* gg = mf->GetGlobalGenerator();
  const cmExportSetMap& exportSets = gg->GetExportSets();

  for(cmExportSetMap::const_iterator expIt = exportSets.begin();
      expIt != exportSets.end();
      ++expIt)
    {
    const cmExportSet* exportSet = expIt->second;
    std::vector<cmTargetExport*> const* targets =
                                                 exportSet->GetTargetExports();

    bool containsTarget = false;
    for(unsigned int i=0; i<targets->size(); i++)
      {
      if (name == (*targets)[i]->Target->GetName())
        {
        containsTarget = true;
        break;
        }
      }

    if (containsTarget)
      {
      std::vector<cmInstallExportGenerator const*> const* installs =
                                                 exportSet->GetInstallations();
      for(unsigned int i=0; i<installs->size(); i++)
        {
        namespaces.push_back((*installs)[i]->GetNamespace());
        }
      }
    }

  return namespaces;
}

//----------------------------------------------------------------------------
void
cmExportInstallFileGenerator
::ComplainAboutMissingTarget(cmTarget* depender,
                             cmTarget* dependee,
                             int occurrences)
{
  std::ostringstream e;
  e << "install(EXPORT \""
    << this->IEGen->GetExportSet()->GetName()
    << "\" ...) "
    << "includes target \"" << depender->GetName()
    << "\" which requires target \"" << dependee->GetName() << "\" ";
  if (occurrences == 0)
    {
    e << "that is not in the export set.";
    }
  else
    {
    e << "that is not in this export set, but " << occurrences
    << " times in others.";
    }
  cmSystemTools::Error(e.str().c_str());
}

std::string
cmExportInstallFileGenerator::InstallNameDir(cmTarget* target,
                                             const std::string&)
{
  std::string install_name_dir;

  cmMakefile* mf = target->GetMakefile();
  if(mf->IsOn("CMAKE_PLATFORM_HAS_INSTALLNAME"))
    {
    install_name_dir =
      target->GetInstallNameDirForInstallTree();
    }

  return install_name_dir;
}
