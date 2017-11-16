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
#include <inttypes.h>

struct ImageData {
  std::vector<uint8_t> pix;
  int height;
  int width;
  int byteOrder;
};

void gmrunner_init(std::list<std::string>& tests);
void gmrunner_run_test(const char* testName, ImageData* outputImg, std::string& err);

#endif
