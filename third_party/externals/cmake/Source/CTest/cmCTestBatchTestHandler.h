/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/

#ifndef cmCTestBatchTestHandler_h
#define cmCTestBatchTestHandler_h

#include <cmStandardIncludes.h>
#include <cmCTestTestHandler.h>
#include <cmCTestMultiProcessHandler.h>
#include <cmCTestRunTest.h>
#include <cmsys/FStream.hxx>

/** \class cmCTestBatchTestHandler
 * \brief run parallel ctest
 *
 * cmCTestBatchTestHandler
 */
class cmCTestBatchTestHandler : public cmCTestMultiProcessHandler
{
public:
  ~cmCTestBatchTestHandler();
  virtual void RunTests();
protected:
  void WriteBatchScript();
  void WriteSrunArgs(int test, cmsys::ofstream& fout);
  void WriteTestCommand(int test, cmsys::ofstream& fout);

  void SubmitBatchScript();

  std::string Script;
};

#endif
