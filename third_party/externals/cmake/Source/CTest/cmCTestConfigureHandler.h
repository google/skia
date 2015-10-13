/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc.

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/

#ifndef cmCTestConfigureHandler_h
#define cmCTestConfigureHandler_h


#include "cmCTestGenericHandler.h"
#include "cmListFileCache.h"

/** \class cmCTestConfigureHandler
 * \brief A class that handles ctest -S invocations
 *
 */
class cmCTestConfigureHandler : public cmCTestGenericHandler
{
public:
  cmTypeMacro(cmCTestConfigureHandler, cmCTestGenericHandler);

  /*
   * The main entry point for this class
   */
  int ProcessHandler();

  cmCTestConfigureHandler();

  void Initialize();
};

#endif
