/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skpdiff_util_DEFINED
#define skpdiff_util_DEFINED

#include <CL/cl.h>
#include "SkString.h"
#include "SkTArray.h"

/**
 * Converts an OpenCL error number into the string of its enumeration name.
 * @param  err The OpenCL error number
 * @return The string of the name of the error; "UNKOWN" if the error number is invalid
 */
const char* cl_error_to_string(cl_int err);

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


#endif
