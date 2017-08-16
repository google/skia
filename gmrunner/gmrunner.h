/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// EXPERIMENTAL EXPERIMENTAL EXPERIMENTAL EXPERIMENTAL
// DO NOT USE -- FOR INTERNAL TESTING ONLY

#ifndef gmrunner_DEFINED
#define gmrunner_DEFINED

#include <string>

bool gmrunner_init();
std::string gmrunner_run_test(const char* testName);

#endif
