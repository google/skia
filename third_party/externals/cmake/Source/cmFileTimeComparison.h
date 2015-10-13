/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmFileTimeComparison_h
#define cmFileTimeComparison_h

#include "cmStandardIncludes.h"

class cmFileTimeComparisonInternal;

/** \class cmFileTimeComparison
 * \brief Helper class for performing globbing searches.
 *
 * Finds all files that match a given globbing expression.
 */
class cmFileTimeComparison
{
public:
  cmFileTimeComparison();
  ~cmFileTimeComparison();

  /**
   *  Compare file modification times.
   *  Return true for successful comparison and false for error.
   *  When true is returned, result has -1, 0, +1 for
   *  f1 older, same, or newer than f2.
   */
  bool FileTimeCompare(const char* f1, const char* f2, int* result);

  /**
   *  Compare file modification times.  Return true unless both files
   *  exist and have modification times less than 1 second apart.
   */
  bool FileTimesDiffer(const char* f1, const char* f2);

protected:

  cmFileTimeComparisonInternal* Internals;
};


#endif

