/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef CodecBench_DEFINED
#define CodecBench_DEFINED

#include "bench/Benchmark.h"
#include "include/core/SkData.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkString.h"
#include "src/core/SkAutoMalloc.h"

/**
 *  Time SkCodec.
 */
class CodecBench : public Benchmark {
public:
    // Calls encoded->ref()
    CodecBench(SkString basename, SkData* encoded, SkColorType colorType, SkAlphaType alphaType);

protected:
    const char* onGetName() override;
    bool isSuitableFor(Backend backend) override;
    void onDraw(int n, SkCanvas* canvas) override;
    void onDelayedSetup() override;

private:
    SkString                fName;
    const SkColorType       fColorType;
    const SkAlphaType       fAlphaType;
    sk_sp<SkData>           fData;
    SkImageInfo             fInfo;          // Set in onDelayedSetup.
    SkAutoMalloc            fPixelStorage;
    typedef Benchmark INHERITED;
};
#endif // CodecBench_DEFINED
