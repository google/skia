/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2015 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmLocale_h
#define cmLocale_h

#include <locale.h>

class cmLocaleRAII
{
  const char* OldLocale;
public:
  cmLocaleRAII(): OldLocale(setlocale(LC_CTYPE, 0))
    {
    setlocale(LC_CTYPE, "");
    }
  ~cmLocaleRAII()
    {
    setlocale(LC_CTYPE, this->OldLocale);
    }
};

#endif
