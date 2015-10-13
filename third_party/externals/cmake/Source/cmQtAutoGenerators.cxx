/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2004-2011 Kitware, Inc.
  Copyright 2011 Alexander Neundorf (neundorf@kde.org)

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/

#include "cmGlobalGenerator.h"
#include "cmLocalGenerator.h"
#include "cmMakefile.h"
#include "cmSourceFile.h"
#include "cmSystemTools.h"
#include "cmState.h"
#include "cmAlgorithms.h"

#if defined(_WIN32) && !defined(__CYGWIN__)
# include "cmGlobalVisualStudioGenerator.h"
#endif

#include <sys/stat.h>

#include <cmsys/Terminal.h>
#include <cmsys/ios/sstream>
#include <cmsys/FStream.hxx>
#include <assert.h>

#include <string.h>
#if defined(__APPLE__)
#include <unistd.h>
#endif

#include "cmQtAutoGenerators.h"


static bool requiresMocing(const std::string& text, std::string &macroName)
{
  // this simple check is much much faster than the regexp
  if (strstr(text.c_str(), "Q_OBJECT") == NULL
      && strstr(text.c_str(), "Q_GADGET") == NULL)
    {
    return false;
    }

  cmsys::RegularExpression qObjectRegExp("[\n][ \t]*Q_OBJECT[^a-zA-Z0-9_]");
  if (qObjectRegExp.find(text))
    {
    macroName = "Q_OBJECT";
    return true;
    }
  cmsys::RegularExpression qGadgetRegExp("[\n][ \t]*Q_GADGET[^a-zA-Z0-9_]");
  if (qGadgetRegExp.find(text))
    {
    macroName = "Q_GADGET";
    return true;
    }
  return false;
}


static std::string findMatchingHeader(const std::string& absPath,
                                      const std::string& mocSubDir,
                                      const std::string& basename,
                              const std::vector<std::string>& headerExtensions)
{
  std::string header;
  for(std::vector<std::string>::const_iterator ext = headerExtensions.begin();
      ext != headerExtensions.end();
      ++ext)
    {
    std::string sourceFilePath = absPath + basename + "." + (*ext);
    if (cmsys::SystemTools::FileExists(sourceFilePath.c_str()))
      {
      header = sourceFilePath;
      break;
      }
    if (!mocSubDir.empty())
      {
      sourceFilePath = mocSubDir + basename + "." + (*ext);
      if (cmsys::SystemTools::FileExists(sourceFilePath.c_str()))
        {
        header = sourceFilePath;
        break;
        }
      }
    }

  return header;
}


static std::string extractSubDir(const std::string& absPath,
                                 const std::string& currentMoc)
{
  std::string subDir;
  if (currentMoc.find_first_of('/') != std::string::npos)
    {
    subDir = absPath
                  + cmsys::SystemTools::GetFilenamePath(currentMoc) + '/';
    }
  return subDir;
}


static void copyTargetProperty(cmTarget* destinationTarget,
                               cmTarget* sourceTarget,
                               const std::string& propertyName)
{
  const char* propertyValue = sourceTarget->GetProperty(propertyName);
  if (propertyValue)
    {
    destinationTarget->SetProperty(propertyName, propertyValue);
    }
}


static std::string ReadAll(const std::string& filename)
{
  cmsys::ifstream file(filename.c_str());
  cmsys_ios::stringstream stream;
  stream << file.rdbuf();
  file.close();
  return stream.str();
}

cmQtAutoGenerators::cmQtAutoGenerators()
:Verbose(cmsys::SystemTools::GetEnv("VERBOSE") != 0)
,ColorOutput(true)
,RunMocFailed(false)
,RunUicFailed(false)
,RunRccFailed(false)
,GenerateAll(false)
{

  std::string colorEnv = "";
  cmsys::SystemTools::GetEnv("COLOR", colorEnv);
  if(!colorEnv.empty())
    {
    if(cmSystemTools::IsOn(colorEnv.c_str()))
      {
      this->ColorOutput = true;
      }
    else
      {
      this->ColorOutput = false;
      }
    }
}

static std::string getAutogenTargetName(cmTarget const* target)
{
  std::string autogenTargetName = target->GetName();
  autogenTargetName += "_automoc";
  return autogenTargetName;
}

static std::string getAutogenTargetDir(cmTarget const* target)
{
  cmMakefile* makefile = target->GetMakefile();
  std::string targetDir = makefile->GetCurrentBinaryDirectory();
  targetDir += makefile->GetCMakeInstance()->GetCMakeFilesDirectory();
  targetDir += "/";
  targetDir += getAutogenTargetName(target);
  targetDir += ".dir/";
  return targetDir;
}

static std::string cmQtAutoGeneratorsStripCR(std::string const& line)
{
  // Strip CR characters rcc may have printed (possibly more than one!).
  std::string::size_type cr = line.find('\r');
  if (cr != line.npos)
    {
    return line.substr(0, cr);
    }
  return line;
}

std::string cmQtAutoGenerators::ListQt5RccInputs(cmSourceFile* sf,
                                            cmTarget const* target,
                                            std::vector<std::string>& depends)
{
  std::string rccCommand = this->GetRccExecutable(target);
  std::vector<std::string> qrcEntries;

  std::vector<std::string> command;
  command.push_back(rccCommand);
  command.push_back("--list");

  std::string absFile = cmsys::SystemTools::GetRealPath(
                                              sf->GetFullPath());

  command.push_back(absFile);

  std::string rccStdOut;
  std::string rccStdErr;
  int retVal = 0;
  bool result = cmSystemTools::RunSingleCommand(
    command, &rccStdOut, &rccStdErr,
    &retVal, 0, cmSystemTools::OUTPUT_NONE);
  if (!result || retVal)
    {
    std::cerr << "AUTOGEN: error: Rcc list process for " << sf->GetFullPath()
              << " failed:\n" << rccStdOut << "\n" << rccStdErr << std::endl;
    return std::string();
    }

  {
  std::istringstream ostr(rccStdOut);
  std::string oline;
  while(std::getline(ostr, oline))
    {
    oline = cmQtAutoGeneratorsStripCR(oline);
    if(!oline.empty())
      {
      qrcEntries.push_back(oline);
      }
    }
  }

  {
  std::istringstream estr(rccStdErr);
  std::string eline;
  while(std::getline(estr, eline))
    {
    eline = cmQtAutoGeneratorsStripCR(eline);
    if (cmHasLiteralPrefix(eline, "RCC: Error in"))
      {
      static std::string searchString = "Cannot find file '";

      std::string::size_type pos = eline.find(searchString);
      if (pos == std::string::npos)
        {
        std::cerr << "AUTOGEN: error: Rcc lists unparsable output "
                  << eline << std::endl;
        return std::string();
        }
      pos += searchString.length();
      std::string::size_type sz = eline.size() - pos - 1;
      qrcEntries.push_back(eline.substr(pos, sz));
      }
    }
  }

  depends.insert(depends.end(), qrcEntries.begin(), qrcEntries.end());
  return cmJoin(qrcEntries, "@list_sep@");
}

std::string cmQtAutoGenerators::ListQt4RccInputs(cmSourceFile* sf,
                                            std::vector<std::string>& depends)
{
  const std::string qrcContents = ReadAll(sf->GetFullPath());

  cmsys::RegularExpression fileMatchRegex("(<file[^<]+)");

  std::string entriesList;
  const char* sep = "";

  size_t offset = 0;
  while (fileMatchRegex.find(qrcContents.c_str() + offset))
    {
    std::string qrcEntry = fileMatchRegex.match(1);

    offset += qrcEntry.size();

    cmsys::RegularExpression fileReplaceRegex("(^<file[^>]*>)");
    fileReplaceRegex.find(qrcEntry);
    std::string tag = fileReplaceRegex.match(1);

    qrcEntry = qrcEntry.substr(tag.size());

    if (!cmSystemTools::FileIsFullPath(qrcEntry.c_str()))
      {
      qrcEntry = sf->GetLocation().GetDirectory() + "/" + qrcEntry;
      }

    entriesList += sep;
    entriesList += qrcEntry;
    sep = "@list_sep@";
    depends.push_back(qrcEntry);
    }
  return entriesList;
}

