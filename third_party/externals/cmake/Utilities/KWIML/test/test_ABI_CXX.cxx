/*============================================================================
  Kitware Information Macro Library
  Copyright 2010-2011 Kitware, Inc.

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "test.h"
#include KWIML_HEADER(ABI.h)
#include "test_ABI_endian.h"
extern "C" int test_ABI_CXX(void)
{
  if(!test_ABI_endian())
    {
    return 0;
    }
  return 1;
}
