/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "cmCTestLaunch.h"

#include "cmGeneratedFileStream.h"
#include "cmSystemTools.h"
#include "cmXMLWriter.h"
#include "cmake.h"

#include <cmsys/MD5.h>
#include <cmsys/Process.h>
#include <cmsys/RegularExpression.hxx>
#include <cmsys/FStream.hxx>

#ifdef _WIN32
#include <io.h> // for _setmode
#include <fcntl.h> // for _O_BINARY
#include <stdio.h> // for std{out,err} and fileno
#endif

//----------------------------------------------------------------------------
cmCTestLaunch::cmCTestLaunch(int argc, const char* const* argv)
{
  this->Passthru = true;
  this->Process = 0;
  this->ExitCode = 1;
  this->CWD = cmSystemTools::GetCurrentWorkingDirectory();

  if(!this->ParseArguments(argc, argv))
    {
    return;
    }

  this->ComputeFileNames();

  this->ScrapeRulesLoaded = false;
  this->HaveOut = false;
  this->HaveErr = false;
  this->Process = cmsysProcess_New();
}

//----------------------------------------------------------------------------
cmCTestLaunch::~cmCTestLaunch()
{
  cmsysProcess_Delete(this->Process);
  if(!this->Passthru)
    {
    cmSystemTools::RemoveFile(this->LogOut);
    cmSystemTools::RemoveFile(this->LogErr);
    }
}

//----------------------------------------------------------------------------
bool cmCTestLaunch::ParseArguments(int argc, const char* const* argv)
{
  // Launcher options occur first and are separated from the real
  // command line by a '--' option.
  enum Doing { DoingNone,
               DoingOutput,
               DoingSource,
               DoingLanguage,
               DoingTargetName,
               DoingTargetType,
               DoingBuildDir,
               DoingCount,
               DoingFilterPrefix };
  Doing doing = DoingNone;
  int arg0 = 0;
  for(int i=1; !arg0 && i < argc; ++i)
    {
    const char* arg = argv[i];
    if(strcmp(arg, "--") == 0)
      {
      arg0 = i+1;
      }
    else if(strcmp(arg, "--output") == 0)
      {
      doing = DoingOutput;
      }
    else if(strcmp(arg, "--source") == 0)
      {
      doing = DoingSource;
      }
    else if(strcmp(arg, "--language") == 0)
      {
      doing = DoingLanguage;
      }
    else if(strcmp(arg, "--target-name") == 0)
      {
      doing = DoingTargetName;
      }
    else if(strcmp(arg, "--target-type") == 0)
      {
      doing = DoingTargetType;
      }
    else if(strcmp(arg, "--build-dir") == 0)
      {
      doing = DoingBuildDir;
      }
    else if(strcmp(arg, "--filter-prefix") == 0)
      {
      doing = DoingFilterPrefix;
      }
    else if(doing == DoingOutput)
      {
      this->OptionOutput = arg;
      doing = DoingNone;
      }
    else if(doing == DoingSource)
      {
      this->OptionSource = arg;
      doing = DoingNone;
      }
    else if(doing == DoingLanguage)
      {
      this->OptionLanguage = arg;
      if(this->OptionLanguage == "CXX")
        {
        this->OptionLanguage = "C++";
        }
      doing = DoingNone;
      }
    else if(doing == DoingTargetName)
      {
      this->OptionTargetName = arg;
      doing = DoingNone;
      }
    else if(doing == DoingTargetType)
      {
      this->OptionTargetType = arg;
      doing = DoingNone;
      }
    else if(doing == DoingBuildDir)
      {
      this->OptionBuildDir = arg;
      doing = DoingNone;
      }
    else if(doing == DoingFilterPrefix)
      {
      this->OptionFilterPrefix = arg;
      doing = DoingNone;
      }
    }

  // Extract the real command line.
  if(arg0)
    {
    this->RealArgC = argc - arg0;
    this->RealArgV = argv + arg0;
    for(int i=0; i < this->RealArgC; ++i)
      {
      this->HandleRealArg(this->RealArgV[i]);
      }
    return true;
    }
  else
    {
    this->RealArgC = 0;
    this->RealArgV = 0;
    std::cerr << "No launch/command separator ('--') found!\n";
    return false;
    }
}