bool cmQtAutoGenerators::InitializeAutogenTarget(cmTarget* target)
{
  cmMakefile* makefile = target->GetMakefile();
  // don't do anything if there is no Qt4 or Qt5Core (which contains moc):
  std::string qtMajorVersion = makefile->GetSafeDefinition("QT_VERSION_MAJOR");
  if (qtMajorVersion == "")
    {
    qtMajorVersion = makefile->GetSafeDefinition("Qt5Core_VERSION_MAJOR");
    }
  if (qtMajorVersion != "4" && qtMajorVersion != "5")
    {
    return false;
    }

  if (target->GetPropertyAsBool("AUTOMOC"))
    {
    std::string automocTargetName = getAutogenTargetName(target);
    std::string mocCppFile = makefile->GetCurrentBinaryDirectory();
    mocCppFile += "/";
    mocCppFile += automocTargetName;
    mocCppFile += ".cpp";
    makefile->GetOrCreateSource(mocCppFile, true);
    makefile->AppendProperty("ADDITIONAL_MAKE_CLEAN_FILES",
                            mocCppFile.c_str(), false);

    target->AddSource(mocCppFile);
    }
  // create a custom target for running generators at buildtime:
  std::string autogenTargetName = getAutogenTargetName(target);

  std::string targetDir = getAutogenTargetDir(target);

  cmCustomCommandLine currentLine;
  currentLine.push_back(cmSystemTools::GetCMakeCommand());
  currentLine.push_back("-E");
  currentLine.push_back("cmake_autogen");
  currentLine.push_back(targetDir);
  currentLine.push_back("$<CONFIGURATION>");

  cmCustomCommandLines commandLines;
  commandLines.push_back(currentLine);

  std::string workingDirectory = cmSystemTools::CollapseFullPath(
                                    "", makefile->GetCurrentBinaryDirectory());

  std::vector<std::string> depends;
  if (const char *autogenDepends =
                                target->GetProperty("AUTOGEN_TARGET_DEPENDS"))
    {
    cmSystemTools::ExpandListArgument(autogenDepends, depends);
    }
  std::vector<std::string> toolNames;
  if (target->GetPropertyAsBool("AUTOMOC"))
    {
    toolNames.push_back("moc");
    }
  if (target->GetPropertyAsBool("AUTOUIC"))
    {
    toolNames.push_back("uic");
    }
  if (target->GetPropertyAsBool("AUTORCC"))
    {
    toolNames.push_back("rcc");
    }

  std::string tools = toolNames[0];
  toolNames.erase(toolNames.begin());
  while (toolNames.size() > 1)
    {
    tools += ", " + toolNames[0];
    toolNames.erase(toolNames.begin());
    }
  if (toolNames.size() == 1)
    {
    tools += " and " + toolNames[0];
    }
  std::string autogenComment = "Automatic " + tools + " for target ";
  autogenComment += target->GetName();

#if defined(_WIN32) && !defined(__CYGWIN__)
  bool usePRE_BUILD = false;
  cmLocalGenerator* localGen = makefile->GetLocalGenerator();
  cmGlobalGenerator* gg = localGen->GetGlobalGenerator();
  if(gg->GetName().find("Visual Studio") != std::string::npos)
    {
    cmGlobalVisualStudioGenerator* vsgg =
      static_cast<cmGlobalVisualStudioGenerator*>(gg);
    // Under VS >= 7 use a PRE_BUILD event instead of a separate target to
    // reduce the number of targets loaded into the IDE.
    // This also works around a VS 11 bug that may skip updating the target:
    //  https://connect.microsoft.com/VisualStudio/feedback/details/769495
    usePRE_BUILD = vsgg->GetVersion() >= cmGlobalVisualStudioGenerator::VS7;
    if(usePRE_BUILD)
      {
      for (std::vector<std::string>::iterator it = depends.begin();
            it != depends.end(); ++it)
        {
        if(!makefile->FindTargetToUse(it->c_str()))
          {
          usePRE_BUILD = false;
          break;
          }
        }
      }
    }
#endif

  std::vector<std::string> rcc_output;
  bool const isNinja =
    makefile->GetGlobalGenerator()->GetName() == "Ninja";
  if(isNinja
#if defined(_WIN32) && !defined(__CYGWIN__)
        || usePRE_BUILD
#endif
        )
    {
    std::vector<cmSourceFile*> srcFiles;
    target->GetConfigCommonSourceFiles(srcFiles);
    for(std::vector<cmSourceFile*>::const_iterator fileIt = srcFiles.begin();
        fileIt != srcFiles.end();
        ++fileIt)
      {
      cmSourceFile* sf = *fileIt;
      std::string absFile = cmsys::SystemTools::GetRealPath(
                                                sf->GetFullPath());

      std::string ext = sf->GetExtension();

      if (target->GetPropertyAsBool("AUTORCC"))
        {
        if (ext == "qrc"
            && !cmSystemTools::IsOn(sf->GetPropertyForUser("SKIP_AUTORCC")))
          {
          std::string basename = cmsys::SystemTools::
                                  GetFilenameWithoutLastExtension(absFile);

          std::string rcc_output_dir = target->GetSupportDirectory();
          cmSystemTools::MakeDirectory(rcc_output_dir.c_str());
          std::string rcc_output_file = rcc_output_dir;
          rcc_output_file += "/qrc_" + basename + ".cpp";
          rcc_output.push_back(rcc_output_file);

          if (!cmSystemTools::IsOn(sf->GetPropertyForUser("GENERATED")))
            {
            if (qtMajorVersion == "5")
              {
              this->ListQt5RccInputs(sf, target, depends);
              }
            else
              {
              this->ListQt4RccInputs(sf, depends);
              }
#if defined(_WIN32) && !defined(__CYGWIN__)
            usePRE_BUILD = false;
#endif
            }
          }
        }
      }
    }

#if defined(_WIN32) && !defined(__CYGWIN__)
  if(usePRE_BUILD)
    {
    // Add the pre-build command directly to bypass the OBJECT_LIBRARY
    // rejection in cmMakefile::AddCustomCommandToTarget because we know
    // PRE_BUILD will work for an OBJECT_LIBRARY in this specific case.
    std::vector<std::string> no_output;
    std::vector<std::string> no_byproducts;
    cmCustomCommand cc(makefile, no_output, no_byproducts, depends,
                       commandLines, autogenComment.c_str(),
                       workingDirectory.c_str());
    cc.SetEscapeOldStyle(false);
    cc.SetEscapeAllowMakeVars(true);
    target->AddPreBuildCommand(cc);
    }
  else
#endif
    {
    cmTarget* autogenTarget = 0;
    if (!rcc_output.empty() && !isNinja)
      {
      std::vector<std::string> no_byproducts;
      makefile->AddCustomCommandToOutput(rcc_output, no_byproducts,
                                         depends, "",
                                         commandLines, 0,
                                         workingDirectory.c_str(),
                                         false, false);

      cmCustomCommandLines no_commands;
      autogenTarget = makefile->AddUtilityCommand(
                          autogenTargetName, true,
                          workingDirectory.c_str(), rcc_output,
                          no_commands, false, autogenComment.c_str());

      }
    else
      {
      autogenTarget = makefile->AddUtilityCommand(
                                autogenTargetName, true,
                                workingDirectory.c_str(),
                                /*byproducts=*/rcc_output, depends,
                                commandLines, false, autogenComment.c_str());
      }

    // Set target folder
    const char* autogenFolder = makefile->GetState()
                                ->GetGlobalProperty("AUTOMOC_TARGETS_FOLDER");
    if (!autogenFolder)
      {
      autogenFolder = makefile->GetState()
                                ->GetGlobalProperty("AUTOGEN_TARGETS_FOLDER");
      }
    if (autogenFolder && *autogenFolder)
      {
      autogenTarget->SetProperty("FOLDER", autogenFolder);
      }
    else
      {
      // inherit FOLDER property from target (#13688)
      copyTargetProperty(autogenTarget, target, "FOLDER");
      }

    target->AddUtility(autogenTargetName);
    }

  return true;
}

static void GetCompileDefinitionsAndDirectories(cmTarget const* target,
                                                const std::string& config,
                                                std::string &incs,
                                                std::string &defs)
{
  cmMakefile* makefile = target->GetMakefile();
  cmLocalGenerator* localGen = makefile->GetLocalGenerator();
  std::vector<std::string> includeDirs;
  cmGeneratorTarget *gtgt = localGen->GetGlobalGenerator()
                                    ->GetGeneratorTarget(target);
  // Get the include dirs for this target, without stripping the implicit
  // include dirs off, see http://public.kitware.com/Bug/view.php?id=13667
  localGen->GetIncludeDirectories(includeDirs, gtgt, "CXX", config, false);

  incs = cmJoin(includeDirs, ";");

  std::set<std::string> defines;
  localGen->AddCompileDefinitions(defines, target, config, "CXX");

  defs += cmJoin(defines, ";");
}

void cmQtAutoGenerators::SetupAutoGenerateTarget(cmTarget const* target)
{
  cmMakefile* makefile = target->GetMakefile();

  // forget the variables added here afterwards again:
  cmMakefile::ScopePushPop varScope(makefile);
  static_cast<void>(varScope);

  // create a custom target for running generators at buildtime:
  std::string autogenTargetName = getAutogenTargetName(target);

  makefile->AddDefinition("_moc_target_name",
          cmLocalGenerator::EscapeForCMake(autogenTargetName).c_str());
  makefile->AddDefinition("_origin_target_name",
          cmLocalGenerator::EscapeForCMake(target->GetName()).c_str());

  std::string targetDir = getAutogenTargetDir(target);

  const char *qtVersion = makefile->GetDefinition("Qt5Core_VERSION_MAJOR");
  if (!qtVersion)
    {
    qtVersion = makefile->GetDefinition("QT_VERSION_MAJOR");
    }
  if (const char *targetQtVersion =
      target->GetLinkInterfaceDependentStringProperty("QT_MAJOR_VERSION", ""))
    {
    qtVersion = targetQtVersion;
    }
  if (qtVersion)
    {
    makefile->AddDefinition("_target_qt_version", qtVersion);
    }

  std::map<std::string, std::string> configIncludes;
  std::map<std::string, std::string> configDefines;
  std::map<std::string, std::string> configUicOptions;

  if (target->GetPropertyAsBool("AUTOMOC")
      || target->GetPropertyAsBool("AUTOUIC")
      || target->GetPropertyAsBool("AUTORCC"))
    {
    this->SetupSourceFiles(target);
    }
  makefile->AddDefinition("_cpp_files",
          cmLocalGenerator::EscapeForCMake(this->Sources).c_str());
  if (target->GetPropertyAsBool("AUTOMOC"))
    {
    this->SetupAutoMocTarget(target, autogenTargetName,
                             configIncludes, configDefines);
    }
  if (target->GetPropertyAsBool("AUTOUIC"))
    {
    this->SetupAutoUicTarget(target, configUicOptions);
    }
  if (target->GetPropertyAsBool("AUTORCC"))
    {
    this->SetupAutoRccTarget(target);
    }

  const char* cmakeRoot = makefile->GetSafeDefinition("CMAKE_ROOT");
  std::string inputFile = cmakeRoot;
  inputFile += "/Modules/AutogenInfo.cmake.in";
  std::string outputFile = targetDir;
  outputFile += "/AutogenInfo.cmake";
  makefile->AddDefinition("_qt_rcc_inputs",
              makefile->GetDefinition("_qt_rcc_inputs_" + target->GetName()));
  makefile->ConfigureFile(inputFile.c_str(), outputFile.c_str(),
                          false, true, false);

  // Ensure we have write permission in case .in was read-only.
  mode_t perm = 0;
#if defined(WIN32) && !defined(__CYGWIN__)
  mode_t mode_write = S_IWRITE;
#else
  mode_t mode_write = S_IWUSR;
#endif
  cmSystemTools::GetPermissions(outputFile, perm);
  if (!(perm & mode_write))
    {
    cmSystemTools::SetPermissions(outputFile, perm | mode_write);
    }
  if (!configDefines.empty()
      || !configIncludes.empty()
      || !configUicOptions.empty())
    {
    cmsys::ofstream infoFile(outputFile.c_str(), std::ios::app);
    if ( !infoFile )
      {
      std::string error = "Internal CMake error when trying to open file: ";
      error += outputFile.c_str();
      error += " for writing.";
      cmSystemTools::Error(error.c_str());
      return;
      }
    if (!configDefines.empty())
      {
      for (std::map<std::string, std::string>::iterator
            it = configDefines.begin(), end = configDefines.end();
            it != end; ++it)
        {
        infoFile << "set(AM_MOC_COMPILE_DEFINITIONS_" << it->first <<
          " " << it->second << ")\n";
        }
      }
    if (!configIncludes.empty())
      {
      for (std::map<std::string, std::string>::iterator
            it = configIncludes.begin(), end = configIncludes.end();
            it != end; ++it)
        {
        infoFile << "set(AM_MOC_INCLUDES_" << it->first <<
          " " << it->second << ")\n";
        }
      }
    if (!configUicOptions.empty())
      {
      for (std::map<std::string, std::string>::iterator
            it = configUicOptions.begin(), end = configUicOptions.end();
            it != end; ++it)
        {
        infoFile << "set(AM_UIC_TARGET_OPTIONS_" << it->first <<
          " " << it->second << ")\n";
        }
      }
    }
}

