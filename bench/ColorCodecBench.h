/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef ColorCodecBench_DEFINED
#define ColorCodecBench_DEFINED

#include "Benchmark.h"
#include "SkAutoMalloc.h"
#include "SkData.h"
#include "SkImageInfo.h"

class ColorCodecBench : public Benchmark {
public:
    ColorCodecBench(const char* name, sk_sp<SkData> encoded);

protected:
    const char* onGetName() override;
    bool isSuitableFor(Backend backend) override;
    void onDraw(int n, SkCanvas* canvas) override;
    void onDelayedSetup() override;

private:
    void decodeAndXform();
    void xformOnly();

    SkString            fName;
    sk_sp<SkData>       fEncoded;
    SkImageInfo         fSrcInfo;
    SkImageInfo         fDstInfo;
    SkAutoMalloc        fDst;
    SkAutoMalloc        fSrc;
    sk_sp<SkColorSpace> fDstSpace;
    sk_sp<SkColorSpace> fSrcSpace;

    typedef Benchmark INHERITED;
};
#endif // ColorCodecBench_DEFINED