//----------------------------------------------------------------------------
void cmCTestLaunch::HandleRealArg(const char* arg)
{
#ifdef _WIN32
  // Expand response file arguments.
  if(arg[0] == '@' && cmSystemTools::FileExists(arg+1))
    {
    cmsys::ifstream fin(arg+1);
    std::string line;
    while(cmSystemTools::GetLineFromStream(fin, line))
      {
      cmSystemTools::ParseWindowsCommandLine(line.c_str(), this->RealArgs);
      }
    return;
    }
#endif
  this->RealArgs.push_back(arg);
}

//----------------------------------------------------------------------------
void cmCTestLaunch::ComputeFileNames()
{
  // We just passthru the behavior of the real command unless the
  // CTEST_LAUNCH_LOGS environment variable is set.
  const char* d = getenv("CTEST_LAUNCH_LOGS");
  if(!(d && *d))
    {
    return;
    }
  this->Passthru = false;

  // The environment variable specifies the directory into which we
  // generate build logs.
  this->LogDir = d;
  cmSystemTools::ConvertToUnixSlashes(this->LogDir);
  this->LogDir += "/";

  // We hash the input command working dir and command line to obtain
  // a repeatable and (probably) unique name for log files.
  char hash[32];
  cmsysMD5* md5 = cmsysMD5_New();
  cmsysMD5_Initialize(md5);
  cmsysMD5_Append(md5, (unsigned char const*)(this->CWD.c_str()), -1);
  for(std::vector<std::string>::const_iterator ai = this->RealArgs.begin();
      ai != this->RealArgs.end(); ++ai)
    {
    cmsysMD5_Append(md5, (unsigned char const*)ai->c_str(), -1);
    }
  cmsysMD5_FinalizeHex(md5, hash);
  cmsysMD5_Delete(md5);
  this->LogHash.assign(hash, 32);

  // We store stdout and stderr in temporary log files.
  this->LogOut = this->LogDir;
  this->LogOut += "launch-";
  this->LogOut += this->LogHash;
  this->LogOut += "-out.txt";
  this->LogErr = this->LogDir;
  this->LogErr += "launch-";
  this->LogErr += this->LogHash;
  this->LogErr += "-err.txt";
}

//----------------------------------------------------------------------------
void cmCTestLaunch::RunChild()
{
  // Ignore noopt make rules
  if(this->RealArgs.empty() || this->RealArgs[0] == ":")
    {
    this->ExitCode = 0;
    return;
    }

  // Prepare to run the real command.
  cmsysProcess* cp = this->Process;
  cmsysProcess_SetCommand(cp, this->RealArgV);

  cmsys::ofstream fout;
  cmsys::ofstream ferr;
  if(this->Passthru)
    {
    // In passthru mode we just share the output pipes.
    cmsysProcess_SetPipeShared(cp, cmsysProcess_Pipe_STDOUT, 1);
    cmsysProcess_SetPipeShared(cp, cmsysProcess_Pipe_STDERR, 1);
    }
  else
    {
    // In full mode we record the child output pipes to log files.
    fout.open(this->LogOut.c_str(),
              std::ios::out | std::ios::binary);
    ferr.open(this->LogErr.c_str(),
              std::ios::out | std::ios::binary);
    }

#ifdef _WIN32
  // Do this so that newline transformation is not done when writing to cout
  // and cerr below.
  _setmode(fileno(stdout), _O_BINARY);
  _setmode(fileno(stderr), _O_BINARY);
#endif

  // Run the real command.
  cmsysProcess_Execute(cp);

  // Record child stdout and stderr if necessary.
  if(!this->Passthru)
    {
    char* data = 0;
    int length = 0;
    while(int p = cmsysProcess_WaitForData(cp, &data, &length, 0))
      {
      if(p == cmsysProcess_Pipe_STDOUT)
        {
        fout.write(data, length);
        std::cout.write(data, length);
        this->HaveOut = true;
        }
      else if(p == cmsysProcess_Pipe_STDERR)
        {
        ferr.write(data, length);
        std::cerr.write(data, length);
        this->HaveErr = true;
        }
      }
    }

  // Wait for the real command to finish.
  cmsysProcess_WaitForExit(cp, 0);
  this->ExitCode = cmsysProcess_GetExitValue(cp);
}

