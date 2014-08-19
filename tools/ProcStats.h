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
 *  If not implemented for this OS, returns -1.  Otherwise, return
 *  the maximum resident set size, as reported by getrusage().
 */
int getMaxResidentSetSizeMB();

}  // namespace sk_tools

#endif  // ProcStats_DEFINED
