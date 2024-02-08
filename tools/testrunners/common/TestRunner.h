/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef TestRunner_DEFINED
#define TestRunner_DEFINED

#include "tools/flags/CommandLineFlags.h"

namespace TestRunner {

// Determines whether a test case should be run based on the --match and --skip command-line flags.
bool ShouldRunTestCase(const char* name,
                       CommandLineFlags::StringArray& matchFlag,
                       CommandLineFlags::StringArray& skipFlag);

}  // namespace TestRunner

#endif  // TestRunner_DEFINED
