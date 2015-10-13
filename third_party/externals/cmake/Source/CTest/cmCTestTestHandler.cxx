/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/

#include "cmCTestTestHandler.h"
#include "cmCTestMultiProcessHandler.h"
#include "cmCTestBatchTestHandler.h"
#include "cmCTest.h"
#include "cmCTestRunTest.h"
#include "cmake.h"
#include "cmGeneratedFileStream.h"
#include <cmsys/Process.h>
#include <cmsys/RegularExpression.hxx>
#include <cmsys/Base64.h>
#include <cmsys/Directory.hxx>
#include <cmsys/FStream.hxx>
#include "cmMakefile.h"
#include "cmGlobalGenerator.h"
#include "cmLocalGenerator.h"
#include "cmCommand.h"
#include "cmSystemTools.h"
#include "cmXMLWriter.h"
#include "cm_utf8.h"

#include <stdlib.h>
#include <math.h>
#include <float.h>

#include <set>

//----------------------------------------------------------------------
class cmCTestSubdirCommand : public cmCommand
{
public:
  /**
   * This is a virtual constructor for the command.
   */
  virtual cmCommand* Clone()
    {
    cmCTestSubdirCommand* c = new cmCTestSubdirCommand;
    c->TestHandler = this->TestHandler;
    return c;
    }

  /**
   * This is called when the command is first encountered in
   * the CMakeLists.txt file.
   */
  virtual bool InitialPass(std::vector<std::string> const& args,
                           cmExecutionStatus &);

  /**
   * The name of the command as specified in CMakeList.txt.
   */
  virtual std::string GetName() const { return "subdirs";}

  cmTypeMacro(cmCTestSubdirCommand, cmCommand);

  cmCTestTestHandler* TestHandler;
};

//----------------------------------------------------------------------
bool cmCTestSubdirCommand
::InitialPass(std::vector<std::string> const& args, cmExecutionStatus &)
{
  if(args.size() < 1 )
    {
    this->SetError("called with incorrect number of arguments");
    return false;
    }
  std::vector<std::string>::const_iterator it;
  std::string cwd = cmSystemTools::GetCurrentWorkingDirectory();
  for ( it = args.begin(); it != args.end(); ++ it )
    {
    std::string fname;

    if(cmSystemTools::FileIsFullPath(it->c_str()))
      {
      fname = *it;
      }
    else
      {
      fname = cwd;
      fname += "/";
      fname += *it;
      }

    if ( !cmSystemTools::FileIsDirectory(fname) )
      {
      // No subdirectory? So what...
      continue;
      }
    cmSystemTools::ChangeDirectory(fname);
    const char* testFilename;
    if( cmSystemTools::FileExists("CTestTestfile.cmake") )
      {
      // does the CTestTestfile.cmake exist ?
      testFilename = "CTestTestfile.cmake";
      }
    else if( cmSystemTools::FileExists("DartTestfile.txt") )
      {
      // does the DartTestfile.txt exist ?
      testFilename = "DartTestfile.txt";
      }
    else
      {
      // No CTestTestfile? Who cares...
      continue;
      }
    fname += "/";
    fname += testFilename;
    bool readit = this->Makefile->ReadDependentFile(fname.c_str());
    cmSystemTools::ChangeDirectory(cwd);
    if(!readit)
      {
      std::string m = "Could not find include file: ";
      m += fname;
      this->SetError(m);
      return false;
      }
    }
  cmSystemTools::ChangeDirectory(cwd);
  return true;
}

//----------------------------------------------------------------------
class cmCTestAddSubdirectoryCommand : public cmCommand
{
public:
  /**
   * This is a virtual constructor for the command.
   */
  virtual cmCommand* Clone()
    {
    cmCTestAddSubdirectoryCommand* c = new cmCTestAddSubdirectoryCommand;
    c->TestHandler = this->TestHandler;
    return c;
    }

  /**
   * This is called when the command is first encountered in
   * the CMakeLists.txt file.
   */
  virtual bool InitialPass(std::vector<std::string> const& args,
                           cmExecutionStatus &);

  /**
   * The name of the command as specified in CMakeList.txt.
   */
  virtual std::string GetName() const { return "add_subdirectory";}

  cmTypeMacro(cmCTestAddSubdirectoryCommand, cmCommand);

  cmCTestTestHandler* TestHandler;
};

//----------------------------------------------------------------------
bool cmCTestAddSubdirectoryCommand
::InitialPass(std::vector<std::string> const& args, cmExecutionStatus &)
{
  if(args.size() < 1 )
    {
    this->SetError("called with incorrect number of arguments");
    return false;
    }

  std::string cwd = cmSystemTools::GetCurrentWorkingDirectory();
  cmSystemTools::ChangeDirectory(cwd);
  std::string fname = cwd;
  fname += "/";
  fname += args[0];

  if ( !cmSystemTools::FileExists(fname.c_str()) )
    {
    // No subdirectory? So what...
    return true;
    }
  cmSystemTools::ChangeDirectory(fname);
  const char* testFilename;
  if( cmSystemTools::FileExists("CTestTestfile.cmake") )
    {
    // does the CTestTestfile.cmake exist ?
    testFilename = "CTestTestfile.cmake";
    }
  else if( cmSystemTools::FileExists("DartTestfile.txt") )
    {
    // does the DartTestfile.txt exist ?
    testFilename = "DartTestfile.txt";
    }
  else
    {
    // No CTestTestfile? Who cares...
    cmSystemTools::ChangeDirectory(cwd);
    return true;
    }
  fname += "/";
  fname += testFilename;
  bool readit = this->Makefile->ReadDependentFile(fname.c_str());
  cmSystemTools::ChangeDirectory(cwd);
  if(!readit)
    {
    std::string m = "Could not find include file: ";
    m += fname;
    this->SetError(m);
    return false;
    }
  return true;
}

//----------------------------------------------------------------------
class cmCTestAddTestCommand : public cmCommand
{
public:
  /**
   * This is a virtual constructor for the command.
   */
  virtual cmCommand* Clone()
    {
    cmCTestAddTestCommand* c = new cmCTestAddTestCommand;
    c->TestHandler = this->TestHandler;
    return c;
    }

  /**
   * This is called when the command is first encountered in
   * the CMakeLists.txt file.
   */
  virtual bool InitialPass(std::vector<std::string> const&,
                           cmExecutionStatus &);

  /**
   * The name of the command as specified in CMakeList.txt.
   */
  virtual std::string GetName() const { return "add_test";}

  cmTypeMacro(cmCTestAddTestCommand, cmCommand);

  cmCTestTestHandler* TestHandler;
};

//----------------------------------------------------------------------
bool cmCTestAddTestCommand
::InitialPass(std::vector<std::string> const& args, cmExecutionStatus &)
{
  if ( args.size() < 2 )
    {
    this->SetError("called with incorrect number of arguments");
    return false;
    }
  return this->TestHandler->AddTest(args);
}

//----------------------------------------------------------------------
class cmCTestSetTestsPropertiesCommand : public cmCommand
{
public:
  /**
   * This is a virtual constructor for the command.
   */
  virtual cmCommand* Clone()
    {
    cmCTestSetTestsPropertiesCommand* c
      = new cmCTestSetTestsPropertiesCommand;
    c->TestHandler = this->TestHandler;
    return c;
    }

  /**
   * This is called when the command is first encountered in
   * the CMakeLists.txt file.
  */
  virtual bool InitialPass(std::vector<std::string> const&,
                           cmExecutionStatus &);

  /**
   * The name of the command as specified in CMakeList.txt.
   */
  virtual std::string GetName() const { return "set_tests_properties";}

  cmTypeMacro(cmCTestSetTestsPropertiesCommand, cmCommand);

  cmCTestTestHandler* TestHandler;
};

//----------------------------------------------------------------------
bool cmCTestSetTestsPropertiesCommand
::InitialPass(std::vector<std::string> const& args, cmExecutionStatus &)
{
  return this->TestHandler->SetTestsProperties(args);
}

//----------------------------------------------------------------------
// get the next number in a string with numbers separated by ,
// pos is the start of the search and pos2 is the end of the search
// pos becomes pos2 after a call to GetNextNumber.
// -1 is returned at the end of the list.
inline int GetNextNumber(std::string const& in,
                         int& val,
                         std::string::size_type& pos,
                         std::string::size_type& pos2)
{
  pos2 = in.find(',', pos);
  if(pos2 != in.npos)
    {
    if(pos2-pos == 0)
      {
      val = -1;
      }
    else
      {
      val = atoi(in.substr(pos, pos2-pos).c_str());
      }
    pos = pos2+1;
    return 1;
    }
  else
    {
    if(in.size()-pos == 0)
      {
       val = -1;
      }
    else
      {
      val = atoi(in.substr(pos, in.size()-pos).c_str());
      }
    return 0;
    }
}

