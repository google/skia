/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "cmCTestTestCommand.h"

#include "cmCTest.h"
#include "cmCTestGenericHandler.h"

cmCTestTestCommand::cmCTestTestCommand()
{
  this->Arguments[ctt_START] = "START";
  this->Arguments[ctt_END] = "END";
  this->Arguments[ctt_STRIDE] = "STRIDE";
  this->Arguments[ctt_EXCLUDE] = "EXCLUDE";
  this->Arguments[ctt_INCLUDE] = "INCLUDE";
  this->Arguments[ctt_EXCLUDE_LABEL] = "EXCLUDE_LABEL";
  this->Arguments[ctt_INCLUDE_LABEL] = "INCLUDE_LABEL";
  this->Arguments[ctt_PARALLEL_LEVEL] = "PARALLEL_LEVEL";
  this->Arguments[ctt_SCHEDULE_RANDOM] = "SCHEDULE_RANDOM";
  this->Arguments[ctt_STOP_TIME] = "STOP_TIME";
  this->Arguments[ctt_LAST] = 0;
  this->Last = ctt_LAST;
}

cmCTestGenericHandler* cmCTestTestCommand::InitializeHandler()
{
  const char* ctestTimeout =
    this->Makefile->GetDefinition("CTEST_TEST_TIMEOUT");

  double timeout = this->CTest->GetTimeOut();
  if ( ctestTimeout )
    {
    timeout = atof(ctestTimeout);
    }
  else
    {
    if ( timeout <= 0 )
      {
      // By default use timeout of 10 minutes
      timeout = 600;
      }
    }
  this->CTest->SetTimeOut(timeout);
  cmCTestGenericHandler* handler = this->InitializeActualHandler();
  if ( this->Values[ctt_START] || this->Values[ctt_END] ||
    this->Values[ctt_STRIDE] )
    {
    std::ostringstream testsToRunString;
    if ( this->Values[ctt_START] )
      {
      testsToRunString << this->Values[ctt_START];
      }
    testsToRunString << ",";
    if ( this->Values[ctt_END] )
      {
      testsToRunString << this->Values[ctt_END];
      }
    testsToRunString << ",";
    if ( this->Values[ctt_STRIDE] )
      {
      testsToRunString << this->Values[ctt_STRIDE];
      }
    handler->SetOption("TestsToRunInformation",
      testsToRunString.str().c_str());
    }
  if(this->Values[ctt_EXCLUDE])
    {
    handler->SetOption("ExcludeRegularExpression", this->Values[ctt_EXCLUDE]);
    }
  if(this->Values[ctt_INCLUDE])
    {
    handler->SetOption("IncludeRegularExpression", this->Values[ctt_INCLUDE]);
    }
  if(this->Values[ctt_EXCLUDE_LABEL])
    {
    handler->SetOption("ExcludeLabelRegularExpression",
                       this->Values[ctt_EXCLUDE_LABEL]);
    }
  if(this->Values[ctt_INCLUDE_LABEL])
    {
    handler->SetOption("LabelRegularExpression",
                       this->Values[ctt_INCLUDE_LABEL]);
    }
  if(this->Values[ctt_PARALLEL_LEVEL])
    {
    handler->SetOption("ParallelLevel",
                       this->Values[ctt_PARALLEL_LEVEL]);
    }
  if(this->Values[ctt_SCHEDULE_RANDOM])
    {
    handler->SetOption("ScheduleRandom",
                       this->Values[ctt_SCHEDULE_RANDOM]);
    }
  if(this->Values[ctt_STOP_TIME])
    {
    this->CTest->SetStopTime(this->Values[ctt_STOP_TIME]);
    }
  handler->SetQuiet(this->Quiet);
  return handler;
}

cmCTestGenericHandler* cmCTestTestCommand::InitializeActualHandler()
{
  return this->CTest->GetInitializedHandler("test");
}
