/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef CodecBench_DEFINED
#define CodecBench_DEFINED

#include "Benchmark.h"
#include "SkData.h"
#include "SkImageInfo.h"
#include "SkRefCnt.h"
#include "SkString.h"

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
    SkAutoTUnref<SkData>    fData;
    SkImageInfo             fInfo;          // Set in onDelayedSetup.
    SkAutoMalloc            fPixelStorage;
    typedef Benchmark INHERITED;
};
#endif // CodecBench_DEFINED
