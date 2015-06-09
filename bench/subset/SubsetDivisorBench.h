/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Benchmark.h"
#include "SkImageDecoder.h"
#include "SkImageInfo.h"
#include "SkStream.h"
#include "SkString.h"

/*
 *
 * This benchmark is designed to test the performance of subset decoding.
 * It uses a divisor to decode the entire image in a grid of divisor x divisor blocks.
 *
 */
class SubsetDivisorBench : public Benchmark {
public:

    SubsetDivisorBench(const SkString& path,
                       SkColorType colorType,
                       uint32_t divisor,
                       bool useCodec);

protected:
    const char* onGetName() override;
    bool isSuitableFor(Backend backend) override;
    void onDraw(const int n, SkCanvas* canvas) override;

private:
    SkString                      fName;
    SkColorType                   fColorType;
    const uint32_t                fDivisor;
    const bool                    fUseCodec;
    SkAutoTDelete<SkMemoryStream> fStream;
    typedef Benchmark INHERITED;
};
