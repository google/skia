/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmCallVisualStudioMacro_h
#define cmCallVisualStudioMacro_h

#include "cmStandardIncludes.h"

/** \class cmCallVisualStudioMacro
 * \brief Control class for communicating with CMake's Visual Studio macros
 *
 * Find running instances of Visual Studio by full path solution name.
 * Call a Visual Studio IDE macro in any of those instances.
 */
class cmCallVisualStudioMacro
{
public:
  ///! Call the named macro in instances of Visual Studio with the
  ///! given solution file open. Pass "ALL" for slnFile to call the
  ///! macro in each Visual Studio instance.
  static int CallMacro(const std::string& slnFile,
                       const std::string& macro,
                       const std::string& args,
                       const bool logErrorsAsMessages);

  ///! Count the number of running instances of Visual Studio with the
  ///! given solution file open. Pass "ALL" for slnFile to count all
  ///! running Visual Studio instances.
  static int GetNumberOfRunningVisualStudioInstances(
    const std::string& slnFile);

protected:

private:
};

#endif