//----------------------------------------------------------------------
// get the next number in a string with numbers separated by ,
// pos is the start of the search and pos2 is the end of the search
// pos becomes pos2 after a call to GetNextNumber.
// -1 is returned at the end of the list.
inline int GetNextRealNumber(std::string const& in,
                             double& val,
                             std::string::size_type& pos,
                             std::string::size_type& pos2)
{
  pos2 = in.find(',', pos);
  if(pos2 != in.npos)
    {
    if(pos2-pos == 0)
      {
      val = -1;
      }
    else
      {
      val = atof(in.substr(pos, pos2-pos).c_str());
      }
    pos = pos2+1;
    return 1;
    }
  else
    {
    if(in.size()-pos == 0)
      {
       val = -1;
      }
    else
      {
      val = atof(in.substr(pos, in.size()-pos).c_str());
      }
    return 0;
    }
}


//----------------------------------------------------------------------
cmCTestTestHandler::cmCTestTestHandler()
{
  this->UseUnion = false;

  this->UseIncludeLabelRegExpFlag   = false;
  this->UseExcludeLabelRegExpFlag   = false;
  this->UseIncludeRegExpFlag   = false;
  this->UseExcludeRegExpFlag   = false;
  this->UseExcludeRegExpFirst  = false;

  this->CustomMaximumPassedTestOutputSize = 1 * 1024;
  this->CustomMaximumFailedTestOutputSize = 300 * 1024;

  this->MemCheck = false;

  this->LogFile = 0;

  // regex to detect <DartMeasurement>...</DartMeasurement>
  this->DartStuff.compile(
    "(<DartMeasurement.*/DartMeasurement[a-zA-Z]*>)");
  // regex to detect each individual <DartMeasurement>...</DartMeasurement>
  this->DartStuff1.compile(
    "(<DartMeasurement[^<]*</DartMeasurement[a-zA-Z]*>)");
}

//----------------------------------------------------------------------
void cmCTestTestHandler::Initialize()
{
  this->Superclass::Initialize();

  this->ElapsedTestingTime = -1;

  this->TestResults.clear();

  this->CustomTestsIgnore.clear();
  this->StartTest = "";
  this->EndTest = "";

  this->CustomPreTest.clear();
  this->CustomPostTest.clear();
  this->CustomMaximumPassedTestOutputSize = 1 * 1024;
  this->CustomMaximumFailedTestOutputSize = 300 * 1024;

  this->TestsToRun.clear();

  this->UseIncludeLabelRegExpFlag = false;
  this->UseExcludeLabelRegExpFlag = false;
  this->UseIncludeRegExpFlag = false;
  this->UseExcludeRegExpFlag = false;
  this->UseExcludeRegExpFirst = false;
  this->IncludeLabelRegularExpression = "";
  this->ExcludeLabelRegularExpression = "";
  this->IncludeRegExp = "";
  this->ExcludeRegExp = "";

  TestsToRunString = "";
  this->UseUnion = false;
  this->TestList.clear();
}

//----------------------------------------------------------------------
void cmCTestTestHandler::PopulateCustomVectors(cmMakefile *mf)
{
  this->CTest->PopulateCustomVector(mf, "CTEST_CUSTOM_PRE_TEST",
                                this->CustomPreTest);
  this->CTest->PopulateCustomVector(mf, "CTEST_CUSTOM_POST_TEST",
                                this->CustomPostTest);
  this->CTest->PopulateCustomVector(mf,
                             "CTEST_CUSTOM_TESTS_IGNORE",
                             this->CustomTestsIgnore);
  this->CTest->PopulateCustomInteger(mf,
                             "CTEST_CUSTOM_MAXIMUM_PASSED_TEST_OUTPUT_SIZE",
                             this->CustomMaximumPassedTestOutputSize);
  this->CTest->PopulateCustomInteger(mf,
                             "CTEST_CUSTOM_MAXIMUM_FAILED_TEST_OUTPUT_SIZE",
                             this->CustomMaximumFailedTestOutputSize);
}

//----------------------------------------------------------------------
int cmCTestTestHandler::PreProcessHandler()
{
  if ( !this->ExecuteCommands(this->CustomPreTest) )
    {
    cmCTestLog(this->CTest, ERROR_MESSAGE,
      "Problem executing pre-test command(s)." << std::endl);
    return 0;
    }
  return 1;
}

//----------------------------------------------------------------------
int cmCTestTestHandler::PostProcessHandler()
{
  if ( !this->ExecuteCommands(this->CustomPostTest) )
    {
    cmCTestLog(this->CTest, ERROR_MESSAGE,
      "Problem executing post-test command(s)." << std::endl);
    return 0;
    }
  return 1;
}

//----------------------------------------------------------------------
//clearly it would be nice if this were broken up into a few smaller
//functions and commented...
int cmCTestTestHandler::ProcessHandler()
{
  // Update internal data structure from generic one
  this->SetTestsToRunInformation(this->GetOption("TestsToRunInformation"));
  this->SetUseUnion(cmSystemTools::IsOn(this->GetOption("UseUnion")));
  if(cmSystemTools::IsOn(this->GetOption("ScheduleRandom")))
    {
    this->CTest->SetScheduleType("Random");
    }
  if(this->GetOption("ParallelLevel"))
    {
    this->CTest->SetParallelLevel(atoi(this->GetOption("ParallelLevel")));
    }

  const char* val;
  val = this->GetOption("LabelRegularExpression");
  if ( val )
    {
    this->UseIncludeLabelRegExpFlag = true;
    this->IncludeLabelRegExp = val;
    }
  val = this->GetOption("ExcludeLabelRegularExpression");
  if ( val )
    {
    this->UseExcludeLabelRegExpFlag = true;
    this->ExcludeLabelRegExp = val;
    }
  val = this->GetOption("IncludeRegularExpression");
  if ( val )
    {
    this->UseIncludeRegExp();
    this->SetIncludeRegExp(val);
    }
  val = this->GetOption("ExcludeRegularExpression");
  if ( val )
    {
    this->UseExcludeRegExp();
    this->SetExcludeRegExp(val);
    }
  this->SetRerunFailed(cmSystemTools::IsOn(this->GetOption("RerunFailed")));

  this->TestResults.clear();

  cmCTestOptionalLog(this->CTest, HANDLER_OUTPUT,
             (this->MemCheck ? "Memory check" : "Test")
             << " project " << cmSystemTools::GetCurrentWorkingDirectory()
             << std::endl, this->Quiet);
  if ( ! this->PreProcessHandler() )
    {
    return -1;
    }

  cmGeneratedFileStream mLogFile;
  this->StartLogFile((this->MemCheck ? "DynamicAnalysis" : "Test"), mLogFile);
  this->LogFile = &mLogFile;

  std::vector<std::string> passed;
  std::vector<std::string> failed;
  int total;

  //start the real time clock
  double clock_start, clock_finish;
  clock_start = cmSystemTools::GetTime();

  this->ProcessDirectory(passed, failed);

  clock_finish = cmSystemTools::GetTime();

  total = int(passed.size()) + int(failed.size());

  if (total == 0)
    {
    if ( !this->CTest->GetShowOnly() && !this->CTest->ShouldPrintLabels() )
      {
      cmCTestLog(this->CTest, ERROR_MESSAGE, "No tests were found!!!"
        << std::endl);
      }
    }
  else
    {
    if (this->HandlerVerbose && !passed.empty() &&
      (this->UseIncludeRegExpFlag || this->UseExcludeRegExpFlag))
      {
      cmCTestOptionalLog(this->CTest, HANDLER_VERBOSE_OUTPUT, std::endl
        << "The following tests passed:" << std::endl, this->Quiet);
      for(std::vector<std::string>::iterator j = passed.begin();
          j != passed.end(); ++j)
        {
        cmCTestOptionalLog(this->CTest, HANDLER_VERBOSE_OUTPUT, "\t" << *j
          << std::endl, this->Quiet);
        }
      }

    float percent = float(passed.size()) * 100.0f / float(total);
    if (!failed.empty() && percent > 99)
      {
      percent = 99;
      }

    cmCTestLog(this->CTest, HANDLER_OUTPUT, std::endl
               << static_cast<int>(percent + .5) << "% tests passed, "
               << failed.size() << " tests failed out of "
               << total << std::endl);
    if(this->CTest->GetLabelSummary())
      {
      this->PrintLabelSummary();
      }
    char realBuf[1024];
    sprintf(realBuf, "%6.2f sec", (double)(clock_finish - clock_start));
    cmCTestOptionalLog(this->CTest, HANDLER_OUTPUT,
      "\nTotal Test time (real) = " << realBuf << "\n", this->Quiet );

    if (!failed.empty())
      {
      cmGeneratedFileStream ofs;
      cmCTestLog(this->CTest, HANDLER_OUTPUT, std::endl
                 << "The following tests FAILED:" << std::endl);
      this->StartLogFile("TestsFailed", ofs);

      typedef std::set<cmCTestTestHandler::cmCTestTestResult,
                       cmCTestTestResultLess> SetOfTests;
      SetOfTests resultsSet(this->TestResults.begin(),
                            this->TestResults.end());

      for(SetOfTests::iterator ftit = resultsSet.begin();
          ftit != resultsSet.end(); ++ftit)
        {
        if ( ftit->Status != cmCTestTestHandler::COMPLETED )
          {
          ofs << ftit->TestCount << ":" << ftit->Name << std::endl;
          cmCTestOptionalLog(this->CTest, HANDLER_OUTPUT, "\t" << std::setw(3)
                     << ftit->TestCount << " - "
                     << ftit->Name << " ("
                     << this->GetTestStatus(ftit->Status) << ")"
                     << std::endl, this->Quiet);
          }
        }
      }
    }

  if ( this->CTest->GetProduceXML() )
    {
    cmGeneratedFileStream xmlfile;
    if( !this->StartResultingXML(
          (this->MemCheck ? cmCTest::PartMemCheck : cmCTest::PartTest),
        (this->MemCheck ? "DynamicAnalysis" : "Test"), xmlfile) )
      {
      cmCTestLog(this->CTest, ERROR_MESSAGE, "Cannot create "
        << (this->MemCheck ? "memory check" : "testing")
        << " XML file" << std::endl);
      this->LogFile = 0;
      return 1;
      }
    cmXMLWriter xml(xmlfile);
    this->GenerateDartOutput(xml);
    }

  if ( ! this->PostProcessHandler() )
    {
    this->LogFile = 0;
    return -1;
    }

  if ( !failed.empty() )
    {
    this->LogFile = 0;
    return -1;
    }
  this->LogFile = 0;
  return 0;
}