void cmQtAutoGenerators::SetupSourceFiles(cmTarget const* target)
{
  cmMakefile* makefile = target->GetMakefile();

  const char* sepFiles = "";
  const char* sepHeaders = "";

  std::vector<cmSourceFile*> srcFiles;
  target->GetConfigCommonSourceFiles(srcFiles);

  const char *skipMocSep = "";
  const char *skipUicSep = "";

  std::vector<std::string> newRccFiles;

  for(std::vector<cmSourceFile*>::const_iterator fileIt = srcFiles.begin();
      fileIt != srcFiles.end();
      ++fileIt)
    {
    cmSourceFile* sf = *fileIt;
    std::string absFile = cmsys::SystemTools::GetRealPath(
                                                    sf->GetFullPath());
    bool skipMoc = cmSystemTools::IsOn(sf->GetPropertyForUser("SKIP_AUTOMOC"));
    bool generated = cmSystemTools::IsOn(sf->GetPropertyForUser("GENERATED"));

    if(cmSystemTools::IsOn(sf->GetPropertyForUser("SKIP_AUTOUIC")))
      {
      this->SkipUic += skipUicSep;
      this->SkipUic += absFile;
      skipUicSep = ";";
      }

    std::string ext = sf->GetExtension();

    if (target->GetPropertyAsBool("AUTORCC"))
      {
      if (ext == "qrc"
          && !cmSystemTools::IsOn(sf->GetPropertyForUser("SKIP_AUTORCC")))
        {
        std::string basename = cmsys::SystemTools::
                                      GetFilenameWithoutLastExtension(absFile);

        std::string rcc_output_dir = target->GetSupportDirectory();
        cmSystemTools::MakeDirectory(rcc_output_dir.c_str());
        std::string rcc_output_file = rcc_output_dir;
        rcc_output_file += "/qrc_" + basename + ".cpp";
        makefile->AppendProperty("ADDITIONAL_MAKE_CLEAN_FILES",
                                rcc_output_file.c_str(), false);
        makefile->GetOrCreateSource(rcc_output_file, true);
        newRccFiles.push_back(rcc_output_file);
        }
      }

    if (!generated)
      {
      if (skipMoc)
        {
        this->SkipMoc += skipMocSep;
        this->SkipMoc += absFile;
        skipMocSep = ";";
        }
      else
        {
        cmSystemTools::FileFormat fileType = cmSystemTools::GetFileFormat(
                                                                ext.c_str());
        if (fileType == cmSystemTools::CXX_FILE_FORMAT)
          {
          this->Sources += sepFiles;
          this->Sources += absFile;
          sepFiles = ";";
          }
        else if (fileType == cmSystemTools::HEADER_FILE_FORMAT)
          {
          this->Headers += sepHeaders;
          this->Headers += absFile;
          sepHeaders = ";";
          }
        }
      }
    }

  for(std::vector<std::string>::const_iterator fileIt = newRccFiles.begin();
      fileIt != newRccFiles.end();
      ++fileIt)
    {
    const_cast<cmTarget*>(target)->AddSource(*fileIt);
    }
}

void cmQtAutoGenerators::SetupAutoMocTarget(cmTarget const* target,
                          const std::string &autogenTargetName,
                          std::map<std::string, std::string> &configIncludes,
                          std::map<std::string, std::string> &configDefines)
{
  cmMakefile* makefile = target->GetMakefile();

  const char* tmp = target->GetProperty("AUTOMOC_MOC_OPTIONS");
  std::string _moc_options = (tmp!=0 ? tmp : "");
  makefile->AddDefinition("_moc_options",
          cmLocalGenerator::EscapeForCMake(_moc_options).c_str());
  makefile->AddDefinition("_skip_moc",
          cmLocalGenerator::EscapeForCMake(this->SkipMoc).c_str());
  makefile->AddDefinition("_moc_headers",
          cmLocalGenerator::EscapeForCMake(this->Headers).c_str());
  bool relaxedMode = makefile->IsOn("CMAKE_AUTOMOC_RELAXED_MODE");
  makefile->AddDefinition("_moc_relaxed_mode", relaxedMode ? "TRUE" : "FALSE");

  std::string _moc_incs;
  std::string _moc_compile_defs;
  std::vector<std::string> configs;
  const std::string& config = makefile->GetConfigurations(configs);
  GetCompileDefinitionsAndDirectories(target, config,
                                      _moc_incs, _moc_compile_defs);

  makefile->AddDefinition("_moc_incs",
          cmLocalGenerator::EscapeForCMake(_moc_incs).c_str());
  makefile->AddDefinition("_moc_compile_defs",
          cmLocalGenerator::EscapeForCMake(_moc_compile_defs).c_str());

  for (std::vector<std::string>::const_iterator li = configs.begin();
       li != configs.end(); ++li)
    {
    std::string config_moc_incs;
    std::string config_moc_compile_defs;
    GetCompileDefinitionsAndDirectories(target, *li,
                                        config_moc_incs,
                                        config_moc_compile_defs);
    if (config_moc_incs != _moc_incs)
      {
      configIncludes[*li] =
                    cmLocalGenerator::EscapeForCMake(config_moc_incs);
      if(_moc_incs.empty())
        {
        _moc_incs = config_moc_incs;
        }
      }
    if (config_moc_compile_defs != _moc_compile_defs)
      {
      configDefines[*li] =
            cmLocalGenerator::EscapeForCMake(config_moc_compile_defs);
      if(_moc_compile_defs.empty())
        {
        _moc_compile_defs = config_moc_compile_defs;
        }
      }
    }

  const char *qtVersion = makefile->GetDefinition("_target_qt_version");
  if (strcmp(qtVersion, "5") == 0)
    {
    cmTarget *qt5Moc = makefile->FindTargetToUse("Qt5::moc");
    if (!qt5Moc)
      {
      cmSystemTools::Error("Qt5::moc target not found ",
                          autogenTargetName.c_str());
      return;
      }
    makefile->AddDefinition("_qt_moc_executable", qt5Moc->GetLocation(""));
    }
  else if (strcmp(qtVersion, "4") == 0)
    {
    cmTarget *qt4Moc = makefile->FindTargetToUse("Qt4::moc");
    if (!qt4Moc)
      {
      cmSystemTools::Error("Qt4::moc target not found ",
                          autogenTargetName.c_str());
      return;
      }
    makefile->AddDefinition("_qt_moc_executable", qt4Moc->GetLocation(""));
    }
  else
    {
    cmSystemTools::Error("The CMAKE_AUTOMOC feature supports only Qt 4 and "
                        "Qt 5 ", autogenTargetName.c_str());
    }
}

void cmQtAutoGenerators::MergeUicOptions(std::vector<std::string> &opts,
                         const std::vector<std::string> &fileOpts,
                         bool isQt5)
{
  static const char* valueOptions[] = {
    "tr",
    "translate",
    "postfix",
    "generator",
    "include", // Since Qt 5.3
    "g"
  };
  std::vector<std::string> extraOpts;
  for(std::vector<std::string>::const_iterator it = fileOpts.begin();
      it != fileOpts.end(); ++it)
    {
    std::vector<std::string>::iterator existingIt
                                  = std::find(opts.begin(), opts.end(), *it);
    if (existingIt != opts.end())
      {
      const char *o = it->c_str();
      if (*o == '-')
        {
        ++o;
        }
      if (isQt5 && *o == '-')
        {
        ++o;
        }
      if (std::find_if(cmArrayBegin(valueOptions), cmArrayEnd(valueOptions),
                  cmStrCmp(*it)) != cmArrayEnd(valueOptions))
        {
        assert(existingIt + 1 != opts.end());
        *(existingIt + 1) = *(it + 1);
        ++it;
        }
      }
    else
      {
      extraOpts.push_back(*it);
      }
    }
  opts.insert(opts.end(), extraOpts.begin(), extraOpts.end());
}

static void GetUicOpts(cmTarget const* target, const std::string& config,
                       std::string &optString)
{
  std::vector<std::string> opts;
  target->GetAutoUicOptions(opts, config);
  optString = cmJoin(opts, ";");
}

