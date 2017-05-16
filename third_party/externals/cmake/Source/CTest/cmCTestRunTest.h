/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmCTestRunTest_h
#define cmCTestRunTest_h

#include <cmStandardIncludes.h>
#include <cmCTestTestHandler.h>
#include <cmProcess.h>

/** \class cmRunTest
 * \brief represents a single test to be run
 *
 * cmRunTest contains the information related to running a single test
 */
class cmCTestRunTest
{
public:
  cmCTestRunTest(cmCTestTestHandler* handler);
  ~cmCTestRunTest();

  void SetNumberOfRuns(int n) {this->NumberOfRunsLeft = n;}
  void SetRunUntilFailOn() { this->RunUntilFail = true;}
  void SetTestProperties(cmCTestTestHandler::cmCTestTestProperties * prop)
  { this->TestProperties = prop; }

  cmCTestTestHandler::cmCTestTestProperties * GetTestProperties()
  { return this->TestProperties; }

  void SetIndex(int i) { this->Index = i; }

  int GetIndex() { return this->Index; }

  std::string GetProcessOutput() { return this->ProcessOutput; }

  bool IsStopTimePassed() { return this->StopTimePassed; }

  cmCTestTestHandler::cmCTestTestResult GetTestResults()
  { return this->TestResult; }

  // Read and store output.  Returns true if it must be called again.
  bool CheckOutput();

  // Compresses the output, writing to CompressedOutput
  void CompressOutput();

  //launch the test process, return whether it started correctly
  bool StartTest(size_t total);
  //capture and report the test results
  bool EndTest(size_t completed, size_t total, bool started);
  //Called by ctest -N to log the command string
  void ComputeArguments();

  void ComputeWeightedCost();

  bool StartAgain();
private:
  bool NeedsToRerun();
  void DartProcessing();
  void ExeNotFound(std::string exe);
  // Figures out a final timeout which is min(STOP_TIME, NOW+TIMEOUT)
  double ResolveTimeout();
  bool ForkProcess(double testTimeOut, bool explicitTimeout,
                   std::vector<std::string>* environment);
  void WriteLogOutputTop(size_t completed, size_t total);
  //Run post processing of the process output for MemCheck
  void MemCheckPostProcess();

  cmCTestTestHandler::cmCTestTestProperties * TestProperties;
  //Pointer back to the "parent"; the handler that invoked this test run
  cmCTestTestHandler * TestHandler;
  cmCTest * CTest;
  cmProcess * TestProcess;
  //If the executable to run is ctest, don't create a new process;
  //just instantiate a new cmTest.  (Can be disabled for a single test
  //if this option is set to false.)
  //bool OptimizeForCTest;

  bool UsePrefixCommand;
  std::string PrefixCommand;

  std::string ProcessOutput;
  std::string CompressedOutput;
  double CompressionRatio;
  //The test results
  cmCTestTestHandler::cmCTestTestResult TestResult;
  int Index;
  std::string StartTime;
  std::string ActualCommand;
  std::vector<std::string> Arguments;
  bool StopTimePassed;
  bool RunUntilFail;
  int NumberOfRunsLeft;
  bool RunAgain;
  size_t TotalNumberOfTests;
};

inline int getNumWidth(size_t n)
{
  int numWidth = 1;
  if(n >= 10)
    {
    numWidth = 2;
    }
  if(n >= 100)
    {
    numWidth = 3;
    }
  return numWidth;
}

#endif