//----------------------------------------------------------------------
void cmCTestTestHandler::PrintLabelSummary()
{
  cmCTestTestHandler::ListOfTests::iterator it = this->TestList.begin();
  cmCTestTestHandler::TestResultsVector::iterator ri =
    this->TestResults.begin();
  std::map<std::string, double> labelTimes;
  std::set<std::string> labels;
  // initialize maps
  std::string::size_type maxlen = 0;
  for(; it != this->TestList.end(); ++it)
    {
    cmCTestTestProperties& p = *it;
    if(!p.Labels.empty())
      {
      for(std::vector<std::string>::iterator l = p.Labels.begin();
          l !=  p.Labels.end(); ++l)
        {
        if((*l).size() > maxlen)
          {
          maxlen = (*l).size();
          }
        labels.insert(*l);
        labelTimes[*l] = 0;
        }
      }
    }
  ri = this->TestResults.begin();
  // fill maps
  for(; ri != this->TestResults.end(); ++ri)
    {
    cmCTestTestResult &result = *ri;
    cmCTestTestProperties& p = *result.Properties;
    if(!p.Labels.empty())
      {
      for(std::vector<std::string>::iterator l = p.Labels.begin();
          l !=  p.Labels.end(); ++l)
        {
        labelTimes[*l] += result.ExecutionTime;
        }
      }
    }
  // now print times
  if(!labels.empty())
    {
    cmCTestOptionalLog(this->CTest, HANDLER_OUTPUT, "\nLabel Time Summary:",
      this->Quiet);
    }
  for(std::set<std::string>::const_iterator i = labels.begin();
      i != labels.end(); ++i)
    {
    std::string label = *i;
    label.resize(maxlen +3, ' ');
    char buf[1024];
    sprintf(buf, "%6.2f sec", labelTimes[*i]);
    cmCTestOptionalLog(this->CTest, HANDLER_OUTPUT, "\n"
               << label << " = " << buf, this->Quiet );
    if ( this->LogFile )
      {
      *this->LogFile << "\n" << *i << " = "
                     << buf << "\n";
      }
    }
  if(!labels.empty())
    {
    if(this->LogFile)
      {
      *this->LogFile << "\n";
      }
    cmCTestOptionalLog(this->CTest, HANDLER_OUTPUT, "\n", this->Quiet);
    }

}

//----------------------------------------------------------------------
void cmCTestTestHandler::CheckLabelFilterInclude(cmCTestTestProperties& it)
{
  // if not using Labels to filter then return
  if (!this->UseIncludeLabelRegExpFlag )
    {
    return;
    }
  // if there are no labels and we are filtering by labels
  // then exclude the test as it does not have the label
  if(it.Labels.empty())
    {
    it.IsInBasedOnREOptions = false;
    return;
    }
  // check to see if the label regular expression matches
  bool found = false;  // assume it does not match
  // loop over all labels and look for match
  for(std::vector<std::string>::iterator l = it.Labels.begin();
      l !=  it.Labels.end(); ++l)
    {
    if(this->IncludeLabelRegularExpression.find(*l))
      {
      found = true;
      }
    }
  // if no match was found, exclude the test
  if(!found)
    {
    it.IsInBasedOnREOptions = false;
    }
}


//----------------------------------------------------------------------
void cmCTestTestHandler::CheckLabelFilterExclude(cmCTestTestProperties& it)
{
  // if not using Labels to filter then return
  if (!this->UseExcludeLabelRegExpFlag )
    {
    return;
    }
  // if there are no labels and we are excluding by labels
  // then do nothing as a no label can not be a match
  if(it.Labels.empty())
    {
    return;
    }
  // check to see if the label regular expression matches
  bool found = false;  // assume it does not match
  // loop over all labels and look for match
  for(std::vector<std::string>::iterator l = it.Labels.begin();
      l !=  it.Labels.end(); ++l)
    {
    if(this->ExcludeLabelRegularExpression.find(*l))
      {
      found = true;
      }
    }
  // if match was found, exclude the test
  if(found)
    {
    it.IsInBasedOnREOptions = false;
    }
}

//----------------------------------------------------------------------
void cmCTestTestHandler::CheckLabelFilter(cmCTestTestProperties& it)
{
  this->CheckLabelFilterInclude(it);
  this->CheckLabelFilterExclude(it);
}

//----------------------------------------------------------------------
void cmCTestTestHandler::ComputeTestList()
{
  this->TestList.clear(); // clear list of test
  this->GetListOfTests();

  if (this->RerunFailed)
    {
    this->ComputeTestListForRerunFailed();
    return;
    }

  cmCTestTestHandler::ListOfTests::size_type tmsize = this->TestList.size();
  // how many tests are in based on RegExp?
  int inREcnt = 0;
  cmCTestTestHandler::ListOfTests::iterator it;
  for ( it = this->TestList.begin(); it != this->TestList.end(); it ++ )
    {
    this->CheckLabelFilter(*it);
    if (it->IsInBasedOnREOptions)
      {
      inREcnt ++;
      }
    }
  // expand the test list based on the union flag
  if (this->UseUnion)
    {
    this->ExpandTestsToRunInformation((int)tmsize);
    }
  else
    {
    this->ExpandTestsToRunInformation(inREcnt);
    }
  // Now create a final list of tests to run
  int cnt = 0;
  inREcnt = 0;
  std::string last_directory = "";
  ListOfTests finalList;
  for ( it = this->TestList.begin(); it != this->TestList.end(); it ++ )
    {
    cnt ++;
    if (it->IsInBasedOnREOptions)
      {
      inREcnt++;
      }

    if (this->UseUnion)
      {
      // if it is not in the list and not in the regexp then skip
      if ((!this->TestsToRun.empty() &&
           std::find(this->TestsToRun.begin(), this->TestsToRun.end(), cnt)
           == this->TestsToRun.end()) && !it->IsInBasedOnREOptions)
        {
        continue;
        }
      }
    else
      {
      // is this test in the list of tests to run? If not then skip it
      if ((!this->TestsToRun.empty() &&
           std::find(this->TestsToRun.begin(),
                     this->TestsToRun.end(), inREcnt)
           == this->TestsToRun.end()) || !it->IsInBasedOnREOptions)
        {
        continue;
        }
      }
    it->Index = cnt;  // save the index into the test list for this test
    finalList.push_back(*it);
    }
  // Save the total number of tests before exclusions
  this->TotalNumberOfTests = this->TestList.size();
  // Set the TestList to the final list of all test
  this->TestList = finalList;

  this->UpdateMaxTestNameWidth();
}

void cmCTestTestHandler::ComputeTestListForRerunFailed()
{
  this->ExpandTestsToRunInformationForRerunFailed();

  cmCTestTestHandler::ListOfTests::iterator it;
  ListOfTests finalList;
  int cnt = 0;
  for ( it = this->TestList.begin(); it != this->TestList.end(); it ++ )
    {
    cnt ++;

    // if this test is not in our list of tests to run, then skip it.
    if ((!this->TestsToRun.empty() &&
         std::find(this->TestsToRun.begin(), this->TestsToRun.end(), cnt)
         == this->TestsToRun.end()))
      {
      continue;
      }

    it->Index = cnt;
    finalList.push_back(*it);
    }

  // Save the total number of tests before exclusions
  this->TotalNumberOfTests = this->TestList.size();

  // Set the TestList to the list of failed tests to rerun
  this->TestList = finalList;

  this->UpdateMaxTestNameWidth();
}

void cmCTestTestHandler::UpdateMaxTestNameWidth()
{
  std::string::size_type max = this->CTest->GetMaxTestNameWidth();
  for ( cmCTestTestHandler::ListOfTests::iterator it = this->TestList.begin();
        it != this->TestList.end(); it ++ )
    {
    cmCTestTestProperties& p = *it;
    if(max < p.Name.size())
      {
      max = p.Name.size();
      }
    }
  if(static_cast<std::string::size_type>(this->CTest->GetMaxTestNameWidth())
     != max)
    {
    this->CTest->SetMaxTestNameWidth(static_cast<int>(max));
    }
}