void cmQtAutoGenerators::SetupAutoUicTarget(cmTarget const* target,
                          std::map<std::string, std::string> &configUicOptions)
{
  cmMakefile *makefile = target->GetMakefile();

  std::set<std::string> skipped;
  std::vector<std::string> skipVec;
  cmSystemTools::ExpandListArgument(this->SkipUic, skipVec);
  skipped.insert(skipVec.begin(), skipVec.end());

  makefile->AddDefinition("_skip_uic",
          cmLocalGenerator::EscapeForCMake(this->SkipUic).c_str());

  std::vector<cmSourceFile*> uiFilesWithOptions
                                        = makefile->GetQtUiFilesWithOptions();

  const char *qtVersion = makefile->GetDefinition("_target_qt_version");

  std::string _uic_opts;
  std::vector<std::string> configs;
  const std::string& config = makefile->GetConfigurations(configs);
  GetUicOpts(target, config, _uic_opts);

  if (!_uic_opts.empty())
    {
    _uic_opts = cmLocalGenerator::EscapeForCMake(_uic_opts);
    makefile->AddDefinition("_uic_target_options", _uic_opts.c_str());
    }
  for (std::vector<std::string>::const_iterator li = configs.begin();
       li != configs.end(); ++li)
    {
    std::string config_uic_opts;
    GetUicOpts(target, *li, config_uic_opts);
    if (config_uic_opts != _uic_opts)
      {
      configUicOptions[*li] =
                    cmLocalGenerator::EscapeForCMake(config_uic_opts);
      if(_uic_opts.empty())
        {
        _uic_opts = config_uic_opts;
        }
      }
    }

  std::string uiFileFiles;
  std::string uiFileOptions;
  const char* sep = "";

  for(std::vector<cmSourceFile*>::const_iterator fileIt =
      uiFilesWithOptions.begin();
      fileIt != uiFilesWithOptions.end();
      ++fileIt)
    {
    cmSourceFile* sf = *fileIt;
    std::string absFile = cmsys::SystemTools::GetRealPath(
                                                    sf->GetFullPath());

    if (!skipped.insert(absFile).second)
      {
      continue;
      }
    uiFileFiles += sep;
    uiFileFiles += absFile;
    uiFileOptions += sep;
    std::string opts = sf->GetProperty("AUTOUIC_OPTIONS");
    cmSystemTools::ReplaceString(opts, ";", "@list_sep@");
    uiFileOptions += opts;
    sep = ";";
    }

  makefile->AddDefinition("_qt_uic_options_files",
              cmLocalGenerator::EscapeForCMake(uiFileFiles).c_str());
  makefile->AddDefinition("_qt_uic_options_options",
            cmLocalGenerator::EscapeForCMake(uiFileOptions).c_str());

  std::string targetName = target->GetName();
  if (strcmp(qtVersion, "5") == 0)
    {
    cmTarget *qt5Uic = makefile->FindTargetToUse("Qt5::uic");
    if (!qt5Uic)
      {
      // Project does not use Qt5Widgets, but has AUTOUIC ON anyway
      }
    else
      {
      makefile->AddDefinition("_qt_uic_executable", qt5Uic->GetLocation(""));
      }
    }
  else if (strcmp(qtVersion, "4") == 0)
    {
    cmTarget *qt4Uic = makefile->FindTargetToUse("Qt4::uic");
    if (!qt4Uic)
      {
      cmSystemTools::Error("Qt4::uic target not found ",
                          targetName.c_str());
      return;
      }
    makefile->AddDefinition("_qt_uic_executable", qt4Uic->GetLocation(""));
    }
  else
    {
    cmSystemTools::Error("The CMAKE_AUTOUIC feature supports only Qt 4 and "
                        "Qt 5 ", targetName.c_str());
    }
}

void cmQtAutoGenerators::MergeRccOptions(std::vector<std::string> &opts,
                         const std::vector<std::string> &fileOpts,
                         bool isQt5)
{
  static const char* valueOptions[] = {
    "name",
    "root",
    "compress",
    "threshold"
  };
  std::vector<std::string> extraOpts;
  for(std::vector<std::string>::const_iterator it = fileOpts.begin();
      it != fileOpts.end(); ++it)
    {
    std::vector<std::string>::iterator existingIt
                                  = std::find(opts.begin(), opts.end(), *it);
    if (existingIt != opts.end())
      {
      const char *o = it->c_str();
      if (*o == '-')
        {
        ++o;
        }
      if (isQt5 && *o == '-')
        {
        ++o;
        }
      if (std::find_if(cmArrayBegin(valueOptions), cmArrayEnd(valueOptions),
                  cmStrCmp(*it)) != cmArrayEnd(valueOptions))
        {
        assert(existingIt + 1 != opts.end());
        *(existingIt + 1) = *(it + 1);
        ++it;
        }
      }
    else
      {
      extraOpts.push_back(*it);
      }
    }
  opts.insert(opts.end(), extraOpts.begin(), extraOpts.end());
}

void cmQtAutoGenerators::SetupAutoRccTarget(cmTarget const* target)
{
  std::string _rcc_files;
  const char* sepRccFiles = "";
  cmMakefile *makefile = target->GetMakefile();

  std::vector<cmSourceFile*> srcFiles;
  target->GetConfigCommonSourceFiles(srcFiles);

  std::string qrcInputs;
  const char* qrcInputsSep = "";

  std::string rccFileFiles;
  std::string rccFileOptions;
  const char *optionSep = "";

  const char *qtVersion = makefile->GetDefinition("_target_qt_version");

  std::vector<std::string> rccOptions;
  if (const char* opts = target->GetProperty("AUTORCC_OPTIONS"))
    {
    cmSystemTools::ExpandListArgument(opts, rccOptions);
    }
  std::string qtMajorVersion = makefile->GetSafeDefinition("QT_VERSION_MAJOR");
  if (qtMajorVersion == "")
    {
    qtMajorVersion = makefile->GetSafeDefinition("Qt5Core_VERSION_MAJOR");
    }

  for(std::vector<cmSourceFile*>::const_iterator fileIt = srcFiles.begin();
      fileIt != srcFiles.end();
      ++fileIt)
    {
    cmSourceFile* sf = *fileIt;
    std::string ext = sf->GetExtension();
    if (ext == "qrc")
      {
      std::string absFile = cmsys::SystemTools::GetRealPath(
                                                  sf->GetFullPath());
      bool skip = cmSystemTools::IsOn(sf->GetPropertyForUser("SKIP_AUTORCC"));

      if (!skip)
        {
        _rcc_files += sepRccFiles;
        _rcc_files += absFile;
        sepRccFiles = ";";

        if (const char *prop = sf->GetProperty("AUTORCC_OPTIONS"))
          {
          std::vector<std::string> optsVec;
          cmSystemTools::ExpandListArgument(prop, optsVec);
          this->MergeRccOptions(rccOptions, optsVec,
                                strcmp(qtVersion, "5") == 0);
          }

        if (!rccOptions.empty())
          {
          rccFileFiles += optionSep;
          rccFileFiles += absFile;
          rccFileOptions += optionSep;
          }
        const char *listSep = "";
        for(std::vector<std::string>::const_iterator it = rccOptions.begin();
            it != rccOptions.end();
            ++it)
          {
          rccFileOptions += listSep;
          rccFileOptions += *it;
          listSep = "@list_sep@";
          }
        optionSep = ";";

        std::vector<std::string> depends;

        std::string entriesList;
        if (!cmSystemTools::IsOn(sf->GetPropertyForUser("GENERATED")))
          {
          if (qtMajorVersion == "5")
            {
            entriesList = this->ListQt5RccInputs(sf, target, depends);
            }
          else
            {
            entriesList = this->ListQt4RccInputs(sf, depends);
            }
          if (entriesList.empty())
            {
            return;
            }
          }
        qrcInputs += qrcInputsSep;
        qrcInputs += entriesList;
        qrcInputsSep = ";";
        }
      }
    }
  makefile->AddDefinition("_qt_rcc_inputs_" + target->GetName(),
                      cmLocalGenerator::EscapeForCMake(qrcInputs).c_str());

  makefile->AddDefinition("_rcc_files",
          cmLocalGenerator::EscapeForCMake(_rcc_files).c_str());

  makefile->AddDefinition("_qt_rcc_options_files",
              cmLocalGenerator::EscapeForCMake(rccFileFiles).c_str());
  makefile->AddDefinition("_qt_rcc_options_options",
            cmLocalGenerator::EscapeForCMake(rccFileOptions).c_str());

  makefile->AddDefinition("_qt_rcc_executable",
                          this->GetRccExecutable(target).c_str());
}

std::string cmQtAutoGenerators::GetRccExecutable(cmTarget const* target)
{
  cmMakefile *makefile = target->GetMakefile();
  const char *qtVersion = makefile->GetDefinition("_target_qt_version");
  if (!qtVersion)
    {
    qtVersion = makefile->GetDefinition("Qt5Core_VERSION_MAJOR");
    if (!qtVersion)
      {
      qtVersion = makefile->GetDefinition("QT_VERSION_MAJOR");
      }
    if (const char *targetQtVersion =
        target->GetLinkInterfaceDependentStringProperty("QT_MAJOR_VERSION",
                                                        ""))
      {
      qtVersion = targetQtVersion;
      }
    }

  std::string targetName = target->GetName();
  if (strcmp(qtVersion, "5") == 0)
    {
    cmTarget *qt5Rcc = makefile->FindTargetToUse("Qt5::rcc");
    if (!qt5Rcc)
      {
      cmSystemTools::Error("Qt5::rcc target not found ",
                          targetName.c_str());
      return std::string();
      }
    return qt5Rcc->GetLocation("");
    }
  else if (strcmp(qtVersion, "4") == 0)
    {
    cmTarget *qt4Rcc = makefile->FindTargetToUse("Qt4::rcc");
    if (!qt4Rcc)
      {
      cmSystemTools::Error("Qt4::rcc target not found ",
                          targetName.c_str());
      return std::string();
      }
    return qt4Rcc->GetLocation("");
    }

  cmSystemTools::Error("The CMAKE_AUTORCC feature supports only Qt 4 and "
                      "Qt 5 ", targetName.c_str());
  return std::string();
}

bool cmQtAutoGenerators::Run(const std::string& targetDirectory,
                             const std::string& config)
{
  bool success = true;
  cmake cm;
  cm.SetHomeOutputDirectory(targetDirectory);
  cm.SetHomeDirectory(targetDirectory);
  cmGlobalGenerator gg(&cm);

  cmLocalGenerator* lg = gg.MakeLocalGenerator();
  lg->GetMakefile()->SetCurrentBinaryDirectory(targetDirectory);
  lg->GetMakefile()->SetCurrentSourceDirectory(targetDirectory);
  gg.SetCurrentLocalGenerator(lg);

  this->ReadAutogenInfoFile(lg->GetMakefile(), targetDirectory, config);
  this->ReadOldMocDefinitionsFile(lg->GetMakefile(), targetDirectory);

  this->Init();

  if (this->QtMajorVersion == "4" || this->QtMajorVersion == "5")
    {
    success = this->RunAutogen(lg->GetMakefile());
    }

  this->WriteOldMocDefinitionsFile(targetDirectory);

  delete lg;
  return success;
}

