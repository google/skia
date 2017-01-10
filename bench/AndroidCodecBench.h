/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef AndroidCodecBench_DEFINED
#define AndroidCodecBench_DEFINED

#include "Benchmark.h"
#include "SkAutoMalloc.h"
#include "SkData.h"
#include "SkImageInfo.h"
#include "SkRefCnt.h"
#include "SkString.h"

/**
 *  Time SkAndroidCodec.
 */
class AndroidCodecBench : public Benchmark {
public:
    // Calls encoded->ref()
    AndroidCodecBench(SkString basename, SkData* encoded, int sampleSize);

protected:
    const char* onGetName() override;
    bool isSuitableFor(Backend backend) override;
    void onDraw(int n, SkCanvas* canvas) override;
    void onDelayedSetup() override;

private:
    SkString                fName;
    sk_sp<SkData>           fData;
    const int               fSampleSize;
    SkImageInfo             fInfo;          // Set in onDelayedSetup.
    SkAutoMalloc            fPixelStorage;  // Set in onDelayedSetup.
    typedef Benchmark INHERITED;
};
#endif // AndroidCodecBench_DEFINED