//----------------------------------------------------------------------------
int cmCTestLaunch::Run()
{
  if(!this->Process)
    {
    std::cerr << "Could not allocate cmsysProcess instance!\n";
    return -1;
    }

  this->RunChild();

  if(this->CheckResults())
    {
    return this->ExitCode;
    }

  this->LoadConfig();
  this->WriteXML();

  return this->ExitCode;
}

//----------------------------------------------------------------------------
void cmCTestLaunch::LoadLabels()
{
  if(this->OptionBuildDir.empty() || this->OptionTargetName.empty())
    {
    return;
    }

  // Labels are listed in per-target files.
  std::string fname = this->OptionBuildDir;
  fname += cmake::GetCMakeFilesDirectory();
  fname += "/";
  fname += this->OptionTargetName;
  fname += ".dir/Labels.txt";

  // We are interested in per-target labels for this source file.
  std::string source = this->OptionSource;
  cmSystemTools::ConvertToUnixSlashes(source);

  // Load the labels file.
  cmsys::ifstream fin(fname.c_str(), std::ios::in | std::ios::binary);
  if(!fin) { return; }
  bool inTarget = true;
  bool inSource = false;
  std::string line;
  while(cmSystemTools::GetLineFromStream(fin, line))
    {
    if(line.empty() || line[0] == '#')
      {
      // Ignore blank and comment lines.
      continue;
      }
    else if(line[0] == ' ')
      {
      // Label lines appear indented by one space.
      if(inTarget || inSource)
        {
        this->Labels.insert(line.c_str()+1);
        }
      }
    else if(!this->OptionSource.empty() && !inSource)
      {
      // Non-indented lines specify a source file name.  The first one
      // is the end of the target-wide labels.  Use labels following a
      // matching source.
      inTarget = false;
      inSource = this->SourceMatches(line, source);
      }
    else
      {
      return;
      }
    }
}

//----------------------------------------------------------------------------
bool cmCTestLaunch::SourceMatches(std::string const& lhs,
                                  std::string const& rhs)
{
  // TODO: Case sensitivity, UseRelativePaths, etc.  Note that both
  // paths in the comparison get generated by CMake.  This is done for
  // every source in the target, so it should be efficient (cannot use
  // cmSystemTools::IsSameFile).
  return lhs == rhs;
}

//----------------------------------------------------------------------------
bool cmCTestLaunch::IsError() const
{
  return this->ExitCode != 0;
}

//----------------------------------------------------------------------------
void cmCTestLaunch::WriteXML()
{
  // Name the xml file.
  std::string logXML = this->LogDir;
  logXML += this->IsError()? "error-" : "warning-";
  logXML += this->LogHash;
  logXML += ".xml";

  // Use cmGeneratedFileStream to atomically create the report file.
  cmGeneratedFileStream fxml(logXML.c_str());
  cmXMLWriter xml(fxml, 2);
  xml.StartElement("Failure");
  xml.Attribute("type", this->IsError() ? "Error" : "Warning");
  this->WriteXMLAction(xml);
  this->WriteXMLCommand(xml);
  this->WriteXMLResult(xml);
  this->WriteXMLLabels(xml);
  xml.EndElement(); // Failure
}

//----------------------------------------------------------------------------
void cmCTestLaunch::WriteXMLAction(cmXMLWriter& xml)
{
  xml.Comment("Meta-information about the build action");
  xml.StartElement("Action");

  // TargetName
  if(!this->OptionTargetName.empty())
    {
    xml.Element("TargetName", this->OptionTargetName);
    }

  // Language
  if(!this->OptionLanguage.empty())
    {
    xml.Element("Language", this->OptionLanguage);
    }

  // SourceFile
  if(!this->OptionSource.empty())
    {
    std::string source = this->OptionSource;
    cmSystemTools::ConvertToUnixSlashes(source);

    // If file is in source tree use its relative location.
    if(cmSystemTools::FileIsFullPath(this->SourceDir.c_str()) &&
       cmSystemTools::FileIsFullPath(source.c_str()) &&
       cmSystemTools::IsSubDirectory(source,
                                     this->SourceDir))
      {
      source = cmSystemTools::RelativePath(this->SourceDir.c_str(),
                                           source.c_str());
      }

    xml.Element("SourceFile", source);
    }

  // OutputFile
  if(!this->OptionOutput.empty())
    {
    xml.Element("OutputFile", this->OptionOutput);
    }

  // OutputType
  const char* outputType = 0;
  if(!this->OptionTargetType.empty())
    {
    if(this->OptionTargetType == "EXECUTABLE")
      {
      outputType = "executable";
      }
    else if(this->OptionTargetType == "SHARED_LIBRARY")
      {
      outputType = "shared library";
      }
    else if(this->OptionTargetType == "MODULE_LIBRARY")
      {
      outputType = "module library";
      }
    else if(this->OptionTargetType == "STATIC_LIBRARY")
      {
      outputType = "static library";
      }
    }
  else if(!this->OptionSource.empty())
    {
    outputType = "object file";
    }
  if(outputType)
    {
    xml.Element("OutputType", outputType);
    }

  xml.EndElement(); // Action
}