bool cmQtAutoGenerators::ReadAutogenInfoFile(cmMakefile* makefile,
                                      const std::string& targetDirectory,
                                      const std::string& config)
{
  std::string filename(
      cmSystemTools::CollapseFullPath(targetDirectory));
  cmSystemTools::ConvertToUnixSlashes(filename);
  filename += "/AutogenInfo.cmake";

  if (!makefile->ReadListFile(filename.c_str()))
    {
    cmSystemTools::Error("Error processing file: ", filename.c_str());
    return false;
    }

  this->QtMajorVersion = makefile->GetSafeDefinition("AM_QT_VERSION_MAJOR");
  if (this->QtMajorVersion == "")
    {
    this->QtMajorVersion = makefile->GetSafeDefinition(
                                     "AM_Qt5Core_VERSION_MAJOR");
    }
  this->Sources = makefile->GetSafeDefinition("AM_SOURCES");
  {
  std::string rccSources = makefile->GetSafeDefinition("AM_RCC_SOURCES");
  cmSystemTools::ExpandListArgument(rccSources, this->RccSources);
  }
  this->SkipMoc = makefile->GetSafeDefinition("AM_SKIP_MOC");
  this->SkipUic = makefile->GetSafeDefinition("AM_SKIP_UIC");
  this->Headers = makefile->GetSafeDefinition("AM_HEADERS");
  this->IncludeProjectDirsBefore = makefile->IsOn(
                                "AM_CMAKE_INCLUDE_DIRECTORIES_PROJECT_BEFORE");
  this->Srcdir = makefile->GetSafeDefinition("AM_CMAKE_CURRENT_SOURCE_DIR");
  this->Builddir = makefile->GetSafeDefinition("AM_CMAKE_CURRENT_BINARY_DIR");
  this->MocExecutable = makefile->GetSafeDefinition("AM_QT_MOC_EXECUTABLE");
  this->UicExecutable = makefile->GetSafeDefinition("AM_QT_UIC_EXECUTABLE");
  this->RccExecutable = makefile->GetSafeDefinition("AM_QT_RCC_EXECUTABLE");
  {
  std::string compileDefsPropOrig = "AM_MOC_COMPILE_DEFINITIONS";
  std::string compileDefsProp = compileDefsPropOrig;
  if(!config.empty())
    {
    compileDefsProp += "_";
    compileDefsProp += config;
    }
  const char *compileDefs = makefile->GetDefinition(compileDefsProp);
  this->MocCompileDefinitionsStr = compileDefs ? compileDefs
                  : makefile->GetSafeDefinition(compileDefsPropOrig);
  }
  {
  std::string includesPropOrig = "AM_MOC_INCLUDES";
  std::string includesProp = includesPropOrig;
  if(!config.empty())
    {
    includesProp += "_";
    includesProp += config;
    }
  const char *includes = makefile->GetDefinition(includesProp);
  this->MocIncludesStr = includes ? includes
                      : makefile->GetSafeDefinition(includesPropOrig);
  }
  this->MocOptionsStr = makefile->GetSafeDefinition("AM_MOC_OPTIONS");
  this->ProjectBinaryDir = makefile->GetSafeDefinition("AM_CMAKE_BINARY_DIR");
  this->ProjectSourceDir = makefile->GetSafeDefinition("AM_CMAKE_SOURCE_DIR");
  this->TargetName = makefile->GetSafeDefinition("AM_TARGET_NAME");
  this->OriginTargetName
                      = makefile->GetSafeDefinition("AM_ORIGIN_TARGET_NAME");

  {
  const char *uicOptionsFiles
                        = makefile->GetSafeDefinition("AM_UIC_OPTIONS_FILES");
  std::string uicOptionsPropOrig = "AM_UIC_TARGET_OPTIONS";
  std::string uicOptionsProp = uicOptionsPropOrig;
  if(!config.empty())
    {
    uicOptionsProp += "_";
    uicOptionsProp += config;
    }
  const char *uicTargetOptions
                        = makefile->GetSafeDefinition(uicOptionsProp);
  cmSystemTools::ExpandListArgument(
      uicTargetOptions ? uicTargetOptions
                    : makefile->GetSafeDefinition(uicOptionsPropOrig),
    this->UicTargetOptions);
  const char *uicOptionsOptions
                      = makefile->GetSafeDefinition("AM_UIC_OPTIONS_OPTIONS");
  std::vector<std::string> uicFilesVec;
  cmSystemTools::ExpandListArgument(uicOptionsFiles, uicFilesVec);
  std::vector<std::string> uicOptionsVec;
  cmSystemTools::ExpandListArgument(uicOptionsOptions, uicOptionsVec);
  if (uicFilesVec.size() != uicOptionsVec.size())
    {
    return false;
    }
  for (std::vector<std::string>::iterator fileIt = uicFilesVec.begin(),
                                            optionIt = uicOptionsVec.begin();
                                            fileIt != uicFilesVec.end();
                                            ++fileIt, ++optionIt)
    {
    cmSystemTools::ReplaceString(*optionIt, "@list_sep@", ";");
    this->UicOptions[*fileIt] = *optionIt;
    }
  }
  {
  const char *rccOptionsFiles
                        = makefile->GetSafeDefinition("AM_RCC_OPTIONS_FILES");
  const char *rccOptionsOptions
                      = makefile->GetSafeDefinition("AM_RCC_OPTIONS_OPTIONS");
  std::vector<std::string> rccFilesVec;
  cmSystemTools::ExpandListArgument(rccOptionsFiles, rccFilesVec);
  std::vector<std::string> rccOptionsVec;
  cmSystemTools::ExpandListArgument(rccOptionsOptions, rccOptionsVec);
  if (rccFilesVec.size() != rccOptionsVec.size())
    {
    return false;
    }
  for (std::vector<std::string>::iterator fileIt = rccFilesVec.begin(),
                                            optionIt = rccOptionsVec.begin();
                                            fileIt != rccFilesVec.end();
                                            ++fileIt, ++optionIt)
    {
    cmSystemTools::ReplaceString(*optionIt, "@list_sep@", ";");
    this->RccOptions[*fileIt] = *optionIt;
    }

  const char *rccInputs = makefile->GetSafeDefinition("AM_RCC_INPUTS");
  std::vector<std::string> rccInputLists;
  cmSystemTools::ExpandListArgument(rccInputs, rccInputLists);

  if (this->RccSources.size() != rccInputLists.size())
    {
    cmSystemTools::Error("Error processing file: ", filename.c_str());
    return false;
    }

  for (std::vector<std::string>::iterator fileIt = this->RccSources.begin(),
                                            inputIt = rccInputLists.begin();
                                            fileIt != this->RccSources.end();
                                            ++fileIt, ++inputIt)
    {
    cmSystemTools::ReplaceString(*inputIt, "@list_sep@", ";");
    std::vector<std::string> rccInputFiles;
    cmSystemTools::ExpandListArgument(*inputIt, rccInputFiles);

    this->RccInputs[*fileIt] = rccInputFiles;
    }
  }
  this->CurrentCompileSettingsStr = this->MakeCompileSettingsString(makefile);

  this->RelaxedMode = makefile->IsOn("AM_RELAXED_MODE");

  return true;
}


std::string cmQtAutoGenerators::MakeCompileSettingsString(cmMakefile* makefile)
{
  std::string s;
  s += makefile->GetSafeDefinition("AM_MOC_COMPILE_DEFINITIONS");
  s += " ~~~ ";
  s += makefile->GetSafeDefinition("AM_MOC_INCLUDES");
  s += " ~~~ ";
  s += makefile->GetSafeDefinition("AM_MOC_OPTIONS");
  s += " ~~~ ";
  s += makefile->IsOn("AM_CMAKE_INCLUDE_DIRECTORIES_PROJECT_BEFORE") ? "TRUE"
                                                                     : "FALSE";
  s += " ~~~ ";

  return s;
}


bool cmQtAutoGenerators::ReadOldMocDefinitionsFile(cmMakefile* makefile,
                                            const std::string& targetDirectory)
{
  std::string filename(
      cmSystemTools::CollapseFullPath(targetDirectory));
  cmSystemTools::ConvertToUnixSlashes(filename);
  filename += "/AutomocOldMocDefinitions.cmake";

  if (makefile->ReadListFile(filename.c_str()))
    {
    this->OldCompileSettingsStr =
                        makefile->GetSafeDefinition("AM_OLD_COMPILE_SETTINGS");
    }
  return true;
}


void
cmQtAutoGenerators::WriteOldMocDefinitionsFile(
                                            const std::string& targetDirectory)
{
  std::string filename(
      cmSystemTools::CollapseFullPath(targetDirectory));
  cmSystemTools::ConvertToUnixSlashes(filename);
  filename += "/AutomocOldMocDefinitions.cmake";

  cmsys::ofstream outfile;
  outfile.open(filename.c_str(),
               std::ios::trunc);
  outfile << "set(AM_OLD_COMPILE_SETTINGS "
              << cmLocalGenerator::EscapeForCMake(
                 this->CurrentCompileSettingsStr) << ")\n";

  outfile.close();
}


void cmQtAutoGenerators::Init()
{
  this->OutMocCppFilename = this->Builddir;
  this->OutMocCppFilename += this->TargetName;
  this->OutMocCppFilename += ".cpp";

  std::vector<std::string> cdefList;
  cmSystemTools::ExpandListArgument(this->MocCompileDefinitionsStr, cdefList);
  for(std::vector<std::string>::const_iterator it = cdefList.begin();
      it != cdefList.end();
      ++it)
    {
    this->MocDefinitions.push_back("-D" + (*it));
    }

  cmSystemTools::ExpandListArgument(this->MocOptionsStr, this->MocOptions);

  std::vector<std::string> incPaths;
  cmSystemTools::ExpandListArgument(this->MocIncludesStr, incPaths);

  std::set<std::string> frameworkPaths;
  for(std::vector<std::string>::const_iterator it = incPaths.begin();
      it != incPaths.end();
      ++it)
    {
    const std::string &path = *it;
    this->MocIncludes.push_back("-I" + path);
    if (cmHasLiteralSuffix(path, ".framework/Headers"))
      {
      // Go up twice to get to the framework root
      std::vector<std::string> pathComponents;
      cmsys::SystemTools::SplitPath(path, pathComponents);
      std::string frameworkPath =cmsys::SystemTools::JoinPath(
                             pathComponents.begin(), pathComponents.end() - 2);
      frameworkPaths.insert(frameworkPath);
      }
    }

  for (std::set<std::string>::const_iterator it = frameworkPaths.begin();
         it != frameworkPaths.end(); ++it)
    {
    this->MocIncludes.push_back("-F");
    this->MocIncludes.push_back(*it);
    }


    if (this->IncludeProjectDirsBefore)
      {
      const std::string binDir = "-I" + this->ProjectBinaryDir;

      const std::string srcDir = "-I" + this->ProjectSourceDir;

      std::list<std::string> sortedMocIncludes;
      std::list<std::string>::iterator it = this->MocIncludes.begin();
      while (it != this->MocIncludes.end())
        {
        if (this->StartsWith(*it, binDir))
          {
          sortedMocIncludes.push_back(*it);
          it = this->MocIncludes.erase(it);
          }
        else
          {
          ++it;
          }
        }
      it = this->MocIncludes.begin();
      while (it != this->MocIncludes.end())
        {
        if (this->StartsWith(*it, srcDir))
          {
          sortedMocIncludes.push_back(*it);
          it = this->MocIncludes.erase(it);
          }
        else
          {
          ++it;
          }
        }
      sortedMocIncludes.insert(sortedMocIncludes.end(),
                           this->MocIncludes.begin(), this->MocIncludes.end());
      this->MocIncludes = sortedMocIncludes;
    }

}