bool cmCTestTestHandler::GetValue(const char* tag,
                                  int& value,
                                  std::istream& fin)
{
  std::string line;
  bool ret = true;
  cmSystemTools::GetLineFromStream(fin, line);
  if(line == tag)
    {
    fin >> value;
    ret = cmSystemTools::GetLineFromStream(fin, line); // read blank line
    }
  else
    {
    cmCTestLog(this->CTest, ERROR_MESSAGE,
               "parse error: missing tag: "
               << tag << " found [" << line << "]" << std::endl);
    ret = false;
    }
  return ret;
}

bool cmCTestTestHandler::GetValue(const char* tag,
                                  double& value,
                                  std::istream& fin)
{
  std::string line;
  cmSystemTools::GetLineFromStream(fin, line);
  bool ret = true;
  if(line == tag)
    {
    fin >> value;
    ret = cmSystemTools::GetLineFromStream(fin, line); // read blank line
    }
  else
    {
    cmCTestLog(this->CTest, ERROR_MESSAGE,
               "parse error: missing tag: "
               << tag << " found [" << line << "]" << std::endl);
    ret = false;
    }
  return ret;
}

bool cmCTestTestHandler::GetValue(const char* tag,
                                  bool& value,
                                  std::istream& fin)
{
  std::string line;
  cmSystemTools::GetLineFromStream(fin, line);
  bool ret = true;
  if(line == tag)
    {
#ifdef __HAIKU__
    int tmp = 0;
    fin >> tmp;
    value = false;
    if(tmp)
      {
      value = true;
      }
#else
    fin >> value;
#endif
    ret = cmSystemTools::GetLineFromStream(fin, line); // read blank line
    }
  else
    {
    cmCTestLog(this->CTest, ERROR_MESSAGE,
               "parse error: missing tag: "
               << tag << " found [" << line << "]" << std::endl);
    ret = false;
    }
  return ret;
}

bool cmCTestTestHandler::GetValue(const char* tag,
                                  size_t& value,
                                  std::istream& fin)
{
  std::string line;
  cmSystemTools::GetLineFromStream(fin, line);
  bool ret = true;
  if(line == tag)
    {
    fin >> value;
    ret = cmSystemTools::GetLineFromStream(fin, line); // read blank line
    }
  else
    {
    cmCTestLog(this->CTest, ERROR_MESSAGE,
               "parse error: missing tag: "
               << tag << " found [" << line << "]" << std::endl);
    ret = false;
    }
  return ret;
}

bool cmCTestTestHandler::GetValue(const char* tag,
                                  std::string& value,
                                  std::istream& fin)
{
  std::string line;
  cmSystemTools::GetLineFromStream(fin, line);
  bool ret = true;
  if(line == tag)
    {
    ret =  cmSystemTools::GetLineFromStream(fin, value);
    }
  else
    {
    cmCTestLog(this->CTest, ERROR_MESSAGE,
               "parse error: missing tag: "
               << tag << " found [" << line << "]" << std::endl);
    ret = false;
    }
  return ret;
}

//---------------------------------------------------------------------
void cmCTestTestHandler::ProcessDirectory(std::vector<std::string> &passed,
                                         std::vector<std::string> &failed)
{
  this->ComputeTestList();
  this->StartTest = this->CTest->CurrentTime();
  this->StartTestTime = static_cast<unsigned int>(cmSystemTools::GetTime());
  double elapsed_time_start = cmSystemTools::GetTime();

  cmCTestMultiProcessHandler* parallel = this->CTest->GetBatchJobs() ?
    new cmCTestBatchTestHandler : new cmCTestMultiProcessHandler;
  parallel->SetCTest(this->CTest);
  parallel->SetParallelLevel(this->CTest->GetParallelLevel());
  parallel->SetTestHandler(this);
  parallel->SetQuiet(this->Quiet);

  *this->LogFile << "Start testing: "
    << this->CTest->CurrentTime() << std::endl
    << "----------------------------------------------------------"
    << std::endl;

  cmCTestMultiProcessHandler::TestMap tests;
  cmCTestMultiProcessHandler::PropertiesMap properties;

  bool randomSchedule = this->CTest->GetScheduleType() == "Random";
  if(randomSchedule)
    {
    srand((unsigned)time(0));
    }

  for (ListOfTests::iterator it = this->TestList.begin();
       it != this->TestList.end(); ++it)
    {
    cmCTestTestProperties& p = *it;
    cmCTestMultiProcessHandler::TestSet depends;

    if(randomSchedule)
      {
      p.Cost = static_cast<float>(rand());
      }

    if(p.Timeout == 0 && this->CTest->GetGlobalTimeout() != 0)
      {
      p.Timeout = this->CTest->GetGlobalTimeout();
      }

    if(!p.Depends.empty())
      {
      for(std::vector<std::string>::iterator i = p.Depends.begin();
          i != p.Depends.end(); ++i)
        {
        for(ListOfTests::iterator it2 = this->TestList.begin();
            it2 != this->TestList.end(); ++it2)
          {
          if(it2->Name == *i)
            {
            depends.insert(it2->Index);
            break; // break out of test loop as name can only match 1
            }
          }
        }
      }
    tests[it->Index] = depends;
    properties[it->Index] = &*it;
    }
  parallel->SetTests(tests, properties);
  parallel->SetPassFailVectors(&passed, &failed);
  this->TestResults.clear();
  parallel->SetTestResults(&this->TestResults);

  if(this->CTest->ShouldPrintLabels())
    {
    parallel->PrintLabels();
    }
  else if(this->CTest->GetShowOnly())
    {
    parallel->PrintTestList();
    }
  else
    {
    parallel->RunTests();
    }
  delete parallel;
  this->EndTest = this->CTest->CurrentTime();
  this->EndTestTime = static_cast<unsigned int>(cmSystemTools::GetTime());
  this->ElapsedTestingTime = cmSystemTools::GetTime() - elapsed_time_start;
  *this->LogFile << "End testing: "
     << this->CTest->CurrentTime() << std::endl;
}

//----------------------------------------------------------------------
void cmCTestTestHandler::GenerateTestCommand(std::vector<std::string>&, int)
{
}

//----------------------------------------------------------------------
void cmCTestTestHandler::GenerateDartOutput(cmXMLWriter& xml)
{
  if ( !this->CTest->GetProduceXML() )
    {
    return;
    }

  this->CTest->StartXML(xml, this->AppendXML);
  xml.StartElement("Testing");
  xml.Element("StartDateTime", this->StartTest);
  xml.Element("StartTestTime", this->StartTestTime);
  xml.StartElement("TestList");
  cmCTestTestHandler::TestResultsVector::size_type cc;
  for ( cc = 0; cc < this->TestResults.size(); cc ++ )
    {
    cmCTestTestResult *result = &this->TestResults[cc];
    std::string testPath = result->Path + "/" + result->Name;
    xml.Element("Test", this->CTest->GetShortPathToFile(testPath.c_str()));
    }
  xml.EndElement(); // TestList
  for ( cc = 0; cc < this->TestResults.size(); cc ++ )
    {
    cmCTestTestResult *result = &this->TestResults[cc];
    this->WriteTestResultHeader(xml, result);
    xml.StartElement("Results");
    if ( result->Status != cmCTestTestHandler::NOT_RUN )
      {
      if ( result->Status != cmCTestTestHandler::COMPLETED ||
        result->ReturnValue )
        {
        xml.StartElement("NamedMeasurement");
        xml.Attribute("type", "text/string");
        xml.Attribute("name", "Exit Code");
        xml.Element("Value", this->GetTestStatus(result->Status));
        xml.EndElement(); // NamedMeasurement
        xml.StartElement("NamedMeasurement");
        xml.Attribute("type", "text/string");
        xml.Attribute("name", "Exit Value");
        xml.Element("Value", result->ReturnValue);
        xml.EndElement(); // NamedMeasurement
        }
      this->GenerateRegressionImages(xml, result->DartString);
      xml.StartElement("NamedMeasurement");
      xml.Attribute("type", "numeric/double");
      xml.Attribute("name", "Execution Time");
      xml.Element("Value", result->ExecutionTime);
      xml.EndElement(); // NamedMeasurement
      if(!result->Reason.empty())
        {
        const char* reasonType = "Pass Reason";
        if(result->Status != cmCTestTestHandler::COMPLETED &&
           result->Status != cmCTestTestHandler::NOT_RUN)
          {
          reasonType = "Fail Reason";
          }
        xml.StartElement("NamedMeasurement");
        xml.Attribute("type", "text/string");
        xml.Attribute("name", reasonType);
        xml.Element("Value", result->Reason);
        xml.EndElement(); // NamedMeasurement
        }
      xml.StartElement("NamedMeasurement");
      xml.Attribute("type", "text/string");
      xml.Attribute("name", "Completion Status");
      xml.Element("Value", result->CompletionStatus);
      xml.EndElement(); // NamedMeasurement
      }
    xml.StartElement("NamedMeasurement");
    xml.Attribute("type", "text/string");
    xml.Attribute("name", "Command Line");
    xml.Element("Value", result->FullCommandLine);
    xml.EndElement(); // NamedMeasurement
    std::map<std::string,std::string>::iterator measureIt;
    for ( measureIt = result->Properties->Measurements.begin();
      measureIt != result->Properties->Measurements.end();
      ++ measureIt )
      {
      xml.StartElement("NamedMeasurement");
      xml.Attribute("type", "text/string");
      xml.Attribute("name", measureIt->first);
      xml.Element("Value", measureIt->second);
      xml.EndElement(); // NamedMeasurement
      }
    xml.StartElement("Measurement");
    xml.StartElement("Value");
    if (result->CompressOutput)
      {
      xml.Attribute("encoding", "base64");
      xml.Attribute("compression", "gzip");
      }
    xml.Content(result->Output);
    xml.EndElement(); // Value
    xml.EndElement(); // Measurement
    xml.EndElement(); // Results

    this->AttachFiles(xml, result);
    this->WriteTestResultFooter(xml, result);
    }

  xml.Element("EndDateTime", this->EndTest);
  xml.Element("EndTestTime", this->EndTestTime);
  xml.Element("ElapsedMinutes",
    static_cast<int>(this->ElapsedTestingTime/6)/10.0);
  xml.EndElement(); // Testing
  this->CTest->EndXML(xml);
}

