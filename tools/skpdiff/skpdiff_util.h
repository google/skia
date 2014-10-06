/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skpdiff_util_DEFINED
#define skpdiff_util_DEFINED

#include "SkString.h"
#include "SkTArray.h"

#if SK_SUPPORT_OPENCL
#if defined(SK_BUILD_FOR_MAC)
#   include <OpenCL/cl.h>
#else
#   include <CL/cl.h>
#endif

/**
 * Converts an OpenCL error number into the string of its enumeration name.
 * @param  err The OpenCL error number
 * @return The string of the name of the error; "UNKOWN" if the error number is invalid
 */
const char* cl_error_to_string(cl_int err);
#endif

/**
 * Get a positive monotonic real-time measure of the amount of seconds since some undefined epoch.
 * Maximum precision is the goal of this routine.
 * @return Amount of time in seconds since some epoch
 */
double get_seconds();

/**
 * Get file entries of the given directory.
 * @param  path    A path to a directory to enumerate
 * @param  entries A vector to return the results into
 * @return         True on success, false otherwise
 */
bool get_directory(const char path[], SkTArray<SkString>* entries);

/**
 * Gets the files that match the specified pattern in sorted order.
 * @param  globPattern The pattern to use. Patterns must be valid paths, optionally with wildcards (*)
 * @param  entries     An array to return the results into
 * @return             True on success, false otherwise
 */
bool glob_files(const char globPattern[], SkTArray<SkString>* entries);

/**
 * Gets the absolute version of the given path.
 * @param  path The absolute or relative path to expand
 * @return      The absolute path of the given path on success, or an empty string on failure.
 */
SkString get_absolute_path(const SkString& path);


#endif