bool cmQtAutoGenerators::RunAutogen(cmMakefile* makefile)
{
  if (!cmsys::SystemTools::FileExists(this->OutMocCppFilename.c_str())
    || (this->OldCompileSettingsStr != this->CurrentCompileSettingsStr))
    {
    this->GenerateAll = true;
    }

  // the program goes through all .cpp files to see which moc files are
  // included. It is not really interesting how the moc file is named, but
  // what file the moc is created from. Once a moc is included the same moc
  // may not be included in the _automoc.cpp file anymore. OTOH if there's a
  // header containing Q_OBJECT where no corresponding moc file is included
  // anywhere a moc_<filename>.cpp file is created and included in
  // the _automoc.cpp file.

  // key = moc source filepath, value = moc output filepath
  std::map<std::string, std::string> includedMocs;
  // collect all headers which may need to be mocced
  std::set<std::string> headerFiles;

  std::vector<std::string> sourceFiles;
  cmSystemTools::ExpandListArgument(this->Sources, sourceFiles);

  const std::vector<std::string>& headerExtensions =
                                               makefile->GetHeaderExtensions();

  std::map<std::string, std::vector<std::string> > includedUis;
  std::map<std::string, std::vector<std::string> > skippedUis;
  std::vector<std::string> uicSkipped;
  cmSystemTools::ExpandListArgument(this->SkipUic, uicSkipped);

  for (std::vector<std::string>::const_iterator it = sourceFiles.begin();
       it != sourceFiles.end();
       ++it)
    {
    const bool skipUic = std::find(uicSkipped.begin(), uicSkipped.end(), *it)
        != uicSkipped.end();
    std::map<std::string, std::vector<std::string> >& uiFiles
                                          = skipUic ? skippedUis : includedUis;
    const std::string &absFilename = *it;
    if (this->Verbose)
      {
      std::cout << "AUTOGEN: Checking " << absFilename << std::endl;
      }
    if (this->RelaxedMode)
      {
      this->ParseCppFile(absFilename, headerExtensions, includedMocs,
                         uiFiles);
      }
    else
      {
      this->StrictParseCppFile(absFilename, headerExtensions, includedMocs,
                               uiFiles);
      }
    this->SearchHeadersForCppFile(absFilename, headerExtensions, headerFiles);
    }

  {
  std::vector<std::string> mocSkipped;
  cmSystemTools::ExpandListArgument(this->SkipMoc, mocSkipped);
  for (std::vector<std::string>::const_iterator it = mocSkipped.begin();
       it != mocSkipped.end();
       ++it)
    {
    if (std::find(uicSkipped.begin(), uicSkipped.end(), *it)
        != uicSkipped.end())
      {
      const std::string &absFilename = *it;
      if (this->Verbose)
        {
        std::cout << "AUTOGEN: Checking " << absFilename << std::endl;
        }
      this->ParseForUic(absFilename, includedUis);
      }
    }
  }

  std::vector<std::string> headerFilesVec;
  cmSystemTools::ExpandListArgument(this->Headers, headerFilesVec);
  headerFiles.insert(headerFilesVec.begin(), headerFilesVec.end());

  // key = moc source filepath, value = moc output filename
  std::map<std::string, std::string> notIncludedMocs;
  this->ParseHeaders(headerFiles, includedMocs, notIncludedMocs, includedUis);

  // run moc on all the moc's that are #included in source files
  for(std::map<std::string, std::string>::const_iterator
                                                     it = includedMocs.begin();
      it != includedMocs.end();
      ++it)
    {
    this->GenerateMoc(it->first, it->second);
    }
  for(std::map<std::string, std::vector<std::string> >::const_iterator
      it = includedUis.begin();
      it != includedUis.end();
      ++it)
    {
    for (std::vector<std::string>::const_iterator nit = it->second.begin();
        nit != it->second.end();
        ++nit)
      {
      this->GenerateUi(it->first, *nit);
      }
    }

  if(!this->RccExecutable.empty())
    {
    this->GenerateQrc();
    }

  cmsys_ios::stringstream outStream;
  outStream << "/* This file is autogenerated, do not edit*/\n";

  bool automocCppChanged = false;
  if (notIncludedMocs.empty())
    {
    outStream << "enum some_compilers { need_more_than_nothing };\n";
    }
  else
    {
    // run moc on the remaining headers and include them in
    // the _automoc.cpp file
    for(std::map<std::string, std::string>::const_iterator
                                                  it = notIncludedMocs.begin();
        it != notIncludedMocs.end();
        ++it)
      {
      bool mocSuccess = this->GenerateMoc(it->first, it->second);
      if (mocSuccess)
        {
        automocCppChanged = true;
        }
      outStream << "#include \"" << it->second << "\"\n";
      }
    }

  if (this->RunMocFailed)
    {
    std::cerr << "moc failed..." << std::endl;
    return false;
    }

  if (this->RunUicFailed)
    {
    std::cerr << "uic failed..." << std::endl;
    return false;
    }
  if (this->RunRccFailed)
    {
    std::cerr << "rcc failed..." << std::endl;
    return false;
    }
  outStream.flush();
  std::string automocSource = outStream.str();
  if (!automocCppChanged)
    {
    // compare contents of the _automoc.cpp file
    const std::string oldContents = ReadAll(this->OutMocCppFilename);
    if (oldContents == automocSource)
      {
      // nothing changed: don't touch the _automoc.cpp file
      return true;
      }
    }

  // source file that includes all remaining moc files (_automoc.cpp file)
  cmsys::ofstream outfile;
  outfile.open(this->OutMocCppFilename.c_str(),
               std::ios::trunc);
  outfile << automocSource;
  outfile.close();

  return true;
}


void cmQtAutoGenerators::ParseCppFile(const std::string& absFilename,
                const std::vector<std::string>& headerExtensions,
                std::map<std::string, std::string>& includedMocs,
                std::map<std::string, std::vector<std::string> > &includedUis)
{
  cmsys::RegularExpression mocIncludeRegExp(
              "[\n][ \t]*#[ \t]*include[ \t]+"
              "[\"<](([^ \">]+/)?moc_[^ \">/]+\\.cpp|[^ \">]+\\.moc)[\">]");

  const std::string contentsString = ReadAll(absFilename);
  if (contentsString.empty())
    {
    std::cerr << "AUTOGEN: warning: " << absFilename << ": file is empty\n"
              << std::endl;
    return;
    }
  this->ParseForUic(absFilename, contentsString, includedUis);
  if (this->MocExecutable.empty())
    {
    return;
    }

  const std::string absPath = cmsys::SystemTools::GetFilenamePath(
                   cmsys::SystemTools::GetRealPath(absFilename)) + '/';
  const std::string scannedFileBasename = cmsys::SystemTools::
                                  GetFilenameWithoutLastExtension(absFilename);
  std::string macroName;
  const bool requiresMoc = requiresMocing(contentsString, macroName);
  bool dotMocIncluded = false;
  bool mocUnderscoreIncluded = false;
  std::string ownMocUnderscoreFile;
  std::string ownDotMocFile;
  std::string ownMocHeaderFile;

  std::string::size_type matchOffset = 0;
  // first a simple string check for "moc" is *much* faster than the regexp,
  // and if the string search already fails, we don't have to try the
  // expensive regexp
  if ((strstr(contentsString.c_str(), "moc") != NULL)
                                    && (mocIncludeRegExp.find(contentsString)))
    {
    // for every moc include in the file
    do
      {
      const std::string currentMoc = mocIncludeRegExp.match(1);
      //std::cout << "found moc include: " << currentMoc << std::endl;

      std::string basename = cmsys::SystemTools::
                                   GetFilenameWithoutLastExtension(currentMoc);
      const bool moc_style = cmHasLiteralPrefix(basename, "moc_");

      // If the moc include is of the moc_foo.cpp style we expect
      // the Q_OBJECT class declaration in a header file.
      // If the moc include is of the foo.moc style we need to look for
      // a Q_OBJECT macro in the current source file, if it contains the
      // macro we generate the moc file from the source file.
      // Q_OBJECT
      if (moc_style)
        {
        // basename should be the part of the moc filename used for
        // finding the correct header, so we need to remove the moc_ part
        basename = basename.substr(4);
        std::string mocSubDir = extractSubDir(absPath, currentMoc);
        std::string headerToMoc = findMatchingHeader(
                               absPath, mocSubDir, basename, headerExtensions);

        if (!headerToMoc.empty())
          {
          includedMocs[headerToMoc] = currentMoc;
          if (basename == scannedFileBasename)
            {
            mocUnderscoreIncluded = true;
            ownMocUnderscoreFile = currentMoc;
            ownMocHeaderFile = headerToMoc;
            }
          }
        else
          {
          std::cerr << "AUTOGEN: error: " << absFilename << ": The file "
                    << "includes the moc file \"" << currentMoc << "\", "
                    << "but could not find header \"" << basename
                    << '{' << this->Join(headerExtensions, ',') << "}\" ";
          if (mocSubDir.empty())
            {
            std::cerr << "in " << absPath << "\n" << std::endl;
            }
          else
            {
            std::cerr << "neither in " << absPath
                      << " nor in " << mocSubDir << "\n" << std::endl;
            }

          ::exit(EXIT_FAILURE);
          }
        }
      else
        {
        std::string fileToMoc = absFilename;
        if ((basename != scannedFileBasename) || (requiresMoc==false))
          {
          std::string mocSubDir = extractSubDir(absPath, currentMoc);
          std::string headerToMoc = findMatchingHeader(
                              absPath, mocSubDir, basename, headerExtensions);
          if (!headerToMoc.empty())
            {
            // this is for KDE4 compatibility:
            fileToMoc = headerToMoc;
            if ((requiresMoc==false) &&(basename==scannedFileBasename))
              {
              std::cerr << "AUTOGEN: warning: " << absFilename << ": The file "
                            "includes the moc file \"" << currentMoc <<
                            "\", but does not contain a " << macroName
                            << " macro. Running moc on "
                        << "\"" << headerToMoc << "\" ! Include \"moc_"
                        << basename << ".cpp\" for a compatiblity with "
                           "strict mode (see CMAKE_AUTOMOC_RELAXED_MODE).\n"
                        << std::endl;
              }
            else
              {
              std::cerr << "AUTOGEN: warning: " << absFilename << ": The file "
                            "includes the moc file \"" << currentMoc <<
                            "\" instead of \"moc_" << basename << ".cpp\". "
                            "Running moc on "
                        << "\"" << headerToMoc << "\" ! Include \"moc_"
                        << basename << ".cpp\" for compatiblity with "
                           "strict mode (see CMAKE_AUTOMOC_RELAXED_MODE).\n"
                        << std::endl;
              }
            }
          else
            {
            std::cerr <<"AUTOGEN: error: " << absFilename << ": The file "
                        "includes the moc file \"" << currentMoc <<
                        "\", which seems to be the moc file from a different "
                        "source file. CMake also could not find a matching "
                        "header.\n" << std::endl;
            ::exit(EXIT_FAILURE);
            }
          }
        else
          {
          dotMocIncluded = true;
          ownDotMocFile = currentMoc;
          }
        includedMocs[fileToMoc] = currentMoc;
        }
      matchOffset += mocIncludeRegExp.end();
      } while(mocIncludeRegExp.find(contentsString.c_str() + matchOffset));
    }

  // In this case, check whether the scanned file itself contains a Q_OBJECT.
  // If this is the case, the moc_foo.cpp should probably be generated from
  // foo.cpp instead of foo.h, because otherwise it won't build.
  // But warn, since this is not how it is supposed to be used.
  if ((dotMocIncluded == false) && (requiresMoc == true))
    {
    if (mocUnderscoreIncluded == true)
      {
      // this is for KDE4 compatibility:
      std::cerr << "AUTOGEN: warning: " << absFilename << ": The file "
                << "contains a " << macroName << " macro, but does not "
                "include "
                << "\"" << scannedFileBasename << ".moc\", but instead "
                   "includes "
                << "\"" << ownMocUnderscoreFile  << "\". Running moc on "
                << "\"" << absFilename << "\" ! Better include \""
                << scannedFileBasename << ".moc\" for compatiblity with "
                   "strict mode (see CMAKE_AUTOMOC_RELAXED_MODE).\n"
                << std::endl;
      includedMocs[absFilename] = ownMocUnderscoreFile;
      includedMocs.erase(ownMocHeaderFile);
      }
    else
      {
      // otherwise always error out since it will not compile:
      std::cerr << "AUTOGEN: error: " << absFilename << ": The file "
                << "contains a " << macroName << " macro, but does not "
                "include "
                << "\"" << scannedFileBasename << ".moc\" !\n"
                << std::endl;
      ::exit(EXIT_FAILURE);
      }
    }

}