//----------------------------------------------------------------------------
void cmCTestTestHandler::WriteTestResultHeader(cmXMLWriter& xml,
                                               cmCTestTestResult* result)
{
  xml.StartElement("Test");
  if ( result->Status == cmCTestTestHandler::COMPLETED )
    {
    xml.Attribute("Status", "passed");
    }
  else if ( result->Status == cmCTestTestHandler::NOT_RUN )
    {
    xml.Attribute("Status", "notrun");
    }
  else
    {
    xml.Attribute("Status", "failed");
    }
  std::string testPath = result->Path + "/" + result->Name;
  xml.Element("Name", result->Name);
  xml.Element("Path", this->CTest->GetShortPathToFile(result->Path.c_str()));
  xml.Element("FullName", this->CTest->GetShortPathToFile(testPath.c_str()));
  xml.Element("FullCommandLine", result->FullCommandLine);
}

//----------------------------------------------------------------------------
void cmCTestTestHandler::WriteTestResultFooter(cmXMLWriter& xml,
                                               cmCTestTestResult* result)
{
  if(!result->Properties->Labels.empty())
    {
    xml.StartElement("Labels");
    std::vector<std::string> const& labels = result->Properties->Labels;
    for(std::vector<std::string>::const_iterator li = labels.begin();
        li != labels.end(); ++li)
      {
      xml.Element("Label", *li);
      }
    xml.EndElement(); // Labels
    }

  xml.EndElement(); // Test
}

//----------------------------------------------------------------------
void cmCTestTestHandler::AttachFiles(cmXMLWriter& xml,
                                     cmCTestTestResult* result)
{
  if(result->Status != cmCTestTestHandler::COMPLETED
     && result->Properties->AttachOnFail.size())
    {
    result->Properties->AttachedFiles.insert(
      result->Properties->AttachedFiles.end(),
      result->Properties->AttachOnFail.begin(),
      result->Properties->AttachOnFail.end());
    }
  for(std::vector<std::string>::const_iterator file =
      result->Properties->AttachedFiles.begin();
      file != result->Properties->AttachedFiles.end(); ++file)
    {
    const std::string &base64 = this->CTest->Base64GzipEncodeFile(*file);
    std::string fname = cmSystemTools::GetFilenameName(*file);
    xml.StartElement("NamedMeasurement");
    xml.Attribute("name", "Attached File");
    xml.Attribute("encoding", "base64");
    xml.Attribute("compression", "tar/gzip");
    xml.Attribute("filename", fname);
    xml.Attribute("type", "file");
    xml.Element("Value", base64);
    xml.EndElement(); // NamedMeasurement
    }
}

//----------------------------------------------------------------------
int cmCTestTestHandler::ExecuteCommands(std::vector<std::string>& vec)
{
  std::vector<std::string>::iterator it;
  for ( it = vec.begin(); it != vec.end(); ++it )
    {
    int retVal = 0;
    cmCTestOptionalLog(this->CTest, HANDLER_VERBOSE_OUTPUT, "Run command: " <<
      *it << std::endl, this->Quiet);
    if ( !cmSystemTools::RunSingleCommand(it->c_str(), 0, 0, &retVal, 0,
                                          cmSystemTools::OUTPUT_MERGE
        /*this->Verbose*/) || retVal != 0 )
      {
      cmCTestLog(this->CTest, ERROR_MESSAGE, "Problem running command: "
        << *it << std::endl);
      return 0;
      }
    }
  return 1;
}


//----------------------------------------------------------------------
// Find the appropriate executable to run for a test
std::string cmCTestTestHandler::FindTheExecutable(const char *exe)
{
  std::string resConfig;
  std::vector<std::string> extraPaths;
  std::vector<std::string> failedPaths;
  if(strcmp(exe, "NOT_AVAILABLE") == 0)
    {
    return exe;
    }
  return cmCTestTestHandler::FindExecutable(this->CTest,
                                            exe, resConfig,
                                            extraPaths,
                                            failedPaths);
}

// add additional configurations to the search path
void cmCTestTestHandler
::AddConfigurations(cmCTest *ctest,
                    std::vector<std::string> &attempted,
                    std::vector<std::string> &attemptedConfigs,
                    std::string filepath,
                    std::string &filename)
{
  std::string tempPath;

  if (!filepath.empty() &&
      filepath[filepath.size()-1] != '/')
    {
    filepath += "/";
    }
  tempPath = filepath + filename;
  attempted.push_back(tempPath);
  attemptedConfigs.push_back("");

  if(!ctest->GetConfigType().empty())
    {
    tempPath = filepath;
    tempPath += ctest->GetConfigType();
    tempPath += "/";
    tempPath += filename;
    attempted.push_back(tempPath);
    attemptedConfigs.push_back(ctest->GetConfigType());
    // If the file is an OSX bundle then the configtype
    // will be at the start of the path
    tempPath = ctest->GetConfigType();
    tempPath += "/";
    tempPath += filepath;
    tempPath += filename;
    attempted.push_back(tempPath);
    attemptedConfigs.push_back(ctest->GetConfigType());
    }
  else
    {
    // no config specified - try some options...
    tempPath = filepath;
    tempPath += "Release/";
    tempPath += filename;
    attempted.push_back(tempPath);
    attemptedConfigs.push_back("Release");
    tempPath = filepath;
    tempPath += "Debug/";
    tempPath += filename;
    attempted.push_back(tempPath);
    attemptedConfigs.push_back("Debug");
    tempPath = filepath;
    tempPath += "MinSizeRel/";
    tempPath += filename;
    attempted.push_back(tempPath);
    attemptedConfigs.push_back("MinSizeRel");
    tempPath = filepath;
    tempPath += "RelWithDebInfo/";
    tempPath += filename;
    attempted.push_back(tempPath);
    attemptedConfigs.push_back("RelWithDebInfo");
    tempPath = filepath;
    tempPath += "Deployment/";
    tempPath += filename;
    attempted.push_back(tempPath);
    attemptedConfigs.push_back("Deployment");
    tempPath = filepath;
    tempPath += "Development/";
    tempPath += filename;
    attempted.push_back(tempPath);
    attemptedConfigs.push_back("Deployment");
    }
}


