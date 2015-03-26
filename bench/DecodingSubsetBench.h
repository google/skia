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
 * This benchmark is designed to test the performance of image subset decoding.
 * It is invoked from the nanobench.cpp file.
 *
 */
class DecodingSubsetBench : public Benchmark {
public:
    DecodingSubsetBench(SkString path, SkColorType colorType,
            const int divisor);

protected:
    const char* onGetName() override;
    bool isSuitableFor(Backend backend) override;
    void onDraw(const int n, SkCanvas* canvas) override;
    
private:
    SkString fName;
    SkColorType fColorType;
    const int fDivisor;
    SkAutoTDelete<SkMemoryStream> fStream;
    SkAutoTDelete<SkImageDecoder> fDecoder;
    typedef Benchmark INHERITED;
};
