/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2011 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "cmSystemTools.h"

#define cmPassed(m) std::cout << "Passed: " << m << "\n"
#define cmFailed(m) std::cout << "FAILED: " << m << "\n"; failed=1

int testSystemTools(int, char*[])
{
  int failed = 0;
  // ----------------------------------------------------------------------
  // Test cmSystemTools::UpperCase
  std::string str = "abc";
  std::string strupper = "ABC";
  if(cmSystemTools::UpperCase(str) == strupper)
    {
    cmPassed("cmSystemTools::UpperCase is working");
    }
  else
    {
    cmFailed("cmSystemTools::UpperCase is working");
    }
  return failed;
}