//----------------------------------------------------------------------
// Find the appropriate executable to run for a test
std::string cmCTestTestHandler
::FindExecutable(cmCTest *ctest,
                 const char *testCommand,
                 std::string &resultingConfig,
                 std::vector<std::string> &extraPaths,
                 std::vector<std::string> &failed)
{
  // now run the compiled test if we can find it
  std::vector<std::string> attempted;
  std::vector<std::string> attemptedConfigs;
  std::string tempPath;
  std::string filepath =
    cmSystemTools::GetFilenamePath(testCommand);
  std::string filename =
    cmSystemTools::GetFilenameName(testCommand);

  cmCTestTestHandler::AddConfigurations(ctest, attempted,
                                        attemptedConfigs,
                                        filepath,filename);

  // even if a fullpath was specified also try it relative to the current
  // directory
  if (!filepath.empty() && filepath[0] == '/')
    {
    std::string localfilepath = filepath.substr(1,filepath.size()-1);
    cmCTestTestHandler::AddConfigurations(ctest, attempted,
                                          attemptedConfigs,
                                          localfilepath,filename);
    }


  // if extraPaths are provided and we were not passed a full path, try them,
  // try any extra paths
  if (filepath.empty())
    {
    for (unsigned int i = 0; i < extraPaths.size(); ++i)
      {
      std::string filepathExtra =
        cmSystemTools::GetFilenamePath(extraPaths[i]);
      std::string filenameExtra =
        cmSystemTools::GetFilenameName(extraPaths[i]);
      cmCTestTestHandler::AddConfigurations(ctest,attempted,
                                            attemptedConfigs,
                                            filepathExtra,
                                            filenameExtra);
      }
    }

  // store the final location in fullPath
  std::string fullPath;

  // now look in the paths we specified above
  for(unsigned int ai=0;
      ai < attempted.size() && fullPath.empty(); ++ai)
    {
    // first check without exe extension
    if(cmSystemTools::FileExists(attempted[ai].c_str())
       && !cmSystemTools::FileIsDirectory(attempted[ai]))
      {
      fullPath = cmSystemTools::CollapseFullPath(attempted[ai]);
      resultingConfig = attemptedConfigs[ai];
      }
    // then try with the exe extension
    else
      {
      failed.push_back(attempted[ai]);
      tempPath = attempted[ai];
      tempPath += cmSystemTools::GetExecutableExtension();
      if(cmSystemTools::FileExists(tempPath.c_str())
         && !cmSystemTools::FileIsDirectory(tempPath))
        {
        fullPath = cmSystemTools::CollapseFullPath(tempPath);
        resultingConfig = attemptedConfigs[ai];
        }
      else
        {
        failed.push_back(tempPath);
        }
      }
    }

  // if everything else failed, check the users path, but only if a full path
  // wasn't specified
  if (fullPath.empty() && filepath.empty())
    {
    std::string path = cmSystemTools::FindProgram(filename.c_str());
    if (path != "")
      {
      resultingConfig = "";
      return path;
      }
    }
  if(fullPath.empty())
    {
    cmCTestLog(ctest, HANDLER_OUTPUT,
               "Could not find executable " << testCommand << "\n"
               << "Looked in the following places:\n");
    for(std::vector<std::string>::iterator i = failed.begin();
        i != failed.end(); ++i)
      {
      cmCTestLog(ctest, HANDLER_OUTPUT, i->c_str() << "\n");
      }
    }

  return fullPath;
}


//----------------------------------------------------------------------
void cmCTestTestHandler::GetListOfTests()
{
  if ( !this->IncludeLabelRegExp.empty() )
    {
    this->IncludeLabelRegularExpression.
      compile(this->IncludeLabelRegExp.c_str());
    }
  if ( !this->ExcludeLabelRegExp.empty() )
    {
    this->ExcludeLabelRegularExpression.
      compile(this->ExcludeLabelRegExp.c_str());
    }
  if ( !this->IncludeRegExp.empty() )
    {
    this->IncludeTestsRegularExpression.compile(this->IncludeRegExp.c_str());
    }
  if ( !this->ExcludeRegExp.empty() )
    {
    this->ExcludeTestsRegularExpression.compile(this->ExcludeRegExp.c_str());
    }
  cmCTestOptionalLog(this->CTest, HANDLER_VERBOSE_OUTPUT,
    "Constructing a list of tests" << std::endl, this->Quiet);
  cmake cm;
  cm.SetHomeDirectory("");
  cm.SetHomeOutputDirectory("");
  cmGlobalGenerator gg(&cm);
  cmsys::auto_ptr<cmLocalGenerator> lg(gg.MakeLocalGenerator());
  cmMakefile *mf = lg->GetMakefile();
  mf->AddDefinition("CTEST_CONFIGURATION_TYPE",
    this->CTest->GetConfigType().c_str());

  // Add handler for ADD_TEST
  cmCTestAddTestCommand* newCom1 = new cmCTestAddTestCommand;
  newCom1->TestHandler = this;
  cm.GetState()->AddCommand(newCom1);

  // Add handler for SUBDIRS
  cmCTestSubdirCommand* newCom2 =
    new cmCTestSubdirCommand;
  newCom2->TestHandler = this;
  cm.GetState()->AddCommand(newCom2);

  // Add handler for ADD_SUBDIRECTORY
  cmCTestAddSubdirectoryCommand* newCom3 =
    new cmCTestAddSubdirectoryCommand;
  newCom3->TestHandler = this;
  cm.GetState()->AddCommand(newCom3);

  // Add handler for SET_SOURCE_FILES_PROPERTIES
  cmCTestSetTestsPropertiesCommand* newCom4
    = new cmCTestSetTestsPropertiesCommand;
  newCom4->TestHandler = this;
  cm.GetState()->AddCommand(newCom4);

  const char* testFilename;
  if( cmSystemTools::FileExists("CTestTestfile.cmake") )
    {
    // does the CTestTestfile.cmake exist ?
    testFilename = "CTestTestfile.cmake";
    }
  else if( cmSystemTools::FileExists("DartTestfile.txt") )
    {
    // does the DartTestfile.txt exist ?
    testFilename = "DartTestfile.txt";
    }
  else
    {
    return;
    }

  if ( !mf->ReadListFile(testFilename) )
    {
    return;
    }
  if ( cmSystemTools::GetErrorOccuredFlag() )
    {
    return;
    }
  cmCTestOptionalLog(this->CTest, HANDLER_VERBOSE_OUTPUT,
    "Done constructing a list of tests" << std::endl, this->Quiet);
}

//----------------------------------------------------------------------
void cmCTestTestHandler::UseIncludeRegExp()
{
  this->UseIncludeRegExpFlag = true;
}

//----------------------------------------------------------------------
void cmCTestTestHandler::UseExcludeRegExp()
{
  this->UseExcludeRegExpFlag = true;
  this->UseExcludeRegExpFirst = this->UseIncludeRegExpFlag ? false : true;
}

//----------------------------------------------------------------------
const char* cmCTestTestHandler::GetTestStatus(int status)
{
  static const char statuses[][100] = {
    "Not Run",
    "Timeout",
    "SEGFAULT",
    "ILLEGAL",
    "INTERRUPT",
    "NUMERICAL",
    "OTHER_FAULT",
    "Failed",
    "BAD_COMMAND",
    "Completed"
  };

  if ( status < cmCTestTestHandler::NOT_RUN ||
       status > cmCTestTestHandler::COMPLETED )
    {
    return "No Status";
    }
  return statuses[status];
}

//----------------------------------------------------------------------
void cmCTestTestHandler::ExpandTestsToRunInformation(size_t numTests)
{
  if (this->TestsToRunString.empty())
    {
    return;
    }

  int start;
  int end = -1;
  double stride = -1;
  std::string::size_type pos = 0;
  std::string::size_type pos2;
  // read start
  if(GetNextNumber(this->TestsToRunString, start, pos, pos2))
    {
    // read end
    if(GetNextNumber(this->TestsToRunString, end, pos, pos2))
      {
      // read stride
      if(GetNextRealNumber(this->TestsToRunString, stride, pos, pos2))
        {
        int val =0;
        // now read specific numbers
        while(GetNextNumber(this->TestsToRunString, val, pos, pos2))
          {
          this->TestsToRun.push_back(val);
          }
        this->TestsToRun.push_back(val);
        }
      }
    }

  // if start is not specified then we assume we start at 1
  if(start == -1)
    {
    start = 1;
    }

  // if end isnot specified then we assume we end with the last test
  if(end == -1)
    {
    end = static_cast<int>(numTests);
    }

  // if the stride wasn't specified then it defaults to 1
  if(stride == -1)
    {
    stride = 1;
    }

  // if we have a range then add it
  if(end != -1 && start != -1 && stride > 0)
    {
    int i = 0;
    while (i*stride + start <= end)
      {
      this->TestsToRun.push_back(static_cast<int>(i*stride+start));
      ++i;
      }
    }

  // sort the array
  std::sort(this->TestsToRun.begin(), this->TestsToRun.end(),
    std::less<int>());
  // remove duplicates
  std::vector<int>::iterator new_end =
    std::unique(this->TestsToRun.begin(), this->TestsToRun.end());
  this->TestsToRun.erase(new_end, this->TestsToRun.end());
}

