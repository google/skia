/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2010 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cm_libarchive_h
#define cm_libarchive_h

/* Use the libarchive configured for CMake.  */
#include "cmThirdParty.h"
#ifdef CMAKE_USE_SYSTEM_LIBARCHIVE
# include <archive.h>
# include <archive_entry.h>
#else
# include <cmlibarchive/libarchive/archive.h>
# include <cmlibarchive/libarchive/archive_entry.h>
#endif

#endif
