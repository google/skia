/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef skqp_blacklist_DEFINED
#define skqp_blacklist_DEFINED
namespace skqp {
bool DoNotScoreInCompatibilityTestMode(const char* gmName);
bool DoNotExecuteInExperimentalMode(const char* gmName);
bool IsKnownGpuUnitTest(const char* testName);
bool IsKnownGM(const char* gmName);
}
#endif  // skqp_blacklist_DEFINED
