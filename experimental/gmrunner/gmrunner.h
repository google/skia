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

#include <list>
#include <memory>
#include <string>
#include <vector>

// ImageData holds images produced by GMs.
struct ImageData {
  std::vector<uint8_t> pix;
  int height;
  int width;
};

// gmrunner_init does necessary setup workd and enumerates the available GMs.
void gmrunner_init(std::list<std::string>& tests);

// gmrunner_run_test runs the GM identified by 'testName' and stores the result
// in 'outputImg'. If there is an error 'err' will be set to the appropriate
// error message. If 'err' is empty, no error occured.
void gmrunner_run_test(const char* testName, ImageData* outputImg, std::string& err);

#endif
