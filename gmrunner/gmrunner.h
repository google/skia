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

struct ImageData {
  std::shared_ptr<uint8_t> pix;
  int height;
  int width;
  int byteOrder;
};

void gmrunner_init(std::list<std::string>& tests);
std::string gmrunner_run_test(const char* testName, ImageData* img);

#endif
