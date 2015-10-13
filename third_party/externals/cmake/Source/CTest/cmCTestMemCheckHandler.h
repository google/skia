/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc.

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/

#ifndef cmCTestMemCheckHandler_h
#define cmCTestMemCheckHandler_h


#include "cmCTestTestHandler.h"
#include "cmStandardIncludes.h"
#include "cmListFileCache.h"
#include <vector>
#include <string>

class cmMakefile;
class cmXMLWriter;

/** \class cmCTestMemCheckHandler
 * \brief A class that handles ctest -S invocations
 *
 */
class cmCTestMemCheckHandler : public cmCTestTestHandler
{
  friend class cmCTestRunTest;
public:
  cmTypeMacro(cmCTestMemCheckHandler, cmCTestTestHandler);

  void PopulateCustomVectors(cmMakefile *mf);

  cmCTestMemCheckHandler();

  void Initialize();
protected:
  virtual int PreProcessHandler();
  virtual int PostProcessHandler();
  virtual void GenerateTestCommand(std::vector<std::string>& args, int test);

private:

  enum { // Memory checkers
    UNKNOWN = 0,
    VALGRIND,
    PURIFY,
    BOUNDS_CHECKER,
    // checkers after here do not use the standard error list
    ADDRESS_SANITIZER,
    THREAD_SANITIZER,
    MEMORY_SANITIZER,
    UB_SANITIZER
  };
public:
  enum { // Memory faults
    ABR = 0,
    ABW,
    ABWL,
    COR,
    EXU,
    FFM,
    FIM,
    FMM,
    FMR,
    FMW,
    FUM,
    IPR,
    IPW,
    MAF,
    MLK,
    MPK,
    NPR,
    ODS,
    PAR,
    PLK,
    UMC,
    UMR,
    NO_MEMORY_FAULT
  };
private:
  enum { // Program statuses
    NOT_RUN = 0,
    TIMEOUT,
    SEGFAULT,
    ILLEGAL,
    INTERRUPT,
    NUMERICAL,
    OTHER_FAULT,
    FAILED,
    BAD_COMMAND,
    COMPLETED
  };
  std::string              BoundsCheckerDPBDFile;
  std::string              BoundsCheckerXMLFile;
  std::string              MemoryTester;
  std::vector<std::string> MemoryTesterDynamicOptions;
  std::vector<std::string> MemoryTesterOptions;
  int                      MemoryTesterStyle;
  std::string              MemoryTesterOutputFile;
  std::string              MemoryTesterEnvironmentVariable;
  // these are used to store the types of errors that can show up
  std::vector<std::string> ResultStrings;
  std::vector<std::string> ResultStringsLong;
  std::vector<int>         GlobalResults;
  bool                     LogWithPID; // does log file add pid

  std::vector<int>::size_type FindOrAddWarning(const std::string& warning);
  // initialize the ResultStrings and ResultStringsLong for
  // this type of checker
  void InitializeResultsVectors();

  ///! Initialize memory checking subsystem.
  bool InitializeMemoryChecking();

  /**
   * Generate the Dart compatible output
   */
  void GenerateDartOutput(cmXMLWriter& xml);

  std::vector<std::string> CustomPreMemCheck;
  std::vector<std::string> CustomPostMemCheck;

  //! Parse Valgrind/Purify/Bounds Checker result out of the output
  //string. After running, log holds the output and results hold the
  //different memmory errors.
  bool ProcessMemCheckOutput(const std::string& str,
                             std::string& log, std::vector<int>& results);
  bool ProcessMemCheckValgrindOutput(const std::string& str,
                                     std::string& log,
                                     std::vector<int>& results);
  bool ProcessMemCheckPurifyOutput(const std::string& str,
                                   std::string& log,
                                   std::vector<int>& results);
  bool ProcessMemCheckSanitizerOutput(const std::string& str,
                                      std::string& log,
                                      std::vector<int>& results);
  bool ProcessMemCheckBoundsCheckerOutput(const std::string& str,
                                          std::string& log,
                                          std::vector<int>& results);

  void PostProcessTest(cmCTestTestResult& res, int test);
  void PostProcessBoundsCheckerTest(cmCTestTestResult& res, int test);

  ///! append MemoryTesterOutputFile to the test log
  void AppendMemTesterOutput(cmCTestTestHandler::cmCTestTestResult& res,
                             std::string const& filename);

  ///! generate the output filename for the given test index
  void TestOutputFileNames(int test, std::vector<std::string>& files);
};

#endif

