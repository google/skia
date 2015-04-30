/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef ProcStats_DEFINED
#define ProcStats_DEFINED

/**
 * ProcStats - Process Statistics Functions
 */

namespace sk_tools {

/**
 *  If implemented, returns the maximum resident set size in MB.
 *  If not, returns -1.
 */
int getMaxResidentSetSizeMB();

/**
 *  If implemented, returns the current resident set size in MB.
 *  If not, returns -1.
 */
int getCurrResidentSetSizeMB();

}  // namespace sk_tools

#endif  // ProcStats_DEFINED
