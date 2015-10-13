/*============================================================================
  Kitware Information Macro Library
  Copyright 2010-2011 Kitware, Inc.

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include <string>

#if defined(_MSC_VER) && defined(NDEBUG)
// Use C++ runtime to avoid linker warning:
//  warning LNK4089: all references to 'MSVCP71.dll' discarded by /OPT:REF
std::string test_include_CXX_use_stl_string;
#endif

/* Test KWIML header inclusion after above system headers.  */
#include "test.h"
#include KWIML_HEADER(ABI.h)
#include KWIML_HEADER(INT.h)

extern "C" int test_include_CXX(void)
{
  return 1;
}
