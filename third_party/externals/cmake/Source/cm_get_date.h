/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2015 Kitware, Inc.

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cm_get_date_h
#define cm_get_date_h

#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Parse a date/time string.  Treat relative times with respect to 'now'. */
time_t cm_get_date(time_t now, const char *str);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif
