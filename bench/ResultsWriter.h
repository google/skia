/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 * Classes for writing out bench results in various formats.
 */

#ifndef SkResultsWriter_DEFINED
#define SkResultsWriter_DEFINED

#include "include/core/SkString.h"
#include "include/core/SkTypes.h"
#include "src/utils/SkJSONWriter.h"

/**
 NanoJSONResultsWriter helps nanobench writes the test results out in the following format:

 {
    "key": {
      "arch": "Arm7",
      "gpu": "SGX540",
      "os": "Android",
      "model": "GalaxyNexus",
    }
    "gitHash": "d1830323662ae8ae06908b97f15180fd25808894",
    "build_number": "1234",
    "results" : {
        "Xfermode_Luminosity_640_480" : {
           "8888" : {
                 "median_ms" : 143.188128906250,
                 "min_ms" : 143.835957031250,
                 ...
              },
          ...
*/
class NanoJSONResultsWriter : public SkJSONWriter {
public:
    NanoJSONResultsWriter(SkWStream* stream, Mode mode) : SkJSONWriter(stream, mode) {}

    void beginBench(const char* name, int32_t x, int32_t y) {
        SkString id = SkStringPrintf("%s_%d_%d", name, x, y);
        this->beginObject(id.c_str());
    }

    void endBench() { this->endObject(); }

    void appendMetric(const char* name, double value) {
        // Don't record if nan, or -nan.
        if (!sk_double_isnan(value)) {
            this->appendDoubleDigits(name, value, 16);
        }
    }
};

#endif