void cmCTestTestHandler::ExpandTestsToRunInformationForRerunFailed()
{

  std::string dirName = this->CTest->GetBinaryDir() + "/Testing/Temporary";

  cmsys::Directory directory;
  if (directory.Load(dirName) == 0)
    {
    cmCTestLog(this->CTest, ERROR_MESSAGE, "Unable to read the contents of "
      << dirName << std::endl);
    return;
    }

  int numFiles = static_cast<int>
    (cmsys::Directory::GetNumberOfFilesInDirectory(dirName));
  std::string pattern = "LastTestsFailed";
  std::string logName = "";

  for (int i = 0; i < numFiles; ++i)
    {
    std::string fileName = directory.GetFile(i);
    // bcc crashes if we attempt a normal substring comparison,
    // hence the following workaround
    std::string fileNameSubstring = fileName.substr(0, pattern.length());
    if (fileNameSubstring.compare(pattern) != 0)
      {
      continue;
      }
    if (logName == "")
      {
      logName = fileName;
      }
    else
      {
      // if multiple matching logs were found we use the most recently
      // modified one.
      int res;
      cmSystemTools::FileTimeCompare(logName, fileName, &res);
      if (res == -1)
        {
        logName = fileName;
        }
      }
    }

  std::string lastTestsFailedLog = this->CTest->GetBinaryDir()
    + "/Testing/Temporary/" + logName;

  if ( !cmSystemTools::FileExists(lastTestsFailedLog.c_str()) )
    {
    if ( !this->CTest->GetShowOnly() && !this->CTest->ShouldPrintLabels() )
      {
      cmCTestLog(this->CTest, ERROR_MESSAGE, lastTestsFailedLog
        << " does not exist!" << std::endl);
      }
    return;
    }

  // parse the list of tests to rerun from LastTestsFailed.log
  cmsys::ifstream ifs(lastTestsFailedLog.c_str());
  if ( ifs )
    {
    std::string line;
    std::string::size_type pos;
    while ( cmSystemTools::GetLineFromStream(ifs, line) )
      {
      pos = line.find(':', 0);
      if (pos == line.npos)
        {
        continue;
        }

      int val = atoi(line.substr(0, pos).c_str());
      this->TestsToRun.push_back(val);
      }
    ifs.close();
    }
  else if ( !this->CTest->GetShowOnly() && !this->CTest->ShouldPrintLabels() )
    {
    cmCTestLog(this->CTest, ERROR_MESSAGE, "Problem reading file: "
      << lastTestsFailedLog <<
      " while generating list of previously failed tests." << std::endl);
    }
}

//----------------------------------------------------------------------
// Just for convenience
#define SPACE_REGEX "[ \t\r\n]"
//----------------------------------------------------------------------
void cmCTestTestHandler::GenerateRegressionImages(
  cmXMLWriter& xml, const std::string& dart)
{
  cmsys::RegularExpression twoattributes(
    "<DartMeasurement"
    SPACE_REGEX "*(name|type|encoding|compression)=\"([^\"]*)\""
    SPACE_REGEX "*(name|type|encoding|compression)=\"([^\"]*)\""
    SPACE_REGEX "*>([^<]*)</DartMeasurement>");
  cmsys::RegularExpression threeattributes(
    "<DartMeasurement"
    SPACE_REGEX "*(name|type|encoding|compression)=\"([^\"]*)\""
    SPACE_REGEX "*(name|type|encoding|compression)=\"([^\"]*)\""
    SPACE_REGEX "*(name|type|encoding|compression)=\"([^\"]*)\""
    SPACE_REGEX "*>([^<]*)</DartMeasurement>");
  cmsys::RegularExpression fourattributes(
    "<DartMeasurement"
    SPACE_REGEX "*(name|type|encoding|compression)=\"([^\"]*)\""
    SPACE_REGEX "*(name|type|encoding|compression)=\"([^\"]*)\""
    SPACE_REGEX "*(name|type|encoding|compression)=\"([^\"]*)\""
    SPACE_REGEX "*(name|type|encoding|compression)=\"([^\"]*)\""
    SPACE_REGEX "*>([^<]*)</DartMeasurement>");
  cmsys::RegularExpression cdatastart(
    "<DartMeasurement"
    SPACE_REGEX "*(name|type|encoding|compression)=\"([^\"]*)\""
    SPACE_REGEX "*(name|type|encoding|compression)=\"([^\"]*)\""
    SPACE_REGEX "*>"
    SPACE_REGEX "*<!\\[CDATA\\[");
  cmsys::RegularExpression cdataend(
    "]]>"
    SPACE_REGEX "*</DartMeasurement>");
  cmsys::RegularExpression measurementfile(
    "<DartMeasurementFile"
    SPACE_REGEX "*(name|type|encoding|compression)=\"([^\"]*)\""
    SPACE_REGEX "*(name|type|encoding|compression)=\"([^\"]*)\""
    SPACE_REGEX "*>([^<]*)</DartMeasurementFile>");

  bool done = false;
  std::string cxml = dart;
  while ( ! done )
    {
    if ( twoattributes.find(cxml) )
      {
      xml.StartElement("NamedMeasurement");
      xml.Attribute(twoattributes.match(1).c_str(),
                    twoattributes.match(2));
      xml.Attribute(twoattributes.match(3).c_str(),
                    twoattributes.match(4));
      xml.Element("Value", twoattributes.match(5));
      xml.EndElement();
      cxml.erase(twoattributes.start(),
        twoattributes.end() - twoattributes.start());
      }
    else if ( threeattributes.find(cxml) )
      {
      xml.StartElement("NamedMeasurement");
      xml.Attribute(threeattributes.match(1).c_str(),
                    threeattributes.match(2));
      xml.Attribute(threeattributes.match(3).c_str(),
                    threeattributes.match(4));
      xml.Attribute(threeattributes.match(5).c_str(),
                    threeattributes.match(6));
      xml.Element("Value", twoattributes.match(7));
      xml.EndElement();
      cxml.erase(threeattributes.start(),
        threeattributes.end() - threeattributes.start());
      }
    else if ( fourattributes.find(cxml) )
      {
      xml.StartElement("NamedMeasurement");
      xml.Attribute(fourattributes.match(1).c_str(),
                    fourattributes.match(2));
      xml.Attribute(fourattributes.match(3).c_str(),
                    fourattributes.match(4));
      xml.Attribute(fourattributes.match(5).c_str(),
                    fourattributes.match(6));
      xml.Attribute(fourattributes.match(7).c_str(),
                    fourattributes.match(8));
      xml.Element("Value", twoattributes.match(9));
      xml.EndElement();
      cxml.erase(fourattributes.start(),
        fourattributes.end() - fourattributes.start());
      }
    else if ( cdatastart.find(cxml) && cdataend.find(cxml) )
      {
      xml.StartElement("NamedMeasurement");
      xml.Attribute(cdatastart.match(1).c_str(), cdatastart.match(2));
      xml.Attribute(cdatastart.match(3).c_str(), cdatastart.match(4));
      xml.StartElement("Value");
      xml.CData(
          cxml.substr(cdatastart.end(), cdataend.start() - cdatastart.end()));
      xml.EndElement(); // Value
      xml.EndElement(); // NamedMeasurement
      cxml.erase(cdatastart.start(),
        cdataend.end() - cdatastart.start());
      }
    else if ( measurementfile.find(cxml) )
      {
      const std::string& filename =
        cmCTest::CleanString(measurementfile.match(5));
      if ( cmSystemTools::FileExists(filename.c_str()) )
        {
        long len = cmSystemTools::FileLength(filename);
        if ( len == 0 )
          {
          std::string k1 = measurementfile.match(1);
          std::string v1 = measurementfile.match(2);
          std::string k2 = measurementfile.match(3);
          std::string v2 = measurementfile.match(4);
          if ( cmSystemTools::LowerCase(k1) == "type" )
            {
            v1 = "text/string";
            }
          if ( cmSystemTools::LowerCase(k2) == "type" )
            {
            v2 = "text/string";
            }

          xml.StartElement("NamedMeasurement");
          xml.Attribute(k1.c_str(), v1);
          xml.Attribute(k2.c_str(), v2);
          xml.Attribute("encoding", "none");
          xml.Element("Value", "Image " + filename + " is empty");
          xml.EndElement();
          }
        else
          {
          cmsys::ifstream ifs(filename.c_str(), std::ios::in
#ifdef _WIN32
                            | std::ios::binary
#endif
            );
          unsigned char *file_buffer = new unsigned char [ len + 1 ];
          ifs.read(reinterpret_cast<char*>(file_buffer), len);
          unsigned char *encoded_buffer
            = new unsigned char [ static_cast<int>(
                static_cast<double>(len) * 1.5 + 5.0) ];

          size_t rlen
            = cmsysBase64_Encode(file_buffer, len, encoded_buffer, 1);

          xml.StartElement("NamedMeasurement");
          xml.Attribute(measurementfile.match(1).c_str(),
                        measurementfile.match(2));
          xml.Attribute(measurementfile.match(3).c_str(),
                        measurementfile.match(4));
          xml.Attribute("encoding", "base64");
          std::stringstream ostr;
          for (size_t cc = 0; cc < rlen; cc ++ )
            {
            ostr << encoded_buffer[cc];
            if ( cc % 60 == 0 && cc )
              {
              ostr << std::endl;
              }
            }
          xml.Element("Value", ostr.str());
          xml.EndElement(); // NamedMeasurement
          delete [] file_buffer;
          delete [] encoded_buffer;
          }
        }
      else
        {
        int idx = 4;
        if ( measurementfile.match(1) == "name" )
          {
          idx = 2;
          }
        xml.StartElement("NamedMeasurement");
        xml.Attribute("name", measurementfile.match(idx));
        xml.Attribute("text", "text/string");
        xml.Element("Value", "File " + filename + " not found");
        xml.EndElement();
        cmCTestOptionalLog(this->CTest, HANDLER_OUTPUT, "File \"" << filename
          << "\" not found." << std::endl, this->Quiet);
        }
      cxml.erase(measurementfile.start(),
        measurementfile.end() - measurementfile.start());
      }
    else
      {
      done = true;
      }
    }
}

//----------------------------------------------------------------------
void cmCTestTestHandler::SetIncludeRegExp(const char *arg)
{
  this->IncludeRegExp = arg;
}

