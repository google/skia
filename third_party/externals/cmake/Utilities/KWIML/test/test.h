/*============================================================================
  Kitware Information Macro Library
  Copyright 2010-2011 Kitware, Inc.

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef KWIML_NAMESPACE
# error "Do not include test.h outside of KWIML test files."
#endif

#ifndef KWIML_TEST_H
#define KWIML_TEST_H

/*
  Define KWIML_HEADER macro to help the test files include kwiml
  headers from the configured namespace directory.  The macro can be
  used like this:

  #include KWIML_HEADER(ABI.h)
*/
#define KWIML_HEADER(x) KWIML_HEADER0(KWIML_NAMESPACE/x)
#define KWIML_HEADER0(x) KWIML_HEADER1(x)
#define KWIML_HEADER1(x) <x>

/* Quiet MS standard library deprecation warnings.  */
#ifndef _CRT_SECURE_NO_DEPRECATE
# define _CRT_SECURE_NO_DEPRECATE
#endif

#else
# error "test.h included multiple times."
#endif
