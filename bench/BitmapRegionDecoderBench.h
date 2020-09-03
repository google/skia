/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef BitmapRegionDecoderBench_DEFINED
#define BitmapRegionDecoderBench_DEFINED

#include "bench/Benchmark.h"
#ifdef SK_ENABLE_ANDROID_UTILS
#include "include/core/SkData.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkString.h"

namespace android {
namespace skia {
class BitmapRegionDecoder;
}  // namespace skia
}  // namespace android

/**
 *  Benchmark Android's BitmapRegionDecoder for a particular colorType, sampleSize, and subset.
 *
 *  nanobench.cpp handles creating benchmarks for interesting scaled subsets.  We strive to test
 *  on real use cases.
 */
class BitmapRegionDecoderBench : public Benchmark {
public:
    // Calls encoded->ref()
    BitmapRegionDecoderBench(const char* basename, SkData* encoded, SkColorType colorType,
            uint32_t sampleSize, const SkIRect& subset);

protected:
    const char* onGetName() override;
    bool isSuitableFor(Backend backend) override;
    void onDraw(int n, SkCanvas* canvas) override;
    void onDelayedSetup() override;

private:
    SkString                                            fName;
    std::unique_ptr<android::skia::BitmapRegionDecoder> fBRD;
    sk_sp<SkData>                                       fData;
    const SkColorType                                   fColorType;
    const uint32_t                                      fSampleSize;
    const SkIRect                                       fSubset;
    using INHERITED = Benchmark;
};
#endif // SK_ENABLE_ANDROID_UTILS
#endif // BitmapRegionDecoderBench_DEFINED