void cmQtAutoGenerators::StrictParseCppFile(const std::string& absFilename,
                const std::vector<std::string>& headerExtensions,
                std::map<std::string, std::string>& includedMocs,
                std::map<std::string, std::vector<std::string> >& includedUis)
{
  cmsys::RegularExpression mocIncludeRegExp(
              "[\n][ \t]*#[ \t]*include[ \t]+"
              "[\"<](([^ \">]+/)?moc_[^ \">/]+\\.cpp|[^ \">]+\\.moc)[\">]");

  const std::string contentsString = ReadAll(absFilename);
  if (contentsString.empty())
    {
    std::cerr << "AUTOGEN: warning: " << absFilename << ": file is empty\n"
              << std::endl;
    return;
    }
  this->ParseForUic(absFilename, contentsString, includedUis);
  if (this->MocExecutable.empty())
    {
    return;
    }

  const std::string absPath = cmsys::SystemTools::GetFilenamePath(
                   cmsys::SystemTools::GetRealPath(absFilename)) + '/';
  const std::string scannedFileBasename = cmsys::SystemTools::
                                  GetFilenameWithoutLastExtension(absFilename);

  bool dotMocIncluded = false;

  std::string::size_type matchOffset = 0;
  // first a simple string check for "moc" is *much* faster than the regexp,
  // and if the string search already fails, we don't have to try the
  // expensive regexp
  if ((strstr(contentsString.c_str(), "moc") != NULL)
                                    && (mocIncludeRegExp.find(contentsString)))
    {
    // for every moc include in the file
    do
      {
      const std::string currentMoc = mocIncludeRegExp.match(1);

      std::string basename = cmsys::SystemTools::
                                   GetFilenameWithoutLastExtension(currentMoc);
      const bool mocUnderscoreStyle = cmHasLiteralPrefix(basename, "moc_");

      // If the moc include is of the moc_foo.cpp style we expect
      // the Q_OBJECT class declaration in a header file.
      // If the moc include is of the foo.moc style we need to look for
      // a Q_OBJECT macro in the current source file, if it contains the
      // macro we generate the moc file from the source file.
      if (mocUnderscoreStyle)
        {
        // basename should be the part of the moc filename used for
        // finding the correct header, so we need to remove the moc_ part
        basename = basename.substr(4);
        std::string mocSubDir = extractSubDir(absPath, currentMoc);
        std::string headerToMoc = findMatchingHeader(
                               absPath, mocSubDir, basename, headerExtensions);

        if (!headerToMoc.empty())
          {
          includedMocs[headerToMoc] = currentMoc;
          }
        else
          {
          std::cerr << "AUTOGEN: error: " << absFilename << " The file "
                    << "includes the moc file \"" << currentMoc << "\", "
                    << "but could not find header \"" << basename
                    << '{' << this->Join(headerExtensions, ',') << "}\" ";
          if (mocSubDir.empty())
            {
            std::cerr << "in " << absPath << "\n" << std::endl;
            }
          else
            {
            std::cerr << "neither in " << absPath
                      << " nor in " << mocSubDir << "\n" << std::endl;
            }

          ::exit(EXIT_FAILURE);
          }
        }
      else
        {
        if (basename != scannedFileBasename)
          {
          std::cerr <<"AUTOGEN: error: " << absFilename << ": The file "
                      "includes the moc file \"" << currentMoc <<
                      "\", which seems to be the moc file from a different "
                      "source file. This is not supported. "
                      "Include \"" << scannedFileBasename << ".moc\" to run "
                      "moc on this source file.\n" << std::endl;
          ::exit(EXIT_FAILURE);
          }
        dotMocIncluded = true;
        includedMocs[absFilename] = currentMoc;
        }
      matchOffset += mocIncludeRegExp.end();
      } while(mocIncludeRegExp.find(contentsString.c_str() + matchOffset));
    }

  // In this case, check whether the scanned file itself contains a Q_OBJECT.
  // If this is the case, the moc_foo.cpp should probably be generated from
  // foo.cpp instead of foo.h, because otherwise it won't build.
  // But warn, since this is not how it is supposed to be used.
  std::string macroName;
  if ((dotMocIncluded == false) && (requiresMocing(contentsString,
                                                     macroName)))
    {
    // otherwise always error out since it will not compile:
    std::cerr << "AUTOGEN: error: " << absFilename << ": The file "
              << "contains a " << macroName << " macro, but does not include "
              << "\"" << scannedFileBasename << ".moc\" !\n"
              << std::endl;
    ::exit(EXIT_FAILURE);
    }

}


void cmQtAutoGenerators::ParseForUic(const std::string& absFilename,
                std::map<std::string, std::vector<std::string> >& includedUis)
{
  if (this->UicExecutable.empty())
    {
    return;
    }
  const std::string contentsString = ReadAll(absFilename);
  if (contentsString.empty())
    {
    std::cerr << "AUTOGEN: warning: " << absFilename << ": file is empty\n"
              << std::endl;
    return;
    }
  this->ParseForUic(absFilename, contentsString, includedUis);
}


void cmQtAutoGenerators::ParseForUic(const std::string& absFilename,
                const std::string& contentsString,
                std::map<std::string, std::vector<std::string> >& includedUis)
{
  if (this->UicExecutable.empty())
    {
    return;
    }
  cmsys::RegularExpression uiIncludeRegExp(
              "[\n][ \t]*#[ \t]*include[ \t]+"
              "[\"<](([^ \">]+/)?ui_[^ \">/]+\\.h)[\">]");

  std::string::size_type matchOffset = 0;

  const std::string realName =
                   cmsys::SystemTools::GetRealPath(absFilename);

  matchOffset = 0;
  if ((strstr(contentsString.c_str(), "ui_") != NULL)
                                    && (uiIncludeRegExp.find(contentsString)))
    {
    do
      {
      const std::string currentUi = uiIncludeRegExp.match(1);

      std::string basename = cmsys::SystemTools::
                                  GetFilenameWithoutLastExtension(currentUi);

      // basename should be the part of the ui filename used for
      // finding the correct header, so we need to remove the ui_ part
      basename = basename.substr(3);

      includedUis[realName].push_back(basename);

      matchOffset += uiIncludeRegExp.end();
      } while(uiIncludeRegExp.find(contentsString.c_str() + matchOffset));
    }
}


