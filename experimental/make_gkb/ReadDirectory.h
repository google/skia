/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef ReadDirectory_DEFINED
#define ReadDirectory_DEFINED

#include <string>
#include <vector>

/* Abstract out filesystem tasks. */

/* Return the contents of a filesystem directory.  If ending is non-null, only
   return filenames that end in that string.  */
std::vector<std::string> ReadDirectory(const char* dir, const char* ending = nullptr);

/* Create a directory, if it does not exist already.
  Return true if directory already existed or successfully created. */
bool MakeDirectory(const char* path);

#endif  // ReadDirectory_DEFINED