//----------------------------------------------------------------------------
void cmCTestLaunch::WriteXMLCommand(cmXMLWriter& xml)
{
  xml.Comment("Details of command");
  xml.StartElement("Command");
  if(!this->CWD.empty())
    {
    xml.Element("WorkingDirectory", this->CWD);
    }
  for(std::vector<std::string>::const_iterator ai = this->RealArgs.begin();
      ai != this->RealArgs.end(); ++ai)
    {
    xml.Element("Argument", *ai);
    }
  xml.EndElement(); // Command
}

//----------------------------------------------------------------------------
void cmCTestLaunch::WriteXMLResult(cmXMLWriter& xml)
{
  xml.Comment("Result of command");
  xml.StartElement("Result");

  // StdOut
  xml.StartElement("StdOut");
  this->DumpFileToXML(xml, this->LogOut);
  xml.EndElement(); // StdOut

  // StdErr
  xml.StartElement("StdErr");
  this->DumpFileToXML(xml, this->LogErr);
  xml.EndElement(); // StdErr

  // ExitCondition
  xml.StartElement("ExitCondition");
  cmsysProcess* cp = this->Process;
  switch (cmsysProcess_GetState(cp))
    {
    case cmsysProcess_State_Starting:
      xml.Content("No process has been executed"); break;
    case cmsysProcess_State_Executing:
      xml.Content("The process is still executing"); break;
    case cmsysProcess_State_Disowned:
      xml.Content("Disowned"); break;
    case cmsysProcess_State_Killed:
      xml.Content("Killed by parent"); break;

    case cmsysProcess_State_Expired:
      xml.Content("Killed when timeout expired"); break;
    case cmsysProcess_State_Exited:
      xml.Content(this->ExitCode); break;
    case cmsysProcess_State_Exception:
      xml.Content("Terminated abnormally: ");
      xml.Content(cmsysProcess_GetExceptionString(cp)); break;
    case cmsysProcess_State_Error:
      xml.Content("Error administrating child process: ");
      xml.Content(cmsysProcess_GetErrorString(cp)); break;
    };
  xml.EndElement(); // ExitCondition

  xml.EndElement(); // Result
}

//----------------------------------------------------------------------------
void cmCTestLaunch::WriteXMLLabels(cmXMLWriter& xml)
{
  this->LoadLabels();
  if(!this->Labels.empty())
    {
    xml.Comment("Interested parties");
    xml.StartElement("Labels");
    for(std::set<std::string>::const_iterator li = this->Labels.begin();
        li != this->Labels.end(); ++li)
      {
      xml.Element("Label", *li);
      }
    xml.EndElement(); // Labels
    }
}

//----------------------------------------------------------------------------
void cmCTestLaunch::DumpFileToXML(cmXMLWriter& xml,
                                  std::string const& fname)
{
  cmsys::ifstream fin(fname.c_str(), std::ios::in | std::ios::binary);

  std::string line;
  const char* sep = "";

  while(cmSystemTools::GetLineFromStream(fin, line))
    {
    if(MatchesFilterPrefix(line))
      {
      continue;
      }

    xml.Content(sep);
    xml.Content(line);
    sep = "\n";
    }
}

//----------------------------------------------------------------------------
bool cmCTestLaunch::CheckResults()
{
  // Skip XML in passthru mode.
  if(this->Passthru)
    {
    return true;
    }

  // We always report failure for error conditions.
  if(this->IsError())
    {
    return false;
    }

  // Scrape the output logs to look for warnings.
  if((this->HaveErr && this->ScrapeLog(this->LogErr)) ||
     (this->HaveOut && this->ScrapeLog(this->LogOut)))
    {
    return false;
    }
  return true;
}

