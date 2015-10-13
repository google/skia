/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmCTestLaunch_h
#define cmCTestLaunch_h

#include "cmStandardIncludes.h"
#include <cmsys/RegularExpression.hxx>

class cmXMLWriter;

/** \class cmCTestLaunch
 * \brief Launcher for make rules to report results for ctest
 *
 * This implements the 'ctest --launch' tool.
 */
class cmCTestLaunch
{
public:
  /** Entry point from ctest executable main().  */
  static int Main(int argc, const char* const argv[]);
private:
  // Initialize the launcher from its command line.
  cmCTestLaunch(int argc, const char* const* argv);
  ~cmCTestLaunch();

  // Run the real command.
  int Run();
  void RunChild();

  // Methods to check the result of the real command.
  bool IsError() const;
  bool CheckResults();

  // Launcher options specified before the real command.
  std::string OptionOutput;
  std::string OptionSource;
  std::string OptionLanguage;
  std::string OptionTargetName;
  std::string OptionTargetType;
  std::string OptionBuildDir;
  std::string OptionFilterPrefix;
  bool ParseArguments(int argc, const char* const* argv);

  // The real command line appearing after launcher arguments.
  int RealArgC;
  const char* const* RealArgV;
  std::string CWD;

  // The real command line after response file expansion.
  std::vector<std::string> RealArgs;
  void HandleRealArg(const char* arg);

  // A hash of the real command line is unique and unlikely to collide.
  std::string LogHash;
  void ComputeFileNames();

  bool Passthru;
  struct cmsysProcess_s* Process;
  int ExitCode;

  // Temporary log files for stdout and stderr of real command.
  std::string LogDir;
  std::string LogOut;
  std::string LogErr;
  bool HaveOut;
  bool HaveErr;

  // Labels associated with the build rule.
  std::set<std::string> Labels;
  void LoadLabels();
  bool SourceMatches(std::string const& lhs,
                     std::string const& rhs);

  // Regular expressions to match warnings and their exceptions.
  bool ScrapeRulesLoaded;
  std::vector<cmsys::RegularExpression> RegexWarning;
  std::vector<cmsys::RegularExpression> RegexWarningSuppress;
  void LoadScrapeRules();
  void LoadScrapeRules(const char* purpose,
                       std::vector<cmsys::RegularExpression>& regexps);
  bool ScrapeLog(std::string const& fname);
  bool Match(std::string const& line,
             std::vector<cmsys::RegularExpression>& regexps);
  bool MatchesFilterPrefix(std::string const& line) const;

  // Methods to generate the xml fragment.
  void WriteXML();
  void WriteXMLAction(cmXMLWriter& xml);
  void WriteXMLCommand(cmXMLWriter& xml);
  void WriteXMLResult(cmXMLWriter& xml);
  void WriteXMLLabels(cmXMLWriter& xml);
  void DumpFileToXML(cmXMLWriter& xml, std::string const& fname);

  // Configuration
  void LoadConfig();
  std::string SourceDir;
};

#endif
