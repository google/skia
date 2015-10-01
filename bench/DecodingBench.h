/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef DecodingBench_DEFINED
#define DecodingBench_DEFINED

#include "Benchmark.h"
#include "SkData.h"
#include "SkImageDecoder.h"
#include "SkRefCnt.h"
#include "SkString.h"

/*
 *
 * This benchmark is designed to test the performance of image decoding.
 * It is invoked from the nanobench.cpp file.
 *
 */
class DecodingBench : public Benchmark {
public:
    DecodingBench(SkString path, SkColorType colorType);

protected:
    const char* onGetName() override;
    bool isSuitableFor(Backend backend) override;
    void onDraw(int n, SkCanvas* canvas) override;
    void onDelayedSetup() override;

private:
    SkString                fName;
    SkColorType             fColorType;
    SkAutoTUnref<SkData>    fData;
    SkAutoMalloc            fPixelStorage;
    typedef Benchmark INHERITED;
};
#endif // DecodingBench_DEFINED