void
cmQtAutoGenerators::SearchHeadersForCppFile(const std::string& absFilename,
                              const std::vector<std::string>& headerExtensions,
                              std::set<std::string>& absHeaders)
{
  // search for header files and private header files we may need to moc:
  const std::string basename =
              cmsys::SystemTools::GetFilenameWithoutLastExtension(absFilename);
  const std::string absPath = cmsys::SystemTools::GetFilenamePath(
                   cmsys::SystemTools::GetRealPath(absFilename)) + '/';

  for(std::vector<std::string>::const_iterator ext = headerExtensions.begin();
      ext != headerExtensions.end();
      ++ext)
    {
    const std::string headerName = absPath + basename + "." + (*ext);
    if (cmsys::SystemTools::FileExists(headerName.c_str()))
      {
      absHeaders.insert(headerName);
      break;
      }
    }
  for(std::vector<std::string>::const_iterator ext = headerExtensions.begin();
      ext != headerExtensions.end();
      ++ext)
    {
    const std::string privateHeaderName = absPath+basename+"_p."+(*ext);
    if (cmsys::SystemTools::FileExists(privateHeaderName.c_str()))
      {
      absHeaders.insert(privateHeaderName);
      break;
      }
    }

}


void cmQtAutoGenerators::ParseHeaders(const std::set<std::string>& absHeaders,
                const std::map<std::string, std::string>& includedMocs,
                std::map<std::string, std::string>& notIncludedMocs,
                std::map<std::string, std::vector<std::string> >& includedUis)
{
  for(std::set<std::string>::const_iterator hIt=absHeaders.begin();
      hIt!=absHeaders.end();
      ++hIt)
    {
    const std::string& headerName = *hIt;
    const std::string contents = ReadAll(headerName);

    if (!this->MocExecutable.empty()
        && includedMocs.find(headerName) == includedMocs.end())
      {
      if (this->Verbose)
        {
        std::cout << "AUTOGEN: Checking " << headerName << std::endl;
        }

      const std::string basename = cmsys::SystemTools::
                                   GetFilenameWithoutLastExtension(headerName);

      const std::string currentMoc = "moc_" + basename + ".cpp";
      std::string macroName;
      if (requiresMocing(contents, macroName))
        {
        //std::cout << "header contains Q_OBJECT macro";
        notIncludedMocs[headerName] = currentMoc;
        }
      }
    this->ParseForUic(headerName, contents, includedUis);
    }
}

bool cmQtAutoGenerators::GenerateMoc(const std::string& sourceFile,
                              const std::string& mocFileName)
{
  const std::string mocFilePath = this->Builddir + mocFileName;
  int sourceNewerThanMoc = 0;
  bool success = cmsys::SystemTools::FileTimeCompare(sourceFile,
                                                     mocFilePath,
                                                     &sourceNewerThanMoc);
  if (this->GenerateAll || !success || sourceNewerThanMoc >= 0)
    {
    // make sure the directory for the resulting moc file exists
    std::string mocDir = mocFilePath.substr(0, mocFilePath.rfind('/'));
    if (!cmsys::SystemTools::FileExists(mocDir.c_str(), false))
      {
      cmsys::SystemTools::MakeDirectory(mocDir.c_str());
      }

    std::string msg = "Generating ";
    msg += mocFileName;
    cmSystemTools::MakefileColorEcho(cmsysTerminal_Color_ForegroundBlue
                                           |cmsysTerminal_Color_ForegroundBold,
                                     msg.c_str(), true, this->ColorOutput);

    std::vector<std::string> command;
    command.push_back(this->MocExecutable);
    command.insert(command.end(),
                   this->MocIncludes.begin(), this->MocIncludes.end());
    command.insert(command.end(),
                   this->MocDefinitions.begin(), this->MocDefinitions.end());
    command.insert(command.end(),
                   this->MocOptions.begin(), this->MocOptions.end());
#ifdef _WIN32
    command.push_back("-DWIN32");
#endif
    command.push_back("-o");
    command.push_back(mocFilePath);
    command.push_back(sourceFile);

    if (this->Verbose)
      {
      for(std::vector<std::string>::const_iterator cmdIt = command.begin();
          cmdIt != command.end();
          ++cmdIt)
        {
        std::cout << *cmdIt << " ";
        }
      std::cout << std::endl;
      }

    std::string output;
    int retVal = 0;
    bool result = cmSystemTools::RunSingleCommand(command, &output, &output,
                                                  &retVal);
    if (!result || retVal)
      {
      std::cerr << "AUTOGEN: error: process for " << mocFilePath <<" failed:\n"
                << output << std::endl;
      this->RunMocFailed = true;
      cmSystemTools::RemoveFile(mocFilePath);
      }
    return true;
    }
  return false;
}

bool cmQtAutoGenerators::GenerateUi(const std::string& realName,
                                    const std::string& uiFileName)
{
  if (!cmsys::SystemTools::FileExists(this->Builddir.c_str(), false))
    {
    cmsys::SystemTools::MakeDirectory(this->Builddir.c_str());
    }

  const std::string path = cmsys::SystemTools::GetFilenamePath(
                                                      realName) + '/';

  std::string ui_output_file = "ui_" + uiFileName + ".h";
  std::string ui_input_file = path + uiFileName + ".ui";

  int sourceNewerThanUi = 0;
  bool success = cmsys::SystemTools::FileTimeCompare(ui_input_file,
                                    this->Builddir + ui_output_file,
                                                     &sourceNewerThanUi);
  if (this->GenerateAll || !success || sourceNewerThanUi >= 0)
    {
    std::string msg = "Generating ";
    msg += ui_output_file;
    cmSystemTools::MakefileColorEcho(cmsysTerminal_Color_ForegroundBlue
                                          |cmsysTerminal_Color_ForegroundBold,
                                      msg.c_str(), true, this->ColorOutput);

    std::vector<std::string> command;
    command.push_back(this->UicExecutable);

    std::vector<std::string> opts = this->UicTargetOptions;
    std::map<std::string, std::string>::const_iterator optionIt
            = this->UicOptions.find(ui_input_file);
    if (optionIt != this->UicOptions.end())
      {
      std::vector<std::string> fileOpts;
      cmSystemTools::ExpandListArgument(optionIt->second, fileOpts);
      this->MergeUicOptions(opts, fileOpts, this->QtMajorVersion == "5");
      }
    command.insert(command.end(), opts.begin(), opts.end());

    command.push_back("-o");
    command.push_back(this->Builddir + ui_output_file);
    command.push_back(ui_input_file);

    if (this->Verbose)
      {
      for(std::vector<std::string>::const_iterator cmdIt = command.begin();
          cmdIt != command.end();
          ++cmdIt)
        {
        std::cout << *cmdIt << " ";
        }
      std::cout << std::endl;
      }
    std::string output;
    int retVal = 0;
    bool result = cmSystemTools::RunSingleCommand(command, &output, &output,
                                                  &retVal);
    if (!result || retVal)
      {
      std::cerr << "AUTOUIC: error: process for " << ui_output_file <<
                " needed by\n \"" << realName << "\"\nfailed:\n" << output
                << std::endl;
      this->RunUicFailed = true;
      cmSystemTools::RemoveFile(ui_output_file);
      return false;
      }
    return true;
    }
  return false;
}

bool cmQtAutoGenerators::InputFilesNewerThanQrc(const std::string& qrcFile,
                                                const std::string& rccOutput)
{
  std::vector<std::string> const& files = this->RccInputs[qrcFile];
  for (std::vector<std::string>::const_iterator it = files.begin();
       it != files.end(); ++it)
    {
    int inputNewerThanQrc = 0;
    bool success = cmsys::SystemTools::FileTimeCompare(*it,
                                                      rccOutput,
                                                      &inputNewerThanQrc);
    if (!success || inputNewerThanQrc >= 0)
      {
      return true;
      }
    }
  return false;
}

bool cmQtAutoGenerators::GenerateQrc()
{
  for(std::vector<std::string>::const_iterator si = this->RccSources.begin();
      si != this->RccSources.end(); ++si)
    {
    std::string ext = cmsys::SystemTools::GetFilenameLastExtension(*si);

    if (ext != ".qrc")
      {
      continue;
      }
    std::vector<std::string> command;
    command.push_back(this->RccExecutable);

    std::string basename = cmsys::SystemTools::
                                  GetFilenameWithoutLastExtension(*si);

    std::string rcc_output_file = this->Builddir
                                + "CMakeFiles/" + this->OriginTargetName
                                + ".dir/qrc_" + basename + ".cpp";

    int sourceNewerThanQrc = 0;
    bool generateQrc = !cmsys::SystemTools::FileTimeCompare(*si,
                                                      rcc_output_file,
                                                      &sourceNewerThanQrc);
    generateQrc = generateQrc || (sourceNewerThanQrc >= 0);
    generateQrc = generateQrc || this->InputFilesNewerThanQrc(*si,
                                                          rcc_output_file);

    if (this->GenerateAll || generateQrc)
      {
      std::map<std::string, std::string>::const_iterator optionIt
              = this->RccOptions.find(*si);
      if (optionIt != this->RccOptions.end())
        {
        cmSystemTools::ExpandListArgument(optionIt->second, command);
        }

      command.push_back("-name");
      command.push_back(basename);
      command.push_back("-o");
      command.push_back(rcc_output_file);
      command.push_back(*si);

      if (this->Verbose)
        {
        for(std::vector<std::string>::const_iterator cmdIt = command.begin();
            cmdIt != command.end();
            ++cmdIt)
          {
          std::cout << *cmdIt << " ";
          }
        std::cout << std::endl;
        }
      std::string output;
      int retVal = 0;
      bool result = cmSystemTools::RunSingleCommand(command, &output, &output,
                                                    &retVal);
      if (!result || retVal)
        {
        std::cerr << "AUTORCC: error: process for " << rcc_output_file <<
                  " failed:\n" << output << std::endl;
        this->RunRccFailed = true;
        cmSystemTools::RemoveFile(rcc_output_file);
        return false;
        }
      }
    }
  return true;
}

std::string cmQtAutoGenerators::Join(const std::vector<std::string>& lst,
                              char separator)
{
    if (lst.empty())
      {
      return "";
      }

    std::string result;
    for (std::vector<std::string>::const_iterator it = lst.begin();
         it != lst.end();
         ++it)
      {
      result += "." + (*it) + separator;
      }
    result.erase(result.end() - 1);
    return result;
}


bool cmQtAutoGenerators::StartsWith(const std::string& str,
                                    const std::string& with)
{
  return (str.substr(0, with.length()) == with);
}


bool cmQtAutoGenerators::EndsWith(const std::string& str,
                                  const std::string& with)
{
  if (with.length() > (str.length()))
    {
    return false;
    }
  return (str.substr(str.length() - with.length(), with.length()) == with);
}
