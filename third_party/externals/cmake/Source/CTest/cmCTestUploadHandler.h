/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmCTestUploadHandler_h
#define cmCTestUploadHandler_h

#include "cmCTestGenericHandler.h"

/** \class cmCTestUploadHandler
 * \brief Helper class for CTest
 *
 * Submit arbitrary files
 *
 */
class cmCTestUploadHandler : public cmCTestGenericHandler
{
public:
  cmTypeMacro(cmCTestUploadHandler, cmCTestGenericHandler);

  cmCTestUploadHandler();
  ~cmCTestUploadHandler() {}

  /*
   * The main entry point for this class
   */
  int ProcessHandler();

  void Initialize();

  /** Specify a set of files to submit.  */
  void SetFiles(cmCTest::SetOfStrings const& files);

private:
  cmCTest::SetOfStrings Files;
};

#endif
