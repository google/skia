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
 * It uses an input width, height, left, and top to decode a single subset.
 *
 */
class SubsetSingleBench : public Benchmark {
public:

    SubsetSingleBench(const SkString& path,
                      SkColorType colorType,
                      uint32_t subsetWidth,
                      uint32_t subsetHeight,
                      uint32_t offsetLeft,
                      uint32_t offsetTop,
                      bool useCodec);

protected:
    const char* onGetName() override;
    bool isSuitableFor(Backend backend) override;
    void onDraw(const int n, SkCanvas* canvas) override;

private:
    SkString                      fName;
    SkColorType                   fColorType;
    const uint32_t                fSubsetWidth;
    const uint32_t                fSubsetHeight;
    const uint32_t                fOffsetLeft;
    const uint32_t                fOffsetTop;
    const bool                    fUseCodec;
    SkAutoTDelete<SkMemoryStream> fStream;
    typedef Benchmark INHERITED;
};
