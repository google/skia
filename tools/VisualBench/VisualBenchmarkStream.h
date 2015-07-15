/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 */

#ifndef VisualBenchmarkStream_DEFINED
#define VisualBenchmarkStream_DEFINED

#include "Benchmark.h"
#include "gm.h"
#include "SkCommandLineFlags.h"
#include "SkPicture.h"

DECLARE_string(match);

class VisualBenchmarkStream {
public:
    VisualBenchmarkStream();

    static bool ReadPicture(const char* path, SkAutoTUnref<SkPicture>* pic);

    Benchmark* next();

private:
    Benchmark* innerNext();

    const BenchRegistry* fBenches;
    const skiagm::GMRegistry* fGMs;
    SkTArray<SkString> fSKPs;

    const char* fSourceType;  // What we're benching: bench, GM, SKP, ...
    const char* fBenchType;   // How we bench it: micro, playback, ...
    int fCurrentSKP;
};

#endif