//----------------------------------------------------------------------
void cmCTestTestHandler::SetExcludeRegExp(const char *arg)
{
  this->ExcludeRegExp = arg;
}

//----------------------------------------------------------------------
void cmCTestTestHandler::SetTestsToRunInformation(const char* in)
{
  if ( !in )
    {
    return;
    }
  this->TestsToRunString = in;
  // if the argument is a file, then read it and use the contents as the
  // string
  if(cmSystemTools::FileExists(in))
    {
    cmsys::ifstream fin(in);
    unsigned long filelen = cmSystemTools::FileLength(in);
    char* buff = new char[filelen+1];
    fin.getline(buff, filelen);
    buff[fin.gcount()] = 0;
    this->TestsToRunString = buff;
    delete [] buff;
    }
}

//----------------------------------------------------------------------------
bool cmCTestTestHandler::CleanTestOutput(std::string& output, size_t length)
{
  if(!length || length >= output.size() ||
     output.find("CTEST_FULL_OUTPUT") != output.npos)
    {
    return true;
    }

  // Truncate at given length but do not break in the middle of a multi-byte
  // UTF-8 encoding.
  char const* const begin = output.c_str();
  char const* const end = begin + output.size();
  char const* const truncate = begin + length;
  char const* current = begin;
  while(current < truncate)
    {
    unsigned int ch;
    if(const char* next = cm_utf8_decode_character(current, end, &ch))
      {
      if(next > truncate)
        {
        break;
        }
      current = next;
      }
    else // Bad byte will be handled by cmXMLSafe.
      {
      ++current;
      }
    }
  output = output.substr(0, current - begin);

  // Append truncation message.
  std::ostringstream msg;
  msg << "...\n"
    "The rest of the test output was removed since it exceeds the threshold "
    "of " << length << " bytes.\n";
  output += msg.str();
  return true;
}

//----------------------------------------------------------------------
bool cmCTestTestHandler::SetTestsProperties(
  const std::vector<std::string>& args)
{
  std::vector<std::string>::const_iterator it;
  std::vector<std::string> tests;
  bool found = false;
  for ( it = args.begin(); it != args.end(); ++ it )
    {
    if ( *it == "PROPERTIES" )
      {
      found = true;
      break;
      }
    tests.push_back(*it);
    }
  if ( !found )
    {
    return false;
    }
  ++ it; // skip PROPERTIES
  for ( ; it != args.end(); ++ it )
    {
    std::string key = *it;
    ++ it;
    if ( it == args.end() )
      {
      break;
      }
    std::string val = *it;
    std::vector<std::string>::const_iterator tit;
    for ( tit = tests.begin(); tit != tests.end(); ++ tit )
      {
      cmCTestTestHandler::ListOfTests::iterator rtit;
      for ( rtit = this->TestList.begin();
        rtit != this->TestList.end();
        ++ rtit )
        {
        if ( *tit == rtit->Name )
          {
          if ( key == "WILL_FAIL" )
            {
            rtit->WillFail = cmSystemTools::IsOn(val.c_str());
            }
          if ( key == "ATTACHED_FILES" )
            {
            cmSystemTools::ExpandListArgument(val, rtit->AttachedFiles);
            }
          if ( key == "ATTACHED_FILES_ON_FAIL" )
            {
            cmSystemTools::ExpandListArgument(val, rtit->AttachOnFail);
            }
          if ( key == "RESOURCE_LOCK" )
            {
            std::vector<std::string> lval;
            cmSystemTools::ExpandListArgument(val, lval);

            rtit->LockedResources.insert(lval.begin(), lval.end());
            }
          if ( key == "TIMEOUT" )
            {
            rtit->Timeout = atof(val.c_str());
            rtit->ExplicitTimeout = true;
            }
          if ( key == "COST" )
            {
            rtit->Cost = static_cast<float>(atof(val.c_str()));
            }
          if ( key == "REQUIRED_FILES" )
            {
            cmSystemTools::ExpandListArgument(val, rtit->RequiredFiles);
            }
          if ( key == "RUN_SERIAL" )
            {
            rtit->RunSerial = cmSystemTools::IsOn(val.c_str());
            }
          if ( key == "FAIL_REGULAR_EXPRESSION" )
            {
            std::vector<std::string> lval;
            cmSystemTools::ExpandListArgument(val, lval);
            std::vector<std::string>::iterator crit;
            for ( crit = lval.begin(); crit != lval.end(); ++ crit )
              {
              rtit->ErrorRegularExpressions.push_back(
                std::pair<cmsys::RegularExpression, std::string>(
                  cmsys::RegularExpression(crit->c_str()),
                  std::string(*crit)));
              }
            }
          if ( key == "PROCESSORS" )
            {
            rtit->Processors = atoi(val.c_str());
            if(rtit->Processors < 1)
              {
              rtit->Processors = 1;
              }
            }
          if ( key == "SKIP_RETURN_CODE" )
            {
            rtit->SkipReturnCode = atoi(val.c_str());
            if(rtit->SkipReturnCode < 0 || rtit->SkipReturnCode > 255)
              {
              rtit->SkipReturnCode = -1;
              }
            }
          if ( key == "DEPENDS" )
            {
            cmSystemTools::ExpandListArgument(val, rtit->Depends);
            }
          if ( key == "ENVIRONMENT" )
            {
            cmSystemTools::ExpandListArgument(val, rtit->Environment);
            }
          if ( key == "LABELS" )
            {
            cmSystemTools::ExpandListArgument(val, rtit->Labels);
            }
          if ( key == "MEASUREMENT" )
            {
            size_t pos = val.find_first_of("=");
            if ( pos != val.npos )
              {
              std::string mKey = val.substr(0, pos);
              const char* mVal = val.c_str() + pos + 1;
              rtit->Measurements[mKey] = mVal;
              }
            else
              {
              rtit->Measurements[val] = "1";
              }
            }
          if ( key == "PASS_REGULAR_EXPRESSION" )
            {
            std::vector<std::string> lval;
            cmSystemTools::ExpandListArgument(val, lval);
            std::vector<std::string>::iterator crit;
            for ( crit = lval.begin(); crit != lval.end(); ++ crit )
              {
              rtit->RequiredRegularExpressions.push_back(
                std::pair<cmsys::RegularExpression, std::string>(
                  cmsys::RegularExpression(crit->c_str()),
                  std::string(*crit)));
              }
            }
          if ( key == "WORKING_DIRECTORY" )
            {
            rtit->Directory = val;
            }
          }
        }
      }
    }
  return true;
}

//----------------------------------------------------------------------
bool cmCTestTestHandler::AddTest(const std::vector<std::string>& args)
{
  const std::string& testname = args[0];
  cmCTestOptionalLog(this->CTest, DEBUG, "Add test: " << args[0] << std::endl,
    this->Quiet);

  if (this->UseExcludeRegExpFlag &&
    this->UseExcludeRegExpFirst &&
    this->ExcludeTestsRegularExpression.find(testname.c_str()))
    {
    return true;
    }
  if ( this->MemCheck )
    {
    std::vector<std::string>::iterator it;
    bool found = false;
    for ( it = this->CustomTestsIgnore.begin();
      it != this->CustomTestsIgnore.end(); ++ it )
      {
      if ( *it == testname )
        {
        found = true;
        break;
        }
      }
    if ( found )
      {
      cmCTestOptionalLog(this->CTest, HANDLER_VERBOSE_OUTPUT,
        "Ignore memcheck: " << *it << std::endl, this->Quiet);
      return true;
      }
    }
  else
    {
    std::vector<std::string>::iterator it;
    bool found = false;
    for ( it = this->CustomTestsIgnore.begin();
      it != this->CustomTestsIgnore.end(); ++ it )
      {
      if ( *it == testname )
        {
        found = true;
        break;
        }
      }
    if ( found )
      {
      cmCTestOptionalLog(this->CTest, HANDLER_VERBOSE_OUTPUT, "Ignore test: "
        << *it << std::endl, this->Quiet);
      return true;
      }
    }

  cmCTestTestProperties test;
  test.Name = testname;
  test.Args = args;
  test.Directory = cmSystemTools::GetCurrentWorkingDirectory();
  cmCTestOptionalLog(this->CTest, DEBUG, "Set test directory: "
    << test.Directory << std::endl, this->Quiet);

  test.IsInBasedOnREOptions = true;
  test.WillFail = false;
  test.RunSerial = false;
  test.Timeout = 0;
  test.ExplicitTimeout = false;
  test.Cost = 0;
  test.Processors = 1;
  test.SkipReturnCode = -1;
  test.PreviousRuns = 0;
  if (this->UseIncludeRegExpFlag &&
    !this->IncludeTestsRegularExpression.find(testname.c_str()))
    {
    test.IsInBasedOnREOptions = false;
    }
  else if (this->UseExcludeRegExpFlag &&
    !this->UseExcludeRegExpFirst &&
    this->ExcludeTestsRegularExpression.find(testname.c_str()))
    {
    test.IsInBasedOnREOptions = false;
    }
  this->TestList.push_back(test);
  return true;
}

