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
 * This benchmark is designed to test the performance of image decoding.
 * It is invoked from the nanobench.cpp file.
 *
 */
class DecodingBench : public Benchmark {
public:
    DecodingBench(SkString path, SkColorType colorType);

protected:
    const char* onGetName() SK_OVERRIDE;
    bool isSuitableFor(Backend backend) SK_OVERRIDE;
    void onDraw(const int n, SkCanvas* canvas) SK_OVERRIDE;
    
private:
    SkString fName;
    SkColorType fColorType;
    SkAutoTDelete<SkMemoryStream> fStream;
    SkAutoTDelete<SkImageDecoder> fDecoder;
    typedef Benchmark INHERITED;
};