//----------------------------------------------------------------------------
void cmCTestLaunch::LoadScrapeRules()
{
  if(this->ScrapeRulesLoaded)
    {
    return;
    }
  this->ScrapeRulesLoaded = true;

  // Common compiler warning formats.  These are much simpler than the
  // full log-scraping expressions because we do not need to extract
  // file and line information.
  this->RegexWarning.push_back("(^|[ :])[Ww][Aa][Rr][Nn][Ii][Nn][Gg]");
  this->RegexWarning.push_back("(^|[ :])[Rr][Ee][Mm][Aa][Rr][Kk]");
  this->RegexWarning.push_back("(^|[ :])[Nn][Oo][Tt][Ee]");

  // Load custom match rules given to us by CTest.
  this->LoadScrapeRules("Warning", this->RegexWarning);
  this->LoadScrapeRules("WarningSuppress", this->RegexWarningSuppress);
}

//----------------------------------------------------------------------------
void
cmCTestLaunch
::LoadScrapeRules(const char* purpose,
                  std::vector<cmsys::RegularExpression>& regexps)
{
  std::string fname = this->LogDir;
  fname += "Custom";
  fname += purpose;
  fname += ".txt";
  cmsys::ifstream fin(fname.c_str(), std::ios::in | std::ios::binary);
  std::string line;
  cmsys::RegularExpression rex;
  while(cmSystemTools::GetLineFromStream(fin, line))
    {
    if(rex.compile(line.c_str()))
      {
      regexps.push_back(rex);
      }
    }
}

//----------------------------------------------------------------------------
bool cmCTestLaunch::ScrapeLog(std::string const& fname)
{
  this->LoadScrapeRules();

  // Look for log file lines matching warning expressions but not
  // suppression expressions.
  cmsys::ifstream fin(fname.c_str(), std::ios::in | std::ios::binary);
  std::string line;
  while(cmSystemTools::GetLineFromStream(fin, line))
    {
    if(MatchesFilterPrefix(line))
      {
      continue;
      }

    if(this->Match(line, this->RegexWarning) &&
       !this->Match(line, this->RegexWarningSuppress))
      {
      return true;
      }
    }
  return false;
}

//----------------------------------------------------------------------------
bool cmCTestLaunch::Match(std::string const& line,
                          std::vector<cmsys::RegularExpression>& regexps)
{
  for(std::vector<cmsys::RegularExpression>::iterator ri = regexps.begin();
      ri != regexps.end(); ++ri)
    {
    if(ri->find(line.c_str()))
      {
      return true;
      }
    }
  return false;
}

//----------------------------------------------------------------------------
bool cmCTestLaunch::MatchesFilterPrefix(std::string const& line) const
{
  if(!this->OptionFilterPrefix.empty() && cmSystemTools::StringStartsWith(
      line.c_str(), this->OptionFilterPrefix.c_str()))
    {
    return true;
    }
  return false;
}

//----------------------------------------------------------------------------
int cmCTestLaunch::Main(int argc, const char* const argv[])
{
  if(argc == 2)
    {
    std::cerr << "ctest --launch: this mode is for internal CTest use only"
              << std::endl;
    return 1;
    }
  cmCTestLaunch self(argc, argv);
  return self.Run();
}

//----------------------------------------------------------------------------
#include "cmGlobalGenerator.h"
#include "cmLocalGenerator.h"
#include "cmMakefile.h"
#include "cmake.h"
#include <cmsys/auto_ptr.hxx>
void cmCTestLaunch::LoadConfig()
{
  cmake cm;
  cm.SetHomeDirectory("");
  cm.SetHomeOutputDirectory("");
  cmGlobalGenerator gg(&cm);
  cmsys::auto_ptr<cmLocalGenerator> lg(gg.MakeLocalGenerator());
  cmMakefile* mf = lg->GetMakefile();
  std::string fname = this->LogDir;
  fname += "CTestLaunchConfig.cmake";
  if(cmSystemTools::FileExists(fname.c_str()) &&
     mf->ReadListFile(fname.c_str()))
    {
    this->SourceDir = mf->GetSafeDefinition("CTEST_SOURCE_DIRECTORY");
    cmSystemTools::ConvertToUnixSlashes(this->SourceDir);
    }
}
